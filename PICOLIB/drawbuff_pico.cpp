#include "gbuffer.h"
#include "timer.h"

#include "display.h"

bool GraphicBufferClass::Lock(void)
{
    if(!PaletteSurface)
        return true;

    if(!LockCount)
    {
        
    }

    LockCount++;
    return true;
}

bool GraphicBufferClass::Unlock(void)
{
    if(!PaletteSurface || !LockCount)
        return true;

    LockCount--;

    if(!LockCount)
    {
        
    }

    return true;
}

void GraphicBufferClass::Update_Window_Surface(bool end_frame)
{
    if(end_frame)
        update_display(Get_Time_Ms());
    // wait for sync?
}

void GraphicBufferClass::Update_Palette(uint8_t *palette)
{
    set_screen_palette(palette, 256);
    Update_Window_Surface(false);
}

const void *GraphicBufferClass::Get_Palette() const
{
    return NULL;
}

void GraphicBufferClass::Init_Display_Surface()
{
    Offset = get_framebuffer();
}