#include <stdio.h>

#include "gbuffer.h"
#include "mouse.h"

void Do_Set_Palette(void *palette)
{
    if(WindowBuffer)
        WindowBuffer->Update_Palette((uint8_t *)palette);

    Update_Mouse_Palette();
}