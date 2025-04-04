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

/* $Header:   F:\projects\c&c\vcs\code\facing.h_v   1.14   16 Oct 1995 16:46:02   JOE_BOSTIC  $ */
/*********************************************************************************************** 
 ***              C O N F I D E N T I A L  ---  W E S T W O O D  S T U D I O S               *** 
 *********************************************************************************************** 
 *                                                                                             * 
 *                 Project Name : Command & Conquer                                            * 
 *                                                                                             * 
 *                    File Name : FACING.H                                                     * 
 *                                                                                             * 
 *                   Programmer : Joe L. Bostic                                                * 
 *                                                                                             * 
 *                   Start Date : 03/21/95                                                     * 
 *                                                                                             * 
 *                  Last Update : March 21, 1995 [JLB]                                         * 
 *                                                                                             * 
 *---------------------------------------------------------------------------------------------* 
 * Functions:                                                                                  * 
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#ifndef FACING_H
#define FACING_H

/*
**	This is a general facing handler class. It is used in those cases where facing needs to be
**	kept track of, but there could also be an associated desired facing. The current facing
**	is supposed to transition to the desired state over time. Using this class facilitates this
**	processing as well as isolating the rest of the code from the internals.
*/
class FacingClass 
{
	public:
		FacingClass(void);
		FacingClass(DirType dir) {CurrentFacing = DesiredFacing = dir;};
		FacingClass(NoInitClass const & ) {};

		operator DirType(void) const {return CurrentFacing;};

		DirType Current(void) const {return CurrentFacing;};
		DirType Desired(void) const {return DesiredFacing;};

		int Set_Desired(DirType facing);
		int Set_Current(DirType facing);

		void Set(DirType facing) {
			Set_Current(facing);
			Set_Desired(facing);
		};

		DirType Get(void) const { return CurrentFacing; }

		int Is_Rotating(void) const {return (DesiredFacing != CurrentFacing);};

		int Difference(void) const {return (signed char)(*((unsigned char*)&DesiredFacing) - *((unsigned char*)&CurrentFacing));};
		int Difference(DirType facing) const {return (signed char)(*((signed char*)&facing) - *((signed char*)&CurrentFacing));};
		int Rotation_Adjust(int rate);

	private:
		DirType CurrentFacing;
		DirType DesiredFacing;
};


#endif
