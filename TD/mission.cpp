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

/* $Header:   F:\projects\c&c\vcs\code\mission.cpv   2.18   16 Oct 1995 16:49:12   JOE_BOSTIC  $ */
/***********************************************************************************************
 ***             C O N F I D E N T I A L  ---  W E S T W O O D   S T U D I O S               *** 
 ***********************************************************************************************
 *                                                                                             *
 *                 Project Name : Command & Conquer                                            *
 *                                                                                             *
 *                    File Name : MISSION.CPP                                                  *
 *                                                                                             *
 *                   Programmer : Joe L. Bostic                                                *
 *                                                                                             *
 *                   Start Date : April 23, 1994                                               *
 *                                                                                             *
 *                  Last Update : June 25, 1995 [JLB]                                          *
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 *   MissionClass::AI -- Processes order script.                                               *
 *   MissionClass::Assign_Mission -- Give an order to a unit.                                  *
 *   MissionClass::Commence -- Start script with new order.                                    *
 *   MissionClass::Debug_Dump -- Dumps status values to mono screen.                           *
 *   MissionClass::Get_Mission -- Fetches the mission that this object is acting under.        * 
 *   MissionClass::MissionClass -- Default constructor for the mission object type.            * 
 *   MissionClass::Mission_From_Name -- Fetch order pointer from its name.                     *
 *   MissionClass::Mission_Name -- Converts a mission number into an ASCII string.             * 
 *   MissionClass::Overide_Mission -- temporarily overides the units mission                   *
 *   MissionClass::Restore_Mission -- Restores overidden mission                               *
 *   MissionClass::Set_Mission -- Sets the mission to the specified value.                     * 
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include	"function.h"


/*********************************************************************************************** 
 * MissionClass::MissionClass -- Default constructor for the mission object type.              * 
 *                                                                                             * 
 *    This is the default constructor for the mission class object. It sets the mission        * 
 *    handler into a default -- do nothing -- state.                                           * 
 *                                                                                             * 
 * INPUT:   none                                                                               * 
 *                                                                                             * 
 * OUTPUT:  none                                                                               * 
 *                                                                                             * 
 * WARNINGS:   none                                                                            * 
 *                                                                                             * 
 * HISTORY:                                                                                    * 
 *   01/23/1995 JLB : Created.                                                                 * 
 *=============================================================================================*/
MissionClass::MissionClass(void) 
{
	Status = 0;
	Timer = 0;
	Mission = MISSION_NONE;
	SuspendedMission = MISSION_NONE;
	MissionQueue = MISSION_NONE;
}


int MissionClass::Mission_Sleep(void) {return TICKS_PER_SECOND*30;};
int MissionClass::Mission_Ambush(void) {return TICKS_PER_SECOND*30;};
int MissionClass::Mission_Attack(void) {return TICKS_PER_SECOND*30;};
int MissionClass::Mission_Capture(void) {return TICKS_PER_SECOND*30;};
int MissionClass::Mission_Guard(void) {return TICKS_PER_SECOND*30;};
int MissionClass::Mission_Guard_Area(void) {return TICKS_PER_SECOND*30;};
int MissionClass::Mission_Harvest(void) {return TICKS_PER_SECOND*30;};
int MissionClass::Mission_Hunt(void) {return TICKS_PER_SECOND*30;};
int MissionClass::Mission_Timed_Hunt(void) {return TICKS_PER_SECOND*30;};
int MissionClass::Mission_Move(void) {return TICKS_PER_SECOND*30;};
int MissionClass::Mission_Retreat(void) {return TICKS_PER_SECOND*30;};
int MissionClass::Mission_Return(void) {return TICKS_PER_SECOND*30;};
int MissionClass::Mission_Stop(void) {return TICKS_PER_SECOND*30;};
int MissionClass::Mission_Unload(void) {return TICKS_PER_SECOND*30;};
int MissionClass::Mission_Enter(void) {return TICKS_PER_SECOND*30;};
int MissionClass::Mission_Construction(void) {return TICKS_PER_SECOND*30;};
int MissionClass::Mission_Deconstruction(void) {return TICKS_PER_SECOND*30;};
int MissionClass::Mission_Repair(void) {return TICKS_PER_SECOND*30;};
int MissionClass::Mission_Missile(void) {return TICKS_PER_SECOND*30;};


/*********************************************************************************************** 
 * MissionClass::Set_Mission -- Sets the mission to the specified value.                       * 
 *                                                                                             * 
 *    Use this routine to set the current mission for this object. This routine will blast     * 
 *    over the current mission, bypassing the queue method. Call it when the mission needs     * 
 *    to be changed immediately.                                                               * 
 *                                                                                             * 
 * INPUT:   mission  -- The mission to set to.                                                 * 
 *                                                                                             * 
 * OUTPUT:  none                                                                               * 
 *                                                                                             * 
 * WARNINGS:   none                                                                            * 
 *                                                                                             * 
 * HISTORY:                                                                                    * 
 *   01/23/1995 JLB : Created.                                                                 * 
 *=============================================================================================*/
void MissionClass::Set_Mission(MissionType mission)
{
	Mission = mission; 
	MissionQueue = MISSION_NONE;
}


/*********************************************************************************************** 
 * MissionClass::Get_Mission -- Fetches the mission that this object is acting under.          * 
 *                                                                                             * 
 *    Use this routine to fetch the mission that this object is CURRENTLY acting under. The    * 
 *    mission queue may be filled with a immanent mission change, but this routine does not    * 
 *    consider that. It only returns the CURRENT mission.                                      * 
 *                                                                                             * 
 * INPUT:   none                                                                               * 
 *                                                                                             * 
 * OUTPUT:  Returns with the mission that this unit is currently following.                    * 
 *                                                                                             * 
 * WARNINGS:   none                                                                            * 
 *                                                                                             * 
 * HISTORY:                                                                                    * 
 *   01/23/1995 JLB : Created.                                                                 * 
 *=============================================================================================*/
MissionType MissionClass::Get_Mission(void) const
{
	return(Mission == MISSION_NONE ? MissionQueue : Mission);
}


#ifdef CHEAT_KEYS
/***********************************************************************************************
 * MissionClass::Debug_Dump -- Dumps status values to mono screen.                             *
 *                                                                                             *
 *    This is a debugging function that dumps this class' status to the monochrome screen      *
 *    for review.                                                                              *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   05/28/1994 JLB : Created.                                                                 *
 *=============================================================================================*/
void MissionClass::Debug_Dump(MonoClass *mono) const
{
	mono->Set_Cursor(21, 1);mono->Printf("%5.5s[%4.4s]", MissionClass::Mission_Name(Mission), MissionClass::Mission_Name(MissionQueue));
//	mono->Text_Print(MissionClass::Mission_Name(Mission), 21, 1);
	mono->Set_Cursor(20, 7);mono->Printf("%2d", (long)Timer);
	mono->Set_Cursor(74, 1);mono->Printf("%2d", Status);

	ObjectClass::Debug_Dump(mono);
}
#endif


/***********************************************************************************************
 * MissionClass::AI -- Processes order script.                                                 *
 *                                                                                             *
 *    This routine will process the order script for as much time as                           *
 *    possible or until a script delay is detected. This routine should                        *
 *    be called for every unit once per game loop (if possible).                               *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   04/23/1994 JLB : Created.                                                                 *
 *   06/25/1995 JLB : Added new missions.                                                      * 
 *=============================================================================================*/
void MissionClass::AI(void)
{
	ObjectClass::AI();

	/*
	**	This is the script AI equivalent processing.
	*/
	if (Timer.Expired() && Strength > 0) {
		switch (Mission) {
			default:
			case MISSION_STICKY:
			case MISSION_SLEEP:
				Timer = Mission_Sleep();
				break;

			case MISSION_GUARD:
				Timer = Mission_Guard();
				break;

			case MISSION_ENTER:
				Timer = Mission_Enter();
				break;

			case MISSION_CONSTRUCTION:
				Timer = Mission_Construction();
				break;

			case MISSION_DECONSTRUCTION:
				Timer = Mission_Deconstruction();
				break;

			case MISSION_CAPTURE:
			case MISSION_SABOTAGE:
				Timer = Mission_Capture();
				break;

			case MISSION_MOVE:
				Timer = Mission_Move();
				break;

			case MISSION_ATTACK:
				Timer = Mission_Attack();
				break;
			
			case MISSION_RETREAT:
				Timer = Mission_Retreat();
				break;
			
			case MISSION_HARVEST:
				Timer = Mission_Harvest();
				break;
			
			case MISSION_GUARD_AREA:
				Timer = Mission_Guard_Area();
				break;
			
			case MISSION_RETURN:
				Timer = Mission_Return();
				break;

			case MISSION_STOP:
				Timer = Mission_Stop();
				break;
			
			case MISSION_AMBUSH:
				Timer = Mission_Ambush();
				break;
			
			case MISSION_HUNT:
			case MISSION_RESCUE:
				Timer = Mission_Hunt();
				break;

			case MISSION_TIMED_HUNT:
				Timer = Mission_Timed_Hunt();
				break;

			case MISSION_UNLOAD:
				Timer = Mission_Unload();
				break;

			case MISSION_REPAIR:
				Timer = Mission_Repair();
				break;

			case MISSION_MISSILE:
				Timer = Mission_Missile();
				break;
		}
	}
}


/***********************************************************************************************
 * MissionClass::Commence -- Start script with new order.                                      *
 *                                                                                             *
 *    This routine will start script processing according to any queued                        *
 *    order it may have. If there is no queued order, then this routine                        *
 *    does nothing. Call this routine whenever the unit is in a good                           *
 *    position to change its order (such as when it is stopped).                               *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  Did the mission actually change?                                                   *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   04/23/1994 JLB : Created.                                                                 *
 *   07/14/1994 JLB : Simplified.                                                              *
 *   06/17/1995 JLB : Returns success flag.                                                    * 
 *=============================================================================================*/
bool MissionClass::Commence(void)
{
	if (MissionQueue != MISSION_NONE) {
		Mission = MissionQueue;
		MissionQueue = MISSION_NONE;

		/*
		**	Force immediate state machine processing at the first state machine state value.
		*/
		Timer = 0;
		Status = 0;
		return(true);
	}
	return(false);
}


/***********************************************************************************************
 * MissionClass::Assign_Mission -- Give an order to a unit.                                    *
 *                                                                                             *
 *    This assigns an order to a unit. It does NOT do the AI, but merely                       *
 *    flags the unit for normal AI processing. At that time the computer                       *
 *    will determine the unit's course of action.                                              *
 *                                                                                             *
 * INPUT:   order -- Mission to give the unit.                                                 *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   06/04/1991 JLB : Created.                                                                 *
 *   04/15/1994 JLB : Converted to member function.                                            *
 *=============================================================================================*/
void MissionClass::Assign_Mission(MissionType order)
{
	if (order == MISSION_NONE || Mission == order) return;

//	Status = 0;
	MissionQueue = order;
}

  
/***********************************************************************************************
 * MissionClass::Mission_From_Name -- Fetch order pointer from its name.                       *
 *                                                                                             *
 *    This routine is used to convert an ASCII order name into the actual                      *
 *    order number it represents. Typically, this is used when processing                      *
 *    a scenario INI file.                                                                     *
 *                                                                                             *
 * INPUT:   name  -- The ASCII order name to process.                                          *
 *                                                                                             *
 * OUTPUT:  Returns with the actual order number that the ASCII name                           *
 *          represents.                                                                        *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   10/07/1992 JLB : Created.                                                                 *
 *   04/22/1994 JLB : Converted to static member function.                                     *
 *=============================================================================================*/
MissionType MissionClass::Mission_From_Name(char const *name)
{
	MissionType	order;

	if (name) {
		for (order = MISSION_FIRST; order < MISSION_COUNT; order++) {
			if (stricmp(Missions[order], name) == 0) {
				return(order);
			}
		}
	}
	return(MISSION_NONE);
}


/*********************************************************************************************** 
 * MissionClass::Mission_Name -- Converts a mission number into an ASCII string.               * 
 *                                                                                             * 
 *    Use this routine to convert a mission number into the ASCII string that represents       * 
 *    it. Typical use of this is when generating an INI file.                                  * 
 *                                                                                             * 
 * INPUT:   mission  -- The mission number to convert.                                         * 
 *                                                                                             * 
 * OUTPUT:  Returns with a pointer to the ASCII string that represents the mission type.       * 
 *                                                                                             * 
 * WARNINGS:   none                                                                            * 
 *                                                                                             * 
 * HISTORY:                                                                                    * 
 *   01/23/1995 JLB : Created.                                                                 * 
 *=============================================================================================*/
char const * MissionClass::Mission_Name(MissionType mission)
{
	return(mission == MISSION_NONE ? "None" : Missions[mission]);
}


/***********************************************************************************************
 * MissionClass::Override_Mission -- temporarily overides the units mission                    *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * INPUT:      MissionType mission - the mission we want to overide                            *
 *               TARGET      tarcom  - the new target we want to overide                       *
 *               TARGET      navcom  - the new navigation point to overide                     *
 *                                                                                             *
 * OUTPUT:      none                                                                           *
 *                                                                                             *
 * WARNINGS:   If a mission is already overidden, the current mission is                       *
 *               just re-assigned.                                                             *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   04/28/1995 PWG : Created.                                                                 *
 *=============================================================================================*/
void MissionClass::Override_Mission(MissionType mission, TARGET, TARGET)
{
	if (MissionQueue != MISSION_NONE) {
		SuspendedMission = MissionQueue;
	} else {
		SuspendedMission = Mission;
	}

	Assign_Mission(mission);
}


/***********************************************************************************************
 * MissionClass::Restore_Mission -- Restores overidden mission                                 *
 *                                                                                             *
 * INPUT:      none                                                                            *
 *                                                                                             *
 * OUTPUT:     none                                                                            *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   04/28/1995 PWG : Created.                                                                 *
 *=============================================================================================*/
bool MissionClass::Restore_Mission(void)
{
	if (SuspendedMission != MISSION_NONE) {
		Assign_Mission(SuspendedMission);
	 	SuspendedMission= MISSION_NONE;
		return(true);
	}
	return(false);
}


/***********************************************************************************************
**	Unit order names. These names correspond to the player selectable orders
**	a unit can have. The system initiated orders have no use for the ASCII name
**	associated, but they are listed here for completeness sake.
*/
char const * MissionClass::Missions[MISSION_COUNT] = {
	"Sleep",
	"Attack",
	"Move",
	"Retreat",
	"Guard",
	"Sticky",
	"Enter",
	"Capture",
	"Harvest",
	"Area Guard",
	"Return",
	"Stop",
	"Ambush",
	"Hunt",
	"Timed Hunt",
	"Unload",
	"Sabotage",
	"Construction",
	"Selling",
	"Repair",
	"Rescue",
	"Missile",
};
