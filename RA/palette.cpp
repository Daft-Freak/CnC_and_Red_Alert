#include <stdio.h>

#include "palette.h"
#include "function.h"

PaletteClass PaletteClass::CurrentPalette;

unsigned char *CurrentPalette = PaletteClass::CurrentPalette;

PaletteClass::PaletteClass()
{

}

PaletteClass::PaletteClass(const RGBClass &col)
{
    for(int i = 0; i < COLOR_COUNT; i++)
        data[i] = col;
}

void PaletteClass::Set(int fade, void (*callback)())
{
    if(fade)
    {
        // fade to new palette
        auto start_time = TickCount.Value();

        PaletteClass fade_palette;

        while(true)
        {
            int cur_time = TickCount.Value() - start_time;

            if(cur_time > fade)
                cur_time = fade;

            const unsigned char *old_ptr = CurrentPalette;
            const unsigned char *new_ptr = *this;
            unsigned char *out_ptr = fade_palette;

            for(int c = 0; c < COLOR_COUNT * 3; c++)
            {
                int new_val = *new_ptr++ & 0x3F;
                int old_val = *old_ptr++ & 0x3F;
                *out_ptr++ = old_val + (new_val - old_val) * cur_time / fade;
            }

            Do_Set_Palette(fade_palette);
            if(callback)
                callback();
#ifdef PORTABLE
            else // make sure we actually display the fade
                Video_End_Frame();
#endif

            if(cur_time == fade)
                break;
        }
    }

    CurrentPalette = *this;
    Do_Set_Palette(*this);
}

// the only code that uses these two (Play_Movie and OptionsClass::Proccess)
// only use it to adjust the black palette then immediately adjust it back
// presumably this is to force a palette update
void PaletteClass::Adjust(int)
{
}

void PaletteClass::Adjust(int, PaletteClass &)
{
}

void PaletteClass::Partial_Adjust(int, char *)
{
    printf("PaletteClass::%s\n", __func__);
}

void PaletteClass::Partial_Adjust(int, PaletteClass &, char *)
{
    printf("PaletteClass::%s\n", __func__);
}

int PaletteClass::Closest_Color(const RGBClass &col) const
{
    int index = -1;
    int diff = 256 * 3;

    for(int i = 0; i < COLOR_COUNT; i++)
    {
        int new_diff = std::abs(col.Red_Component() - data[i].Red_Component())
                     + std::abs(col.Green_Component() - data[i].Green_Component())
                     + std::abs(col.Blue_Component() - data[i].Blue_Component());

        if(new_diff == 0)
            return i;

        if(new_diff < diff)
        {
            index = i;
            diff = new_diff;
        }
    }

    return index;
}

RGBClass &PaletteClass::operator[](int index)
{
    return data[index];
}

const RGBClass &PaletteClass::operator[](int index) const
{
    return data[index];
}

PaletteClass::operator unsigned char *()
{
    return (unsigned char *)data;
}

PaletteClass::operator const unsigned char *() const
{
    return (const unsigned char *)data;
}

void Set_Palette(void *palette)
{
    memcpy(&PaletteClass::CurrentPalette, palette, PaletteClass::COLOR_COUNT * 3);
    Do_Set_Palette(palette);
}