#include <cstring>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "driver/gpio.h"
#include "driver/ppa.h"

#include "esp_lcd_panel_io.h"
#include "esp_lcd_panel_vendor.h"
#include "esp_lcd_panel_ops.h"

#include "display.h"
#include "config.h"

// the CLUT isn't supported by current ESP-IDF
// if the commented out parts related to PPA_BLEND_COLOR_MODE_L8 are removed from
// components/hal/esp32p4/include/hal/ppa_types.h and 
// ppa_ll_blend_set_rx_bg_color_mode/ppa_ll_blend_set_rx_fg_color_mode in ppa_ll.h
// this can be removed...
#undef SOC_PPA_SUPPORTED

static esp_lcd_panel_io_handle_t io_handle = nullptr;
static esp_lcd_panel_handle_t panel_handle = nullptr;

#if SOC_PPA_SUPPORTED
static ppa_client_handle_t ppa_client = nullptr;
static volatile int ppa_blend_state = 0; // 0 = doing palette conv, 1 = doing cursor

extern "C" void ppa_set_clut(ppa_client_handle_t ppa_client, const uint8_t *colours, int num_cols);
#else
static uint16_t palette565[256];
#endif

static uint8_t *back_buffer; // paletted
static uint16_t *front_buffer; // rgb565

// cursor overlay
static uint8_t cursor_data[30 * 24];
static int16_t cursor_x = 0, cursor_y = 200;
static uint8_t cursor_w = 0, cursor_h = 0;

static bool backlight_enabled = false;
static bool render_needed = true;

static void *alloc_display_buffer(size_t size)
{
  // this depends on the bus...
#ifdef LCD_I80
    return esp_lcd_i80_alloc_draw_buffer(io_handle, size, 0);
#else
    return nullptr;
#endif
}

static bool on_color_trans_done(esp_lcd_panel_io_handle_t panel_io, esp_lcd_panel_io_event_data_t *edata, void *user_ctx)
{
#ifdef LCD_BACKLIGHT_PIN
    // enable backlight
    if(!backlight_enabled) {
        gpio_set_level(gpio_num_t(LCD_BACKLIGHT_PIN), 1);
        backlight_enabled = true;
    }
#endif

    render_needed = true;

    return false;
}

#if SOC_PPA_SUPPORTED

static void draw_ppa_cursor()
{
    int start_x = cursor_x < 0 ? 0 : cursor_x;
    int end_x = cursor_x + cursor_w;
    if(end_x > 320)
        end_x = 320;

    int start_y = cursor_y < 0 ? 0 : cursor_y;
    int end_y = cursor_y + cursor_h;
    if(end_y > 200)
        end_y = 200;

    ppa_blend_oper_config_t blend_config = {};

    // existing front buffer
    blend_config.in_bg.buffer = front_buffer;
    blend_config.in_bg.pic_w = 320;
    blend_config.in_bg.pic_h = 200;
    blend_config.in_bg.blend_cm = PPA_BLEND_COLOR_MODE_RGB565;

    // cursor image
    blend_config.in_fg.buffer = cursor_data;
    blend_config.in_fg.pic_w = cursor_w;
    blend_config.in_fg.pic_h = cursor_h;
    blend_config.in_fg.block_offset_x = start_x - cursor_x;
    blend_config.in_fg.block_offset_y = start_y - cursor_y;
    blend_config.in_fg.blend_cm = PPA_BLEND_COLOR_MODE_L8;

    // output back to front buffer
    blend_config.out.buffer = front_buffer;
    blend_config.out.buffer_size = 320 * 200 * 2;
    blend_config.out.pic_w = 320;
    blend_config.out.pic_h = 200;
    blend_config.out.blend_cm = PPA_BLEND_COLOR_MODE_RGB565;

    // (clipped) cursor size
    blend_config.in_bg.block_w = blend_config.in_fg.block_w = end_x - start_x;
    blend_config.in_bg.block_h = blend_config.in_fg.block_h = end_y - start_y;

    // cursor pos
    blend_config.in_bg.block_offset_x = blend_config.out.block_offset_x = start_x;
    blend_config.in_bg.block_offset_y = blend_config.out.block_offset_y = start_y;

    blend_config.mode = PPA_TRANS_MODE_NON_BLOCKING;

    ppa_do_blend(ppa_client, &blend_config);
}

static bool on_ppa_trans_done(ppa_client_handle_t ppa_client, ppa_event_data_t *event_data, void *user_data)
{
    if(ppa_blend_state == 0 && cursor_y < 200)
        draw_ppa_cursor();
    else
    {
        // display
        int y_margin = (DISPLAY_HEIGHT - 200) / 2;
        esp_lcd_panel_draw_bitmap(panel_handle, 0, y_margin, 320, y_margin + 200, front_buffer);
    }
    ppa_blend_state++;

    return false;
}
#endif

void init_display()
{
#ifdef LCD_BACKLIGHT_PIN
    // backlight
    gpio_config_t backlight_gpio_config = {};
    backlight_gpio_config.mode = GPIO_MODE_OUTPUT;
    backlight_gpio_config.pin_bit_mask = 1ULL << LCD_BACKLIGHT_PIN;

    ESP_ERROR_CHECK(gpio_config(&backlight_gpio_config));
    gpio_set_level(gpio_num_t(LCD_BACKLIGHT_PIN), 0);
#endif

#ifdef LCD_I80
    // init "I80" bus (8-bit)
    esp_lcd_i80_bus_handle_t i80_bus = nullptr;

    esp_lcd_i80_bus_config_t bus_config = {};

    bus_config.clk_src = LCD_CLK_SRC_DEFAULT;
    bus_config.dc_gpio_num = LCD_DC_PIN;
    bus_config.wr_gpio_num = LCD_WR_PIN;
    bus_config.data_gpio_nums[0] = LCD_DATA0_PIN;
    bus_config.data_gpio_nums[1] = LCD_DATA1_PIN;
    bus_config.data_gpio_nums[2] = LCD_DATA2_PIN;
    bus_config.data_gpio_nums[3] = LCD_DATA3_PIN;
    bus_config.data_gpio_nums[4] = LCD_DATA4_PIN;
    bus_config.data_gpio_nums[5] = LCD_DATA5_PIN;
    bus_config.data_gpio_nums[6] = LCD_DATA6_PIN;
    bus_config.data_gpio_nums[7] = LCD_DATA7_PIN;

    bus_config.bus_width = 8;
    bus_config.max_transfer_bytes = DISPLAY_WIDTH * 200 * sizeof(uint16_t);
    bus_config.dma_burst_size = 64;

    ESP_ERROR_CHECK(esp_lcd_new_i80_bus(&bus_config, &i80_bus));

    // init panel io
    esp_lcd_panel_io_i80_config_t io_config = {};
    io_config.cs_gpio_num = LCD_CS_PIN,
    io_config.pclk_hz = LCD_CLOCK,
    io_config.trans_queue_depth = 10,

    io_config.dc_levels.dc_idle_level = 0,
    io_config.dc_levels.dc_cmd_level = 0,
    io_config.dc_levels.dc_dummy_level = 0,
    io_config.dc_levels.dc_data_level = 1,

    io_config.lcd_cmd_bits = 8,
    io_config.lcd_param_bits = 8,
    io_config.flags.swap_color_bytes = 1;

    ESP_ERROR_CHECK(esp_lcd_new_panel_io_i80(i80_bus, &io_config, &io_handle));
#endif

    esp_lcd_panel_io_callbacks_t io_callbacks = {};
    io_callbacks.on_color_trans_done = on_color_trans_done;

    ESP_ERROR_CHECK(esp_lcd_panel_io_register_event_callbacks(io_handle, &io_callbacks, nullptr));

    // init panel
    esp_lcd_panel_dev_config_t panel_config = {};
    panel_config.reset_gpio_num = LCD_RESET_PIN;
    panel_config.rgb_ele_order = LCD_RGB_ELEMENT_ORDER_RGB;
    panel_config.bits_per_pixel = 16;

#ifdef LCD_ST7789
    ESP_ERROR_CHECK(esp_lcd_new_panel_st7789(io_handle, &panel_config, &panel_handle));

    esp_lcd_panel_reset(panel_handle);
    esp_lcd_panel_init(panel_handle);

    esp_lcd_panel_invert_color(panel_handle, false);
    esp_lcd_panel_set_gap(panel_handle, 0, 0);
    //esp_lcd_panel_swap_xy(panel_handle, true);
    //esp_lcd_panel_mirror(panel_handle, false, true);

    if(DISPLAY_WIDTH == 320 && DISPLAY_HEIGHT == 240)
    {
        esp_lcd_panel_io_tx_param(io_handle, 0xB2/*PORCTRL*/, (uint8_t[]) {
            0x0c, 0x0c, 0x00, 0x33, 0x33
        }, 5);

        esp_lcd_panel_io_tx_param(io_handle, 0xB7/*GCTRL*/, (uint8_t[]) {
            0x35
        }, 1);

        esp_lcd_panel_io_tx_param(io_handle, 0xBB/*VCOMS*/, (uint8_t[]) {
            0x1f
        }, 1);

        esp_lcd_panel_io_tx_param(io_handle, 0xC0/*LCMCTRL*/, (uint8_t[]) {
            0x1f
        }, 1);

        esp_lcd_panel_io_tx_param(io_handle, 0xC2/*VDVVRHEN*/, (uint8_t[]) {
            0x01
        }, 1);

        esp_lcd_panel_io_tx_param(io_handle, 0xC3/*VRHS*/, (uint8_t[]) {
            0x12
        }, 1);

        esp_lcd_panel_io_tx_param(io_handle, 0xC4/*VDVS*/, (uint8_t[]) {
            0x20
        }, 1);

        esp_lcd_panel_io_tx_param(io_handle, 0xD0/*PWCTRL1*/, (uint8_t[]) {
            0xa4, 0xa1
        }, 2);

        esp_lcd_panel_io_tx_param(io_handle, 0xD6/*???*/, (uint8_t[]) {
            0xa1
        }, 1);

        esp_lcd_panel_io_tx_param(io_handle, 0xE0/*PVGAMCTRL*/, (uint8_t[]) {
            0xD0, 0x08, 0x11, 0x08, 0x0C, 0x15, 0x39, 0x33, 0x50, 0x36, 0x13, 0x14, 0x29, 0x2D
        }, 14);

        esp_lcd_panel_io_tx_param(io_handle, 0xE1/*NVGAMCTRL*/, (uint8_t[]) {
            0xD0, 0x08, 0x10, 0x08, 0x06, 0x06, 0x39, 0x44, 0x51, 0x0B, 0x16, 0x14, 0x2F, 0x31
        }, 14);
    }
#endif

    ESP_ERROR_CHECK(esp_lcd_panel_disp_on_off(panel_handle, true));

    // alloc buffers
    front_buffer = (uint16_t *)alloc_display_buffer(320 * 200 * 2);
    back_buffer = (uint8_t *)alloc_display_buffer(320 * 200);

    // clear margins
    int y_margin = (DISPLAY_HEIGHT - 200) / 2;
    esp_lcd_panel_draw_bitmap(panel_handle, 0, 0, 320, y_margin, front_buffer);
    esp_lcd_panel_draw_bitmap(panel_handle, 0, y_margin + 200, 320, DISPLAY_HEIGHT, front_buffer);

    // pixel-processing accelerator
#if SOC_PPA_SUPPORTED
    ppa_client_config_t ppa_config = {};
    ppa_config.oper_type = PPA_OPERATION_BLEND;
    ppa_config.max_pending_trans_num = 1;
    ppa_config.data_burst_length = PPA_DATA_BURST_LENGTH_128;
    ESP_ERROR_CHECK(ppa_register_client(&ppa_config, &ppa_client));

    ppa_event_callbacks_t ppa_callbacks = {};
    ppa_callbacks.on_trans_done = on_ppa_trans_done;
    ESP_ERROR_CHECK(ppa_client_register_event_callbacks(ppa_client, &ppa_callbacks));
#endif
}

void update_display(uint32_t time)
{
    if(!render_needed)
        return;

    // pal -> 565
#if SOC_PPA_SUPPORTED
    ppa_blend_oper_config_t blend_config = {};
    blend_config.in_bg.buffer = back_buffer;
    blend_config.in_bg.pic_w = blend_config.in_bg.block_w = 320;
    blend_config.in_bg.pic_h = blend_config.in_bg.block_h = 200;
    blend_config.in_bg.blend_cm = PPA_BLEND_COLOR_MODE_L8;

    blend_config.in_fg = blend_config.in_bg; // FIXME: setting bypass not supported...

    blend_config.out.buffer = front_buffer;
    blend_config.out.buffer_size = 320 * 200 * 2;
    blend_config.out.pic_w = 320;
    blend_config.out.pic_h = 200;
    blend_config.out.blend_cm = PPA_BLEND_COLOR_MODE_RGB565;

    blend_config.mode = PPA_TRANS_MODE_NON_BLOCKING;

    ppa_blend_state = 0;
    ppa_do_blend(ppa_client, &blend_config);

#else
    auto in = back_buffer;
    auto out = front_buffer;
    for(int i = 0; i < 320 * 200; i++)
        *out++ = palette565[*in++];

    // cursor
    if(cursor_y < 200)
    {
        int start_x = cursor_x < 0 ? 0 : cursor_x;
        int end_x = cursor_x + cursor_w;
        if(end_x > 320)
            end_x = 320;

        int start_y = cursor_y < 0 ? 0 : cursor_y;
        int end_y = cursor_y + cursor_h;
        if(end_y > 200)
            end_y = 200;

        for(int y = start_y; y < end_y; y++)
        {
            for(int x = start_x; x < end_x; x++)
            {
                auto px = cursor_data[(x - cursor_x) + (y - cursor_y) * cursor_w];

                if(px)
                    front_buffer[x + y * 320] = palette565[px];
            }
        }
    }

    int y_margin = (DISPLAY_HEIGHT - 200) / 2;
    esp_lcd_panel_draw_bitmap(panel_handle, 0, y_margin, 320, y_margin + 200, front_buffer);
#endif

    render_needed = false;
}

void set_screen_palette(const uint8_t *colours, int num_cols)
{
#if SOC_PPA_SUPPORTED
    ppa_set_clut(ppa_client, colours, num_cols);
#else
    for(int i = 0; i < num_cols; i++)
    {
        int r = colours[i * 3 + 0] << 2;
        int g = colours[i * 3 + 1] << 2;
        int b = colours[i * 3 + 2] << 2;
        palette565[i] = (r >> 3) | ((g >> 2) << 5) | ((b >> 3) << 11);
    }
#endif
}

bool display_render_needed()
{
    return render_needed;
}

uint8_t *get_framebuffer()
{
    return back_buffer;
}

void display_lock_framebuffer()
{
#if SOC_PPA_SUPPORTED
    //
    // TODO: should probably be smarter about this
    while(ppa_blend_state == 0) vTaskDelay(1);
#else
    // we did the copy in update (because this path is not very optimised...)
#endif
}

void display_set_cursor(uint8_t *data, int w, int h)
{
    memcpy(cursor_data, data, w * h);
    cursor_w = w;
    cursor_h = h;
}

void display_set_cursor_pos(int x, int y)
{
    cursor_x = x;
    cursor_y = y;
}