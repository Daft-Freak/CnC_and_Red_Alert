#include <algorithm>
#include <assert.h>
#include <string.h>

#include "drawbuff.h"
#include "font.h"
#include "gbuffer.h"
#include "misc.h"

void *MainWindow;

GraphicViewPortClass *LogicPage;
bool AllowHardwareBlitFills = true;
bool OverlappedVideoBlits = true;

GraphicBufferClass *WindowBuffer = NULL;

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
    auto vp_dst = (GraphicViewPortClass *)thisptr;

    if(x < 0 || y < 0 || x >= vp_dst->Get_Width() || y >= vp_dst->Get_Height())
        return 0;

    int dst_area = vp_dst->Get_XAdd() + vp_dst->Get_Width() + vp_dst->Get_Pitch();
    auto dst_offset = vp_dst->Get_Offset() + x + y * dst_area;

    return *dst_offset;
}

void Buffer_Clear(void *thisptr, unsigned char color)
{
    auto vp_dst = (GraphicViewPortClass *)thisptr;

    int sx = 0;
    int sy = 0;

    int dst_area = vp_dst->Get_XAdd() + vp_dst->Get_Width() + vp_dst->Get_Pitch();
    auto dst_offset = vp_dst->Get_Offset();

    int pixel_count = vp_dst->Get_Width();
    int line_count = vp_dst->Get_Height();

    // fill lines
    do
    {
        memset(dst_offset, color, pixel_count);
        dst_offset += dst_area;
    }
    while(--line_count);
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
        if(code1 & 0b0100)
            src_x1 = vp_src->Get_Width();
        if(code0 & 0b0010)
        {
            dst_y0 -= src_x0;
            src_y0 = 0;
        }
        if(code1 & 0b0001)
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
        if(code1 & 0b0100)
            dst_x1 = vp_dst->Get_Width();
        if(code0 & 0b0010)
        {
            src_y0 -= dst_x0;
            dst_y0 = 0;
        }
        if(code1 & 0b0001)
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
    // trans seems to only be used by TD

    auto vp_src = (GraphicViewPortClass *)thisptr;
    auto vp_dst = (GraphicViewPortClass *)dest;

    // clip source
    int src_x0 = x_pixel;
    int src_y0 = y_pixel;
    int src_x1 = x_pixel + pixel_width;
    int src_y1 = y_pixel + pixel_height;

    int code0 = Make_Code(src_x0, src_y0, vp_src->Get_Width(), vp_src->Get_Height());
    int code1 = Make_Code(src_x1, src_y1, vp_src->Get_Width() + 1, vp_src->Get_Height() + 1);

    // outside
    if(code0 & code1)
        return true;

    if(code0 | code1)
    {
        // apply clip
        if(code0 & 0b1000)
            src_x0 = 0;
        if(code1 & 0b0100)
            src_x1 = vp_src->Get_Width();
        if(code0 & 0b0010)
            src_y0 = 0;
        if(code1 & 0b0001)
            src_y1 = vp_src->Get_Height();
    }

    // clip dest
    int dst_x0 = dx_pixel;
    int dst_y0 = dy_pixel;
    int dst_x1 = dx_pixel + (src_x1 - src_x0);
    int dst_y1 = dy_pixel + (src_y1 - src_y0);

    code0 = Make_Code(dst_x0, dst_y0, vp_dst->Get_Width(), vp_dst->Get_Height());
    code1 = Make_Code(dst_x1, dst_y1, vp_dst->Get_Width() + 1, vp_dst->Get_Height() + 1);

    // outside
    if(code0 & code1)
        return true; // i'm not sure this actually has a return value...

    if(code0 | code1)
    {
        // apply clip
        if(code0 & 0b1000)
        {
            src_x0 -= dst_x0;
            dst_x0 = 0;
        }
        if(code1 & 0b0100)
        {
            src_x1 = (dst_x1 - vp_dst->Get_Width());
            dst_x1 = vp_dst->Get_Width();
        }
        if(code0 & 0b0010)
        {
            src_y0 -= dst_x0;
            dst_y0 = 0;
        }
        if(code1 & 0b0001)
        {
            src_y1 = (dst_x1 - vp_dst->Get_Height());
            dst_y1 = vp_dst->Get_Height();
        }
    }

    int src_area = vp_src->Get_XAdd() + vp_src->Get_Width() + vp_src->Get_Pitch();
    auto src_offset = vp_src->Get_Offset() + src_x0 + src_y0 * src_area;
    int src_adjust_width = src_area + src_x0 - src_x1;

    int dst_area = vp_dst->Get_XAdd() + vp_dst->Get_Width() + vp_dst->Get_Pitch();
    auto dst_offset = vp_dst->Get_Offset() + dst_x0 + dst_y0 * dst_area;
    int dst_adjust_width = dst_area + dst_x0 - dst_x1;

    if(dst_x1 <= dst_x0 || dst_y1 <= dst_y0)
        return true;
    
    if(src_offset == dst_offset)
        return true;

    int pixel_count = src_x1 - src_x0;
    int line_count = src_y1 - src_y0;

    if(src_offset < dst_offset)
    {
        // backward (bottom -> top)
        if(trans)
        {
            // copy transparent lines backwards
            src_offset += src_area * (line_count - 1);
            dst_offset += dst_area * (line_count - 1);
            do
            {
                for(int x = 0; x < pixel_count; x++)
                {
                    if(src_offset[x])
                        dst_offset[x] = src_offset[x];
                }
                src_offset -= src_area;
                dst_offset -= dst_area;
            }
            while(--line_count);
        }
        else
        {
            // copy lines backwards
            src_offset += src_area * (line_count - 1);
            dst_offset += dst_area * (line_count - 1);
            do
            {
                memmove(dst_offset, src_offset, pixel_count);
                src_offset -= src_area;
                dst_offset -= dst_area;
            }
            while(--line_count);
        }
    }
    else
    {
        // forward (top-> bottom)
        if(trans)
        {
            // copy transparent lines
            do
            {
                for(int x = 0; x < pixel_count; x++)
                {
                    if(src_offset[x])
                        dst_offset[x] = src_offset[x];
                }
                src_offset += src_area;
                dst_offset += dst_area;
            }
            while(--line_count);
        }
        else
        {
            // copy lines
            do
            {
                memmove(dst_offset, src_offset, pixel_count);
                src_offset += src_area;
                dst_offset += dst_area;
            }
            while(--line_count);
        }
    }

    return true;
}

bool Linear_Scale_To_Linear(void *thisptr, void *dest, int src_x, int src_y, int dst_x, int dst_y, int src_w, int src_h, int dst_w, int dst_h, bool trans, char *remap)
{
    // Check for scale error when to or from size 0,0
    if(dst_w == 0 || dst_h == 0 || src_w == 0 || src_h == 0)
        return true;

    auto vp_src = (GraphicViewPortClass *)thisptr;
    auto vp_dst = (GraphicViewPortClass *)dest;

    int src_x0 = src_x;
    int src_y0 = src_y;
    int src_x1 = src_x + src_w;
    int src_y1 = src_y + src_h;

    int dst_x0 = dst_x;
    int dst_y0 = dst_y;
    int dst_x1 = dst_x + dst_w;
    int dst_y1 = dst_y + dst_h;

    // clip source
    int code0 = Make_Code(src_x0, src_y0, vp_src->Get_Width(), vp_src->Get_Height());
    int code1 = Make_Code(src_x1, src_y1, vp_src->Get_Width() + 1, vp_src->Get_Height() + 1);

    // outside
    if(code0 & code1)
        return true;

    if(code0 | code1)
    {
        // apply clip
        if(code0 & 0b1000)
        {
            src_x0 = 0;
            dst_x0 = dst_x + (src_x0 - src_x) * dst_w / src_w;
        }
        if(code1 & 0b0100)
        {
            src_x1 = vp_src->Get_Width();
            dst_x1 = dst_x + (src_x1 - src_x) * dst_w / src_w;
        }
        if(code0 & 0b0010)
        {
            src_y0 = 0;
            dst_y0 = dst_y + (src_y0 - src_y) * dst_h / src_h;
        }
        if(code1 & 0b0001)
        {
            src_y1 = vp_src->Get_Height();
            dst_y1 = dst_y + (src_y1 - src_y) * dst_h / src_h;
        }
    }

    // clip dest
    code0 = Make_Code(dst_x0, dst_y0, vp_dst->Get_Width(), vp_dst->Get_Height());
    code1 = Make_Code(dst_x1, dst_y1, vp_dst->Get_Width() + 1, vp_dst->Get_Height() + 1);

    // outside
    if(code0 & code1)
        return true;

    if(code0 | code1)
    {
        // apply clip
        if(code0 & 0b1000)
        {
            dst_x0 = 0;
            src_x0 = src_x + (dst_x0 - dst_x) * src_w / dst_w;
        }
        if(code1 & 0b0100)
        {
            dst_x1 = vp_dst->Get_Width();
            src_x1 = src_x + (dst_x1 - dst_x) * src_w / dst_w;
        }
        if(code0 & 0b0010)
        {
            src_y0 -= dst_x0;
            src_y0 = src_y + (dst_y0 - dst_y) * src_h / dst_h;
        }
        if(code1 & 0b0001)
        {
            dst_y1 = vp_dst->Get_Height();
            src_y1 = src_y + (dst_y1 - dst_y) * src_h / dst_h;
        }
    }

    // do scale
    int src_win_width = vp_src->Get_XAdd() + vp_src->Get_Width() + vp_src->Get_Pitch();
    auto src_offset = vp_src->Get_Offset() + src_x0 + src_y0 * src_win_width;

    int dst_win_width = vp_dst->Get_XAdd() + vp_dst->Get_Width() + vp_dst->Get_Pitch();
    auto dst_offset = vp_dst->Get_Offset() + dst_x0 + dst_y0 * dst_win_width;

    int dy_intr = src_h / dst_h * src_win_width;
    int dy_frac = src_h % dst_h;
    int dy_acc = -dst_h;

    int dx_frac = (src_w << 16) / dst_w;

    if(dst_x1 <= dst_x0 || dst_y1 <= dst_y0)
        return true;

    int counter_y = dst_y1 - dst_y0;
    int pixel_count = dst_x1 - dst_x0;

    if(trans && remap)
    {
        do
        {
            int counter_x = pixel_count;
            int x = 0;
            auto out = dst_offset;
            do
            {
                uint8_t pixel = src_offset[x >> 16];

                if(pixel)
                    *out = remap[pixel];

                x += dx_frac;
                out++;
            }
            while (--counter_x);
            
            src_offset += dy_intr;
            dst_offset += dst_win_width;

            dy_acc += dy_frac;
            if(dy_acc > 0)
            {
                src_offset += src_win_width;
                dy_acc -= dst_h;
            }
        }
        while(--counter_y);
    }
    else if(trans)
    {
        // normal scale with transparency
        do
        {
            int counter_x = pixel_count;
            int x = 0;
            auto out = dst_offset;
            do
            {
                uint8_t pixel = src_offset[x >> 16];

                if(pixel)
                    *out = pixel;

                x += dx_frac;
                out++;
            }
            while (--counter_x);
            
            src_offset += dy_intr;
            dst_offset += dst_win_width;

            dy_acc += dy_frac;
            if(dy_acc > 0)
            {
                src_offset += src_win_width;
                dy_acc -= dst_h;
            }
        }
        while(--counter_y);
    }
    else if(remap)
    {
        // normal scale with remap
        do
        {
            int counter_x = pixel_count;
            int x = 0;
            auto out = dst_offset;
            do
            {
                *out++ = remap[src_offset[x >> 16]];

                x += dx_frac;
                out++;
            }
            while (--counter_x);
            
            src_offset += dy_intr;
            dst_offset += dst_win_width;

            dy_acc += dy_frac;
            if(dy_acc > 0)
            {
                src_offset += src_win_width;
                dy_acc -= dst_h;
            }
        }
        while(--counter_y);
    }
    else
    {
        // normal scale
        do
        {
            int counter_x = pixel_count;
            int x = 0;
            auto out = dst_offset;
            do
            {
                *out++ = src_offset[x >> 16];

                x += dx_frac;
                out++;
            }
            while (--counter_x);
            
            src_offset += dy_intr;
            dst_offset += dst_win_width;

            dy_acc += dy_frac;
            if(dy_acc > 0)
            {
                src_offset += src_win_width;
                dy_acc -= dst_h;
            }
        }
        while(--counter_y);
    }

    return true;
}

long Buffer_Print(void *thisptr, const char *str, int x, int y, int fcolor, int bcolor)
{
    auto vp_dst = (GraphicViewPortClass *)thisptr;

    if(!str)
        return 0;

    int start_x = x;
    auto font = (uint8_t *)FontPtr; // this could use a struct

    int vpheight = vp_dst->Get_Height();
    int vpwidth = vp_dst->Get_Width();
    int bufferwidth = vpwidth + vp_dst->Get_XAdd() + vp_dst->Get_Pitch();
    auto curline = vp_dst->Get_Offset() + bufferwidth * y;

    if (!FontPtr)
        return 0;

    uint16_t infoblock_off = *((uint16_t *)(font + FONTINFOBLOCK));
    uint16_t offsetblock_off = *((uint16_t *)(font + FONTOFFSETBLOCK));
    uint16_t widthblock_off = *((uint16_t *)(font + FONTWIDTHBLOCK));
    uint16_t heightblock_off = *((uint16_t *)(font + FONTHEIGHTBLOCK));

    uint8_t *infoblock = font + infoblock_off;
    uint16_t *offsetblock = (uint16_t *)(font + offsetblock_off);
    uint8_t *widthblock = font + widthblock_off;
    uint16_t *heightblock = (uint16_t *)(font + heightblock_off);

    uint8_t maxheight = infoblock[FONTINFOMAXHEIGHT];
    y = maxheight + y;
    if(y > vpheight)
        return 0;

    // setup colours
    ColorXlat[1] = fcolor;
    ColorXlat[16] = fcolor;
    ColorXlat[0] = bcolor;
    
    auto next_ptr = curline + x;

    do
    {
        auto startdraw = next_ptr;

        char ch = *str++;
        if(!ch)
            return (long)startdraw;

        if(ch == '\n' || ch == '\r' || x + widthblock[ch] + FontXSpacing > vpwidth)
        {
            // newline
            int line_height = maxheight + FontYSpacing;

            // check bounds
            if (vpheight < y + line_height) break;

            curline += bufferwidth * line_height;
            y = y + line_height;
            x = 0;
            if (ch != '\n')
                x = start_x;
            next_ptr = curline + x;

            if(ch == '\n' || ch == '\r')
                continue;
        }

        int char_w = widthblock[ch];
        x += char_w + FontXSpacing;
        next_ptr = startdraw + FontXSpacing + char_w;

        int nextdraw = bufferwidth - char_w;
        int char_offset = offsetblock[ch];
        int height_val = heightblock[ch];
        int charheight = height_val >> 8;
        int topblank = height_val & 0xFF;
        int bottomblank = maxheight - (charheight + topblank);

        // top blank area
        if(topblank != 0)
        {
            if(ColorXlat[0] == 0) // transparent
                startdraw = startdraw + topblank * bufferwidth;
            else
            {
                do
                {
                    int x_count = char_w;
                    do
                    {
                        *startdraw++ = ColorXlat[0];
                    }
                    while (--x_count);

                    startdraw += nextdraw;
                }
                while(--topblank);
            }
        }

        // draw char
        int y_count = charheight;
        auto char_data = (font + char_offset);
        if(y_count != 0)
        {
            do
            {
                int x_count = char_w;
                do
                {
                    uint8_t b = *char_data++;
                    int col = ColorXlat[b & 0xF];
                    if(col != 0)
                        *startdraw = col;
                    
                    startdraw++;
                    x_count--;

                    if(x_count)
                    {
                        int col = ColorXlat[b & 0xF0];
                        if(col != 0)
                            *startdraw = col;

                        startdraw++;
                        x_count--;
                    }
                }
                while(x_count);

                startdraw = startdraw + nextdraw;
            }
            while (--y_count);

            if(bottomblank && (ColorXlat[0] != 0))
            {
                do
                {
                    int x_count = char_w;
                    do
                    {
                        *startdraw++ = ColorXlat[0];
                    }
                    while (--x_count);

                    startdraw += nextdraw;
                }
                while(--bottomblank);
            }
        }
    }
    while(true);

    return 0;
}

void Buffer_Draw_Line(void *thisptr, int sx, int sy, int dx, int dy, unsigned char color)
{
    auto vp_dst = (GraphicViewPortClass *)thisptr;

    int width = vp_dst->Get_Width();
    int height = vp_dst->Get_Height();

    // this is different to the original asm, but reused from blits
    int code0 = Make_Code(sx, sy, width, height);
    int code1 = Make_Code(dx, dy, width, height);

    if(code0 & code1)
        return; 

    if(code0)
    {
        if(code0 & 0b1000) // left
        {
            sy = sy + (-sx * (dy - sy)) / (dx - sx);
            sx = 0;
        }
        else if(code0 & 0b0100) // right
        {
            sy = sy + (((width - 1) - sx) * (dy - sy)) / (dx - sx);
            sx = width - 1;
        }

        if(code0 & 0b0010) // top
        {
            sx = sx + (-sy * (dx - sx)) / (dy - sy);
            sy = 0;
        }
        else if(code0 & 0b0001) // bottom
        {
            sx = sx + (((height - 1) - sy) * (dx - sx)) / (dy - sy);
            sy = height - 1;
        }
    }

    if(code1)
    {
        if(code1 & 0b1000) // left
        {
            dy = dy + (-dx * (sy - dy)) / (sx - dx);
            dx = 0;
        }
        else if(code1 & 0b0100) // right
        {
            dy = dy + (((width - 1) - dx) * (sy - dy)) / (sx - dx);
            dx = width - 1;
        }

        if(code1 & 0b0010) // top
        {
            dx = dx + (-dy * (sx - dx)) / (sy - dy);
            dy = 0;
        }
        else if(code1 & 0b0001) // bottom
        {
            dx = dx + (((height - 1) - dy) * (sx - dx)) / (sy - dy);
            dy = height - 1;
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
        x_dist = -x_dist;
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

    if(dx >= vp_dst->Get_Width())
        dx = vp_dst->Get_Width() - 1;
    if(dy >= vp_dst->Get_Height())
        dy = vp_dst->Get_Height() - 1;

    // nothing to fill
    if(dx < sx || dy < sy)
        return;

    int dst_area = vp_dst->Get_XAdd() + vp_dst->Get_Width() + vp_dst->Get_Pitch();
    auto dst_offset = vp_dst->Get_Offset() + sx + sy * dst_area;

    int pixel_count = (dx - sx) + 1;
    int line_count = (dy - sy) + 1;

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
    if(!remap)
        return;

    auto vp_dst = (GraphicViewPortClass *)thisptr;

    // clip
    int dst_x0 = sx;
    int dst_y0 = sy;
    int dst_x1 = sx + width;
    int dst_y1 = sy + height;

    int code0 = Make_Code(dst_x0, dst_y0, vp_dst->Get_Width(), vp_dst->Get_Height());
    int code1 = Make_Code(dst_x1, dst_y1, vp_dst->Get_Width() + 1, vp_dst->Get_Height() + 1);

    // outside
    if(code0 & code1)
        return;

    if(code0 | code1)
    {
        // apply clip
        if(code0 & 0b1000)
            dst_x0 = 0;
        if(code1 & 0b0100)
            dst_x1 = vp_dst->Get_Width();
        if(code0 & 0b0010)
            dst_y0 = 0;
        if(code1 & 0b0001)
            dst_y1 = vp_dst->Get_Height();
    }

    int dst_area = vp_dst->Get_XAdd() + vp_dst->Get_Width() + vp_dst->Get_Pitch();
    auto dst_offset = vp_dst->Get_Offset() + dst_x0 + dst_y0 * dst_area;

    if(dst_x1 <= dst_x0 || dst_y1 <= dst_y0)
        return;

    int pixel_count = dst_x1 - dst_x0;
    int line_count = dst_y1 - dst_y0;

    int skip = dst_area - pixel_count;

    // remap lines
    do
    {
        for(int x = 0; x < pixel_count; x++)
        {
            auto v = ((uint8_t *)remap)[*dst_offset];
            *dst_offset++ = v;
        }
        dst_offset += skip;
    }
    while(--line_count);
}

// from misc.h, implemented here to share clipping helpers
int Clip_Rect(int *x, int *y, int *dw, int *dh, int width, int height)
{
    int x0 = *x;
    int y0 = *y;
    int x1 = *x + *dw;
    int y1 = *y + *dh;

    int code0 = Make_Code(x0, y0, width, height);
    int code1 = Make_Code(x1, y1, width + 1, height + 1);

    // outside
    if(code0 & code1)
        return -1;

    if(code0 | code1)
    {
        // apply clip
        if(code0 & 0b1000)
            x0 = 0;
        if(code1 & 0b0100)
            x1 = width;
        if(code0 & 0b0010)
            y0 = 0;
        if(code1 & 0b0001)
            y1 = height;

        *x = x0;
        *y = y0;
        *dw = x1 - x0;
        *dh = y1 - y0;
        return 1;
    }
    
    return 0;
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

GraphicViewPortClass::GraphicViewPortClass(GraphicBufferClass* graphic_buff, int x, int y, int w, int h) : GraphicBuff(NULL)
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
	Lock();
	Draw_Line(sx, sy, dx, sy, color);
	Draw_Line(sx, dy, dx, dy, color);
	Draw_Line(sx, sy, sx, dy, color);
	Draw_Line(dx, sy, dx, dy, color);
	Unlock();
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
        Init_Display_Surface();

        WindowBuffer = this;
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


void Video_End_Frame()
{
    if(WindowBuffer)
        WindowBuffer->Update_Window_Surface(true);
}