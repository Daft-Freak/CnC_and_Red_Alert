#include <stdio.h>
#include <string.h>

#include "mem.h"

#include "picolib.h"

extern const uint8_t asset_tall_font[];


// draws a line of text in the middle of the screen at 2x scale
static void Draw_Basic_Text(const char * str)
{
    // font data
    auto font_data = asset_tall_font + 8 + asset_tall_font[4];
    int char_w = asset_tall_font[5];
    int char_h = asset_tall_font[6];
    int spacing_y = asset_tall_font[7];
    auto char_w_variable = asset_tall_font + 8;

    const int height_bytes = (char_h + 7) / 8;
    const int char_size = char_w * height_bytes;

    // framebuffer
    auto framebuffer = (uint8_t *)nullptr;//get_framebuffer();

    // find start point
    int cursor_x = 0, cursor_y = 100 - char_h;

    for(auto str_ptr = str; *str_ptr; str_ptr++)
    {
        uint8_t chr_idx = *str_ptr & 0x7F;
        chr_idx = chr_idx < ' ' ? 0 : chr_idx - ' ';

        cursor_x += char_w_variable[chr_idx];
    }

    cursor_x = 160 - cursor_x;

    // draw it
    for(auto str_ptr = str; *str_ptr; str_ptr++)
    {
        // draw character
        uint8_t chr_idx = *str_ptr & 0x7F;
        chr_idx = chr_idx < ' ' ? 0 : chr_idx - ' ';

        const uint8_t *font_chr = &font_data[chr_idx * char_size];

        int char_width = char_w_variable[chr_idx];

        for(uint8_t y = 0; y < char_h; y++)
        {
            if(cursor_y + y < 0)
                continue;
    
            auto out_ptr = framebuffer + cursor_x + (cursor_y + y * 2) * 320;
    
            for(uint8_t x = 0; x < char_w; x++)
            {
                int bit = 1 << (y & 7);
                if (font_chr[x * height_bytes + y / 8] & bit)
                    out_ptr[0] = out_ptr[1] = out_ptr[320] = out_ptr[321] = 1;
        
                out_ptr += 2;
            }
        }

        // increment the cursor
        cursor_x += char_width * 2;
    }
}

static void Display_Loading_Message()
{
    /*memset(get_framebuffer(), 0, 320 * 200);

    // minimal palette
    uint8_t col[]{0, 0, 0, 63, 63, 63};
    set_screen_palette(col, 2);

    Draw_Basic_Text("Please Stand By...");

    // display it
    update_display(to_ms_since_boot(get_absolute_time()));

    // wait for DBI's late backlight enabling
    while(!display_render_needed()) __wfe();
    update_display(to_ms_since_boot(get_absolute_time()));*/
}

void Pico_Init(const char *basedir)
{
    //pre_init_display();

    PSRAM_Alloc_Init();

    /*f_mount(&fs, "", 0);

    char buf[10];
    snprintf(buf, sizeof(buf), "/%s/", basedir);
    f_chdir(buf);*/

    Pico_Flash_Cache_Init();

    Pico_Input_Init();

    //init_display();
    //init_audio();

    Display_Loading_Message();
}

void Pico_Wifi_Init(const char *ssid, const char *pass)
{

}

// app_main -> main wrapper
int main(int argc, char * argv[]);

extern "C" void app_main()
{
    char *argv[]{"ra"};
    main(1, argv);
}