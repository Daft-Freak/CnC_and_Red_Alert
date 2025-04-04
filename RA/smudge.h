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

/* $Header: /CounterStrike/SMUDGE.H 1     3/03/97 10:25a Joe_bostic $ */
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
		/*
		**	This is a pointer to the template object's class.
		*/
		CCPtr<SmudgeTypeClass> Class;

		/*-------------------------------------------------------------------
		**	Constructors and destructors.
		*/
		static void * operator new(size_t size)  throw();
		static void * operator new(size_t , void * ptr) throw() {return(ptr);};
		static void operator delete(void *ptr);
		SmudgeClass(SmudgeType type, COORDINATE pos=0xFFFFFFFFUL, HousesType house = HOUSE_NONE);
		SmudgeClass(NoInitClass const & x) : ObjectClass(x), Class(x) {};
		operator SmudgeType(void) const {return Class->Type;};
		virtual ~SmudgeClass(void) {if (GameActive) SmudgeClass::Limbo();Class=0;};

		static void Init(void);

		/*
		**	File I/O.
		*/
		static void Read_INI(CCINIClass & ini);
		static void Write_INI(CCINIClass & ini);
		static char const *INI_Name(void) {return "SMUDGE";};
		bool Load(Straw & file);
		bool Save(Pipe & file) const;

		virtual ObjectTypeClass const & Class_Of(void) const {return *Class;};
		virtual bool Mark(MarkType);
		virtual void Draw_It(int , int , WindowNumberType ) const {};

		void Disown(CELL cell);

	private:

		static HousesType ToOwn;
};

#endif
