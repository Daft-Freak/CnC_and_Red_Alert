#include <algorithm>

#include <SDL.h>

#include "drawbuff.h"
#include "gbuffer.h"

LPDIRECTDRAW DirectDrawObject;
LPDIRECTDRAWPALETTE	PalettePtr;

void *MainWindow;

GraphicViewPortClass *LogicPage;
bool AllowHardwareBlitFills = true;
bool OverlappedVideoBlits = false;

inline int Make_Code(int x, int y, int w, int h)
{
    return (x < 0 ? 0b1000 : 0) | (x >= w ? 0b0100 : 0) | (y < 0 ? 0b0010 : 0) | (y >= h ? 0b0001 : 0);
}

void Buffer_Put_Pixel(void *thisptr, int x, int y, unsigned char color)
{
    auto vp_dst = (GraphicViewPortClass *)thisptr;

    if(x < 0 || y < 0 || x >= vp_dst->Get_Width() || y >= vp_dst->Get_Height())
        return;

    int dst_area = vp_dst->Get_XAdd() + vp_dst->Get_Width() + vp_dst->Get_Pitch();
    auto dst_offset = vp_dst->Get_Offset() + x + y * dst_area;

    *dst_offset = color;
}

int	Buffer_Get_Pixel(void * thisptr, int x, int y)
{
    printf("%s\n", __PRETTY_FUNCTION__);
    return 0;
}

void Buffer_Clear(void *thisptr, unsigned char color)
{
    printf("%s\n", __PRETTY_FUNCTION__);
}

long Buffer_To_Buffer(void *thisptr, int x_pixel, int y_pixel, int pixel_width, int pixel_height, void *buff, long size)
{
    auto vp_src = (GraphicViewPortClass *)thisptr;

    int dst_x0 = 0;
    int dst_y0 = 0;

    // clip src
    int src_x0 = x_pixel;
    int src_y0 = y_pixel;
    int src_x1 = x_pixel + pixel_width;
    int src_y1 = y_pixel + pixel_height;

    int code0 = Make_Code(src_x0, src_y0, vp_src->Get_Width(), vp_src->Get_Height());
    int code1 = Make_Code(src_x1, src_y1, vp_src->Get_Width() + 1, vp_src->Get_Height() + 1);

    // outside
    if(code0 & code1)
        return 0; // i'm not sure this actually has a return value...

    if(code0 | code1)
    {
        // apply clip
        if(code0 & 0b1000)
        {
            dst_x0 -= src_x0;
            src_x0 = 0;
        }
        if(code0 & 0b0100)
            src_x1 = vp_src->Get_Width();
        if(code0 & 0b0010)
        {
            dst_x0 -= src_x0;
            src_x0 = 0;
        }
        if(code0 & 0b0100)
            src_y1 = vp_src->Get_Height();
    }

    int src_area = vp_src->Get_XAdd() + vp_src->Get_Width() + vp_src->Get_Pitch();
    auto src_offset = vp_src->Get_Offset() + src_x0 + src_y0 * src_area;

    auto dst_offset = (uint8_t *)buff + dst_x0 + dst_y0 * pixel_width;

    if(src_x1 <= src_x0 || src_y1 <= src_y0)
        return true;
    
    if(src_offset == dst_offset)
        return true;

    int pixel_count = src_x1 - src_x0;
    int line_count = src_y1 - src_y0;

    // copy lines
    do
    {
        memcpy(dst_offset, src_offset, pixel_count);
        src_offset += src_area;
        dst_offset += pixel_width;
    }
    while(--line_count);

    return 0;
}

long Buffer_To_Page(int dx_pixel, int dy_pixel, int pixel_width, int pixel_height, void *Buffer, void *view)
{
    auto vp_dst = (GraphicViewPortClass *)view;

    int src_x0 = 0;
    int src_y0 = 0;

    // clip dest
    int dst_x0 = dx_pixel;
    int dst_y0 = dy_pixel;
    int dst_x1 = dx_pixel + pixel_width;
    int dst_y1 = dy_pixel + pixel_height;

    int code0 = Make_Code(dst_x0, dst_y0, vp_dst->Get_Width(), vp_dst->Get_Height());
    int code1 = Make_Code(dst_x1, dst_y1, vp_dst->Get_Width() + 1, vp_dst->Get_Height() + 1);

    // outside
    if(code0 & code1)
        return 0; // i'm not sure this actually has a return value...

    if(code0 | code1)
    {
        // apply clip
        if(code0 & 0b1000)
        {
            src_x0 -= dst_x0;
            dst_x0 = 0;
        }
        if(code0 & 0b0100)
            dst_x1 = vp_dst->Get_Width();
        if(code0 & 0b0010)
        {
            src_x0 -= dst_x0;
            dst_x0 = 0;
        }
        if(code0 & 0b0100)
            dst_y1 = vp_dst->Get_Height();
    }

    auto src_offset = (uint8_t *)Buffer + src_x0 + src_y0 * pixel_width;

    int dst_area = vp_dst->Get_XAdd() + vp_dst->Get_Width() + vp_dst->Get_Pitch();
    auto dst_offset = vp_dst->Get_Offset() + dst_x0 + dst_y0 * dst_area;

    if(dst_x1 <= dst_x0 || dst_y1 <= dst_y0)
        return true;
    
    if(src_offset == dst_offset)
        return true;

    int pixel_count = dst_x1 - dst_x0;
    int line_count = dst_y1 - dst_y0;

    // copy lines
    do
    {
        memcpy(dst_offset, src_offset, pixel_count);
        src_offset += pixel_width;
        dst_offset += dst_area;
    }
    while(--line_count);

    return 0;
}

bool Linear_Blit_To_Linear(void *thisptr, void * dest, int x_pixel, int y_pixel, int dx_pixel,
    int dy_pixel, int pixel_width, int pixel_height, bool trans)
{
    printf("%s\n", __PRETTY_FUNCTION__);
    return true;
}

bool Linear_Scale_To_Linear(void *, void *, int, int, int, int, int, int, int, int, bool, char *)
{
    printf("%s\n", __PRETTY_FUNCTION__);
    return true;
}

long Buffer_Print(void *thisptr, const char *str, int x, int y, int fcolor, int bcolor)
{
    printf("%s\n", __PRETTY_FUNCTION__);
    return 0;
}

void Buffer_Draw_Line(void *thisptr, int sx, int sy, int dx, int dy, unsigned char color)
{
    auto vp_dst = (GraphicViewPortClass *)thisptr;

    int width = vp_dst->Get_Width();
    int height = vp_dst->Get_Height();

    // this is different to the original asm, but reused from blits
    int clip_var = Make_Code(sx, sy, width, height) | Make_Code(dx, dy, width, height);

    if(clip_var)
    {
        if(clip_var & 0b1000) // left
        {
            sy = sy + (-sx * (dy - sy)) / (dx - sx);
            sx = 0;
        }
        else if(clip_var & 0b0100) // right
        {
            sy = sy + (((width - 1) - sx) * (dy - sy)) / (dx - sx);
            sx = width - 1;
        }

        if(clip_var & 0b0010) // top
        {
            sx = sx + (-sy * (dx - sx)) / (dy - sy);
            sy = 0;
        }
        else if(clip_var & 0b0001) // bottom
        {
            sx = sx + (((height - 1) - sy) * (dx - sx)) / (dy - sy);
            sy = height - 1;
        }
    }

    int bpr = vp_dst->Get_XAdd() + vp_dst->Get_Width() + vp_dst->Get_Pitch();

    int y_dist = dy - sy;

    if(y_dist == 0)
    {
        // horizontal
        if(dx < sx)
            std::swap(dx, sx);

        int count = (dx - sx) + 1;
        auto ptr = vp_dst->Get_Offset() + sx + bpr * sy;
        if(count < 16)
        {
            for(; count != 0; count--)
                *ptr++ = color;
        }
        else
        {
            // align
            while(((uintptr_t)ptr & 3) != 0)
            {
                *ptr++ = color;
                count--;
            }

            // 32-bit fill
            uint32_t color32 = color | color << 8 | color << 16 | color << 24;
            auto ptr32 = (uint32_t *)ptr; 
            for(int count32 = count >> 2; count32 != 0; count32--)
                *ptr32++ = color32;

            ptr = (uint8_t *)ptr32;

            // draw remainder
            for(count = count & 3; count != 0; count--)
                *ptr++ = color;
        }

        return;
    }

    // not horizontal
    if(y_dist == 0 || dy < sy)
    {
        sy = sy + y_dist;
        y_dist = -y_dist;
        
        std::swap(dx, sx);
    }

    auto ptr = vp_dst->Get_Offset() + sx + bpr * sy;

    int step = 1;
    int x_dist = dx - sx;

    if(x_dist == 0)
    {
        // vertical
        int count = y_dist + 1;
        do
        {
            *ptr = color;
            ptr = ptr + bpr;
        }
        while(--count);
        return;
    }

    // not vertical
    if(x_dist == 0 || dx < sx)
    {
        y_dist = -y_dist;
        step = -1;
    }

    if(x_dist < y_dist)
    {
        int count = y_dist;
        int accum = y_dist >> 1;
        while(true)
        {
            *ptr = color;
            if(--count == 0) break;
            ptr += bpr;

            accum -= x_dist;
            if(accum < 0)
            {
                accum += y_dist;
                ptr += step;
            }
        }
    }
    else
    {
        int count = x_dist;
        int accum = x_dist >> 1;
        while(true)
        {
            *ptr = color;
            if(--count == 0) break;
            ptr = ptr + step;
        
            accum -= y_dist;
            if(accum < 0)
            {
                accum += x_dist;
                ptr += bpr;
            }
        }
    }
}

void Buffer_Fill_Rect(void *thisptr, int sx, int sy, int dx, int dy, unsigned char color)
{
    auto vp_dst = (GraphicViewPortClass *)thisptr;

    if(sx > dx)
        std::swap(sx, dx);
    if(sy > dy)
        std::swap(sy, dy);

    // clamp to bounds
    if(sx < 0)
        sx = 0;
    if(sy < 0)
        sy = 0;

    if(dx > vp_dst->Get_Width())
        dx = vp_dst->Get_Width();
    if(dy > vp_dst->Get_Height())
        dy = vp_dst->Get_Height();

    // nothing to fill
    if(dx <= sx || dy <= sy)
        return;

    int dst_area = vp_dst->Get_XAdd() + vp_dst->Get_Width() + vp_dst->Get_Pitch();
    auto dst_offset = vp_dst->Get_Offset() + sx + sy * dst_area;

    int pixel_count = dx - sx;
    int line_count = dy - sy;

    // fill lines
    do
    {
        memset(dst_offset, color, pixel_count);
        dst_offset += dst_area;
    }
    while(--line_count);
}

void Buffer_Remap(void * thisptr, int sx, int sy, int width, int height, void *remap)
{
    printf("%s\n", __PRETTY_FUNCTION__);
}

void Buffer_Draw_Stamp_Clip(void const *thisptr, void const *icondata, int icon, int x_pixel, int y_pixel, void const *remap, int ,int,int,int)
{
    printf("%s\n", __PRETTY_FUNCTION__);
}

GraphicViewPortClass *Set_Logic_Page(GraphicViewPortClass *ptr)
{
    std::swap(LogicPage, ptr);
    return ptr;
}

GraphicViewPortClass *Set_Logic_Page(GraphicViewPortClass &ptr)
{
    return Set_Logic_Page(&ptr);
}

GraphicViewPortClass::GraphicViewPortClass(GraphicBufferClass* graphic_buff, int x, int y, int w, int h)
{
    Attach(graphic_buff, x, y, w, h);
}

GraphicViewPortClass::GraphicViewPortClass()
{

}

GraphicViewPortClass::~GraphicViewPortClass()
{

}

void GraphicViewPortClass::Draw_Rect(int sx, int sy, int dx, int dy, unsigned char color)
{
    printf("%s\n", __PRETTY_FUNCTION__);
}

void GraphicViewPortClass::Attach(GraphicBufferClass *graphic_buff, int x, int y, int w, int h)
{
	if (this == Get_Graphic_Buffer()) 
		return;

    // clamp bounds
	if (x < 0)
		x = 0;
	if (x >= graphic_buff->Get_Width())
		x = graphic_buff->Get_Width() - 1;
	if (y < 0)
		y = 0;
	if (y >= graphic_buff->Get_Height()) 
		y = graphic_buff->Get_Height() - 1;

	if (x + w > graphic_buff->Get_Width()) 
		w = graphic_buff->Get_Width() - x;

	if (y + h > graphic_buff->Get_Height())
		h = graphic_buff->Get_Height() - y;

	/*======================================================================*/
	/* Get a pointer to the top left edge of the buffer.					*/
	/*======================================================================*/
 	Offset 		= graphic_buff->Get_Offset() + ((graphic_buff->Get_Width() + graphic_buff->Get_Pitch()) * y) + x;

	/*======================================================================*/
	/* Copy over all of the variables that we need to store.						*/
	/*======================================================================*/
 	XPos			= x;
 	YPos			= y;
 	XAdd			= graphic_buff->Get_Width() - w;
 	Width			= w;
 	Height		    = h;
	Pitch			= graphic_buff->Get_Pitch();
 	GraphicBuff     = graphic_buff;
}


GraphicBufferClass::GraphicBufferClass(int w, int h, void *buffer, long size) : GraphicBufferClass()
{
    Init(w, h, buffer, size, GBC_NONE);
}

GraphicBufferClass::GraphicBufferClass(int w, int h, void *buffer) : GraphicBufferClass(w, h, buffer, w * h)
{
}

GraphicBufferClass::GraphicBufferClass(void)
{
    GraphicBuff = this;
}

GraphicBufferClass::~GraphicBufferClass()
{
    Un_Init();
}

void GraphicBufferClass::Init(int w, int h, void *buffer, long size, GBC_Enum flags)
{
    Size = size;
    Width = w;
    Height = h;
    Pitch = 0;
    XAdd = 0;
    XPos = YPos = 0;

    if(flags & GBC_VISIBLE) {
        WindowSurface = SDL_GetWindowSurface((SDL_Window *)MainWindow);
        PaletteSurface = SDL_CreateRGBSurface(0, Width, Height, 8, 0, 0, 0, 0);
    } else {
        // regular allocation
        Allocated = !buffer;
        Buffer = buffer;

        if(!Buffer) {
            if(!Size)
                Size = w * h;
            Buffer = new uint8_t[Size];
        }

        Offset = (uint8_t *)Buffer;
    }
}

void GraphicBufferClass::Un_Init(void)
{
    // de-alloc surface
}

void GraphicBufferClass::Attach_DD_Surface (GraphicBufferClass * attach_buffer)
{
    printf("%s\n", __PRETTY_FUNCTION__);
}

bool GraphicBufferClass::Lock(void)
{
    if(!WindowSurface)
        return true;

    if(!LockCount)
    {
        SDL_LockSurface((SDL_Surface *)PaletteSurface);
        Offset = (uint8_t *)((SDL_Surface *)PaletteSurface)->pixels;
    }

    LockCount++;
    return true;
}

bool GraphicBufferClass::Unlock(void)
{
    if(!WindowSurface || !LockCount)
        return true;

    LockCount--;

    if(!LockCount)
    {
        SDL_UnlockSurface((SDL_Surface *)PaletteSurface);
        Offset = NULL;
    }

    return true;
}

void GraphicBufferClass::Update_Window_Surface()
{
    // convert from paletted...
    SDL_BlitSurface((SDL_Surface *)PaletteSurface, NULL, (SDL_Surface *)WindowSurface, NULL);
    SDL_UpdateWindowSurface((SDL_Window*)MainWindow);

    // update the event loop here too for now
    SDL_Event_Loop();
}
