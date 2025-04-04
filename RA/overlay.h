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

/* $Header: /CounterStrike/OVERLAY.H 1     3/03/97 10:25a Joe_bostic $ */
/***********************************************************************************************
 ***              C O N F I D E N T I A L  ---  W E S T W O O D  S T U D I O S               ***
 ***********************************************************************************************
 *                                                                                             *
 *                 Project Name : Command & Conquer                                            *
 *                                                                                             *
 *                    File Name : OVERLAY.H                                                    *
 *                                                                                             *
 *                   Programmer : Joe L. Bostic                                                *
 *                                                                                             *
 *                   Start Date : May 17, 1994                                                 *
 *                                                                                             *
 *                  Last Update : May 17, 1994   [JLB]                                         *
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#ifndef OVERLAY_H
#define OVERLAY_H

#include	"object.h"
#include	"type.h"

/******************************************************************************
**	This class controls the overlay object. Overlay objects function congruously
**	to carpet on a floor. They have no depth, but merely control the icon to be rendered
**	as the cell's bottom most layer.
*/
class OverlayClass : public ObjectClass
{
	public:
		/*
		**	This is a pointer to the overlay object's class.
		*/
		CCPtr<OverlayTypeClass> Class;

		/*-------------------------------------------------------------------
		**	Constructors and destructors.
		*/
		static void * operator new(size_t size)  throw();
		static void * operator new(size_t , void * ptr) throw() {return(ptr);};
		static void operator delete(void *ptr);
		OverlayClass(OverlayType type, CELL pos=-1, HousesType = HOUSE_NONE);
		OverlayClass(NoInitClass const & x) : ObjectClass(x), Class(x) {};
		virtual ~OverlayClass(void) {if (GameActive) OverlayClass::Limbo();Class=0;};
		operator OverlayType(void) const {return Class->Type;};

		static void Init(void);

		/*
		**	File I/O.
		*/
		static void Read_INI(CCINIClass & ini);
		static void Write_INI(CCINIClass & ini);
		static char const *INI_Name(void) {return "OVERLAY";};
		bool Load(Straw & file);
		bool Save(Pipe & file) const;

		/*
		**	Virtual support functionality.
		*/
		virtual bool Mark(MarkType);
		virtual ObjectTypeClass const & Class_Of(void) const {return *Class;};
		virtual void Draw_It(int , int , WindowNumberType ) const {};

	private:
		/*
		**	This is used to control the marking process of the overlay. If this is
		**	set to a valid house number, then the cell that the overlay is marked down
		**	upon will be flagged as being owned by the specified house.
		*/
		static HousesType ToOwn;
};

#endif
