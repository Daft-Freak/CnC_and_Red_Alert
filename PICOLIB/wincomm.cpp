#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "pico/stdlib.h"
#include "hardware/uart.h"

#include "wincomm.h"
#include "modemreg.h"

WinModemClass *SerialPort = NULL;

// TODO: actually test size/placement
#define UART_BUFFFER_SIZE 2048
[[gnu::section(".psram_data")]] static uint8_t uart_buffer[UART_BUFFFER_SIZE];


#ifdef uart_default
static void on_uart_rx() {
    while(uart_is_readable(uart_default))
    {
        if(SerialPort)
            SerialPort->RX_Byte(uart_getc(uart_default));
    }
}
#endif

WinModemClass::WinModemClass(void) : SerialBuffer(uart_buffer), SerialBufferReadPtr(0), SerialBufferWritePtr(0)
{

}

WinModemClass::~WinModemClass(void)
{

}

HANDLE WinModemClass::Serial_Port_Open(char *device_name, int baud, int parity, int wordlen, int stopbits, int flowcontrol)
{
    if(strcmp(device_name, "Pico Default") != 0)
        return NULL;

#ifdef uart_default

    // disable stdio using uart
#if LIB_PICO_STDIO_UART
    stdio_uart_deinit();
#endif

    uart_init(uart_default, baud);

    // ignoring parity, always set to 0 anyway
    uart_set_format(uart_default, wordlen, stopbits, UART_PARITY_NONE);

    // ignoring for convenience
    // (the menu to turn it off is broken without EXPAND.MIX)
    //uart_set_hw_flow(uart_default, flowcontrol != 0, flowcontrol != 0);
    uart_set_hw_flow(uart_default, false, false);

    gpio_set_function(PICO_DEFAULT_UART_RX_PIN, UART_FUNCSEL_NUM(uart_default, PICO_DEFAULT_UART_RX_PIN));
    gpio_set_function(PICO_DEFAULT_UART_TX_PIN, UART_FUNCSEL_NUM(uart_default, PICO_DEFAULT_UART_TX_PIN));

    // setup rx irq
    int irq = uart_default == uart0 ? UART0_IRQ : UART1_IRQ;
    irq_set_exclusive_handler(irq, on_uart_rx);
    irq_set_enabled(irq, true);
    uart_set_irqs_enabled(uart_default, true, false);

    PortHandle = (void *)1;

    return PortHandle;
#endif

    return NULL;
}

void WinModemClass::Serial_Port_Close(void)
{
#ifdef uart_default
    if(PortHandle)
    {
        irq_set_enabled(uart_default == uart0 ? UART0_IRQ : UART1_IRQ, false);
        uart_deinit(uart_default);
#if LIB_PICO_STDIO_UART
        stdio_uart_init();
#endif
        PortHandle = NULL;
    }
#endif
}

int	WinModemClass::Read_From_Serial_Port(unsigned char *dest_ptr, int buffer_len)
{
#ifdef uart_default
    if(PortHandle)
    {
        int bytes_to_copy = SerialBufferWritePtr - SerialBufferReadPtr;
		if (bytes_to_copy <0 ) bytes_to_copy += UART_BUFFFER_SIZE;
		if (bytes_to_copy>buffer_len) bytes_to_copy = buffer_len;

        for(int i = 0; i < bytes_to_copy; i++)
        {
            *dest_ptr++ = SerialBuffer[SerialBufferReadPtr];

            SerialBufferReadPtr = (SerialBufferReadPtr + 1) & (UART_BUFFFER_SIZE - 1);
        }

        return bytes_to_copy;
    }
#endif
    return 0;
}

void WinModemClass::Write_To_Serial_Port(unsigned char *buffer, int length)
{
#ifdef uart_default
    if(PortHandle)
        uart_write_blocking(uart_default, buffer, length);
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

void WinModemClass::RX_Byte(uint8_t b)
{
    // check for space
    if((SerialBufferWritePtr + 1) & (UART_BUFFFER_SIZE - 1) == SerialBufferReadPtr)
        return;

    SerialBuffer[SerialBufferWritePtr] = b;

    SerialBufferWritePtr = (SerialBufferWritePtr + 1) & (UART_BUFFFER_SIZE - 1);
}

ModemRegistryEntryClass::ModemRegistryEntryClass(int modem_number) :
    ModemName(NULL), ModemDeviceName(NULL), ErrorCorrectionEnable(NULL), ErrorCorrectionDisable(NULL), CompressionEnable(NULL),
    CompressionDisable(NULL), HardwareFlowControl(NULL), NoFlowControl(NULL)
{
#ifdef uart_default
    if(modem_number == 0)
        ModemName = ModemDeviceName = "Pico Default";
#endif
}

ModemRegistryEntryClass::~ModemRegistryEntryClass()
{
}