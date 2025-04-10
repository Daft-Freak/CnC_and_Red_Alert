#include <cassert>
#include <chrono>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <map>

#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

#define RA_PORT 1234
#define RA_GLOBAL_MAGIC 0x1235
#define RA_PRODUCT_ID 0xAA00

#define RA_GLOBAL_PACKET_LEN 156

struct GlobalPacketHeader
{
    uint16_t magic;
    uint8_t code;
    uint32_t id;

    uint16_t product_id;
    uint16_t pad;

    uint16_t command;
};

class Address
{
public:
    Address(sockaddr_storage &sa) : storage(sa)
    {
        assert(sa.ss_family == AF_INET6);
    }

    bool operator<(const Address &other) const
    {
        auto this6 = (sockaddr_in6 *)&storage;
        auto other6 = (sockaddr_in6 *)&other.storage;

        return memcmp(&this6->sin6_addr, &other6->sin6_addr, sizeof(in6_addr)) < 0;
    }

    bool operator==(const Address &other) const
    {
        auto this6 = (sockaddr_in6 *)&storage;
        auto other6 = (sockaddr_in6 *)&other.storage;

        return memcmp(&this6->sin6_addr, &other6->sin6_addr, sizeof(in6_addr)) == 0;
    }

    sockaddr_storage storage;
};

class RemoteInfo
{
public:
    std::chrono::steady_clock::time_point last_seen;
};

static std::map<Address, RemoteInfo> remote_hosts;

static bool validate_packet(const uint8_t *data, ssize_t len)
{
    // some basic validation
    if(len != RA_GLOBAL_PACKET_LEN)
        return false;

    auto header = (GlobalPacketHeader *)data;

    if(header->magic != RA_GLOBAL_MAGIC)
        return false;

    if(header->product_id != RA_PRODUCT_ID)
        return false;

    return true;
}

int main(int argc, char *argv[])
{
    // the game really doesn't support IPv6 but let's do that anyway
    int sock_fd = socket(AF_INET6, SOCK_DGRAM, 0);

    if(sock_fd == -1)
        return 1;

    // set options
    int yes = 1;

    // allow reuse
    if(setsockopt(sock_fd, SOL_SOCKET, SO_REUSEADDR, (char *)&yes, sizeof(int)) == -1)
    {
        close(sock_fd);
        return 1;
    }

    // allow IPv4 connections
    yes = 0; // no
    if(setsockopt(sock_fd, IPPROTO_IPV6, IPV6_V6ONLY, (char *)&yes, sizeof(int)) == -1)
    {
        close(sock_fd);
        return 1;
    }

    // bind
    sockaddr_in6 addr = {0};
    addr.sin6_family = AF_INET6;
    addr.sin6_port = htons(RA_PORT);

    addr.sin6_addr = IN6ADDR_ANY_INIT;

    if(bind(sock_fd, (sockaddr *)&addr, sizeof(addr)) == -1)
    {
        close(sock_fd);
        return 1;
    }

    auto start_time = std::chrono::steady_clock::now();

    while(true)
    {
        uint8_t buf[1024];
        sockaddr_storage remote_addr;
        socklen_t remote_addr_len = sizeof(remote_addr);

        auto size = recvfrom(sock_fd, buf, sizeof(buf), 0, (sockaddr *)&remote_addr, &remote_addr_len);

        if(size > 0)
        {
            if(!validate_packet(buf, size))
                continue;

            auto recv_time = std::chrono::steady_clock::now();

            // get info
            char hoststr[NI_MAXHOST];
            char portstr[NI_MAXSERV];

            // update remote info
            remote_hosts[Address(remote_addr)].last_seen = recv_time;

            // loop through other remotes
            for(auto it = remote_hosts.begin(); it != remote_hosts.end();)
            {
                auto &other_remote = *it;
                if(other_remote.first == remote_addr)
                {
                    ++it;
                    continue;
                }

                // check for stale entries
                if(other_remote.second.last_seen - recv_time > std::chrono::minutes(10))
                {
                    it = remote_hosts.erase(it);
                    continue;
                }

                // forward packet
                // we should probably limit the number of query packets... (command 0)
                auto &forward_addr = other_remote.first.storage;
                socklen_t forward_addr_len = sizeof(sockaddr_storage);

                sendto(sock_fd, buf, size, 0, (sockaddr *)&forward_addr, forward_addr_len);

                ++it;
            }
        }
    }

    close(sock_fd);

    return 0;
}