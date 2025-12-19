#include "ipx95.h"


bool __stdcall IPX_Initialise(void)
{
    return false;
}

bool __stdcall IPX_Get_Outstanding_Buffer95(unsigned char *buffer)
{
    return false;
}

void __stdcall IPX_Shut_Down95(void)
{

}

int __stdcall IPX_Send_Packet95(unsigned char *, unsigned char *, int, unsigned char*, unsigned char*)
{
    return 0;
}

int __stdcall IPX_Broadcast_Packet95(unsigned char *, int)
{
    return 0;
}

bool __stdcall IPX_Start_Listening95(void)
{
    return false;
}

int __stdcall IPX_Open_Socket95(int socket)
{
    return 0;
}

void __stdcall IPX_Close_Socket95(int socket)
{

}

int __stdcall IPX_Get_Connection_Number95(void)
{
    return 0;
}

int __stdcall IPX_Get_Local_Target95(unsigned char *, unsigned char*, unsigned short, unsigned char*)
{
    return 0;
}
