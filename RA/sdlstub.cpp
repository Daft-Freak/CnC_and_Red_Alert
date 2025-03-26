// more portable replacements for winstub

#include "function.h"

#include "ww_win.h"

#undef WIN32
#include <SDL.h> // sdl includes leaking into the rest of the code is bad

void Focus_Loss(void);
void Focus_Restore(void);

void VQA_ResumeAudio(void);

void WWDebugString (char const *string)
{}

void Check_For_Focus_Loss(void)
{
	if(!GameInFocus)
	{
		SDL_Event_Loop();
		if(GameInFocus)
			VQA_ResumeAudio();
	}
}
void Memory_Error_Handler(void)
{
	VisiblePage.Clear();
	CCPalette.Set();
	while (Get_Mouse_State()) {Show_Mouse();};
	WWMessageBox().Process(TEXT_MEMORY_ERROR, TEXT_ABORT);

	ReadyToQuit = 1;

	do
	{
		Keyboard->Check();
	}while (ReadyToQuit == 1);

	exit(0);
}


#if (ENGLISH)
#define WINDOW_NAME		"Red Alert"
#endif

#if (FRENCH)
#define WINDOW_NAME		"Alerte Rouge"
#endif

#if (GERMAN)
#define WINDOW_NAME		"Alarmstufe Rot"
#endif

void Create_Main_Window(HANDLE instance, int command_show, int width, int height)
{
	SDL_Create_Main_Window(WINDOW_NAME, width, height);

	//Audio_Focus_Loss_Function = &Focus_Loss;
	Misc_Focus_Loss_Function = &Focus_Loss;
	Misc_Focus_Restore_Function = &Focus_Restore;
	//Gbuffer_Focus_Loss_Function = &Focus_Loss;
}

void SDL_Event_Handler(SDL_Event *event)
{
	if(Keyboard->Event_Handler(event))
		return;

	switch(event->type)
	{
		case SDL_WINDOWEVENT:
		{
			switch(event->window.event)
			{
				case SDL_WINDOWEVENT_FOCUS_GAINED:
					GameInFocus = true;
					Focus_Restore();
					break;
				case SDL_WINDOWEVENT_FOCUS_LOST:
					GameInFocus = false;
					Focus_Loss();
					break;
			}
			break;
		}
		case SDL_QUIT:
			Prog_End();
			VisiblePage.Un_Init();
			HiddenPage.Un_Init();

			exit(0);
			break;
	}
}
