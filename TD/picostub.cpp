// more portable replacements for winstub

#include "function.h"

#include "ww_win.h"

bool ReadyToQuit = 0;

void Focus_Loss(void);
void Focus_Restore(void);

void VQA_ResumeAudio(void);

void CCDebugString (char const *string)
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
	Set_Palette(GamePalette);
	while (Get_Mouse_State()) {Show_Mouse();};
	CCMessageBox().Process("Error - out of memory.", "Abort");

	exit(0);
}

#define WINDOW_NAME "Command & Conquer"

void Create_Main_Window(HANDLE instance, int command_show, int width, int height)
{
	GameInFocus = true; // there is no other state
}

// CRC.ASM in WIN32LIB
// (re-implemented in RA)
extern "C" long Calculate_CRC(void *buffer, long length)
{
	if(!length)
		return 0;

	uint32_t crc = 0;
	auto ptr32 = (uint32_t *)buffer;

	auto len32 = length >> 2;
	do
	{
		crc = (crc << 1 | crc >> 31) + *ptr32++;
	}
	while(--len32);

	int remainder = length & 3;
	if(remainder)
	{
		auto ptr8 = (uint8_t *)ptr32;
		uint32_t tmp = 0;
		int i;
		for(i = 0; i < remainder; i++)
			tmp = tmp >> 8 | *ptr8++ << 24;
		for(; i < 4; i++)
			tmp = tmp >> 8 | tmp << 24;

		crc = (crc << 1 | crc >> 31) + tmp;
	}

	return crc;
}

// SHAKESCR.ASM in WIN32LIB
// based on Shake_The_Screen in RA's conquer.cpp
void Shake_Screen(int shakes)
{
	shakes += shakes;

	Hide_Mouse();
	SeenBuff.Blit(HidPage);
	int oldyoff = 0;
	int newyoff = 0;
	while(shakes--) {
		int x = TickCount.Time();

		do {
			newyoff = Sim_Random_Pick(0,2) - 1;
		} while (newyoff == oldyoff);
		switch (newyoff) {
			case -1:
				HidPage.Blit(SeenBuff, 0,2, 0,0, 640,398);
				break;
			case 0:
				HidPage.Blit(SeenBuff);
				break;
			case 1:
				HidPage.Blit(SeenBuff, 0,0, 0,2, 640,398);
				break;
		}
#ifdef PORTABLE
		while (x == TickCount.Time()) Video_End_Frame();
#else
		while (x == TickCount);
#endif
	}

	HidPage.Blit(SeenBuff);
	Show_Mouse();
}