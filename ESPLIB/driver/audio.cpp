#include <array>

#include "driver/i2s_std.h"
#include "driver/gpio.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "audio.h"
#include "config.h"

#define AUDIO_BUFFER_SIZE 256

#ifdef AUDIO_I2S
static i2s_chan_handle_t i2s_handle = nullptr;

static void audio_task(void *)
{
    int16_t samples[AUDIO_BUFFER_SIZE];
    while(true)
    {
        auto buffer = samples;

        auto buf_end = buffer + AUDIO_BUFFER_SIZE / 2;
        audio_callback(buf_end, AUDIO_BUFFER_SIZE / 2);

        for(int i = 0; i < AUDIO_BUFFER_SIZE; i += 2)
        {
            // 22050 -> 44100
            auto val = *buf_end++;
            *buffer++ = val;
            *buffer++ = val;
        }

        i2s_channel_write(i2s_handle, &samples, sizeof(samples), nullptr, portMAX_DELAY);
    }
}
#endif

void init_audio()
{
#ifdef AUDIO_I2S
#ifdef AUDIO_I2S_MUTE_PIN
    // configure mute pin
    gpio_config_t mute_gpio_config = {};
    mute_gpio_config.mode = GPIO_MODE_OUTPUT;
    mute_gpio_config.pin_bit_mask = 1ULL << AUDIO_I2S_MUTE_PIN;

    ESP_ERROR_CHECK(gpio_config(&mute_gpio_config));
#endif

    // init channel
    i2s_chan_config_t chan_config = I2S_CHANNEL_DEFAULT_CONFIG(I2S_NUM_AUTO, I2S_ROLE_MASTER);
    chan_config.dma_frame_num = 512;
    ESP_ERROR_CHECK(i2s_new_channel(&chan_config, &i2s_handle, nullptr));

    i2s_std_config_t std_config = {};
    std_config.clk_cfg = I2S_STD_CLK_DEFAULT_CONFIG(44100);
    std_config.slot_cfg = I2S_STD_MSB_SLOT_DEFAULT_CONFIG(I2S_DATA_BIT_WIDTH_16BIT, I2S_SLOT_MODE_MONO);

    std_config.gpio_cfg.mclk = I2S_GPIO_UNUSED;
    std_config.gpio_cfg.bclk = gpio_num_t(AUDIO_I2S_BCLK_PIN);
    std_config.gpio_cfg.ws = gpio_num_t(AUDIO_I2S_LRCLK_PIN);
    std_config.gpio_cfg.dout = gpio_num_t(AUDIO_I2S_DATA_PIN);
    std_config.gpio_cfg.din = I2S_GPIO_UNUSED;

    ESP_ERROR_CHECK(i2s_channel_init_std_mode(i2s_handle, &std_config));

    // enable
    ESP_ERROR_CHECK(i2s_channel_enable(i2s_handle));

#ifdef AUDIO_I2S_MUTE_PIN
    gpio_set_level(gpio_num_t(AUDIO_I2S_MUTE_PIN), 1);
#endif

    // create task
    xTaskCreate(audio_task, "i2s", 2048, nullptr, 5, nullptr);
#endif
}

bool audio_available()
{
#ifdef AUDIO_I2S
    return true;
#else
    return false;
#endif
}