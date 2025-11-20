// Arduino NINA/Adafruit Airlift

#include <algorithm>
#include <cstring>

#include "hardware/gpio.h"
#include "hardware/spi.h"
#include "pico/time.h"

#include "wifi_nina.h"
#include "config.h"

#ifdef WIFI_ESP32_NINA

#define wifi_spi __CONCAT(spi, WIFI_ESP32_NINA_SPI)

enum NINACommand
{
    SET_NET              = 0x10,
    SET_PASSPHRASE       = 0x11,
    SET_IP               = 0x14,
    SET_DNS_CONFIG       = 0x15,
    SET_HOSTNAME         = 0x16,
    SET_AP_NET           = 0x18,
    SET_AP_PASSPHRASE    = 0x19,
    SET_DEBUG            = 0x1A,

    GET_CONN_STATUS      = 0x20,
    GET_IPADDR           = 0x21,
    GET_MACADDR          = 0x22,
    GET_CURR_SSID        = 0x23,
    GET_CURR_BSSID       = 0x24,
    GET_CURR_RSSI        = 0x25,
    GET_CURR_ENCT        = 0x26,

    SCAN_NETWORKS        = 0x27,
    START_SERVER_TCP     = 0x28,
    GET_SOCKET           = 0x3F,
    GET_STATE_TCP        = 0x29,
    DATA_SENT_TCP        = 0x2A,
    AVAIL_DATA_TCP       = 0x2B,
    GET_DATA_TCP         = 0x2C,
    START_CLIENT_TCP     = 0x2D,
    STOP_CLIENT_TCP      = 0x2E,
    GET_CLIENT_STATE_TCP = 0x2F,
    DISCONNECT           = 0x30,
    GET_IDX_RSSI         = 0x32,
    GET_IDX_ENCT         = 0x33,
    REQ_HOST_BY_NAME     = 0x34,
    GET_HOST_BY_NAME     = 0x35,
    START_SCAN_NETWORKS  = 0x36,
    GET_FW_VERSION       = 0x37,
    SEND_UDP_DATA        = 0x39,
    GET_REMOTE_DATA      = 0x3A,
    GET_TIME             = 0x3B,
    GET_IDX_BSSID        = 0x3C,
    GET_IDX_CHAN         = 0x3D,
    PING                 = 0x3E,

    SEND_DATA_TCP        = 0x44,
    GET_DATABUF_TCP      = 0x45,
    INSERT_DATABUF_TCP   = 0x46,
    SET_ENT_IDENT        = 0x4A,
    SET_ENT_UNAME        = 0x4B,
    SET_ENT_PASSWD       = 0x4C,
    SET_ENT_ENABLE       = 0x4F,
    SET_CLI_CERT         = 0x40,
    SET_PK               = 0x41,

    SET_PIN_MODE         = 0x50,
    SET_DIGITAL_WRITE    = 0x51,
    SET_ANALOG_WRITE     = 0x52,
    SET_DIGITAL_READ     = 0x53,
    SET_ANALOG_READ      = 0x54,
};

enum NINAConnStatus
{
    NO_MODULE       = 0xFF,
    IDLE            = 0,
    NO_SSID_AVAIL   = 1,
    SCAN_COMPLETED  = 2,
    CONNECTED       = 3,
    CONNECT_FAILED  = 4,
    CONNECTION_LOST = 5,
    DISCONNECTED    = 6,
    AP_LISTENING    = 7,
    AP_CONNECTED    = 8,
    AP_FAILED       = 9,
};

static uint8_t spi_get_byte()
{
    uint8_t byte = 0xFF;
    spi_read_blocking(wifi_spi, 0xFF, &byte, 1);
    return byte;
}

static void nina_select()
{
    // TODO: timeout

    // wait ready
    while(gpio_get(WIFI_ESP32_NINA_BUSY));

    // select
    gpio_put(WIFI_ESP32_NINA_CS, false);

    // wait busy
    while(!gpio_get(WIFI_ESP32_NINA_BUSY));
}

static void nina_deselect()
{
    gpio_put(WIFI_ESP32_NINA_CS, true); // deselect
}

static void nina_command(NINACommand command, int num_params = 0, const uint8_t *param_lengths = nullptr, const uint8_t **param_data = nullptr)
{
    nina_select();

    // header
    uint8_t buf[]{0xE0, uint8_t(command), uint8_t(num_params)};
    spi_write_blocking(wifi_spi, buf, sizeof(buf));

    // params
    for(int i = 0; i < num_params; i++)
    {
        spi_write_blocking(wifi_spi, param_lengths + i, 1);
        spi_write_blocking(wifi_spi, param_data[i], param_lengths[i]);
    }

    // end
    buf[0] = 0xEE;
    spi_write_blocking(wifi_spi, buf, 1);

    nina_deselect();
}

static int nina_response(NINACommand command, int &num_responses, uint8_t *response_buf, unsigned response_len)
{
    nina_select();

    // wait for start byte
    while(true)
    {
        auto b = spi_get_byte();
        if(b == 0xE0)
            break;

        // error
        if(b == 0xEF)
            return -1;
    }

    // next byte should be command | 0x80
    auto reply = spi_get_byte();
    if(reply != (uint8_t(command) | 0x80))
        return -1;

    // get response count
    num_responses = spi_get_byte();

    auto out = response_buf;
    auto out_end = out + response_len;

    for(int i = 0; i < num_responses; i++)
    {
        int response_len = spi_get_byte();

        if(out < out_end)
            *out++ = response_len;

        // clamp read len to remaining buffer
        auto read_len = std::min(response_len, out_end - out);

        spi_read_blocking(wifi_spi, 0xFF, out, read_len);
        out += read_len;

        // drop the rest (is this needed?)
        for(int j = read_len; j < response_len; j++)
            spi_get_byte();
    }

    nina_deselect();

    return out - response_buf;
}

static int nina_response(NINACommand command, uint8_t *response_buf, unsigned response_len)
{
    int num_responses;
    return nina_response(command, num_responses, response_buf, response_len);
}

// simple one byte reply helper
static int nina_command_response(NINACommand command, uint8_t &res)
{
    nina_command(command);
    uint8_t tmp[2];
    if(nina_response(command, tmp, 2) != 2 || tmp[0] != 1)
        return -1;

    res = tmp[1]; // first byte is the length

    return 1;
}

void nina_spi_init()
{
    // we did the reset earlier (shared with audio on fruit jam)
    // TODO: other board support?

    sleep_ms(750); // wait for boot...

    spi_init(wifi_spi, 8000000);

    gpio_set_function(WIFI_ESP32_NINA_SCK , GPIO_FUNC_SPI);
    gpio_set_function(WIFI_ESP32_NINA_MOSI, GPIO_FUNC_SPI);
    gpio_set_function(WIFI_ESP32_NINA_MISO, GPIO_FUNC_SPI);

    gpio_init(WIFI_ESP32_NINA_BUSY);
}

NINAConnStatus nina_get_status()
{
    uint8_t status;
    if(nina_command_response(GET_CONN_STATUS, status) != 1)
        return NO_MODULE;

    return static_cast<NINAConnStatus>(status);
}

bool nina_connect_timeout(const char *ssid, const char *pass, uint32_t timeout_ms)
{
    // set ssid/password
    uint8_t len[]
    {
        uint8_t(strlen(ssid)),
        uint8_t(strlen(pass))
    };
    const uint8_t *data[]
    {
        reinterpret_cast<const uint8_t *>(ssid),
        reinterpret_cast<const uint8_t *>(pass)
    };

    nina_command(SET_PASSPHRASE, 2, len, data);

    // check response
    uint8_t res[2];
    if(nina_response(SET_PASSPHRASE, res, 2) != 2 || res[1] != 1)
        return false;

    // wait for connected status
    auto timeout_time = make_timeout_time_ms(timeout_ms);

    NINAConnStatus status;

    while(!time_reached(timeout_time))
    {
        status = nina_get_status();

        if(status == CONNECTED)
            return true;

        sleep_ms(50);
    }

    return false;
}
#endif