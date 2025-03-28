#include <cstdlib>
#include <stdio.h>
#include <pico/stdlib.h>
#include <math.h>
#include <cstring>

extern "C" {
#include <pico/lock_core.h>
}

#include "display.h"
#include "display_commands.h"

#include "hardware/clocks.h"
#include "hardware/dma.h"
#include "hardware/irq.h"
#include "hardware/pwm.h"
#include "hardware/resets.h"
#include "hardware/structs/bus_ctrl.h"
#include "hardware/structs/hstx_ctrl.h"
#include "hardware/structs/hstx_fifo.h"

#include "hardware/structs/ioqspi.h"
#include "hardware/vreg.h"
#include "hardware/structs/qmi.h"
#include "hardware/pll.h"

#include "pico/binary_info.h"
#include "pico/time.h"

extern "C" {
  #include "dvi.h"
}

#include "config.h"

static uint16_t screen_palette565[256];
static uint8_t frame_buffer[320 * 200];

// For now no back buffer - maybe we can put it in PSRAM?
#define frame_buffer_display frame_buffer
#define frame_buffer_back frame_buffer

#define NUM_CHANS 3
#define NUM_FRAME_LINES 2

// double buffering for lores
static volatile int buf_index = 0;

static volatile bool do_render = true;

static uint32_t vblank_line_vsync_off[6];
static uint32_t vblank_line_vsync_on[6];

static const struct dvi_timing* timing_mode;
static int v_inactive_total;
static int v_total_active_lines;
static const int v_repeat_shift = 1;
static const int h_repeat_shift = 1;

// three scanlines.  7 word header plus one word per pixel (because they are pixel doubled)
static uint32_t line_buffers[2 * (7 + DISPLAY_WIDTH)];

// pixel double scanline counter
static volatile int v_scanline = 0;
static int ch_num = 0;
static int line_num = -1;
static volatile bool blank = true;
static volatile int frame_num = 0;
static volatile int last_frame = 0;

static irq_handler_t cur_irq_handler = nullptr;

// palette lookup
static inline void convert_paletted(const uint8_t *in, uint16_t *out, int count) {
  for(int i = 0; i < count; i++) {
    *out++ = screen_palette565[*in++];
  }
}

static void __no_inline_not_in_flash_func(palette_dma_irq_handler)() {
    // ch_num indicates the channel that just finished, which is the one
    // we're about to reload.
    dma_channel_hw_t *ch = &dma_hw->ch[ch_num];
    dma_hw->intr = 1u << ch_num;
    if (++ch_num == NUM_CHANS) ch_num = 0;

    if (v_scanline >= timing_mode->v_front_porch && v_scanline < (timing_mode->v_front_porch + timing_mode->v_sync_width)) {
          ch->read_addr = (uintptr_t)vblank_line_vsync_on;
          ch->transfer_count = count_of(vblank_line_vsync_on);
    } else if (v_scanline < v_inactive_total) {
        ch->read_addr = (uintptr_t)vblank_line_vsync_off;
        ch->transfer_count = count_of(vblank_line_vsync_off);
    } else {
        int y = (v_scanline - v_inactive_total) >> v_repeat_shift;
        const int new_line_num = (v_repeat_shift == 0) ? ch_num : (y & (NUM_FRAME_LINES - 1));
        const int line_len = DISPLAY_WIDTH * 4;
        const uint line_buf_total_len = (line_len >> 2) + 7;

        ch->read_addr = (uintptr_t)&line_buffers[new_line_num * line_buf_total_len];
        ch->transfer_count = line_buf_total_len;

        // Fill line buffer
        if (line_num != new_line_num)
        {
            line_num = new_line_num;

            if (blank) {
                uint32_t* dst_ptr = &line_buffers[line_num * line_buf_total_len + 7];
                memset(dst_ptr, 0, line_len);
            }
            else {
                uint32_t* dst_ptr = &line_buffers[line_num * line_buf_total_len + 7 + (DISPLAY_WIDTH - 320) / 2];
                uint8_t* src_ptr = &frame_buffer_display[y * 320];
                //if (h_repeat_shift == 1) {
                    for (int i = 0; i < 320; ++i) {
                        uint32_t val = (uint32_t)(screen_palette565[*src_ptr++]) * 0x10001;
                        *dst_ptr++ = val;
                    }
                //}
            }
        }
    }

    if (++v_scanline == v_total_active_lines) {
        v_scanline = 0;
        line_num = -1;
        ++frame_num;
        __sev();
    }
}

static void update() {
  // TODO something something back buffer
}

// ----------------------------------------------------------------------------
// Experimental clock config

static void __no_inline_not_in_flash_func(set_qmi_timing)() {
    // Make sure flash is deselected - QMI doesn't appear to have a busy flag(!)
    while ((ioqspi_hw->io[1].status & IO_QSPI_GPIO_QSPI_SS_STATUS_OUTTOPAD_BITS) != IO_QSPI_GPIO_QSPI_SS_STATUS_OUTTOPAD_BITS)
        ;

    qmi_hw->m[0].timing = 0x40000202;
    //qmi_hw->m[0].timing = 0x40000101;
    // Force a read through XIP to ensure the timing is applied
    volatile uint32_t* ptr = (volatile uint32_t*)0x14000000;
    (void) *ptr;
}

extern "C" void __no_inline_not_in_flash_func(display_setup_clock_preinit)() {
    uint32_t intr_stash = save_and_disable_interrupts();

    // Before messing with clock speeds ensure QSPI clock is nice and slow
    hw_write_masked(&qmi_hw->m[0].timing, 6, QMI_M0_TIMING_CLKDIV_BITS);

    // We're going to go fast, boost the voltage a little
    vreg_set_voltage(VREG_VOLTAGE_1_15);

    // Force a read through XIP to ensure the timing is applied before raising the clock rate
    volatile uint32_t* ptr = (volatile uint32_t*)0x14000000;
    (void) *ptr;

    // Before we touch PLLs, switch sys and ref cleanly away from their aux sources.
    hw_clear_bits(&clocks_hw->clk[clk_sys].ctrl, CLOCKS_CLK_SYS_CTRL_SRC_BITS);
    while (clocks_hw->clk[clk_sys].selected != 0x1)
        tight_loop_contents();
    hw_write_masked(&clocks_hw->clk[clk_ref].ctrl, CLOCKS_CLK_REF_CTRL_SRC_VALUE_XOSC_CLKSRC, CLOCKS_CLK_REF_CTRL_SRC_BITS);
    while (clocks_hw->clk[clk_ref].selected != 0x4)
        tight_loop_contents();

    // Stop the other clocks so we don't worry about overspeed
    clock_stop(clk_usb);
    clock_stop(clk_adc);
    clock_stop(clk_peri);
    clock_stop(clk_hstx);

    // Set USB PLL to 528MHz
    pll_init(pll_usb, PLL_COMMON_REFDIV, 1440 * MHZ, 6, 1);

    const uint32_t usb_pll_freq = 240 * MHZ;

    // CLK SYS = PLL USB = 240MHz
    clock_configure(clk_sys,
                    CLOCKS_CLK_SYS_CTRL_SRC_VALUE_CLKSRC_CLK_SYS_AUX,
                    CLOCKS_CLK_SYS_CTRL_AUXSRC_VALUE_CLKSRC_PLL_USB,
                    usb_pll_freq, usb_pll_freq);

    // CLK PERI = PLL USB 240MHz / 2 = 120MHz
    clock_configure(clk_peri,
                    0, // Only AUX mux on ADC
                    CLOCKS_CLK_PERI_CTRL_AUXSRC_VALUE_CLKSRC_PLL_USB,
                    usb_pll_freq, usb_pll_freq / 2);

    // CLK USB = PLL USB 240MHz / 5 = 48MHz
    clock_configure(clk_usb,
                    0, // No GLMUX
                    CLOCKS_CLK_USB_CTRL_AUXSRC_VALUE_CLKSRC_PLL_USB,
                    usb_pll_freq,
                    USB_CLK_KHZ * KHZ);

    // CLK ADC = PLL USB 240MHz / 5 = 48MHz
    clock_configure(clk_adc,
                    0, // No GLMUX
                    CLOCKS_CLK_ADC_CTRL_AUXSRC_VALUE_CLKSRC_PLL_USB,
                    usb_pll_freq,
                    USB_CLK_KHZ * KHZ);

    // Now we are running fast set fast QSPI clock and read delay
    // On MicroPython this is setup by main.
    set_qmi_timing();

    restore_interrupts(intr_stash);
}

// Trigger clock setup early
void pre_init_display() {
    display_setup_clock_preinit();
    dma_claim_mask((1 << NUM_CHANS) - 1);
}

static void display_setup_clock() {
    const uint32_t dvi_clock_khz = timing_mode->bit_clk_khz >> 1;
    uint vco_freq, post_div1, post_div2;
    if (!check_sys_clock_khz(dvi_clock_khz, &vco_freq, &post_div1, &post_div2))
        panic("System clock of %u kHz cannot be exactly achieved", dvi_clock_khz);
    const uint32_t freq = vco_freq / (post_div1 * post_div2);

    if (timing_mode->bit_clk_khz > 600000) {
        vreg_set_voltage(VREG_VOLTAGE_1_25);
    } 
    if (timing_mode->bit_clk_khz > 800000) {
        vreg_set_voltage(VREG_VOLTAGE_1_30);
        if (timing_mode->bit_clk_khz > 1000000) {
            // YOLO mode
            hw_set_bits(&powman_hw->vreg_ctrl, POWMAN_PASSWORD_BITS | POWMAN_VREG_CTRL_DISABLE_VOLTAGE_LIMIT_BITS);
            vreg_set_voltage(VREG_VOLTAGE_1_40);
        }
        sleep_ms(1);
    }

    // Set the sys PLL to the requested freq
    pll_init(pll_sys, PLL_COMMON_REFDIV, vco_freq, post_div1, post_div2);

    // CLK HSTX = Requested freq
    clock_configure(clk_hstx,
                    0,
                    CLOCKS_CLK_HSTX_CTRL_AUXSRC_VALUE_CLKSRC_PLL_SYS,
                    freq, freq);
}

void init_display() {
}

void init_display_core1() {
    ch_num = 0;
    line_num = -1;
    v_scanline = 2;

    timing_mode = &dvi_timing_720x400p_70hz;
    if (get_core_num() == 1) {
        hw_set_bits(&bus_ctrl_hw->priority, (BUSCTRL_BUS_PRIORITY_PROC1_BITS | BUSCTRL_BUS_PRIORITY_DMA_R_BITS | BUSCTRL_BUS_PRIORITY_DMA_W_BITS));
    }

    printf("Setup clock\n");
    display_setup_clock();

    printf("Clock setup done\n");

    v_inactive_total = timing_mode->v_front_porch + timing_mode->v_sync_width + timing_mode->v_back_porch;
    v_total_active_lines = v_inactive_total + timing_mode->v_active_lines;
    const int v_repeat = 1 << v_repeat_shift;
    const int h_repeat = 1 << h_repeat_shift;

    uint32_t sync_v0_h0 = timing_mode->h_sync_polarity ? (timing_mode->v_sync_polarity ? SYNC_V0_H0 : SYNC_V1_H0) :
                                                         (timing_mode->v_sync_polarity ? SYNC_V0_H1 : SYNC_V1_H1);
    uint32_t sync_v0_h1 = timing_mode->h_sync_polarity ? (timing_mode->v_sync_polarity ? SYNC_V0_H1 : SYNC_V1_H1) :
                                                         (timing_mode->v_sync_polarity ? SYNC_V0_H0 : SYNC_V1_H0);
    uint32_t sync_v1_h0 = timing_mode->h_sync_polarity ? (timing_mode->v_sync_polarity ? SYNC_V1_H0 : SYNC_V0_H0) :
                                                         (timing_mode->v_sync_polarity ? SYNC_V1_H1 : SYNC_V0_H1);
    uint32_t sync_v1_h1 = timing_mode->h_sync_polarity ? (timing_mode->v_sync_polarity ? SYNC_V1_H1 : SYNC_V0_H1) :
                                                         (timing_mode->v_sync_polarity ? SYNC_V1_H0 : SYNC_V0_H0);

    vblank_line_vsync_off[0] = HSTX_CMD_RAW_REPEAT | timing_mode->h_front_porch;
    vblank_line_vsync_off[1] = sync_v0_h0;
    vblank_line_vsync_off[2] = HSTX_CMD_RAW_REPEAT | timing_mode->h_sync_width;
    vblank_line_vsync_off[3] = sync_v0_h1;
    vblank_line_vsync_off[4] = HSTX_CMD_RAW_REPEAT | (timing_mode->h_back_porch + timing_mode->h_active_pixels);
    vblank_line_vsync_off[5] = sync_v0_h0;

    vblank_line_vsync_on[0] = HSTX_CMD_RAW_REPEAT | timing_mode->h_front_porch;
    vblank_line_vsync_on[1] = sync_v1_h0;
    vblank_line_vsync_on[2] = HSTX_CMD_RAW_REPEAT | timing_mode->h_sync_width;
    vblank_line_vsync_on[3] = sync_v1_h1;
    vblank_line_vsync_on[4] = HSTX_CMD_RAW_REPEAT | (timing_mode->h_back_porch + timing_mode->h_active_pixels);
    vblank_line_vsync_on[5] = sync_v1_h0;

    for (int i = 0; i < 2; ++i) {
      uint32_t* line_header = &line_buffers[i * (7 + DISPLAY_WIDTH)];
      line_header[0] = HSTX_CMD_RAW_REPEAT | timing_mode->h_front_porch;
      line_header[1] = sync_v0_h0;
      line_header[2] = HSTX_CMD_RAW_REPEAT | timing_mode->h_sync_width;
      line_header[3] = sync_v0_h1;
      line_header[4] = HSTX_CMD_RAW_REPEAT | timing_mode->h_back_porch;
      line_header[5] = sync_v0_h0;
      line_header[6] = HSTX_CMD_TMDS | timing_mode->h_active_pixels;
    }

    printf("Frame buffers inited\n");

    // Ensure HSTX FIFO is clear
    reset_block_num(RESET_HSTX);
    sleep_us(10);
    unreset_block_num_wait_blocking(RESET_HSTX);
    sleep_us(10);

    // Configure HSTX's TMDS encoder for RGB565
    hstx_ctrl_hw->expand_tmds =
        4  << HSTX_CTRL_EXPAND_TMDS_L2_NBITS_LSB |
        8 << HSTX_CTRL_EXPAND_TMDS_L2_ROT_LSB   |
        5  << HSTX_CTRL_EXPAND_TMDS_L1_NBITS_LSB |
        3  << HSTX_CTRL_EXPAND_TMDS_L1_ROT_LSB   |
        4  << HSTX_CTRL_EXPAND_TMDS_L0_NBITS_LSB |
        29 << HSTX_CTRL_EXPAND_TMDS_L0_ROT_LSB;

    // Pixels (TMDS) come in 2 16-bit chunks. Control symbols (RAW) are an
    // entire 32-bit word.
    hstx_ctrl_hw->expand_shift =
        2 << HSTX_CTRL_EXPAND_SHIFT_ENC_N_SHIFTS_LSB |
        16 << HSTX_CTRL_EXPAND_SHIFT_ENC_SHIFT_LSB |
        1 << HSTX_CTRL_EXPAND_SHIFT_RAW_N_SHIFTS_LSB |
        0 << HSTX_CTRL_EXPAND_SHIFT_RAW_SHIFT_LSB;

    // Serial output config: clock period of 5 cycles, pop from command
    // expander every 5 cycles, shift the output shiftreg by 2 every cycle.
    hstx_ctrl_hw->csr = 0;
    hstx_ctrl_hw->csr =
        HSTX_CTRL_CSR_EXPAND_EN_BITS |
        5u << HSTX_CTRL_CSR_CLKDIV_LSB |
        5u << HSTX_CTRL_CSR_N_SHIFTS_LSB |
        2u << HSTX_CTRL_CSR_SHIFT_LSB |
        HSTX_CTRL_CSR_EN_BITS; 

    // HSTX outputs 0 through 7 appear on GPIO 12 through 19.
    constexpr int HSTX_FIRST_PIN = 12;

    // Assign clock pair to two neighbouring pins:
    {
    int bit = DVI_CLK_P - HSTX_FIRST_PIN;
    hstx_ctrl_hw->bit[bit    ] = HSTX_CTRL_BIT0_CLK_BITS;
    hstx_ctrl_hw->bit[bit ^ 1] = HSTX_CTRL_BIT0_CLK_BITS | HSTX_CTRL_BIT0_INV_BITS;
    }

    int rgb_p[3] = { DVI_D0_P, DVI_D1_P, DVI_D2_P };
    for (uint lane = 0; lane < 3; ++lane) {
        // For each TMDS lane, assign it to the correct GPIO pair based on the
        // desired pinout:
        int bit = rgb_p[lane] - HSTX_FIRST_PIN;
        // Output even bits during first half of each HSTX cycle, and odd bits
        // during second half. The shifter advances by two bits each cycle.
        uint32_t lane_data_sel_bits =
            (lane * 10    ) << HSTX_CTRL_BIT0_SEL_P_LSB |
            (lane * 10 + 1) << HSTX_CTRL_BIT0_SEL_N_LSB;
        // The two halves of each pair get identical data, but one pin is inverted.
        hstx_ctrl_hw->bit[bit    ] = lane_data_sel_bits;
        hstx_ctrl_hw->bit[bit ^ 1] = lane_data_sel_bits | HSTX_CTRL_BIT0_INV_BITS;
    }

    for (int i = 12; i <= 19; ++i) {
        gpio_set_function(i, GPIO_FUNC_HSTX);
        gpio_set_drive_strength(i, GPIO_DRIVE_STRENGTH_4MA);
        if (timing_mode->bit_clk_khz > 1000000) {
            gpio_set_slew_rate(i, GPIO_SLEW_RATE_FAST);
        }
    }

    printf("GPIO configured\n");

    // The channels are set up identically, to transfer a whole scanline and
    // then chain to the next channel. Each time a channel finishes, we
    // reconfigure the one that just finished, meanwhile the other channel(s)
    // are already making progress.
    // Using just 2 channels was insufficient to avoid issues with the IRQ.
    dma_channel_config c;
    c = dma_channel_get_default_config(0);
    channel_config_set_chain_to(&c, 1);
    channel_config_set_dreq(&c, DREQ_HSTX);
    dma_channel_configure(
        0,
        &c,
        &hstx_fifo_hw->fifo,
        vblank_line_vsync_off,
        count_of(vblank_line_vsync_off),
        false
    );
    c = dma_channel_get_default_config(1);
    channel_config_set_chain_to(&c, 2);
    channel_config_set_dreq(&c, DREQ_HSTX);
    dma_channel_configure(
        1,
        &c,
        &hstx_fifo_hw->fifo,
        vblank_line_vsync_off,
        count_of(vblank_line_vsync_off),
        false
    );
    for (int i = 2; i < NUM_CHANS; ++i) {
        c = dma_channel_get_default_config(i);
        channel_config_set_chain_to(&c, (i+1) % NUM_CHANS);
        channel_config_set_dreq(&c, DREQ_HSTX);
        dma_channel_configure(
            i,
            &c,
            &hstx_fifo_hw->fifo,
            vblank_line_vsync_off,
            count_of(vblank_line_vsync_off),
            false
        );
    }

    printf("DMA channels claimed\n");

    dma_hw->intr = (1 << NUM_CHANS) - 1;
    dma_hw->ints2 = (1 << NUM_CHANS) - 1;
    dma_hw->inte2 = (1 << NUM_CHANS) - 1;
    irq_set_exclusive_handler(DMA_IRQ_2, palette_dma_irq_handler);
    irq_set_enabled(DMA_IRQ_2, true);
    irq_set_priority(DMA_IRQ_2, 0x40);

    dma_channel_start(0);

    printf("DVHSTX started\n");
}

void update_display(uint32_t time) {
  blank = false;
  last_frame = frame_num;
#if 0
  if((do_render || (!have_vsync && time - last_render >= 20)) && !dma_is_busy()) {

    //::render(time);

    if(!have_vsync) {
      while(dma_is_busy()) {} // may need to wait for lores.
      ::update();
    }

    if(last_render && !backlight_enabled) {
      // the first render should have made it to the screen at this point
      set_backlight(255);
      backlight_enabled = true;
    }

    last_render = time;
    do_render = false;
  }
#endif
}


bool display_render_needed() {
  return last_frame != frame_num;
}

void set_screen_palette(const uint8_t *colours, int num_cols) {
  for(int i = 0; i < num_cols; i++) {
    int r = colours[i * 3 + 2] << 2;
    int g = colours[i * 3 + 1] << 2;
    int b = colours[i * 3 + 0] << 2;
    screen_palette565[i] = (r >> 3) | ((g >> 2) << 5) | ((b >> 3) << 11);
  }
}

uint8_t *get_framebuffer() {
  return frame_buffer;
}