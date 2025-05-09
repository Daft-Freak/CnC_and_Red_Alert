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

/***************************************************************************
 **   C O N F I D E N T I A L --- W E S T W O O D    S T U D I O S        **
 ***************************************************************************
 *                                                                         *
 *                 Project Name : Westwood Library                         *
 *                                                                         *
 *                    File Name : SOUND.H                                  *
 *                                                                         *
 *                   Programmer : Joe L. Bostic                            *
 *                                                                         *
 *                   Start Date : September 1, 1993                        *
 *                                                                         *
 *                  Last Update : September 1, 1993   [JLB]                *
 *                                                                         *
 *-------------------------------------------------------------------------*
 * Functions:                                                              *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#ifndef SOUND_H
#define SOUND_H

#define	HMI_DRIVER	TRUE
#include	"sos.h"
#include	"soscomp.h"

/*
**	Maximum number of sound effects that may run at once.
*/
#define	MAX_SFX		4

/*
** Size of temp HMI low memory staging buffer.
*/
#define	SFX_MINI_STAGE_BUFFER_SIZE		(1024*4)


extern WORD _SoundVolume;
extern WORD _ScoreVolume;

#endif
