#include <stdio.h>

#ifdef WIFI_ENABLED
#include "pico/cyw43_arch.h"
#include "lwip/pbuf.h"
#include "lwip/udp.h"

#include "picosock.h"

static udp_pcb *udp = NULL;
static const int socket_fd = 5;

static pbuf *last_recv_buf = NULL;
static ip_addr_t last_recv_addr;
static uint16_t last_recv_port;

uint32_t inet_addr(const char *str)
{
    return ipaddr_addr(str);
}

hostent *gethostbyname(const char *name)
{
    static uint32_t *addr_list[2]{NULL, NULL};
    static uint32_t addr_data = 0;
    static hostent he;
    he.h_addr_list = (char **)addr_list;

    if(!name[0])
    {
        // get self
        auto addr = netif_ip4_addr(netif_list);
        addr_data = addr->addr;
        addr_list[0] = &addr_data;
    }
    else
        addr_list[0] = NULL;

    return &he;
}

int socket(int domain, int type, int protocol)
{
    if(domain != AF_INET || type != SOCK_DGRAM)
        return -1;

    if(udp)
        return -1;

    cyw43_arch_lwip_begin();
    udp = udp_new();
    cyw43_arch_lwip_end();

    if(!udp)
        return -1;
    
    return socket_fd;
}

int bind(int fd, sockaddr *addr, socklen_t len)
{
    if(fd != socket_fd)
        return -1;

    if(!addr || len < sizeof(sockaddr_in))
        return -1;

    if(addr->sa_family != AF_INET)
        return -1;

    auto sin_addr = (sockaddr_in *)addr;

    ip_addr_t ip_addr;
    ip_addr.addr = sin_addr->sin_addr.s_addr;
    uint16_t port = __builtin_bswap16(sin_addr->sin_port);

    cyw43_arch_lwip_begin();

    int res = udp_bind(udp, &ip_addr, port);

    cyw43_arch_lwip_end();

    if(res == ERR_OK)
        return 0;

    return -1;
}

int closesocket(int fd)
{
    if(udp && fd == socket_fd)
    {
        cyw43_arch_lwip_begin();
        udp_remove(udp);
        cyw43_arch_lwip_end();
        udp = NULL;
        return 0;
    }

    return -1;
}

ssize_t recvfrom(int fd, void *buf, size_t n, int flags, sockaddr *addr, socklen_t *addr_len)
{
    if(fd != socket_fd)
        return -1;

    if(!last_recv_buf)
        return -1;

    ssize_t ret = pbuf_copy_partial(last_recv_buf, buf, n, 0);

    if(addr)
    {
        auto sin_addr = (sockaddr_in *)addr;
        sin_addr->sin_family = AF_INET;
        sin_addr->sin_addr.s_addr = last_recv_addr.addr;
        sin_addr->sin_port = __builtin_bswap16(last_recv_port);
    }

    // assuming we read the whole thing
    cyw43_arch_lwip_begin();
    pbuf_free(last_recv_buf);
    last_recv_buf = NULL;
    cyw43_arch_lwip_end();

    return ret;
}

ssize_t sendto(int fd, const void *buf, size_t n, int flags, const sockaddr *addr, socklen_t addr_len)
{
    if(fd != socket_fd)
        return -1;

    cyw43_arch_lwip_begin();

    // alloc buf
    struct pbuf *p = pbuf_alloc(PBUF_TRANSPORT, n, PBUF_RAM);
    if(!p)
    {
        cyw43_arch_lwip_end();
        return -1;
    }
    memcpy(p->payload, buf, n);

    // address
    auto sin_addr = (sockaddr_in *)addr;
    ip_addr_t ip_addr;
    ip_addr.addr = sin_addr->sin_addr.s_addr;
    uint16_t port = __builtin_bswap16(sin_addr->sin_port);

    // send it
    int res = udp_sendto(udp, p, &ip_addr, port);
    pbuf_free(p);

    cyw43_arch_lwip_end();

    return res == ERR_OK ? 0 : -1;
}

int getsockopt(int fd, int level, int optname, void *optval, socklen_t *optlen)
{
    printf("%s\n", __func__);
    return -1;
}
int setsockopt(int fd, int level, int optname, const void *optval, socklen_t optlen)
{
    if(fd != socket_fd)
        return -1;

    printf("%s(%i, %i, %i, %p, %i)\n", __func__, fd, level, optname, optval, optlen);
    if(optval && optlen == 4)
        printf("\t(%i)\n", *(int *)optval);
    return 0;
}

int fcntl(int fd, int cmd, int data)
{
    printf("%s(%i, %i, %i)\n", __func__, fd, cmd, data);
    return -1;
}

// helpers for the select wrapper
static void recv_callback(void *arg, struct udp_pcb *pcb, struct pbuf *p, const ip_addr_t *addr, u16_t port)
{
    if(last_recv_buf)
    {
        // last packet not handled yet , but this is UDP so uh, just pretend it got lost somewhere
        pbuf_free(p);
        return;
    }

    last_recv_buf = p;
    last_recv_addr = *addr;
    last_recv_port = port;
}

bool socket_can_recv(int fd)
{
    return fd == socket_fd && last_recv_buf != NULL;
}

void socket_setup_recv(int fd, bool enable)
{
    if(fd != socket_fd)
        return;

    cyw43_arch_lwip_begin();
    udp_recv(udp, enable ? recv_callback : NULL, NULL);
    cyw43_arch_lwip_end();
}

#else
#include "picosock.h"

uint32_t inet_addr(const char *str)
{
    return 0;
}

hostent *gethostbyname(const char *name)
{
    static hostent he;
    he.h_addr_list = NULL;
    return &he;
}

int socket(int domain, int type, int protocol)
{
    return -1;
}

int bind(int fd, sockaddr *addr, socklen_t len)
{
    return -1;
}

int closesocket(int fd)
{
    return -1;
}

ssize_t recvfrom(int fd, void *buf, size_t n, int flags, sockaddr *addr, socklen_t *addr_len)
{
    return -1;
}

ssize_t sendto(int fd, const void *buf, size_t n, int flags, const sockaddr *addr, socklen_t addr_len)
{
    return -1;
}

int getsockopt(int fd, int level, int optname, void *optval, socklen_t *optlen)
{
    return -1;
}

int setsockopt(int fd, int level, int optname, const void *optval, socklen_t optlen)
{
    return -1;
}

int fcntl(int fd, int cmd, int data)
{
    return -1;
}

bool socket_can_recv(int fd)
{
    return false;
}

void socket_setup_recv(int fd, bool enable)
{
}
#endif