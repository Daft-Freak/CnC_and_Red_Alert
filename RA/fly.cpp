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

/* $Header: /CounterStrike/FLY.CPP 1     3/03/97 10:24a Joe_bostic $ */
/***********************************************************************************************
 ***              C O N F I D E N T I A L  ---  W E S T W O O D  S T U D I O S               ***
 ***********************************************************************************************
 *                                                                                             *
 *                 Project Name : Command & Conquer                                            *
 *                                                                                             *
 *                    File Name : FLY.CPP                                                      *
 *                                                                                             *
 *                   Programmer : Joe L. Bostic                                                *
 *                                                                                             *
 *                   Start Date : April 24, 1994                                               *
 *                                                                                             *
 *                  Last Update : June 5, 1995 [JLB]                                           *
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 *   FlyClass::Fly_Speed -- Sets the flying object to the speed specified.                     *
 *   FlyClass::Physics -- Performs vector physics (movement).                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include	"function.h"


/***********************************************************************************************
 * FlyClass::Physics -- Performs vector physics (movement).                                    *
 *                                                                                             *
 *    This routine performs movement (vector) physics. It takes the                            *
 *    specified location and moves it according to the facing and speed                        *
 *    of the vector. It returns the status of the move.                                        *
 *                                                                                             *
 * INPUT:   coord -- Reference to the coordinate that the vector will                          *
 *                   be applied to.                                                            *
 *                                                                                             *
 * OUTPUT:  Returns with the status of the vector physics. This could                          *
 *          range from no effect, to exiting the edge of the world.                            *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   04/24/1994 JLB : Created.                                                                 *
 *   06/05/1995 JLB : Simplified to just do movement.                                          *
 *=============================================================================================*/
ImpactType FlyClass::Physics(COORDINATE & coord, DirType facing)
{
	if (SpeedAdd != MPH_IMMOBILE) {
		int actual = (int)SpeedAdd + SpeedAccum;
		div_t result = div(actual, PIXEL_LEPTON_W);
		SpeedAccum = result.rem;
		actual -= result.rem;
		COORDINATE old = coord;

		/*
		**	If movement occurred that is at least one
		**	pixel, then check update the coordinate and
		**	check for edge of world collision.
		*/
		if (result.quot) {
			COORDINATE		newcoord;		// New working coordinate.
			newcoord = Coord_Move(coord, facing, actual);
			/*
			**	If no movement occurred, then presume it hasn't moved at all
			**	and return immediately with this indication.
			*/
			if (newcoord == coord) {
				return(IMPACT_NONE);
			}

			/*
			**	Remember the new position.
			*/
			coord = newcoord;

			/*
			**	If the new coordinate is off the edge of the world, then report
			**	this.
			*/
			if (newcoord & HIGH_COORD_MASK /*|| !Map.In_Radar(Coord_Cell(newcoord))*/) {
//			if (!Map.In_Radar(Coord_Cell(newcoord))) {
				coord = old;
				return(IMPACT_EDGE);
			}

			return(IMPACT_NORMAL);
		}
	}
	return(IMPACT_NONE);
}


/***********************************************************************************************
 * FlyClass::Fly_Speed -- Sets the flying object to the speed specified.                       *
 *                                                                                             *
 *    This sets the speed of the projectile. It basically functions like a throttle value      *
 *    where 0 equals stop and 255 equals maximum speed (whatever that is for the particular    *
 *    object).                                                                                 *
 *                                                                                             *
 * INPUT:   speed -- Speed setting from 0 to 255.                                              *
 *                                                                                             *
 *          maximum  -- The maximum speed of the object.                                       *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   07/26/1994 JLB : Created.                                                                 *
 *   07/26/1994 JLB : Added maximum speed as guiding value.                                    *
 *=============================================================================================*/
void FlyClass::Fly_Speed(int speed, MPHType maximum)
{
	SpeedAdd = (MPHType)( maximum * fixed(speed, 256));
//	SpeedAdd = (MPHType)Fixed_To_Cardinal((int)maximum, speed);
}


