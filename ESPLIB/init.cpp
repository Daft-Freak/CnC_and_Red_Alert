#include <stdio.h>
#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"

#include "esp_err.h"
#include "esp_vfs_fat.h"
#include "esp_wifi.h"

#ifdef ESP_HOSTED
#include "esp_hosted.h"
#endif

#include "sdmmc_cmd.h"
#include "sd_pwr_ctrl_by_on_chip_ldo.h"

#include "driver/sdspi_host.h"
#include "driver/sdmmc_host.h"

#include "diskio_impl.h"
#include "diskio_sdmmc.h"

#include "config.h"
#include "driver/audio.h"
#include "display.h"
#include "timer.h"
#include "usb.h"

#include "picolib.h"

static sdmmc_host_t sd_host;
static sdmmc_card_t sd_card;

// wifi
static EventGroupHandle_t wifi_event_group;

#define WIFI_CONNECTED_BIT BIT0
#define WIFI_FAIL_BIT      BIT1

static int wifi_retry_num = 0;

static void wifi_event_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data)
{
    if(event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START)
        esp_wifi_connect();
    else if(event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED)
    {
        if (wifi_retry_num < 10)
        {
            esp_wifi_connect();
            wifi_retry_num++;
            printf("retrying wifi connection...\n");
        }
        else
            xEventGroupSetBits(wifi_event_group, WIFI_FAIL_BIT);
    }
    else if(event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP)
    {
        ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
        printf("wifi ip:" IPSTR "\n", IP2STR(&event->ip_info.ip));
        wifi_retry_num = 0;
        xEventGroupSetBits(wifi_event_group, WIFI_CONNECTED_BIT);
    }
}


extern const uint8_t asset_tall_font[];


// draws a line of text in the middle of the screen at 2x scale
static void Draw_Basic_Text(const char * str)
{
    // font data
    auto font_data = asset_tall_font + 8 + asset_tall_font[4];
    int char_w = asset_tall_font[5];
    int char_h = asset_tall_font[6];
    int spacing_y = asset_tall_font[7];
    auto char_w_variable = asset_tall_font + 8;

    const int height_bytes = (char_h + 7) / 8;
    const int char_size = char_w * height_bytes;

    // framebuffer
    auto framebuffer = get_framebuffer();

    // find start point
    int cursor_x = 0, cursor_y = 100 - char_h;

    for(auto str_ptr = str; *str_ptr; str_ptr++)
    {
        uint8_t chr_idx = *str_ptr & 0x7F;
        chr_idx = chr_idx < ' ' ? 0 : chr_idx - ' ';

        cursor_x += char_w_variable[chr_idx];
    }

    cursor_x = 160 - cursor_x;

    // draw it
    for(auto str_ptr = str; *str_ptr; str_ptr++)
    {
        // draw character
        uint8_t chr_idx = *str_ptr & 0x7F;
        chr_idx = chr_idx < ' ' ? 0 : chr_idx - ' ';

        const uint8_t *font_chr = &font_data[chr_idx * char_size];

        int char_width = char_w_variable[chr_idx];

        for(uint8_t y = 0; y < char_h; y++)
        {
            if(cursor_y + y < 0)
                continue;
    
            auto out_ptr = framebuffer + cursor_x + (cursor_y + y * 2) * 320;
    
            for(uint8_t x = 0; x < char_w; x++)
            {
                int bit = 1 << (y & 7);
                if (font_chr[x * height_bytes + y / 8] & bit)
                    out_ptr[0] = out_ptr[1] = out_ptr[320] = out_ptr[321] = 1;
        
                out_ptr += 2;
            }
        }

        // increment the cursor
        cursor_x += char_width * 2;
    }
}

static void Display_Loading_Message()
{
    memset(get_framebuffer(), 0, 320 * 200);

    // minimal palette
    uint8_t col[]{0, 0, 0, 63, 63, 63};
    set_screen_palette(col, 2);

    Draw_Basic_Text("Please Stand By...");

    // display it
    update_display(Get_Time_Ms());

    // wait for DBI's late backlight enabling
    //while(!display_render_needed()) __wfe();
    update_display(Get_Time_Ms());
}

void Init_SD()
{
    FATFS *fs = nullptr;

#ifdef SD_SPI
    // int spi bus
    spi_host_device_t spi_slot = SDSPI_DEFAULT_HOST;
    spi_bus_config_t bus_config = {};
    bus_config.mosi_io_num = SD_SPI_MOSI_PIN;
    bus_config.miso_io_num = SD_SPI_MISO_PIN;
    bus_config.sclk_io_num = SD_SPI_SCK_PIN;
    ESP_ERROR_CHECK(spi_bus_initialize(spi_slot, &bus_config, SDSPI_DEFAULT_DMA));

    // init sdspi host
    ESP_ERROR_CHECK(sdspi_host_init());

    sdspi_dev_handle_t sdspi_handle;
    sdspi_device_config_t sdspi_config = SDSPI_DEVICE_CONFIG_DEFAULT();
    sdspi_config.gpio_cs = gpio_num_t(SD_SPI_CS_PIN);
    sdspi_config.host_id = spi_slot;
    ESP_ERROR_CHECK(sdspi_host_init_device(&sdspi_config, &sdspi_handle));

    // prepare to init sdmmc host
    sd_host = SDSPI_HOST_DEFAULT();
    sd_host.slot = spi_slot;
#elif defined(SD_SDMMC)
    // init sdmmc
    ESP_ERROR_CHECK(sdmmc_host_init());

    sdmmc_slot_config_t slot_config = SDMMC_SLOT_CONFIG_DEFAULT();

#ifdef SD_SDMMC_1BIT
    slot_config.width = 1;
#endif

    ESP_ERROR_CHECK(sdmmc_host_init_slot(0, &slot_config));

    sd_host = SDMMC_HOST_DEFAULT();
    sd_host.slot = 0;
#endif

#ifdef SD_LDO_ID
    // setup LDO
    sd_pwr_ctrl_ldo_config_t ldo_config = {
        .ldo_chan_id = SD_LDO_ID,
    };
    sd_pwr_ctrl_handle_t pwr_ctrl_handle = nullptr;

    ESP_ERROR_CHECK(sd_pwr_ctrl_new_on_chip_ldo(&ldo_config, &pwr_ctrl_handle));
    sd_host.pwr_ctrl_handle = pwr_ctrl_handle;
#endif

#ifdef SD_HIGH_SPEED
    sd_host.max_freq_khz = SDMMC_FREQ_HIGHSPEED;
#endif

    // init card
    if(sdmmc_card_init(&sd_host, &sd_card) != ESP_OK)
    {
        printf("failed to init sd card\n");
        return;
    }
    
    // mount fs
    BYTE pdrv;
    ESP_ERROR_CHECK(ff_diskio_get_drive(&pdrv));

    ff_diskio_register_sdmmc(pdrv, &sd_card);

    esp_vfs_fat_conf_t vfs_config = {};
    vfs_config.base_path = "";
    vfs_config.fat_drive = "";
    vfs_config.max_files = 2;

    ESP_ERROR_CHECK(esp_vfs_fat_register_cfg(&vfs_config, &fs));

    f_mount(fs, "", 0);
}

void Pico_Init(const char *basedir)
{
    init_display();
    init_audio();

    Init_SD();

    extern char path_prefix[10];
    snprintf(path_prefix, sizeof(path_prefix), "/%s/", basedir);

    Pico_Flash_Cache_Init();

    init_usb();

    Pico_Input_Init();

    Display_Loading_Message();
}

void Pico_Wifi_Init(const char *ssid, const char *pass)
{
#ifdef ESP_HOSTED
    esp_hosted_init();
#endif

    wifi_event_group = xEventGroupCreate();

    ESP_ERROR_CHECK(esp_netif_init());

    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_sta();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    esp_event_handler_instance_t instance_any_id;
    esp_event_handler_instance_t instance_got_ip;
    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT,
                                                        ESP_EVENT_ANY_ID,
                                                        &wifi_event_handler,
                                                        NULL,
                                                        &instance_any_id));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT,
                                                        IP_EVENT_STA_GOT_IP,
                                                        &wifi_event_handler,
                                                        NULL,
                                                        &instance_got_ip));

    wifi_config_t wifi_config = {};

    strncpy((char *)wifi_config.sta.ssid, ssid, 31);
    strncpy((char *)wifi_config.sta.password, pass, 63);
    wifi_config.sta.threshold.authmode = WIFI_AUTH_WPA_PSK;
    wifi_config.sta.sae_pwe_h2e = WPA3_SAE_PWE_BOTH;

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());

    // wait for connection or error
    EventBits_t bits = xEventGroupWaitBits(wifi_event_group, WIFI_CONNECTED_BIT | WIFI_FAIL_BIT, pdFALSE, pdFALSE, portMAX_DELAY);

    if (bits & WIFI_CONNECTED_BIT)
        printf("Connected to SSID:%s password:%s\n", ssid, pass);
    else if (bits & WIFI_FAIL_BIT)
        printf("Failed to connect to SSID:%s password:%s\n", ssid, pass); 
}

// app_main -> main wrapper
int main(int argc, char * argv[]);

extern "C" void app_main()
{
    char *argv[]{"ra"};
    main(1, argv);
}