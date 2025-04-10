;
;	Command & Conquer Red Alert(tm)
;	Copyright 2025 Electronic Arts Inc.
;
;	This program is free software: you can redistribute it and/or modify
;	it under the terms of the GNU General Public License as published by
;	the Free Software Foundation, either version 3 of the License, or
;	(at your option) any later version.
;
;	This program is distributed in the hope that it will be useful,
;	but WITHOUT ANY WARRANTY; without even the implied warranty of
;	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
;	GNU General Public License for more details.
;
;	You should have received a copy of the GNU General Public License
;	along with this program.  If not, see <http://www.gnu.org/licenses/>.
;

;***************************************************************************
;**     C O N F I D E N T I A L --- W E S T W O O D   S T U D I O S       **
;***************************************************************************
;*                                                                         *
;*                 Project Name : GraphicViewPortClass			   *
;*                                                                         *
;*                    File Name : GETPIXEL.ASM                             *
;*                                                                         *
;*                   Programmer : Phil Gorrow				   *
;*                                                                         *
;*                   Start Date : June 7, 1994				   *
;*                                                                         *
;*                  Last Update : June 7, 1994   [PWG]                     *
;*                                                                         *
;*-------------------------------------------------------------------------*
;* Functions:                                                              *
;*   VVPC::Buffer_Get_Pixel -- get the colour of a pixel at given coords   *
;* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - *

IDEAL
P386
MODEL USE32 FLAT

INCLUDE ".\drawbuff.inc"
INCLUDE ".\gbuffer.inc"


CODESEG

;***************************************************************************
;* VVPC::GET_PIXEL -- Gets a pixel from the current view port		   *
;*                                                                         *
;* INPUT:	WORD the x pixel on the screen.				   *
;*		WORD the y pixel on the screen.				   *
;*                                                                         *
;* OUTPUT:      UBYTE the pixel at the specified location		   *
;*                                                                         *
;* WARNING:	If pixel is to be placed outside of the viewport then	   *
;*		this routine will abort.				   *
;*                                                                         *
;* HISTORY:                                                                *
;*   06/07/1994 PWG : Created.                                             *
;*=========================================================================*
	PROC	Buffer_Get_Pixel C near
	USES	ebx,ecx,edx,edi

	ARG    	this_object:DWORD				; this is a member function
	ARG	x_pixel:DWORD				; x position of pixel to set
	ARG	y_pixel:DWORD				; y position of pixel to set

	;*===================================================================
	; Get the viewport information and put bytes per row in ecx
	;*===================================================================
	mov	ebx,[this_object]				; get a pointer to viewport
	xor	eax,eax
	mov	edi,[(GraphicViewPort ebx).GVPOffset]	; get the correct offset
	mov	ecx,[(GraphicViewPort ebx).GVPHeight]	; edx = height of viewport
	mov	edx,[(GraphicViewPort ebx).GVPWidth]	; ecx = width of viewport

	;*===================================================================
	; Verify that the X pixel offset if legal
	;*===================================================================
	mov	eax,[x_pixel]				; find the x position
	cmp	eax,edx					;   is it out of bounds
	jae	short ??exit				; if so then get out
	add	edi,eax					; otherwise add in offset

	;*===================================================================
	; Verify that the Y pixel offset if legal
	;*===================================================================
	mov	eax,[y_pixel]				; get the y position
	cmp	eax,ecx					;  is it out of bounds
	jae	??exit					; if so then get out
	add	edx,[(GraphicViewPort ebx).GVPXAdd]	; otherwise find bytes per row
	add	edx,[(GraphicViewPort ebx).GVPPitch]	; otherwise find bytes per row
	mul	edx					; offset = bytes per row * y
	add	edi,eax					; add it into the offset

	;*===================================================================
	; Write the pixel to the screen
	;*===================================================================
	xor	eax,eax					; clear the word
	mov	al,[edi]				; read in the pixel
??exit:
	ret
	ENDP	Buffer_Get_Pixel

END