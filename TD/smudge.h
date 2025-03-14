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

/* $Header:   F:\projects\c&c\vcs\code\smudge.h_v   2.16   16 Oct 1995 16:47:32   JOE_BOSTIC  $ */
/***********************************************************************************************
 ***              C O N F I D E N T I A L  ---  W E S T W O O D  S T U D I O S               ***
 ***********************************************************************************************
 *                                                                                             *
 *                 Project Name : Command & Conquer                                            *
 *                                                                                             *
 *                    File Name : SMUDGE.H                                                     *
 *                                                                                             *
 *                   Programmer : Joe L. Bostic                                                *
 *                                                                                             *
 *                   Start Date : August 9, 1994                                               *
 *                                                                                             *
 *                  Last Update : August 9, 1994   [JLB]                                       *
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#ifndef SMUDGE_H
#define SMUDGE_H

#include	"object.h"
#include	"type.h"

/******************************************************************************
**	This is the transitory form for smudges. They exist as independent objects
**	only in the transition stage from creation to placement upon the map. Once
**	they are placed on the map, they merely become 'smudges' in the cell data. This
**	object is then destroyed.
*/
class SmudgeClass : public ObjectClass
{
	public:
		/*-------------------------------------------------------------------
		**	Constructors and destructors.
		*/
		static void * operator new(size_t size);
		static void operator delete(void *ptr);
		SmudgeClass(SmudgeType type, COORDINATE pos=-1, HousesType house = HOUSE_NONE);
		SmudgeClass(void) : Class(0) {};
		operator SmudgeType(void) const {return Class->Type;};
		virtual ~SmudgeClass(void) {if (GameActive) SmudgeClass::Limbo();};
		virtual RTTIType What_Am_I(void) const {return RTTI_SMUDGE;};

		static void Init(void);

		/*
		**	File I/O.
		*/
		static void Read_INI(char *);
		static void Write_INI(char *);
		static char *INI_Name(void) {return "SMUDGE";};
		bool Load(FileClass & file);
		bool Save(FileClass & file);
		virtual void Code_Pointers(void);
		virtual void Decode_Pointers(void);

		virtual ObjectTypeClass const & Class_Of(void) const {return *Class;};
		virtual bool Mark(MarkType);
		virtual void Draw_It(int , int , WindowNumberType ) {};

		void Disown(CELL cell);

		/*
		**	Dee-buggin' support.
		*/
		int Validate(void) const;

	private:

		static HousesType ToOwn;

		/*
		**	This is a pointer to the template object's class.
		*/
		SmudgeTypeClass const * const Class;

		/*
		** This contains the value of the Virtual Function Table Pointer
		*/
		static void * VTable;
};

#endif

