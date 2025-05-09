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

/* $Header: /CounterStrike/EDIT.H 1     3/03/97 10:24a Joe_bostic $ */
/***********************************************************************************************
 ***              C O N F I D E N T I A L  ---  W E S T W O O D  S T U D I O S               ***
 ***********************************************************************************************
 *                                                                                             *
 *                 Project Name : Command & Conquer                                            *
 *                                                                                             *
 *                    File Name : EDIT.H                                                       *
 *                                                                                             *
 *                   Programmer : Joe L. Bostic                                                *
 *                                                                                             *
 *                   Start Date : 01/15/95                                                     *
 *                                                                                             *
 *                  Last Update : January 15, 1995 [JLB]                                       *
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#ifndef EDIT_H
#define EDIT_H

class EditClass : public ControlClass
{
	public:
		typedef enum EditStyle {
			ALPHA			=0x0001,		// Edit accepts alphabetic characters.
			NUMERIC		=0x0002,		// Edit accepts numbers.
			MISC			=0x0004,		// Edit accepts misc graphic characters.
			UPPERCASE	=0x0008,		// Force to upper case.
			ALPHANUMERIC=(int)ALPHA|(int)NUMERIC|(int)MISC
		} EditStyle;

		EditClass (int id, char * text, int max_len, TextPrintType flags, int x, int y, int w=-1, int h=-1, EditStyle style=ALPHANUMERIC);
		virtual ~EditClass(void);

		virtual void Set_Focus(void);
		virtual int  Draw_Me(int forced);
		virtual void Set_Text(char * text, int max_len);
		virtual char * Get_Text(void) {return(String);};
		void Set_Color (RemapControlType * color) { Color = color; }

		void Set_Read_Only(int rdonly) {IsReadOnly = rdonly;}

	protected:

		/*
		**	These are the text size and style flags to be used when displaying the text
		**	of the edit gadget.
		*/
		TextPrintType TextFlags;

		/*
		**	Input flags that control what characters are allowed in the string.
		*/
		EditStyle EditFlags;

		/*
		**	Pointer to text staging buffer and the maximum length of the string it
		**	can contain.
		*/
		char *String;
		int MaxLength;

		/*
		**	This is the current length of the string. This length will never exceed the
		**	MaxLength allowed.
		*/
		int Length;

		/*
		**	This is the desired color of the edit control.
		*/
		RemapControlType * Color;

		virtual int Action (unsigned flags, KeyNumType &key);
		virtual void Draw_Background(void);
		virtual void Draw_Text(char const * text);
		virtual bool Handle_Key(KeyASCIIType ascii);

	private:
		int IsReadOnly;
};

#endif
