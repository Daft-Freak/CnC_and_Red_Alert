#include "ipx95.h"
#if defined(_MSC_VER) && (_MSC_VER >= 1200)
#define stdcall __stdcall
#else
#define stdcall
#endif

bool stdcall IPX_Initialise(void)
{
    return false;
}

bool stdcall IPX_Get_Outstanding_Buffer95(unsigned char *buffer)
{
    return false;
}

void stdcall IPX_Shut_Down95(void)
{

}

int stdcall IPX_Send_Packet95(unsigned char *, unsigned char *, int, unsigned char*, unsigned char*)
{
    return 0;
}

int stdcall IPX_Broadcast_Packet95(unsigned char *, int)
{
    return 0;
}

bool stdcall IPX_Start_Listening95(void)
{
    return false;
}

int stdcall IPX_Open_Socket95(int socket)
{
    return 0;
}

void stdcall IPX_Close_Socket95(int socket)
{

}

int stdcall IPX_Get_Connection_Number95(void)
{
    return 0;
}

int stdcall IPX_Get_Local_Target95(unsigned char *, unsigned char*, unsigned short, unsigned char*)
{
    return 0;
}
