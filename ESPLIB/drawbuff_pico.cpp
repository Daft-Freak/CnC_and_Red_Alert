#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "gbuffer.h"
#include "timer.h"

#include "display.h"

bool GraphicBufferClass::Lock(void)
{
    if(!LockCount)
    {
        /*if(Offset == get_framebuffer())
        {
            // this is possibly a bit specific to the DBI driver but
            // "render needed" means that the last frame transfer has finished
            // so if render is NOT needed it means we're still reading the framebuffer
            while(!display_render_needed()) __wfe();
        }*/
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
        //while(!display_render_needed()) __wfe();
        update_display(Get_Time_Ms());
        
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