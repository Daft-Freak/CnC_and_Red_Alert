#include <stdio.h>

#include "tusb.h"

#include "ww_win.h"
#include "net_select.h"
#include "gbuffer.h"


unsigned int WinX;
unsigned int WinY;
unsigned int Window;

int Change_Window(int windnum)
{
    printf("%s\n", __func__);
    return 0;
}

void SDL_Event_Loop()
{

    // this is replacing WSAAsyncSelect, which would send through the windows event loop
    Socket_Select();

    tuh_task();
}

void SDL_Send_Quit()
{

}