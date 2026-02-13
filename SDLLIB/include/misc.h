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

/***************************************************************************
 **     C O N F I D E N T I A L --- W E S T W O O D   S T U D I O S       **
 ***************************************************************************
 *                                                                         *
 *                 Project Name : 32 bit library                           *
 *                                                                         *
 *                    File Name : MISC.H                                   *
 *                                                                         *
 *                   Programmer : Scott K. Bowen                           *
 *                                                                         *
 *                   Start Date : August 3, 1994                           *
 *                                                                         *
 *                  Last Update : August 3, 1994   [SKB]                   *
 *                                                                         *
 *-------------------------------------------------------------------------*
 * Functions:                                                              *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#ifndef MISC_H
#define MISC_H

/*========================= C++ Routines ==================================*/


/*=========================================================================*/
/* The following prototypes are for the file: DDRAW.CPP							*/
/*=========================================================================*/
bool 		Set_Video_Mode(void *hwnd, int w, int h, int bits_per_pixel);
void 		Wait_Blit(void);

/*
** Pointer to function to call if we detect a focus loss
*/
extern	void (*Misc_Focus_Loss_Function)(void);
/*
** Pointer to function to call if we detect a surface restore
*/
extern	void (*Misc_Focus_Restore_Function)(void);

class SurfaceMonitorClass {
	public:
		bool	SurfacesRestored;
};

extern	SurfaceMonitorClass	AllSurfaces;				//List of all direct draw surfaces


extern bool				OverlappedVideoBlits;	// Can video driver blit overlapped regions?

/*=========================================================================*/
/* The following prototypes are for the file: EXIT.CPP							*/
/* Prog_End Must be supplied by the user program in startup.cpp				*/
/*=========================================================================*/
void Prog_End(void);

/*=========================================================================*/
/* The following prototypes are for the file: DELAY.CPP							*/
/*=========================================================================*/
void Delay(int duration);

// extras for TD
#ifdef TD
inline void randomize(){srand(time(NULL));}
int IRandom(int minval, int maxval);
void IRandom_Seed(int seed);
uint8_t Random();

void Shake_Screen(int shakes);
inline void Wait_Vert_Blank() {Video_End_Frame();}

void Convert_RGB_To_HSV(unsigned int r, unsigned int g, unsigned int b, unsigned int *h, unsigned int *s, unsigned int *v);
void Convert_HSV_To_RGB(unsigned int h, unsigned int s, unsigned int v, unsigned int *r, unsigned int *g, unsigned int *b);
#endif


/*========================= Assembly Routines ==============================*/

#ifdef __cplusplus
extern "C" {
#endif



void * Build_Fading_Table(void const *palette, void *dest, long int color, long int frac);
/*=========================================================================*/
/* The following prototype is for the file: CRC.ASM							*/
/*=========================================================================*/

long Calculate_CRC(void *buffer, long length);


extern int Clip_Rect ( int * x , int * y , int * dw , int * dh ,
	       	   			 int width , int height ) ;
extern int Confine_Rect ( int * x , int * y , int dw , int dh ,
	      	      			 int width , int height ) ;



#ifdef __cplusplus
}
#endif

/*=========================================================================*/

#endif // MISC_H

