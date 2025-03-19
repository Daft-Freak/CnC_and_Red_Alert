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

/* $Header: /CounterStrike/CDATA.CPP 1     3/03/97 10:24a Joe_bostic $ */
/***********************************************************************************************
 ***              C O N F I D E N T I A L  ---  W E S T W O O D  S T U D I O S               ***
 ***********************************************************************************************
 *                                                                                             *
 *                 Project Name : Command & Conquer                                            *
 *                                                                                             *
 *                    File Name : CDATA.CPP                                                    *
 *                                                                                             *
 *                   Programmer : Joe L. Bostic                                                *
 *                                                                                             *
 *                   Start Date : May 16, 1994                                                 *
 *                                                                                             *
 *                  Last Update : July 6, 1996 [JLB]                                           *
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 *   TemplateTypeClass::As_Reference -- Fetches a reference to the template specified.         *
 *   TemplateTypeClass::Create_And_Place -- Creates and places a template object on the map.   *
 *   TemplateTypeClass::Create_One_Of -- Creates an object of this template type.              *
 *   TemplateTypeClass::Display -- Displays a generic representation of template.              *
 *   TemplateTypeClass::From_Name -- Determine template from ASCII name.                       *
 *   TemplateTypeClass::Init -- Loads graphic data for templates.                              *
 *   TemplateTypeClass::Land_Type -- Determines land type from template and icon number.       *
 *   TemplateTypeClass::Occupy_List -- Determines occupation list.                             *
 *   TemplateTypeClass::One_Time -- Performs one-time initialization                           *
 *   TemplateTypeClass::Prep_For_Add -- Prepares to add template to scenario.                  *
 *   TemplateTypeClass::TemplateTypeClass -- Constructor for template type objects.            *
 *   TemplateTypeClass::operator delete -- Deletes a template type object.                     *
 *   TemplateTypeClass::operator new -- Allocates a template type from special heap.           *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include	"function.h"


/***********************************************************************************************
 * TemplateTypeClass::TemplateTypeClass -- Constructor for template type objects.              *
 *                                                                                             *
 *    This is the constructor for the template types.                                          *
 *                                                                                             *
 * INPUT:   see below...                                                                       *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   07/29/1994 JLB : Created.                                                                 *
 *=============================================================================================*/
TemplateTypeClass::TemplateTypeClass(
	TemplateType iconset,
	int theater,
	char const * ininame,
	int fullname) :
		ObjectTypeClass(
			RTTI_TEMPLATETYPE,
			int(iconset),
			false,
			true,
			false,
			false,
			true,
			true,
			false,
			fullname,
			ininame),
	Type(iconset),
	Theater(theater),
	Width(0),
	Height(0)
{
}


/***********************************************************************************************
 * TemplateTypeClass::operator new -- Allocates a template type from special heap.             *
 *                                                                                             *
 *    This allocates a template type object from the special heap used for that purpose.       *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  Returns with a pointer to the newly allocated template type object. If no object   *
 *          could be allocated, then NULL is returned.                                         *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   07/06/1996 JLB : Created.                                                                 *
 *=============================================================================================*/
void * TemplateTypeClass::operator new(size_t) throw()
{
	return(TemplateTypes.Alloc());
}


/***********************************************************************************************
 * TemplateTypeClass::operator delete -- Deletes a template type object.                       *
 *                                                                                             *
 *    This routine will return a template type object back to the special heap it was          *
 *    allocated from.                                                                          *
 *                                                                                             *
 * INPUT:   ptr   -- Pointer to the template type object to free.                              *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   07/06/1996 JLB : Created.                                                                 *
 *=============================================================================================*/
void TemplateTypeClass::operator delete(void * ptr)
{
	TemplateTypes.Free((TemplateTypeClass *)ptr);
}


static void _Watcom_Ugh_Hack(void)
{
	(void)new TemplateTypeClass(				// TEMPLATE_ROAD37
		TEMPLATE_ROAD37,
		THEATERF_TEMPERATE|THEATERF_SNOW,
		"D37",
		TXT_ROAD
	);
	(void)new TemplateTypeClass(				// TEMPLATE_ROAD38
		TEMPLATE_ROAD38,
		THEATERF_TEMPERATE|THEATERF_SNOW,
		"D38",
		TXT_ROAD
	);
	(void)new TemplateTypeClass(				// TEMPLATE_ROAD39
		TEMPLATE_ROAD39,
		THEATERF_TEMPERATE|THEATERF_SNOW,
		"D39",
		TXT_ROAD
	);
	(void)new TemplateTypeClass(				// TEMPLATE_ROAD40
		TEMPLATE_ROAD40,
		THEATERF_TEMPERATE|THEATERF_SNOW,
		"D40",
		TXT_ROAD
	);
	(void)new TemplateTypeClass(				// TEMPLATE_ROAD41
		TEMPLATE_ROAD41,
		THEATERF_TEMPERATE|THEATERF_SNOW,
		"D41",
		TXT_ROAD
	);
	(void)new TemplateTypeClass(				// TEMPLATE_ROAD42
		TEMPLATE_ROAD42,
		THEATERF_TEMPERATE|THEATERF_SNOW,
		"D42",
		TXT_ROAD
	);
	(void)new TemplateTypeClass(				// TEMPLATE_ROAD43
		TEMPLATE_ROAD43,
		THEATERF_TEMPERATE|THEATERF_SNOW,
		"D43",
		TXT_ROAD
	);
	(void)new TemplateTypeClass(				// TEMPLATE_ROUGH01
		TEMPLATE_ROUGH01,
		THEATERF_TEMPERATE|THEATERF_SNOW,
		"RF01",
		TXT_ROCK
	);
	(void)new TemplateTypeClass(				// TEMPLATE_ROUGH02
		TEMPLATE_ROUGH02,
		THEATERF_TEMPERATE|THEATERF_SNOW,
		"RF02",
		TXT_ROCK
	);
	(void)new TemplateTypeClass(				// TEMPLATE_ROUGH03
		TEMPLATE_ROUGH03,
		THEATERF_TEMPERATE|THEATERF_SNOW,
		"RF03",
		TXT_ROCK
	);
	(void)new TemplateTypeClass(				// TEMPLATE_ROUGH04
		TEMPLATE_ROUGH04,
		THEATERF_TEMPERATE|THEATERF_SNOW,
		"RF04",
		TXT_ROCK
	);
	(void)new TemplateTypeClass(				// TEMPLATE_ROUGH05
		TEMPLATE_ROUGH05,
		THEATERF_TEMPERATE|THEATERF_SNOW,
		"RF05",
		TXT_ROCK
	);
	(void)new TemplateTypeClass(				// TEMPLATE_ROUGH06
		TEMPLATE_ROUGH06,
		THEATERF_TEMPERATE|THEATERF_SNOW,
		"RF06",
		TXT_ROCK
	);
	(void)new TemplateTypeClass(				// TEMPLATE_ROUGH07
		TEMPLATE_ROUGH07,
		THEATERF_TEMPERATE|THEATERF_SNOW,
		"RF07",
		TXT_ROCK
	);
	(void)new TemplateTypeClass(				// TEMPLATE_ROUGH08
		TEMPLATE_ROUGH08,
		THEATERF_TEMPERATE|THEATERF_SNOW,
		"RF08",
		TXT_ROCK
	);
	(void)new TemplateTypeClass(				// TEMPLATE_ROUGH09
		TEMPLATE_ROUGH09,
		THEATERF_TEMPERATE|THEATERF_SNOW,
		"RF09",
		TXT_ROCK
	);
	(void)new TemplateTypeClass(				// TEMPLATE_ROUGH10
		TEMPLATE_ROUGH10,
		THEATERF_TEMPERATE|THEATERF_SNOW,
		"RF10",
		TXT_ROCK
	);
	(void)new TemplateTypeClass(				// TEMPLATE_ROUGH11
		TEMPLATE_ROUGH11,
		THEATERF_TEMPERATE|THEATERF_SNOW,
		"RF11",
		TXT_ROCK
	);
	(void)new TemplateTypeClass(				// TEMPLATE_ROAD44
		TEMPLATE_ROAD44,
		THEATERF_TEMPERATE|THEATERF_SNOW,
		"D44",
		TXT_ROAD
	);
	(void)new TemplateTypeClass(				// TEMPLATE_ROAD45
		TEMPLATE_ROAD45,
		THEATERF_TEMPERATE|THEATERF_SNOW,
		"D45",
		TXT_ROAD
	);
	(void)new TemplateTypeClass(				// TEMPLATE_RIVER14
		TEMPLATE_RIVER14,
		THEATERF_TEMPERATE|THEATERF_SNOW,
		"RV14",
		TXT_RIVER
	);
	(void)new TemplateTypeClass(				// TEMPLATE_RIVER15
		TEMPLATE_RIVER15,
		THEATERF_TEMPERATE|THEATERF_SNOW,
		"RV15",
		TXT_RIVER
	);
	(void)new TemplateTypeClass(				// TEMPLATE_RIVERCLIFF01
		TEMPLATE_RIVERCLIFF01,
		THEATERF_TEMPERATE|THEATERF_SNOW,
		"RC01",
		TXT_RIVER
	);
	(void)new TemplateTypeClass(				// TEMPLATE_RIVERCLIFF02
		TEMPLATE_RIVERCLIFF02,
		THEATERF_TEMPERATE|THEATERF_SNOW,
		"RC02",
		TXT_RIVER
	);
	(void)new TemplateTypeClass(				// TEMPLATE_RIVERCLIFF03
		TEMPLATE_RIVERCLIFF03,
		THEATERF_TEMPERATE|THEATERF_SNOW,
		"RC03",
		TXT_RIVER
	);
	(void)new TemplateTypeClass(				// TEMPLATE_RIVERCLIFF04
		TEMPLATE_RIVERCLIFF04,
		THEATERF_TEMPERATE|THEATERF_SNOW,
		"RC04",
		TXT_RIVER
	);
	(void)new TemplateTypeClass(				// TEMPLATE_BRIDGE_1A
		TEMPLATE_BRIDGE_1A,
		THEATERF_TEMPERATE|THEATERF_SNOW,
		"BR1A",
		TXT_BRIDGE
	);
	(void)new TemplateTypeClass(				// TEMPLATE_BRIDGE_1B
		TEMPLATE_BRIDGE_1B,
		THEATERF_TEMPERATE|THEATERF_SNOW,
		"BR1B",
		TXT_BRIDGE
	);
	(void)new TemplateTypeClass(				// TEMPLATE_BRIDGE_1C
		TEMPLATE_BRIDGE_1C,
		THEATERF_TEMPERATE|THEATERF_SNOW,
		"BR1C",
		TXT_BRIDGE
	);
	(void)new TemplateTypeClass(				// TEMPLATE_BRIDGE_2A
		TEMPLATE_BRIDGE_2A,
		THEATERF_TEMPERATE|THEATERF_SNOW,
		"BR2A",
		TXT_BRIDGE
	);
	(void)new TemplateTypeClass(				// TEMPLATE_BRIDGE_2B
		TEMPLATE_BRIDGE_2B,
		THEATERF_TEMPERATE|THEATERF_SNOW,
		"BR2B",
		TXT_BRIDGE
	);
	(void)new TemplateTypeClass(				// TEMPLATE_BRIDGE_2C
		TEMPLATE_BRIDGE_2C,
		THEATERF_TEMPERATE|THEATERF_SNOW,
		"BR2C",
		TXT_BRIDGE
	);
	(void)new TemplateTypeClass(				// TEMPLATE_BRIDGE_3A
		TEMPLATE_BRIDGE_3A,
		THEATERF_TEMPERATE|THEATERF_SNOW,
		"BR3A",
		TXT_BRIDGE
	);
	(void)new TemplateTypeClass(				// TEMPLATE_BRIDGE_3B
		TEMPLATE_BRIDGE_3B,
		THEATERF_TEMPERATE|THEATERF_SNOW,
		"BR3B",
		TXT_BRIDGE
	);
	(void)new TemplateTypeClass(				// TEMPLATE_BRIDGE_3C
		TEMPLATE_BRIDGE_3C,
		THEATERF_TEMPERATE|THEATERF_SNOW,
		"BR3C",
		TXT_BRIDGE
	);
	(void)new TemplateTypeClass(				// TEMPLATE_BRIDGE_3D
		TEMPLATE_BRIDGE_3D,
		THEATERF_TEMPERATE|THEATERF_SNOW,
		"BR3D",
		TXT_BRIDGE
	);
	(void)new TemplateTypeClass(				// TEMPLATE_BRIDGE_3E
		TEMPLATE_BRIDGE_3E,
		THEATERF_TEMPERATE|THEATERF_SNOW,
		"BR3E",
		TXT_BRIDGE
	);
	(void)new TemplateTypeClass(				// TEMPLATE_BRIDGE_3F
		TEMPLATE_BRIDGE_3F,
		THEATERF_TEMPERATE|THEATERF_SNOW,
		"BR3F",
		TXT_BRIDGE
	);
	(void)new TemplateTypeClass(				// TEMPLATE_F01
		TEMPLATE_F01,
		THEATERF_TEMPERATE|THEATERF_SNOW,
		"F01",
		TXT_RIVER
	);
	(void)new TemplateTypeClass(				// TEMPLATE_F02
		TEMPLATE_F02,
		THEATERF_TEMPERATE|THEATERF_SNOW,
		"F02",
		TXT_RIVER
	);
	(void)new TemplateTypeClass(				// TEMPLATE_F03
		TEMPLATE_F03,
		THEATERF_TEMPERATE|THEATERF_SNOW,
		"F03",
		TXT_RIVER
	);
	(void)new TemplateTypeClass(				// TEMPLATE_F04
		TEMPLATE_F04,
		THEATERF_TEMPERATE|THEATERF_SNOW,
		"F04",
		TXT_RIVER
	);
	(void)new TemplateTypeClass(				// TEMPLATE_F05
		TEMPLATE_F05,
		THEATERF_TEMPERATE|THEATERF_SNOW,
		"F05",
		TXT_RIVER
	);
	(void)new TemplateTypeClass(				// TEMPLATE_F06
		TEMPLATE_F06,
		THEATERF_TEMPERATE|THEATERF_SNOW,
		"F06",
		TXT_RIVER
	);
	(void)new TemplateTypeClass(				// TEMPLATE_ARRO0001
		TEMPLATE_ARRO0001,
		THEATERF_INTERIOR,
		"ARRO0001",
		TXT_INTERIOR
	);
	(void)new TemplateTypeClass(				// TEMPLATE_ARRO0002
		TEMPLATE_ARRO0002,
		THEATERF_INTERIOR,
		"ARRO0002",
		TXT_INTERIOR
	);
	(void)new TemplateTypeClass(				// TEMPLATE_ARRO0003
		TEMPLATE_ARRO0003,
		THEATERF_INTERIOR,
		"ARRO0003",
		TXT_INTERIOR
	);
	(void)new TemplateTypeClass(				// TEMPLATE_ARRO0004
		TEMPLATE_ARRO0004,
		THEATERF_INTERIOR,
		"ARRO0004",
		TXT_INTERIOR
	);
	(void)new TemplateTypeClass(				// TEMPLATE_ARRO0005
		TEMPLATE_ARRO0005,
		THEATERF_INTERIOR,
		"ARRO0005",
		TXT_INTERIOR
	);
	(void)new TemplateTypeClass(				// TEMPLATE_ARRO0006
		TEMPLATE_ARRO0006,
		THEATERF_INTERIOR,
		"ARRO0006",
		TXT_INTERIOR
	);
	(void)new TemplateTypeClass(				// TEMPLATE_ARRO0007
		TEMPLATE_ARRO0007,
		THEATERF_INTERIOR,
		"ARRO0007",
		TXT_INTERIOR
	);
	(void)new TemplateTypeClass(				// TEMPLATE_ARRO0008
		TEMPLATE_ARRO0008,
		THEATERF_INTERIOR,
		"ARRO0008",
		TXT_INTERIOR
	);
	(void)new TemplateTypeClass(				// TEMPLATE_ARRO0009
		TEMPLATE_ARRO0009,
		THEATERF_INTERIOR,
		"ARRO0009",
		TXT_INTERIOR
	);
	(void)new TemplateTypeClass(				// TEMPLATE_ARRO0010
		TEMPLATE_ARRO0010,
		THEATERF_INTERIOR,
		"ARRO0010",
		TXT_INTERIOR
	);
	(void)new TemplateTypeClass(				// TEMPLATE_ARRO0011
		TEMPLATE_ARRO0011,
		THEATERF_INTERIOR,
		"ARRO0011",
		TXT_INTERIOR
	);
	(void)new TemplateTypeClass(				// TEMPLATE_ARRO0012
		TEMPLATE_ARRO0012,
		THEATERF_INTERIOR,
		"ARRO0012",
		TXT_INTERIOR
	);
	(void)new TemplateTypeClass(				// TEMPLATE_ARRO0013
		TEMPLATE_ARRO0013,
		THEATERF_INTERIOR,
		"ARRO0013",
		TXT_INTERIOR
	);
	(void)new TemplateTypeClass(				// TEMPLATE_ARRO0014
		TEMPLATE_ARRO0014,
		THEATERF_INTERIOR,
		"ARRO0014",
		TXT_INTERIOR
	);
	(void)new TemplateTypeClass(				// TEMPLATE_ARRO0015
		TEMPLATE_ARRO0015,
		THEATERF_INTERIOR,
		"ARRO0015",
		TXT_INTERIOR
	);
	(void)new TemplateTypeClass(				// TEMPLATE_FLOR0001
		TEMPLATE_FLOR0001,
		THEATERF_INTERIOR,
		"FLOR0001",
		TXT_INTERIOR
	);
	(void)new TemplateTypeClass(				// TEMPLATE_FLOR0002
		TEMPLATE_FLOR0002,
		THEATERF_INTERIOR,
		"FLOR0002",
		TXT_INTERIOR
	);
	(void)new TemplateTypeClass(				// TEMPLATE_FLOR0003
		TEMPLATE_FLOR0003,
		THEATERF_INTERIOR,
		"FLOR0003",
		TXT_INTERIOR
	);
	(void)new TemplateTypeClass(				// TEMPLATE_FLOR0004
		TEMPLATE_FLOR0004,
		THEATERF_INTERIOR,
		"FLOR0004",
		TXT_INTERIOR
	);
	(void)new TemplateTypeClass(				// TEMPLATE_FLOR0005
		TEMPLATE_FLOR0005,
		THEATERF_INTERIOR,
		"FLOR0005",
		TXT_INTERIOR
	);
	(void)new TemplateTypeClass(				// TEMPLATE_FLOR0006
		TEMPLATE_FLOR0006,
		THEATERF_INTERIOR,
		"FLOR0006",
		TXT_INTERIOR
	);
	(void)new TemplateTypeClass(				// TEMPLATE_FLOR0007
		TEMPLATE_FLOR0007,
		THEATERF_INTERIOR,
		"FLOR0007",
		TXT_INTERIOR
	);
	(void)new TemplateTypeClass(				// TEMPLATE_GFLR0001
		TEMPLATE_GFLR0001,
		THEATERF_INTERIOR,
		"GFLR0001",
		TXT_INTERIOR
	);
	(void)new TemplateTypeClass(				// TEMPLATE_GFLR0002
		TEMPLATE_GFLR0002,
		THEATERF_INTERIOR,
		"GFLR0002",
		TXT_INTERIOR
	);
	(void)new TemplateTypeClass(				// TEMPLATE_GFLR0003
		TEMPLATE_GFLR0003,
		THEATERF_INTERIOR,
		"GFLR0003",
		TXT_INTERIOR
	);
	(void)new TemplateTypeClass(				// TEMPLATE_GFLR0004
		TEMPLATE_GFLR0004,
		THEATERF_INTERIOR,
		"GFLR0004",
		TXT_INTERIOR
	);
	(void)new TemplateTypeClass(				// TEMPLATE_GFLR0005
		TEMPLATE_GFLR0005,
		THEATERF_INTERIOR,
		"GFLR0005",
		TXT_INTERIOR
	);
	(void)new TemplateTypeClass(				// TEMPLATE_GSTR0001
		TEMPLATE_GSTR0001,
		THEATERF_INTERIOR,
		"GSTR0001",
		TXT_INTERIOR
	);
	(void)new TemplateTypeClass(				// TEMPLATE_GSTR0002
		TEMPLATE_GSTR0002,
		THEATERF_INTERIOR,
		"GSTR0002",
		TXT_INTERIOR
	);
	(void)new TemplateTypeClass(				// TEMPLATE_GSTR0003
		TEMPLATE_GSTR0003,
		THEATERF_INTERIOR,
		"GSTR0003",
		TXT_INTERIOR
	);
	(void)new TemplateTypeClass(				// TEMPLATE_GSTR0004
		TEMPLATE_GSTR0004,
		THEATERF_INTERIOR,
		"GSTR0004",
		TXT_INTERIOR
	);
	(void)new TemplateTypeClass(				// TEMPLATE_GSTR0005
		TEMPLATE_GSTR0005,
		THEATERF_INTERIOR,
		"GSTR0005",
		TXT_INTERIOR
	);
	(void)new TemplateTypeClass(				// TEMPLATE_GSTR0006
		TEMPLATE_GSTR0006,
		THEATERF_INTERIOR,
		"GSTR0006",
		TXT_INTERIOR
	);
	(void)new TemplateTypeClass(				// TEMPLATE_GSTR0007
		TEMPLATE_GSTR0007,
		THEATERF_INTERIOR,
		"GSTR0007",
		TXT_INTERIOR
	);
	(void)new TemplateTypeClass(				// TEMPLATE_GSTR0008
		TEMPLATE_GSTR0008,
		THEATERF_INTERIOR,
		"GSTR0008",
		TXT_INTERIOR
	);
	(void)new TemplateTypeClass(				// TEMPLATE_GSTR0009
		TEMPLATE_GSTR0009,
		THEATERF_INTERIOR,
		"GSTR0009",
		TXT_INTERIOR
	);
	(void)new TemplateTypeClass(				// TEMPLATE_GSTR0010
		TEMPLATE_GSTR0010,
		THEATERF_INTERIOR,
		"GSTR0010",
		TXT_INTERIOR
	);
	(void)new TemplateTypeClass(				// TEMPLATE_GSTR0011
		TEMPLATE_GSTR0011,
		THEATERF_INTERIOR,
		"GSTR0011",
		TXT_INTERIOR
	);
	(void)new TemplateTypeClass(				// TEMPLATE_LWAL0001
		TEMPLATE_LWAL0001,
		THEATERF_INTERIOR,
		"LWAL0001",
		TXT_INTERIOR
	);
	(void)new TemplateTypeClass(				// TEMPLATE_LWAL0002
		TEMPLATE_LWAL0002,
		THEATERF_INTERIOR,
		"LWAL0002",
		TXT_INTERIOR
	);
	(void)new TemplateTypeClass(				// TEMPLATE_LWAL0003
		TEMPLATE_LWAL0003,
		THEATERF_INTERIOR,
		"LWAL0003",
		TXT_INTERIOR
	);
	(void)new TemplateTypeClass(				// TEMPLATE_LWAL0004
		TEMPLATE_LWAL0004,
		THEATERF_INTERIOR,
		"LWAL0004",
		TXT_INTERIOR
	);
	(void)new TemplateTypeClass(				// TEMPLATE_LWAL0005
		TEMPLATE_LWAL0005,
		THEATERF_INTERIOR,
		"LWAL0005",
		TXT_INTERIOR
	);
	(void)new TemplateTypeClass(				// TEMPLATE_LWAL0006
		TEMPLATE_LWAL0006,
		THEATERF_INTERIOR,
		"LWAL0006",
		TXT_INTERIOR
	);
	(void)new TemplateTypeClass(				// TEMPLATE_LWAL0007
		TEMPLATE_LWAL0007,
		THEATERF_INTERIOR,
		"LWAL0007",
		TXT_INTERIOR
	);
	(void)new TemplateTypeClass(				// TEMPLATE_LWAL0008
		TEMPLATE_LWAL0008,
		THEATERF_INTERIOR,
		"LWAL0008",
		TXT_INTERIOR
	);
	(void)new TemplateTypeClass(				// TEMPLATE_LWAL0009
		TEMPLATE_LWAL0009,
		THEATERF_INTERIOR,
		"LWAL0009",
		TXT_INTERIOR
	);
	(void)new TemplateTypeClass(				// TEMPLATE_LWAL0010
		TEMPLATE_LWAL0010,
		THEATERF_INTERIOR,
		"LWAL0010",
		TXT_INTERIOR
	);
	(void)new TemplateTypeClass(				// TEMPLATE_LWAL0011
		TEMPLATE_LWAL0011,
		THEATERF_INTERIOR,
		"LWAL0011",
		TXT_INTERIOR
	);
	(void)new TemplateTypeClass(				// TEMPLATE_LWAL0012
		TEMPLATE_LWAL0012,
		THEATERF_INTERIOR,
		"LWAL0012",
		TXT_INTERIOR
	);
	(void)new TemplateTypeClass(				// TEMPLATE_LWAL0013
		TEMPLATE_LWAL0013,
		THEATERF_INTERIOR,
		"LWAL0013",
		TXT_INTERIOR
	);
	(void)new TemplateTypeClass(				// TEMPLATE_LWAL0014
		TEMPLATE_LWAL0014,
		THEATERF_INTERIOR,
		"LWAL0014",
		TXT_INTERIOR
	);
	(void)new TemplateTypeClass(				// TEMPLATE_LWAL0015
		TEMPLATE_LWAL0015,
		THEATERF_INTERIOR,
		"LWAL0015",
		TXT_INTERIOR
	);
	(void)new TemplateTypeClass(				// TEMPLATE_LWAL0016
		TEMPLATE_LWAL0016,
		THEATERF_INTERIOR,
		"LWAL0016",
		TXT_INTERIOR
	);
	(void)new TemplateTypeClass(				// TEMPLATE_LWAL0017
		TEMPLATE_LWAL0017,
		THEATERF_INTERIOR,
		"LWAL0017",
		TXT_INTERIOR
	);
	(void)new TemplateTypeClass(				// TEMPLATE_LWAL0018
		TEMPLATE_LWAL0018,
		THEATERF_INTERIOR,
		"LWAL0018",
		TXT_INTERIOR
	);
	(void)new TemplateTypeClass(				// TEMPLATE_LWAL0019
		TEMPLATE_LWAL0019,
		THEATERF_INTERIOR,
		"LWAL0019",
		TXT_INTERIOR
	);
	(void)new TemplateTypeClass(				// TEMPLATE_LWAL0020
		TEMPLATE_LWAL0020,
		THEATERF_INTERIOR,
		"LWAL0020",
		TXT_INTERIOR
	);
	(void)new TemplateTypeClass(				// TEMPLATE_LWAL0021
		TEMPLATE_LWAL0021,
		THEATERF_INTERIOR,
		"LWAL0021",
		TXT_INTERIOR
	);
	(void)new TemplateTypeClass(				// TEMPLATE_LWAL0022
		TEMPLATE_LWAL0022,
		THEATERF_INTERIOR,
		"LWAL0022",
		TXT_INTERIOR
	);
	(void)new TemplateTypeClass(				// TEMPLATE_LWAL0023
		TEMPLATE_LWAL0023,
		THEATERF_INTERIOR,
		"LWAL0023",
		TXT_INTERIOR
	);
	(void)new TemplateTypeClass(				// TEMPLATE_LWAL0024
		TEMPLATE_LWAL0024,
		THEATERF_INTERIOR,
		"LWAL0024",
		TXT_INTERIOR
	);
	(void)new TemplateTypeClass(				// TEMPLATE_LWAL0025
		TEMPLATE_LWAL0025,
		THEATERF_INTERIOR,
		"LWAL0025",
		TXT_INTERIOR
	);
	(void)new TemplateTypeClass(				// TEMPLATE_LWAL0026
		TEMPLATE_LWAL0026,
		THEATERF_INTERIOR,
		"LWAL0026",
		TXT_INTERIOR
	);
	(void)new TemplateTypeClass(				// TEMPLATE_LWAL0027
		TEMPLATE_LWAL0027,
		THEATERF_INTERIOR,
		"LWAL0027",
		TXT_INTERIOR
	);
	(void)new TemplateTypeClass(				// TEMPLATE_STRP0001
		TEMPLATE_STRP0001,
		THEATERF_INTERIOR,
		"STRP0001",
		TXT_INTERIOR
	);
	(void)new TemplateTypeClass(				// TEMPLATE_STRP0002
		TEMPLATE_STRP0002,
		THEATERF_INTERIOR,
		"STRP0002",
		TXT_INTERIOR
	);
	(void)new TemplateTypeClass(				// TEMPLATE_STRP0003
		TEMPLATE_STRP0003,
		THEATERF_INTERIOR,
		"STRP0003",
		TXT_INTERIOR
	);
	(void)new TemplateTypeClass(				// TEMPLATE_STRP0004
		TEMPLATE_STRP0004,
		THEATERF_INTERIOR,
		"STRP0004",
		TXT_INTERIOR
	);
	(void)new TemplateTypeClass(				// TEMPLATE_STRP0005
		TEMPLATE_STRP0005,
		THEATERF_INTERIOR,
		"STRP0005",
		TXT_INTERIOR
	);
	(void)new TemplateTypeClass(				// TEMPLATE_STRP0006
		TEMPLATE_STRP0006,
		THEATERF_INTERIOR,
		"STRP0006",
		TXT_INTERIOR
	);
	(void)new TemplateTypeClass(				// TEMPLATE_STRP0007
		TEMPLATE_STRP0007,
		THEATERF_INTERIOR,
		"STRP0007",
		TXT_INTERIOR
	);
	(void)new TemplateTypeClass(				// TEMPLATE_STRP0008
		TEMPLATE_STRP0008,
		THEATERF_INTERIOR,
		"STRP0008",
		TXT_INTERIOR
	);
	(void)new TemplateTypeClass(				// TEMPLATE_STRP0009
		TEMPLATE_STRP0009,
		THEATERF_INTERIOR,
		"STRP0009",
		TXT_INTERIOR
	);
	(void)new TemplateTypeClass(				// TEMPLATE_STRP0010
		TEMPLATE_STRP0010,
		THEATERF_INTERIOR,
		"STRP0010",
		TXT_INTERIOR
	);
	(void)new TemplateTypeClass(				// TEMPLATE_STRP0011
		TEMPLATE_STRP0011,
		THEATERF_INTERIOR,
		"STRP0011",
		TXT_INTERIOR
	);
	(void)new TemplateTypeClass(				// TEMPLATE_WALL0001
		TEMPLATE_WALL0001,
		THEATERF_INTERIOR,
		"WALL0001",
		TXT_INTERIOR
	);
	(void)new TemplateTypeClass(				// TEMPLATE_WALL0002
		TEMPLATE_WALL0002,
		THEATERF_INTERIOR,
		"WALL0002",
		TXT_INTERIOR
	);
	(void)new TemplateTypeClass(				// TEMPLATE_WALL0003
		TEMPLATE_WALL0003,
		THEATERF_INTERIOR,
		"WALL0003",
		TXT_INTERIOR
	);
	(void)new TemplateTypeClass(				// TEMPLATE_WALL0004
		TEMPLATE_WALL0004,
		THEATERF_INTERIOR,
		"WALL0004",
		TXT_INTERIOR
	);
	(void)new TemplateTypeClass(				// TEMPLATE_WALL0005
		TEMPLATE_WALL0005,
		THEATERF_INTERIOR,
		"WALL0005",
		TXT_INTERIOR
	);
	(void)new TemplateTypeClass(				// TEMPLATE_WALL0006
		TEMPLATE_WALL0006,
		THEATERF_INTERIOR,
		"WALL0006",
		TXT_INTERIOR
	);
	(void)new TemplateTypeClass(				// TEMPLATE_WALL0007
		TEMPLATE_WALL0007,
		THEATERF_INTERIOR,
		"WALL0007",
		TXT_INTERIOR
	);
	(void)new TemplateTypeClass(				// TEMPLATE_WALL0008
		TEMPLATE_WALL0008,
		THEATERF_INTERIOR,
		"WALL0008",
		TXT_INTERIOR
	);
	(void)new TemplateTypeClass(				// TEMPLATE_WALL0009
		TEMPLATE_WALL0009,
		THEATERF_INTERIOR,
		"WALL0009",
		TXT_INTERIOR
	);
	(void)new TemplateTypeClass(				// TEMPLATE_WALL0010
		TEMPLATE_WALL0010,
		THEATERF_INTERIOR,
		"WALL0010",
		TXT_INTERIOR
	);
	(void)new TemplateTypeClass(				// TEMPLATE_WALL0011
		TEMPLATE_WALL0011,
		THEATERF_INTERIOR,
		"WALL0011",
		TXT_INTERIOR
	);
	(void)new TemplateTypeClass(				// TEMPLATE_WALL0012
		TEMPLATE_WALL0012,
		THEATERF_INTERIOR,
		"WALL0012",
		TXT_INTERIOR
	);
	(void)new TemplateTypeClass(				// TEMPLATE_WALL0013
		TEMPLATE_WALL0013,
		THEATERF_INTERIOR,
		"WALL0013",
		TXT_INTERIOR
	);
	(void)new TemplateTypeClass(				// TEMPLATE_WALL0014
		TEMPLATE_WALL0014,
		THEATERF_INTERIOR,
		"WALL0014",
		TXT_INTERIOR
	);
	(void)new TemplateTypeClass(				// TEMPLATE_WALL0015
		TEMPLATE_WALL0015,
		THEATERF_INTERIOR,
		"WALL0015",
		TXT_INTERIOR
	);
	(void)new TemplateTypeClass(				// TEMPLATE_WALL0016
		TEMPLATE_WALL0016,
		THEATERF_INTERIOR,
		"WALL0016",
		TXT_INTERIOR
	);
	(void)new TemplateTypeClass(				// TEMPLATE_WALL0017
		TEMPLATE_WALL0017,
		THEATERF_INTERIOR,
		"WALL0017",
		TXT_INTERIOR
	);
	(void)new TemplateTypeClass(				// TEMPLATE_WALL0018
		TEMPLATE_WALL0018,
		THEATERF_INTERIOR,
		"WALL0018",
		TXT_INTERIOR
	);
	(void)new TemplateTypeClass(				// TEMPLATE_WALL0019
		TEMPLATE_WALL0019,
		THEATERF_INTERIOR,
		"WALL0019",
		TXT_INTERIOR
	);
	(void)new TemplateTypeClass(				// TEMPLATE_WALL0020
		TEMPLATE_WALL0020,
		THEATERF_INTERIOR,
		"WALL0020",
		TXT_INTERIOR
	);
	(void)new TemplateTypeClass(				// TEMPLATE_WALL0021
		TEMPLATE_WALL0021,
		THEATERF_INTERIOR,
		"WALL0021",
		TXT_INTERIOR
	);
	(void)new TemplateTypeClass(				// TEMPLATE_WALL0022
		TEMPLATE_WALL0022,
		THEATERF_INTERIOR,
		"WALL0022",
		TXT_INTERIOR
	);
	(void)new TemplateTypeClass(				// TEMPLATE_WALL0023
		TEMPLATE_WALL0023,
		THEATERF_INTERIOR,
		"WALL0023",
		TXT_INTERIOR
	);
	(void)new TemplateTypeClass(				// TEMPLATE_WALL0024
		TEMPLATE_WALL0024,
		THEATERF_INTERIOR,
		"WALL0024",
		TXT_INTERIOR
	);
	(void)new TemplateTypeClass(				// TEMPLATE_WALL0025
		TEMPLATE_WALL0025,
		THEATERF_INTERIOR,
		"WALL0025",
		TXT_INTERIOR
	);
	(void)new TemplateTypeClass(				// TEMPLATE_WALL0026
		TEMPLATE_WALL0026,
		THEATERF_INTERIOR,
		"WALL0026",
		TXT_INTERIOR
	);
	(void)new TemplateTypeClass(				// TEMPLATE_WALL0027
		TEMPLATE_WALL0027,
		THEATERF_INTERIOR,
		"WALL0027",
		TXT_INTERIOR
	);
	(void)new TemplateTypeClass(				// TEMPLATE_WALL0028
		TEMPLATE_WALL0028,
		THEATERF_INTERIOR,
		"WALL0028",
		TXT_INTERIOR
	);
	(void)new TemplateTypeClass(				// TEMPLATE_WALL0029
		TEMPLATE_WALL0029,
		THEATERF_INTERIOR,
		"WALL0029",
		TXT_INTERIOR
	);
	(void)new TemplateTypeClass(				// TEMPLATE_WALL0030
		TEMPLATE_WALL0030,
		THEATERF_INTERIOR,
		"WALL0030",
		TXT_INTERIOR
	);
	(void)new TemplateTypeClass(				// TEMPLATE_WALL0031
		TEMPLATE_WALL0031,
		THEATERF_INTERIOR,
		"WALL0031",
		TXT_INTERIOR
	);
	(void)new TemplateTypeClass(				// TEMPLATE_WALL0032
		TEMPLATE_WALL0032,
		THEATERF_INTERIOR,
		"WALL0032",
		TXT_INTERIOR
	);
	(void)new TemplateTypeClass(				// TEMPLATE_WALL0033
		TEMPLATE_WALL0033,
		THEATERF_INTERIOR,
		"WALL0033",
		TXT_INTERIOR
	);
	(void)new TemplateTypeClass(				// TEMPLATE_WALL0034
		TEMPLATE_WALL0034,
		THEATERF_INTERIOR,
		"WALL0034",
		TXT_INTERIOR
	);
	(void)new TemplateTypeClass(				// TEMPLATE_WALL0035
		TEMPLATE_WALL0035,
		THEATERF_INTERIOR,
		"WALL0035",
		TXT_INTERIOR
	);
	(void)new TemplateTypeClass(				// TEMPLATE_WALL0036
		TEMPLATE_WALL0036,
		THEATERF_INTERIOR,
		"WALL0036",
		TXT_INTERIOR
	);
	(void)new TemplateTypeClass(				// TEMPLATE_WALL0037
		TEMPLATE_WALL0037,
		THEATERF_INTERIOR,
		"WALL0037",
		TXT_INTERIOR
	);
	(void)new TemplateTypeClass(				// TEMPLATE_WALL0038
		TEMPLATE_WALL0038,
		THEATERF_INTERIOR,
		"WALL0038",
		TXT_INTERIOR
	);
	(void)new TemplateTypeClass(				// TEMPLATE_WALL0039
		TEMPLATE_WALL0039,
		THEATERF_INTERIOR,
		"WALL0039",
		TXT_INTERIOR
	);
	(void)new TemplateTypeClass(				// TEMPLATE_WALL0040
		TEMPLATE_WALL0040,
		THEATERF_INTERIOR,
		"WALL0040",
		TXT_INTERIOR
	);
	(void)new TemplateTypeClass(				// TEMPLATE_WALL0041
		TEMPLATE_WALL0041,
		THEATERF_INTERIOR,
		"WALL0041",
		TXT_INTERIOR
	);
	(void)new TemplateTypeClass(				// TEMPLATE_WALL0042
		TEMPLATE_WALL0042,
		THEATERF_INTERIOR,
		"WALL0042",
		TXT_INTERIOR
	);
	(void)new TemplateTypeClass(				// TEMPLATE_WALL0043
		TEMPLATE_WALL0043,
		THEATERF_INTERIOR,
		"WALL0043",
		TXT_INTERIOR
	);
	(void)new TemplateTypeClass(				// TEMPLATE_WALL0044
		TEMPLATE_WALL0044,
		THEATERF_INTERIOR,
		"WALL0044",
		TXT_INTERIOR
	);
	(void)new TemplateTypeClass(				// TEMPLATE_WALL0045
		TEMPLATE_WALL0045,
		THEATERF_INTERIOR,
		"WALL0045",
		TXT_INTERIOR
	);
	(void)new TemplateTypeClass(				// TEMPLATE_WALL0046
		TEMPLATE_WALL0046,
		THEATERF_INTERIOR,
		"WALL0046",
		TXT_INTERIOR
	);
	(void)new TemplateTypeClass(				// TEMPLATE_WALL0047
		TEMPLATE_WALL0047,
		THEATERF_INTERIOR,
		"WALL0047",
		TXT_INTERIOR
	);
	(void)new TemplateTypeClass(				// TEMPLATE_WALL0048
		TEMPLATE_WALL0048,
		THEATERF_INTERIOR,
		"WALL0048",
		TXT_INTERIOR
	);
	(void)new TemplateTypeClass(				// TEMPLATE_WALL0049
		TEMPLATE_WALL0049,
		THEATERF_INTERIOR,
		"WALL0049",
		TXT_INTERIOR
	);
	(void)new TemplateTypeClass(				// TEMPLATE_BRIDGE1H
		TEMPLATE_BRIDGE1H,
		THEATERF_TEMPERATE|THEATERF_SNOW,
		"BRIDGE1H",
		TXT_BRIDGE
	);
	(void)new TemplateTypeClass(				// TEMPLATE_BRIDGE2H
		TEMPLATE_BRIDGE2H,
		THEATERF_TEMPERATE|THEATERF_SNOW,
		"BRIDGE2H",
		TXT_BRIDGE
	);

	(void)new TemplateTypeClass(				// TEMPLATE_BRIDGE_1AX
		TEMPLATE_BRIDGE_1AX,
		THEATERF_TEMPERATE|THEATERF_SNOW,
		"BR1X",
		TXT_BRIDGE
	);
	(void)new TemplateTypeClass(				// TEMPLATE_BRIDGE_2AX
		TEMPLATE_BRIDGE_2AX,
		THEATERF_TEMPERATE|THEATERF_SNOW,
		"BR2X",
		TXT_BRIDGE
	);
	(void)new TemplateTypeClass(				// TEMPLATE_BRIDGE1X
		TEMPLATE_BRIDGE1X,
		THEATERF_TEMPERATE|THEATERF_SNOW,
		"BRIDGE1X",
		TXT_BRIDGE
	);
	(void)new TemplateTypeClass(				// TEMPLATE_BRIDGE2X
		TEMPLATE_BRIDGE2X,
		THEATERF_TEMPERATE|THEATERF_SNOW,
		"BRIDGE2X",
		TXT_BRIDGE
	);

	(void)new TemplateTypeClass(				// TEMPLATE_XTRA0001
		TEMPLATE_XTRA0001,
		THEATERF_INTERIOR,
		"XTRA0001",
		TXT_INTERIOR
	);
	(void)new TemplateTypeClass(				// TEMPLATE_XTRA0002
		TEMPLATE_XTRA0002,
		THEATERF_INTERIOR,
		"XTRA0002",
		TXT_INTERIOR
	);
	(void)new TemplateTypeClass(				// TEMPLATE_XTRA0003
		TEMPLATE_XTRA0003,
		THEATERF_INTERIOR,
		"XTRA0003",
		TXT_INTERIOR
	);
	(void)new TemplateTypeClass(				// TEMPLATE_XTRA0004
		TEMPLATE_XTRA0004,
		THEATERF_INTERIOR,
		"XTRA0004",
		TXT_INTERIOR
	);
	(void)new TemplateTypeClass(				// TEMPLATE_XTRA0005
		TEMPLATE_XTRA0005,
		THEATERF_INTERIOR,
		"XTRA0005",
		TXT_INTERIOR
	);
	(void)new TemplateTypeClass(				// TEMPLATE_XTRA0006
		TEMPLATE_XTRA0006,
		THEATERF_INTERIOR,
		"XTRA0006",
		TXT_INTERIOR
	);
	(void)new TemplateTypeClass(				// TEMPLATE_XTRA0007
		TEMPLATE_XTRA0007,
		THEATERF_INTERIOR,
		"XTRA0007",
		TXT_INTERIOR
	);
	(void)new TemplateTypeClass(				// TEMPLATE_XTRA0008
		TEMPLATE_XTRA0008,
		THEATERF_INTERIOR,
		"XTRA0008",
		TXT_INTERIOR
	);
	(void)new TemplateTypeClass(				// TEMPLATE_XTRA0009
		TEMPLATE_XTRA0009,
		THEATERF_INTERIOR,
		"XTRA0009",
		TXT_INTERIOR
	);
	(void)new TemplateTypeClass(				// TEMPLATE_XTRA0010
		TEMPLATE_XTRA0010,
		THEATERF_INTERIOR,
		"XTRA0010",
		TXT_INTERIOR
	);
	(void)new TemplateTypeClass(				// TEMPLATE_XTRA0011
		TEMPLATE_XTRA0011,
		THEATERF_INTERIOR,
		"XTRA0011",
		TXT_INTERIOR
	);
	(void)new TemplateTypeClass(				// TEMPLATE_XTRA0012
		TEMPLATE_XTRA0012,
		THEATERF_INTERIOR,
		"XTRA0012",
		TXT_INTERIOR
	);
	(void)new TemplateTypeClass(				// TEMPLATE_XTRA0013
		TEMPLATE_XTRA0013,
		THEATERF_INTERIOR,
		"XTRA0013",
		TXT_INTERIOR
	);
	(void)new TemplateTypeClass(				// TEMPLATE_XTRA0014
		TEMPLATE_XTRA0014,
		THEATERF_INTERIOR,
		"XTRA0014",
		TXT_INTERIOR
	);
	(void)new TemplateTypeClass(				// TEMPLATE_XTRA0015
		TEMPLATE_XTRA0015,
		THEATERF_INTERIOR,
		"XTRA0015",
		TXT_INTERIOR
	);
	(void)new TemplateTypeClass(				// TEMPLATE_XTRA0016
		TEMPLATE_XTRA0016,
		THEATERF_INTERIOR,
		"XTRA0016",
		TXT_INTERIOR
	);

#ifdef FIXIT_ANTS
	(void)new TemplateTypeClass(				// TEMPLATE_HILL01
		TEMPLATE_HILL01,
		THEATERF_TEMPERATE,
		"HILL01",
		TXT_ROCK
	);
#endif
}



void TemplateTypeClass::Init_Heap(void)
{
	/*
	**	These template type class objects must be allocated in the exact order that they
	**	are specified in the TemplateType enumeration. This is necessary because the heap
	**	allocation block index serves double duty as the type number index.
	*/
	(void)new TemplateTypeClass(				// TEMPLATE_CLEAR1
		TEMPLATE_CLEAR1,
		THEATERF_TEMPERATE|THEATERF_SNOW|THEATERF_INTERIOR,
		"CLEAR1",
		TXT_CLEAR
	);
	(void)new TemplateTypeClass(				// TEMPLATE_WATER
		TEMPLATE_WATER,
		THEATERF_TEMPERATE|THEATERF_SNOW,
		"W1",
		TXT_WATER
	);
	(void)new TemplateTypeClass(				// TEMPLATE_WATER2
		TEMPLATE_WATER2,
		THEATERF_TEMPERATE|THEATERF_SNOW,
		"W2",
		TXT_WATER
	);
	(void)new TemplateTypeClass(				// TEMPLATE_SHORE01
		TEMPLATE_SHORE01,
		THEATERF_TEMPERATE|THEATERF_SNOW,
		"SH01",
		TXT_SHORE
	);
	(void)new TemplateTypeClass(				// TEMPLATE_SHORE02
		TEMPLATE_SHORE02,
		THEATERF_TEMPERATE|THEATERF_SNOW,
		"SH02",
		TXT_SHORE
	);
	(void)new TemplateTypeClass(				// TEMPLATE_SHORE03
		TEMPLATE_SHORE03,
		THEATERF_TEMPERATE|THEATERF_SNOW,
		"SH03",
		TXT_SHORE
	);
	(void)new TemplateTypeClass(				// TEMPLATE_SHORE04
		TEMPLATE_SHORE04,
		THEATERF_TEMPERATE|THEATERF_SNOW,
		"SH04",
		TXT_SHORE
	);
	(void)new TemplateTypeClass(				// TEMPLATE_SHORE05
		TEMPLATE_SHORE05,
		THEATERF_TEMPERATE|THEATERF_SNOW,
		"SH05",
		TXT_SHORE
	);
	(void)new TemplateTypeClass(				// TEMPLATE_SHORE06
		TEMPLATE_SHORE06,
		THEATERF_TEMPERATE|THEATERF_SNOW,
		"SH06",
		TXT_SHORE
	);
	(void)new TemplateTypeClass(				// TEMPLATE_SHORE07
		TEMPLATE_SHORE07,
		THEATERF_TEMPERATE|THEATERF_SNOW,
		"SH07",
		TXT_SHORE
	);
	(void)new TemplateTypeClass(				// TEMPLATE_SHORE08
		TEMPLATE_SHORE08,
		THEATERF_TEMPERATE|THEATERF_SNOW,
		"SH08",
		TXT_SHORE
	);
	(void)new TemplateTypeClass(				// TEMPLATE_SHORE09
		TEMPLATE_SHORE09,
		THEATERF_TEMPERATE|THEATERF_SNOW,
		"SH09",
		TXT_SHORE
	);
	(void)new TemplateTypeClass(				// TEMPLATE_SHORE10
		TEMPLATE_SHORE10,
		THEATERF_TEMPERATE|THEATERF_SNOW,
		"SH10",
		TXT_SHORE
	);
	(void)new TemplateTypeClass(				// TEMPLATE_SHORE11
		TEMPLATE_SHORE11,
		THEATERF_TEMPERATE|THEATERF_SNOW,
		"SH11",
		TXT_SHORE
	);
	(void)new TemplateTypeClass(				// TEMPLATE_SHORE12
		TEMPLATE_SHORE12,
		THEATERF_TEMPERATE|THEATERF_SNOW,
		"SH12",
		TXT_SHORE
	);
	(void)new TemplateTypeClass(				// TEMPLATE_SHORE13
		TEMPLATE_SHORE13,
		THEATERF_TEMPERATE|THEATERF_SNOW,
		"SH13",
		TXT_SHORE
	);
	(void)new TemplateTypeClass(				// TEMPLATE_SHORE14
		TEMPLATE_SHORE14,
		THEATERF_TEMPERATE|THEATERF_SNOW,
		"SH14",
		TXT_SHORE
	);
	(void)new TemplateTypeClass(				// TEMPLATE_SHORE15
		TEMPLATE_SHORE15,
		THEATERF_TEMPERATE|THEATERF_SNOW,
		"SH15",
		TXT_SHORE
	);
	(void)new TemplateTypeClass(				// TEMPLATE_SHORE16
		TEMPLATE_SHORE16,
		THEATERF_TEMPERATE|THEATERF_SNOW,
		"SH16",
		TXT_SHORE
	);
	(void)new TemplateTypeClass(				// TEMPLATE_SHORE17
		TEMPLATE_SHORE17,
		THEATERF_TEMPERATE|THEATERF_SNOW,
		"SH17",
		TXT_SHORE
	);
	(void)new TemplateTypeClass(				// TEMPLATE_SHORE18
		TEMPLATE_SHORE18,
		THEATERF_TEMPERATE|THEATERF_SNOW,
		"SH18",
		TXT_SHORE
	);
	(void)new TemplateTypeClass(				// TEMPLATE_SHORE19
		TEMPLATE_SHORE19,
		THEATERF_TEMPERATE|THEATERF_SNOW,
		"SH19",
		TXT_SHORE
	);
	(void)new TemplateTypeClass(				// TEMPLATE_SHORE20
		TEMPLATE_SHORE20,
		THEATERF_TEMPERATE|THEATERF_SNOW,
		"SH20",
		TXT_SHORE
	);
	(void)new TemplateTypeClass(				// TEMPLATE_SHORE21
		TEMPLATE_SHORE21,
		THEATERF_TEMPERATE|THEATERF_SNOW,
		"SH21",
		TXT_SHORE
	);
	(void)new TemplateTypeClass(				// TEMPLATE_SHORE22
		TEMPLATE_SHORE22,
		THEATERF_TEMPERATE|THEATERF_SNOW,
		"SH22",
		TXT_SHORE
	);
	(void)new TemplateTypeClass(				// TEMPLATE_SHORE23
		TEMPLATE_SHORE23,
		THEATERF_TEMPERATE|THEATERF_SNOW,
		"SH23",
		TXT_SHORE
	);
	(void)new TemplateTypeClass(				// TEMPLATE_SHORE24
		TEMPLATE_SHORE24,
		THEATERF_TEMPERATE|THEATERF_SNOW,
		"SH24",
		TXT_SHORE
	);
	(void)new TemplateTypeClass(				// TEMPLATE_SHORE25
		TEMPLATE_SHORE25,
		THEATERF_TEMPERATE|THEATERF_SNOW,
		"SH25",
		TXT_SHORE
	);
	(void)new TemplateTypeClass(				// TEMPLATE_SHORE26
		TEMPLATE_SHORE26,
		THEATERF_TEMPERATE|THEATERF_SNOW,
		"SH26",
		TXT_SHORE
	);
	(void)new TemplateTypeClass(				// TEMPLATE_SHORE27
		TEMPLATE_SHORE27,
		THEATERF_TEMPERATE|THEATERF_SNOW,
		"SH27",
		TXT_SHORE
	);
	(void)new TemplateTypeClass(				// TEMPLATE_SHORE28
		TEMPLATE_SHORE28,
		THEATERF_TEMPERATE|THEATERF_SNOW,
		"SH28",
		TXT_SHORE
	);
	(void)new TemplateTypeClass(				// TEMPLATE_SHORE29
		TEMPLATE_SHORE29,
		THEATERF_TEMPERATE|THEATERF_SNOW,
		"SH29",
		TXT_SHORE
	);
	(void)new TemplateTypeClass(				// TEMPLATE_SHORE30
		TEMPLATE_SHORE30,
		THEATERF_TEMPERATE|THEATERF_SNOW,
		"SH30",
		TXT_SHORE
	);
	(void)new TemplateTypeClass(				// TEMPLATE_SHORE31
		TEMPLATE_SHORE31,
		THEATERF_TEMPERATE|THEATERF_SNOW,
		"SH31",
		TXT_SHORE
	);
	(void)new TemplateTypeClass(				// TEMPLATE_SHORE32
		TEMPLATE_SHORE32,
		THEATERF_TEMPERATE|THEATERF_SNOW,
		"SH32",
		TXT_SHORE
	);
	(void)new TemplateTypeClass(				// TEMPLATE_SHORE33
		TEMPLATE_SHORE33,
		THEATERF_TEMPERATE|THEATERF_SNOW,
		"SH33",
		TXT_SHORE
	);
	(void)new TemplateTypeClass(				// TEMPLATE_SHORE34
		TEMPLATE_SHORE34,
		THEATERF_TEMPERATE|THEATERF_SNOW,
		"SH34",
		TXT_SHORE
	);
	(void)new TemplateTypeClass(				// TEMPLATE_SHORE35
		TEMPLATE_SHORE35,
		THEATERF_TEMPERATE|THEATERF_SNOW,
		"SH35",
		TXT_SHORE
	);
	(void)new TemplateTypeClass(				// TEMPLATE_SHORE36
		TEMPLATE_SHORE36,
		THEATERF_TEMPERATE|THEATERF_SNOW,
		"SH36",
		TXT_SHORE
	);
	(void)new TemplateTypeClass(				// TEMPLATE_SHORE37
		TEMPLATE_SHORE37,
		THEATERF_TEMPERATE|THEATERF_SNOW,
		"SH37",
		TXT_SHORE
	);
	(void)new TemplateTypeClass(				// TEMPLATE_SHORE38
		TEMPLATE_SHORE38,
		THEATERF_TEMPERATE|THEATERF_SNOW,
		"SH38",
		TXT_SHORE
	);
	(void)new TemplateTypeClass(				// TEMPLATE_SHORE39
		TEMPLATE_SHORE39,
		THEATERF_TEMPERATE|THEATERF_SNOW,
		"SH39",
		TXT_SHORE
	);
	(void)new TemplateTypeClass(				// TEMPLATE_SHORE40
		TEMPLATE_SHORE40,
		THEATERF_TEMPERATE|THEATERF_SNOW,
		"SH40",
		TXT_SHORE
	);
	(void)new TemplateTypeClass(				// TEMPLATE_SHORE41
		TEMPLATE_SHORE41,
		THEATERF_TEMPERATE|THEATERF_SNOW,
		"SH41",
		TXT_SHORE
	);
	(void)new TemplateTypeClass(				// TEMPLATE_SHORE42
		TEMPLATE_SHORE42,
		THEATERF_TEMPERATE|THEATERF_SNOW,
		"SH42",
		TXT_SHORE
	);
	(void)new TemplateTypeClass(				// TEMPLATE_SHORE43
		TEMPLATE_SHORE43,
		THEATERF_TEMPERATE|THEATERF_SNOW,
		"SH43",
		TXT_SHORE
	);
	(void)new TemplateTypeClass(				// TEMPLATE_SHORE44
		TEMPLATE_SHORE44,
		THEATERF_TEMPERATE|THEATERF_SNOW,
		"SH44",
		TXT_SHORE
	);
	(void)new TemplateTypeClass(				// TEMPLATE_SHORE45
		TEMPLATE_SHORE45,
		THEATERF_TEMPERATE|THEATERF_SNOW,
		"SH45",
		TXT_SHORE
	);
	(void)new TemplateTypeClass(				// TEMPLATE_SHORE46
		TEMPLATE_SHORE46,
		THEATERF_TEMPERATE|THEATERF_SNOW,
		"SH46",
		TXT_SHORE
	);
	(void)new TemplateTypeClass(				// TEMPLATE_SHORE47
		TEMPLATE_SHORE47,
		THEATERF_TEMPERATE|THEATERF_SNOW,
		"SH47",
		TXT_SHORE
	);
	(void)new TemplateTypeClass(				// TEMPLATE_SHORE48
		TEMPLATE_SHORE48,
		THEATERF_TEMPERATE|THEATERF_SNOW,
		"SH48",
		TXT_SHORE
	);
	(void)new TemplateTypeClass(				// TEMPLATE_SHORE49
		TEMPLATE_SHORE49,
		THEATERF_TEMPERATE|THEATERF_SNOW,
		"SH49",
		TXT_SHORE
	);
	(void)new TemplateTypeClass(				// TEMPLATE_SHORE50
		TEMPLATE_SHORE50,
		THEATERF_TEMPERATE|THEATERF_SNOW,
		"SH50",
		TXT_SHORE
	);
	(void)new TemplateTypeClass(				// TEMPLATE_SHORE51
		TEMPLATE_SHORE51,
		THEATERF_TEMPERATE|THEATERF_SNOW,
		"SH51",
		TXT_SHORE
	);
	(void)new TemplateTypeClass(				// TEMPLATE_SHORE52
		TEMPLATE_SHORE52,
		THEATERF_TEMPERATE|THEATERF_SNOW,
		"SH52",
		TXT_SHORE
	);
	(void)new TemplateTypeClass(				// TEMPLATE_SHORE53
		TEMPLATE_SHORE53,
		THEATERF_TEMPERATE|THEATERF_SNOW,
		"SH53",
		TXT_SHORE
	);
	(void)new TemplateTypeClass(				// TEMPLATE_SHORE54
		TEMPLATE_SHORE54,
		THEATERF_TEMPERATE|THEATERF_SNOW,
		"SH54",
		TXT_SHORE
	);
	(void)new TemplateTypeClass(				// TEMPLATE_SHORE55
		TEMPLATE_SHORE55,
		THEATERF_TEMPERATE|THEATERF_SNOW,
		"SH55",
		TXT_SHORE
	);
	(void)new TemplateTypeClass(				// TEMPLATE_SHORE56
		TEMPLATE_SHORE56,
		THEATERF_TEMPERATE|THEATERF_SNOW,
		"SH56",
		TXT_SHORE
	);
	(void)new TemplateTypeClass(				// TEMPLATE_SHORECLIFF01
		TEMPLATE_SHORECLIFF01,
		THEATERF_TEMPERATE|THEATERF_SNOW,
		"WC01",
		TXT_SHORE
	);
	(void)new TemplateTypeClass(				// TEMPLATE_SHORECLIFF02
		TEMPLATE_SHORECLIFF02,
		THEATERF_TEMPERATE|THEATERF_SNOW,
		"WC02",
		TXT_SHORE
	);
	(void)new TemplateTypeClass(				// TEMPLATE_SHORECLIFF03
		TEMPLATE_SHORECLIFF03,
		THEATERF_TEMPERATE|THEATERF_SNOW,
		"WC03",
		TXT_SHORE
	);
	(void)new TemplateTypeClass(				// TEMPLATE_SHORECLIFF04
		TEMPLATE_SHORECLIFF04,
		THEATERF_TEMPERATE|THEATERF_SNOW,
		"WC04",
		TXT_SHORE
	);
	(void)new TemplateTypeClass(				// TEMPLATE_SHORECLIFF05
		TEMPLATE_SHORECLIFF05,
		THEATERF_TEMPERATE|THEATERF_SNOW,
		"WC05",
		TXT_SHORE
	);
	(void)new TemplateTypeClass(				// TEMPLATE_SHORECLIFF06
		TEMPLATE_SHORECLIFF06,
		THEATERF_TEMPERATE|THEATERF_SNOW,
		"WC06",
		TXT_SHORE
	);
	(void)new TemplateTypeClass(				// TEMPLATE_SHORECLIFF07
		TEMPLATE_SHORECLIFF07,
		THEATERF_TEMPERATE|THEATERF_SNOW,
		"WC07",
		TXT_SHORE
	);
	(void)new TemplateTypeClass(				// TEMPLATE_SHORECLIFF08
		TEMPLATE_SHORECLIFF08,
		THEATERF_TEMPERATE|THEATERF_SNOW,
		"WC08",
		TXT_SHORE
	);
	(void)new TemplateTypeClass(				// TEMPLATE_SHORECLIFF09
		TEMPLATE_SHORECLIFF09,
		THEATERF_TEMPERATE|THEATERF_SNOW,
		"WC09",
		TXT_SHORE
	);
	(void)new TemplateTypeClass(				// TEMPLATE_SHORECLIFF10
		TEMPLATE_SHORECLIFF10,
		THEATERF_TEMPERATE|THEATERF_SNOW,
		"WC10",
		TXT_SHORE
	);
	(void)new TemplateTypeClass(				// TEMPLATE_SHORECLIFF11
		TEMPLATE_SHORECLIFF11,
		THEATERF_TEMPERATE|THEATERF_SNOW,
		"WC11",
		TXT_SHORE
	);
	(void)new TemplateTypeClass(				// TEMPLATE_SHORECLIFF12
		TEMPLATE_SHORECLIFF12,
		THEATERF_TEMPERATE|THEATERF_SNOW,
		"WC12",
		TXT_SHORE
	);
	(void)new TemplateTypeClass(				// TEMPLATE_SHORECLIFF13
		TEMPLATE_SHORECLIFF13,
		THEATERF_TEMPERATE|THEATERF_SNOW,
		"WC13",
		TXT_SHORE
	);
	(void)new TemplateTypeClass(				// TEMPLATE_SHORECLIFF14
		TEMPLATE_SHORECLIFF14,
		THEATERF_TEMPERATE|THEATERF_SNOW,
		"WC14",
		TXT_SHORE
	);
	(void)new TemplateTypeClass(				// TEMPLATE_SHORECLIFF15
		TEMPLATE_SHORECLIFF15,
		THEATERF_TEMPERATE|THEATERF_SNOW,
		"WC15",
		TXT_SHORE
	);
	(void)new TemplateTypeClass(				// TEMPLATE_SHORECLIFF16
		TEMPLATE_SHORECLIFF16,
		THEATERF_TEMPERATE|THEATERF_SNOW,
		"WC16",
		TXT_SHORE
	);
	(void)new TemplateTypeClass(				// TEMPLATE_SHORECLIFF17
		TEMPLATE_SHORECLIFF17,
		THEATERF_TEMPERATE|THEATERF_SNOW,
		"WC17",
		TXT_SHORE
	);
	(void)new TemplateTypeClass(				// TEMPLATE_SHORECLIFF18
		TEMPLATE_SHORECLIFF18,
		THEATERF_TEMPERATE|THEATERF_SNOW,
		"WC18",
		TXT_SHORE
	);
	(void)new TemplateTypeClass(				// TEMPLATE_SHORECLIFF19
		TEMPLATE_SHORECLIFF19,
		THEATERF_TEMPERATE|THEATERF_SNOW,
		"WC19",
		TXT_SHORE
	);
	(void)new TemplateTypeClass(				// TEMPLATE_SHORECLIFF20
		TEMPLATE_SHORECLIFF20,
		THEATERF_TEMPERATE|THEATERF_SNOW,
		"WC20",
		TXT_SHORE
	);
	(void)new TemplateTypeClass(				// TEMPLATE_SHORECLIFF21
		TEMPLATE_SHORECLIFF21,
		THEATERF_TEMPERATE|THEATERF_SNOW,
		"WC21",
		TXT_SHORE
	);
	(void)new TemplateTypeClass(				// TEMPLATE_SHORECLIFF22
		TEMPLATE_SHORECLIFF22,
		THEATERF_TEMPERATE|THEATERF_SNOW,
		"WC22",
		TXT_SHORE
	);
	(void)new TemplateTypeClass(				// TEMPLATE_SHORECLIFF23
		TEMPLATE_SHORECLIFF23,
		THEATERF_TEMPERATE|THEATERF_SNOW,
		"WC23",
		TXT_SHORE
	);
	(void)new TemplateTypeClass(				// TEMPLATE_SHORECLIFF24
		TEMPLATE_SHORECLIFF24,
		THEATERF_TEMPERATE|THEATERF_SNOW,
		"WC24",
		TXT_SHORE
	);
	(void)new TemplateTypeClass(				// TEMPLATE_SHORECLIFF25
		TEMPLATE_SHORECLIFF25,
		THEATERF_TEMPERATE|THEATERF_SNOW,
		"WC25",
		TXT_SHORE
	);
	(void)new TemplateTypeClass(				// TEMPLATE_SHORECLIFF26
		TEMPLATE_SHORECLIFF26,
		THEATERF_TEMPERATE|THEATERF_SNOW,
		"WC26",
		TXT_SHORE
	);
	(void)new TemplateTypeClass(				// TEMPLATE_SHORECLIFF27
		TEMPLATE_SHORECLIFF27,
		THEATERF_TEMPERATE|THEATERF_SNOW,
		"WC27",
		TXT_SHORE
	);
	(void)new TemplateTypeClass(				// TEMPLATE_SHORECLIFF28
		TEMPLATE_SHORECLIFF28,
		THEATERF_TEMPERATE|THEATERF_SNOW,
		"WC28",
		TXT_SHORE
	);
	(void)new TemplateTypeClass(				// TEMPLATE_SHORECLIFF29
		TEMPLATE_SHORECLIFF29,
		THEATERF_TEMPERATE|THEATERF_SNOW,
		"WC29",
		TXT_SHORE
	);
	(void)new TemplateTypeClass(				// TEMPLATE_SHORECLIFF30
		TEMPLATE_SHORECLIFF30,
		THEATERF_TEMPERATE|THEATERF_SNOW,
		"WC30",
		TXT_SHORE
	);
	(void)new TemplateTypeClass(				// TEMPLATE_SHORECLIFF31
		TEMPLATE_SHORECLIFF31,
		THEATERF_TEMPERATE|THEATERF_SNOW,
		"WC31",
		TXT_SHORE
	);
	(void)new TemplateTypeClass(				// TEMPLATE_SHORECLIFF32
		TEMPLATE_SHORECLIFF32,
		THEATERF_TEMPERATE|THEATERF_SNOW,
		"WC32",
		TXT_SHORE
	);
	(void)new TemplateTypeClass(				// TEMPLATE_SHORECLIFF33
		TEMPLATE_SHORECLIFF33,
		THEATERF_TEMPERATE|THEATERF_SNOW,
		"WC33",
		TXT_SHORE
	);
	(void)new TemplateTypeClass(				// TEMPLATE_SHORECLIFF34
		TEMPLATE_SHORECLIFF34,
		THEATERF_TEMPERATE|THEATERF_SNOW,
		"WC34",
		TXT_SHORE
	);
	(void)new TemplateTypeClass(				// TEMPLATE_SHORECLIFF35
		TEMPLATE_SHORECLIFF35,
		THEATERF_TEMPERATE|THEATERF_SNOW,
		"WC35",
		TXT_SHORE
	);
	(void)new TemplateTypeClass(				// TEMPLATE_SHORECLIFF36
		TEMPLATE_SHORECLIFF36,
		THEATERF_TEMPERATE|THEATERF_SNOW,
		"WC36",
		TXT_SHORE
	);
	(void)new TemplateTypeClass(				// TEMPLATE_SHORECLIFF37
		TEMPLATE_SHORECLIFF37,
		THEATERF_TEMPERATE|THEATERF_SNOW,
		"WC37",
		TXT_SHORE
	);
	(void)new TemplateTypeClass(				// TEMPLATE_SHORECLIFF38
		TEMPLATE_SHORECLIFF38,
		THEATERF_TEMPERATE|THEATERF_SNOW,
		"WC38",
		TXT_SHORE
	);
	(void)new TemplateTypeClass(				// TEMPLATE_BOULDER1
		TEMPLATE_BOULDER1,
		THEATERF_TEMPERATE|THEATERF_SNOW,
		"B1",
		TXT_SLOPE
	);
	(void)new TemplateTypeClass(				// TEMPLATE_BOULDER2
		TEMPLATE_BOULDER2,
		THEATERF_TEMPERATE|THEATERF_SNOW,
		"B2",
		TXT_SLOPE
	);
	(void)new TemplateTypeClass(				// TEMPLATE_BOULDER3
		TEMPLATE_BOULDER3,
		THEATERF_TEMPERATE|THEATERF_SNOW,
		"B3",
		TXT_SLOPE
	);
	(void)new TemplateTypeClass(				// TEMPLATE_BOULDER4
		TEMPLATE_BOULDER4,
		THEATERF_TEMPERATE|THEATERF_SNOW,
		"B4",
		TXT_SLOPE
	);
	(void)new TemplateTypeClass(				// TEMPLATE_BOULDER5
		TEMPLATE_BOULDER5,
		THEATERF_TEMPERATE|THEATERF_SNOW,
		"B5",
		TXT_SLOPE
	);
	(void)new TemplateTypeClass(				// TEMPLATE_BOULDER6
		TEMPLATE_BOULDER6,
		THEATERF_TEMPERATE|THEATERF_SNOW,
		"B6",
		TXT_SLOPE
	);
	(void)new TemplateTypeClass(				// TEMPLATE_PATCH01
		TEMPLATE_PATCH01,
		THEATERF_TEMPERATE|THEATERF_SNOW,
		"P01",
		TXT_PATCH
	);
	(void)new TemplateTypeClass(				// TEMPLATE_PATCH02
		TEMPLATE_PATCH02,
		THEATERF_TEMPERATE|THEATERF_SNOW,
		"P02",
		TXT_PATCH
	);
	(void)new TemplateTypeClass(				// TEMPLATE_PATCH03
		TEMPLATE_PATCH03,
		THEATERF_TEMPERATE|THEATERF_SNOW,
		"P03",
		TXT_PATCH
	);
	(void)new TemplateTypeClass(				// TEMPLATE_PATCH04
		TEMPLATE_PATCH04,
		THEATERF_TEMPERATE|THEATERF_SNOW,
		"P04",
		TXT_PATCH
	);
	(void)new TemplateTypeClass(				// TEMPLATE_PATCH07
		TEMPLATE_PATCH07,
		THEATERF_TEMPERATE|THEATERF_SNOW,
		"P07",
		TXT_PATCH
	);
	(void)new TemplateTypeClass(				// TEMPLATE_PATCH08
		TEMPLATE_PATCH08,
		THEATERF_TEMPERATE|THEATERF_SNOW,
		"P08",
		TXT_PATCH
	);
	(void)new TemplateTypeClass(				// TEMPLATE_PATCH13
		TEMPLATE_PATCH13,
		THEATERF_TEMPERATE|THEATERF_SNOW,
		"P13",
		TXT_PATCH
	);
	(void)new TemplateTypeClass(				// TEMPLATE_PATCH14
		TEMPLATE_PATCH14,
		THEATERF_TEMPERATE|THEATERF_SNOW,
		"P14",
		TXT_PATCH
	);
	(void)new TemplateTypeClass(				// TEMPLATE_PATCH15
		TEMPLATE_PATCH15,
		THEATERF_TEMPERATE|THEATERF_SNOW,
		"P15",
		TXT_PATCH
	);
	(void)new TemplateTypeClass(				// TEMPLATE_RIVER01
		TEMPLATE_RIVER01,
		THEATERF_TEMPERATE|THEATERF_SNOW,
		"RV01",
		TXT_RIVER
	);
	(void)new TemplateTypeClass(				// TEMPLATE_RIVER02
		TEMPLATE_RIVER02,
		THEATERF_TEMPERATE|THEATERF_SNOW,
		"RV02",
		TXT_RIVER
	);
	(void)new TemplateTypeClass(				// TEMPLATE_RIVER03
		TEMPLATE_RIVER03,
		THEATERF_TEMPERATE|THEATERF_SNOW,
		"RV03",
		TXT_RIVER
	);
	(void)new TemplateTypeClass(				// TEMPLATE_RIVER04
		TEMPLATE_RIVER04,
		THEATERF_TEMPERATE|THEATERF_SNOW,
		"RV04",
		TXT_RIVER
	);
	(void)new TemplateTypeClass(				// TEMPLATE_RIVER05
		TEMPLATE_RIVER05,
		THEATERF_TEMPERATE|THEATERF_SNOW,
		"RV05",
		TXT_RIVER
	);
	(void)new TemplateTypeClass(				// TEMPLATE_RIVER06
		TEMPLATE_RIVER06,
		THEATERF_TEMPERATE|THEATERF_SNOW,
		"RV06",
		TXT_RIVER
	);
	(void)new TemplateTypeClass(				// TEMPLATE_RIVER07
		TEMPLATE_RIVER07,
		THEATERF_TEMPERATE|THEATERF_SNOW,
		"RV07",
		TXT_RIVER
	);
	(void)new TemplateTypeClass(				// TEMPLATE_RIVER08
		TEMPLATE_RIVER08,
		THEATERF_TEMPERATE|THEATERF_SNOW,
		"RV08",
		TXT_RIVER
	);
	(void)new TemplateTypeClass(				// TEMPLATE_RIVER09
		TEMPLATE_RIVER09,
		THEATERF_TEMPERATE|THEATERF_SNOW,
		"RV09",
		TXT_RIVER
	);
	(void)new TemplateTypeClass(				// TEMPLATE_RIVER10
		TEMPLATE_RIVER10,
		THEATERF_TEMPERATE|THEATERF_SNOW,
		"RV10",
		TXT_RIVER
	);
	(void)new TemplateTypeClass(				// TEMPLATE_RIVER11
		TEMPLATE_RIVER11,
		THEATERF_TEMPERATE|THEATERF_SNOW,
		"RV11",
		TXT_RIVER
	);
	(void)new TemplateTypeClass(				// TEMPLATE_RIVER12
		TEMPLATE_RIVER12,
		THEATERF_TEMPERATE|THEATERF_SNOW,
		"RV12",
		TXT_RIVER
	);
	(void)new TemplateTypeClass(				// TEMPLATE_RIVER13
		TEMPLATE_RIVER13,
		THEATERF_TEMPERATE|THEATERF_SNOW,
		"RV13",
		TXT_RIVER
	);
	(void)new TemplateTypeClass(				// TEMPLATE_FALLS1
		TEMPLATE_FALLS1,
		THEATERF_TEMPERATE|THEATERF_SNOW,
		"FALLS1",
		TXT_RIVER
	);
	(void)new TemplateTypeClass(				// TEMPLATE_FALLS1A
		TEMPLATE_FALLS1A,
		THEATERF_TEMPERATE|THEATERF_SNOW,
		"FALLS1A",
		TXT_RIVER
	);
	(void)new TemplateTypeClass(				// TEMPLATE_FALLS2
		TEMPLATE_FALLS2,
		THEATERF_TEMPERATE|THEATERF_SNOW,
		"FALLS2",
		TXT_RIVER
	);
	(void)new TemplateTypeClass(				// TEMPLATE_FALLS2A
		TEMPLATE_FALLS2A,
		THEATERF_TEMPERATE|THEATERF_SNOW,
		"FALLS2A",
		TXT_RIVER
	);
	(void)new TemplateTypeClass(				// TEMPLATE_FORD1
		TEMPLATE_FORD1,
		THEATERF_TEMPERATE|THEATERF_SNOW,
		"FORD1",
		TXT_RIVER
	);
	(void)new TemplateTypeClass(				// TEMPLATE_FORD2
		TEMPLATE_FORD2,
		THEATERF_TEMPERATE|THEATERF_SNOW,
		"FORD2",
		TXT_RIVER
	);
	(void)new TemplateTypeClass(				// TEMPLATE_BRIDGE1
		TEMPLATE_BRIDGE1,
		THEATERF_TEMPERATE|THEATERF_SNOW,
		"BRIDGE1",
		TXT_BRIDGE
	);
	(void)new TemplateTypeClass(				// TEMPLATE_BRIDGE1D
		TEMPLATE_BRIDGE1D,
		THEATERF_TEMPERATE|THEATERF_SNOW,
		"BRIDGE1D",
		TXT_BRIDGE
	);
	(void)new TemplateTypeClass(				// TEMPLATE_BRIDGE2
		TEMPLATE_BRIDGE2,
		THEATERF_TEMPERATE|THEATERF_SNOW,
		"BRIDGE2",
		TXT_BRIDGE
	);
	(void)new TemplateTypeClass(				// TEMPLATE_BRIDGE2D
		TEMPLATE_BRIDGE2D,
		THEATERF_TEMPERATE|THEATERF_SNOW,
		"BRIDGE2D",
		TXT_BRIDGE
	);
	(void)new TemplateTypeClass(				// TEMPLATE_SLOPE01
		TEMPLATE_SLOPE01,
		THEATERF_TEMPERATE|THEATERF_SNOW,
		"S01",
		TXT_SLOPE
	);
	(void)new TemplateTypeClass(				// TEMPLATE_SLOPE02
		TEMPLATE_SLOPE02,
		THEATERF_TEMPERATE|THEATERF_SNOW,
		"S02",
		TXT_SLOPE
	);
	(void)new TemplateTypeClass(				// TEMPLATE_SLOPE03
		TEMPLATE_SLOPE03,
		THEATERF_TEMPERATE|THEATERF_SNOW,
		"S03",
		TXT_SLOPE
	);
	(void)new TemplateTypeClass(				// TEMPLATE_SLOPE04
		TEMPLATE_SLOPE04,
		THEATERF_TEMPERATE|THEATERF_SNOW,
		"S04",
		TXT_SLOPE
	);
	(void)new TemplateTypeClass(				// TEMPLATE_SLOPE05
		TEMPLATE_SLOPE05,
		THEATERF_TEMPERATE|THEATERF_SNOW,
		"S05",
		TXT_SLOPE
	);
	(void)new TemplateTypeClass(				// TEMPLATE_SLOPE06
		TEMPLATE_SLOPE06,
		THEATERF_TEMPERATE|THEATERF_SNOW,
		"S06",
		TXT_SLOPE
	);
	(void)new TemplateTypeClass(				// TEMPLATE_SLOPE07
		TEMPLATE_SLOPE07,
		THEATERF_TEMPERATE|THEATERF_SNOW,
		"S07",
		TXT_SLOPE
	);
	(void)new TemplateTypeClass(				// TEMPLATE_SLOPE08
		TEMPLATE_SLOPE08,
		THEATERF_TEMPERATE|THEATERF_SNOW,
		"S08",
		TXT_SLOPE
	);
	(void)new TemplateTypeClass(				// TEMPLATE_SLOPE09
		TEMPLATE_SLOPE09,
		THEATERF_TEMPERATE|THEATERF_SNOW,
		"S09",
		TXT_SLOPE
	);
	(void)new TemplateTypeClass(				// TEMPLATE_SLOPE10
		TEMPLATE_SLOPE10,
		THEATERF_TEMPERATE|THEATERF_SNOW,
		"S10",
		TXT_SLOPE
	);
	(void)new TemplateTypeClass(				// TEMPLATE_SLOPE11
		TEMPLATE_SLOPE11,
		THEATERF_TEMPERATE|THEATERF_SNOW,
		"S11",
		TXT_SLOPE
	);
	(void)new TemplateTypeClass(				// TEMPLATE_SLOPE12
		TEMPLATE_SLOPE12,
		THEATERF_TEMPERATE|THEATERF_SNOW,
		"S12",
		TXT_SLOPE
	);
	(void)new TemplateTypeClass(				// TEMPLATE_SLOPE13
		TEMPLATE_SLOPE13,
		THEATERF_TEMPERATE|THEATERF_SNOW,
		"S13",
		TXT_SLOPE
	);
	(void)new TemplateTypeClass(				// TEMPLATE_SLOPE14
		TEMPLATE_SLOPE14,
		THEATERF_TEMPERATE|THEATERF_SNOW,
		"S14",
		TXT_SLOPE
	);
	(void)new TemplateTypeClass(				// TEMPLATE_SLOPE15
		TEMPLATE_SLOPE15,
		THEATERF_TEMPERATE|THEATERF_SNOW,
		"S15",
		TXT_SLOPE
	);
	(void)new TemplateTypeClass(				// TEMPLATE_SLOPE16
		TEMPLATE_SLOPE16,
		THEATERF_TEMPERATE|THEATERF_SNOW,
		"S16",
		TXT_SLOPE
	);
	(void)new TemplateTypeClass(				// TEMPLATE_SLOPE17
		TEMPLATE_SLOPE17,
		THEATERF_TEMPERATE|THEATERF_SNOW,
		"S17",
		TXT_SLOPE
	);
	(void)new TemplateTypeClass(				// TEMPLATE_SLOPE18
		TEMPLATE_SLOPE18,
		THEATERF_TEMPERATE|THEATERF_SNOW,
		"S18",
		TXT_SLOPE
	);
	(void)new TemplateTypeClass(				// TEMPLATE_SLOPE19
		TEMPLATE_SLOPE19,
		THEATERF_TEMPERATE|THEATERF_SNOW,
		"S19",
		TXT_SLOPE
	);
	(void)new TemplateTypeClass(				// TEMPLATE_SLOPE20
		TEMPLATE_SLOPE20,
		THEATERF_TEMPERATE|THEATERF_SNOW,
		"S20",
		TXT_SLOPE
	);
	(void)new TemplateTypeClass(				// TEMPLATE_SLOPE21
		TEMPLATE_SLOPE21,
		THEATERF_TEMPERATE|THEATERF_SNOW,
		"S21",
		TXT_SLOPE
	);
	(void)new TemplateTypeClass(				// TEMPLATE_SLOPE22
		TEMPLATE_SLOPE22,
		THEATERF_TEMPERATE|THEATERF_SNOW,
		"S22",
		TXT_SLOPE
	);
	(void)new TemplateTypeClass(				// TEMPLATE_SLOPE23
		TEMPLATE_SLOPE23,
		THEATERF_TEMPERATE|THEATERF_SNOW,
		"S23",
		TXT_SLOPE
	);
	(void)new TemplateTypeClass(				// TEMPLATE_SLOPE24
		TEMPLATE_SLOPE24,
		THEATERF_TEMPERATE|THEATERF_SNOW,
		"S24",
		TXT_SLOPE
	);
	(void)new TemplateTypeClass(				// TEMPLATE_SLOPE25
		TEMPLATE_SLOPE25,
		THEATERF_TEMPERATE|THEATERF_SNOW,
		"S25",
		TXT_SLOPE
	);
	(void)new TemplateTypeClass(				// TEMPLATE_SLOPE26
		TEMPLATE_SLOPE26,
		THEATERF_TEMPERATE|THEATERF_SNOW,
		"S26",
		TXT_SLOPE
	);
	(void)new TemplateTypeClass(				// TEMPLATE_SLOPE27
		TEMPLATE_SLOPE27,
		THEATERF_TEMPERATE|THEATERF_SNOW,
		"S27",
		TXT_SLOPE
	);
	(void)new TemplateTypeClass(				// TEMPLATE_SLOPE28
		TEMPLATE_SLOPE28,
		THEATERF_TEMPERATE|THEATERF_SNOW,
		"S28",
		TXT_SLOPE
	);
	(void)new TemplateTypeClass(				// TEMPLATE_SLOPE29
		TEMPLATE_SLOPE29,
		THEATERF_TEMPERATE|THEATERF_SNOW,
		"S29",
		TXT_SLOPE
	);
	(void)new TemplateTypeClass(				// TEMPLATE_SLOPE30
		TEMPLATE_SLOPE30,
		THEATERF_TEMPERATE|THEATERF_SNOW,
		"S30",
		TXT_SLOPE
	);
	(void)new TemplateTypeClass(				// TEMPLATE_SLOPE31
		TEMPLATE_SLOPE31,
		THEATERF_TEMPERATE|THEATERF_SNOW,
		"S31",
		TXT_SLOPE
	);
	(void)new TemplateTypeClass(				// TEMPLATE_SLOPE32
		TEMPLATE_SLOPE32,
		THEATERF_TEMPERATE|THEATERF_SNOW,
		"S32",
		TXT_SLOPE
	);
	(void)new TemplateTypeClass(				// TEMPLATE_SLOPE33
		TEMPLATE_SLOPE33,
		THEATERF_TEMPERATE|THEATERF_SNOW,
		"S33",
		TXT_SLOPE
	);
	(void)new TemplateTypeClass(				// TEMPLATE_SLOPE34
		TEMPLATE_SLOPE34,
		THEATERF_TEMPERATE|THEATERF_SNOW,
		"S34",
		TXT_SLOPE
	);
	(void)new TemplateTypeClass(				// TEMPLATE_SLOPE35
		TEMPLATE_SLOPE35,
		THEATERF_TEMPERATE|THEATERF_SNOW,
		"S35",
		TXT_SLOPE
	);
	(void)new TemplateTypeClass(				// TEMPLATE_SLOPE36
		TEMPLATE_SLOPE36,
		THEATERF_TEMPERATE|THEATERF_SNOW,
		"S36",
		TXT_SLOPE
	);
	(void)new TemplateTypeClass(				// TEMPLATE_SLOPE37
		TEMPLATE_SLOPE37,
		THEATERF_TEMPERATE|THEATERF_SNOW,
		"S37",
		TXT_SLOPE
	);
	(void)new TemplateTypeClass(				// TEMPLATE_SLOPE38
		TEMPLATE_SLOPE38,
		THEATERF_TEMPERATE|THEATERF_SNOW,
		"S38",
		TXT_SLOPE
	);
	(void)new TemplateTypeClass(				// TEMPLATE_ROAD01
		TEMPLATE_ROAD01,
		THEATERF_TEMPERATE|THEATERF_SNOW,
		"D01",
		TXT_ROAD
	);
	(void)new TemplateTypeClass(				// TEMPLATE_ROAD02
		TEMPLATE_ROAD02,
		THEATERF_TEMPERATE|THEATERF_SNOW,
		"D02",
		TXT_ROAD
	);
	(void)new TemplateTypeClass(				// TEMPLATE_ROAD03
		TEMPLATE_ROAD03,
		THEATERF_TEMPERATE|THEATERF_SNOW,
		"D03",
		TXT_ROAD
	);
	(void)new TemplateTypeClass(				// TEMPLATE_ROAD04
		TEMPLATE_ROAD04,
		THEATERF_TEMPERATE|THEATERF_SNOW,
		"D04",
		TXT_ROAD
	);
	(void)new TemplateTypeClass(				// TEMPLATE_ROAD05
		TEMPLATE_ROAD05,
		THEATERF_TEMPERATE|THEATERF_SNOW,
		"D05",
		TXT_ROAD
	);
	(void)new TemplateTypeClass(				// TEMPLATE_ROAD06
		TEMPLATE_ROAD06,
		THEATERF_TEMPERATE|THEATERF_SNOW,
		"D06",
		TXT_ROAD
	);
	(void)new TemplateTypeClass(				// TEMPLATE_ROAD07
		TEMPLATE_ROAD07,
		THEATERF_TEMPERATE|THEATERF_SNOW,
		"D07",
		TXT_ROAD
	);
	(void)new TemplateTypeClass(				// TEMPLATE_ROAD08
		TEMPLATE_ROAD08,
		THEATERF_TEMPERATE|THEATERF_SNOW,
		"D08",
		TXT_ROAD
	);
	(void)new TemplateTypeClass(				// TEMPLATE_ROAD09
		TEMPLATE_ROAD09,
		THEATERF_TEMPERATE|THEATERF_SNOW,
		"D09",
		TXT_ROAD
	);
	(void)new TemplateTypeClass(				// TEMPLATE_ROAD10
		TEMPLATE_ROAD10,
		THEATERF_TEMPERATE|THEATERF_SNOW,
		"D10",
		TXT_ROAD
	);
	(void)new TemplateTypeClass(				// TEMPLATE_ROAD11
		TEMPLATE_ROAD11,
		THEATERF_TEMPERATE|THEATERF_SNOW,
		"D11",
		TXT_ROAD
	);
	(void)new TemplateTypeClass(				// TEMPLATE_ROAD12
		TEMPLATE_ROAD12,
		THEATERF_TEMPERATE|THEATERF_SNOW,
		"D12",
		TXT_ROAD
	);
	(void)new TemplateTypeClass(				// TEMPLATE_ROAD13
		TEMPLATE_ROAD13,
		THEATERF_TEMPERATE|THEATERF_SNOW,
		"D13",
		TXT_ROAD
	);
	(void)new TemplateTypeClass(				// TEMPLATE_ROAD14
		TEMPLATE_ROAD14,
		THEATERF_TEMPERATE|THEATERF_SNOW,
		"D14",
		TXT_ROAD
	);
	(void)new TemplateTypeClass(				// TEMPLATE_ROAD15
		TEMPLATE_ROAD15,
		THEATERF_TEMPERATE|THEATERF_SNOW,
		"D15",
		TXT_ROAD
	);
	(void)new TemplateTypeClass(				// TEMPLATE_ROAD16
		TEMPLATE_ROAD16,
		THEATERF_TEMPERATE|THEATERF_SNOW,
		"D16",
		TXT_ROAD
	);
	(void)new TemplateTypeClass(				// TEMPLATE_ROAD17
		TEMPLATE_ROAD17,
		THEATERF_TEMPERATE|THEATERF_SNOW,
		"D17",
		TXT_ROAD
	);
	(void)new TemplateTypeClass(				// TEMPLATE_ROAD18
		TEMPLATE_ROAD18,
		THEATERF_TEMPERATE|THEATERF_SNOW,
		"D18",
		TXT_ROAD
	);
	(void)new TemplateTypeClass(				// TEMPLATE_ROAD19
		TEMPLATE_ROAD19,
		THEATERF_TEMPERATE|THEATERF_SNOW,
		"D19",
		TXT_ROAD
	);
	(void)new TemplateTypeClass(				// TEMPLATE_ROAD20
		TEMPLATE_ROAD20,
		THEATERF_TEMPERATE|THEATERF_SNOW,
		"D20",
		TXT_ROAD
	);
	(void)new TemplateTypeClass(				// TEMPLATE_ROAD21
		TEMPLATE_ROAD21,
		THEATERF_TEMPERATE|THEATERF_SNOW,
		"D21",
		TXT_ROAD
	);
	(void)new TemplateTypeClass(				// TEMPLATE_ROAD22
		TEMPLATE_ROAD22,
		THEATERF_TEMPERATE|THEATERF_SNOW,
		"D22",
		TXT_ROAD
	);
	(void)new TemplateTypeClass(				// TEMPLATE_ROAD23
		TEMPLATE_ROAD23,
		THEATERF_TEMPERATE|THEATERF_SNOW,
		"D23",
		TXT_ROAD
	);
	(void)new TemplateTypeClass(				// TEMPLATE_ROAD24
		TEMPLATE_ROAD24,
		THEATERF_TEMPERATE|THEATERF_SNOW,
		"D24",
		TXT_ROAD
	);
	(void)new TemplateTypeClass(				// TEMPLATE_ROAD25
		TEMPLATE_ROAD25,
		THEATERF_TEMPERATE|THEATERF_SNOW,
		"D25",
		TXT_ROAD
	);
	(void)new TemplateTypeClass(				// TEMPLATE_ROAD26
		TEMPLATE_ROAD26,
		THEATERF_TEMPERATE|THEATERF_SNOW,
		"D26",
		TXT_ROAD
	);
	(void)new TemplateTypeClass(				// TEMPLATE_ROAD27
		TEMPLATE_ROAD27,
		THEATERF_TEMPERATE|THEATERF_SNOW,
		"D27",
		TXT_ROAD
	);
	(void)new TemplateTypeClass(				// TEMPLATE_ROAD28
		TEMPLATE_ROAD28,
		THEATERF_TEMPERATE|THEATERF_SNOW,
		"D28",
		TXT_ROAD
	);
	(void)new TemplateTypeClass(				// TEMPLATE_ROAD29
		TEMPLATE_ROAD29,
		THEATERF_TEMPERATE|THEATERF_SNOW,
		"D29",
		TXT_ROAD
	);
	(void)new TemplateTypeClass(				// TEMPLATE_ROAD30
		TEMPLATE_ROAD30,
		THEATERF_TEMPERATE|THEATERF_SNOW,
		"D30",
		TXT_ROAD
	);
	(void)new TemplateTypeClass(				// TEMPLATE_ROAD31
		TEMPLATE_ROAD31,
		THEATERF_TEMPERATE|THEATERF_SNOW,
		"D31",
		TXT_ROAD
	);
	(void)new TemplateTypeClass(				// TEMPLATE_ROAD32
		TEMPLATE_ROAD32,
		THEATERF_TEMPERATE|THEATERF_SNOW,
		"D32",
		TXT_ROAD
	);
	(void)new TemplateTypeClass(				// TEMPLATE_ROAD33
		TEMPLATE_ROAD33,
		THEATERF_TEMPERATE|THEATERF_SNOW,
		"D33",
		TXT_ROAD
	);
	(void)new TemplateTypeClass(				// TEMPLATE_ROAD34
		TEMPLATE_ROAD34,
		THEATERF_TEMPERATE|THEATERF_SNOW,
		"D34",
		TXT_ROAD
	);
	(void)new TemplateTypeClass(				// TEMPLATE_ROAD35
		TEMPLATE_ROAD35,
		THEATERF_TEMPERATE|THEATERF_SNOW,
		"D35",
		TXT_ROAD
	);
	(void)new TemplateTypeClass(				// TEMPLATE_ROAD36
		TEMPLATE_ROAD36,
		THEATERF_TEMPERATE|THEATERF_SNOW,
		"D36",
		TXT_ROAD
	);

	/*
	**	Separate out the list of new operator calls. Watcom bombs
	**	if they are kept together.
	*/
	_Watcom_Ugh_Hack();
}


/***********************************************************************************************
 * TemplateTypeClass::Land_Type -- Determines land type from template and icon number.         *
 *                                                                                             *
 *    This routine will convert the specified icon number into the appropriate land type. The  *
 *    land type can be determined from the embedded colors in the "control template" section   *
 *    of the original art file. This control information is encoded into the icon data file    *
 *    to be retrieved and interpreted as the program sees fit. The engine only recognizes      *
 *    the first 16 colors as control colors, so the control map color value serves as an       *
 *    index into a simple lookup table.                                                        *
 *                                                                                             *
 * INPUT:   icon  -- The icon number within this template that is to be examined and used      *
 *                   to determine the land type.                                               *
 *                                                                                             *
 * OUTPUT:  Returns with the land type that corresponds to the icon number specified.          *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   12/12/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
LandType TemplateTypeClass::Land_Type(int icon) const
{
	IconsetClass const * icontrol = (IconsetClass const *)Get_Image_Data();

	if (icontrol != NULL) {
		unsigned char const * map = icontrol->Control_Map();
		if (map != NULL) {
			static LandType _land[16] = {
				LAND_CLEAR,
				LAND_CLEAR,
				LAND_CLEAR,
				LAND_CLEAR,			// Clear
				LAND_CLEAR,
				LAND_CLEAR,
				LAND_BEACH,			// Beach
				LAND_CLEAR,
				LAND_ROCK,			// Rock
				LAND_ROAD,			// Road
				LAND_WATER,			// Water
				LAND_RIVER,			//	River
				LAND_CLEAR,
				LAND_CLEAR,
				LAND_ROUGH,			// Rough
				LAND_CLEAR,
			};

			return(_land[map[icon % (icontrol->Map_Width() * icontrol->Map_Height())]]);
		}
	}
	return(LAND_CLEAR);
}


/***********************************************************************************************
 * TemplateTypeClass::From_Name -- Determine template from ASCII name.                         *
 *                                                                                             *
 *    This routine is used to determine the template number given only                         *
 *    an ASCII representation. The scenario loader uses this routine                           *
 *    to construct the map from the INI control file.                                          *
 *                                                                                             *
 * INPUT:   name  -- Pointer to the ASCII name of the template.                                *
 *                                                                                             *
 * OUTPUT:  Returns with the template number. If the name had no match,                        *
 *          then returns with TEMPLATE_NONE.                                                   *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   05/23/1994 JLB : Created.                                                                 *
 *=============================================================================================*/
TemplateType TemplateTypeClass::From_Name(char const * name)
{
	if (name != NULL) {
		for (TemplateType index = TEMPLATE_FIRST; index < TEMPLATE_COUNT; index++) {
			if (stricmp(As_Reference(index).IniName, name) == 0) {
				return(index);
			}
		}
	}
	return(TEMPLATE_NONE);
}


/***********************************************************************************************
 * TemplateTypeClass::Occupy_List -- Determines occupation list.                               *
 *                                                                                             *
 *    This routine is used to examine the template map and build an                            *
 *    occupation list. This list is used to render a template cursor as                        *
 *    well as placement of icon numbers.                                                       *
 *                                                                                             *
 * INPUT:   placement   -- Is this for placement legality checking only? The normal condition  *
 *                         is for marking occupation flags.                                    *
 *                                                                                             *
 * OUTPUT:  Returns with a pointer to the template occupation list.                            *
 *                                                                                             *
 * WARNINGS:   The return pointer is valid only until the next time that                       *
 *             this routine is called.                                                         *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   05/23/1994 JLB : Created.                                                                 *
 *   12/12/1995 JLB : Optimized for direct access to iconset data.                             *
 *=============================================================================================*/
short const * TemplateTypeClass::Occupy_List(bool) const
{
	static short _occupy[13*8+5];
	short	* ptr;

	IconsetClass const * iconset = (IconsetClass const *)Get_Image_Data();
	unsigned char const * map = iconset->Map_Data();

	ptr = &_occupy[0];
	for (int index = 0; index < Width * Height; index++) {
		if (*map++ != 0xFF) {
			*ptr++ = (index % Width) + ((index / Width)*MAP_CELL_W);
		}
	}
	*ptr = REFRESH_EOL;

	return((short const *)&_occupy[0]);
}


/***********************************************************************************************
 * TemplateTypeClass::Init -- Loads graphic data for templates.                                *
 *                                                                                             *
 *    This routine loads the template graphic data for all the template                        *
 *    type supported for the specified theater. This routine is called                         *
 *    whenever the theater for the scenario is first determined.                               *
 *                                                                                             *
 * INPUT:   theater  -- The theater that the template data is to be                            *
 *                      loaded for.                                                            *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   This routine goes to disk!                                                      *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   05/23/1994 JLB : Created.                                                                 *
 *   06/02/1994 JLB : Only handles iconset loading now (as it should).                         *
 *=============================================================================================*/
void TemplateTypeClass::Init(TheaterType theater)
{
	char fullname[_MAX_FNAME+_MAX_EXT];	// Fully constructed iconset name.
	void const * ptr;		// Working loaded iconset pointer.

	for (TemplateType index = TEMPLATE_FIRST; index < TEMPLATE_COUNT; index++) {
		TemplateTypeClass	const & tplate = As_Reference(index);

		((void const *&)tplate.ImageData) = NULL;
		if (tplate.Theater & (1<<theater)) {
			_makepath(fullname, NULL, NULL, tplate.IniName, Theaters[theater].Suffix);
			ptr = MFCD::Retrieve(fullname);
			((void const *&)tplate.ImageData) = ptr;

#ifdef WIN32
			Register_Icon_Set((void*)ptr, TRUE);		//Register icon set for video memory caching
#endif

			((unsigned char &)tplate.Width) = Get_IconSet_MapWidth(ptr);
			((unsigned char &)tplate.Height) = Get_IconSet_MapHeight(ptr);
		}
	}
}


#ifdef SCENARIO_EDITOR
/***********************************************************************************************
 * TemplateTypeClass::Display -- Displays a generic representation of template.                *
 *                                                                                             *
 *    This routine is used to display a generic view of the template                           *
 *    object. This is necessary for selection in the scenario editor.                          *
 *                                                                                             *
 * INPUT:   x,y   -- The coordinates to center the display about.                              *
 *                                                                                             *
 *          window-- The window to base the coordinates upon.                                  *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   05/23/1994 JLB : Created.                                                                 *
 *=============================================================================================*/
void TemplateTypeClass::Display(int x, int y, WindowNumberType window, HousesType ) const
{
	int	w,h;
	int	index;
	bool	scale;		// Should the template be half sized?

	w = Bound(Width, 1, 13);
	h = Bound(Height, 1, 8);
	scale = (w > 3 || h > 3);
	if (scale) {
		x -= (w * ICON_PIXEL_W) / 4;
		y -= (h * ICON_PIXEL_H) / 4;
	} else {
		x -= (w * ICON_PIXEL_W) / 2;
		y -= (h * ICON_PIXEL_H) / 2;
	}
	x += WindowList[window][WINDOWX];
	y += WindowList[window][WINDOWY];

	IconsetClass const * iconset = (IconsetClass const *)Get_Image_Data();
	unsigned char const * map = iconset->Map_Data();

	for (index = 0; index < w*h; index++) {
		if (map[index] != 0xFF) {
			HidPage.Draw_Stamp(iconset, index, 0, 0, NULL, WINDOW_MAIN);
			if (scale) {

				HidPage.Scale((*LogicPage), 0, 0,
					x + ((index % w)*(ICON_PIXEL_W/2)),
					y + ((index / w)*(ICON_PIXEL_H/2)),
					ICON_PIXEL_W, ICON_PIXEL_H,
					ICON_PIXEL_W/2, ICON_PIXEL_H/2, (char *)NULL);

			} else {
				HidPage.Blit((*LogicPage), 0, 0, x + ((index % w)*(ICON_PIXEL_W)),
					y + ((index / w)*(ICON_PIXEL_H)), ICON_PIXEL_W, ICON_PIXEL_H);
			}
		}
	}
}


/***********************************************************************************************
 * TemplateTypeClass::Prep_For_Add -- Prepares to add template to scenario.                    *
 *                                                                                             *
 *    This routine prepares a list of template objects so that the                             *
 *    scenario editor can use this list to display a dialog box. The                           *
 *    selection of a template object will allow its placement upon the                         *
 *    map.                                                                                     *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   05/23/1994 JLB : Created.                                                                 *
 *   05/28/1994 JLB : Only handles real templates now.                                         *
 *   06/04/1994 JLB : Uses map editing interface functions.                                    *
 *=============================================================================================*/
void TemplateTypeClass::Prep_For_Add(void)
{
	for (TemplateType index = TEMPLATE_CLEAR1; index < TEMPLATE_COUNT; index++) {
		if (As_Reference(index).Get_Image_Data()) {
			Map.Add_To_List(&As_Reference(index));
		}
	}
}
#endif


/***********************************************************************************************
 * TemplateTypeClass::Create_And_Place -- Creates and places a template object on the map.     *
 *                                                                                             *
 *    This support routine is used by the scenario editor to add a template object to the map  *
 *    and to the game.                                                                         *
 *                                                                                             *
 * INPUT:   cell  -- The cell to place the template object.                                    *
 *                                                                                             *
 * OUTPUT:  bool; Was the template object placed successfully?                                 *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   05/28/1994 JLB : Created.                                                                 *
 *=============================================================================================*/
bool TemplateTypeClass::Create_And_Place(CELL cell, HousesType ) const
{
	if (new TemplateClass(Type, cell)) {
		return(true);
	}
	return(false);
}


/***********************************************************************************************
 * TemplateTypeClass::Create_One_Of -- Creates an object of this template type.                *
 *                                                                                             *
 *    This routine will create an object of this type. For certain template objects, such      *
 *    as walls, it is actually created as a building. The "building" wall is converted into    *
 *    a template at the moment of placing down on the map.                                     *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  Returns with a pointer to the appropriate object for this template type.           *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   06/18/1994 JLB : Created.                                                                 *
 *=============================================================================================*/
ObjectClass * TemplateTypeClass::Create_One_Of(HouseClass *) const
{
	return(new TemplateClass(Type, -1));
}


/***********************************************************************************************
 * TemplateTypeClass::One_Time -- Performs one-time initialization                             *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   08/12/1994 JLB : Created.                                                                 *
 *=============================================================================================*/
void TemplateTypeClass::One_Time(void)
{
}


/***********************************************************************************************
 * TemplateTypeClass::As_Reference -- Fetches a reference to the template specified.           *
 *                                                                                             *
 *    This will return a reference to the TemplateTypeClass requested.                         *
 *                                                                                             *
 * INPUT:   type  -- The template type to fetch a reference to.                                *
 *                                                                                             *
 * OUTPUT:  Returns with a reference to the template type class specified.                     *
 *                                                                                             *
 * WARNINGS:   Be sure to pass a valid type parameter. This routine doesn't check it for       *
 *             legality.                                                                       *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   07/03/1996 JLB : Created.                                                                 *
 *=============================================================================================*/
TemplateTypeClass & TemplateTypeClass::As_Reference(TemplateType type)
{
	return(*TemplateTypes.Ptr(type));
}


COORDINATE TemplateTypeClass::Coord_Fixup(COORDINATE coord) const
{
	return Coord_Whole(coord);
}
