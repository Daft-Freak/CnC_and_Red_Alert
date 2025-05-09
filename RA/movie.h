/*
**	Command & Conquer Red Alert(tm)
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

#ifndef _MPGMOVIE_H_
#define _MPGMOVIE_H_
/****************************************************************************
*
* FILE
*     MpgMovie.h
*
* DESCRIPTION
*     Movie playback using DirectShow Multimedia streaming and DirectDraw
*
* PROGRAMMER
*     Denzil E. Long, Jr.
*
* DATE
*     May 27, 1998
*
****************************************************************************/

#include <windows.h>
#include <ddraw.h>

#ifdef __GNUC__
#define DLLCALL
#else
#ifdef MPGEXPORT		  
#define DLLCALL __declspec(dllexport)
#else
#define DLLCALL __declspec(dllimport)
#endif
#endif

typedef enum
	{
	MPGCMD_ERROR = -1,
	MPGCMD_INIT = 0,
	MPGCMD_CLEANUP,
	MPGCMD_PALETTE,
	MPGCMD_UPDATE
	} MPG_CMD;

typedef enum
	{
	MPGRES_QUIT = -1,
	MPGRES_CONTINUE = 0,
	MPGRES_LOSTFOCUS,
	} MPG_RESPONSE;

typedef MPG_RESPONSE (far __stdcall *LPMPGCALLBACK)(MPG_CMD cmd, LPVOID data, LPVOID user);

extern "C"
	{
	DLLCALL void __stdcall MpgPlay(const char* name, IDirectDraw* dd,
		IDirectDrawSurface* surface, RECT* dstRect);
	DLLCALL void __stdcall MpgPause(void);
	DLLCALL void __stdcall MpgResume(void);
	DLLCALL void __stdcall MpgSetCallback(LPMPGCALLBACK callback, LPVOID user);
	}

#endif // _MPGMOVIE_H_
