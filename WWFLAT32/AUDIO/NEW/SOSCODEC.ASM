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

;****************************************************************************
;*
;*          Copyright (c) 1994, HMI, INC. All Rights Reserved
;*
;*---------------------------------------------------------------------------
;*
;* FILE
;*     soscodec.asm
;*
;* DESCRIPTION
;*     HMI SOS ADPCM compression/decompression.
;*
;* PROGRAMMER
;*     Nick Skrepetos
;*     Denzil E. Long, Jr. (Fixed bugs, rewrote for watcom)
;*	   Bill Petro		   (Added stereo support)
;* DATE
;*     Febuary 15, 1995
;*
;*---------------------------------------------------------------------------
;*
;* PUBLIC
;*
;****************************************************************************

	IDEAL
	P386
	MODEL	USE32 FLAT
	LOCALS	??


DPMI_INTR		equ	31h	
IF_LOCKED_PM_CODE	equ	1h		; Locked PM code for DPMI.
IF_LOCKED_PM_DATA	equ	2h		; Locked PM code for DPMI.

STRUC	sCompInfo
lpSource		DD	?	;Compressed data pointer
lpDest			DD	?	;Uncompressed data pointer
dwCompSize		DD	?	;Compressed size
dwUnCompSize		DD	?	;Uncompressed size

dwSampleIndex		DD	?	;Index into sample
dwPredicted		DD	?	;Next predicted value
dwDifference		DD	?	;Difference from last sample
wCodeBuf		DW	?	;Holds 2 nibbles for decompression
wCode			DW	?	;Current 4 bit code
wStep			DW	?	;Step value in table
wIndex			DW	?	;Index into step table

dwSampleIndex2		DD	?	;Index into sample
dwPredicted2		DD	?	;Next predicted value
dwDifference2		DD	?	;Difference from last sample
wCodeBuf2 		DW	?	;Holds 2 nibbles for decompression
wCode2			DW	?	;Current 4 bit code
wStep2			DW	?	;Step value in table
wIndex2			DW	?	;Index into step table

wBitSize		DW	?	;Bit size for decompression
wChannels		DW	?    	;number of channels
ENDS	sCompInfo


	DATASEG


InitFlags	DD	0		; Flags to indicate what has been initialized.


LABEL	LockedDataStart	BYTE	

;* Index table for stepping into step table

wCODECIndexTab	DD	-1,-1,-1,-1,2,4,6,8
		DD	-1,-1,-1,-1,2,4,6,8


;Lookup table of replacement values
;The actual sound value is replaced with an index to lookup in this table
;The index only takes up a nibble(4bits) and represents an int(16bits)
;Essentially: 
;Get a value
;compare it with the value before it
;find closest value in table and store the index into the table
;if i'm going down then negitize it
;go to next byte.

;Theory for stereo:
;1)handle stereo and mono in two seperate loops. cleaner...
;start at byte 0 and skip every other byte(or word) both write and read
;when we get done
;	set start byte to 1 and do it again 




;This table essentialy round off to closes values in 3 distinct bands
; precalculated and optimized(i guess) for human hearing.

wCODECStepTab	DD	7,  	8,  	9,   	10,   	11,  	12,   	13,14   		
		DD	16, 	17, 	19, 	21,  	23,   	25,  	28,   	31			
		DD	34, 	37, 	41, 	45,  	50,   	55,  	60,   	66			
		DD	73, 	80, 	88, 	97,  	107,  	118, 	130,  	143			
		DD	157,	173,	190,	209, 	230,  	253, 	279,  	307			
		DD	337,	371,	408,	449, 	494,  	544, 	598,  	658			
		DD	724,	796,	876,	963, 	1060,	1166,	1282,	1411			
		DD	1552,	1707,	1878,	2066,	2272,	2499,	2749,	3024			
		DD	3327,	3660,	4026,	4428,	4871,	5358,	5894,	6484
		DD	7132,	7845,	8630,	9493,	10442,	11487,	12635,	13899		
		DD	15289,	16818,	18500,	20350,	22385,	24623,	27086,	29794 
		DD	32767			

wCode				DD	0 ; this is to hold index into wCodeStep
dwCODECByteIndex		DD	0 ; this is when to stop compressing
dwCODECBytesProcessed		DD	0 ; this is how many so far compressed
dwCODECTempStep			DD	0 ; tempory storage for step value
wCODECMask			DW	0 ; Current mask

LABEL	LockedDataEnd	BYTE


	CODESEG

LABEL	LockedCodeStart	BYTE

;****************************************************************************
;*
;* NAME
;*     sosCODECInitStream - Initialize compression stream.
;*
;* SYNOPSIS
;*     sosCODECInitStream(CompInfo)
;*
;*     void sosCODECInitStream(_SOS_COMPRESS_INFO *);
;*
;* FUNCTION
;*     Initialize compression stream for compression and decompression.
;*
;* INPUTS
;*     CompInfo - Compression information structure.
;*
;* RESULT
;*     NONE
;*
;****************************************************************************

	GLOBAL	sosCODECInitStream:NEAR
	PROC	sosCODECInitStream C NEAR

	ARG	sSOSInfo:NEAR PTR

	mov	eax,[sSOSInfo]
	mov	[(sCompInfo eax).wIndex],0 		; starting index 0
	mov	[(sCompInfo eax).wStep],7  		; start with a step of 7
	mov	[(sCompInfo eax).dwPredicted],0 	; no predicted value
	mov	[(sCompInfo eax).dwSampleIndex],0	;start at head of index
	mov	[(sCompInfo eax).wIndex2],0 		; starting index 0
	mov	[(sCompInfo eax).wStep2],7  		; start with a step of 7
	mov	[(sCompInfo eax).dwPredicted2],0 	; no predicted value
	mov	[(sCompInfo eax).dwSampleIndex2],0 	;start at head of index
	ret

	ENDP	sosCODECInitStream



;****************************************************************************
;*
;* NAME
;*     sosCODECDecompressData - Decompress audio data.
;*
;* SYNOPSIS
;*     Size = sosCODECDecompressData(CompInfo, NumBytes)
;*
;*     long sosCODECDecompressData(_SOS_COMPRESS_INFO *, long);
;*
;* FUNCTION
;*     Decompress data from a 4:1 ADPCM compressed stream. The number of
;*     bytes decompressed is returned.
;*
;* INPUTS
;*     CompInfo - Compress information structure.
;*     NumBytes - Number of bytes to compress.
;*
;* RESULT
;*     Size - Size of decompressed data.
;*
;****************************************************************************

	GLOBAL	sosCODECDecompressData:NEAR
	PROC	sosCODECDecompressData C NEAR

	ARG	sSOSInfo:NEAR PTR
	ARG	wBytes:DWORD

	push	esi
	push	edi			   
	push	ebx
	push	ecx
	push	edx

;*---------------------------------------------------------------------------
;*	Initialize
;*---------------------------------------------------------------------------

	mov	ebx,[sSOSInfo]
	mov	eax,[wBytes]
	mov	[dwCODECBytesProcessed],eax
	mov	[(sCompInfo ebx).dwSampleIndex],0 	;start at head of index
	mov	[(sCompInfo ebx).dwSampleIndex2],0 	;start at head of index

	cmp	[(sCompInfo ebx).wBitSize],16
	jne	short ??skipByteDivide

	shr	eax,1	;Divide size by two

??skipByteDivide:
	mov	[dwCODECByteIndex],eax
	mov	esi,[(sCompInfo ebx).lpSource]
	mov	edi,[(sCompInfo ebx).lpDest]

	cmp	[(sCompInfo ebx).wChannels],2		;stereo check
	je	??mainloopl				;do left side first	
	
;	Determine if sample index is even or odd. This will determine
;	if we need to get a new token or not.

;---------------------------------------------------------------------------
;Main Mono Loop
;---------------------------------------------------------------------------
	push	ebp
	movzx	edx,[(sCompInfo ebx).wIndex] 		;preload index
	mov	ebp, [dwCODECByteIndex]
	mov	ecx,[(sCompInfo ebx).dwSampleIndex]	;preload SampleIndex
	
??mainloop:	 
	xor	eax,eax					;get a new token
	test	ecx,1					;odd Sample??
	je	short ??fetchToken			; if so get new token
	mov	ax,[(sCompInfo ebx).wCodeBuf]		;ored with Code
	shr	eax,4
	jmp	short ??calcDifference
	align	4

??fetchToken:
	mov	al,[esi]				;put in codebuf
	inc	esi
	mov	[(sCompInfo ebx).wCodeBuf],ax


??calcDifference:
	xor	ecx,ecx
	and	eax,000Fh
	mov	cx,[(sCompInfo ebx).wStep]		;cx is step value
	mov	[wCode],eax
	jmp	[DWORD PTR JumpTable + eax*4]
	align	4

; note: it is important for speed reasons to keep the order the 
; following jumps entries as well as the "align 4" after some of
; the jmp statements

??7:
	; eax = x + x/2 + x/4 + x/8 = (8*x + 4*x +2*x + x)>>3 =
	;     = ( x * ( 8 + 4 + 2 + 1 )) >> 3 = ( x * 15 ) >> 3 
	lea	ecx,[ecx+ecx*2]
	lea	eax,[ecx+ecx*4]
	sar	eax,3				    ; now we divide x>>3
	jmp	??save_dif
	align	4

??6:
	; eax = x + x / 2 + x / 8 = (8*x + 4*x + x) >> 3 =
	;     =	( x * 8 + x * 5	) >> 8
	lea	eax,[ecx+ecx*4]
	lea	eax,[eax+ecx*8]
	sar	eax,3				    ; now we divide x>>3
	jmp	??save_dif
	align	4

??5:
	; eax = x + x / 4 + x / 8 = (8*x + 2*x + x) >> 3 = 
	;     =	( 8 * x + 3 * x) >> 3
	lea	eax,[ecx+ecx*2]
	lea	eax,[eax+ecx*8]
	sar	eax,3				    ; now we divide x>>3
	jmp	??save_dif
	align	4

??4:
	; eax = x + x / 8 = (8*x + x) >> 3 = (x * 9)>> 3
	lea	eax,[ecx+ecx*8]
	sar	eax,3				    ; now we divide x>>3
	jmp	??save_dif
	align	4

??3:
	; eax = x/2 + x/4 + x/8 = (4*x + 2*x + x) >> 3
	;     =	( 4x + 3x ) >> 3
	lea	eax,[ecx+ecx*2]
	lea	eax,[eax+ecx*4]
	sar	eax,3				    ; now we divide x>>3
	jmp	??save_dif
	align	4

??2:
	; eax = x/2 + x/8 = (4*x + x) >> 3
	lea	eax,[ecx+ecx*4]
	sar	eax,3				    ; now we divide x>>3
	jmp	??save_dif
	align	4

??1:
	; eax = x/4 + x/8 = (2x + x )>>8
	lea	eax,[ecx+ecx*2]
	sar	eax,3				    ; now we divide x>>3
	jmp	??save_dif
	align	4

??0:
	; eax = x/8 = x >> 3
	mov	eax,ecx
	sar	eax,3				    ; now we divide x>>3
	jmp	 ??save_dif
	align	4

??15:
	; eax = x + x/2 + x/4 + x/8 = (8*x + 4*x +2*x + x)>>3 =
	;     = ( x * ( 8 + 4 + 2 + 1 )) >> 3 = ( x * 15 ) >> 3 
	lea	ecx,[ecx+ecx*2]
	lea	eax,[ecx+ecx*4]
	jmp	??neg_save_dif
	align	4

??14:
	; eax = x + x / 2 + x / 8 = (8*x + 4*x + x) >> 3 =
	;     =	( x * 8 + x * 5	) >> 8
	lea	eax,[ecx+ecx*4]
	lea	eax,[eax+ecx*8]
	jmp	??neg_save_dif
	align	4

??13:
	; eax = x + x / 4 + x / 8 = (8*x + 2*x + x) >> 3 = 
	;     =	( 8 * x + 3 * x) >> 3
	lea	eax,[ecx+ecx*2]
	lea	eax,[eax+ecx*8]
	jmp	??neg_save_dif
	align	4

??12:
	; eax = x + x / 8 = (8*x + x) >> 3 = (x * 9)>> 3
	lea	eax,[ecx+ecx*8]
	jmp	??neg_save_dif
	align	4

??11:
	; eax = x/2 + x/4 + x/8 = (4*x + 2*x + x) >> 3
	;     =	( 4*x - 3*x ) >> 3
	lea	eax,[ecx+ecx*2]
	lea	eax,[eax+ecx*4]
	jmp	??neg_save_dif
	align	4

??10:
	; eax = x/2 + x/8 = (4*x + x) >> 3
	lea	eax,[ecx+ecx*4]
	jmp	??neg_save_dif
	align	4

??9:
	; eax = x/4 + x/8 = (2x + x )>>8
	lea	eax,[ecx+ecx*2]
	jmp	??neg_save_dif
	align	4

??8:
	; eax = x/8 = x >> 3
	mov	eax,ecx		       ; !!important!! no need for align here	

??neg_save_dif:
	sar	eax,3				    ; now we divide x>>3
	neg 	eax

??save_dif:
 	mov	ecx,[wCode]			    ; load offset into CODETab table	
	mov	[(sCompInfo ebx).dwDifference],eax  ;Store wStep

	; add difference to predicted value.
	add	eax,[(sCompInfo ebx).dwPredicted]

	; make sure there is no under or  overflow.

	cmp	eax,7FFFh
	jl	short ??noOverflow
	mov	eax,7FFFh

??noOverflow:
	cmp	eax,0FFFF8000h
	jg	short ??noUnderflow
	mov	eax,0FFFF8000h

??noUnderflow:
	add	edx,[wCODECIndexTab + ecx*4]	  ; won't hurt 486 	
	cmp	[(sCompInfo ebx).wBitSize],16
	mov	[(sCompInfo ebx).dwPredicted],eax
	mov	ecx,[(sCompInfo ebx).dwSampleIndex] ;load dwSampleindex
	je	short ??output16Bit
	; output 8 bit sample
	xor	ah,80h
	mov	[edi],ah
	inc	edi
	jmp	short ??adjustIndex
	align	4

??output16Bit:
	mov	[edi],ax			  ;Output 16bit sample
	add	edi,2

??adjustIndex:
	cmp	edx,8000h
	jb	short ??checkOverflow
	mov	edx,0				;reset index to zero

??checkOverflow:
	inc	ecx				; advance index and store step value
	cmp	edx,88				;check if wIndex > 88
	jbe	short ??adjustStep
	mov	edx,88				;reset index to 88

??adjustStep:
	; advance index and store step value
	mov	[(sCompInfo ebx).dwSampleIndex],ecx

	; fetch wIndex so we can fetch new step value
	mov	eax,[wCODECStepTab + edx*4]

	; decrement bytes processed and loop back.
	dec	ebp
	mov	[(sCompInfo ebx).wStep],ax
	jne	??mainloop
	pop	ebp

	mov	eax,[wCode]			; these three lines do not
	mov	[(sCompInfo ebx).wCode],ax	; seem to have any relevance

	mov	[(sCompInfo ebx).wIndex],dx  	; save index
	jmp	??exitout

;--------------------------------------------------------------------------
;Left Channel Start
;--------------------------------------------------------------------------


??mainloopl:
	test	[(sCompInfo ebx).dwSampleIndex],1
	je	short ??fetchTokenl
	
	xor	eax,eax
	mov	ax,[(sCompInfo ebx).wCodeBuf]
	shr	eax,4
	and	eax,000Fh
	mov	[(sCompInfo ebx).wCode],ax
	jmp	short ??calcDifferencel

??fetchTokenl:
	xor	eax,eax
	mov	al,[esi]
	mov	[(sCompInfo ebx).wCodeBuf],ax
	add	esi,2					;2 for stereo			
	and	eax,000Fh
	mov	[(sCompInfo ebx).wCode],ax

??calcDifferencel:
							; reset difference

	mov	[(sCompInfo ebx).dwDifference],0
	xor	ecx,ecx
	mov	cx,[(sCompInfo ebx).wStep]
	test	eax,4					;Check for wCode & 4
	je	short ??no4l
	add	[(sCompInfo ebx).dwDifference],ecx	;Add wStep

??no4l:
	test	eax,2					;Check for wCode & 2
	je	short ??no2l
	mov	edx,ecx					;Add wStep >> 1
	shr	edx,1
	add	[(sCompInfo ebx).dwDifference],edx

??no2l:
	test	eax,1					;Check for wCode & 1
	je	short ??no1l
	mov	edx,ecx					;Add wStep >> 2
	shr	edx,2
	add	[(sCompInfo ebx).dwDifference],edx

??no1l:
	mov	edx,ecx					;Add in wStep >> 3
	shr	edx,3
	add	[(sCompInfo ebx).dwDifference],edx
	test	eax,8					;Check for wCode & 8
	je	short ??no8l
	neg	[(sCompInfo ebx).dwDifference]		;Negate diff

??no8l:
		; add difference to predicted value.

	mov	eax,[(sCompInfo ebx).dwPredicted]
	add	eax,[(sCompInfo ebx).dwDifference]

		; make sure there is no under or  overflow.

	cmp	eax,7FFFh
	jl	short ??noOverflowl
	mov	eax,7FFFh

??noOverflowl:
	cmp	eax,0FFFF8000h
	jg	short ??noUnderflowl
	mov	eax,0FFFF8000h

??noUnderflowl:
	mov	[(sCompInfo ebx).dwPredicted],eax
	cmp	[(sCompInfo ebx).wBitSize],16
	jne	short ??output8Bitl
	mov	[edi],ax				;Output 16bit sample
	add	edi,4					;4 for stereo		
	jmp	short ??adjustIndexl

??output8Bitl:
	; output 8 bit sample

	xor	ah,80h
	mov	[edi],ah
	add	edi,2					;2 for stereo       

??adjustIndexl:
	xor	ecx,ecx
	mov	cx,[(sCompInfo ebx).wCode]
	xor	eax,eax
	shl	ecx,2
	mov	eax,[wCODECIndexTab + ecx]
	add	[(sCompInfo ebx).wIndex],ax
							; check if wIndex < 0
	cmp	[(sCompInfo ebx).wIndex],8000h
	jb	short ??checkOverflowl
	mov	[(sCompInfo ebx).wIndex],0
	jmp	short ??adjustStepl			;reset index to zero


??checkOverflowl:
							
	cmp	[(sCompInfo ebx).wIndex],88		; check if wIndex > 88
	jbe	short ??adjustStepl
	mov	[(sCompInfo ebx).wIndex],88		; reset index to 88

??adjustStepl:
	; fetch wIndex so we can fetch new step value

	xor	ecx,ecx
	mov	cx,[(sCompInfo ebx).wIndex]
	xor	eax,eax
	shl	ecx,2
	mov	eax,[wCODECStepTab + ecx]

	; advance index and store step value

	add	[(sCompInfo ebx).dwSampleIndex],1
	mov	[(sCompInfo ebx).wStep],ax

	; decrement bytes processed and loop back.

	sub	[dwCODECByteIndex],2
	jne	??mainloopl
;----------------------------------------------------------------------------
; Right Side Setup
;----------------------------------------------------------------------------
	mov	eax,[wBytes]
	mov	[dwCODECBytesProcessed],eax
	mov	esi,[(sCompInfo ebx).lpSource]
	mov	edi,[(sCompInfo ebx).lpDest]
	inc	esi					; skip left channel
	inc	edi				     	; skip left channel
	cmp	[(sCompInfo ebx).wBitSize],16		;16 bit ??
	je	short ??doByteDivide			
	mov	[dwCODECByteIndex],eax
	jmp  short ??mainloopr
	
??doByteDivide:
	shr	eax,1 					;Divide size by two
	inc	edi   					; 16 bit so skip 1 more
	mov	[dwCODECByteIndex],eax
	

;--------------------------------------------------------------------------
;Right Channel Start
;--------------------------------------------------------------------------


??mainloopr:	 
	test	[(sCompInfo ebx).dwSampleIndex2],1
	je	short ??fetchTokenr
	xor	eax,eax
	mov	ax,[(sCompInfo ebx).wCodeBuf2]
	shr	eax,4
	and	eax,000Fh
	mov	[(sCompInfo ebx).wCode2],ax
	jmp	short ??calcDifferencer

??fetchTokenr:
	xor	eax,eax
	mov	al,[esi]
	mov	[(sCompInfo ebx).wCodeBuf2],ax
	add	esi,2				     	;2 for stereo			
	and	eax,000Fh
	mov	[(sCompInfo ebx).wCode2],ax

??calcDifferencer:
							; reset difference

	mov	[(sCompInfo ebx).dwDifference2],0
	xor	ecx,ecx
	mov	cx,[(sCompInfo ebx).wStep2]
	test	eax,4					;Check for wCode & 4
	je	short ??no4r
	add	[(sCompInfo ebx).dwDifference2],ecx	;Add wStep

??no4r:
	test	eax,2					;Check for wCode & 2
	je	short ??no2r
	mov	edx,ecx					;Add wStep >> 1
	shr	edx,1
	add	[(sCompInfo ebx).dwDifference2],edx

??no2r:
	test	eax,1					;Check for wCode & 1
	je	short ??no1r
	mov	edx,ecx					;Add wStep >> 2
	shr	edx,2
	add	[(sCompInfo ebx).dwDifference2],edx

??no1r:
	mov	edx,ecx					;Add in wStep >> 3
	shr	edx,3
	add	[(sCompInfo ebx).dwDifference2],edx
	test	eax,8					;Check for wCode & 8
	je	short ??no8r
	neg	[(sCompInfo ebx).dwDifference2]		;Negate diff

??no8r:
	; add difference to predicted value.
	mov	eax,[(sCompInfo ebx).dwPredicted2]
	add	eax,[(sCompInfo ebx).dwDifference2]
	cmp	eax,7FFFh
	jl	short ??noOverflowr
	mov	eax,7FFFh

??noOverflowr:
	cmp	eax,0FFFF8000h
	jg	short ??noUnderflowr
	mov	eax,0FFFF8000h

??noUnderflowr:
	mov	[(sCompInfo ebx).dwPredicted2],eax
	cmp	[(sCompInfo ebx).wBitSize],16
	jne	short ??output8Bitr
	mov	[edi],ax				;Output 16bit sample
	add	edi,4					;4 for stereo		***
	jmp	short ??adjustIndexr

??output8Bitr:
							; output 8 bit sample
	xor	ah,80h
	mov	[edi],ah
	add	edi,2					;2 for stereo       

??adjustIndexr:
	xor	ecx,ecx
	mov	cx,[(sCompInfo ebx).wCode2]
	xor	eax,eax
	shl	ecx,2
	mov	eax,[wCODECIndexTab + ecx]
	add	[(sCompInfo ebx).wIndex2],ax
							; check if wIndex < 0
	cmp	[(sCompInfo ebx).wIndex2],8000h
	jb	short ??checkOverflowr
							; reset index to zero
	mov	[(sCompInfo ebx).wIndex2],0
	jmp	short ??adjustStepr

??checkOverflowr:
							; check if wIndex > 88
	cmp	[(sCompInfo ebx).wIndex2],88	        
	jbe	short ??adjustStepr
	mov	[(sCompInfo ebx).wIndex2],88		; reset index to 88

??adjustStepr:
	; fetch wIndex so we can fetch new step value

	xor	ecx,ecx
	mov	cx,[(sCompInfo ebx).wIndex2]
	xor	eax,eax
	shl	ecx,2
	mov	eax,[wCODECStepTab + ecx]

	; advance index and store step value

	add	[(sCompInfo ebx).dwSampleIndex2],1
	mov	[(sCompInfo ebx).wStep2],ax

	; decrement bytes processed and loop back.

	sub	[dwCODECByteIndex],2
	jne	??mainloopr
	

??exitout:
;     	don't think we need this but just in case i'll leave it here!!

;	mov	[(sCompInfo ebx).lpSource],esi
;	mov	[(sCompInfo ebx).lpDest],edi
	; set up return value for number of bytes processed.
	mov	eax,[dwCODECBytesProcessed]
	pop	edx
	pop	ecx
	pop	ebx
	pop	edi
	pop	esi
	ret

JumpTable	DD	??0
		DD	??1
		DD	??2
		DD	??3
		DD	??4
		DD	??5
		DD	??6
		DD	??7
		DD	??8
		DD	??9
		DD	??10
		DD	??11
		DD	??12
		DD	??13
		DD	??14
		DD	??15
			 

	ENDP	sosCODECDecompressData

LABEL	LockedCodeEnd	BYTE

;***************************************************************************
;* sosCODEC_LOCK -- locks the JLB audio decompression code		   *
;*                                                                         *
;* INPUT:	none                                                       *
;*                                                                         *
;* OUTPUT:      BOOL true is lock sucessful, false otherwise		   *
;*                                                                         *
;* PROTO:       BOOL sosCODEC_Lock(void);                         	   *
;*                                                                         *
;* HISTORY:								   *
;*   06/26/1995 PWG : Created.                                             *
;*=========================================================================*
	GLOBAL	sosCODEC_Lock:NEAR
	PROC	sosCODEC_Lock C NEAR USES ebx ecx edx esi edi

	;
	; Lock the code that is used by the sos decompression method.
	;
	mov	eax,0600h			; function number.
	mov	ecx,OFFSET LockedCodeStart	; ecx must have start of memory.
	mov	edi,OFFSET LockedCodeEnd	; edi will have size of region in bytes.
	shld	ebx,ecx,16
	sub	edi, ecx
	shld	esi,edi,16
  	int	DPMI_INTR			; do call.
	jc	??error
	or	[InitFlags], IF_LOCKED_PM_CODE

	;
	; Lock the data used by the sos decompression method.
	;
	mov	eax,0600h			; function number.
	mov	ecx,OFFSET LockedDataStart	; ecx must have start of memory.
	mov	edi,OFFSET LockedDataEnd	; edi will have size of region in bytes.
	shld	ebx,ecx,16
	sub	edi, ecx
	shld	esi,edi,16
   	int	DPMI_INTR			; do call.
	jc	??error				; eax = 8 if mem err, eax = 9 if invalid mem region.
	or	[InitFlags], IF_LOCKED_PM_DATA

	mov	eax,1
	jmp	??exit
??error:
	xor	eax,eax
??exit:
	ret
	ENDP	sosCODEC_Lock

;***************************************************************************
;* DECOMPRESS_FRAME_UNLOCK -- Unlocks the JLB audio compression code       *
;*                                                                         *
;* INPUT:	none                                                       *
;*                                                                         *
;* OUTPUT:      BOOL true is unlock sucessful, false otherwise		   *
;*                                                                         *
;* PROTO:	BOOL sosCODEC_Unlock(void);                        	   *
;*                                                                         *
;* HISTORY:								   *
;*   06/26/1995 PWG : Created.                                             *
;*=========================================================================*
	GLOBAL	sosCODEC_Unlock:NEAR
	PROC	sosCODEC_Unlock C NEAR USES ebx ecx edx esi edi

	test	[InitFlags],IF_LOCKED_PM_CODE
	jz	??code_not_locked

	mov	eax , 0601h
	mov	ecx,OFFSET LockedCodeStart	; ecx must have start of memory.
	mov	edi,OFFSET LockedCodeEnd	; edx will have size of region in bytes.
	sub	edi,ecx				;  - figure size.
	shld	ebx , ecx , 16
	shld	esi , edi , 16
	int	DPMI_INTR			; do call.
	jc	??error

??code_not_locked:
	test	[InitFlags],IF_LOCKED_PM_DATA
	jz	??data_not_locked

	mov	ax,0601h				; set es to descriptor of data.
	mov	ecx,OFFSET LockedDataStart	; ecx must have start of memory.
	mov	edi,OFFSET LockedDataEnd	; edx will have size of region in bytes.
	sub	edi,ecx				;  - figure size.
	shld	ebx , ecx , 16
	shld	esi , edi , 16
	int	DPMI_INTR			; do call.
	jc	??error				; eax = 8 if mem err, eax = 9 if invalid mem region.

??data_not_locked:
	mov	[InitFlags],0
	mov	eax,1
	jmp	??exit
??error:
	xor	eax,eax
??exit:
	ret
	ENDP	sosCODEC_Unlock

	END

