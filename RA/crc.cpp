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

/* $Header: /CounterStrike/CRC.CPP 1     3/03/97 10:24a Joe_bostic $ */
/***********************************************************************************************
 ***              C O N F I D E N T I A L  ---  W E S T W O O D  S T U D I O S               ***
 ***********************************************************************************************
 *                                                                                             *
 *                 Project Name : Command & Conquer                                            *
 *                                                                                             *
 *                    File Name : CRC.CPP                                                      *
 *                                                                                             *
 *                   Programmer : Joe L. Bostic                                                *
 *                                                                                             *
 *                   Start Date : 03/02/96                                                     *
 *                                                                                             *
 *                  Last Update : March 2, 1996 [JLB]                                          *
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 *   CRCEngine::operator() -- Submits one byte of data to the CRC engine.                      *
 *   CRCEngine::operator() -- Submits an arbitrary data block to the CRC engine.               *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "crc.h"


/***********************************************************************************************
 * CRCEngine::operator() -- Submits one byte of data to the CRC engine.                        *
 *                                                                                             *
 *    This routine will take the specified byte of data and submit it to the CRC engine        *
 *    for processing. This routine is designed to be as fast as possible since the typical     *
 *    use of this routine is to feed one of presumably many byte sized chunks of data to the   *
 *    CRC engine.                                                                              *
 *                                                                                             *
 * INPUT:   datum -- One byte of data to submit to the CRC engine.                             *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   If possible, use the buffer/size operator to submit data rather than repeated   *
 *             calls to this routine.                                                          *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   03/02/1996 JLB : Created.                                                                 *
 *=============================================================================================*/
void CRCEngine::operator() (char datum)
{
	StagingBuffer.Buffer[Index++] = datum;

	if (Index == sizeof(int32_t))  {
		CRC = Value();
		StagingBuffer.Composite = 0;
		Index = 0;
	}
}


/***********************************************************************************************
 * CRCEngine::operator() -- Submits an arbitrary data block to the CRC engine.                 *
 *                                                                                             *
 *    This routine will submit the specified block to the CRC engine. The block can be of      *
 *    arbitrary length.                                                                        *
 *                                                                                             *
 * INPUT:   buffer   -- Pointer to the buffer that contains the data. The buffer will not      *
 *                      be modified.                                                           *
 *                                                                                             *
 *          length   -- The length of the buffer (in bytes).                                   *
 *                                                                                             *
 * OUTPUT:  Returns with the current CRC value accumulated so far.                             *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   03/02/1996 JLB : Created.                                                                 *
 *=============================================================================================*/
int32_t CRCEngine::operator() (void const * buffer, int length)
{
	if (buffer != NULL && length > 0)  {
		char const * dataptr = (char const *)buffer;
		int bytes_left = length;

		/*
		**	If there are any leader bytes (needed to fill the staging buffer)
		**	then process those by first using them to fill up the staging
		**	buffer. The bulk of the data block will be processed by the high
		**	speed longword processing loop.
		*/
		while (bytes_left && Buffer_Needs_Data()) {
			operator()(*dataptr);
			dataptr++;
			bytes_left--;
		}

		/*
		**	Perform the fast 'bulk' processing by reading long word sized
		**	data blocks.
		*/
		int32_t const * longptr = (int32_t const *)dataptr;
		int longcount = bytes_left / sizeof(int32_t);		// Whole 'long' elements remaining.
		while (longcount--) {
			CRC = (CRC << 1 | CRC >> 31) + *longptr++;
			bytes_left -= sizeof(int32_t);
		}

		/*
		**	If there are remainder bytes, then process these by adding them
		**	to the staging buffer.
		*/
		dataptr = (char const *)longptr;
		while (bytes_left) {
			operator()(*dataptr);
			dataptr++;
			bytes_left--;
		}
	}

	/*
	**	Return the current CRC value.
	*/
	return(Value());
}
