#include "gbuffer.h"

#include <SDL.h>

extern SDL_Renderer *SDLRenderer;

extern Uint32 ForceRenderEventID;
static Uint32 Force_Redraw_Timer(Uint32 interval, void *)
{
    // something has been draw and not displayed for 33ms
    // go tell the main thread it should probably display that
    SDL_Event ev;
    ev.type = ForceRenderEventID;
    SDL_PushEvent(&ev);

    return 0;
}

bool GraphicBufferClass::Lock(void)
{
    if(!PaletteSurface)
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
    if(!PaletteSurface || !LockCount)
        return true;

    LockCount--;

    if(!LockCount)
    {
        SDL_UnlockSurface((SDL_Surface *)PaletteSurface);
        Offset = NULL;
        Update_Window_Surface(false);
    }

    return true;
}

void GraphicBufferClass::Update_Window_Surface(bool end_frame)
{
    auto window_tex = (SDL_Texture *)WindowTexture;

    if(!end_frame)
    {
        if(!RedrawTimer)
            RedrawTimer = SDL_AddTimer(1000/30, Force_Redraw_Timer, NULL);
        return;
    }

    bool update_tex = false;

    if(RedrawTimer)
    {
        SDL_RemoveTimer(RedrawTimer);
        RedrawTimer = 0;

        // if a timer was set, the surface was locked at some point
        // so probably modified
        update_tex = true;
    }

    // blit from paletted surface if needed
    if(update_tex)
    {
        SDL_Surface *tmp_surf;
        SDL_LockTextureToSurface(window_tex, NULL, &tmp_surf);
        SDL_BlitSurface((SDL_Surface *)PaletteSurface, NULL, tmp_surf, NULL);
        SDL_UnlockTexture(window_tex);
    }

    // copy to screen
    SDL_RenderClear(SDLRenderer);
    SDL_RenderCopy(SDLRenderer, window_tex, NULL, NULL);
    SDL_RenderPresent(SDLRenderer);

    // update the event loop here too for now
    SDL_Event_Loop();
}

void GraphicBufferClass::Update_Palette(uint8_t *palette)
{
    auto sdl_pal = ((SDL_Surface *)PaletteSurface)->format->palette;

    bool changed = false;

    for(int i = 0; i < sdl_pal->ncolors; i++)
    {
        // convert from 6-bit
        int new_r = palette[i * 3 + 0] << 2 | palette[i * 3 + 0] >> 4;
        int new_g = palette[i * 3 + 1] << 2 | palette[i * 3 + 1] >> 4;
        int new_b = palette[i * 3 + 2] << 2 | palette[i * 3 + 2] >> 4;

        changed = changed || sdl_pal->colors[i].r != new_r || sdl_pal->colors[i].g != new_g || sdl_pal->colors[i].b != new_b;

        sdl_pal->colors[i].r = new_r;
        sdl_pal->colors[i].g = new_g;
        sdl_pal->colors[i].b = new_b;
    }

    if(!changed)
        return;

    // make sure it gets updated
    SDL_SetPaletteColors(sdl_pal, sdl_pal->colors, 0, sdl_pal->ncolors);

    Update_Window_Surface(false);
}

const void *GraphicBufferClass::Get_Palette() const
{
    return ((SDL_Surface *)PaletteSurface)->format->palette;
}

void GraphicBufferClass::Init_Display_Surface()
{
    WindowTexture = SDL_CreateTexture(SDLRenderer, SDL_PIXELFORMAT_RGB888, SDL_TEXTUREACCESS_STREAMING, Width, Height);
    PaletteSurface = SDL_CreateRGBSurface(0, Width, Height, 8, 0, 0, 0, 0);
}