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

/* $Header:   F:\projects\c&c\vcs\code\checkbox.cpv   1.6   16 Oct 1995 16:49:36   JOE_BOSTIC  $ */
/*********************************************************************************************** 
 ***              C O N F I D E N T I A L  ---  W E S T W O O D  S T U D I O S               *** 
 *********************************************************************************************** 
 *                                                                                             * 
 *                 Project Name : Command & Conquer                                            * 
 *                                                                                             * 
 *                    File Name : CHECKBOX.CPP                                                 * 
 *                                                                                             * 
 *                   Programmer : Joe L. Bostic                                                * 
 *                                                                                             * 
 *                   Start Date : 05/26/95                                                     * 
 *                                                                                             * 
 *                  Last Update : July 1, 1995 [JLB]                                           * 
 *                                                                                             * 
 *---------------------------------------------------------------------------------------------* 
 * Functions:                                                                                  * 
 *   CheckBoxClass::Draw_Me -- Draws the checkbox imagery.                                     * 
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include	"function.h"
#include	"checkbox.h"


/*********************************************************************************************** 
 * CheckBoxClass::Draw_Me -- Draws the checkbox imagery.                                       * 
 *                                                                                             * 
 *    This routine will draw the checkbox either filled or empty as necessary.                 * 
 *                                                                                             * 
 * INPUT:   forced   -- Should the check box be drawn even if it doesn't think it needs to?    * 
 *                                                                                             * 
 * OUTPUT:  Was the check box rendered?                                                        * 
 *                                                                                             * 
 * WARNINGS:   none                                                                            * 
 *                                                                                             * 
 * HISTORY:                                                                                    * 
 *   07/01/1995 JLB : Created.                                                                 * 
 *=============================================================================================*/
int CheckBoxClass::Draw_Me(int forced)
{
	if (ToggleClass::Draw_Me(forced)) {

		Hide_Mouse();
		Draw_Box(X, Y, Width, Height, BOXSTYLE_GREEN_DOWN, false);
		LogicPage->Fill_Rect(X+1, Y+1, X+Width-2, Y+Height-2, DKGREY);
		if (IsOn) {
			LogicPage->Draw_Line(X+1, Y+1, X+Width-2, Y+Height-2, BLACK);
			LogicPage->Draw_Line(X+Width-2, Y+1, X+1, Y+Height-2, BLACK);
		}
		Show_Mouse();
		return(true);
	}
	return(false);
}	
