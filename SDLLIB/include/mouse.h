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

/***********************************************************************************************
 *                                                                                             *
 *                 Project Name : Westwood 32 Bit Library                                      *
 *                                                                                             *
 *                    File Name : MOUSE.H                                                      *
 *                                                                                             *
 *                   Programmer : Philip W. Gorrow                                             *
 *                                                                                             *
 *                   Start Date : 12/12/95                                                     *
 *                                                                                             *
 *                  Last Update : December 12, 1995 [PWG]                                      *
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#ifndef WW_MOUSE_H
#define WW_MOUSE_H

#include "gbuffer.h"

class WWMouseClass {
	public:
		WWMouseClass(GraphicViewPortClass *scr, int mouse_max_width, int mouse_max_height);
		~WWMouseClass();
		void *Set_Cursor(int xhotspot, int yhotspot, void *cursor);
		void Process_Mouse(void);
		void Hide_Mouse(void);
		void Show_Mouse(void);
		void Conditional_Hide_Mouse(int x1, int y1, int x2, int y2);
		void Conditional_Show_Mouse(void);
		int Get_Mouse_State(void);
		int Get_Mouse_X(void);
		int Get_Mouse_Y(void);
		void Get_Mouse_XY(int &x, int &y);
		//
		// The following two routines can be used to render the mouse onto a graphicbuffer
		// other than the hidpage.
		//
		void Draw_Mouse(GraphicViewPortClass *scr);
		void Erase_Mouse(GraphicViewPortClass *scr, bool forced = false);

		void Block_Mouse(GraphicBufferClass *buffer);
		void Unblock_Mouse(GraphicBufferClass *buffer);
		void Set_Cursor_Clip(void);
		void Clear_Cursor_Clip(void);

		void Update_Palette();
		void Update_Pos(int x, int y);

	private:
		enum 	{
			CONDHIDE		= 1,
			CONDHIDDEN 	= 2,
		};

		uint8_t	*				MouseCursor = NULL;	// pointer to the mouse cursor in memory
		void *					SDLCursor = NULL;
		void *					SDLSurface = NULL;
		uint8_t					MouseXHot;		// X hot spot of the current mouse cursor
		uint8_t					MouseYHot;		// Y hot spot of the current mouse cursor

		uint8_t					MaxWidth;		// maximum width of mouse background buffer
		uint8_t					MaxHeight;		// maximum height of mouse background buffer

		uint8_t					CursorWidth;
		uint8_t					CursorHeight;

		uint16_t				MouseCXLeft;	// left x pos if conditional hide mouse in effect
		uint16_t				MouseCYUpper;	// upper y pos if conditional hide mouse in effect
		uint16_t				MouseCXRight;	// right x pos if conditional hide mouse in effect
		uint16_t				MouseCYLower;	// lower y pos if conditional hide mouse in effect
		char						MCFlags;			// conditional hide mouse flags
		char						MCCount;			// nesting count for conditional hide mouse

		int16_t					EraseBuffX;
		int16_t					EraseBuffY;

		GraphicViewPortClass	*Screen;			// pointer to the surface mouse was init'd with
		char *					PrevCursor;		// pointer to previous cursor shape
		int						State;

		int LastX = 0, LastY = 0;
		bool PaletteDirty = false;
};

void Hide_Mouse(void);
void Show_Mouse(void);
void Conditional_Hide_Mouse(int x1, int y1, int x2, int y2);
void Conditional_Show_Mouse(void);
int Get_Mouse_State(void);
void *Set_Mouse_Cursor(int hotx, int hoty, void *cursor);
int Get_Mouse_X(void);
int Get_Mouse_Y(void);

void Update_Mouse_Palette();
void Update_Mouse_Pos(int x, int y);

#endif
