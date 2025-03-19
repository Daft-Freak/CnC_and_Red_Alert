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

/* $Header: /CounterStrike/ADATA.CPP 3     3/07/97 4:27p Joe_bostic $ */
/***********************************************************************************************
 ***              C O N F I D E N T I A L  ---  W E S T W O O D  S T U D I O S               ***
 ***********************************************************************************************
 *                                                                                             *
 *                 Project Name : Command & Conquer                                            *
 *                                                                                             *
 *                    File Name : ADATA.CPP                                                    *
 *                                                                                             *
 *                   Programmer : Joe L. Bostic                                                *
 *                                                                                             *
 *                   Start Date : May 30, 1994                                                 *
 *                                                                                             *
 *                  Last Update : July 9, 1996 [JLB]                                           *
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 *   AnimTypeClass::AnimTypeClass -- Constructor for animation types.                          *
 *   AnimTypeClass::One_Time -- Performs one time action for animation types.                  *
 *   AnimTypeClass::Init -- Load any animation artwork that is theater specific.               *
 *   Anim_Name -- Fetches the ASCII name of the animation type specified.                      *
 *   AnimTypeClass::As_Reference -- Fetch a reference to the animation type specified.         *
 *   AnimTypeClass::Init_Heap -- Initialize the animation type system.                         *
 *   AnimTypeClass::operator new -- Allocate an animation type object from private pool.       *
 *   AnimTypeClass::operator delete -- Returns an anim type class object back to the pool.     *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include	"function.h"

/***********************************************************************************************
 * AnimTypeClass::AnimTypeClass -- Constructor for animation types.                            *
 *                                                                                             *
 *    This is the constructor for static objects that elaborate the various animation types    *
 *    allowed in the game. Each animation in the game is of one of these types.                *
 *                                                                                             *
 * INPUT:   see below...                                                                       *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   08/23/1994 JLB : Created.                                                                 *
 *=============================================================================================*/
AnimTypeClass::AnimTypeClass(
		AnimType anim,
		char const * name,
		int size,
		int biggest,
		bool istheater,
		bool isnormal,
		bool iswhitetrans,
		bool isscorcher,
		bool iscrater,
		bool issticky,
		bool ground,
		bool istrans,
		bool isflame,
		fixed damage,
		int delaytime,
		int start,
		int loopstart,
		int loopend,
		int stages,
		int loops,
		VocType soundid,
		AnimType chainto) :
	ObjectTypeClass(RTTI_ANIMTYPE,
			int(anim),
			true,
			true,
			false,
			false,
			true,
			true,
			false,
			TXT_NONE,
			name
			),
	IsNormalized(isnormal),
	IsGroundLayer(ground),
	IsTranslucent(istrans),
	IsWhiteTrans(iswhitetrans),
	IsFlameThrower(isflame),
	IsScorcher(isscorcher),
	IsCraterForming(iscrater),
	IsSticky(issticky),
	IsTheater(istheater),
	Type(anim),
	Size(size),
	Biggest(biggest),
	Damage(damage),
	Delay(delaytime),
	Start(start),
	LoopStart(loopstart),
	LoopEnd(loopend),
	Stages(stages),
	Loops(loops),
	Sound(soundid),
	ChainTo(chainto)
{
}


/***********************************************************************************************
 * AnimTypeClass::operator new -- Allocate an animation type object from private pool.         *
 *                                                                                             *
 *    This routine will allocate an animation type class object.                               *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  Returns with a pointer to the newly allocated anim type object. If no anim type    *
 *          could be allocated, then NULL is returned.                                         *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   07/09/1996 JLB : Created.                                                                 *
 *=============================================================================================*/
void * AnimTypeClass::operator new(size_t) throw()
{
	return(AnimTypes.Alloc());
}


/***********************************************************************************************
 * AnimTypeClass::operator delete -- Returns an anim type class object back to the pool.       *
 *                                                                                             *
 *    This will return the anim type class object back to the memory pool from whence it was   *
 *    previously allocated.                                                                    *
 *                                                                                             *
 * INPUT:   pointer  -- Pointer to the anim type class object to return to the memory pool.    *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   07/09/1996 JLB : Created.                                                                 *
 *=============================================================================================*/
void AnimTypeClass::operator delete(void * pointer)
{
	AnimTypes.Free((AnimTypeClass *)pointer);
}


/***********************************************************************************************
 * AnimTypeClass::Init_Heap -- Initialize the animation type system.                           *
 *                                                                                             *
 *    This routine is called to initialize the animation type class heap. It allocates all     *
 *    known animation types.                                                                   *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   07/09/1996 JLB : Created.                                                                 *
 *=============================================================================================*/
void AnimTypeClass::Init_Heap(void)
{
	/*
	**	These anim type class objects must be allocated in the exact order that they
	**	are specified in the AnimType enumeration. This is necessary because the heap
	**	allocation block index serves double duty as the type number index.
	*/
	new AnimTypeClass( // FBall1
		ANIM_FBALL1,							// Animation number.
		"FBALL1",								// Data name of animation.
		67,										// Maximum dimension of animation.
		6,											// Biggest animation stage.
		false,									// Theater specific art imagery?
		true,										// Normalized animation rate?
		false,									// Uses white translucent table?
		false,									// Scorches the ground?
		true,										// Forms a crater?
		false,									// Sticks to unit in square?
		false,									// Ground level animation?
		false,									// Translucent colors in this animation?
		false,									// Is this a flame thrower animation?
		0,											// Damage to apply per tick (fixed point).
		1,											// Delay between frames.
		0,											// Starting frame number.
		0,											// Loop start frame number.
		-1,										// Ending frame of loop back.
		-1,										// Number of animation stages.
		1,											// Number of times the animation loops.
		VOC_KABOOM25,							// Sound effect to play.
		ANIM_NONE
	);
	new AnimTypeClass( // FireBallFade
		ANIM_FBALL_FADE,						// Animation number.
		"FB2",									// Data name of animation.
		24,										// Maximum dimension of animation.
		1,											// Biggest animation stage.
		false,									// Theater specific art imagery?
		true,										// Normalized animation rate?
		false,									// Uses white translucent table?
		false,									// Scorches the ground?
		false,									// Forms a crater?
		false,									// Sticks to unit in square?
		false,									// Ground level animation?
		false,									// Translucent colors in this animation?
		false,									// Is this a flame thrower animation?
		0,											// Damage to apply per tick (fixed point).
		1,											// Delay between frames.
		0,											// Starting frame number.
		0,											// Loop start frame number.
		-1,										// Ending frame of loop back.
		-1,										// Number of animation stages.
		1,											// Number of times the animation loops.
		VOC_NONE,								// Sound effect to play.
		ANIM_NONE
	);
	new AnimTypeClass( // Frag1
		ANIM_FRAG1,								// Animation number.
		"FRAG1",									// Data name of animation.
		45,										// Maximum dimension of animation.
		3,											// Biggest animation stage.
		false,									// Theater specific art imagery?
		true,										// Normalized animation rate?
		false,									// Uses white translucent table?
		false,									// Scorches the ground?
		true,										// Forms a crater?
		true,										// Sticks to unit in square?
		true,										// Ground level animation?
		false,									// Translucent colors in this animation?
		false,									// Is this a flame thrower animation?
		0,											// Damage to apply per tick (fixed point).
		1,											// Delay between frames.
		0,											// Starting frame number.
		0,											// Loop start frame number.
		-1,										// Ending frame of loop back.
		-1,										// Number of animation stages.
		1,											// Number of times the animation loops.
		VOC_KABOOM30,							// Sound effect to play.
		ANIM_NONE
	);
	new AnimTypeClass( // VehHit1
		ANIM_VEH_HIT1,							// Animation number.
		"VEH-HIT1",								// Data name of animation.
		30,										// Maximum dimension of animation.
		4,											// Biggest animation stage.
		false,									// Theater specific art imagery?
		true,										// Normalized animation rate?
		false,									// Uses white translucent table?
		false,									// Scorches the ground?
		true,										// Forms a crater?
		true,										// Sticks to unit in square?
		false,									// Ground level animation?
		false,									// Translucent colors in this animation?
		false,									// Is this a flame thrower animation?
		0,											// Damage to apply per tick (fixed point).
		1,											// Delay between frames.
		0,											// Starting frame number.
		0,											// Loop start frame number.
		-1,										// Ending frame of loop back.
		-1,										// Number of animation stages.
		1,											// Number of times the animation loops.
		VOC_KABOOM25,							// Sound effect to play.
		ANIM_NONE
	);
	new AnimTypeClass( // VehHit2
		ANIM_VEH_HIT2,							// Animation number.
		"VEH-HIT2",								// Data name of animation.
		21,										// Maximum dimension of animation.
		1,											// Biggest animation stage.
		false,									// Theater specific art imagery?
		true,										// Normalized animation rate?
		false,									// Uses white translucent table?
		false,									// Scorches the ground?
		true,										// Forms a crater?
		true,										// Sticks to unit in square?
		false,									// Ground level animation?
		false,									// Translucent colors in this animation?
		false,									// Is this a flame thrower animation?
		0,											// Damage to apply per tick (fixed point).
		1,											// Delay between frames.
		0,											// Starting frame number.
		0,											// Loop start frame number.
		-1,										// Ending frame of loop back.
		-1,										// Number of animation stages.
		1,											// Number of times the animation loops.
		VOC_KABOOM12,								// Sound effect to play.
		ANIM_NONE
	);
	new AnimTypeClass( // VehHit3
		ANIM_VEH_HIT3,							// Animation number.
		"VEH-HIT3",								// Data name of animation.
		19,										// Maximum dimension of animation.
		3,											// Biggest animation stage.
		false,									// Theater specific art imagery?
		true,										// Normalized animation rate?
		false,									// Uses white translucent table?
		false,									// Scorches the ground?
		false,									// Forms a crater?
		true,										// Sticks to unit in square?
		false,									// Ground level animation?
		false,									// Translucent colors in this animation?
		false,									// Is this a flame thrower animation?
		0,											// Damage to apply per tick (fixed point).
		1,											// Delay between frames.
		0,											// Starting frame number.
		0,											// Loop start frame number.
		-1,										// Ending frame of loop back.
		-1,										// Number of animation stages.
		1,											// Number of times the animation loops.
		VOC_KABOOM12,								// Sound effect to play.
		ANIM_NONE
	);
	new AnimTypeClass( // ArtExp1
		ANIM_ART_EXP1,							// Animation number.
		"ART-EXP1",								// Data name of animation.
		41,										// Maximum dimension of animation.
		1,											// Biggest animation stage.
		false,									// Theater specific art imagery?
		true,										// Normalized animation rate?
		false,									// Uses white translucent table?
		false,									// Scorches the ground?
		true,										// Forms a crater?
		false,									// Sticks to unit in square?
		false,									// Ground level animation?
		false,									// Translucent colors in this animation?
		false,									// Is this a flame thrower animation?
		0,											// Damage to apply per tick (fixed point).
		1,											// Delay between frames.
		0,											// Starting frame number.
		0,											// Loop start frame number.
		-1,										// Ending frame of loop back.
		-1,										// Number of animation stages.
		1,											// Number of times the animation loops.
		VOC_KABOOM22,							// Sound effect to play.
		ANIM_NONE
	);
	new AnimTypeClass( // Napalm1
		ANIM_NAPALM1,							// Animation number.
		"NAPALM1",								// Data name of animation.
		21,										// Maximum dimension of animation.
		5,											// Biggest animation stage.
		false,									// Theater specific art imagery?
		false,									// Normalized animation rate?
		false,									// Uses white translucent table?
		true,										// Scorches the ground?
		false,									// Forms a crater?
		false,									// Sticks to unit in square?
		false,									// Ground level animation?
		false,									// Translucent colors in this animation?
		false,									// Is this a flame thrower animation?
		0,											// Damage to apply per tick (fixed point).
		1,											// Delay between frames.
		0,											// Starting frame number.
		0,											// Loop start frame number.
		-1,										// Ending frame of loop back.
		-1,										// Number of animation stages.
		1,											// Number of times the animation loops.
		VOC_FIRE_EXPLODE,						// Sound effect to play.
		ANIM_NONE
	);
	new AnimTypeClass( // Napalm2
		ANIM_NAPALM2,							// Animation number.
		"NAPALM2",								// Data name of animation.
		41,										// Maximum dimension of animation.
		5,											// Biggest animation stage.
		false,									// Theater specific art imagery?
		false,									// Normalized animation rate?
		false,									// Uses white translucent table?
		true,										// Scorches the ground?
		false,									// Forms a crater?
		false,									// Sticks to unit in square?
		false,									// Ground level animation?
		false,									// Translucent colors in this animation?
		false,									// Is this a flame thrower animation?
		0,											// Damage to apply per tick (fixed point).
		1,											// Delay between frames.
		0,											// Starting frame number.
		0,											// Loop start frame number.
		-1,										// Ending frame of loop back.
		-1,										// Number of animation stages.
		1,											// Number of times the animation loops.
		VOC_FIRE_EXPLODE,						// Sound effect to play.
		ANIM_NONE
	);
	new AnimTypeClass( // Napalm3
		ANIM_NAPALM3,							// Animation number.
		"NAPALM3",								// Data name of animation.
		78,										// Maximum dimension of animation.
		5,											// Biggest animation stage.
		false,									// Theater specific art imagery?
		false,									// Normalized animation rate?
		false,									// Uses white translucent table?
		true,										// Scorches the ground?
		false,									// Forms a crater?
		false,									// Sticks to unit in square?
		false,									// Ground level animation?
		false,									// Translucent colors in this animation?
		false,									// Is this a flame thrower animation?
		0,											// Damage to apply per tick (fixed point).
		1,											// Delay between frames.
		0,											// Starting frame number.
		0,											// Loop start frame number.
		-1,										// Ending frame of loop back.
		-1,										// Number of animation stages.
		1,											// Number of times the animation loops.
		VOC_FIRE_LAUNCH,							// Sound effect to play.
		ANIM_NONE
	);
	new AnimTypeClass( // SmokePuff
		ANIM_SMOKE_PUFF,						// Animation number.
		"SMOKEY",								// Data name of animation.
		24,										// Maximum dimension of animation.
		2,											// Biggest animation stage.
		false,									// Theater specific art imagery?
		true,										// Normalized animation rate?
		false,									// Uses white translucent table?
		false,									// Scorches the ground?
		false,									// Forms a crater?
		false,									// Sticks to unit in square?
		false,									// Ground level animation?
		true,										// Translucent colors in this animation?
		false,									// Is this a flame thrower animation?
		0,											// Damage to apply per tick (fixed point).
		1,											// Delay between frames.
		0,											// Starting frame number.
		0,											// Loop start frame number.
		-1,										// Ending frame of loop back.
		-1,										// Number of animation stages.
		1,											// Number of times the animation loops.
		VOC_NONE,								// Sound effect to play.
		ANIM_NONE
	);
	new AnimTypeClass( // Piff
		ANIM_PIFF,								// Animation number.
		"PIFF",									// Data name of animation.
		13,										// Maximum dimension of animation.
		1,											// Biggest animation stage.
		false,									// Theater specific art imagery?
		true,										// Normalized animation rate?
		false,									// Uses white translucent table?
		false,									// Scorches the ground?
		false,									// Forms a crater?
		false,									// Sticks to unit in square?
		false,									// Ground level animation?
		false,									// Translucent colors in this animation?
		false,									// Is this a flame thrower animation?
		0,											// Damage to apply per tick (fixed point).
		1,											// Delay between frames.
		0,											// Starting frame number.
		0,											// Loop start frame number.
		-1,										// Ending frame of loop back.
		-1,										// Number of animation stages.
		1,											// Number of times the animation loops.
		VOC_NONE,								// Sound effect to play.
		ANIM_NONE
	);
	new AnimTypeClass( // PiffPiff
		ANIM_PIFFPIFF,							// Animation number.
		"PIFFPIFF",								// Data name of animation.
		20,										// Maximum dimension of animation.
		2,											// Biggest animation stage.
		false,									// Theater specific art imagery?
		true,										// Normalized animation rate?
		false,									// Uses white translucent table?
		false,									// Scorches the ground?
		false,									// Forms a crater?
		false,									// Sticks to unit in square?
		false,									// Ground level animation?
		false,									// Translucent colors in this animation?
		false,									// Is this a flame thrower animation?
		0,											// Damage to apply per tick (fixed point).
		1,											// Delay between frames.
		0,											// Starting frame number.
		0,											// Loop start frame number.
		-1,										// Ending frame of loop back.
		-1,										// Number of animation stages.
		1,											// Number of times the animation loops.
		VOC_NONE,								// Sound effect to play.
		ANIM_NONE
	);
	new AnimTypeClass( // Fire3
		ANIM_FIRE_SMALL,						// Animation number.
		"FIRE3",									// Data name of animation.
		23,										// Maximum dimension of animation.
		0,											// Biggest animation stage.
		false,									// Theater specific art imagery?
		false,									// Normalized animation rate?
		false,									// Uses white translucent table?
		false,									// Scorches the ground?
		false,									// Forms a crater?
		false,									// Sticks to unit in square?
		true,										// Ground level animation?
		false,									// Translucent colors in this animation?
		false,									// Is this a flame thrower animation?
		fixed(1, 32),							// Damage to apply per tick (fixed point).
		1,											// Delay between frames.
		0,											// Starting frame number.
		0,											// Loop start frame number.
		-1,										// Ending frame of loop back.
		-1,										// Number of animation stages.
		2,											// Number of times the animation loops.
		VOC_NONE,								// Sound effect to play.
		ANIM_NONE
	);
	new AnimTypeClass( // Fire2
		ANIM_FIRE_MED,							// Animation number.
		"FIRE2",									// Data name of animation.
		23,										// Maximum dimension of animation.
		0,											// Biggest animation stage.
		false,									// Theater specific art imagery?
		false,									// Normalized animation rate?
		false,									// Uses white translucent table?
		true,										// Scorches the ground?
		false,									// Forms a crater?
		false,									// Sticks to unit in square?
		true,										// Ground level animation?
		false,									// Translucent colors in this animation?
		false,									// Is this a flame thrower animation?
		fixed(1, 16),							// Damage to apply per tick (fixed point).
		1,											// Delay between frames.
		0,											// Starting frame number.
		0,											// Loop start frame number.
		-1,										// Ending frame of loop back.
		-1,										// Number of animation stages.
		3,											// Number of times the animation loops.
		VOC_NONE,								// Sound effect to play.
		ANIM_NONE
	);
	new AnimTypeClass( // Fire1
		ANIM_FIRE_MED2,		 				// Animation number.
		"FIRE1",									// Data name of animation.
		23,										// Maximum dimension of animation.
		0,											// Biggest animation stage.
		false,									// Theater specific art imagery?
		false,									// Normalized animation rate?
		false,									// Uses white translucent table?
		true,										// Scorches the ground?
		false,									// Forms a crater?
		false,									// Sticks to unit in square?
		true,										// Ground level animation?
		false,									// Translucent colors in this animation?
		false,									// Is this a flame thrower animation?
		fixed(1, 16),							// Damage to apply per tick (fixed point).
		1,											// Delay between frames.
		0,											// Starting frame number.
		0,											// Loop start frame number.
		-1,										// Ending frame of loop back.
		-1,										// Number of animation stages.
		3,											// Number of times the animation loops.
		VOC_NONE,								// Sound effect to play.
		ANIM_NONE
	);
	new AnimTypeClass( // Fire4
		ANIM_FIRE_TINY,		 				// Animation number.
		"FIRE4",									// Data name of animation.
		7,											// Maximum dimension of animation.
		0,											// Biggest animation stage.
		false,									// Theater specific art imagery?
		false,									// Normalized animation rate?
		false,									// Uses white translucent table?
		false,									// Scorches the ground?
		false,									// Forms a crater?
		false,									// Sticks to unit in square?
		true,										// Ground level animation?
		false,									// Translucent colors in this animation?
		false,									// Is this a flame thrower animation?
		fixed(1, 32),							// Damage to apply per tick (fixed point).
		1,											// Delay between frames.
		0,											// Starting frame number.
		0,											// Loop start frame number.
		-1,										// Ending frame of loop back.
		-1,										// Number of animation stages.
		3,											// Number of times the animation loops.
		VOC_NONE,								// Sound effect to play.
		ANIM_NONE
	);	
	new AnimTypeClass( // Gunfire
		ANIM_MUZZLE_FLASH,					// Animation number.
		"GUNFIRE",								// Data name of animation.
		16,										// Maximum dimension of animation.
		0,											// Biggest animation stage.
		false,									// Theater specific art imagery?
		false,									// Normalized animation rate?
		false,									// Uses white translucent table?
		false,									// Scorches the ground?
		false,									// Forms a crater?
		false,									// Sticks to unit in square?
		true,										// Ground level animation?
		true,										// Translucent colors in this animation?
		false,									// Is this a flame thrower animation?
		0,											// Damage to apply per tick (fixed point).
		1,											// Delay between frames.
		0,											// Starting frame number.
		0,											// Loop start frame number.
		0,											// Number of times the animation loops.
		1,											// Number of animation stages.
		1,											// Ending frame of loop back.
		VOC_NONE,								// Sound effect to play.
		ANIM_NONE
	);
	new AnimTypeClass( // SmokeM
		ANIM_SMOKE_M,							// Animation number.
		"SMOKE_M",								// Data name of animation.
		28,										// Maximum dimension of animation.
		30,										// Biggest animation stage.
		false,									// Theater specific art imagery?
		true,										// Normalized animation rate?
		false,									// Uses white translucent table?
		false,									// Scorches the ground?
		false,									// Forms a crater?
		false,									// Sticks to unit in square?
		true,										// Ground level animation?
		false,									// Translucent colors in this animation?
		false,									// Is this a flame thrower animation?
		0,											// Damage to apply per tick (fixed point).
		1,											// Delay between frames.
		0,											// Starting frame number.
		67,										// Loop start frame number.
		-1,										// Loopback frame number.
		-1,										// Number of animation stages.
		6,											// Number of times the animation loops.
		VOC_NONE,								// Sound effect to play.
		ANIM_NONE
	);

	/*
	**	Flammable object burning animations. Primarily used on trees and buildings.
	*/
	new AnimTypeClass( // BurnSmall
		ANIM_BURN_SMALL,						// Animation number.
		"BURN-S",								// Data name of animation.
		11,										// Maximum dimension of animation.
		13,										// Biggest animation stage.
		false,									// Theater specific art imagery?
		false,									// Normalized animation rate?
		false,									// Uses white translucent table?
		false,									// Scorches the ground?
		false,									// Forms a crater?
		false,									// Sticks to unit in square?
		true,										// Ground level animation?
		false,									// Translucent colors in this animation?
		false,									// Is this a flame thrower animation?
		fixed(1, 32),							// Damage to apply per tick (fixed point).
		2,											// Delay between frames.
		0,											// Starting frame number.
		30,										// Loop start frame number.
		62,										// Ending frame of loop back.
		-1,										// Number of animation stages.
		4,											// Number of times the animation loops.
		VOC_NONE,								// Sound effect to play.
		ANIM_NONE
	);
	new AnimTypeClass( // BurnMed
		ANIM_BURN_MED,							// Animation number.
		"BURN-M",								// Data name of animation.
		14,										// Maximum dimension of animation.
		13,										// Biggest animation stage.
		false,									// Theater specific art imagery?
		false,									// Normalized animation rate?
		false,									// Uses white translucent table?
		false,									// Scorches the ground?
		false,									// Forms a crater?
		false,									// Sticks to unit in square?
		true,										// Ground level animation?
		false,									// Translucent colors in this animation?
		false,									// Is this a flame thrower animation?
		fixed(1, 16),							// Damage to apply per tick (fixed point).
		2,											// Delay between frames.
		0,											// Starting frame number.
		30,										// Loop start frame number.
		62,										// Ending frame of loop back.
		-1,										// Number of animation stages.
		4,											// Number of times the animation loops.
		VOC_NONE,								// Sound effect to play.
		ANIM_NONE
	);
	new AnimTypeClass( // BurnBig
		ANIM_BURN_BIG,							// Animation number.
		"BURN-L",								// Data name of animation.
		23,										// Maximum dimension of animation.
		13,										// Biggest animation stage.
		false,									// Theater specific art imagery?
		false,									// Normalized animation rate?
		false,									// Uses white translucent table?
		true,										// Scorches the ground?
		false,									// Forms a crater?
		false,									// Sticks to unit in square?
		true,										// Ground level animation?
		false,									// Translucent colors in this animation?
		false,									// Is this a flame thrower animation?
		fixed(1, 10),							// Damage to apply per tick (fixed point).
		2,											// Delay between frames.
		0,											// Starting frame number.
		30,										// Loop start frame number.
		62,										// Ending frame of loop back.
		-1,										// Number of animation stages.
		4,											// Number of times the animation loops.
		VOC_NONE,								// Sound effect to play.
		ANIM_NONE
	);
	/*
	**	Flammable object burning animations that trail into smoke. Used for
	**	buildings and the gunboat.
	*/
	new AnimTypeClass( // OnFireSmall
		ANIM_ON_FIRE_SMALL,					// Animation number.
		"BURN-S",								// Data name of animation.
		11,										// Maximum dimension of animation.
		13,										// Biggest animation stage.
		false,									// Theater specific art imagery?
		false,									// Normalized animation rate?
		false,									// Uses white translucent table?
		false,									// Scorches the ground?
		false,									// Forms a crater?
		false,									// Sticks to unit in square?
		true,										// Ground level animation?
		false,									// Translucent colors in this animation?
		false,									// Is this a flame thrower animation?
		fixed(1, 32),							// Damage to apply per tick (fixed point).
		2,											// Delay between frames.
		0,											// Starting frame number.
		30,										// Loop start frame number.
		62,										// Ending frame of loop back.
		-1,										// Number of animation stages.
		4,											// Number of times the animation loops.
		VOC_NONE,								// Sound effect to play.
		ANIM_SMOKE_M
	);
	new AnimTypeClass( // OnFireMed
		ANIM_ON_FIRE_MED,						// Animation number.
		"BURN-M",								// Data name of animation.
		14,										// Maximum dimension of animation.
		13,										// Biggest animation stage.
		false,									// Theater specific art imagery?
		false,									// Normalized animation rate?
		false,									// Uses white translucent table?
		false,									// Scorches the ground?
		false,									// Forms a crater?
		false,									// Sticks to unit in square?
		true,										// Ground level animation?
		false,									// Translucent colors in this animation?
		false,									// Is this a flame thrower animation?
		fixed(1, 16),							// Damage to apply per tick (fixed point).
		2,											// Delay between frames.
		0,											// Starting frame number.
		30,										// Loop start frame number.
		62,										// Ending frame of loop back.
		-1,										// Number of animation stages.
		4,											// Number of times the animation loops.
		VOC_NONE,								// Sound effect to play.
		ANIM_ON_FIRE_SMALL
	);
	new AnimTypeClass( // OnFireBig
		ANIM_ON_FIRE_BIG,						// Animation number.
		"BURN-L",								// Data name of animation.
		23,										// Maximum dimension of animation.
		13,										// Biggest animation stage.
		false,									// Theater specific art imagery?
		false,									// Normalized animation rate?
		false,									// Uses white translucent table?
		true,										// Scorches the ground?
		false,									// Forms a crater?
		false,									// Sticks to unit in square?
		true,										// Ground level animation?
		false,									// Translucent colors in this animation?
		false,									// Is this a flame thrower animation?
		fixed(1, 10),							// Damage to apply per tick (fixed point).
		2,											// Delay between frames.
		0,											// Starting frame number.
		30,										// Loop start frame number.
		62,										// Ending frame of loop back.
		-1,										// Number of animation stages.
		4,											// Number of times the animation loops.
		VOC_NONE,								// Sound effect to play.
		ANIM_ON_FIRE_MED
	);
	new AnimTypeClass( // SAMN
		ANIM_SAM_N,								// Animation number.
		"SAMFIRE",								// Data name of animation.
		55,										// Maximum dimension of animation.
		4,											// Biggest animation stage.
		false,									// Theater specific art imagery?
		false,									// Normalized animation rate?
		false,									// Uses white translucent table?
		false,									// Scorches the ground?
		false,									// Forms a crater?
		false,									// Sticks to unit in square?
		false,									// Ground level animation?
		false,									// Translucent colors in this animation?
		false,									// Is this a flame thrower animation?
		0,											// Damage to apply per tick (fixed point).
		1,											// Delay between frames.
		18*0,										// Starting frame number.
		0,											// Loop start frame number.
		0,											// Ending frame of loop back.
		18,										// Number of animation stages.
		0,											// Number of times the animation loops.
		VOC_NONE,								// Sound effect to play.
		ANIM_NONE
	);
	new AnimTypeClass( // SAMNE
		ANIM_SAM_NE,							// Animation number.
		"SAMFIRE",								// Data name of animation.
		55,										// Maximum dimension of animation.
		130,										// Biggest animation stage.
		false,									// Theater specific art imagery?
		false,									// Normalized animation rate?
		false,									// Uses white translucent table?
		false,									// Scorches the ground?
		false,									// Forms a crater?
		false,									// Sticks to unit in square?
		false,									// Ground level animation?
		false,									// Translucent colors in this animation?
		false,									// Is this a flame thrower animation?
		0,											// Damage to apply per tick (fixed point).
		1,											// Delay between frames.
		18*7,										// Starting frame number.
		0,											// Loop start frame number.
		0,											// Ending frame of loop back.
		18,										// Number of animation stages.
		0,											// Number of times the animation loops.
		VOC_NONE,								// Sound effect to play.
		ANIM_NONE
	);
	new AnimTypeClass( // SAME
		ANIM_SAM_E,								// Animation number.
		"SAMFIRE",								// Data name of animation.
		55,										// Maximum dimension of animation.
		112,										// Biggest animation stage.
		false,									// Theater specific art imagery?
		false,									// Normalized animation rate?
		false,									// Uses white translucent table?
		false,									// Scorches the ground?
		false,									// Forms a crater?
		false,									// Sticks to unit in square?
		false,									// Ground level animation?
		false,									// Translucent colors in this animation?
		false,									// Is this a flame thrower animation?
		0,											// Damage to apply per tick (fixed point).
		1,											// Delay between frames.
		18*6,										// Starting frame number.
		0,											// Loop start frame number.
		0,											// Ending frame of loop back.
		18,										// Number of animation stages.
		0,											// Number of times the animation loops.
		VOC_NONE,								// Sound effect to play.
		ANIM_NONE
	);
	new AnimTypeClass( // SAMSE
		ANIM_SAM_SE,							// Animation number.
		"SAMFIRE",								// Data name of animation.
		55,										// Maximum dimension of animation.
		94,										// Biggest animation stage.
		false,									// Theater specific art imagery?
		false,									// Normalized animation rate?
		false,									// Uses white translucent table?
		false,									// Scorches the ground?
		false,									// Forms a crater?
		false,									// Sticks to unit in square?
		false,									// Ground level animation?
		false,									// Translucent colors in this animation?
		false,									// Is this a flame thrower animation?
		0,											// Damage to apply per tick (fixed point).
		1,											// Delay between frames.
		18*5,										// Starting frame number.
		0,											// Loop start frame number.
		0,											// Ending frame of loop back.
		18,										// Number of animation stages.
		0,											// Number of times the animation loops.
		VOC_NONE,								// Sound effect to play.
		ANIM_NONE
	);
	new AnimTypeClass( // SAMS
		ANIM_SAM_S,								// Animation number.
		"SAMFIRE",								// Data name of animation.
		55,										// Maximum dimension of animation.
		76,										// Biggest animation stage.
		false,									// Theater specific art imagery?
		false,									// Normalized animation rate?
		false,									// Uses white translucent table?
		false,									// Scorches the ground?
		false,									// Forms a crater?
		false,									// Sticks to unit in square?
		false,									// Ground level animation?
		false,									// Translucent colors in this animation?
		false,									// Is this a flame thrower animation?
		0,											// Damage to apply per tick (fixed point).
		1,											// Delay between frames.
		18*4,										// Starting frame number.
		0,											// Loop start frame number.
		0,											// Ending frame of loop back.
		18,										// Number of animation stages.
		0,											// Number of times the animation loops.
		VOC_NONE,								// Sound effect to play.
		ANIM_NONE
	);
	new AnimTypeClass( // SAMSW
		ANIM_SAM_SW,							// Animation number.
		"SAMFIRE",								// Data name of animation.
		55,										// Maximum dimension of animation.
		58,										// Biggest animation stage.
		false,									// Theater specific art imagery?
		false,									// Normalized animation rate?
		false,									// Uses white translucent table?
		false,									// Scorches the ground?
		false,									// Forms a crater?
		false,									// Sticks to unit in square?
		false,									// Ground level animation?
		false,									// Translucent colors in this animation?
		false,									// Is this a flame thrower animation?
		0,											// Damage to apply per tick (fixed point).
		1,											// Delay between frames.
		18*3,										// Starting frame number.
		0,											// Loop start frame number.
		0,											// Ending frame of loop back.
		18,										// Number of animation stages.
		0,											// Number of times the animation loops.
		VOC_NONE,								// Sound effect to play.
		ANIM_NONE
	);
	new AnimTypeClass( // SAMW
		ANIM_SAM_W,								// Animation number.
		"SAMFIRE",								// Data name of animation.
		55,										// Maximum dimension of animation.
		40,										// Biggest animation stage.
		false,									// Theater specific art imagery?
		false,									// Normalized animation rate?
		false,									// Uses white translucent table?
		false,									// Scorches the ground?
		false,									// Forms a crater?
		false,									// Sticks to unit in square?
		false,									// Ground level animation?
		false,									// Translucent colors in this animation?
		false,									// Is this a flame thrower animation?
		0,											// Damage to apply per tick (fixed point).
		1,											// Delay between frames.
		18*2,										// Starting frame number.
		0,											// Loop start frame number.
		0,											// Ending frame of loop back.
		18,										// Number of animation stages.
		0,											// Number of times the animation loops.
		VOC_NONE,								// Sound effect to play.
		ANIM_NONE
	);
	new AnimTypeClass( // SAMNW
		ANIM_SAM_NW,							// Animation number.
		"SAMFIRE",								// Data name of animation.
		55,										// Maximum dimension of animation.
		22,										// Biggest animation stage.
		false,									// Theater specific art imagery?
		false,									// Normalized animation rate?
		false,									// Uses white translucent table?
		false,									// Scorches the ground?
		false,									// Forms a crater?
		false,									// Sticks to unit in square?
		false,									// Ground level animation?
		false,									// Translucent colors in this animation?
		false,									// Is this a flame thrower animation?
		0,											// Damage to apply per tick (fixed point).
		1,											// Delay between frames.
		18*1,										// Starting frame number.
		0,											// Loop start frame number.
		0,											// Ending frame of loop back.
		18,										// Number of animation stages.
		0,											// Number of times the animation loops.
		VOC_NONE,								// Sound effect to play.
		ANIM_NONE
	);
	/*
	**	Mini-gun fire effect -- used by guard towers.
	*/
	new AnimTypeClass( // GUNN
		ANIM_GUN_N,								// Animation number.
		"MINIGUN",								// Data name of animation.
		18,										// Maximum dimension of animation.
		0,											// Biggest animation stage.
		false,									// Theater specific art imagery?
		false,									// Normalized animation rate?
		false,									// Uses white translucent table?
		false,									// Scorches the ground?
		false,									// Forms a crater?
		false,									// Sticks to unit in square?
		false,									// Ground level animation?
		false,									// Translucent colors in this animation?
		false,									// Is this a flame thrower animation?
		0,											// Damage to apply per tick (fixed point).
		1,											// Delay between frames.
		0,											// Starting frame number.
		0,											// Loop start frame number.
		0,											// Number of times the animation loops.
		6,											// Number of animation stages.
		0,											// Ending frame of loop back.
		VOC_NONE,								// Sound effect to play.
		ANIM_NONE
	);
	new AnimTypeClass( // GUNNE
		ANIM_GUN_NE,							// Animation number.
		"MINIGUN",								// Data name of animation.
		18,										// Maximum dimension of animation.
		0,											// Biggest animation stage.
		false,									// Theater specific art imagery?
		false,									// Normalized animation rate?
		false,									// Uses white translucent table?
		false,									// Scorches the ground?
		false,									// Forms a crater?
		false,									// Sticks to unit in square?
		false,									// Ground level animation?
		false,									// Translucent colors in this animation?
		false,									// Is this a flame thrower animation?
		0,											// Damage to apply per tick (fixed point).
		1,											// Delay between frames.
		42,										// Starting frame number.
		0,											// Loop start frame number.
		0,											// Number of times the animation loops.
		6,											// Number of animation stages.
		0,											// Ending frame of loop back.
		VOC_NONE,								// Sound effect to play.
		ANIM_NONE
	);
	new AnimTypeClass( // GUNE
		ANIM_GUN_E,								// Animation number.
		"MINIGUN",								// Data name of animation.
		18,										// Maximum dimension of animation.
		0,											// Biggest animation stage.
		false,									// Theater specific art imagery?
		false,									// Normalized animation rate?
		false,									// Uses white translucent table?
		false,									// Scorches the ground?
		false,									// Forms a crater?
		false,									// Sticks to unit in square?
		false,									// Ground level animation?
		false,									// Translucent colors in this animation?
		false,									// Is this a flame thrower animation?
		0,											// Damage to apply per tick (fixed point).
		1,											// Delay between frames.
		36,										// Starting frame number.
		0,											// Loop start frame number.
		0,											// Number of times the animation loops.
		6,											// Number of animation stages.
		0,											// Ending frame of loop back.
		VOC_NONE,								// Sound effect to play.
		ANIM_NONE
	);
	new AnimTypeClass( // GUNSE
		ANIM_GUN_SE,							// Animation number.
		"MINIGUN",								// Data name of animation.
		18,										// Maximum dimension of animation.
		0,											// Biggest animation stage.
		false,									// Theater specific art imagery?
		false,									// Normalized animation rate?
		false,									// Uses white translucent table?
		false,									// Scorches the ground?
		false,									// Forms a crater?
		false,									// Sticks to unit in square?
		false,									// Ground level animation?
		false,									// Translucent colors in this animation?
		false,									// Is this a flame thrower animation?
		0,											// Damage to apply per tick (fixed point).
		1,											// Delay between frames.
		30,										// Starting frame number.
		0,											// Loop start frame number.
		0,											// Number of times the animation loops.
		6,											// Number of animation stages.
		0,											// Ending frame of loop back.
		VOC_NONE,								// Sound effect to play.
		ANIM_NONE
	);
	new AnimTypeClass( // GUNS
		ANIM_GUN_S,								// Animation number.
		"MINIGUN",								// Data name of animation.
		18,										// Maximum dimension of animation.
		0,											// Biggest animation stage.
		false,									// Theater specific art imagery?
		false,									// Normalized animation rate?
		false,									// Uses white translucent table?
		false,									// Scorches the ground?
		false,									// Forms a crater?
		false,									// Sticks to unit in square?
		false,									// Ground level animation?
		false,									// Translucent colors in this animation?
		false,									// Is this a flame thrower animation?
		0,											// Damage to apply per tick (fixed point).
		1,											// Delay between frames.
		24,										// Starting frame number.
		0,											// Loop start frame number.
		0,											// Number of times the animation loops.
		6,											// Number of animation stages.
		0,											// Ending frame of loop back.
		VOC_NONE,								// Sound effect to play.
		ANIM_NONE
	);
	new AnimTypeClass( // GUNSW
		ANIM_GUN_SW,							// Animation number.
		"MINIGUN",								// Data name of animation.
		18,										// Maximum dimension of animation.
		0,											// Biggest animation stage.
		false,									// Theater specific art imagery?
		false,									// Normalized animation rate?
		false,									// Uses white translucent table?
		false,									// Scorches the ground?
		false,									// Forms a crater?
		false,									// Sticks to unit in square?
		false,									// Ground level animation?
		false,									// Translucent colors in this animation?
		false,									// Is this a flame thrower animation?
		0,											// Damage to apply per tick (fixed point).
		1,											// Delay between frames.
		18,										// Starting frame number.
		0,											// Loop start frame number.
		0,											// Number of times the animation loops.
		6,											// Number of animation stages.
		0,											// Ending frame of loop back.
		VOC_NONE,								// Sound effect to play.
		ANIM_NONE
	);
	new AnimTypeClass( // GUNW
		ANIM_GUN_W,								// Animation number.
		"MINIGUN",								// Data name of animation.
		18,										// Maximum dimension of animation.
		0,											// Biggest animation stage.
		false,									// Theater specific art imagery?
		false,									// Normalized animation rate?
		false,									// Uses white translucent table?
		false,									// Scorches the ground?
		false,									// Forms a crater?
		false,									// Sticks to unit in square?
		false,									// Ground level animation?
		false,									// Translucent colors in this animation?
		false,									// Is this a flame thrower animation?
		0,											// Damage to apply per tick (fixed point).
		1,											// Delay between frames.
		12,										// Starting frame number.
		0,											// Loop start frame number.
		0,											// Number of times the animation loops.
		6,											// Number of animation stages.
		0,											// Ending frame of loop back.
		VOC_NONE,								// Sound effect to play.
		ANIM_NONE
	);
	new AnimTypeClass( // GUNNW
		ANIM_GUN_NW,							// Animation number.
		"MINIGUN",								// Data name of animation.
		18,										// Maximum dimension of animation.
		0,											// Biggest animation stage.
		false,									// Theater specific art imagery?
		false,									// Normalized animation rate?
		false,									// Uses white translucent table?
		false,									// Scorches the ground?
		false,									// Forms a crater?
		false,									// Sticks to unit in square?
		false,									// Ground level animation?
		false,									// Translucent colors in this animation?
		false,									// Is this a flame thrower animation?
		0,											// Damage to apply per tick (fixed point).
		1,											// Delay between frames.
		6,											// Starting frame number.
		0,											// Loop start frame number.
		0,											// Number of times the animation loops.
		6,											// Number of animation stages.
		0,											// Ending frame of loop back.
		VOC_NONE,								// Sound effect to play.
		ANIM_NONE
	);
	new AnimTypeClass( // LZSmoke
		ANIM_LZ_SMOKE,						// Animation number.
		"SMOKLAND",								// Data name of animation.
		32,										// Maximum dimension of animation.
		72,										// Biggest animation stage.
		false,									// Theater specific art imagery?
		true,										// Normalized animation rate?
		false,									// Uses white translucent table?
		false,									// Scorches the ground?
		false,									// Forms a crater?
		false,									// Sticks to unit in square?
		true,										// Ground level animation?
		false,									// Translucent colors in this animation?
		false,									// Is this a flame thrower animation?
		0,											// Damage to apply per tick (fixed point).
		2,											// Delay between frames.
		0,											// Starting frame number.
		72,										// Loop start frame number.
		91,										// Ending frame of loop back.
		-1,										// Number of animation stages.
		255,										// Number of times the animation loops.
		VOC_NONE,								// Sound effect to play.
		ANIM_NONE
	);
	new AnimTypeClass( // CDeviator
		ANIM_CRATE_DEVIATOR,					// Animation number.
		"DEVIATOR",								// Data name of animation.
		48,										// Maximum dimension of animation.
		0,											// Biggest animation stage.
		false,									// Theater specific art imagery?
		true,										// Normalized animation rate?
		false,									// Uses white translucent table?
		false,									// Scorches the ground?
		false,									// Forms a crater?
		false,									// Sticks to unit in square?
		false,									// Ground level animation?
		false,									// Translucent colors in this animation?
		false,									// Is this a flame thrower animation?
		0,											// Damage to apply per tick (fixed point).
		2,											// Delay between frames.
		0,											// Starting frame number.
		0,											// Loop start frame number.
		0,											// Ending frame of loop back.
		-1,										// Number of animation stages.
		0,											// Number of times the animation loops.
		VOC_NONE,								// Sound effect to play.
		ANIM_NONE								// Follow up animation.
	);
	new AnimTypeClass( // CDollar
		ANIM_CRATE_DOLLAR,					// Animation number.
		"DOLLAR",								// Data name of animation.
		48,										// Maximum dimension of animation.
		0,											// Biggest animation stage.
		false,									// Theater specific art imagery?
		true,										// Normalized animation rate?
		false,									// Uses white translucent table?
		false,									// Scorches the ground?
		false,									// Forms a crater?
		false,									// Sticks to unit in square?
		false,									// Ground level animation?
		false,									// Translucent colors in this animation?
		false,									// Is this a flame thrower animation?
		0,											// Damage to apply per tick (fixed point).
		2,											// Delay between frames.
		0,											// Starting frame number.
		0,											// Loop start frame number.
		0,											// Ending frame of loop back.
		-1,										// Number of animation stages.
		0,											// Number of times the animation loops.
		VOC_NONE,								// Sound effect to play.
		ANIM_NONE								// Follow up animation.
	);
	new AnimTypeClass( // CEarth
		ANIM_CRATE_EARTH,						// Animation number.
		"EARTH",									// Data name of animation.
		48,										// Maximum dimension of animation.
		0,											// Biggest animation stage.
		false,									// Theater specific art imagery?
		true,										// Normalized animation rate?
		false,									// Uses white translucent table?
		false,									// Scorches the ground?
		false,									// Forms a crater?
		false,									// Sticks to unit in square?
		false,									// Ground level animation?
		false,									// Translucent colors in this animation?
		false,									// Is this a flame thrower animation?
		0,											// Damage to apply per tick (fixed point).
		2,											// Delay between frames.
		0,											// Starting frame number.
		0,											// Loop start frame number.
		0,											// Ending frame of loop back.
		-1,										// Number of animation stages.
		0,											// Number of times the animation loops.
		VOC_NONE,								// Sound effect to play.
		ANIM_NONE								// Follow up animation.
	);
	new AnimTypeClass( // CEmpulse
		ANIM_CRATE_EMPULSE,					// Animation number.
		"EMPULSE",								// Data name of animation.
		48,										// Maximum dimension of animation.
		0,											// Biggest animation stage.
		false,									// Theater specific art imagery?
		true,										// Normalized animation rate?
		false,									// Uses white translucent table?
		false,									// Scorches the ground?
		false,									// Forms a crater?
		false,									// Sticks to unit in square?
		false,									// Ground level animation?
		false,									// Translucent colors in this animation?
		false,									// Is this a flame thrower animation?
		0,											// Damage to apply per tick (fixed point).
		2,											// Delay between frames.
		0,											// Starting frame number.
		0,											// Loop start frame number.
		0,											// Ending frame of loop back.
		-1,										// Number of animation stages.
		0,											// Number of times the animation loops.
		VOC_NONE,								// Sound effect to play.
		ANIM_NONE								// Follow up animation.
	);
	new AnimTypeClass( // CInvun
		ANIM_CRATE_INVUN,						// Animation number.
		"INVUN",									// Data name of animation.
		48,										// Maximum dimension of animation.
		0,											// Biggest animation stage.
		false,									// Theater specific art imagery?
		true,										// Normalized animation rate?
		false,									// Uses white translucent table?
		false,									// Scorches the ground?
		false,									// Forms a crater?
		false,									// Sticks to unit in square?
		false,									// Ground level animation?
		false,									// Translucent colors in this animation?
		false,									// Is this a flame thrower animation?
		0,											// Damage to apply per tick (fixed point).
		2,											// Delay between frames.
		0,											// Starting frame number.
		0,											// Loop start frame number.
		0,											// Ending frame of loop back.
		-1,										// Number of animation stages.
		0,											// Number of times the animation loops.
		VOC_NONE,								// Sound effect to play.
		ANIM_NONE								// Follow up animation.
	);
	new AnimTypeClass( // CMine
		ANIM_CRATE_MINE,						// Animation number.
		"MINE",									// Data name of animation.
		48,										// Maximum dimension of animation.
		0,											// Biggest animation stage.
		false,									// Theater specific art imagery?
		true,										// Normalized animation rate?
		false,									// Uses white translucent table?
		false,									// Scorches the ground?
		false,									// Forms a crater?
		false,									// Sticks to unit in square?
		false,									// Ground level animation?
		false,									// Translucent colors in this animation?
		false,									// Is this a flame thrower animation?
		0,											// Damage to apply per tick (fixed point).
		2,											// Delay between frames.
		0,											// Starting frame number.
		0,											// Loop start frame number.
		0,											// Ending frame of loop back.
		-1,										// Number of animation stages.
		0,											// Number of times the animation loops.
		VOC_NONE,								// Sound effect to play.
		ANIM_NONE								// Follow up animation.
	);
	new AnimTypeClass( // CRapid
		ANIM_CRATE_RAPID,						// Animation number.
		"RAPID",									// Data name of animation.
		48,										// Maximum dimension of animation.
		0,											// Biggest animation stage.
		false,									// Theater specific art imagery?
		true,										// Normalized animation rate?
		false,									// Uses white translucent table?
		false,									// Scorches the ground?
		false,									// Forms a crater?
		false,									// Sticks to unit in square?
		false,									// Ground level animation?
		false,									// Translucent colors in this animation?
		false,									// Is this a flame thrower animation?
		0,											// Damage to apply per tick (fixed point).
		2,											// Delay between frames.
		0,											// Starting frame number.
		0,											// Loop start frame number.
		0,											// Ending frame of loop back.
		-1,										// Number of animation stages.
		0,											// Number of times the animation loops.
		VOC_NONE,								// Sound effect to play.
		ANIM_NONE								// Follow up animation.
	);
	new AnimTypeClass( // CStealth
		ANIM_CRATE_STEALTH,					// Animation number.
		"STEALTH2",								// Data name of animation.
		48,										// Maximum dimension of animation.
		0,											// Biggest animation stage.
		false,									// Theater specific art imagery?
		true,										// Normalized animation rate?
		false,									// Uses white translucent table?
		false,									// Scorches the ground?
		false,									// Forms a crater?
		false,									// Sticks to unit in square?
		false,									// Ground level animation?
		false,									// Translucent colors in this animation?
		false,									// Is this a flame thrower animation?
		0,											// Damage to apply per tick (fixed point).
		2,											// Delay between frames.
		0,											// Starting frame number.
		0,											// Loop start frame number.
		0,											// Ending frame of loop back.
		-1,										// Number of animation stages.
		0,											// Number of times the animation loops.
		VOC_NONE,								// Sound effect to play.
		ANIM_NONE								// Follow up animation.
	);
	new AnimTypeClass( // CMissile
		ANIM_CRATE_MISSILE,					// Animation number.
		"MISSILE2",								// Data name of animation.
		48,										// Maximum dimension of animation.
		0,											// Biggest animation stage.
		false,									// Theater specific art imagery?
		true,										// Normalized animation rate?
		false,									// Uses white translucent table?
		false,									// Scorches the ground?
		false,									// Forms a crater?
		false,									// Sticks to unit in square?
		false,									// Ground level animation?
		false,									// Translucent colors in this animation?
		false,									// Is this a flame thrower animation?
		0,											// Damage to apply per tick (fixed point).
		2,											// Delay between frames.
		0,											// Starting frame number.
		0,											// Loop start frame number.
		0,											// Ending frame of loop back.
		-1,										// Number of animation stages.
		0,											// Number of times the animation loops.
		VOC_NONE,								// Sound effect to play.
		ANIM_NONE								// Follow up animation.
	);
	new AnimTypeClass( // MoveFlash
		ANIM_MOVE_FLASH,							// Animation number.
		"MOVEFLSH",								// Data name of animation.
		24,										// Maximum dimension of animation.
		0,											// Biggest animation stage.
		true,										// Theater specific art imagery?
		true,										// Normalized animation rate?
		true,										// Uses white translucent table?
		false,									// Scorches the ground?
		false,									// Forms a crater?
		false,									// Sticks to unit in square?
		true,										// Ground level animation?
		false,									// Translucent colors in this animation?
		false,									// Is this a flame thrower animation?
		0,											// Damage to apply per tick (fixed point).
		1,											// Delay between frames.
		0,											// Starting frame number.
		0,											// Loop start frame number.
		0,											// Ending frame of loop back.
		-1,										// Number of animation stages.
		0,											// Number of times the animation loops.
		VOC_NONE,								// Sound effect to play.
		ANIM_NONE								// Follow up animation.
	);
	new AnimTypeClass( // OilFieldBurn
		ANIM_OILFIELD_BURN,					// Animation number.
		"FLMSPT",								// Data name of animation.
		42,										// Maximum dimension of animation.
		58,										// Biggest animation stage.
		false,									// Theater specific art imagery?
		true,										// Normalized animation rate?
		false,									// Uses white translucent table?
		false,									// Scorches the ground?
		false,									// Forms a crater?
		false,									// Sticks to unit in square?
		true,										// Ground level animation?
		false,									// Translucent colors in this animation?
		false,									// Is this a flame thrower animation?
		0,											// Damage to apply per tick (fixed point).
		1,											// Delay between frames.
		0,											// Starting frame number.
		33,										// Loop start frame number.
		99,										// Ending frame of loop back.
		66,										// Number of animation stages.
		65535,									// Number of times the animation loops.
		VOC_NONE,								// Sound effect to play.
		ANIM_NONE
	);
	// Electrocution death anim from Tesla coil
	new AnimTypeClass( // ElectricDie
		ANIM_ELECT_DIE,						// Animation number.
		"ELECTRO",									// Data name of animation.
		16,										// Maximum dimension of animation.
		0,											// Biggest animation stage.
		true,										// Theater specific art imagery?
		false,									// Normalized animation rate?
		false,									// Uses white translucent table?
		true,										// Scorches the ground?
		false,									// Forms a crater?
		false,									// Sticks to unit in square?
		true,										// Ground level animation?
		false,									// Translucent colors in this animation?
		false,									// Is this a flame thrower animation?
		0,											// Damage to apply per tick (fixed point).
		1,											// Delay between frames.
		0,											// Starting frame number.
		0,											// Loop start frame number.
		3,											// Ending frame of loop back.
		-1,										// Number of animation stages.
		5,											// Number of times the animation loops.
		VOC_NONE,								// Sound effect to play.
		ANIM_FIRE_MED
	);
	new AnimTypeClass( // Parachute
		ANIM_PARACHUTE,						// Animation number.
		"PARACH",								// Data name of animation.
		32,										// Maximum dimension of animation.
		15,										// Biggest animation stage.
		false,									// Theater specific art imagery?
		false,									// Normalized animation rate?
		false,									// Uses white translucent table?
		false,									// Scorches the ground?
		false,									// Forms a crater?
		false,									// Sticks to unit in square?
		false,									// Ground level animation?
		false,									// Translucent colors in this animation?
		false,									// Is this a flame thrower animation?
		0,											// Damage to apply per tick (fixed point).
		4,											// Delay between frames.
		0,											// Starting frame number.
		7,											// Loop start frame number.
		-1,										// Loopback frame number.
		-1,										// Number of animation stages.
		15,										// Number of times the animation loops.
		VOC_NONE,								// Sound effect to play.
		ANIM_NONE
	);
	// Electrocution death anim from Tesla coil for dog
	new AnimTypeClass( // DogElectricDie
		ANIM_DOG_ELECT_DIE,					// Animation number.
		"ELECTDOG",								// Data name of animation.
		17,										// Maximum dimension of animation.
		0,											// Biggest animation stage.
		false,									// Theater specific art imagery?
		false,									// Normalized animation rate?
		false,									// Uses white translucent table?
		true,										// Scorches the ground?
		false,									// Forms a crater?
		false,									// Sticks to unit in square?
		true,										// Ground level animation?
		false,									// Translucent colors in this animation?
		false,									// Is this a flame thrower animation?
		0,											// Damage to apply per tick (fixed point).
		1,											// Delay between frames.
		0,											// Starting frame number.
		0,											// Loop start frame number.
		3,											// Ending frame of loop back.
		-1,										// Number of animation stages.
		5,											// Number of times the animation loops.
		VOC_NONE,								// Sound effect to play.
		ANIM_FIRE_MED
	);
	new AnimTypeClass( // Corpse1
		ANIM_CORPSE1,							// Animation number.
		"CORPSE1",								// Data name of animation.
		24,										// Maximum dimension of animation.
		1,											// Biggest animation stage.
		true,										// Theater specific art imagery?
		true,										// Normalized animation rate?
		false,									// Uses white translucent table?
		false,									// Scorches the ground?
		false,									// Forms a crater?
		false,									// Sticks to unit in square?
		true,										// Ground level animation?
		true,										// Translucent colors in this animation?
		false,									// Is this a flame thrower animation?
		0,											// Damage to apply per tick (fixed point).
		15,										// Delay between frames.
		0,											// Starting frame number.
		0,											// Loop start frame number.
		0,											// Ending frame of loop back.
		-1,										// Number of animation stages.
		0,											// Number of times the animation loops.
		VOC_NONE,								// Sound effect to play.
		ANIM_NONE
	);
	new AnimTypeClass( // Corpse2
		ANIM_CORPSE2,							// Animation number.
		"CORPSE2",								// Data name of animation.
		24,										// Maximum dimension of animation.
		1,											// Biggest animation stage.
		true,										// Theater specific art imagery?
		true,										// Normalized animation rate?
		false,									// Uses white translucent table?
		false,									// Scorches the ground?
		false,									// Forms a crater?
		false,									// Sticks to unit in square?
		true,										// Ground level animation?
		true,										// Translucent colors in this animation?
		false,									// Is this a flame thrower animation?
		0,											// Damage to apply per tick (fixed point).
		15,										// Delay between frames.
		0,											// Starting frame number.
		0,											// Loop start frame number.
		0,											// Ending frame of loop back.
		-1,										// Number of animation stages.
		0,											// Number of times the animation loops.
		VOC_NONE,								// Sound effect to play.
		ANIM_NONE
	);
	new AnimTypeClass( // Corpse3
		ANIM_CORPSE3,							// Animation number.
		"CORPSE3",								// Data name of animation.
		24,										// Maximum dimension of animation.
		1,											// Biggest animation stage.
		true,										// Theater specific art imagery?
		true,										// Normalized animation rate?
		false,									// Uses white translucent table?
		false,									// Scorches the ground?
		false,									// Forms a crater?
		false,									// Sticks to unit in square?
		true,										// Ground level animation?
		true,										// Translucent colors in this animation?
		false,									// Is this a flame thrower animation?
		0,											// Damage to apply per tick (fixed point).
		15,										// Delay between frames.
		0,											// Starting frame number.
		0,											// Loop start frame number.
		0,											// Ending frame of loop back.
		-1,										// Number of animation stages.
		0,											// Number of times the animation loops.
		VOC_NONE,								// Sound effect to play.
		ANIM_NONE
	);
	new AnimTypeClass( // SputDoor
		ANIM_SPUTDOOR,							// Animation number.
		"SPUTDOOR",								// Data name of animation.
		42,										// Maximum dimension of animation.
		1,											// Biggest animation stage.
		false,									// Theater specific art imagery?
		true,										// Normalized animation rate?
		false,									// Uses white translucent table?
		false,									// Scorches the ground?
		false,									// Forms a crater?
		false,									// Sticks to unit in square?
		false,									// Ground level animation?
		false,									// Translucent colors in this animation?
		false,									// Is this a flame thrower animation?
		0,											// Damage to apply per tick (fixed point).
		1,											// Delay between frames.
		0,											// Starting frame number.
		0,											// Loop start frame number.
		-1,										// Ending frame of loop back.
		-1,										// Number of animation stages.
		1,											// Number of times the animation loops.
		VOC_NONE,								// Sound effect to play.
		ANIM_NONE
	);
	new AnimTypeClass( // AtomBomb
		ANIM_ATOM_BLAST,						// Animation number.
		"ATOMSFX",								// Data name of animation.
		72,										// Maximum dimension of animation.
		19,										// Biggest animation stage.
		false,									// Theater specific art imagery?
		false,									// Normalized animation rate?
		false,									// Uses white translucent table?
		true,										// Scorches the ground?
		true,										// Forms a crater?
		false,									// Sticks to unit in square?
		false,									// Ground level animation?
		false,									// Translucent colors in this animation?
		false,									// Is this a flame thrower animation?
		0,											// Damage to apply per tick (fixed point).
		1,											// Delay between frames.
		0,											// Starting frame number.
		0,											// Loop start frame number.
		0,											// Ending frame of loop back.
		-1,										// Number of animation stages.
		0,											// Number of times the animation loops.
		VOC_NONE,								// Sound effect to play.
		ANIM_NONE
	);
	new AnimTypeClass( // ChronoBox
		ANIM_CHRONO_BOX,						// Animation number.
		"CHRONBOX",								// Data name of animation.
		48,										// Maximum dimension of animation.
		0,											// Biggest animation stage.
		false,									// Theater specific art imagery?
		true,										// Normalized animation rate?
		false,									// Uses white translucent table?
		false,									// Scorches the ground?
		false,									// Forms a crater?
		false,									// Sticks to unit in square?
		false,									// Ground level animation?
		false,									// Translucent colors in this animation?
		false,									// Is this a flame thrower animation?
		0,											// Damage to apply per tick (fixed point).
		2,											// Delay between frames.
		0,											// Starting frame number.
		0,											// Loop start frame number.
		0,											// Ending frame of loop back.
		-1,										// Number of animation stages.
		0,											// Number of times the animation loops.
		VOC_NONE,								// Sound effect to play.
		ANIM_NONE								// Follow up animation.
	);
	new AnimTypeClass( // GPSBox
		ANIM_GPS_BOX,							// Animation number.
		"GPSBOX",								// Data name of animation.
		48,										// Maximum dimension of animation.
		0,											// Biggest animation stage.
		false,									// Theater specific art imagery?
		true,										// Normalized animation rate?
		false,									// Uses white translucent table?
		false,									// Scorches the ground?
		false,									// Forms a crater?
		false,									// Sticks to unit in square?
		false,									// Ground level animation?
		false,									// Translucent colors in this animation?
		false,									// Is this a flame thrower animation?
		0,											// Damage to apply per tick (fixed point).
		2,											// Delay between frames.
		0,											// Starting frame number.
		0,											// Loop start frame number.
		0,											// Ending frame of loop back.
		-1,										// Number of animation stages.
		0,											// Number of times the animation loops.
		VOC_NONE,								// Sound effect to play.
		ANIM_NONE								// Follow up animation.
	);
	new AnimTypeClass( // InvulBox
		ANIM_INVUL_BOX,						// Animation number.
		"INVULBOX",								// Data name of animation.
		48,										// Maximum dimension of animation.
		0,											// Biggest animation stage.
		false,									// Theater specific art imagery?
		true,										// Normalized animation rate?
		false,									// Uses white translucent table?
		false,									// Scorches the ground?
		false,									// Forms a crater?
		false,									// Sticks to unit in square?
		false,									// Ground level animation?
		false,									// Translucent colors in this animation?
		false,									// Is this a flame thrower animation?
		0,											// Damage to apply per tick (fixed point).
		2,											// Delay between frames.
		0,											// Starting frame number.
		0,											// Loop start frame number.
		0,											// Ending frame of loop back.
		-1,										// Number of animation stages.
		0,											// Number of times the animation loops.
		VOC_NONE,								// Sound effect to play.
		ANIM_NONE								// Follow up animation.
	);
	new AnimTypeClass( // ParaBox
		ANIM_PARA_BOX,							// Animation number.
		"PARABOX",								// Data name of animation.
		48,										// Maximum dimension of animation.
		0,											// Biggest animation stage.
		false,									// Theater specific art imagery?
		true,										// Normalized animation rate?
		false,									// Uses white translucent table?
		false,									// Scorches the ground?
		false,									// Forms a crater?
		false,									// Sticks to unit in square?
		false,									// Ground level animation?
		false,									// Translucent colors in this animation?
		false,									// Is this a flame thrower animation?
		0,											// Damage to apply per tick (fixed point).
		2,											// Delay between frames.
		0,											// Starting frame number.
		0,											// Loop start frame number.
		0,											// Ending frame of loop back.
		-1,										// Number of animation stages.
		0,											// Number of times the animation loops.
		VOC_NONE,								// Sound effect to play.
		ANIM_NONE								// Follow up animation.
	);
	new AnimTypeClass( // SonarBox
		ANIM_SONAR_BOX,						// Animation number.
		"SONARBOX",								// Data name of animation.
		48,										// Maximum dimension of animation.
		0,											// Biggest animation stage.
		false,									// Theater specific art imagery?
		true,										// Normalized animation rate?
		false,									// Uses white translucent table?
		false,									// Scorches the ground?
		false,									// Forms a crater?
		false,									// Sticks to unit in square?
		false,									// Ground level animation?
		false,									// Translucent colors in this animation?
		false,									// Is this a flame thrower animation?
		0,											// Damage to apply per tick (fixed point).
		2,											// Delay between frames.
		0,											// Starting frame number.
		0,											// Loop start frame number.
		0,											// Ending frame of loop back.
		-1,										// Number of animation stages.
		0,											// Number of times the animation loops.
		VOC_NONE,								// Sound effect to play.
		ANIM_NONE								// Follow up animation.
	);
	new AnimTypeClass( // Twinkle1
		ANIM_TWINKLE1,							// Animation number.
		"TWINKLE1",								// Data name of animation.
		8,											// Maximum dimension of animation.
		1,											// Biggest animation stage.
		false,									// Theater specific art imagery?
		true,										// Normalized animation rate?
		false,									// Uses white translucent table?
		false,									// Scorches the ground?
		false,									// Forms a crater?
		false,									// Sticks to unit in square?
		false,									// Ground level animation?
		false,									// Translucent colors in this animation?
		false,									// Is this a flame thrower animation?
		0,											// Damage to apply per tick (fixed point).
		1,											// Delay between frames.
		0,											// Starting frame number.
		0,											// Loop start frame number.
		-1,										// Ending frame of loop back.
		-1,										// Number of animation stages.
		1,											// Number of times the animation loops.
		VOC_NONE,								// Sound effect to play.
		ANIM_NONE
	);
	new AnimTypeClass( // Twinkle2
		ANIM_TWINKLE2,							// Animation number.
		"TWINKLE2",								// Data name of animation.
		8,											// Maximum dimension of animation.
		1,											// Biggest animation stage.
		false,									// Theater specific art imagery?
		true,										// Normalized animation rate?
		false,									// Uses white translucent table?
		false,									// Scorches the ground?
		false,									// Forms a crater?
		false,									// Sticks to unit in square?
		false,									// Ground level animation?
		false,									// Translucent colors in this animation?
		false,									// Is this a flame thrower animation?
		0,											// Damage to apply per tick (fixed point).
		1,											// Delay between frames.
		0,											// Starting frame number.
		0,											// Loop start frame number.
		-1,										// Ending frame of loop back.
		-1,										// Number of animation stages.
		1,											// Number of times the animation loops.
		VOC_NONE,								// Sound effect to play.
		ANIM_NONE
	);
	new AnimTypeClass( // Twinkle3
		ANIM_TWINKLE3,							// Animation number.
		"TWINKLE3",								// Data name of animation.
		8,											// Maximum dimension of animation.
		1,											// Biggest animation stage.
		false,									// Theater specific art imagery?
		true,										// Normalized animation rate?
		false,									// Uses white translucent table?
		false,									// Scorches the ground?
		false,									// Forms a crater?
		false,									// Sticks to unit in square?
		false,									// Ground level animation?
		false,									// Translucent colors in this animation?
		false,									// Is this a flame thrower animation?
		0,											// Damage to apply per tick (fixed point).
		1,											// Delay between frames.
		0,											// Starting frame number.
		0,											// Loop start frame number.
		-1,										// Ending frame of loop back.
		-1,										// Number of animation stages.
		1,											// Number of times the animation loops.
		VOC_NONE,								// Sound effect to play.
		ANIM_NONE
	);
	new AnimTypeClass( // Flak
		ANIM_FLAK,								// Animation number.
		"FLAK",									// Data name of animation.
		8,											// Maximum dimension of animation.
		7,											// Biggest animation stage.
		false,									// Theater specific art imagery?
		true,										// Normalized animation rate?
		false,									// Uses white translucent table?
		false,									// Scorches the ground?
		false,									// Forms a crater?
		false,									// Sticks to unit in square?
		false,									// Ground level animation?
		false,									// Translucent colors in this animation?
		false,									// Is this a flame thrower animation?
		0,											// Damage to apply per tick (fixed point).
		1,											// Delay between frames.
		0,											// Starting frame number.
		0,											// Loop start frame number.
		-1,										// Ending frame of loop back.
		-1,										// Number of animation stages.
		1,											// Number of times the animation loops.
		VOC_NONE,								// Sound effect to play.
		ANIM_NONE
	);
	new AnimTypeClass( // WaterExp1
		ANIM_WATER_EXP1,						// Animation number.
		"H2O_EXP1",								// Data name of animation.
		64,										// Maximum dimension of animation.
		3,											// Biggest animation stage.
		false,									// Theater specific art imagery?
		true,										// Normalized animation rate?
		false,									// Uses white translucent table?
		false,									// Scorches the ground?
		false,									// Forms a crater?
		false,									// Sticks to unit in square?
		true,										// Ground level animation?
		false,									// Translucent colors in this animation?
		false,									// Is this a flame thrower animation?
		0,											// Damage to apply per tick (fixed point).
		1,											// Delay between frames.
		0,											// Starting frame number.
		0,											// Loop start frame number.
		-1,										// Ending frame of loop back.
		-1,										// Number of animation stages.
		1,											// Number of times the animation loops.
		VOC_SPLASH,								// Sound effect to play.
		ANIM_NONE
	);
	new AnimTypeClass( // WaterExp2
		ANIM_WATER_EXP2,						// Animation number.
		"H2O_EXP2",								// Data name of animation.
		40,										// Maximum dimension of animation.
		3,											// Biggest animation stage.
		false,									// Theater specific art imagery?
		true,										// Normalized animation rate?
		false,									// Uses white translucent table?
		false,									// Scorches the ground?
		false,									// Forms a crater?
		false,									// Sticks to unit in square?
		true,										// Ground level animation?
		false,									// Translucent colors in this animation?
		false,									// Is this a flame thrower animation?
		0,											// Damage to apply per tick (fixed point).
		1,											// Delay between frames.
		0,											// Starting frame number.
		0,											// Loop start frame number.
		-1,										// Ending frame of loop back.
		-1,										// Number of animation stages.
		1,											// Number of times the animation loops.
		VOC_SPLASH,								// Sound effect to play.
		ANIM_NONE
	);
	new AnimTypeClass( // WaterExp3
		ANIM_WATER_EXP3,						// Animation number.
		"H2O_EXP3",								// Data name of animation.
		32,										// Maximum dimension of animation.
		3,											// Biggest animation stage.
		false,									// Theater specific art imagery?
		true,										// Normalized animation rate?
		false,									// Uses white translucent table?
		false,									// Scorches the ground?
		false,									// Forms a crater?
		false,									// Sticks to unit in square?
		true,										// Ground level animation?
		false,									// Translucent colors in this animation?
		false,									// Is this a flame thrower animation?
		0,											// Damage to apply per tick (fixed point).
		1,											// Delay between frames.
		0,											// Starting frame number.
		0,											// Loop start frame number.
		-1,										// Ending frame of loop back.
		-1,										// Number of animation stages.
		1,											// Number of times the animation loops.
		VOC_SPLASH,								// Sound effect to play.
		ANIM_NONE
	);
	new AnimTypeClass( // CrateArmor
		ANIM_CRATE_ARMOR,						// Animation number.
		"ARMOR",									// Data name of animation.
		48,										// Maximum dimension of animation.
		0,											// Biggest animation stage.
		false,									// Theater specific art imagery?
		true,										// Normalized animation rate?
		false,									// Uses white translucent table?
		false,									// Scorches the ground?
		false,									// Forms a crater?
		false,									// Sticks to unit in square?
		false,									// Ground level animation?
		false,									// Translucent colors in this animation?
		false,									// Is this a flame thrower animation?
		0,											// Damage to apply per tick (fixed point).
		2,											// Delay between frames.
		0,											// Starting frame number.
		0,											// Loop start frame number.
		0,											// Ending frame of loop back.
		-1,										// Number of animation stages.
		0,											// Number of times the animation loops.
		VOC_NONE,								// Sound effect to play.
		ANIM_NONE								// Follow up animation.
	);
	new AnimTypeClass( // CrateSpeed
		ANIM_CRATE_SPEED,						// Animation number.
		"SPEED",									// Data name of animation.
		48,										// Maximum dimension of animation.
		0,											// Biggest animation stage.
		false,									// Theater specific art imagery?
		true,										// Normalized animation rate?
		false,									// Uses white translucent table?
		false,									// Scorches the ground?
		false,									// Forms a crater?
		false,									// Sticks to unit in square?
		false,									// Ground level animation?
		false,									// Translucent colors in this animation?
		false,									// Is this a flame thrower animation?
		0,											// Damage to apply per tick (fixed point).
		2,											// Delay between frames.
		0,											// Starting frame number.
		0,											// Loop start frame number.
		0,											// Ending frame of loop back.
		-1,										// Number of animation stages.
		0,											// Number of times the animation loops.
		VOC_NONE,								// Sound effect to play.
		ANIM_NONE								// Follow up animation.
	);
	new AnimTypeClass( // CrateFPower
		ANIM_CRATE_FPOWER,					// Animation number.
		"FPOWER",								// Data name of animation.
		48,										// Maximum dimension of animation.
		0,											// Biggest animation stage.
		false,									// Theater specific art imagery?
		true,										// Normalized animation rate?
		false,									// Uses white translucent table?
		false,									// Scorches the ground?
		false,									// Forms a crater?
		false,									// Sticks to unit in square?
		false,									// Ground level animation?
		false,									// Translucent colors in this animation?
		false,									// Is this a flame thrower animation?
		0,											// Damage to apply per tick (fixed point).
		2,											// Delay between frames.
		0,											// Starting frame number.
		0,											// Loop start frame number.
		0,											// Ending frame of loop back.
		-1,										// Number of animation stages.
		0,											// Number of times the animation loops.
		VOC_NONE,								// Sound effect to play.
		ANIM_NONE								// Follow up animation.
	);
	new AnimTypeClass( // CrateTQuake
		ANIM_CRATE_TQUAKE,					// Animation number.
		"TQUAKE",								// Data name of animation.
		48,										// Maximum dimension of animation.
		0,											// Biggest animation stage.
		false,									// Theater specific art imagery?
		true,										// Normalized animation rate?
		false,									// Uses white translucent table?
		false,									// Scorches the ground?
		false,									// Forms a crater?
		false,									// Sticks to unit in square?
		false,									// Ground level animation?
		false,									// Translucent colors in this animation?
		false,									// Is this a flame thrower animation?
		0,											// Damage to apply per tick (fixed point).
		2,											// Delay between frames.
		0,											// Starting frame number.
		0,											// Loop start frame number.
		0,											// Ending frame of loop back.
		-1,										// Number of animation stages.
		0,											// Number of times the animation loops.
		VOC_NONE,								// Sound effect to play.
		ANIM_NONE								// Follow up animation.
	);
	new AnimTypeClass( // ParaBomb
		ANIM_PARA_BOMB,						// Animation number.
		"PARABOMB",								// Data name of animation.
		32,										// Maximum dimension of animation.
		8,											// Biggest animation stage.
		false,									// Theater specific art imagery?
		false,									// Normalized animation rate?
		false,									// Uses white translucent table?
		false,									// Scorches the ground?
		false,									// Forms a crater?
		false,									// Sticks to unit in square?
		false,									// Ground level animation?
		false,									// Translucent colors in this animation?
		false,									// Is this a flame thrower animation?
		0,											// Damage to apply per tick (fixed point).
		4,											// Delay between frames.
		0,											// Starting frame number.
		7,											// Loop start frame number.
		-1,										// Loopback frame number.
		-1,										// Number of animation stages.
		15,										// Number of times the animation loops.
		VOC_NONE,								// Sound effect to play.
		ANIM_NONE
	);
	new AnimTypeClass( // MineExp1
		ANIM_MINE_EXP1,						// Animation number.
		"VEH-HIT2",								// Data name of animation.
		21,										// Maximum dimension of animation.
		1,											// Biggest animation stage.
		false,									// Theater specific art imagery?
		true,										// Normalized animation rate?
		false,									// Uses white translucent table?
		false,									// Scorches the ground?
		true,										// Forms a crater?
		false,									// Sticks to unit in square?
		false,									// Ground level animation?
		false,									// Translucent colors in this animation?
		false,									// Is this a flame thrower animation?
		0,											// Damage to apply per tick (fixed point).
		1,											// Delay between frames.
		0,											// Starting frame number.
		0,											// Loop start frame number.
		-1,										// Ending frame of loop back.
		-1,										// Number of animation stages.
		1,											// Number of times the animation loops.
		VOC_MINEBLOW,							// Sound effect to play.
		ANIM_NONE
	);
#ifdef FIXIT_ANTS
	new AnimTypeClass( // AntDeath
		ANIM_ANT_DEATH,						// Animation number.
		"ANTDIE",								// Data name of animation.
		28,										// Maximum dimension of animation.
		1,											// Biggest animation stage.
		false,									// Theater specific art imagery?
		true,										// Normalized animation rate?
		false,									// Uses white translucent table?
		false,									// Scorches the ground?
		false,									// Forms a crater?
		false,									// Sticks to unit in square?
		true,										// Ground level animation?
		true,										// Translucent colors in this animation?
		false,									// Is this a flame thrower animation?
		0,											// Damage to apply per tick (fixed point).
		4,											// Delay between frames.
		0,											// Starting frame number.
		0,											// Loop start frame number.
		-1,										// Ending frame of loop back.
		-1,										// Number of animation stages.
		1,											// Number of times the animation loops.
		VOC_ANTDIE,								// Sound effect to play.
		ANIM_NONE
	);
#endif
}

/***********************************************************************************************
 * AnimTypeClass::One_Time -- Performs one time action for animation types.                    *
 *                                                                                             *
 *    This will load the animation shape data. It is called by the game initialization         *
 *    process.                                                                                 *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   This routine should be called ONLY once.                                        *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   06/02/1994 JLB : Created.                                                                 *
 *=============================================================================================*/
void AnimTypeClass::One_Time(void)
{
	for (AnimType index = ANIM_FIRST; index < ANIM_COUNT; index++) {
		char fullname[_MAX_FNAME+_MAX_EXT];

		AnimTypeClass const & anim = As_Reference(index);

		if (!anim.IsTheater) {

			_makepath(fullname, NULL, NULL, As_Reference(index).IniName, ".SHP");

			#ifndef NDEBUG
				RawFileClass file(fullname);
				if (file.Is_Available()) {
					((void const *&)As_Reference(index).ImageData) = Load_Alloc_Data(file);
				} else {
					((void const *&)As_Reference(index).ImageData) = MFCD::Retrieve(fullname);
				}
			#else
				((void const *&)As_Reference(index).ImageData) = MFCD::Retrieve(fullname);
			#endif
		}
	}
}


/***********************************************************************************************
 * AnimTypeClass::Init -- Load any animation artwork that is theater specific.                 *
 *                                                                                             *
 *    This routine will examine all the animation types and for any that are theater           *
 *    specific, it will fetch a pointer to the artwork appropriate for the theater specified.  *
 *                                                                                             *
 * INPUT:   theater  -- The theater to align the animation artwork with.                       *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   Call this routine when the theater changes.                                     *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   07/06/1996 JLB : Created.                                                                 *
 *=============================================================================================*/
void AnimTypeClass::Init(TheaterType theater)
{
	if (theater != LastTheater) {
		for (AnimType index = ANIM_FIRST; index < ANIM_COUNT; index++) {
			AnimTypeClass const & anim = As_Reference(index);

			if (anim.IsTheater) {
				char fullname[_MAX_FNAME+_MAX_EXT];	// Fully constructed iconset name.
				_makepath(fullname, NULL, NULL, anim.IniName, Theaters[theater].Suffix);
				((void const *&)anim.ImageData) = MFCD::Retrieve(fullname);
			}
		}
	}
}


/***********************************************************************************************
 * Anim_Name -- Fetches the ASCII name of the animation type specified.                        *
 *                                                                                             *
 *    This will convert the animation type specified into a text name. This name can be used   *
 *    for uniquely identifying the animation.                                                  *
 *                                                                                             *
 * INPUT:   anim  -- The anim type to convert to a text string.                                *
 *                                                                                             *
 * OUTPUT:  Returns with a pointer to the ASCII string that identifies this animation.         *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   07/06/1996 JLB : Created.                                                                 *
 *=============================================================================================*/
char const * Anim_Name(AnimType anim)
{
	if (anim == ANIM_NONE) return("");

	return(AnimTypeClass::As_Reference(anim).IniName);
}


/***********************************************************************************************
 * AnimTypeClass::As_Reference -- Fetch a reference to the animation type specified.           *
 *                                                                                             *
 *    This routine will convert the animation type specified into a reference to the           *
 *    animation type class object.                                                             *
 *                                                                                             *
 * INPUT:   type  -- The animation type to convert into a reference.                           *
 *                                                                                             *
 * OUTPUT:  Returns with a reference to the animation type class object.                       *
 *                                                                                             *
 * WARNINGS:   Be sure that the animation type specified is legal. If it isn't then the        *
 *             results of this routine are undefined.                                          *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   07/06/1996 JLB : Created.                                                                 *
 *=============================================================================================*/
AnimTypeClass & AnimTypeClass::As_Reference(AnimType type)
{
	return(* AnimTypes.Ptr(type));
}

