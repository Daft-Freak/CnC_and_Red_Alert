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

/* $Header:   F:\projects\c&c\vcs\code\rawfile.cpv   2.18   16 Oct 1995 16:50:54   JOE_BOSTIC  $ */
/***********************************************************************************************
 ***             C O N F I D E N T I A L  ---  W E S T W O O D   S T U D I O S               ***
 ***********************************************************************************************
 *                                                                                             *
 *                 Project Name : Westwood Library                                             *
 *                                                                                             *
 *                    File Name : RAWFILE.CPP                                                  *
 *                                                                                             *
 *                   Programmer : Joe L. Bostic                                                *
 *                                                                                             *
 *                   Start Date : August 8, 1994                                               *
 *                                                                                             *
 *                  Last Update : October 18, 1994   [JLB]                                     *
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 *   RawFileClass::Close -- Perform a closure of the file.                                     *
 *   RawFileClass::Create -- Creates an empty file.                                            *
 *   RawFileClass::Delete -- Deletes the file object from the disk.                            *
 *   RawFileClass::Error -- Handles displaying a file error message.                           *
 *   RawFileClass::Is_Available -- Checks to see if the specified file is available to open.   *
 *   RawFileClass::Open -- Assigns name and opens file in one operation.                       *
 *   RawFileClass::Open -- Opens the file object with the rights specified.                    *
 *   RawFileClass::RawFileClass -- Simple constructor for a file object.                       *
 *   RawFileClass::Read -- Reads the specified number of bytes into a memory buffer.           *
 *   RawFileClass::Seek -- Reposition the file pointer as indicated.                           *
 *   RawFileClass::Set_Name -- Manually sets the name for a file object.                       *
 *   RawFileClass::Size -- Determines size of file (in bytes).                                 *
 *   RawFileClass::Write -- Writes the specified data to the buffer specified.                 *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


#include	"function.h"
#include	<stdlib.h>
#include	<stdio.h>
#include	<string.h>
#ifndef PORTABLE
#include	<direct.h>
#include	<fcntl.h>
#include	<io.h>
#include	<dos.h>
#include	<share.h>
#endif

#include "wwlib32.h"
#include	"compat.h"
#include	"rawfile.h"

#ifndef PORTABLE
extern GraphicBufferClass	SeenBuff;
extern GraphicBufferClass	HidPage;
#endif

/***********************************************************************************************
 * RawFileClass::Error -- Handles displaying a file error message.                             *
 *                                                                                             *
 *    Display an error message as indicated. If it is allowed to retry, then pressing a key    *
 *    will return from this function. Otherwise, it will exit the program with "exit()".       *
 *                                                                                             *
 * INPUT:   error    -- The error number (same as the DOSERR.H error numbers).                 *
 *                                                                                             *
 *          canretry -- Can this routine exit normally so that retrying can occur? If this is  *
 *                      false, then the program WILL exit in this routine.                     *
 *                                                                                             *
 *          filename -- Optional filename to report with this error. If no filename is         *
 *                      supplied, then no filename is listed in the error message.             *
 *                                                                                             *
 * OUTPUT:  none, but this routine might not return at all if the "canretry" parameter is      *
 *          false or the player pressed ESC.                                                   *
 *                                                                                             *
 * WARNINGS:   This routine may not return at all. It handles being in text mode as well as    *
 *             if in a graphic mode.                                                           *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   10/17/1994 JLB : Created.                                                                 *
 *=============================================================================================*/
void RawFileClass::Error(int error, int canretry, char const * filename)
{
#ifndef PORTABLE
	char	message[256];				// Staging buffer for error message string.

	/*
	**	Build the complete text of the error message. This text is used in either the graphic
	**	mode or the text mode version.
	*/
#ifdef GERMAN
	strcpy(message, "DATEIFEHLER");
#else
#ifdef FRENCH
	strcpy(message, "ERREUR DE FICHIER");
#else
	strcpy(message, "FILE ERROR");
#endif
#endif
	if (filename) {
		strcat(message, "(");
		strcat(message, filename);
		strcat(message, ")");
	}
	strcat(message, ": ");
//BG: Borland only	strcat(message, _sys_errlist[error]);
	strcat(message, strerror(error) );
	strcat(message, ". ");

	/*
	**	If it can't properly handle displaying the error message in the
	**	current graphic mode, then this forces the error to become non
	**	recoverable. Go into text mode and proceed.
	*/
	if (!FontPtr && GraphicMode != TXT_MODE) {
		Set_Video_Mode(RESET_MODE);
		canretry = false;
		GraphicMode = TXT_MODE;
	}

	/*
	**	Add the text explaining the valid actions to take.
	*/
	if (canretry) {
		if (GraphicMode == TXT_MODE) strcat(message, "\n");
#ifdef GERMAN
		strcat(message, " Beliebige Taste drücken für erneuten Versuch.");
		if (GraphicMode == TXT_MODE) strcat(message, "\n");
		strcat(message, " <ESC> drücken, um das Programm zu verlassen.");
#else
#ifdef FRENCH
		strcat(message, " Appuyez sur une touche pour recommencer.");
		if (GraphicMode == TXT_MODE) strcat(message, "\n");
		strcat(message, " Appuyez sur Echap pour quitter le programme.");
#else
		strcat(message, " Press any key to retry.");
		if (GraphicMode == TXT_MODE) strcat(message, "\n");
		strcat(message, " Press <ESC> to exit program.");
#endif
#endif
		if (GraphicMode == TXT_MODE) strcat(message, "\n");
	} else {
		if (GraphicMode == TXT_MODE) strcat(message, "\n");
#ifdef GERMAN
		strcat(message, " Beliebige Taste drücken, um das Programm zu verlassen.");
#else
#ifdef FRENCH
		strcat(message, " Appuyez sur une touche pour quitter le programme.");
#else
		strcat(message, " Press any key to exit program.");
#endif
#endif
		if (GraphicMode == TXT_MODE) strcat(message, "\n");
	}

	/*
	**	In text mode, the error handler is very simple. It just prints the error message
	**	to the screen and waits for a response.
	*/
	if (GraphicMode == TXT_MODE) {
		int	input;

		/*
		**	Display the error message and wait for a response.
		*/
		printf(message);
		Keyboard::Clear();
		input = Keyboard::Get();

		/*
		**	Check for input. If the ESC key was pressed or if retrying is not allowed,
		**	then exit the program. Otherwise, return from this routine for a retry
		**	attempt.
		*/
		if (input == KN_ESC || !canretry) {
			Prog_End();
			exit(EXIT_FAILURE);
		}

	} else {

		/*
		**	The graphic mode version of the error handler will display a simple message
		**	box on the screen. If the palette is black at this point, then the error will
		**	be invisible. For more thorough and pleasing results, you should replace this
		**	virtual function with one of your own, that is more aware of the environment
		**	in which is exists.
		*/
		void			*background;		// Pointer to background saving buffer.
		GraphicBufferClass * 	oldpage;				// Copy of old logic page.
		int			oldwindow;			// Copy of old window number.
		void const	*oldfont;			// Copy of old font pointer.
		int			oldspacing;			// Old font X spacing.

		/*
		**	Setup display in preparation for printing the error message.
		*/
		oldpage = Set_Logic_Page(SeenBuff);
		oldwindow = Change_Window(ErrorWindow);
		oldfont = Set_Font(FontPtr);
		oldspacing = FontXSpacing; FontXSpacing = 0;
		Hide_Mouse();

		/*
		**	Try to allocate a storage buffer for the background to the
		**	error window.
		*/
		background = new char [Size_Of_Region(WinW<<3, WinH)];

		/*
		**	If there is memory for the background storage, then save the
		**	screen image area to that buffer.
		*/
		if (background) {
			SeenBuff.To_Buffer(WinX<<3, WinY, WinW<<3, WinH, background, Size_Of_Region(WinW<<3, WinH));
		}

		/*
		**	Draw a rudimentary box.
		*/
		New_Window();
		LogicPage->Draw_Rect(WinX<<3, WinY, (WinX+WinW)<<3, WinY+WinH, WinC);

		/*
		** shrinks window down one byte in all directions.
		*/
		WindowList[Window][WINDOWX] += 1;
		WindowList[Window][WINDOWY] += 1<<3;
		WindowList[Window][WINDOWWIDTH] -= 1<<1;
		WindowList[Window][WINDOWHEIGHT] -= 1<<4;
		Change_Window(Window);
		WinCx = WinCy = 0;

		Window_Print(message);
		Keyboard::Clear();

		/*
		**	Check for input. If the ESC key was pressed or if retrying is not allowed,
		**	then exit the program. Otherwise, return from this routine for a retry
		**	attempt.
		*/
		int input = Keyboard::Get();
		if (input == KN_ESC || !canretry) {
			Prog_End();
			exit(EXIT_FAILURE);
		}

		/*
		**	Restore the window back to its original size.
		*/
		WindowList[Window][WINDOWX] -= 1;
		WindowList[Window][WINDOWY] -= 1<<3;
		WindowList[Window][WINDOWWIDTH] += 1<<1;
		WindowList[Window][WINDOWHEIGHT] += 1<<4;
		Change_Window(Window);
		WinCx = WinCy = 0;

		/*
		**	If the background was saved off, then restore it.
		*/
		if (background) {
			Buffer_To_Page(WinX<<3, WinY, WinW<<3, WinH, background, SeenBuff);
			delete [] background;
		}

		/*
		**	Restore the system global settings to original values.
		*/
		Show_Mouse();
		Change_Window(oldwindow);
		Set_Font(oldfont);
		Set_Logic_Page(oldpage);
		FontXSpacing = oldspacing;
	}
#endif
}


/***********************************************************************************************
 * RawFileClass::RawFileClass -- Simple constructor for a file object.                         *
 *                                                                                             *
 *    This constructor is called when a file object is created with a supplied filename, but   *
 *    not opened at the same time. In this case, an assumption is made that the supplied       *
 *    filename is a constant string. A duplicate of the filename string is not created since   *
 *    it would be wasteful in that case.                                                       *
 *                                                                                             *
 * INPUT:   filename -- The filename to assign to this file object.                            *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   10/17/1994 JLB : Created.                                                                 *
 *=============================================================================================*/
RawFileClass::RawFileClass(char const *filename) : 
#ifdef PORTABLE
	Handle(NULL),
#else
	Handle(-1),
#endif
	Filename(filename), 
	Allocated(false)
{
}


RawFileClass::~RawFileClass(void) 
{
	if (Allocated && Filename) {
		free((char *)Filename); 
	}
	Allocated = false; 
	Filename = 0;
}


/***********************************************************************************************
 * RawFileClass::Set_Name -- Manually sets the name for a file object.                         *
 *                                                                                             *
 *    This routine will set the name for the file object to the name specified. This name is   *
 *    duplicated in free store. This allows the supplied name to be a temporarily constructed  *
 *    text string. Setting the name in this fashion doesn't affect the closed or opened state  *
 *    of the file.                                                                             *
 *                                                                                             *
 * INPUT:   filename -- The filename to assign to this file object.                            *
 *                                                                                             *
 * OUTPUT:  Returns with a pointer to the allocated copy of this filename. This pointer is     *
 *          guaranteed to remain valid for the duration of this file object or until the name  *
 *          is changed -- whichever is sooner.                                                 *
 *                                                                                             *
 * WARNINGS:   Because of the allocation this routine must perform, memory could become        *
 *             fragmented.                                                                     *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   10/17/1994 JLB : Created.                                                                 *
 *=============================================================================================*/
char const * RawFileClass::Set_Name(char const *filename)
{
	if (Filename && Allocated) {
		free((char*)Filename);
		Filename = 0;
		Allocated = false;
	}

	if (!filename) return(NULL);

	Filename = strdup(filename);
	if (!Filename) {
		Error(ENOMEM, false, filename);
	}
	Allocated = true;
	return(Filename);
}


/***********************************************************************************************
 * RawFileClass::Open -- Assigns name and opens file in one operation.                         *
 *                                                                                             *
 *    This routine will assign the specified filename to the file object and open it at the    *
 *    same time. If the file object was already open, then it will be closed first. If the     *
 *    file object was previously assigned a filename, then it will be replaced with the new    *
 *    name. Typically, this routine is used when an anonymous file object has been crated and  *
 *    now it needs to be assigned a name and opened.                                           *
 *                                                                                             *
 * INPUT:   filename -- The filename to assign to this file object.                            *
 *                                                                                             *
 *          rights   -- The open file access rights to use.                                    *
 *                                                                                             *
 * OUTPUT:  bool; Was the file opened? The return value of this is moot, since the open file   *
 *          is designed to never return unless it succeeded.                                   *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   10/17/1994 JLB : Created.                                                                 *
 *=============================================================================================*/
int RawFileClass::Open(char const *filename, int rights)
{
	Set_Name(filename);
	return(Open(rights));
}


/***********************************************************************************************
 * RawFileClass::Open -- Opens the file object with the rights specified.                      *
 *                                                                                             *
 *    This routine is used to open the specified file object with the access rights indicated. *
 *    This only works if the file has already been assigned a filename. It is guaranteed, by   *
 *    the error handler, that this routine will always return with success.                    *
 *                                                                                             *
 * INPUT:   rights   -- The file access rights to use when opening this file. This is a        *
 *                      combination of READ and/or WRITE bit flags.                            *
 *                                                                                             *
 * OUTPUT:  bool; Was the file opened successfully? This will always return true by reason of  *
 *          the error handler.                                                                 *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   10/17/1994 JLB : Created.                                                                 *
 *=============================================================================================*/
int RawFileClass::Open(int rights)
{
	Close();

	/*
	**	Verify that there is a filename associated with this file object. If not, then this is a
	**	big error condition.
	*/
	if (!Filename) {
		Error(ENOENT, false);
	}

	/*
	**	Record the access rights used for this open call. These rights will be used if the
	**	file object is duplicated.
	*/
	Rights = rights;

	/*
	**	Repetatively try to open the file. Abort if a fatal error condition occurs.
	*/
	for (;;) {

		/*
		**	Try to open the file according to the access rights specified.
		*/
#ifdef PORTABLE
		Handle = IO_Open_File(Filename, rights);
		if(!Handle)
			return false;
#else
		Hard_Error_Occured = 0;
		switch (rights) {

			/*
			**	If the access rights are not recognized, then report this as
			**	an invalid access code.
			*/
			default:
				errno = EINVAL;
				break;

			case READ:
				_dos_open(Filename, O_RDONLY|SH_DENYNO, &Handle);
				break;

			case WRITE:
				_dos_creat(Filename, 0, &Handle);
				break;

			case READ|WRITE:
				_dos_open(Filename, O_RDWR|O_CREAT|SH_DENYWR, &Handle);
				break;
		}

		/*
		**	If the handle indicates the file is not open, then this is an error condition.
		**	For the case of the file cannot be found, then allow a retry. All other cases
		**	are fatal.
		*/
		if (Handle < 0) {

			/*
			**	If this flag is set, then some hard error occurred. Just assume that the error
			**	is probably a removed CD-ROM and allow a retry.
			*/
			if (Hard_Error_Occured) {
				Error(Hard_Error_Occured, true, Filename);
			} else {
				if (errno == ENOENT) {
					Error(ENOENT, true, Filename);
				} else {
					Error(errno, false, Filename);
				}
			}
			continue;
		}
#endif
		break;
	}
	return(true);
}


/***********************************************************************************************
 * RawFileClass::Is_Available -- Checks to see if the specified file is available to open.     *
 *                                                                                             *
 *    This routine will examine the disk system to see if the specified file can be opened     *
 *    or not. Use this routine before opening a file in order to make sure that is available   *
 *    or to perform other necessary actions.                                                   *
 *                                                                                             *
 * INPUT:   force -- Should this routine keep retrying until the file becomes available? If    *
 *                   in this case it doesn't become available, then the program will abort.    *
 *                                                                                             *
 * OUTPUT:  bool; Is the file available to be opened?                                          *
 *                                                                                             *
 * WARNINGS:   Depending on the parameter passed in, this routine may never return.            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   10/18/1994 JLB : Created.                                                                 *
 *=============================================================================================*/
int RawFileClass::Is_Available(int forced)
{
#ifdef PORTABLE
	void *file;
#else
	int	file;		// Working file handle.
#endif
	int	open_failed;

	/*
	**	If the file is already open, then is must have already passed the availability check.
	**	Return true in this case.
	*/
	if (Is_Open()) {
		return(true);
	}

	/*
	**	If this is a forced check, then go through the normal open channels, since those
	**	channels ensure that the file must exist.
	*/
	if (forced) {
		RawFileClass::Open(READ);
		RawFileClass::Close();
		return(true);
	}

	/*
	**	Perform a raw open of the file. If this open fails for ANY REASON, including a missing
	**	CD-ROM, this routine will return a failure condition. In all but the missing file
	**	condition, go through the normal error recover channels.
	*/
	for (;;) {
#ifdef PORTABLE
		file = IO_Open_File(Filename, READ);
		if (!file) {
			// retry with lowercase name for case-sensitive fs
			char *lower_name = strlwr(strdup(Filename));
			file = IO_Open_File(lower_name, READ);
		
			if(file) {
				// if successful, replace the filename with the working one
				if(Allocated)
					free((char *)Filename);
		
				((char *&)Filename) = lower_name;
			} else
				free(lower_name);
		}

		if(!file)
			return false;
#else
		Hard_Error_Occured = 0;
		open_failed = _dos_open(Filename, O_RDONLY|SH_DENYNO, &file);

		/*
		**	If DOS reports that everything is just fine except that the file is not present,
		**	then return with this fact. Any other case will fall through to the error handler
		**	routine.
		*/
		if (open_failed && errno == ENOENT) {
			return(false);
		}

		/*
		** If we got an access error it could be because there is no cd in
		** the drive.  Call the error handler but allow a continue if it
		** returns.
		*/
		if (open_failed && errno == EACCES) {
//			Error(errno, true, Filename);
//			continue;
			return(false);
		}

		/*
		**	If the file could not be found, then return with this information. If a more
		**	serious error occurred, then display the error message and abort.
		*/
		if (Hard_Error_Occured) {
			return(false);
//			Error(Hard_Error_Occured, true, Filename);
//			continue;
		} else {
			if (open_failed) {
				/*
				**	An unhandled error condition is fatal. Display the error message and then
				**	abort.
				*/
//				Error(errno, false, Filename);
				return(false);
			}
		}
#endif
		break;
	}

	/*
	**	Since the file could be opened, then close it and return that the file exists.
	*/
#ifdef PORTABLE
	IO_Close_File(file);
#else
	if (_dos_close(file)) {
		Error(errno, false, Filename);
	}
#endif
	return(true);
}


/***********************************************************************************************
 * RawFileClass::Close -- Perform a closure of the file.                                       *
 *                                                                                             *
 *    Close the file object. In the rare case of an error, handle it as appropriate.           *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   Some rare error conditions may cause this routine to abort the program.         *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   10/18/1994 JLB : Created.                                                                 *
 *=============================================================================================*/
void RawFileClass::Close(void)
{

	/*
	**	If the file is open, then close it. If the file is already closed, then just return. This
	**	isn't considered an error condition.
	*/
	if (Is_Open()) {

		for (;;) {
			/*
			**	Close the file. If there was an error in the close operation -- abort.
			*/
#ifdef PORTABLE
			IO_Close_File(Handle);
#else
			Hard_Error_Occured = 0;
			if (_dos_close(Handle)) {

				/*
				**	By definition, this error can only be a bad file handle. This a fatal condition
				**	of course, so abort with an error message.
				*/
				Error(errno, false, Filename);
			}

			/*
			**	In the condition (if it is even possible) of a hard error occurring, then
			**	assume it is the case of missing media. Display an error message and try
			**	again if indicated.
			*/
			if (Hard_Error_Occured) {
				Error(Hard_Error_Occured, true, Filename);
				continue;
			}
#endif
			break;
		}

		/*
		**	At this point the file must have been closed. Mark the file as empty and return.
		*/
#ifdef PORTABLE
		Handle = NULL;
#else
		Handle = -1;
#endif
	}
}


/***********************************************************************************************
 * RawFileClass::Read -- Reads the specified number of bytes into a memory buffer.             *
 *                                                                                             *
 *    This routine will read the specified number of bytes and place the data into the buffer  *
 *    indicated. It is legal to call this routine with a request for more bytes than are in    *
 *    the file. This condition can result in fewer bytes being read than requested. Determine  *
 *    this by examining the return value.                                                      *
 *                                                                                             *
 * INPUT:   buffer   -- Pointer to the buffer to read data into. If NULL is passed, no read    *
 *                      is performed.                                                          *
 *                                                                                             *
 *          size     -- The number of bytes to read. If NULL is passed, then no read is        *
 *                      performed.                                                             *
 *                                                                                             *
 * OUTPUT:  Returns with the number of bytes read into the buffer. If this number is less      *
 *          than requested, it indicates that the file has been exhausted.                     *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   10/18/1994 JLB : Created.                                                                 *
 *=============================================================================================*/
long RawFileClass::Read(void *buffer, long size)
{
	long	bytesread = 0;			// Running count of the number of bytes read into the buffer.
	int	opened = false;		// Was the file opened by this routine?
	int	readresult;

	/*
	**	If the file isn't opened, open it. This serves as a convenience
	**	for the programmer.
	*/
	if (!Is_Open()) {

		/*
		**	The error check here is moot. Open will never return unless it succeeded.
		*/
		if (!Open(READ)) {
			return(0);
		}
		opened = true;
	}

#ifdef PORTABLE
	size_t actual = 0;
	IO_Read_File(Handle, buffer, size, actual);
	bytesread = actual;
#else
	/*
	**	Read the file in convenient chunk sizes. When the actual number
	**	of bytes read does not match the desired, then assume that the file
	**	is exhausted and bail. This loop was adjusted to take into
	**	consideration the fact that "read" returns a SIGNED value whereas
	**	it takes an UNSIGNED value as the byte count.
	*/
	while (size) {
		unsigned	desired;		// Bytes desired to be read this pass.
		unsigned actual;		// Actual number of bytes read.

		/*
		**	Break the read request into chunks no bigger than the low level DOS read
		**	can handle.
		*/
		desired = size;

		Hard_Error_Occured = 0;
		readresult = _dos_read(Handle, buffer, desired, &actual);

		/*
		**	If a hard error occurred, then assume that it is the case of the CD-ROM or
		**	floppy media having been removed. Display the error and retry as directed.
		*/
		if (Hard_Error_Occured) {
			Error(Hard_Error_Occured, true, Filename);
			continue;			// Not technically needed, but to be consistent...
		} else {

			/*
			**	If negative one is returned from the read operation, then this indicates
			**	either a bad file number or invalid access. These are fatal conditions, so
			**	display the error and then abort.
			*/
			if (readresult != 0) {
				Error(errno, false, Filename);
			} else {

				/*
				**	No error occurred during the read. Adjust the pointers and size counters and
				**	loop again if more data is needed to be read.
				*/
				buffer = Add_Long_To_Pointer(buffer, actual);
				bytesread += actual;
				size -= actual;
				if (actual != desired) break;		// No more data?
			}
		}
	}
#endif
	/*
	**	Close the file if it was opened by this routine and return
	**	the actual number of bytes read into the buffer.
	*/
	if (opened) Close();
	return(bytesread);
}


/***********************************************************************************************
 * RawFileClass::Write -- Writes the specified data to the buffer specified.                   *
 *                                                                                             *
 *    This routine will write the data specified to the file.                                  *
 *                                                                                             *
 * INPUT:   buffer   -- The buffer that holds the data to write.                               *
 *                                                                                             *
 *          size     -- The number of bytes to write to the file.                              *
 *                                                                                             *
 * OUTPUT:  Returns with the number of bytes written to the file. This routine catches the     *
 *          case of a disk full condition, so this routine will always return with the number  *
 *          matching the size request.                                                         *
 *                                                                                             *
 * WARNINGS:   A fatal file condition could cause this routine to never return.                *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   10/18/1994 JLB : Created.                                                                 *
 *=============================================================================================*/
long RawFileClass::Write(void const *buffer, long size)
{
	long	bytesread = 0;
	int	opened = false;		// Was the file manually opened?
	int	writeresult;

	/*
	**	Check to open status of the file. If the file is open, then merely write to
	**	it. Otherwise, open the file for writing and then close the file when the
	**	output is finished.
	*/
	if (!Is_Open()) {
		if (!Open(WRITE)) {
			return(0);
		}
		opened = true;
	}

#ifdef PORTABLE
	size_t actual = 0;
	IO_Write_File(Handle, buffer, size, actual);
	bytesread = actual;
#else
	/*
	**	Write the data to the file in chunks no bigger than what the low level DOS write
	**	can handle.
	*/
	while (size) {
		unsigned	desired;		// Bytes desired to be write this pass.
		unsigned actual;		// Actual number of bytes written.

		Hard_Error_Occured = 0;
//		desired = (unsigned)MIN(size, Transfer_Block_Size());
		desired = size;
		writeresult = _dos_write(Handle, buffer, desired, &actual);

		/*
		**	If a hard error occurred, then assume it is the case of the media being
		**	removed. Print the error message an retry as directed.
		*/
		if (Hard_Error_Occured) {
			Error(Hard_Error_Occured, true, Filename);
			continue;			// Not technically needed, but to be consistent...
		} else {

			/*
			**	If negative one is returned by the DOS read, then this indicates a bad file
			**	handle or invalid access. Either condition is fatal -- display error condition
			**	and abort.
			*/
			if (writeresult != 0) {
				Error(errno, false, Filename);
			} else {

				/*
				**	A successful write occurred. Update pointers and byte counter as appropriate.
				*/
				buffer = Add_Long_To_Pointer((void *)buffer, actual);
				bytesread += actual;
				size -= actual;

				/*
				**	If the actual bytes written is less than requested, assume this is a case of
				**	the disk being full. Consider this a fatal error condition.
				*/
				if (actual != desired) {
					Error(ENOSPC, false, Filename);
				}
			}
		}
	}
#endif
	/*
	**	If this routine had to open the file, then close it before returning.
	*/
	if (opened) {
		Close();
	}

	/*
	**	Return with the number of bytes written. This will always be the number of bytes
	**	requested, since the case of the disk being full is caught by this routine.
	*/
	return(bytesread);
}


/***********************************************************************************************
 * RawFileClass::Seek -- Reposition the file pointer as indicated.                             *
 *                                                                                             *
 *    Use this routine to move the filepointer to the position indicated. It can move either   *
 *    relative to current position or absolute from the beginning or ending of the file. This  *
 *    routine will only return if it successfully performed the seek.                          *
 *                                                                                             *
 * INPUT:   pos   -- The position to seek to. This is interpreted as relative to the position  *
 *                   indicated by the "dir" parameter.                                         *
 *                                                                                             *
 *          dir   -- The relative position to relate the seek to. This can be either SEEK_SET  *
 *                   for the beginning of the file, SEEK_CUR for the current position, or      *
 *                   SEEK_END for the end of the file.                                         *
 *                                                                                             *
 * OUTPUT:  This routine returns the position that the seek ended up at.                       *
 *                                                                                             *
 * WARNINGS:   If there was a file error, then this routine might never return.                *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   10/18/1994 JLB : Created.                                                                 *
 *=============================================================================================*/
long RawFileClass::Seek(long pos, int dir)
{
	/*
	**	If the file isn't opened, then this is a fatal error condition.
	*/
	if (!Is_Open()) {
		Error(EBADF, false, Filename);
	}

#ifdef PORTABLE
	pos = IO_Seek_File(Handle, pos, dir);
#else
	/*
	**	Keep trying to seek until a non-retry condition occurs.
	*/
	for (;;) {

		/*
		**	Perform the low level seek on the file.
		*/
		Hard_Error_Occured = 0;
		pos = lseek(Handle, pos, dir);

		/*
		**	If a hard error occurred, then assume that it is the case of removed media. Display
		**	error message and retry.
		*/
		if (Hard_Error_Occured) {
			Error(Hard_Error_Occured, true, Filename);
			continue;
		} else {

			/*
			**	A negative one indicates a fatal error with the seek operation. Display error
			**	condition and then abort.
			*/
			if (pos == -1) {
				Error(errno, false, Filename);
			}
		}
		break;
	}
#endif
	/*
	**	Return with the new position of the file. This will range between zero and the number of
	**	bytes the file contains.
	*/
	return(pos);
}


/***********************************************************************************************
 * RawFileClass::Size -- Determines size of file (in bytes).                                   *
 *                                                                                             *
 *    Use this routine to determine the size of the file. The file must exist or this is an    *
 *    error condition.                                                                         *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  Returns with the number of bytes in the file.                                      *
 *                                                                                             *
 * WARNINGS:   This routine handles error conditions and will not return unless the file       *
 *             exists and can successfully be queried for file length.                         *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   10/18/1994 JLB : Created.                                                                 *
 *=============================================================================================*/
long RawFileClass::Size(void)
{
	long	size = 0;

	/*
	**	If the file is open, then proceed normally.
	*/
	if (Is_Open()) {

#ifdef PORTABLE
		return IO_Get_File_Size(Handle);
#else
		/*
		**	Repetitively try to determine the file size until a fatal error condition or success
		**	is achieved.
		*/
		for (;;) {
			Hard_Error_Occured = 0;
			size = filelength(Handle);

			/*
			**	If a hard error occurred, then assume it is the case of removed media. Display an
			**	error condition and allow retry.
			*/
			if (Hard_Error_Occured) {
				Error(Hard_Error_Occured, true, Filename);
				continue;
			} else {
				if (size == -1) {
					Error(errno, false, Filename);
				}
			}
			break;
		}
#endif
	} else {

		/*
		**	If the file wasn't open, then open the file and call this routine again. Count on
		**	the fact that the open function must succeed.
		*/
		if (Open()) {
			size = Size();

			/*
			**	Since we needed to open the file we must remember to close the file when the
			**	size has been determined.
			*/
			Close();
		}
	}
	return(size);
}


/***********************************************************************************************
 * RawFileClass::Create -- Creates an empty file.                                              *
 *                                                                                             *
 *    This routine will create an empty file from the file object. The file object's filename  *
 *    must already have been assigned before this routine will function.                       *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  bool; Was the file successfully created? This routine will always return true.     *
 *                                                                                             *
 * WARNINGS:   A fatal error condition could occur with this routine. Especially if the disk   *
 *             is full or a read-only media was selected.                                      *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   10/18/1994 JLB : Created.                                                                 *
 *=============================================================================================*/
int RawFileClass::Create(void)
{
	Close();
	if (Open(WRITE)) {
		Close();
		return(true);
	}
	return(false);
}


/***********************************************************************************************
 * RawFileClass::Delete -- Deletes the file object from the disk.                              *
 *                                                                                             *
 *    This routine will delete the file object from the disk. If the file object doesn't       *
 *    exist, then this routine will return as if it had succeeded (since the effect is the     *
 *    same).                                                                                   *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  bool; Was the file deleted? If the file was already missing, the this value will   *
 *                be false.                                                                    *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   10/18/1994 JLB : Created.                                                                 *
 *=============================================================================================*/
int RawFileClass::Delete(void)
{
	/*
	**	If the file was open, then it must be closed first.
	*/
	Close();

	/*
	**	If there is no filename associated with this object, then this indicates a fatal error
	**	condition. Report this and abort.
	*/
	if (!Filename) {
		Error(ENOENT, false);
	}

#ifdef PORTABLE
	if(!IO_Delete_File(Filename))
		return false;
#else
	/*
	**	Repetitively try to delete the file if possible. Either return with success, or
	**	abort the program with an error.
	*/
	for (;;) {

		/*
		**	If the file is already missing, then return with this fact. No action is necessary.
		**	This can occur as this section loops if the file exists on a floppy and the floppy
		**	was removed, the file deleted on another machine, and then the floppy was
		**	reinserted. Admittedly, this is a rare case, but is handled here.
		*/
		if (!Is_Available()) {
			return(false);
		}

		Hard_Error_Occured = 0;
		if (remove(Filename) == -1) {

			/*
			**	If a hard error occurred, then assume that the media has been removed. Display
			**	error message and retry as directed.
			*/
			if (Hard_Error_Occured) {
				Error(Hard_Error_Occured, true, Filename);
				continue;
			}

			/*
			**	If at this point, DOS says the file doesn't exist, then just exit with this
			**	fact. It should have been caught earlier, but in any case, this is a legal
			**	condition.
			*/
			if (errno == ENOENT) break;

			/*
			**	The only way it can reach this point is if DOS indicates that access is denied
			**	on the file. This occurs when trying to delete a file on a read-only media such
			**	as a CD-ROM. Report this as a fatal error and then abort.
			*/
			Error(errno, false, Filename);
		}
		break;
	}
#endif
	/*
	**	DOS reports that the file was successfully deleted. Return with this fact.
	*/
	return(true);
}
