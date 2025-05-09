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
;**      C O N F I D E N T I A L --- W E S T W O O D   S T U D I O S      **
;***************************************************************************
;*                                                                         *
;*                 Project Name : Westwood 32 bit Library                  *
;*                                                                         *
;*                    File Name : TXTPRNT.ASM                              *
;*                                                                         *
;*                   Programmer : Phil W. Gorrow                           *
;*                                                                         *
;*                   Start Date : January 17, 1995                         *
;*                                                                         *
;*                  Last Update : January 17, 1995   [PWG]                 *
;*                                                                         *
;*-------------------------------------------------------------------------*
;* Functions:                                                              *
;*   VESA_Print -- Assembly VESA text print routine                        *
;* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - *

IDEAL
P386
MODEL USE32 FLAT

INCLUDE "svgaprim.inc"
INCLUDE ".\gbuffer.inc"

;*=========================================================================*
;* Extern the font pointer which is defined by the font class		   *
;*=========================================================================*
	GLOBAL	FontPtr:DWORD
	GLOBAL	ColorXlat:BYTE

;*=========================================================================*
;* Define the necessary equates for structures and bounds checking	   *
;*=========================================================================*
; The header of the font file looks like this:
; 	UWORD	FontLength;		0
; 	BYTE	FontCompress;		2
;	BYTE	FontDataBlocks;		3
;	UWORD	InfoBlockOffset;	4
;	UWORD	OffsetBlockOffset;	6
;	UWORD	WidthBlockOffset;	8
;	UWORD	DataBlockOffset;	10
;	UWORD	HeightOffset;		12
; For this reason the following equates have these values:
FONTINFOBLOCK		EQU	4
FONTOFFSETBLOCK		EQU	6
FONTWIDTHBLOCK		EQU	8
FONTDATABLOCK		EQU	10
FONTHEIGHTBLOCK		EQU	12

FONTINFOMAXHEIGHT	EQU	4
FONTINFOMAXWIDTH	EQU	5


LOCALS ??
;*=========================================================================*
;* Define the color xlate table in the data segment			   *
;*=========================================================================*

	CODESEG


;***************************************************************************
;* VESA_PRINT -- Assembly VESA text print routine                          *
;*                                                                         *
;*                                                                         *
;*                                                                         *
;* INPUT:                                                                  *
;*                                                                         *
;* OUTPUT:                                                                 *
;*                                                                         *
;* PROTO:                                                                  *
;*                                                                         *
;* WARNINGS:                                                               *
;*                                                                         *
;* HISTORY:                                                                *
;*   01/17/1995 PWG : Created.                                             *
;*=========================================================================*
	PROC	Vesa_Print C near
	USES	ebx,ecx,edx,esi,edi

	ARG	this:DWORD
	ARG	string:DWORD	 
	ARG	x_pixel:DWORD
	ARG	y_pixel:DWORD
	ARG	fcolor:DWORD
	ARG	bcolor:DWORD
		
	LOCAL	infoblock:DWORD		; pointer to info block
	LOCAL	offsetblock:DWORD	; pointer to offset block  (UWORD *)
	LOCAL	widthblock:DWORD	; pointer to width block   (BYTE  *)
	LOCAL	heightblock:DWORD	; pointer to height block  (UWORD *)

	LOCAL	curline:DWORD		; pointer to first column of current row.
	local	ptr_string:dword	; pointer to string
	LOCAL	bufferwidth:DWORD    	; width of buffer (vpwidth + Xadd)
	LOCAL	nextdraw:DWORD		; bufferwidth - width of cur character.
	LOCAL	startdraw:DWORD		; where next character will start being drawn.

	LOCAL	char:DWORD		; current character value.	

	LOCAL	maxheight:BYTE		; max height of characters in font.
	LOCAL	bottomblank:BYTE	; amount of empty space below current character.
	LOCAL	charheight:BYTE		; true height of current character.
	LOCAL	vpwidth:DWORD
	LOCAL	vpheight:DWORD
	LOCAL	remainder:DWORD
	LOCAL	fullwidth:DWORD
	LOCAL	currwidth:DWORD


;-------------------------------- Where to draw -----------------------------------------------
	; Set up memory location to start drawing.
	mov  	ebx,[this]				; get a pointer to dest
	mov	eax,[(GraphicViewPort ebx).GVPHeight]	; get height of viewport
	mov	[vpheight],eax				; save off height of viewport
	mov	eax,[(GraphicViewPort ebx).GVPWidth]	; get width of viewport
	mov	[vpwidth],eax				; save it off for later
	add	eax,[(GraphicViewPort ebx).GVPXAdd]	; add in xadd for bytes_per_line
	mov	[bufferwidth],eax     			; save it off for later use.

	mul	[y_pixel]				; multiply rowsize * y_pixel start.
	mov	edi,[(GraphicViewPort ebx).GVPOffset]	; get start of the viewport
	add	edi,eax					; add y position to start of vp 
	mov	[curline],edi				; save 0,y address for line feed stuff.
	add	edi,[x_pixel]				; add to get starting column in starting row.

	mov	[startdraw],edi				; save it off.


;-------------------------------- Create block pointers ----------------------------------------
	; Get the pointer to the font.
	; We could check for NULL but why waste the time.
	; It is up to programmer to make sure it is set.
	mov	esi,[FontPtr]		; Get the font pointer
	or	esi,esi
	jz	??done

	; Set up some pointers to the different memory blocks.
	; esi (FontPtr) is added to each to get the true address of each block.
	; Many registers are used for P5 optimizations.
	; ebx is used for InfoBlock which is then used in the next section.
	movzx	eax,[WORD PTR esi+FONTOFFSETBLOCK]	; get offset to offset block
	movzx	ebx,[WORD PTR esi+FONTINFOBLOCK]      	; get offset to info block (must be ebx for height test)
	movzx	ecx,[WORD PTR esi+FONTWIDTHBLOCK] 	; get offset to width block
	movzx	edx,[WORD PTR esi+FONTHEIGHTBLOCK]	; get offset to height block

	add	eax,esi				; add offset of FontPtr to offset block  
	add	ebx,esi				; add offset of FontPtr to info block 
	add	ecx,esi				; add offset of FontPtr to width block
	add	edx,esi				; add offset of FontPtr to height block

	mov	[offsetblock],eax		; save offset to offset block
	mov	[infoblock],ebx			; save offset to info block
	mov	[widthblock],ecx   		; save offset to width block
	mov	[heightblock],edx		; save offset to height block

;------------------------------------------ Test for fit ----------------------------------------------
	; Test to make sure the height of the max character will fit on this line
	; and and not fall out of the viewport.
	; remember we set ebx to FONTINFOBLOCK above.
	movzx	eax,[BYTE PTR ebx + FONTINFOMAXHEIGHT]; get the max height in font.
	mov	[maxheight],al			; save it for later use.
	add	eax,[y_pixel]			; add current y_value.
	cmp	eax,[vpheight]			; are we over the edge?
	jg	??done			; if so, we're outa here.

	mov	[y_pixel],eax			; save for next line feed. y value for next line.

	cld					; Make sure we are always forward copying.

;------------------------ Set palette foreground and background ----------------------------------
	mov	eax,[fcolor]		; foreground color
	mov	[ColorXlat+1],al
	mov	[ColorXlat+16],al

	mov	eax,[bcolor]		; background color
	mov	[ColorXlat],al

;-------------------------------------------------------------------------------------------------
;----------------------------------------- Main loop ----------------------------------------------
	; Now we go into the main loop of reading each character in the string and doing
	; something with it.
??next_char:
	; while (*string++)
	xor	eax,eax				; zero out since we will just load al.	
	mov	esi,[string]			; get address on next character.	
	lodsb					; load the character into al.
	test	eax,0FFH			; test to see if character is a NULL
	jz	??done				; character is NULL, get outa here.
	mov	edi,[startdraw]			; Load the starting address.

	mov	[string],esi			; save index into string. (incremented by lodsb)	

	cmp	eax,10				; is the character a line feed?
	je	??line_feed			; if so, go to special case.
	
	cmp	eax,13				; is the character a line feed?
	je	??line_feed			; if so, go to special case.

	mov	[char],eax			; save the character off for later reference.
	mov	ebx,eax				; save it in ebx for later use also.

	add	eax,[widthblock]		; figure address of width of character.
	mov	ecx,[x_pixel]			; get current x_pixel.
	movzx	edx,[BYTE PTR eax]	 	; get the width of the character in dl.
	add	ecx,edx				; add width of char to current x_pixel.
	add	[startdraw],edx			; save start draw for next character.

	cmp	ecx,[vpwidth]			; is the pixel greater then the vp width?
	jg	??force_line_feed		; if so, force a line feed.

	mov	[x_pixel],ecx			; save value of start of next character.
	mov	ecx,[bufferwidth]		; get amount to next y same x (one row down)
	sub	ecx,edx				; take the current width off.
	mov	[nextdraw],ecx			; save it to add to edi when done with a row.

	; At this point we got the character. It is now time to find out specifics
	; about drawing the darn thing.
	; ebx = char so they can be used as an indexes.
	; edx = width of character for loop later.

	; get offset of data for character into esi.
	shl	ebx,1				; mult by 2 to later use as a WORD index.
	mov	esi,[offsetblock]		; get pointer to begining of offset block.
	add	esi,ebx				; index into offset block.	
	movzx	esi,[WORD PTR esi]		; get true offset into data block from FontPtr.
	add	esi,[FontPtr]			; Now add FontPtr address to get true address.

	; Get top and bottom blank sizes and the true height of the character.
	add	ebx,[heightblock]		; point ebx to element in height array.
	mov	al,[ebx+1]			; load the data height into dl.
	mov	cl,[ebx]			; load the first data row into cl.
	mov	bl,[maxheight]			; get the max height of characters.
	mov	[charheight],al			; get number of rows with data.
	add	al,cl				; add the two heights.
	sub	bl,al				; subract topblank + char height from maxheight.
	mov	[bottomblank],bl		; save off the number of blank rows on the bottom.
	; leaving this section:
	; dl is still the width of the character.
	; cl is the height of the top blank area.

	mov	ebx,OFFSET ColorXlat		; setup ebx for xlat commands.
	mov	dh,dl				; save the width of the character to restore each loop.
	call	Vesa_Asm_Set_Win		; adjust edi & vesa page

	cmp	cl,0				; is there any blank rows on top?
	jz	??draw_char			; if not go and draw the real character.

	xor	eax,eax				; get color 0 for background.
	xlat	[ebx]				; get translated color into al
	test	al,al				; is it transparent black
	jnz	??loop_top			; if not go and write the color

;----------------------------------------- skip Top blank area ----------------------------------------
	; this case, the top is transparrent, but we need to increase our dest pointer to correct row.
	movzx	eax,cl				; get number of rows into eax;
	mov	ecx,edx				; save width since edx will be destroyed by mul.
	mul	[bufferwidth]			; multiply that by the width of the buffer.
	mov	edx,ecx				; restore the width
	add	edi,eax				; update the pointer.	
	cmp	edi,0b0000h			; have we gone over win edge
	jl	??draw_char			; if not keep writing to window
        add	edi , [ cpu_video_page ]
	call	Vesa_Asm_Set_Win		; instead switch to next window
	jmp	short ??draw_char		; now go draw the character.

;----------------------------------------- fill Top blank area ----------------------------------------

??loop_top:
	stosb					; store the value
	cmp	edi,0b0000h			; have we gone over win edge
	jl	??top_no_window_change		; if not keep writing to window
        add	edi , [ cpu_video_page ]
	call	Vesa_Asm_Set_Win		; instead switch to next window
??top_no_window_change:

	dec	dh				; decrement our width.
	jnz	??loop_top			; if more width, continue on.

	add	edi,[nextdraw]			; add amount for entire row.
	cmp	edi,0b0000h			; have we gone over win edge
	jl	??top2_no_window_change	; if not keep writing to window
        add	edi , [ cpu_video_page ]
	call	Vesa_Asm_Set_Win		; instead switch to next window

??top2_no_window_change:

	dec	cl				; decrement or row count
	mov	dh,dl				; restore width in dh for loop.
	jz	??draw_char			; we are done here, go draw the character.
	jmp	short ??loop_top		; go back to top of loop.
;----------------------------------------- Draw character ----------------------------------------------
??draw_char:
	movzx	ecx,[charheight]		; get the height of character to count down rows.
	test	ecx,ecx				; is there any data? (blank would not have any)
	jz	??next_char			; if no data, go on to next character.	


??while_data:
	lodsb					; get byte value from font data
	mov	ah,al				; save hinibble
	and	eax,0F00FH	       		; mask of low nibble in al hi nibble in ah.
	xlat	[ebx]				; get new color

	test	al,al				; is it a transparent?
	jz	short ??skiplo			; skip over write
	mov	[es:edi],al			; write it out
??skiplo:
	inc	edi
	cmp	edi,0b0000h			; have we gone over win edge
	jl	??lo_no_window_change		; if not keep writing to window
        add	edi , [ cpu_video_page ]
	call	Vesa_Asm_Set_Win		; instead switch to next window
??lo_no_window_change:
	dec	dh				; decrement our width.
	jz	short ??nextrow			; check if done with width of char

	mov	al,ah				; restore to get
	; test the time difference between looking up in a large table when shr al,4 is not done as
	; compared to using only a 16 byte table when using the shr al,4
	;shr	al,4				; shift the hi nibble down to low nibble
	xlat	[ebx]				; get new color

	test	al,al				; is it a transparent?
	jz	short ??skiphi			; skip over write
	mov	[edi],al			; write it out
??skiphi:
	inc	edi
	cmp	edi,0b0000h			; have we gone over win edge
	jl	??hi_no_window_change		; if not keep writing to window
        add	edi , [ cpu_video_page ]
	call	Vesa_Asm_Set_Win		; instead switch to next window
??hi_no_window_change:

	dec	dh				; decrement our width.
	jnz	short ??while_data		; check if done with width of char

??nextrow:
	add	edi,[nextdraw]			; go to next line.
	cmp	edi,0b0000h			; have we gone over win edge
	jl	??next_row_no_window_change	; if not keep writing to window
        add	edi , [ cpu_video_page ]
	call	Vesa_Asm_Set_Win		; instead switch to next window
??next_row_no_window_change:
	dec	ecx				; decrement the number of rows to go
	mov	dh,dl				; restore our column count for row.
	jnz	??while_data			; more data for character.

	; Now it is time to setup for clearing out the bottom of the character.
	movzx	ecx,[bottomblank]		; get amount on bottom that is blank
	cmp	ecx,0				; if there is no blank bottom...
	jz	??next_char			; then skip to go to next character

	xor	eax,eax				; get color 0 for background.
	xlat	[ebx]				; get translated color into al
	test	al,al				; is it transparent black
	jz	??next_char			; skip the top black section to let the background through

	mov	dh,dl				; restore width in dh for loop.

;----------------------------------------- Blank below character -----------------------------------
??loop_bottom:
	stosb					; store the value
	cmp	edi,0b0000h			; have we gone over win edge
	jl	??bottom_no_window_change	; if not keep writing to window
        add	edi , [ cpu_video_page ]
	call	Vesa_Asm_Set_Win		; instead switch to next window
??bottom_no_window_change:
	dec	dh				; decrement our width.
	jnz	??loop_bottom			; if more width, continue on.

	add	edi,[nextdraw]			; add amount for entire row.
	cmp	edi,0b0000h			; have we gone over win edge
	jl	??bottom2_no_window_change	; if not keep writing to window
        add	edi , [ cpu_video_page ]
	call	Vesa_Asm_Set_Win		; instead switch to next window
??bottom2_no_window_change:

	mov	dh,dl				; restore width in dh for loop.
	dec	cl				; decrement or row count
	jz	??next_char			; we are done here, go to the next character.
	jmp	short ??loop_bottom		; go back to top of loop.

;----------------------------------- end of next_char (main) loop ------------------------------------
;-------------------------------------------------------------------------------------------------

;----------------------------------- special case line feeds ----------------------------------------
; JRJ 05/01/95 This is the problem However made this change introduced
; a error in the code, this function is not supposed to handle
; Text wrapping
??force_line_feed:	
	; decrement pointer *string so that it will be back at same character
	; when it goes through the loop.
	dec	[dword ptr string]		  ; overflow by one charater
	jmp	??done


??line_feed:
	mov	edx,[y_pixel]			; get the current y pixel value.
	movzx	ecx,[maxheight]			; get max height for later use.
	add	edx,ecx				; add max height to y_pixel
	cmp	edx,[vpheight]			; are we over the edge?
	jg	??done			; if so, we are outa here.

	mov	eax,[bufferwidth]      		; get bytes to next line.
	mov	edi,[curline]			; get start of current line.	
	mul	ecx				; mult max height * next line.	

	add	edi,eax				; add adjustment to current line.
	add	[y_pixel],ecx			; increment to our next y position.
	mov	[curline],edi			; save it off for next line_feed.
	mov	[startdraw],edi			; save it off so we know where to draw next char.w

	mov	[x_pixel],0			; zero out x_pixel

	jmp	??next_char
??done:
	mov	eax,[string]			; return the number of charaters 
	sub	eax,[ptr_string]		; printed
	ret					
		
	ENDP	Vesa_Print

END
