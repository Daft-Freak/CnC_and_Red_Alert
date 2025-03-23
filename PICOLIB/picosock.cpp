#include "picosock.h"

uint32_t inet_addr(const char *str)
{
    return 0;
}

hostent *gethostbyname(const char *name)
{
    return NULL;
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