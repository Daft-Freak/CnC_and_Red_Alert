// parts of winstub that didn't depend on windows
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


#include	"function.h"


void output(short,short)
{}


ThemeType OldTheme = THEME_NONE;
extern bool InMovie;

/***********************************************************************************************
 * Focus_Loss -- this function is called when a library function detects focus loss            *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * INPUT:    Nothing                                                                           *
 *                                                                                             *
 * OUTPUT:   Nothing                                                                           *
 *                                                                                             *
 * WARNINGS: None                                                                              *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *    2/1/96 2:10PM ST : Created                                                               *
 *=============================================================================================*/

void Focus_Loss(void)
{
	if (SoundOn){
		if (OldTheme == THEME_NONE){
			OldTheme = Theme.What_Is_Playing();
		}
	}
	Theme.Stop();
	Stop_Primary_Sound_Buffer();
	if (WWMouse) WWMouse->Clear_Cursor_Clip();
}

void Focus_Restore(void)
{
	Restore_Cached_Icons();
	Map.Flag_To_Redraw(true);
	Start_Primary_Sound_Buffer(TRUE);

	if (!InMovie){
		Theme.Queue_Song(OldTheme);
		OldTheme = THEME_NONE;
	}

	if (WWMouse) WWMouse->Set_Cursor_Clip();
#ifndef PORTABLE
	VisiblePage.Clear();
	HiddenPage.Clear();
#endif
}

unsigned char	*VQPalette;
long				VQNumBytes;
unsigned long	VQSlowpal;
bool				VQPaletteChange = false;


extern "C"{
	void __cdecl SetPalette(unsigned char *palette, long numbytes, unsigned long slowpal);
}



void Flag_To_Set_Palette(unsigned char *palette, long numbytes, unsigned long slowpal)
{
	VQPalette = palette;
	VQNumBytes = numbytes;
	VQSlowpal = slowpal;
	VQPaletteChange = true;
}



void Check_VQ_Palette_Set(void)
{
	if (VQPaletteChange) {
		SetPalette(VQPalette, VQNumBytes, VQSlowpal);
		VQPaletteChange = false;
	}
}





void __cdecl SetPalette(unsigned char *palette, long, unsigned long)
{
	for (int i=0 ; i<256*3 ; i++) {
		*(palette+i)&=63;
	}
	Increase_Palette_Luminance(palette , 15 , 15 , 15 ,63);

	if (PalettesRead) {
		memcpy (&PaletteInterpolationTable[0][0] , InterpolatedPalettes[PaletteCounter++] , 65536);
	}
	Set_Palette(palette);
}


GraphicBufferClass* Read_PCX_File(char const* name, char* Palette, void *Buff, long Size);



/***********************************************************************************************
 * Load_Title_Screen -- loads the title screen into the given video buffer                     *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * INPUT:    screen name                                                                       *
 *           video buffer                                                                      *
 *           ptr to buffer for palette                                                         *
 *                                                                                             *
 * OUTPUT:   Nothing                                                                           *
 *                                                                                             *
 * WARNINGS: None                                                                              *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *    7/5/96 11:30AM ST : Created                                                              *
 *=============================================================================================*/

void Load_Title_Screen(char const*name, GraphicViewPortClass *video_page, unsigned char *palette)
{

	GraphicBufferClass *load_buffer;


	load_buffer = Read_PCX_File (name, (char*)palette, NULL, 0);

	if (load_buffer) {
		load_buffer->Blit(*video_page);
		delete load_buffer;
	}
}



#include "filepcx.h"

/***************************************************************************
 * READ_PCX_FILE -- read a pcx file into a Graphic Buffer                  *
 *                                                                         *
 *	GraphicBufferClass* Read_PCX_File (char* name, char* palette ,void *Buff, long size );	*
 *  																								*
 *                                                                         *
 * INPUT: name is a NULL terminated string of the format [xxxx.pcx]        *
 *        palette is optional, if palette != NULL the the color palette of *
 *					 the pcx file will be place in the memory block pointed	   *
 *               by palette.																*
 *			 Buff is optional, if Buff == NULL a new memory Buffer		 		*
 *					 will be allocated, otherwise the file will be placed 		*
 *					 at location pointed by Buffer;										*
 *			Size is the size in bytes of the memory block pointed by Buff		*
 *				  is also optional;															*                                                                         *
 * OUTPUT: on success a pointer to a GraphicBufferClass containing the     *
 *         pcx file, NULL otherwise.                                       *
 *																									*
 * WARNINGS:                                                               *
 *         Appears to be a comment-free zone                               *
 *                                                                         *
 * HISTORY:                                                                *
 *   05/03/1995 JRJ : Created.                                             *
 *   04/30/1996 ST : Tidied up and modified to use CCFileClass             *
 *=========================================================================*/

#define	POOL_SIZE 2048
#define	READ_CHAR()  *file_ptr++ ; \
							 if ( file_ptr	>= & pool [ POOL_SIZE ]	) { \
								 file_handle.Read (pool , POOL_SIZE ); \
								 file_ptr = pool ; \
							 }


GraphicBufferClass* Read_PCX_File(char const* name, char* palette, void *Buff, long Size)
{
	unsigned					i, j;
	unsigned					rle;
	unsigned					color;
	unsigned					scan_pos;
	char						*file_ptr;
	int						width;
	int						height;
	char						*buffer;
	PCX_HEADER				header;
	RGB						*pal;
	char						pool [POOL_SIZE];
	GraphicBufferClass	*pic;

	CCFileClass file_handle(name);

	if (!file_handle.Is_Available()) return (NULL);

	file_handle.Open(READ);

	file_handle.Read (&header, sizeof (PCX_HEADER));

	if (header.id != 10 &&  header.version != 5 && header.pixelsize != 8 ) return NULL ;

	width = header.width - header.x + 1;
	height = header.height - header.y + 1;

	if (Buff) {
    	buffer = (char *)Buff;
    	i = Size / width;
    	height = MIN (i - 1, height);
    	pic = new GraphicBufferClass(width, height, buffer, Size);
    	if ( !(pic && pic->Get_Buffer()) ) return NULL ;
	} else {
    	pic = new GraphicBufferClass(width, height, NULL, width*(height+4));
    	if ( !(pic && pic->Get_Buffer()) ) return NULL ;
	}

	buffer = (char *) pic->Get_Buffer();
	file_ptr = pool ;
	file_handle.Read (pool , POOL_SIZE);

	if ( header.byte_per_line != width ) {

		for ( scan_pos = j = 0 ; j < height ; j ++, scan_pos += width ) {
			for ( i = 0 ; i < width ; ) {
				rle = READ_CHAR ();
				if ( rle > 192 ) {
					rle -= 192 ;
					color =	READ_CHAR (); ;
					memset ( buffer + scan_pos + i , color , rle );
					i += rle;
				} else {
					*(buffer+scan_pos + i++ ) = (char)rle;
				}
			}
      	}

		if ( i == width ) rle = READ_CHAR ();
		if ( rle > 192 )  rle = READ_CHAR ();

	} else {

		for ( i = 0 ; i < width * height ; ) {
  			rle = READ_CHAR ();
  			rle &= 0xff;
  			if ( rle > 192 ) {
        		rle -= 192 ;
        		color = READ_CHAR ();
  				memset ( buffer + i , color , rle );
        		i += rle ;
     		} else {
				*(buffer + i++) = (char)rle;
			}
		}
	}

	if ( palette ) {
		file_handle.Seek (- (256 * sizeof ( RGB )) , SEEK_END );
		file_handle.Read (palette , 256L * sizeof ( RGB ));

		pal = ( RGB * ) palette;
		for (i = 0 ; i < 256 ; i ++) {
			pal ->red	>>= 2;
			pal ->green	>>= 2;
			pal ->blue	>>= 2;
			pal ++ ;
		}
	}

	file_handle.Close();
	return pic;
}

