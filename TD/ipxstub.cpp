#include <cstdio>
#include <cstring>

#ifdef _WIN32
#include <winsock.h>
typedef int socklen_t;
#else
#include <unistd.h>
#include <netinet/in.h>
#include <sys/socket.h>
#endif

#include "ipx95.h"
#include "ipx.h"

#include "net_select.h"

extern long PlanetWestwoodPortNumber;

static int SocketFd = -1;
static char ReceiveBuffer[1024];

struct RecvBuffer
{
    struct sockaddr_in Addr;
    char Buffer[1024];
    int Len;
    RecvBuffer *Next;
};

static RecvBuffer *NextRecvBuffer = NULL, *LastRecvBuffer = NULL;

static void Socket_Event_Handler(int socket, SocketEvent event, void *data)
{
    switch(event)
    {
        case SOCKEV_READ:
        {
            // recieve packet
            struct sockaddr_in addr;
            socklen_t addr_len = sizeof(addr);
            int rc = recvfrom(SocketFd, ReceiveBuffer, sizeof(ReceiveBuffer), 0, (sockaddr *)&addr, &addr_len);
            if(rc <= 0)
                return;

            // TODO: filter out self?

            // add to list
            auto new_buf = new RecvBuffer;
            memcpy(&new_buf->Addr, &addr, sizeof(addr));
            memcpy(new_buf->Buffer, ReceiveBuffer, rc);
            new_buf->Len = rc;
            new_buf->Next = NULL;

            if(LastRecvBuffer) {
                LastRecvBuffer->Next = new_buf;
                LastRecvBuffer = new_buf;
            } else {
                NextRecvBuffer = LastRecvBuffer = new_buf;
            }

            break;
        }
    }
}

bool __stdcall IPX_Initialise(void)
{
#ifdef _WIN32
    WSADATA wsaData;

    if(WSAStartup(MAKEWORD(1, 1), &wsaData) != 0)
        return false;
#endif

    return true;
}

bool __stdcall IPX_Get_Outstanding_Buffer95(unsigned char *buffer)
{
    if(NextRecvBuffer)
    {
        auto header = (IPXHeaderType *)buffer;
        auto data = buffer + sizeof(IPXHeaderType);

        // fill in header
        memset(header, 0, sizeof(IPXHeaderType));

        header->Length = htons(NextRecvBuffer->Len + sizeof(IPXHeaderType));

        // store addr in the node field
        auto &addr = NextRecvBuffer->Addr;
        header->SourceNetworkNode[0] = addr.sin_addr.s_addr;
        header->SourceNetworkNode[1] = addr.sin_addr.s_addr >> 8;
        header->SourceNetworkNode[2] = addr.sin_addr.s_addr >> 16;
        header->SourceNetworkNode[3] = addr.sin_addr.s_addr >> 24;

        memcpy(data, NextRecvBuffer->Buffer, NextRecvBuffer->Len);

        // clean up
        auto old_buf = NextRecvBuffer;

        NextRecvBuffer = NextRecvBuffer->Next;
        if(!NextRecvBuffer)
            LastRecvBuffer = NULL;

        delete old_buf;

        return true;
    }
    return false;
}

void __stdcall IPX_Shut_Down95(void)
{

}

int __stdcall IPX_Send_Packet95(unsigned char *address, unsigned char *buf, int len, unsigned char *network, unsigned char *node)
{
    struct sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_port = htons(PlanetWestwoodPortNumber);
	addr.sin_addr.s_addr = node[0] | node[1] << 8 | node[2] << 16 | node[3] << 24;

    int ret = sendto(SocketFd, (const char *)buf, len, 0, (sockaddr *)&addr, sizeof(addr));

    return ret > 0 ? 1 : 0;
}

int __stdcall IPX_Broadcast_Packet95(unsigned char *buf, int len)
{
    struct sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_port = htons(PlanetWestwoodPortNumber);
	addr.sin_addr.s_addr = INADDR_BROADCAST;

    int ret = sendto(SocketFd, (const char *)buf, len, 0, (sockaddr *)&addr, sizeof(addr));

    return ret > 0 ? 1 : 0;
}

bool __stdcall IPX_Start_Listening95(void)
{
    return Socket_Register_Select(SocketFd, Socket_Event_Handler, NULL);
}

int __stdcall IPX_Open_Socket95(int socket)
{
    if(SocketFd != -1)
        return IPXERR_SOCKET_ERROR;

    // open UDP socket
    SocketFd = ::socket(AF_INET, SOCK_DGRAM, 0);

    if(SocketFd == -1)
        return IPXERR_SOCKET_ERROR;

    // bind to port
    struct sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_port = htons(PlanetWestwoodPortNumber);
	addr.sin_addr.s_addr = INADDR_ANY;

	if(bind(SocketFd, (sockaddr *)&addr, sizeof(addr)) == -1) {
		IPX_Close_Socket95(0);
		return IPXERR_SOCKET_ERROR;
	}

    // setup for broadcast
    int yes = 1;
	setsockopt(SocketFd, SOL_SOCKET, SO_BROADCAST, (char *)&yes, sizeof(int));

    return 0;
}

void __stdcall IPX_Close_Socket95(int socket)
{
    if(SocketFd != -1) {
        Socket_Unregister_Select(SocketFd);

#ifdef _WIN32
        closesocket(SocketFd);
#else
        close(SocketFd);
#endif
        SocketFd = -1;
    }
}

int __stdcall IPX_Get_Connection_Number95(void)
{
    printf("IPX_Get_Connection_Number\n");
    return 0;
}

int __stdcall IPX_Get_Local_Target95(unsigned char *dest_network, unsigned char *dest_node, unsigned short dest_socket, unsigned char *bridge_address)
{
    printf("IPX_Get_Local_Target\n");
    return 0;
}
