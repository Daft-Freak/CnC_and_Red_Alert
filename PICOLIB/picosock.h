#pragma once
#include <stdint.h>
#include <sys/types.h>

#define htons __builtin_bswap16
#define ntohs __builtin_bswap16
#define htonl __builtin_bswap32
#define ntohl __builtin_bswap32

typedef int socklen_t;
typedef uint8_t sa_family_t;

// addresses
#define INADDR_ANY 0

#pragma pack(push, 1)
struct in_addr
{
    uint32_t s_addr;
};
struct sockaddr
{
    sa_family_t sa_family;
    char sa_data[];
};
struct sockaddr_in
{
    sa_family_t sin_family;
    uint16_t sin_port;
    in_addr sin_addr;
};
struct sockaddr_storage
{
    sa_family_t ss_family;
    char data[6];
};
#pragma pack(pop)

struct hostent
{
    char **h_addr_list;
};

uint32_t inet_addr(const char *str);

#define gethostname(name, size) name[0] = 0;

hostent *gethostbyname(const char *name);

// sockets
#define AF_INET 2

#define SOCK_DGRAM 2

int socket(int domain, int type, int protocol);
int bind(int fd, sockaddr *addr, socklen_t len);
int closesocket(int fd);

ssize_t recvfrom(int fd, void *buf, size_t n, int flags, sockaddr *addr, socklen_t *addr_len);
ssize_t sendto(int fd, const void *buf, size_t n, int flags, const sockaddr *addr, socklen_t addr_len);

// options
#define SOL_SOCKET 1

#define SO_ERROR     4
#define SO_BROADCAST 6
#define SO_SNDBUF    7
#define SO_RCVBUF    8
#define SO_LINGER   13

struct linger
{
    int l_onoff;
    int l_linger;
};

int getsockopt(int fd, int level, int optname, void *optval, socklen_t *optlen);
int setsockopt(int fd, int level, int optname, const void *optval, socklen_t optlen);

#define F_GETFL 3
#define F_SETFL 4

#define O_NONBLOCK 04000

// simplified
int fcntl(int fd, int cmd, int data);
