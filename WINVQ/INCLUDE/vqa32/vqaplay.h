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

#ifndef VQAPLAY_H
#define VQAPLAY_H
/****************************************************************************
*
*         C O N F I D E N T I A L -- W E S T W O O D  S T U D I O S
*
*----------------------------------------------------------------------------
*
* PROJECT
*     VQA player library. (32-Bit protected mode)
*
* FILE
*     vqaplay.h
*
* DESCRIPTION
*      VQAPlay library definitions.
*
* PROGRAMMER
*     Bill Randolph
*     Denzil E. Long, Jr.
*
* DATE
*     April 10, 1995
*
****************************************************************************/
#include <stdint.h>

/*---------------------------------------------------------------------------
 * CONDITIONAL COMPILATION FLAGS
 *-------------------------------------------------------------------------*/

// MEG - 11.28.95 - added for debug
extern void Debug_Printf( char *format_string, ... );

#ifdef __WATCOMC__
#define VQASTANDALONE 0  /* Stand alone player */
#define VQAVOC_ON     0  /* Enable VOC file override */
#define	VQAMONO_ON    0  /* Mono display output enable/disable */
#define VQADIRECT_SOUND 1	/* Use windows direct sound system */
#define VQAAUDIO_ON   1  /* Audio playback enable/disable */
#define VQAVIDEO_ON   0  /* Video manager enable/disable */
#define VQAMCGA_ON    1  /* MCGA enable/disable */
#define VQAXMODE_ON   0  /* Xmode enable/disable */
#define VQAVESA_ON    0  /* VESA enable/disable */
#define	VQABLOCK_2X2  0  /* 2x2 block decode enable/disable */
#define	VQABLOCK_2X3  0  /* 2x2 block decode enable/disable */
#define	VQABLOCK_4X2  1  /* 4x2 block decode enable/disable */
#define	VQABLOCK_4X4  0  /* 4x4 block decode enable/disable */
#define VQAWOOFER_ON  0
#else
#define VQASTANDALONE 0  /* Stand alone player */
#define VQAVOC_ON     0  /* Enable VOC file override */
#if PORTABLE
#define	VQAMONO_ON    0
#define VQADIRECT_SOUND 0
#ifdef PICO_BUILD
#define VQAPICO_SOUND 1
#else
#define VQASDL_SOUND 1
#endif
#else
#define	VQAMONO_ON    1  /* Mono display output enable/disable */
#define VQADIRECT_SOUND 1	/* Use windows direct sound system */
#define VQASDL_SOUND 0
#endif
#define VQAAUDIO_ON   1  /* Audio playback enable/disable */
#define VQAVIDEO_ON   0  /* Video manager enable/disable */
#define VQAMCGA_ON    0  /* MCGA enable/disable */
#define VQAXMODE_ON   0  /* Xmode enable/disable */
#define VQAVESA_ON    0  /* VESA enable/disable */
#define	VQABLOCK_2X2  0  /* 2x2 block decode enable/disable */
#define	VQABLOCK_2X3  0  /* 2x2 block decode enable/disable */
#define	VQABLOCK_4X2  1  /* 4x2 block decode enable/disable */
#define	VQABLOCK_4X4  0  /* 4x4 block decode enable/disable */
#define VQAWOOFER_ON  0
#endif



#if (VQAAUDIO_ON && VQADIRECT_SOUND)
#ifndef WIN32
#define WIN32 1
#ifndef _WIN32 // Denzil 6/2/98 Watcom 11.0 complains without this check
#define _WIN32
#endif // _WIN32
#endif
#undef WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <windowsx.h>
#include "dsound.h"
#endif



/*---------------------------------------------------------------------------
 * GENERAL CONSTANT DEFINITIONS
 *-------------------------------------------------------------------------*/

/* Playback modes. */
#define VQAMODE_RUN   0  /* Run the movie through the end. */
#define VQAMODE_WALK  1  /* Draw the next frame then return. */
#define VQAMODE_PAUSE 2  /* Suspend movie playback. */
#define VQAMODE_STOP  3  /* Stop the movie. */

/* Playback timer methods */
#define VQA_TMETHOD_DEFAULT -1 /* Use default timer method. */
#define	VQA_TMETHOD_DOS      1 /* DOS timer method */
#define	VQA_TMETHOD_INT      2 /* Interrupt timer method */
#define	VQA_TMETHOD_AUDIO    3 /* Audio timer method */

#define	VQA_TIMETICKS 60 /* Clock ticks per second */

/* Error/Status conditions */
#define VQAERR_NONE      0  /* No error */
#define	VQAERR_EOF      -1  /* Valid end of file */
#define VQAERR_OPEN     -2  /* Unable to open */
#define	VQAERR_READ     -3  /* Read error */
#define VQAERR_WRITE    -4  /* Write error */
#define VQAERR_SEEK     -5  /* Seek error */
#define VQAERR_NOTVQA   -6  /* Not a valid VQA file. */
#define VQAERR_NOMEM    -7  /* Unable to allocate memory */
#define	VQAERR_NOBUFFER -8  /* No buffer avail for load/draw */
#define	VQAERR_NOT_TIME -9  /* Not time for frame yet */
#define	VQAERR_SLEEPING -10 /* Function is in a sleep state */
#define VQAERR_VIDEO    -11 /* Video related error. */
#define VQAERR_AUDIO    -12 /* Audio related error. */
#define VQAERR_PAUSED   -13 /* In paused state. */

/* Event flags. */
#define VQAEVENT_PALETTE (1<<0)
#define VQAEVENT_SYNC    (1<<1)


/*---------------------------------------------------------------------------
 * STRUCTURES AND RELATED DEFINITIONS
 *-------------------------------------------------------------------------*/

/* VQAConfig: Player configuration structure
 *
 * DrawerCallback - User routine for Drawer to call each frame (NULL = none)
 * EventHandler   - User routine for notification to client of events.
 * NotifyFlags    - User specified events to be notified about.
 * Vmode          - Requested Video mode (May be promoted).
 * VBIBit         - Vertical blank bit polarity.
 * ImageBuf       - Pointer to caller's buffer for the Drawer to use as its
 *                  ImageBuf; NULL = player will allocate its own, if
 *                  VQACFGF_BUFFER is set in DrawFlags.
 * ImageWidth     - Width of Image buffer.
 * ImageHeight    - Height of Image buffer.
 * X1             - Draw window X coordinate (-1 = Center).
 * Y1             - Draw window Y coordinate (-1 = Center).
 * FrameRate      - Desired frames per second (-1 = use VQA header's value).
 * DrawRate       - Desired drawing frame rate; allows the Drawer to draw at
 *                  a separate rate from the Loader.
 * TimerMethod    - Timer method to use during playback.
 * DrawFlags      - Bits control various special drawing options. (See below)
 * OptionFlags    - Bits control various special misc options. (See below)
 * NumFrameBufs   - Desired number of frame buffers. (Default = 6)
 * NumCBBufs      - Desired number of codebook buffers. (Default = 3)
 * SoundObject		- Ptr to callers Direct Sound Object (Default =NULL)
 * PrimaryBufferPtr- Ptr to callers Primary Sound Buffer. (Default = NULL)
 * VocFile        - Name of VOC file to play instead of VQA audio track.
 * AudioBuf       - Pointer to audio buffer.
 * AudioBufSize   - Size of audio buffer. (Default = 32768)
 * AudioRate      - Audio data playback rate (-1 = use samplerate scaled
 *                  to the frame rate)
 * Volume         - Audio playback volume. (0x7FFF = max)
 * HMIBufSize     - Desired HMI buffer size. (Default = 2000)
 * DigiHandle     - Handle to an initialized sound driver. (-1 = none)
 * DigiCard       - HMI ID of card to use. (0 = none, -1 = auto-detect)
 * DigiPort       - Audio port address. (-1 = auto-detect)
 * DigiIRQ        - Audio IRQ. (-1 = auto-detect)
 * DigiDMA        - Audio DMA channel. (-1 = auto-detect)
 * Language       - Language identifier. (Not used)
 * CapFont        - Pointer to font to use for subtitle text captions.
 * EVAFont        - Pointer to font to use for E.V.A text cations. (For C&C)
 */
typedef struct _VQAConfig {
	long          			(*DrawerCallback)(unsigned char *screen, long framenum);
	long          			(*EventHandler)(unsigned long event,void *buffer,long nbytes);
	unsigned long 			NotifyFlags;
	long          			Vmode;
	long          			VBIBit;
	unsigned char 			*ImageBuf;
	long          			ImageWidth;
	long          			ImageHeight;
	long          			X1,Y1;
	long          			FrameRate;
	long          			DrawRate;
	long          			TimerMethod;
	long          			DrawFlags;
	long          			OptionFlags;
	long          			NumFrameBufs;
	long          			NumCBBufs;
#if VQASDL_SOUND
	uint32_t AudioDeviceID; // SDL_AudioDeviceID
	void (**AudioCallback)(uint8_t *, int);
	void *AudioSpec; // pointer to an SDL_AudioSpec
#endif
#if VQAPICO_SOUND
	void (**AudioCallback)(uint8_t *, int);
#endif
#if (VQADIRECT_SOUND)
	LPDIRECTSOUND			SoundObject;
	LPDIRECTSOUNDBUFFER	PrimaryBufferPtr;
#endif	//(VQADIRECT_SOUND)
	char          			*VocFile;
	unsigned char 			*AudioBuf;
	long          			AudioBufSize;
	long          			AudioRate;
	long          			Volume;
	long          			HMIBufSize;
	long          			DigiHandle;
	long          			DigiCard;
	long          			DigiPort;
	long          			DigiIRQ;
	long          			DigiDMA;
	long          			Language;
	char          			*CapFont;
	char          			*EVAFont; /* For C&C Only */
} VQAConfig;

/* Drawer Configuration flags (DrawFlags) */
#define	VQACFGB_BUFFER   0 /* Buffer UnVQ enable */
#define	VQACFGB_NODRAW   1 /* Drawing disable */
#define VQACFGB_NOSKIP   2 /* Disable frame skipping. */
#define	VQACFGB_VRAMCB   3 /* XMode VRAM copy enable */
#define VQACFGB_ORIGIN   4 /* 0,0 origin position */
#define	VQACFGB_SCALEX2  6 /* Scale X2 enable (VESA 320x200 to 640x400) */
#define VQACFGB_WOOFER   7
#define	VQACFGF_BUFFER   (1<<VQACFGB_BUFFER)
#define	VQACFGF_NODRAW   (1<<VQACFGB_NODRAW)
#define	VQACFGF_NOSKIP   (1<<VQACFGB_NOSKIP)
#define	VQACFGF_VRAMCB   (1<<VQACFGB_VRAMCB)
#define	VQACFGF_ORIGIN   (3<<VQACFGB_ORIGIN)
#define	VQACFGF_TOPLEFT  (0<<VQACFGB_ORIGIN)
#define	VQACFGF_TOPRIGHT (1<<VQACFGB_ORIGIN)
#define	VQACFGF_BOTRIGHT (2<<VQACFGB_ORIGIN)
#define	VQACFGF_BOTLEFT  (3<<VQACFGB_ORIGIN)
#define	VQACFGF_SCALEX2  (1<<VQACFGB_SCALEX2)
#define VQACFGF_WOOFER   (1<<VQACFGB_WOOFER)

/* Options Configuration (OptionFlags) */
#define	VQAOPTB_AUDIO    0 /* Audio enable. */
#define	VQAOPTB_STEP     1 /* Single step enable. */
#define	VQAOPTB_MONO     2 /* Mono output enable. */
#define VQAOPTB_PALOFF   3 /* Palette set disable. */
#define	VQAOPTB_SLOWPAL  4 /* Slow palette enable. */
#define VQAOPTB_HMIINIT  5 /* HMI already initialized by client. */
#define VQAOPTB_ALTAUDIO 6 /* Use alternate audio track. */
#define VQAOPTB_CAPTIONS 7 /* Show captions. */
#define VQAOPTB_EVA      8 /* Show EVA text (For C&C only) */
#define	VQAOPTF_AUDIO    (1<<VQAOPTB_AUDIO)
#define	VQAOPTF_STEP     (1<<VQAOPTB_STEP)
#define	VQAOPTF_MONO     (1<<VQAOPTB_MONO)
#define VQAOPTF_PALOFF   (1<<VQAOPTB_PALOFF)
#define	VQAOPTF_SLOWPAL  (1<<VQAOPTB_SLOWPAL)
#define VQAOPTF_HMIINIT  (1<<VQAOPTB_HMIINIT)
#define	VQAOPTF_ALTAUDIO (1<<VQAOPTB_ALTAUDIO)
#define VQAOPTF_CAPTIONS (1<<VQAOPTB_CAPTIONS)
#define VQAOPTF_EVA      (1<<VQAOPTB_EVA) /* For C&C only */


/* VQAInfo: Information about the VQA movie.
 *
 * NumFrames   - The number of frames contained in the movie.
 * ImageHeight - Height of image in pixels.
 * ImageWidth  - Width of image in pixels.
 * ImageBuf    - Pointer to the image buffer VQA draw into.
 */
typedef struct _VQAInfo {
	long NumFrames;
	long ImageWidth;
	long ImageHeight;
	unsigned char *ImageBuf;
} VQAInfo;


/* VQAStatistics: Statistics about the VQA movie played.
 *
 * StartTime     - Time movie started.
 * EndTime       - Time movie stoped.
 * FramesLoaded  - Total number of frames loaded.
 * FramesDrawn   - Total number of frames drawn.
 * FramesSkipped - Total number of frames skipped.
 * MaxFrameSize  - Size of largest frame.
 * SamplesPlayed - Number of sample bytes played.
 * MemUsed       - Total bytes used. (Low memory)
 */
typedef struct _VQAStatistics {
	long          StartTime;
	long          EndTime;
	long          FramesLoaded;
	long          FramesDrawn;
	long          FramesSkipped;
	long          MaxFrameSize;
	unsigned long SamplesPlayed;
	unsigned long MemUsed;
} VQAStatistics;


/* VQAHandle: VQA file handle. (Must be obtained by calling VQA_Alloc()
 *            and freed through VQA_Free(). This is the only legal way
 *            to obtain and dispose of a VQAHandle.
 *
 * VQAio - Something meaningful to the IO manager. (See DOCS)
 */
typedef struct _VQAHandle {
	uintptr_t VQAio;
} VQAHandle;

/* Possible IO command values */
#define VQACMD_INIT    1 /* Prepare the IO for a session */
#define VQACMD_CLEANUP 2 /* Terminate IO session */
#define VQACMD_OPEN    3 /* Open file */
#define VQACMD_CLOSE   4 /* Close file */
#define VQACMD_READ    5 /* Read bytes */
#define VQACMD_WRITE   6 /* Write bytes */
#define VQACMD_SEEK    7 /* Seek */


/*---------------------------------------------------------------------------
 * FUNCTION PROTOTYPES
 *-------------------------------------------------------------------------*/

/* Configuration routines. */
void VQA_INIConfig(VQAConfig *config);
void VQA_DefaultConfig(VQAConfig *config);

/* Handle manipulation routines. */
VQAHandle *VQA_Alloc(void);
void VQA_Free(VQAHandle *vqa);
void VQA_Reset(VQAHandle *vqa);
void VQA_InitAsDOS(VQAHandle *vqa);
void VQA_Init(VQAHandle *vqa, long(*iohandler)(VQAHandle *vqa, long action,
		void *buffer, long nbytes));
unsigned char *VQA_GetPalette(VQAHandle *vqa);
long VQA_GetPaletteSize(VQAHandle *vqa);
void VQA_Set_DrawBuffer(VQAHandle *vqa, unsigned char *buffer,
		unsigned long width, unsigned long height,
		long xpos, long ypos);

/* File routines. */
long VQA_Open(VQAHandle *vqa, char const *filename, VQAConfig *config);
void VQA_Close(VQAHandle *vqa);
long VQA_Play(VQAHandle *vqa, long mode);
long VQA_SeekFrame(VQAHandle *vqa, long frame, long fromwhere);
long VQA_SetStop(VQAHandle *vqa, long stop);

/* Information/statistics access routines. */
void VQA_GetInfo(VQAHandle *vqa, VQAInfo *info);
void VQA_GetStats(VQAHandle *vqa, VQAStatistics *stats);
char *VQA_Version(void);
char *VQA_IDString(void);

#endif /* VQAPLAY_H */

