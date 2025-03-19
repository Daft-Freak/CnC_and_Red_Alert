#include <stdio.h>
#include <string.h>

#include "iconcach.h"
#include "gbuffer.h"

static void const *LastIconset = NULL;
static uint8_t *StampPtr = NULL;

static uint8_t *IsTrans = NULL;

static uint8_t *MapPtr = NULL;
static int IconWidth = 0;
static int IconHeight = 0;
static int IconSize = 0;
static int IconCount = 0;

typedef struct {
	short	Width;			// Width of icons (pixels).
	short	Height;			// Height of icons (pixels).
	short	Count;			// Number of (logical) icons in this set.
	short	Allocated;		// Was this iconset allocated?
	int32_t	Size;				// Size of entire iconset memory block.
	int32_t	Icons;			// Offset from buffer start to icon data.
	int32_t	Palettes;		// Offset from buffer start to palette data.
	int32_t	Remaps;			// Offset from buffer start to remap index data.
	int32_t	TransFlag;		// Offset for transparency flag table.
	int32_t	Map;				// Icon map offset (if present).
} IControl_Type_Old;

void Init_Stamps(void const *icon_ptr)
{
    // Verify legality of parameter.
    if(!icon_ptr)
        return;

    // Don't initialize if already initialized to this set (speed reasons).
    if(LastIconset == icon_ptr)
        return;

    LastIconset = icon_ptr;

    // Record number of icons in set.
    auto control = (IControl_Type *)icon_ptr;
    IconCount = control->Count;

    // Record width of icon.
    IconWidth = control->Width;

    // Record height of icon
    IconHeight = control->Height;

    // Record size of icon (in bytes)
    IconSize = IconWidth * IconHeight;

    // hack to detect old format
    // (these fields are actually Size in that case)
    if(!control->MapHeight || control->MapWidth > 256)
    {
        auto old = (IControl_Type_Old *)control;
        MapPtr = (uint8_t *)icon_ptr + old->Map;
        StampPtr = (uint8_t *)icon_ptr + old->Icons;
        IsTrans = (uint8_t *)icon_ptr + old->TransFlag;
    }
    else
    {
        // Record hard pointer to icon map data.
        MapPtr = (uint8_t *)icon_ptr + control->Map;

        // Record hard pointer to icon data
        StampPtr = (uint8_t *)icon_ptr + control->Icons;

        // Record the transparent table
        IsTrans = (uint8_t *)icon_ptr + control->TransFlag;
    }
}

void Buffer_Draw_Stamp_Clip(void const *thisptr, void const *icondata, int icon, int x_pixel, int y_pixel, void const *remap, int min_x, int min_y, int max_x, int max_y)
{
    if(!icondata)
        return;

    // Initialize the stamp data if necessary.
    if(icondata != LastIconset)
        Init_Stamps(icondata);

	// Determine if the icon number requested is actually in the set.
	// Perform the logical icon to actual icon number remap if necessary.
    if(MapPtr)
        icon = MapPtr[icon];

    if(icon >= IconCount)
        return;

    // Setup some working variables.
    int iwidth = IconWidth;
    int iheight = IconHeight;

    // Fetch pointer to start of icon's data.
    auto ptr = StampPtr + icon * IconSize;

    // Update the clipping window coordinates to be valid maxes instead of width & height
    // , and change the coordinates to be window-relative
    max_x += min_x;
    x_pixel += min_x;
    max_y += min_y;
    y_pixel += min_y;

    // See if the icon is within the clipping window
    // First, verify that the icon position is less than the maximums
    if(x_pixel >= max_x || y_pixel >= max_y)
        return;

    // Now verify that the icon position is >= the minimums
    if(x_pixel + IconWidth < min_x || y_pixel + IconHeight < min_y)
        return;

    // Now, clip the x, y, width, and height variables to be within the
    // clipping rectangle

    if(x_pixel < min_x)
    {
        ptr += (min_x - x_pixel);
        iwidth -= (min_x - x_pixel);
        x_pixel = min_x;
    }

    int skip = IconWidth - iwidth;

    if(x_pixel + iwidth > max_x)
    {
        int ow = iwidth;
        iwidth = max_x - x_pixel;
        skip += ow - iwidth;
    }

    if(y_pixel < min_y)
    {
        iheight -= (min_y - y_pixel);
        ptr += IconWidth * (min_y - y_pixel);
        y_pixel = min_y;
    }

    if(y_pixel + iheight > max_y)
        iheight = max_y - y_pixel;

    if(!iwidth || !iheight)
        return;

    // If the remap table pointer passed in is NULL, then flag this condition
    // so that the faster (non-remapping) icon draw loop will be used.
    bool doremap = remap != NULL;

    // Get pointer to position to render icon.
    auto vp_dst = (GraphicViewPortClass *)thisptr;
    int dst_area = vp_dst->Get_XAdd() + vp_dst->Get_Width() + vp_dst->Get_Pitch();
    auto dst_offset = vp_dst->Get_Offset() + x_pixel + y_pixel * dst_area;

    // Determine row modulo for advancing to next line.
    int modulo = vp_dst->Get_Width() - iwidth;

    if(doremap)
    {
        // Complex icon draw -- extended remap.
        do
        {
            for(int x = 0; x < iwidth; x++)
            {
                uint8_t pixel = ((uint8_t *)remap)[*ptr++];
                if(pixel)
                    *dst_offset = pixel;
                dst_offset++;
            }

            ptr += skip;
            dst_offset += modulo;
        }
        while (--iheight);
    }
    // Check to see if transparent or generic draw is necessary.
    else if(IsTrans[icon])
    {
        // Transparent icon draw routine -- no extended remap.
        do
        {
            for(int x = 0; x < iwidth; x++)
            {
                uint8_t pixel = *ptr++;
                if(pixel)
                    *dst_offset = pixel;
                dst_offset++;
            }

            ptr += skip;
            dst_offset += modulo;
        }
        while (--iheight);
    }
    else
    {
        // Fast non-transparent icon draw routine.
        do
        {
            memcpy(dst_offset, ptr, iwidth);
            dst_offset += dst_area;
            ptr += IconWidth;
        }
        while (--iheight);
    }
}

void Restore_Cached_Icons(void)
{
    printf("%s\n", __func__);
}

void Register_Icon_Set(void *icon_data, bool pre_cache)
{
}
