// more portable replacements for winstub

#include "function.h"

#include "ww_win.h"

void Focus_Loss(void);
void Focus_Restore(void);

void VQA_ResumeAudio(void);

void WWDebugString (char const *string)
{
#ifndef NDEBUG
	printf("%s", string);
#endif
}

void Check_For_Focus_Loss(void)
{
	// not happening
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

void Create_Main_Window(HANDLE instance, int command_show, int width, int height)
{
	GameInFocus = true; // there is no other state
}

// event handler