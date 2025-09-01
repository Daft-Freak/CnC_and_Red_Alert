#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "gbuffer.h"
#include "timer.h"

#include "display.h"

bool GraphicBufferClass::Lock(void)
{
    if(!LockCount)
    {
        if(Offset == get_framebuffer())
            display_lock_framebuffer();
    }

    LockCount++;
    return true;
}

bool GraphicBufferClass::Unlock(void)
{
    if(!LockCount)
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
    {
        bool waited = false;
        while(!display_render_needed())
        {
            vTaskDelay(1);
            waited = true;
        }
        update_display(Get_Time_Ms());
        
        if(!waited)
            vTaskDelay(1); // let idle task run
    }
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