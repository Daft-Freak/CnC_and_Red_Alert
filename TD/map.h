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

/* $Header:   F:\projects\c&c\vcs\code\map.h_v   2.19   16 Oct 1995 16:46:12   JOE_BOSTIC  $ */
/***********************************************************************************************
 ***              C O N F I D E N T I A L  ---  W E S T W O O D  S T U D I O S               ***
 ***********************************************************************************************
 *                                                                                             *
 *                 Project Name : Command & Conquer                                            *
 *                                                                                             *
 *                    File Name : MAP.H                                                        *
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

#ifndef MAP_H
#define MAP_H

#include	"gscreen.h"

#define	BIGMAP	0


class MapClass: public GScreenClass
{
	public:
		MapClass(void) {};
		MapClass(NoInitClass const & x) : GScreenClass(x) {};

		/*
		** Initialization
		*/
		virtual void One_Time(void);						// Theater-specific inits
		virtual void Init_Clear(void);						// Clears all to known state
		virtual void Alloc_Cells(void);						// Allocates buffers
		virtual void Free_Cells(void);							// Frees buffers
		virtual void Init_Cells(void);							// Frees buffers

		/*--------------------------------------------------------
		** Main functions that deal with groupings of cells within the map or deals with the cell
		** as it relates to the map - not what the cell contains.
		*/
		ObjectClass *  Close_Object(COORDINATE coord) const;
		virtual void Detach(ObjectClass * ) {};
		int  Cell_Region(CELL cell);
		int  Cell_Threat(CELL cell, HousesType house);
		int  Cell_Distance(CELL cell1, CELL cell2);
		bool  In_Radar(CELL cell) const;
		void  Sight_From(CELL cell, int sightrange, bool incremental=false);
		void  Place_Down(CELL cell, ObjectClass * object);
		void  Pick_Up(CELL cell, ObjectClass * object);
		void  Overlap_Down(CELL cell, ObjectClass * object);
		void  Overlap_Up(CELL cell, ObjectClass * object);
		bool  Read_Binary(char const *root, uint32_t *crc);
		bool  Write_Binary(char const *root);
		bool  Place_Random_Crate(void);

		long  Overpass(void);

		virtual void Logic(void);
		virtual void Set_Map_Dimensions(int x, int y, int w, int h);

		/*
		**	File I/O.
		*/
		virtual void Code_Pointers(void);
		virtual void Decode_Pointers(void);

		/*
		** Debug routine
		*/
		int Validate(void);

		/*
		**	This is the dimensions and position of the sub section of the global map.
		**	It is this region that appears on the radar map and constrains normal
		**	movement.
		*/
		int MapCellX;
		int MapCellY;
		int MapCellWidth;
		int MapCellHeight;

		/*
		**	This is the total value of all harvestable Tiberium on the map.
		*/
		long TotalValue;

	protected:

		/*
		**	These are the size dimensions of the underlying array of cell objects.
		**	This is the dimensions of the "map" that the tactical view is
		**	restricted to.
		*/
		int	XSize;
		int	YSize;
		int	Size;

		static int const RadiusCount[11];
		static int const RadiusOffset[];

	private:
		friend class CellClass;

		/*
		**	Tiberium growth potiential cells are recorded here.
		*/
		CELL TiberiumGrowth[50];
		int TiberiumGrowthCount;

		/*
		**	List of cells that are full enough strength that they could spread
		**	Tiberium to adjacent cells.
		*/
		CELL TiberiumSpread[50];
		int TiberiumSpreadCount;

		/*
		**	This is the current cell number in the incremental map scan process.
		*/
		CELL TiberiumScan;

		/*
		**	If the Tiberium map scan is processing forward, then this flag
		**	will be true. It alternates between forward and backward scanning
		**	in order to avoid the "Tiberium Creep".
		*/
		unsigned IsForwardScan:1;

		enum MapEnum {SCAN_AMOUNT=MAP_CELL_TOTAL};
};

#endif
