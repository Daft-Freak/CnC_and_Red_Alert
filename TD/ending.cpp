/*
**	Command & Conquer(tm)
**	Copyright 2025 Electronic Arts Inc.
**
**	This program is free software: you can redistribute it and/or modify
**	it under the terms of the GNU General Public License as published by
**	the Free Software Foundation, either version 3 of the License, or
**	(at your option) any later version.
**
**	This program is distributed in the hope that it will be useful,
**	but WITHOUT ANY WARRANTY; without even the implied warranty of
**	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**	GNU General Public License for more details.
**
**	You should have received a copy of the GNU General Public License
**	along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

/* $Header:   F:\projects\c&c\vcs\code\ending.cpv   1.5   16 Oct 1995 16:50:30   JOE_BOSTIC  $ */
/***********************************************************************************************
 ***              C O N F I D E N T I A L  ---  W E S T W O O D  S T U D I O S               ***
 ***********************************************************************************************
 *                                                                                             *
 *                 Project Name : Command & Conquer                                            *
 *                                                                                             *
 *                    File Name : ENDING.H                                                     *
 *                                                                                             *
 *                   Programmer : Barry W. Green                                               *
 *                                                                                             *
 *                   Start Date : July 10, 1995                                                *
 *                                                                                             *
 *                  Last Update : July 10, 1995 [BWG]                                          *
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include	"function.h"
#include	"textblit.h"

void GDI_Ending(void)
{
#ifdef DEMO
	Fade_Palette_To(BlackPalette, FADE_PALETTE_MEDIUM, Call_Back);
	Load_Title_Screen("DEMOPIC.PCX", &HidPage, Palette);
	HidPage.Blit(SeenBuff);
	Fade_Palette_To(Palette, FADE_PALETTE_MEDIUM, Call_Back);
	Clear_KeyBuffer();
	Get_Key_Num();
	Fade_Palette_To(BlackPalette, FADE_PALETTE_MEDIUM, Call_Back);
	VisiblePage.Clear();

#else
	if (TempleIoned) {
		Play_Movie("GDIFINB");
	} else {
		Play_Movie("GDIFINA");
	}

	Score.Presentation();

	if (TempleIoned) {
		Play_Movie("GDIEND2");
	} else {
		Play_Movie("GDIEND1");
	}

	CountDownTimerClass count;
	if (CCFileClass("TRAILER.VQA").Is_Available()) {
		Fade_Palette_To(BlackPalette, FADE_PALETTE_MEDIUM, Call_Back);
		CCFileClass f("ATTRACT2.CPS");
#ifdef LORES
		Load_Uncompress(f, HiddenPage, HiddenPage, Palette);
		HidPage.Blit(SeenBuff);
#else
		Load_Uncompress(f, SysMemPage, SysMemPage, Palette);
		SysMemPage.Scale(SeenBuff, 0, 0, 0, 0, 320, 199, 640, 398);
#endif
		Fade_Palette_To(Palette, FADE_PALETTE_MEDIUM, Call_Back);
		Clear_KeyBuffer();
		count.Set(TIMER_SECOND*3);
		while (count.Time()) {
			Call_Back();
		}
		Fade_Palette_To(BlackPalette, FADE_PALETTE_MEDIUM, Call_Back);

		Play_Movie("TRAILER");	// Red Alert teaser.
	}

	Fade_Palette_To(BlackPalette, FADE_PALETTE_MEDIUM, Call_Back);
	CCFileClass f("ATTRACT2.CPS");
#ifdef LORES
	Load_Uncompress(f, HiddenPage, HiddenPage, Palette);
	HidPage.Blit(SeenBuff);
#else
	Load_Uncompress(f, SysMemPage, SysMemPage, Palette);
	SysMemPage.Scale(SeenBuff, 0, 0, 0, 0, 320, 199, 640, 398);
#endif
	Fade_Palette_To(Palette, FADE_PALETTE_MEDIUM, Call_Back);
	Clear_KeyBuffer();
//	CountDownTimerClass count;
	count.Set(TIMER_SECOND*3);
	while (count.Time()) {
		Call_Back();
	}
	Fade_Palette_To(BlackPalette, FADE_PALETTE_MEDIUM, Call_Back);

	Play_Movie("CC2TEASE");
#endif
}


#ifndef DEMO
/***********************************************************************************************
 * Nod_Ending -- play ending movies for Nod players                                            *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   7/10/1995 BWG : Created.                                                                  *
 *=============================================================================================*/
void Nod_Ending(void)
{
	static unsigned char const _tanpal[]={0x0,0xED,0xED,0x2C,0x2C,0xFB,0xFB,0xFD,0xFD,0x0,0x0,0x0,0x0,0x0,0x52,0x0};

	char fname[12];
#ifdef NOT_FOR_WIN95
	char *satpic = new char[64000];
#endif	//NOT_FOR_WIN95
	int oldfontxspacing = FontXSpacing;
	void const *oldfont;

	Score.Presentation();

	oldfont = Set_Font(ScoreFontPtr);
#ifndef LORES
	PseudoSeenBuff = new GraphicBufferClass(320,200,(void*)NULL);
#endif
	TextPrintBuffer = new GraphicBufferClass(SeenBuff.Get_Width(), SeenBuff.Get_Height(), (void*)NULL);
	TextPrintBuffer->Clear();
	BlitList.Clear();
	SeenBuff.Clear();
	HidPage.Clear();
#ifndef LORES
	PseudoSeenBuff->Clear();
#endif

	CCFileClass f("SATSEL.PAL");
	void * localpal = Load_Alloc_Data(f);
	f.Open("SATSEL.CPS");
	Load_Uncompress(f, SysMemPage, SysMemPage, NULL);
#ifdef NOT_FOR_WIN95
	memcpy(satpic, HidPage.Get_Buffer(), 64000);
#elif !defined(LORES)
	SysMemPage.Blit(*PseudoSeenBuff);
#endif	//NOT_FOR_WIN95
	void *kanefinl = Load_Sample("KANEFINL.AUD");
	void *loopie6m = Load_Sample("LOOPIE6M.AUD");

	Play_Movie("NODFINAL", THEME_NONE, false);

	Hide_Mouse();
	Wait_Vert_Blank();
	Set_Palette(localpal);
#ifdef NOT_FOR_WIN95
	memcpy(SeenBuff.Get_Buffer(), satpic, 64000);
#endif	//NOT_FOR_WIN95
	Show_Mouse();

#ifdef LORES
	SysMemPage.Blit(SeenBuff);
	SysMemPage.Blit(HidPage);
#else
	InterpolationPaletteChanged = TRUE;
	InterpolationPalette = (unsigned char*)localpal;
	Increase_Palette_Luminance(InterpolationPalette , 30,30,30,63);
	Read_Interpolation_Palette("SATSELIN.PAL");
	Interpolate_2X_Scale(PseudoSeenBuff, &SeenBuff,"SATSELIN.PAL");
#endif

	Keyboard::Clear();
	Play_Sample(kanefinl,255,128);
	Play_Sample(loopie6m,255,128);

	bool mouseshown = false;
	bool done = false;
	int selection = 1;
	bool printedtext = false;
	while (!done) {
		if (!printedtext && !Is_Sample_Playing(kanefinl)) {
			printedtext++;
			Alloc_Object(new ScorePrintClass(Text_String(TXT_SEL_TARGET), 0, 180,_tanpal));
			mouseshown = true;
			Show_Mouse();
		}
		Call_Back_Delay(1);
		if (!Keyboard::Check()) {
			if (!Is_Sample_Playing(loopie6m))	Play_Sample(loopie6m,255,128);
		} else {
			if (Is_Sample_Playing(kanefinl)) {
				Clear_KeyBuffer();
			} else {
				int key = Keyboard::Get();
				if ((key & 0x10FF) == KN_LMOUSE && !(key & KN_RLSE_BIT)) {
					int mousex = _Kbd->MouseQX;
					int mousey = _Kbd->MouseQY;
					if (mousey >= 22*RESFACTOR && mousey <= 177*RESFACTOR) {
						done++;
						if (mousex <  160*RESFACTOR && mousey <  100*RESFACTOR) selection = 2;
						if (mousex <  160*RESFACTOR && mousey >= 100*RESFACTOR) selection = 3;
						if (mousex >= 160*RESFACTOR && mousey >= 100*RESFACTOR) selection = 4;
					}
				}
			}
		}
	}
	if (mouseshown) Hide_Mouse();
#ifdef NOT_FOR_WIN95
	delete satpic;
#elif !defined(LORES)
	delete PseudoSeenBuff;
#endif //NOT_FOR_WIN95

/* get rid of all the animating objects */
	for (int i = 0; i < MAXSCOREOBJS; i++) if (ScoreObjs[i]) {
		delete ScoreObjs[i];
		ScoreObjs[i] = 0;
	}
	// erase the "choose a target" text
	SeenBuff.Fill_Rect(0,180*RESFACTOR,319*RESFACTOR,199*RESFACTOR,0);
	TextPrintBuffer->Fill_Rect(0,180*RESFACTOR,319*RESFACTOR,199*RESFACTOR,0);

	Hide_Mouse();
	Keyboard::Clear();

	Set_Font(oldfont);
	FontXSpacing = oldfontxspacing;
	Free_Sample(kanefinl);
	Free_Sample(loopie6m);

	sprintf(fname,"NODEND%d",selection);
	PreserveVQAScreen = 1;
	Play_Movie(fname);

	CountDownTimerClass count;
	if (CCFileClass("TRAILER.VQA").Is_Available()) {
		Fade_Palette_To(BlackPalette, FADE_PALETTE_MEDIUM, Call_Back);
		CCFileClass f("ATTRACT2.CPS");
#ifdef LORES
		Load_Uncompress(f, HiddenPage, HiddenPage, Palette);
		HidPage.Blit(SeenBuff);
#else
		Load_Uncompress(f, SysMemPage, SysMemPage, Palette);
		SysMemPage.Scale(SeenBuff, 0, 0, 0, 0, 320, 199, 640, 398);
#endif
		Fade_Palette_To(Palette, FADE_PALETTE_MEDIUM, Call_Back);
		Clear_KeyBuffer();
		count.Set(TIMER_SECOND*3);
		while (count.Time()) {
			Call_Back();
		}
		Fade_Palette_To(BlackPalette, FADE_PALETTE_MEDIUM, Call_Back);

		Play_Movie("TRAILER");	// Red Alert teaser.
	}

	Fade_Palette_To(BlackPalette, FADE_PALETTE_MEDIUM, Call_Back);
	CCFileClass f2("ATTRACT2.CPS");
#ifdef LORES
	Load_Uncompress(f2, HiddenPage, HiddenPage, Palette);
	HidPage.Blit(SeenBuff);
#else
	Load_Uncompress(f2, SysMemPage, SysMemPage, Palette);
	SysMemPage.Scale(SeenBuff, 0, 0, 0, 0, 320, 199, 640, 398);
#endif
	Fade_Palette_To(Palette, FADE_PALETTE_MEDIUM, Call_Back);
	Clear_KeyBuffer();
//	CountDownTimerClass count;
	count.Set(TIMER_SECOND*3);
	while (count.Time()) {
		Call_Back();
	}
	Fade_Palette_To(BlackPalette, FADE_PALETTE_MEDIUM, Call_Back);

	Play_Movie("CC2TEASE");

	delete [] (char *)localpal;
	delete TextPrintBuffer;
	BlitList.Clear();
}
#endif
