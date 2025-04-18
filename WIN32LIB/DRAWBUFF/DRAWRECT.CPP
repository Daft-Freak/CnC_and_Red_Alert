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
 **   C O N F I D E N T I A L --- W E S T W O O D   A S S O C I A T E S   **
 ***************************************************************************
 *                                                                         *
 *                 Project Name : Westwood 32 Bit Library						*
 *                                                                         *
 *                    File Name : DRAWRECT.C                               *
 *                                                                         *
 *                   Programmer : Christopher Yates                        *
 *                                                                         *
 *                  Last Update : August 20, 1993   [JLB]                  *
 *                                                                         *
 *-------------------------------------------------------------------------*
 * Functions:                                                              *
 *   Draw_Rect -- Draws a rectangle to the LogicPage.                      *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "gbuffer.h"

/***************************************************************************
 * Draw_Rect -- Draws a rectangle to the LogicPage.                        *
 *                                                                         *
 *    This routine will draw a rectangle to the LogicPage.  The rectangle  *
 *    doesn't have to be aligned on the vertical or horizontal axis.  In   *
 *    fact, it doesn't even have to be a rectangle.  The "square" can be   *
 *    skewed.                                                              *
 *                                                                         *
 * INPUT:   x1_pixel, y1_pixel   -- One corner.                            *
 *                                                                         *
 *          x2_pixel, y2_pixel   -- The other corner.                      *
 *                                                                         *
 *          color                -- The color to draw the lines.           *
 *                                                                         *
 * OUTPUT:  none                                                           *
 *                                                                         *
 * WARNINGS:   None, but the rectangle will be clipped to the current      *
 *             draw line clipping rectangle.                               *
 *                                                                         *
 * HISTORY:                                                                *
 *   08/20/1993 JLB : Created.                                             *
 *=========================================================================*/
VOID GraphicViewPortClass::Draw_Rect(int x1_pixel, int y1_pixel, int x2_pixel, int y2_pixel, unsigned char color)
{
	Lock();
	Draw_Line(x1_pixel, y1_pixel, x2_pixel, y1_pixel, color);
	Draw_Line(x1_pixel, y2_pixel, x2_pixel, y2_pixel, color);
	Draw_Line(x1_pixel, y1_pixel, x1_pixel, y2_pixel, color);
	Draw_Line(x2_pixel, y1_pixel, x2_pixel, y2_pixel, color);
	Unlock();
}
