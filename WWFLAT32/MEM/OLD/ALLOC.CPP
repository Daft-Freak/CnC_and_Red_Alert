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
 **   C O N F I D E N T I A L --- W E S T W O O D   A S S O C I A T E S   **
 ***************************************************************************
 *                                                                         *
 *                 Project Name : Westwood Library                         *
 *                                                                         *
 *                    File Name : ALLOC.CPP                                *
 *                                                                         *
 *                   Programmer : Joe L. Bostic                            *
 *                                                                         *
 *                   Start Date : February 1, 1992                         *
 *                                                                         *
 *                  Last Update : March 9, 1995 [JLB]                      *
 *                                                                         *
 *-------------------------------------------------------------------------*
 * Functions:                                                              *
 *   Alloc -- Allocates system RAM.                                        *
 *   Ram_Free -- Determines the largest free chunk of RAM.                 *
 *   Free -- Free an Alloc'ed block of RAM.                                *
 *   Resize_Alloc -- Change the size of an allocated block.                *
 *   Heap_Size -- Size of the heap we have.                                *
 *   Total_Ram_Free -- Total amount of free RAM.                           *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include <malloc.h>
#include <string.h>
#include <stdlib.h>
#include <dos.h>
#include <bios.h>
#include <stdio.h>


#ifndef WWMEM_H
#include "wwmem.h"
#endif


extern "C" unsigned long Largest_Mem_Block ( void ) ;

/*
** Define the equates necessary to call a DPMI interrupt.
*/
#define	DPMI_INT									0x0031
#define	DPMI_LOCK_MEM							0x0600
#define	DPMI_UNLOCK_MEM						0x0601
#define  LOGGING									FALSE
/*=========================================================================*/
/* The following PRIVATE functions are in this file:                       */
/*=========================================================================*/


/*= = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = =*/

unsigned long MinRam=0L;		// Record of least memory at worst case.
unsigned long MaxRam=0L;		// Record of total allocated at worst case.
static unsigned long TotalRam = 0L;
static unsigned long Memory_Calls = 0L;
		
void (*Memory_Error)(void) = NULL;

/***************************************************************************
 * DPMI_LOCK -- handles locking a block of DPMI memory                     *
 *                                                                         *
 * INPUT:                                                                  *
 *                                                                         *
 * OUTPUT:                                                                 *
 *                                                                         *
 * WARNINGS:                                                               *
 *                                                                         *
 * HISTORY:                                                                *
 *   06/23/1995 PWG : Created.                                             *
 *=========================================================================*/
#include"mono.h"
void DPMI_Lock(VOID const *ptr, long const size)
{
	union REGS regs;
	struct SREGS sregs;

	/*
	** Lock memory
	** AX = 0x600
	** BX:CX = starting linear address of memory to lock
	** SI:DI = size of region to lock (in bytes)
	** - If Failure, carry flag is set.
	*/
	memset (&regs, 0 ,sizeof(regs));
	segread (&sregs);
	regs.x.eax = DPMI_LOCK_MEM;
	regs.x.ebx = ((long)ptr & 0xffff0000) >> 16;
	regs.x.ecx = ((long)ptr & 0x0000ffff);
	regs.x.esi = ((long)size & 0xffff0000) >> 16;
	regs.x.edi = ((long)size & 0x0000ffff);
	int386x (DPMI_INT, &regs, &regs, &sregs);			// call DPMI
//	if (regs.x.cflag) {
//	}
#if(0)
	char *temp = (char *)ptr;
	char hold;
	for (int lp = 0; lp < size; lp += 2048) {
		hold = *temp;
		temp += 2048;
	}
#endif

}

/***************************************************************************
 * DPMI_UNLOCK -- Handles unlocking a locked block of DPMI                 *
 *                                                                         *
 * INPUT:                                                                  *
 *                                                                         *
 * OUTPUT:                                                                 *
 *                                                                         *
 * WARNINGS:                                                               *
 *                                                                         *
 * HISTORY:                                                                *
 *   06/23/1995 PWG : Created.                                             *
 *=========================================================================*/
void DPMI_Unlock(void const *ptr, long const size)
{
	union REGS regs;
	struct SREGS sregs;

	/*
	** Unlock the memory
	*/
	memset (&regs, 0 ,sizeof(regs));
	segread (&sregs);
	regs.x.eax = DPMI_UNLOCK_MEM;							// DPMI function to call
	regs.x.ebx = ((long)ptr & 0xffff0000) >> 16;
	regs.x.ecx = ((long)ptr & 0x0000ffff);
	regs.x.esi = ((long)size & 0xffff0000) >> 16;
	regs.x.edi = ((long)size & 0x0000ffff);
	int386x (DPMI_INT, &regs, &regs, &sregs);			// call DPMI
//	if (regs.x.cflag) {
//	}

}

/***************************************************************************
 * Alloc -- Allocates system RAM.                                          *
 *                                                                         *
 *    This is the basic RAM allocation function.  It is used for all       *
 *    memory allocations needed by the system or the main program.         *
 *                                                                         *
 * INPUT:   bytes_to_alloc -- LONG value of the number of bytes to alloc.  *
 *                                                                         *
 *          flags          -- Memory allocation control flags.             *
 *             MEM_NORMAL: No special flags.                               *
 *             MEM_CLEAR:  Zero out memory block.                        	*
 *             MEM_NEW:		Called by a new.                                *
 *                                                                         *
 * OUTPUT:  Returns with pointer to allocated block.  If NULL was returned *
 *          it indicates a failure to allocate.  Note: NULL will never be  *
 *          returned if the standard library allocation error routine is   *
 *          used.                                                          *
 *                                                                         *
 * WARNINGS:   If you replace the standard memory allocation error routine *
 *             and make it so that Alloc CAN return with a NULL, be sure   *
 *             and check for this in your code.                            *
 *                                                                         *
 * HISTORY:                                                                *
 *   09/03/1991 JLB : Documented.                                          *
 *   08/09/1993 JLB : Updated with EMS memory support.                     *
 *   04/28/1994 JAW : Updated to 32bit Protected mode.                     *
 *   03/09/1995 JLB : Fixed                                                * 
 *=========================================================================*/
void *Alloc(unsigned long bytes_to_alloc, MemoryFlagType flags)
{
	union REGS		regs ;
	struct SREGS 	sregs ;
	unsigned char *retval=NULL;	// Pointer to allocated block.
	unsigned long	original_size;	// Original allocation size.
	unsigned long	bytesfree;		// Number of free bytes.
	long 				*longptr=NULL;	// Pointer used to store selector

	/* 
	** Save the original allocated space size so that we can clear the
	** exact amount of RAM if they specified MEM_CLEAR.
	*/
	original_size = bytes_to_alloc;

	/*
	** Reserve one byte for the header of the memory we allocated.
	** We will store the flags variable there for later use.
	*/
	bytes_to_alloc += (flags & MEM_LOCK) ? 5 : 1;

	/*
	**	Initialize the total ram available value.
	*/
	if (!TotalRam) {
		TotalRam = Total_Ram_Free(MEM_NORMAL);
	}


	// Try to allocate the memory out of the protected mode memory
	// chain if we did not require a real mode allocation.  If this 
	// fails we will have to try to allocate it out of real mode memory.
	// Real mode memory is a last resort because some types of applications
	// require real mode memory.
	if (!(flags & MEM_REAL)) {
		retval = (unsigned char*)malloc(bytes_to_alloc);
	}

	// Try to allocate the memory out of the real mode memory using DPMI
	// service 0x100.  Note that retval will be null if we are requesting
	// real mode memory so that we do not have to explicitly check for the
	// real mode flag.  Remember we need to reserve room for the dos 
	// selector value at the beginning of our allocated block so rather than
	// adding fifteen and rounding, we need to add 19 and round.
	if (!retval) {
		flags = (MemoryFlagType)(flags | MEM_REAL);
		regs.x.eax = 0x100;
		regs.x.ebx = (bytes_to_alloc + 19) >> 4;
		if (regs.x.ebx & 0xFFFF0000) {
			retval = NULL;
		} else {
			segread ( & sregs ) ;
			int386x ( 0x31 , & regs, & regs , & sregs	) ;
			if (regs.x.cflag)
				retval = NULL;
			else {
				longptr		= (long *)(((regs.x.eax & 0xFFFF) << 4)+ 1);
				*longptr++	= regs.x.edx & 0xFFFF;
				retval		= (unsigned char *)longptr;
			}
		}
	}

	// If the alloc failed then we need to signify a memory error.
	if (retval == NULL) {
		if(Memory_Error != NULL)
			Memory_Error();
		return NULL;		
	}

	// If the memory needs to be DPMI locked then we should store the
	// original size in the header before we store the flags.
	if (flags & MEM_LOCK) {
		longptr		= (long *)retval;
		*longptr++	= original_size;
		retval		= (unsigned char *)longptr;
	}


	// Now that we know the alloc was sucessful (and for an extra byte
	// more than the user wanted) we need to stick in the memory flags.
	*retval++ 		= flags;

	// If the memory needed to be DPMI locked then set it up so it
	// is locked.
	if (flags & MEM_LOCK) {
		DPMI_Lock(retval, original_size);
		
	}


	/* Clear the space if they wanted it clear */

	if (flags & MEM_CLEAR) {
		unsigned char	*ptr;		// Working memory block pointer.

		ptr = retval;
		memset(ptr, '\0', original_size);
	}

	bytesfree = Total_Ram_Free(MEM_NORMAL);
	if (bytesfree < MinRam) {
		MinRam = bytesfree;
	}
	if (TotalRam-bytesfree > MaxRam) {
		MaxRam = TotalRam-bytesfree;
	}

	Memory_Calls++;

#if(LOGGING)
	int val = _heapchk();

	FILE *file = fopen("mem.txt","at");
	fprintf(file, "%P Alloc size = %d, Actual Size = %d, flags = %d, heap = %d\n", 
						retval,
						original_size,
						bytes_to_alloc,
						flags,
						val);
	fclose(file);
#endif

	return(retval);
}


/***************************************************************************
 * Free -- Free an Alloc'ed block of RAM.                                  *
 *                                                                         *
 * FUNCTION:                                                               *
 *                                                                         *
 * INPUT:       A pointer to a block of RAM from Alloc.                    *
 *                                                                         *
 * OUTPUT:      None.                                                      *
 *                                                                         *
 * WARNINGS:    Don't use this for an Alloc_Block'ed RAM block.            *
 *                                                                         *
 * HISTORY:                                                                *
 *   05/25/1990     : Created.                                             *
 ***************************************************************************/
void Free(void const *pointer)
{
	union REGS		regs ;
	struct SREGS 	sregs ;

	void const *original = pointer;

	if (pointer) {
		/*
		** Get a pointer to the flags that we stored off.
		*/
		char *byteptr	= ((char *)pointer) - 1;

		/*
		** Check to see if this was locked me and if it was unlock it.
		*/
		if (*byteptr & MEM_LOCK) {
			long *longptr = ((long *)byteptr) - 1;
			DPMI_Unlock(pointer, *longptr);
			pointer = (void *)longptr;
		} else
			pointer = (void *)byteptr;

#if(LOGGING)		
		int val = _heapchk();
		FILE *file = fopen("mem.txt","at");
		fprintf(file, "%P Free flags = %d, Heap = %d\n", 
							original,
							*byteptr, 
							val);
		fclose(file);
#endif

		// If the pointer is a real mode pointer than it will point to the
		// first megabyte of system memory.  If it does than we need to 
		// use DPMI to free it.
		if (*byteptr & MEM_REAL) {
			regs.x.eax	= 0x101;
			regs.x.edx	= *(((long *)pointer) - 1);
			segread ( & sregs ) ;
			int386x(0x31, &regs, &regs, &sregs);
		} else {
			free((void *)pointer);
		}
		Memory_Calls--;
	}
}


/***************************************************************************
 * Resize_Alloc -- Change the size of an allocated block.                  *
 *                                                                         *
 *    This routine will take a previously allocated block and change its   *
 *    size without unnecessarily altering its contents.                    *
 *                                                                         *
 * INPUT:   pointer  -- Pointer to the original memory allocation.         *
 *                                                                         *
 *          new_size -- Size in bytes that it will be converted to.        *
 *                                                                         *
 * OUTPUT:  Returns with a pointer to the new allocation.                  *
 *                                                                         *
 * WARNINGS:   ???                                                         *
 *                                                                         *
 * HISTORY:                                                                *
 *   02/01/1992 JLB : Commented.                                           *
 *=========================================================================*/
void *Resize_Alloc(void *original_ptr, unsigned long new_size_in_bytes)
{

	unsigned long *temp;
	unsigned long diff, flags;

	temp = (unsigned long*)original_ptr;

	/* ReAlloc the space */
	temp = (unsigned long *)realloc(temp, new_size_in_bytes);
	if (temp == NULL) {
		if(Memory_Error != NULL)
			Memory_Error();
		return NULL;		
	}			   

	return(temp);
}


/***************************************************************************
 * Ram_Free -- Determines the largest free chunk of RAM.                   *
 *                                                                         *
 *    Use this routine to determine the largest free chunk of available    *
 *    RAM for allocation.  It also performs a check of the memory chain.   *
 *                                                                         *
 * INPUT:   none                                                           *
 *                                                                         *
 * OUTPUT:  Returns with the size of the largest free chunk of RAM.        *
 *                                                                         *
 * WARNINGS:   This does not return the TOTAL memory free, only the        *
 *             largest free chunk.                                         *
 *                                                                         *
 * HISTORY:                                                                *
 *   09/03/1991 JLB : Commented.                                           *
 *=========================================================================*/
long Ram_Free(MemoryFlagType)
{
//	return(_memmax());
	return Largest_Mem_Block();
}


/***************************************************************************
 * Heap_Size -- Size of the heap we have.                                  *
 *                                                                         *
 *                                                                         *
 *                                                                         *
 * INPUT:                                                                  *
 *                                                                         *
 * OUTPUT:                                                                 *
 *                                                                         *
 * WARNINGS:                                                               *
 *                                                                         *
 * HISTORY:                                                                *
 *   06/21/1994 SKB : Created.                                             *
 *=========================================================================*/
long Heap_Size(MemoryFlagType )
{
	if (!TotalRam) {
		TotalRam = Total_Ram_Free(MEM_NORMAL);
	}
	return(TotalRam);
}


/***************************************************************************
 * Total_Ram_Free -- Total amount of free RAM.                             *
 *                                                                         *
 *                                                                         *
 *                                                                         *
 * INPUT:                                                                  *
 *                                                                         *
 * OUTPUT:                                                                 *
 *                                                                         *
 * WARNINGS:                                                               *
 *                                                                         *
 * HISTORY:                                                                *
 *   06/21/1994 SKB : Created.                                             *
 *   03/09/1995 JLB : Uses prerecorded heap size maximum.                  * 
 *=========================================================================*/
long Total_Ram_Free(MemoryFlagType )
{
//	return(_memavl());
	return Largest_Mem_Block () ;
}

