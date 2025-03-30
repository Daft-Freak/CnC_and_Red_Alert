#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef LIBSERIALPORT
#include <libserialport.h>
#endif

#include "wincomm.h"
#include "modemreg.h"

WinModemClass *SerialPort = NULL;

WinModemClass::WinModemClass(void) : PortHandle(NULL)
{

}

WinModemClass::~WinModemClass(void)
{

}

HANDLE WinModemClass::Serial_Port_Open(char *device_name, int baud, int parity, int wordlen, int stopbits, int flowcontrol)
{
#ifdef LIBSERIALPORT
    sp_port *port;
    sp_return result = sp_get_port_by_name(device_name, &port);

    if(result != SP_OK)
        return NULL;

    // open
    result = sp_open(port, SP_MODE_READ_WRITE);
    if(result != SP_OK)
    {
        sp_free_port(port);
        return NULL;
    }

    // configure
    result = sp_set_baudrate(port, baud);
    if(result != SP_OK)
    {
        sp_close(port);
        sp_free_port(port);
        return NULL;
    }

    sp_parity libsp_parity = SP_PARITY_NONE;
    switch(parity)
    {
        case 'O': libsp_parity = SP_PARITY_ODD; break;
        case 'E': libsp_parity = SP_PARITY_EVEN; break;
        case 'N': libsp_parity = SP_PARITY_NONE; break;
        case 'S': libsp_parity = SP_PARITY_SPACE; break;
        case 'M': libsp_parity = SP_PARITY_MARK; break;
    }

    result = sp_set_parity(port, libsp_parity);
    if(result != SP_OK)
    {
        sp_close(port);
        sp_free_port(port);
        return NULL;
    }

    result = sp_set_bits(port, wordlen);
    if(result != SP_OK)
    {
        sp_close(port);
        sp_free_port(port);
        return NULL;
    }

    result = sp_set_stopbits(port, stopbits);
    if(result != SP_OK)
    {
        sp_close(port);
        sp_free_port(port);
        return NULL;
    }

    result = sp_set_flowcontrol(port, flowcontrol ? SP_FLOWCONTROL_RTSCTS : SP_FLOWCONTROL_NONE);
    if(result != SP_OK)
    {
        sp_close(port);
        sp_free_port(port);
        return NULL;
    }

    PortHandle = port;
    return PortHandle;
#else

    printf("WinModemClass::%s : no serial port support\n", __func__);
    return NULL;
#endif
}

void WinModemClass::Serial_Port_Close(void)
{
#ifdef LIBSERIALPORT
    if(PortHandle)
    {
        sp_port *port = (sp_port *)PortHandle;
        sp_close(port);
        sp_free_port(port);
        PortHandle = NULL;
    }
#endif
}

int	WinModemClass::Read_From_Serial_Port(unsigned char *dest_ptr, int buffer_len)
{
#ifdef LIBSERIALPORT
    if(PortHandle)
    {
        sp_return result = sp_nonblocking_read((sp_port *)PortHandle, dest_ptr, buffer_len);

        if(result > 0) {
            return result;
        }
    }
#endif
    return 0;
}

void WinModemClass::Write_To_Serial_Port(unsigned char *buffer, int length)
{
#ifdef LIBSERIALPORT
    if(PortHandle)
        sp_blocking_write((sp_port *)PortHandle, buffer, length, 0);
#endif
}

void WinModemClass::Set_Modem_Dial_Type(WinCommDialMethodType method)
{
    printf("WinModemClass::%s\n", __func__);
}

unsigned WinModemClass::Get_Modem_Status(void)
{
    printf("WinModemClass::%s\n", __func__);
    return 0;
}

void WinModemClass::Set_Serial_DTR(bool state)
{
    printf("WinModemClass::%s\n", __func__);
}

int WinModemClass::Get_Modem_Result(int delay, char *buffer, int buffer_len)
{
    printf("WinModemClass::%s\n", __func__);
    return 0;
}

void WinModemClass::Dial_Modem(char *dial_number)
{
    printf("WinModemClass::%s\n", __func__);
}

int WinModemClass::Send_Command_To_Modem(char *command, char terminator, char *buffer, int buflen, int delay, int retries)
{
    printf("WinModemClass::%s\n", __func__);
    return 0;
}

void WinModemClass::Set_Echo_Function(void(*func)(char c))
{
    printf("WinModemClass::%s\n", __func__);
}

void WinModemClass::Set_Abort_Function(int (*func)(void))
{
    printf("WinModemClass::%s\n", __func__);
}

HANDLE WinModemClass::Get_Port_Handle(void)
{
    return PortHandle;
}

ModemRegistryEntryClass::ModemRegistryEntryClass(int modem_number) :
    ModemName(NULL), ModemDeviceName(NULL), ErrorCorrectionEnable(NULL), ErrorCorrectionDisable(NULL), CompressionEnable(NULL),
    CompressionDisable(NULL), HardwareFlowControl(NULL), NoFlowControl(NULL)
{
#ifdef LIBSERIALPORT
    struct sp_port **port_list;

    sp_return result = sp_list_ports(&port_list);

    if(result == SP_OK)
    {
        for(int i = 0; port_list[i]; i++)
        {
            if(i == modem_number)
            {
                ModemName = ModemDeviceName = strdup(sp_get_port_name(port_list[i]));
                break;
            }
        }

        sp_free_port_list(port_list);
    }
#endif
}

ModemRegistryEntryClass::~ModemRegistryEntryClass()
{
    if(ModemName) free(ModemName);
}