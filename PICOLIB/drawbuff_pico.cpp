#include "gbuffer.h"


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
}

void GraphicBufferClass::Update_Palette(uint8_t *palette)
{
    Update_Window_Surface(false);
}

const void *GraphicBufferClass::Get_Palette() const
{
    return NULL;
}

void GraphicBufferClass::Init_Display_Surface()
{
}