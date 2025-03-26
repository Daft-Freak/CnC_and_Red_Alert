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

/* $Header:   F:\projects\c&c\vcs\code\terrain.h_v   2.16   16 Oct 1995 16:47:48   JOE_BOSTIC  $ */
/***********************************************************************************************
 ***              C O N F I D E N T I A L  ---  W E S T W O O D  S T U D I O S               ***
 ***********************************************************************************************
 *                                                                                             *
 *                 Project Name : Command & Conquer                                            *
 *                                                                                             *
 *                    File Name : TERRAIN.H                                                    *
 *                                                                                             *
 *                   Programmer : Joe L. Bostic                                                *
 *                                                                                             *
 *                   Start Date : April 29, 1994                                               *
 *                                                                                             *
 *                  Last Update : April 29, 1994   [JLB]                                       *
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#ifndef TERRAIN_H
#define TERRAIN_H

#include	"object.h"
#include	"type.h"


/****************************************************************************
**	Each type of terrain has certain pieces of static information associated
**	with it. This class elaborates this data.
*/
class TerrainClass : public ObjectClass, public StageClass
{
	public:
		TerrainTypeClass const * const Class;
		operator TerrainType(void) const {return Class->Type;};

  		/*
		**	Constructor for terrain object class.
		*/
		static void * operator new(size_t size) throw();
		static void * operator new(size_t , void * ptr) throw() {return(ptr);};
		static void operator delete(void *ptr);
		TerrainClass(void);
		TerrainClass(TerrainType id, CELL cell);
		TerrainClass(NoInitClass const & x) : ObjectClass(x), Class(Class), StageClass(x) {};
		virtual ~TerrainClass(void);
		virtual RTTIType What_Am_I(void) const {return RTTI_TERRAIN;};

		static void Init(void);

		/*
		**	Terrain specific support functions.
		*/
		void Start_To_Crumble(void);

		/*
		**	Query functions.
		*/
		virtual ObjectTypeClass const & Class_Of(void) const {return *Class;};

		/*
		**	Coordinate inquiry functions. These are used for both display and
		**	combat purposes.
		*/
		virtual COORDINATE Center_Coord(void) const;
		virtual COORDINATE Render_Coord(void) const {return Coord;};
		virtual COORDINATE Sort_Y(void) const {return Coord_Add(Coord, Class->CenterBase);};
		virtual COORDINATE Target_Coord(void) const {return Sort_Y();};

		/*
		**	Object entry and exit from the game system.
		*/
		virtual bool Unlimbo(COORDINATE coord, DirType dir=DIR_N);
		virtual bool Limbo(void);
		virtual MoveType Can_Enter_Cell(CELL cell, FacingType facing = FACING_NONE) const;

		/*
		**	Display and rendering support functionality. Supports imagery and how
		**	object interacts with the map and thus indirectly controls rendering.
		*/
		virtual void Draw_It(int x, int y, WindowNumberType window);
		virtual bool Mark(MarkType mark=MARK_CHANGE);
		unsigned char *Radar_Icon(CELL cell);

		/*
		**	User I/O.
		*/
		virtual void Clicked_As_Target(int) {};

		/*
		**	Combat related.
		*/
		virtual void Fire_Out(void);
		virtual bool Catch_Fire(void);
		virtual ResultType Take_Damage(int & damage, int distance, WarheadType warhead, TechnoClass * source);
		virtual TARGET As_Target(void) const;

		/*
		**	AI.
		*/
		virtual void AI(void);

		/*
		**	Scenario and debug support.
		*/
		#ifdef CHEAT_KEYS
		virtual void Debug_Dump(MonoClass *mono) const;
		#endif

		/*
		**	File I/O.
		*/
		static void Read_INI(char *buffer);
		static void Write_INI(char *buffer);
		static char const *INI_Name(void) {return "TERRAIN";};
		bool Load(FileClass & file);
		bool Save(FileClass & file);
		virtual void Code_Pointers(void);
		virtual void Decode_Pointers(void);

		/*
		**	Dee-buggin' support.
		*/
		int Validate(void) const;

	private:

		/*
		**	If this terrain object is on fire, then this flag will be true.
		*/
		unsigned IsOnFire:1;

		/*
		**	Is this a terrain object that undergoes crumbling animation and it is
		**	in fact crumbling at this time?
		*/
		unsigned IsCrumbling:1;

		/*
		** If this is a tree that becomes a blossom tree, is it currently doing so?
		*/
		unsigned IsBlossoming:1;

		/*
		** If this is a blossom tree, is it barnacled?
		*/
		unsigned IsBarnacled:1;

		/*
		** If this is a blossom tree that is barnacled, is it pulsing and spewing
		** out spores?
		*/
		unsigned IsSporing:1;

		/*
		** This contains the value of the Virtual Function Table Pointer
		*/
		static void * VTable;
};

#endif
