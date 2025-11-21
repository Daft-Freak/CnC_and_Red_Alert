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

    // turns out we can get the raw socket API through these
    SOCKET_SOCKET        = 0x70,
    SOCKET_CLOSE         = 0x71,
    SOCKET_ERRNO         = 0x72,
    SOCKET_BIND          = 0x73,
    SOCKET_LISTEN        = 0x74,
    SOCKET_ACCEPT        = 0x75,
    SOCKET_CONNECT       = 0x76,
    SOCKET_SEND          = 0x77,
    SOCKET_RECV          = 0x78,
    SOCKET_SENDTO        = 0x79,
    SOCKET_RECVFROM      = 0x7A,
    SOCKET_IOCTL         = 0x7B,
    SOCKET_POLL          = 0x7C,
    SOCKET_SETSOCKOPT    = 0x7D,
    SOCKET_GETSOCKOPT    = 0x7E,
    SOCKET_GETPEERNAME   = 0x7F,
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

// 16-bit param lengths
static void nina_command(NINACommand command, int num_params, const uint16_t *param_lengths, const uint8_t **param_data)
{
    nina_select();

    // header
    uint8_t buf[]{0xE0, uint8_t(command), uint8_t(num_params)};
    spi_write_blocking(wifi_spi, buf, sizeof(buf));

    // params
    for(int i = 0; i < num_params; i++)
    {
        buf[0] = param_lengths[i] >> 8;
        buf[1] = param_lengths[i] & 0xFF;
        spi_write_blocking(wifi_spi, buf, 2);
        spi_write_blocking(wifi_spi, param_data[i], param_lengths[i]);
    }

    // end
    buf[0] = 0xEE;
    spi_write_blocking(wifi_spi, buf, 1);

    nina_deselect();
}

static int nina_response(NINACommand command, int &num_responses, uint8_t *response_buf, unsigned response_buf_len, bool response_len16 = false, uint8_t *response_buf2 = nullptr, unsigned response_buf2_len = 0)
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
    auto out_end = out + response_buf_len;

    for(int i = 0; i < num_responses; i++)
    {
        int response_len = spi_get_byte();

        // get the other length byte if 16bit (and store the first one)
        if(response_len16)
        {
            if(out < out_end)
                *out++ = response_len;

            response_len = response_len << 8 | spi_get_byte();
        }

        if(out < out_end)
            *out++ = response_len;

        // handle split response buffer (for recv)
        if(out == out_end && response_buf2)
        {
            out = response_buf2;
            out_end = out + response_buf2_len;
        }

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

bool nina_get_ip_address(uint32_t &addr, uint32_t &mask, uint32_t &gateway)
{
    nina_command(GET_IPADDR);
    uint8_t buf[15];
    int response_count;
    if(nina_response(GET_IPADDR, response_count, buf, 15) != 15 || response_count != 3)
        return false;

    addr = *reinterpret_cast<uint32_t *>(buf + 1);
    mask = *reinterpret_cast<uint32_t *>(buf + 6);
    gateway = *reinterpret_cast<uint32_t *>(buf + 11);

    return true;
}

int nina_socket(int domain, int type, int protocol)
{
    if(domain != 2/*AF_INET*/)
        return -1;

    uint8_t type8 = type;
    uint8_t protocol8 = protocol;

    uint8_t len[]{1, 1};
    const uint8_t *data[]{&type8, &protocol8};

    nina_command(SOCKET_SOCKET, 2, len, data);

    uint8_t res[2];
    if(nina_response(SOCKET_SOCKET, res, 2) != 2 || res[0] != 1)
        return -1;

    return res[1] == 0xFF ? -1 : res[1];
}

int nina_bind(int fd, uint16_t port)
{
    uint8_t fd8 = fd;
    uint8_t len[]{1, 2};
    const uint8_t *data[]{&fd8, reinterpret_cast<uint8_t *>(&port)};

    nina_command(SOCKET_BIND, 2, len, data);

    uint8_t res[2];
    if(nina_response(SOCKET_BIND, res, 2) != 2 || res[0] != 1)
        return -1;

    return res[1] == 0 ? -1 : 0;
}

int nina_close(int fd)
{
    uint8_t fd8 = fd;
    uint8_t len[]{1};
    const uint8_t *data[]{&fd8};

    nina_command(SOCKET_CLOSE, 1, len, data);

    uint8_t res[2];
    if(nina_response(SOCKET_CLOSE, res, 2) != 2 || res[0] != 1)
        return -1;

    return res[1] == 0 ? -1 : 0;
}

int32_t nina_sendto(int fd, const void *buf, uint16_t n, const nina_sockaddr *addr)
{
    uint8_t fd8 = fd;

    uint16_t len[]{1, 4, 2, n};
    const uint8_t *data[]{
        &fd8,
        reinterpret_cast<const uint8_t *>(&addr->addr),
        reinterpret_cast<const uint8_t *>(&addr->port),
        reinterpret_cast<const uint8_t *>(buf)
    };

    nina_command(SOCKET_SENDTO, 4, len, data);

    uint8_t res[3];
    if(nina_response(SOCKET_SENDTO, res, 3) != 3 || res[0] != 2)
        return -1;

    return res[1] << 8 | res[2];
}

int32_t nina_recvfrom(int fd, void *buf, uint16_t n, nina_sockaddr *addr)
{
    uint8_t fd8 = fd;
    uint8_t len[]{1, 2};
    const uint8_t *data[]{
        &fd8,
        reinterpret_cast<const uint8_t *>(&n)
    };

    nina_command(SOCKET_RECVFROM, 2, len, data);

    // addr+port and received len
    uint8_t res[2 + 4 + 2 + 2 + 2];
    int num_responses;
    if(nina_response(SOCKET_RECVFROM, num_responses, res, sizeof(res), true, reinterpret_cast<uint8_t *>(buf), n) < sizeof(res) || num_responses != 3)
        return -1;

    addr->addr = *reinterpret_cast<uint32_t *>(res + 2);
    addr->port = *reinterpret_cast<uint32_t *>(res + 8);

    return res[10] << 8 | res[11];
}

int nina_poll(int fd)
{
    uint8_t fd8 = fd;
    uint8_t len[]{1};
    const uint8_t *data[]{&fd8};

    nina_command(SOCKET_POLL, 1, len, data);

    uint8_t res[2];
    if(nina_response(SOCKET_POLL, res, 2) != 2 || res[0] != 1)
        return -1;

    return res[1];
}

#endif