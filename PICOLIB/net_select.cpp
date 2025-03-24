#include <algorithm>
#include <stdio.h>
#include <forward_list>

#include "net_select.h"

bool socket_can_recv(int fd);
void socket_setup_recv(int fd, bool enable);

struct SocketInfo
{
    int socket;
    SocketCallback callback;
    void *data;
    bool check_write;
};

static std::forward_list<SocketInfo> Sockets;

static std::forward_list<SocketInfo>::iterator Find_Socket(int socket)
{
    return std::find_if(Sockets.begin(), Sockets.end(), [socket](SocketInfo &s) {return s.socket == socket;});
}

bool Socket_Register_Select(int socket, SocketCallback callback, void *data)
{
    if(!callback)
        return false;

    if(Find_Socket(socket) != Sockets.end())
        return false;

    // add to list
    SocketInfo info;
    info.socket = socket;
    info.callback = callback;
    info.data = data;
    info.check_write = false;
    Sockets.push_front(info);

    socket_setup_recv(socket, true);

    return true;
}

void Socket_Unregister_Select(int socket)
{
    Sockets.remove_if([socket](SocketInfo &s){return s.socket == socket;});

    socket_setup_recv(socket, false);
}

void Socket_Check_Write(int socket, bool check)
{
    auto it = Find_Socket(socket);
    if(it != Sockets.end())
        it->check_write = check;
}

void Socket_Select()
{
    for(auto &sock : Sockets)
    {
        if(socket_can_recv(sock.socket))
            sock.callback(sock.socket, SOCKEV_READ, sock.data);
        if(sock.check_write)
            sock.callback(sock.socket, SOCKEV_WRITE, sock.data);
    }
}