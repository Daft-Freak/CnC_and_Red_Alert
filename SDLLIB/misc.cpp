#include <stddef.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#include "misc.h"
#include "timer.h"
#include "ww_win.h"

SurfaceMonitorClass	AllSurfaces;

extern "C"
{
long RandNumb;
}

void (*Misc_Focus_Loss_Function)(void);
void (*Misc_Focus_Restore_Function)(void);

bool Set_Video_Mode(void *hwnd, int w, int h, int bits_per_pixel)
{
    printf("%s\n", __func__);
    return true;
}

void Wait_Blit(void)
{
    // nothing to wait for
}

void Delay(int duration)
{
    auto target = WindowsTimer->Get_System_Tick_Count() + duration;

	while(WindowsTimer->Get_System_Tick_Count() < target)
		Video_End_Frame();
}

void *Build_Fading_Table(void const *palette, void *dest, long int color, long int frac)
{
    unsigned matchvalue;
    uint8_t targetred;
    uint8_t targetgreen;
    uint8_t targetblue;
    uint8_t idealred;
    uint8_t idealgreen;
    uint8_t idealblue;
    uint8_t matchcolor;

    // If the source palette is NULL, then just return with current fading table pointer.
    if(!palette || !dest)
        return dest;

    // Fractions above 255 become 255.
    if(frac > 255)
        frac = 255;

    // Record the target gun values.
    auto pal8 = (uint8_t *)palette;
    targetred = pal8[color * 3 + 0];
    targetgreen = pal8[color * 3 + 0];
    targetblue = pal8[color * 3 + 0];

    // Main loop

    auto dptr = (uint8_t *)dest;

    // Transparent black never gets remapped.
    *dptr++ = 0;

    for(int remap_index = 1; remap_index < 256; remap_index++)
    {
        // new = orig - ((orig-target) * fraction);
        uint8_t origred = pal8[remap_index * 3 + 0];
        uint8_t origgreen = pal8[remap_index * 3 + 1];
        uint8_t origblue = pal8[remap_index * 3 + 2];

        uint16_t tmp = (origred - targetred) * (frac >> 1);
        idealred = origred - (tmp >> 7);

        tmp = (origgreen - targetgreen) * (frac >> 1);
        idealgreen = origgreen - (tmp >> 7);

        tmp = (origblue - targetblue) * (frac >> 1);
        idealblue = origblue - (tmp >> 7);

        // Sweep through the entire existing palette to find the closest
	    // matching color.  Never matches with color 0.

        matchcolor = color; // Default color (self).
        matchvalue = -1; // Ridiculous match value init.

        auto palptr = pal8 + 3;

        for(int color_index = 1; color_index < 256; color_index++)
        {
            if(color_index != remap_index)
            {
                int compval = 0;

                // Build the comparison value based on the sum of the differences of the color
                // guns squared
                int diff = palptr[0] - idealred;
                compval += diff * diff;
                diff = palptr[1] - idealgreen;
                compval += diff * diff;
                diff = palptr[2] - idealgreen;
                compval += diff * diff;

                if(compval == 0) // If perfect match found then quit early.
                {
                    matchcolor = color_index;
                    break;
                }

                if(compval < matchvalue)
                {
                    matchcolor = color_index;
                    matchvalue = compval;
                }
            }
            palptr += 3;
        }

        // When the loop exits, we have found the closest match.
        *dptr++ = matchcolor;
    }

    return dest;
}


int Confine_Rect(int *x, int *y, int dw, int dh, int width, int height)
{
    int ret = 0;

    if(*x < 0)
    {
        *x = 0;
        ret = 1;
    }
    else if(*x + dw > width)
    {
        *x -= (*x + dw) - width;

        if(*x < 0)
            *x = 0;

        ret = 1;
    }

    if(*y < 0)
    {
        *y = 0;
        ret = 1;
    }
    else if(*y + dh > height)
    {
        *y -= (*y + dh) - height;

        if(*y < 0)
            *y = 0;

        ret = 1;
    }

    return ret;
}

// these are used by TD
static int Get_Random_Mask(unsigned int maxval)
{
    // original asm did something using bsr but I can't be bothered
    maxval |= maxval >> 1;
    maxval |= maxval >> 2;
    maxval |= maxval >> 4;
    maxval |= maxval >> 8;
    maxval |= maxval >> 16;
    return maxval;
}

int IRandom(int minval, int maxval)
{
	int num,mask;

	// Keep minval and maxval straight.
	if (minval > maxval) {
		minval ^= maxval;
		maxval ^= minval;
		minval ^= maxval;
	}

	mask = Get_Random_Mask(maxval - minval);

	while( (num = (rand() & mask) + minval) > maxval ) ;
	return(num);
}

uint8_t Random()
{
    // mmm
    auto r = (uint8_t *)&RandNumb;

    uint8_t tmp = r[0] >> 1;
    int c = tmp & 1;
    tmp >>= 1;

    int c1 = r[2] & 0x80;
    r[2] = r[2] << 1 | c;

    int c2 = r[1] & 0x80;
    r[1] = r[1] << 1 | c1 >> 7;

    tmp = tmp - ((RandNumb & 0xFF) + (1 - c2));
    int c3 = tmp & 1;
    tmp >>= 1;

    r[0] = r[0] >> 1 | c3 << 7;

    return r[0] ^ r[1];
}

// from WIN32LIB/MISC/LIB.CPP
static unsigned Divide_With_Round(unsigned num, unsigned den)
{
	// return num/den + (0 ro 1).  1 if the remainder is more than half the denominator.
	return( (num / den) + (unsigned)((num % den) >= ((den + 1) >> 1)) );
}

#define HSV_BASE 255 // This is used to get a little better persion on HSV conversion.
#define RGB_BASE 63  // Not 64, this is really the max value.


void Convert_RGB_To_HSV(unsigned int r, unsigned int g, unsigned int b, unsigned int *h, unsigned int *s, unsigned int *v)
{
 	unsigned int m, r1, g1, b1, tmp;

	// Convert RGB base to HSV base.
	r = Divide_With_Round((r * HSV_BASE), RGB_BASE);
	g = Divide_With_Round((g * HSV_BASE), RGB_BASE);
	b = Divide_With_Round((b * HSV_BASE), RGB_BASE);

	// Set hue to default.
	*h = 0;

	// Set v = Max(r,g,b) to find dominant primary color.
	*v = (r > g) ? r : g;
	if (b > *v) *v = b;

	// Set m = min(r,g,b) to find amount of white.
	m = (r < g) ? r : g;
	if (b < m) m = b;

	// Determine the normalized saturation.
	if (*v != 0) {
		*s = Divide_With_Round( (*v - m) * HSV_BASE ,*v);
	} else {
		*s = 0;
	}

	if (*s != 0) {
		tmp = *v - m;
	 	r1 = Divide_With_Round( (*v - r) * HSV_BASE, tmp);
	 	g1 = Divide_With_Round( (*v - g) * HSV_BASE, tmp);
	 	b1 = Divide_With_Round( (*v - b) * HSV_BASE, tmp);

		// Find effect of second most predominant color.
		// In which section of the hexagon of colors does the color lie?
		if ((*v) == r) {
		 	if (m == g) {
				*h = 5 * HSV_BASE + b1;
			} else {
				*h = 1 * HSV_BASE - g1;
			}
		} else {
			if ((*v) == g) {
			 	if (m == b) {
					*h = 1 * HSV_BASE + r1;
				} else {
					*h = 3 * HSV_BASE - b1;
				}
			} else {
				// *v == b
			 	if (m == r) {
					*h = 3 * HSV_BASE + g1;
				} else {
					
					*h = 5 * HSV_BASE - r1;
				}
			}
		}

		// Divide by six and round.
		*h = Divide_With_Round(*h, 6);
	}
}

void Convert_HSV_To_RGB(unsigned int h, unsigned int s, unsigned int v, unsigned int *r, unsigned int *g, unsigned int *b)
{
	unsigned int i;				// Integer part.
	unsigned int f;				// Fractional or remainder part.  f/HSV_BASE gives fraction.
	unsigned int tmp;			// Tempary variable to help with calculations.
	unsigned int values[7];	// Possible rgb values.  Don't use zero.


	h *= 6;
	f = h % HSV_BASE;

	// Set up possible red, green and blue values.
	values[1] =	
	values[2] = v;

	//
	// The following lines of code change 
	//	values[3] = (v * (HSV_BASE - ( (s * f) / HSV_BASE) )) / HSV_BASE;
	//	values[4] = values[5] = (v * (HSV_BASE - s)) / HSV_BASE;
	// values[6] = (v * (HSV_BASE - (s * (HSV_BASE - f)) / HSV_BASE)) / HSV_BASE;
	// so that the are rounded divides.
	//

	tmp = Divide_With_Round(s * f, HSV_BASE);
	values[3] = Divide_With_Round(v * (HSV_BASE - tmp), HSV_BASE);

	values[4] = 
	values[5] = Divide_With_Round(v * (HSV_BASE - s), HSV_BASE);

	tmp = HSV_BASE - Divide_With_Round(s * (HSV_BASE - f),  HSV_BASE);
	values[6] = Divide_With_Round(v * tmp, HSV_BASE);


	// This should not be rounded.
	i = h / HSV_BASE;

	i += (i > 4) ? -4 : 2;
	*r = Divide_With_Round(values[i] * RGB_BASE, HSV_BASE);

	i += (i > 4) ? -4 : 2;
	*b = Divide_With_Round(values[i] * RGB_BASE, HSV_BASE);

	i += (i > 4) ? -4 : 2;
	*g = Divide_With_Round(values[i] * RGB_BASE, HSV_BASE);
} 