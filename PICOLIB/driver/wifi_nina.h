#pragma once
#include <cstdint>

void nina_spi_init();

bool nina_connect_timeout(const char *ssid, const char *pass, uint32_t timeout_ms);

bool nina_get_ip_address(uint32_t &addr, uint32_t &mask, uint32_t &gateway);

// socket api
struct nina_sockaddr
{
    uint32_t addr;
    uint16_t port;
};

int nina_socket(int domain, int type, int protocol);
int nina_bind(int fd, uint16_t port); // doesn't take an address
int nina_close(int fd);

int32_t nina_sendto(int fd, const void *buf, uint16_t n, const nina_sockaddr *addr);
int32_t nina_recvfrom(int fd, void *buf, uint16_t n, nina_sockaddr *addr);

// this is a select on one fd with 0 timeout
int nina_poll(int fd);
