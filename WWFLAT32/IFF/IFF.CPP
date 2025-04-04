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
 *                    File Name : IFF.C                                    *
 *                                                                         *
 *                   Programmer : Joe L. Bostic                            *
 *                                                                         *
 *                   Start Date : May 16, 1991                             *
 *                                                                         *
 *                  Last Update : April 19, 1994   [SKB]                   *
 *                                                                         *
 *                                                                         *
 * IFF reader code designed for loading pictures (ILBM or PBM).            *
 *                                                                         *
 *-------------------------------------------------------------------------*
 * Functions:                                                              *
 *   Close_Iff_File -- Closes an IFF file handle.                          *
 *   Get_Iff_Chunk_Size -- Get the size of the given IFF chunk.            *
 *   Open_Iff_File -- Opens an IFF file for reading.                       *
 *   Read_Iff_Chunk -- Reads a chunk from an IFF file.                     *
 *   Write_Iff_Chunk -- Writes an IFF chuck out.                           *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
 
#include <wwstd.h>
#include "iff.h"
#include "file.h"

#define ID_FORM			MAKE_ID('F','O','R','M')

/***************************************************************************
 * OPEN_IFF_FILE -- Opens an IFF file for reading.                         *
 *                                                                         *
 *    This function will open an IFF file for reading.  It will perform    *
 *    a the simple validity test of checking the first four bytes to make  *
 *    sure they are "FORM".  The value returned is the filehandle of the   *
 *    opened file.                                                         *
 *                                                                         *
 * INPUT:   filename - ASCII name of the IFF file to be opened.            *
 *                                                                         *
 * OUTPUT:  Returns the filehandle.  If there is an error or the file      *
 *          is not an IFF FORM then -1 will be returned.                   *
 *                                                                         *
 * WARNINGS:   You are responsible for error handling if this function     *
 *             returns -1 (not an IFF file).                               *
 *                                                                         *
 * HISTORY:                                                                *
 *   05/16/1991 JLB : Created.                                             *
 *   04/19/1994 SKB : Update to 32 bit library.                            *
 *=========================================================================*/
WORD cdecl Open_Iff_File(BYTE const *filename)
{
	WORD	fh;		// File handle.
	LONG	type;		// IFF file type.


	/* We want to be able to open the file for READ | WRITE, but we do not
	   want the Open_File to create it.  So check to see if it exists before
		the Open_File */

//	fh = Open_File(filename, READ);		// Open the source file for READ
//	Close_File(fh);

	//fh = Open_File(filename, READ | WRITE);	// Open the source file again
	fh = Open_File(filename, READ);	// Open the source file again

	//	Validate that it is a FORM type.

	Read_File(fh, &type, 4L);

	if (type == ID_FORM) {

		//	The file is valid (so far).  Position the read so that the actual
		//	IFF file type code can be read.

		Seek_File(fh, 4L, SEEK_CUR);		// Skip the filesize bytes.

	} else {

		// This is NOT an IFF file.  Close the source file and return with
		//	the error code.
		Close_File(fh);
		fh = ERROR;
	}
	return fh;
}


/***************************************************************************
 * CLOSE_IFF_FILE -- Closes an IFF file handle.                            *
 *                                                                         *
 *    The routine will close the file that was opened with the             *
 *    Open_Iff_File() function.                                            *
 *                                                                         *
 * INPUT:   fh - File handle that was returned from Open_Iff_File().       *
 *                                                                         *
 * OUTPUT:  none                                                           *
 *                                                                         *
 * WARNINGS:   none                                                        *
 *                                                                         *
 * HISTORY:                                                                *
 *   05/16/1991 JLB : Created.                                             *
 *   04/19/1994 SKB : Update to 32 bit library.                            *
 *=========================================================================*/
VOID cdecl Close_Iff_File(WORD fh)
{
	if (fh != ERROR) Close_File(fh);
}


/***************************************************************************
 * GET_IFF_CHUNK_SIZE -- Get the size of the given IFF chunk.              *
 *                                                                         *
 * INPUT:      WORD file handle to open IFF file, LONG id to get size of   *
 *                                                                         *
 * OUTPUT:     LONG size of the chunk or 0L if it was not found            *
 *                                                                         *
 * WARNINGS:   none                                                        *
 *                                                                         *
 * HISTORY:                                                                *
 *   06/03/1991  CY : Created.                                             *
 *   04/19/1994 SKB : Update to 32 bit library.                            *
 *=========================================================================*/
ULONG cdecl Get_Iff_Chunk_Size(WORD fh, LONG id)
{
	LONG	form;					// Chunk iff form name.
	LONG	chunksize;			// Size of the chunk.
	BYTE	first_iteration;	// Check once the current chunk name


	first_iteration = TRUE;

	for (;;) {
		if (Read_File(fh, &form, 4L) != 4L && !first_iteration) break;


		if (Read_File(fh, (BYTE *) &chunksize, 4L) != 4L && !first_iteration) break;

#if(IBM)
		chunksize = Reverse_LONG(chunksize);
#endif

		if (id == form) {
			Seek_File(fh, -8L, SEEK_CUR);			// Seek back to the start of
			return(chunksize);						// the chunk & return size
		} else {

			if (first_iteration) {
				Seek_File(fh, 12L, SEEK_SET);		// Start at beginning of file.
				first_iteration = FALSE;			// Don't do this again
			} else {

				/* Otherwise, go to the next chunk in the file */

				chunksize = (chunksize + 1) & 0xFFFFFFFEL;
				Seek_File(fh, chunksize, SEEK_CUR);
			}
		}
	}

	return(0L);
}


/***************************************************************************
 * READ_IFF_CHUNK -- Reads a chunk from an IFF file.                       *
 *                                                                         *
 *    Once an IFF file is opened, various chunks must be read from it.     *
 *    This routine will search through the IFF file and load in the        *
 *    specified chunk.  It will scan through the entire file when          *
 *    searching for the chunk.  It will load the FIRST chunk of the given  *
 *    type.                                                                *
 *                                                                         *
 * INPUT:   fh       - File handle of IFF file.                            *
 *                                                                         *
 *          id       - Chunk ID code.                                      *
 *                                                                         *
 *          buffer   - Pointer to buffer to load the chunk.                *
 *                                                                         *
 *          maxsize  - Maximum data bytes to read.                         *
 *                                                                         *
 * OUTPUT:     Returns with the number of bytes read from the chunk.       *
 *             If 0 is returned, this indicates that the chunk wasn't      *
 *             found.                                                      *
 *                                                                         *
 * WARNINGS:   none                                                        *
 *                                                                         *
 * HISTORY:                                                                *
 *   05/16/1991 JLB : Created.                                             *
 *   04/19/1994 SKB : Update to 32 bit library.                            *
 *=========================================================================*/
ULONG cdecl Read_Iff_Chunk(WORD fh, LONG id, VOID *buffer, ULONG maxsize)
{
	LONG	form;					// Chunk iff form name.
	ULONG	chunksize;			// Size of the chunk.
	BYTE	first_iteration;	// Check once the current chunk name

	first_iteration = TRUE;

	for (;;) {
		if (Read_File(fh, &form, 4L) != 4L && !first_iteration) break;

		if (Read_File(fh, (BYTE *) &chunksize, 4L) != 4L && !first_iteration) break;

#if(IBM)
		chunksize = Reverse_LONG(chunksize);
#endif

		if (id == form) {

			maxsize = MIN(maxsize, chunksize);
			Read_File(fh, buffer, maxsize);		// Read the buffer.

			chunksize = (chunksize + 1) & 0xFFFFFFFEL;
			if (maxsize < chunksize) {
				Seek_File(fh, chunksize - maxsize, SEEK_CUR);
			}
			return(maxsize);
		} else {

			if (first_iteration) {
				Seek_File(fh, 12L, SEEK_SET);		// Start at beginning of file.
				first_iteration = FALSE;			// Don't do this again

			} else {

				/* Otherwise, go to the next chunk in the file */

				chunksize = (chunksize + 1) & 0xFFFFFFFEL;
				Seek_File(fh, chunksize, SEEK_CUR);
			}
		}
	}

	return(0L);
}



/***************************************************************************
 * WRITE_IFF_CHUNK -- Writes an IFF chuck out.                             *
 *                                                                         *
 * INPUT:                                                                  *
 *                                                                         *
 * OUTPUT:                                                                 *
 *                                                                         *
 * WARNINGS:                                                               *
 *                                                                         *
 * HISTORY:                                                                *
 *   04/19/1994 SKB : Created.                                             *
 *=========================================================================*/
VOID cdecl Write_Iff_Chunk(WORD file, LONG id, VOID *buffer, LONG length)
{
	LONG	pos;		// Current position in the IFF file.
	LONG	oldpos;	// Record of start of chunk offset.
	LONG	endpos;	// end of file offset before we write our data
	LONG	value;
	BOOL	odd;		// Is length odd?
	BYTE	pad = 0;	// Optional padding byte for even sized chunks.

	/*
	** Get the current end of file (before we write more data to the file)
	*/
	pos 	 = Seek_File (file, 0L, SEEK_CUR);
	endpos = Seek_File (file, 0L, SEEK_END);
	Seek_File (file, pos, SEEK_SET);

	if (length) {
		value = id;
		odd = (WORD)length & 0x01;

		Write_File(file, &value, 4L);
		oldpos = Seek_File(file, 0L, SEEK_CUR);
		Write_File(file, &value, 4L);
		Write_File(file, buffer, length);
		pos = Seek_File(file, 0L, SEEK_CUR);
		if (odd) {
			Write_File(file, &pad, 1L);
		}

		/*
		**	Update the chunk size LONG.
		*/
		Seek_File(file, oldpos, SEEK_SET);
		value = IFFize_LONG((pos - oldpos)-4);
		Write_File(file, &value, 4L);

		/*
		**	Update the file size LONG. if we are not just overwriting existing data
		*/
		// (MCC)
		if ( endpos < pos ) {
			Seek_File(file, 4L, SEEK_SET);
			value = IFFize_LONG((pos+odd) - 8);
			Write_File(file, &value, 4L);
		}

		/*
		**	Return to end of file.
		*/
		Seek_File(file, 0L, SEEK_END);
	}
}


