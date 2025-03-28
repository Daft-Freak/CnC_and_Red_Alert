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

/****************************************************************************
*
*        C O N F I D E N T I A L -- W E S T W O O D  S T U D I O S
*
*----------------------------------------------------------------------------
*
* PROJECT
*     VQA player library. (32-Bit protected mode)
*
* FILE
*     loader.c
*
* DESCRIPTION
*     Stream loading and pre-processing.
*
* PROGRAMMER
*     Bill Randolph
*     Denzil E. Long, Jr.
*
* DATE
*     August 21, 1995
*
*----------------------------------------------------------------------------
*
* PUBLIC
*     VQA_Open      - Open a VQA file to play.
*     VQA_Close     - Close an opened VQA file.
*     VQA_LoadFrame - Load the next video frame from the VQA data stream.
*     VQA_SeekFrame - Position the movie stream to the specified frame.
*
* PRIVATE
*     AllocBuffers  - Allocates the numerous VQA play buffers
*     FreeBuffers   - Frees the VQA play buffers
*     PrimeBuffers  - Pre-Load the internal buffers.
*     Load_FINF     - Loads the Frame Info Table.
*     Load_VQHD     - Loads a VQA Header.
*     Load_CBF0     - Loads a full, uncompressed codebook
*     Load_CBFZ     - Loads a full, compressed codebook
*     Load_CBP0     - Loads a partial uncompressed codebook
*     Load_CBPZ     - Loads a partial compressed codebook
*     Load_CPL0     - Loads an uncompressed palette
*     Load_CPLZ     - Loads a compressed palette
*     Load_VPT0     - Loads uncompressed pointers
*     Load_VPTZ     - Loads compressed pointers
*     Load_VQF      - Loads a VQ Frame chunk
*     Load_SND0     - Loads an uncompressed sound chunk
*     Load_SND1     - Loads a compressed sound chunk
*     Load_AudFrame - Loads blocks from separate audio file, if needed.
*
****************************************************************************/

#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#ifdef _WIN32
#include <io.h>
#else
#include <unistd.h>
#endif
#include <stdlib.h>
#include "vq.h"
#include "vqaplayp.h"
#include <vqm32/all.h>
#include "caption.h"

/*---------------------------------------------------------------------------
 * PRIVATE DECLARATIONS
 *-------------------------------------------------------------------------*/

static VQAData *AllocBuffers(VQAHeader *header, VQAConfig *config);
static void FreeBuffers(VQAData *vqa, VQAConfig *config, VQAHeader *header);
static long PrimeBuffers(VQAHandle *vqa);
static long Load_VQF(VQAHandleP *vqap, unsigned long iffsize);
static long Load_FINF(VQAHandleP *vqap, unsigned long iffsize);
static long Load_VQHD(VQAHandleP *vqap, unsigned long iffsize);
static long Load_CBF0(VQAHandleP *vqap, unsigned long iffsize);
static long Load_CBFZ(VQAHandleP *vqap, unsigned long iffsize);
static long Load_CBP0(VQAHandleP *vqap, unsigned long iffsize);
static long Load_CBPZ(VQAHandleP *vqap, unsigned long iffsize);
static long Load_CPL0(VQAHandleP *vqap, unsigned long iffsize);
static long Load_CPLZ(VQAHandleP *vqap, unsigned long iffsize);
static long Load_VPT0(VQAHandleP *vqap, unsigned long iffsize);
static long Load_VPTZ(VQAHandleP *vqap, unsigned long iffsize);

#if(VQAAUDIO_ON)
static long Load_SND0(VQAHandleP *vqap, unsigned long iffsize);
static long Load_SND1(VQAHandleP *vqap, unsigned long iffsize);
static long Load_SND2(VQAHandleP *vqap, unsigned long iffsize);

#if(VQAVOC_ON)
static void Load_AudFrame(VQAHandleP *vqap);
#endif /* VQAVOC_ON */

#endif /* VQAAUDIO_ON */


extern "C"{
	void __cdecl Force_VM_Page_In (void *buffer, int length);
}

/****************************************************************************
*
* NAME
*     VQA_Open - Open a VQA file to play.
*
* SYNOPSIS
*     Error = VQA_Open(VQA, Name, Config)
*
*     long VQA_Open(VQAHandle *, char *, VQAConfig *);
*
* FUNCTION
*     - Open a VQA file for reading.
*     - Validate that it is an IFF file, of the VQA type.
*     - Read the VQA header.
*     - Open a VOC file for playback, if requested.
*     - Set the Loader's frame rate, if the caller's Config structure's
*       FrameRate is set to -1
*     - Set the Drawer's frame rate, if the caller's Config structure's
*       DrawRate is set to -1
*
* INPUTS
*     VQA    - Pointer to initialized handle. Obtained by VQA_Alloc().
*     Name   - Pointer to name of VQA file to open.
*     Config - Pointer to initialized VQA configuration structure.
*
* RESULT
*     Error - 0 if successful, or VQAERR_ error code.
*
****************************************************************************/

#define OPEN_VQHD     (1<<0)
#define OPEN_FINF     (1<<1)
#define OPEN_CAPTIONS (1<<2)
#define OPEN_EVA      (1<<3)
extern int VQAMovieDone;
long VQA_Open(VQAHandle *vqa, char const *filename, VQAConfig *config)
{
	VQAHandleP  *vqap;
	VQAHeader   *header;
	ChunkHeader chunk;
	long        max_frm_size;
	long        i;
	long        done;
	long        found;
	char        *ptr;

	/* Dereference commonly used data members for quicker access. */
	vqap = (VQAHandleP *)vqa;
	header = &vqap->Header;

	VQAMovieDone = 0;
	/*-------------------------------------------------------------------------
	 * VERIFY VALIDITY OF VQA FILE.
	 *-----------------------------------------------------------------------*/

	/* Open the file. */
	if (vqap->IOHandler(vqa, VQACMD_OPEN, (void *)filename, 0)) {
		return (VQAERR_OPEN);
	}

	/* Read the file ID & Size */
	if (vqap->IOHandler(vqa, VQACMD_READ, &chunk, 8)) {
		VQA_Close(vqa);
		return (VQAERR_READ);
	}

	/* Verify an IFF FORM */
	if ((chunk.id != ID_FORM) || (chunk.size == 0)) {
		VQA_Close(vqa);
		return (VQAERR_NOTVQA);
	}

	/* Read in WVQA ID */
	if (vqap->IOHandler(vqa, VQACMD_READ, &chunk, 4)) {
		VQA_Close(vqa);
		return (VQAERR_READ);
	}

	/* Verify VQA */
	if (chunk.id != ID_WVQA) {
		VQA_Close(vqa);
		return (VQAERR_NOTVQA);
	}

	/*-------------------------------------------------------------------------
	 * INITIALIZE THE PLAYERS CONFIGURATION
	 *-----------------------------------------------------------------------*/

	/* Use the clients configuration if they provided one. */
	if (config != NULL) {
		memcpy(&vqap->Config, config, sizeof(VQAConfig));
	} else {
		VQA_DefaultConfig(&vqap->Config);
	}

	/* Use the internal configuration structure from now on. */
	config = &vqap->Config;

	/*-------------------------------------------------------------------------
	 * PROCESS THE PRE-FRAME CHUNKS (VQHD, CAP, FINF, ETC...)
	 *-----------------------------------------------------------------------*/
	found = 0;
	done = 0;

	while (!done) {
		if (vqap->IOHandler(vqa, VQACMD_READ, &chunk, 8)) {
			VQA_Close(vqa);
			return (VQAERR_READ);
		}

		chunk.size = REVERSE_LONG(chunk.size);

		switch (chunk.id) {

			/*---------------------------------------------------------------------
			 * READ IN THE VQA HEADER.
			 *-------------------------------------------------------------------*/
			case ID_VQHD:
				if (chunk.size != sizeof(VQAHeader)) {
					VQA_Close(vqa);
					return (VQAERR_NOTVQA);
				}

				/* Read the header data. */
				if (vqap->IOHandler(vqa, VQACMD_READ, header, PADSIZE(chunk.size))) {
					VQA_Close(vqa);
					return (VQAERR_READ);
				}

				/*-------------------------------------------------------------------
				 * SETUP THE CONFIGURATION FROM THE HEADER.
				 *-----------------------------------------------------------------*/
				if (config->ImageWidth == -1) {
					config->ImageWidth = header->ImageWidth;
				}

				if (config->ImageHeight == -1) {
					config->ImageHeight = header->ImageHeight;
				}

				/* If Loaders frame rate is -1 then use the value from the header. */
				if (config->FrameRate == -1) {
					config->FrameRate = header->FPS;
				}

				/* If Drawers frame rate is -1 then use the value from the header,
				 * which will result in a "variable" frame rate.
				 */
				if (config->DrawRate == -1) {
					config->DrawRate = header->FPS;
				}

				/* Finally, if the DrawRate was set to -1 or 0 (ie MaxRate contained
				 * bogus values), set it to the header value.
				 */
				if ((config->DrawRate == -1) || (config->DrawRate == 0)) {
					config->DrawRate = header->FPS;
				}

				#if(VQAAUDIO_ON)
				/* If an alternate audio track is not available then turn it off.
				 * This enables the primary audio track to be played.
				 */
				if ((header->Version > VQAHD_VER1)
						&& !(header->Flags & VQAHDF_ALTAUDIO)) {
					config->OptionFlags &= ~VQAOPTF_ALTAUDIO;
				}
				#endif

				/*-------------------------------------------------------------------
				 * ALLOCATE THE BUFFERS THAT WE NEED TO PLAY THE VQA.
				 *-----------------------------------------------------------------*/
				if ((vqap->VQABuf = AllocBuffers(header, config)) == NULL) {
					VQA_Close(vqa);
					return (VQAERR_NOMEM);
				}

				found |= OPEN_VQHD;
				break;

			#if( VQACAPTIONS_ON )

			/*---------------------------------------------------------------------
			 * READ IN AND OPEN THE CAPTIONS STREAM.
			 *-------------------------------------------------------------------*/
			case ID_CAP0:
				if ((config->CapFont != NULL)
						&& (config->OptionFlags & VQAOPTF_CAPTIONS)) {

					short size = 0;

					/* Get uncompressed size of captions. */
					if (vqap->IOHandler(vqa, VQACMD_READ, &size, sizeof(short))) {
						VQA_Close(vqa);
						return (VQAERR_READ);
					}

					/* Allocate buffer for captions. */
					i = size + 50;

					if ((ptr = (char *)malloc(i)) == NULL) {
						VQA_Close(vqa);
						return (VQAERR_NOMEM);
					}

					/* Read in the captions chunk. */
					i -= PADSIZE(chunk.size);

					if (vqap->IOHandler(vqa, VQACMD_READ, (ptr + i),
							PADSIZE(chunk.size - sizeof(short)))) {

						free(ptr);
						VQA_Close(vqa);
						return (VQAERR_READ);
					}

					/* Decompress the captions. */
					LCW_Uncompress((ptr + i), ptr, size);
					vqap->Caption = OpenCaptions(ptr, config->CapFont);

					if (vqap->Caption == NULL) {
						VQA_Close(vqa);
						return (VQAERR_NOMEM);
					}

					found |= OPEN_CAPTIONS;
				} else {
					if (vqap->IOHandler(vqa, VQACMD_SEEK, (void *)SEEK_CUR,
							PADSIZE(chunk.size))) {
						VQA_Close(vqa);
						return (VQAERR_SEEK);
					}
				}
				break;


			case ID_EVA0:
				if ((config->EVAFont != NULL)
						&& (config->OptionFlags & VQAOPTF_EVA)) {

					short size = 0;

					/* Get uncompressed size of captions. */
					if (vqap->IOHandler(vqa, VQACMD_READ, &size, sizeof(short))) {
						VQA_Close(vqa);
						return (VQAERR_READ);
					}

					/* Allocate buffer for captions. */
					i = size + 50;

					if ((ptr = (char *)malloc(i)) == NULL) {
						VQA_Close(vqa);
						return (VQAERR_NOMEM);
					}

					/* Read in the captions chunk. */
					i -= PADSIZE(chunk.size);

					if (vqap->IOHandler(vqa, VQACMD_READ, (ptr + i),
							PADSIZE(chunk.size - sizeof(short)))) {
						free (ptr);
						VQA_Close(vqa);
						return (VQAERR_READ);
					}

					/* Decompress the captions. */
					LCW_Uncompress((ptr + i), ptr, size);
					vqap->EVA = OpenCaptions(ptr, config->EVAFont);

					if (vqap->EVA == NULL) {
						VQA_Close(vqa);
						return (VQAERR_NOMEM);
					}

					found |= OPEN_EVA;
				} else {
					if (vqap->IOHandler(vqa, VQACMD_SEEK, (void *)SEEK_CUR,
							PADSIZE(chunk.size))) {
						VQA_Close(vqa);
						return (VQAERR_SEEK);
					}
				}
				break;



			#endif

			/*---------------------------------------------------------------------
			 * READ FRAME INFORMATION
			 *-------------------------------------------------------------------*/
			case ID_FINF:
				if (Load_FINF(vqap, chunk.size)) {
					VQA_Close(vqa);
					return (VQAERR_READ);
				}

				done = 1;
				break;

			default:
				if (vqap->IOHandler(vqa, VQACMD_SEEK, (void *)SEEK_CUR,
						PADSIZE(chunk.size))) {
					VQA_Close(vqa);
					return (VQAERR_SEEK);
				}
				break;
		}
	}

	/*-------------------------------------------------------------------------
	 * INITIALIZE THE VIDEO SYSTEM IF WE ARE REQUIRED TO HANDLE THAT.
	 *-----------------------------------------------------------------------*/
	#if(VQAVIDEO_ON)
	if ((vqap->VQABuf->Drawer.Display = SetVideoMode(config->Vmode)) == 0) {
		VQA_Close(vqa);
		return (VQAERR_VIDEO);
	}

	/* Set the VBIBit polarity. */
	vqap->VQABuf->VBIBit = GetVBIBit();
	#else

	#if !PORTABLE
	if (config->VBIBit == -1) {
		config->VBIBit = TestVBIBit();
	}
	#endif

	vqap->VQABuf->VBIBit = config->VBIBit;

	#if( VQACAPTIONS_ON )

	if (found & OPEN_CAPTIONS|OPEN_EVA) {
		SetDAC(251,255,255,255); /* White */
		SetDAC(252,255,000,000); /* Red */
		SetDAC(253,000,255,000); /* Green */
		SetDAC(254,255,255,255);
		SetDAC(255,255,000,255); /* Cycle */
	}

	#endif

	#endif /* VQAVIDEO_ON */

	/*-------------------------------------------------------------------------
	 * AUDIO TRACK OVERRIDE FROM EXTERNAL FILE (.VOC)
	 *-----------------------------------------------------------------------*/

	/* Open VOC file if one is requested. */
	#if(VQAVOC_ON && VQAAUDIO_ON)
	if (config->VocFile != NULL) {
		vqap->vocfh = open(config->VocFile, (O_RDONLY|O_BINARY));
	} else {
		vqap->vocfh = -1;
	}

	/* Make sure we won't try to play audio. */
	if ((vqap->vocfh == -1) && ((header->Flags & VQAHDF_AUDIO) == 0)) {
		config->OptionFlags &= (~VQAOPTF_AUDIO);
	}
	#else  /* VQAVOC_ON */

	/* If the movie does not contain an audio track make sure we won't try
	 * to play one.
	 */
	if (((header->Flags & VQAHDF_AUDIO) == 0)) {
		config->OptionFlags &= (~VQAOPTF_AUDIO);
	}
	#endif /* VQAVOC_ON */

	/*-------------------------------------------------------------------------
	 * INITIALIZE THE AUDIO PLAYBACK/TIMING SYSTEM.
	 *-----------------------------------------------------------------------*/
	#if(VQAAUDIO_ON)
	if (config->OptionFlags & VQAOPTF_AUDIO) {
		VQAAudio *audio;

		/* Dereference for quick access. */
		audio = &vqap->VQABuf->Audio;

		/* Open HMI audio resource for playback. */
		if (VQA_OpenAudio(vqap , MainWindow)) {
			VQA_Close(vqa);
			return (VQAERR_AUDIO);
		}

		/* Initialize ADPCM information structure for audio stream. */
		VQA_sosCODECInitStream(&audio->ADPCM_Info);

		if (header->Version == VQAHD_VER1) {
			audio->ADPCM_Info.wBitSize = 8;
			audio->ADPCM_Info.dwUnCompSize = (22050L/header->FPS) * header->Frames;
			audio->ADPCM_Info.wChannels = 1;
		} else {
			audio->ADPCM_Info.wBitSize = audio->BitsPerSample;
			audio->ADPCM_Info.dwUnCompSize = (((audio->SampleRate / header->FPS)
					* (audio->BitsPerSample >> 3)) * audio->Channels) * header->Frames;

			audio->ADPCM_Info.wChannels = audio->Channels;
		}

		audio->ADPCM_Info.dwCompSize = audio->ADPCM_Info.dwUnCompSize
				/ (audio->ADPCM_Info.wBitSize / 4);
	}

	/* Turn off audio if the HMI DigiHandle is invalid. */
#if (!VQADIRECT_SOUND) && !VQASDL_SOUND && !VQAPICO_SOUND
	if (config->DigiHandle == -1) {
		config->OptionFlags &= ~VQAOPTF_AUDIO;
	}

	/* Setup the timer interrupt if the client requests it for the timing
	 * source.
	 */
	if (!(config->OptionFlags & VQAOPTF_AUDIO)
			|| (config->TimerMethod == VQA_TMETHOD_INT)) {

		/* Start HMI timer system for timing. */
		if (VQA_StartTimerInt(vqap, (config->OptionFlags & VQAOPTF_HMIINIT))) {
			VQA_Close(vqa);
			return (VQAERR_AUDIO);
		}
	}
#endif	//VQADIRECT_SOUND
	#endif /* VQAAUDIO_ON */

	/*-------------------------------------------------------------------------
	 * PRIME THE BUFFERS BY PRE-LOADING THEM WITH FRAME DATA.
	 *-----------------------------------------------------------------------*/
	if (PrimeBuffers(vqa) != 0) {
		VQA_Close(vqa);
		return (VQAERR_READ);
	}

	return (0);
}


/****************************************************************************
*
* NAME
*     VQA_Close - Close an opened VQA file.
*
* SYNOPSIS
*     VQA_Close(VQA)
*
*     void VQA_Close(VQAHandle *);
*
* FUNCTION
*     Close the file that was opened with VQA_Open().
*
* INPUTS
*     VQA - Pointer VQAHandle to close.
*
* RESULT
*     NONE
*
****************************************************************************/

void VQA_Close(VQAHandle *vqa)
{
	long (*iohandler)(VQAHandle *, long, void *, long);

	/* Restore video mode to text. */
	#if(VQAVIDEO_ON)
	SetVideoMode(TEXT_VIDEO);
	#endif /* VQAVIDEO_ON */

	/* Shutdown audio/timing system. */
	#if(VQAAUDIO_ON)
	if (((VQAHandleP *)vqa)->Config.OptionFlags & VQAOPTF_AUDIO) {
		VQA_CloseAudio((VQAHandleP *)vqa);
	} else {
		VQA_StopTimerInt((VQAHandleP *)vqa);
	}
	#endif /* VQAAUDIO_ON */

	#if( VQACAPTIONS_ON )

	/* Free captions. */
	if (((VQAHandleP *)vqa)->Caption != NULL) {
		if (((VQAHandleP *)vqa)->Caption->Buffer != NULL) {
			free(((VQAHandleP *)vqa)->Caption->Buffer);
		}

		CloseCaptions(((VQAHandleP *)vqa)->Caption);
	}

	/* Free EVA. */
	if (((VQAHandleP *)vqa)->EVA != NULL) {
		if (((VQAHandleP *)vqa)->EVA->Buffer != NULL) {
			free(((VQAHandleP *)vqa)->EVA->Buffer);
		}

		CloseCaptions(((VQAHandleP *)vqa)->EVA);
	}


	#endif

	/* Free memory */
	if (((VQAHandleP *)vqa)->VQABuf != NULL) {
		FreeBuffers(((VQAHandleP *)vqa)->VQABuf, &((VQAHandleP *)vqa)->Config,
				&((VQAHandleP *)vqa)->Header);
	}

	/* Close the VOC override file if one was opened */
	#if(VQAVOC_ON && VQAAUDIO_ON)
	if (((VQAHandleP *)vqa)->vocfh != -1) {
		close(((VQAHandleP *)vqa)->vocfh);
	}
	#endif /* VQAVOC_ON */

	/* Close the VQA file */
	((VQAHandleP *)vqa)->IOHandler(vqa, VQACMD_CLOSE, NULL, 0);

	/* Reset the VQAHandle */
	iohandler = ((VQAHandleP *)vqa)->IOHandler;
	memset(vqa, 0, sizeof(VQAHandleP));
	((VQAHandleP *)vqa)->IOHandler = iohandler;
}


/****************************************************************************
*
* NAME
*     VQA_LoadFrame - Load the next video frame from the VQA data stream.
*
* SYNOPSIS
*     Error = VQA_LoadFrame(VQA)
*
*     long VQA_LoadFrame(VQAHandle *);
*
* FUNCTION
*     The codebook is split up such that the last frame of every group gets
*     a new, complete codebook, ready for the next group.  The first codebook
*     in the VQA is a full codebook, and goes with the first frame's data.
*     Partial codebooks are stored per frame after that, and they add up to
*     a full codebook just before the first frame for the next group is read.
*
*     (Currently, this routine can read either the older non-frame-grouped
*     VQA file format, or the new frame-chunk format.  For the older format,
*     it's assumed that the last chunk in a frame is the pointer data.)
*
*     This routine also does a sort of "cooperative multitasking".  If the
*     Loader hits a "wait state" where it has to wait on the audio to finish
*     playing before it can continue to load, it sets a "sleep" flag and
*     just returns.  The sleep flag is checked on entry to see if it needs
*     to jump to the proper execution point. This may improve performance on
*     some platforms, but it also allows the Loader to be called regardless
*     of the size of the buffers; if the buffers fill up or the audio fails
*     to play, the Loader won't just get stuck.
*
* INPUTS
*     VQA - Pointer to VQAHandle structure.
*
* RESULT
*     Error - 0 if successful or VQAERR_??? error code.
*
****************************************************************************/

long VQA_LoadFrame(VQAHandle *vqa)
{
	VQAHandleP    *vqap;
	VQAData       *vqabuf;
	VQALoader     *loader;
	VQADrawer     *drawer;
	VQAFrameNode  *curframe;
	ChunkHeader   *chunk;
	unsigned long iffsize;
	long          frame_loaded = 0;

	/* Dereference commonly used data members for quicker access. */
	vqap = (VQAHandleP *)vqa;
	vqabuf = vqap->VQABuf;
	loader = &vqabuf->Loader;
	drawer = &((VQAHandleP *)vqa)->VQABuf->Drawer;
	curframe = loader->CurFrame;
	chunk = &loader->CurChunkHdr;

	iffsize = REVERSE_LONG(chunk->size);

	/* We have reached the end of the file if we loaded all the frames. */
	if (loader->CurFrameNum >= vqap->Header.Frames) {
		return (VQAERR_EOF);
	}

	/* If we're reading audio from a VOC file then service that requirement. */
	#if(VQAVOC_ON && VQAAUDIO_ON)
	if (vqap->vocfh != -1) {
		Load_AudFrame(vqap);
	}
	#endif /* VQAAUDIO_ON & VQAVOC_ON */

	/* If no buffer is available for loading then return. This allows the
	 * drawer to service one of the buffers more readily. (We'll wait for one
	 * to free up).
	 */
	if (curframe->Flags & VQAFRMF_LOADED) {
		loader->WaitsOnDrawer++;
		return (VQAERR_NOBUFFER);
	}

	/* If we're not sleeping, initialize */
	if (!(vqabuf->Flags & VQADATF_LSLEEP)) {
		frame_loaded = 0;
		loader->FrameSize = 0;

		/* Initialize the codebook ptr for the frame we're about to load:
		 * (This frame's codebook is the last full codebook; we have to init it
		 * now, because if we're on the last frame in a group, we'll get a new
		 * FullCB pointer.)
		 */
		curframe->Codebook = loader->FullCB;
	}

	/*-------------------------------------------------------------------------
	 * THE MAIN LOADER LOOP
	 *-----------------------------------------------------------------------*/
	while (frame_loaded == 0) {

		/* Read new chunk, only if we're not sleeping */
		if (!(vqabuf->Flags & VQADATF_LSLEEP)) {

			/* Read chunk ID */
			if (vqap->IOHandler(vqa, VQACMD_READ, chunk, 8)) {
				printf("EOF?\n");
				return (VQAERR_EOF);
			}

			iffsize = REVERSE_LONG(chunk->size);
			loader->FrameSize += iffsize;
		}

		/* Handle each chunk type */
		switch (chunk->id) {

			/* VQ Normal Frame */
			case ID_VQFR:
				if (Load_VQF(vqap, iffsize)) {
					return (VQAERR_READ);
				}

				frame_loaded = 1;
				break;

			/* VQ Key Frame */
			case ID_VQFK:
				if (Load_VQF(vqap, iffsize)) {
					return (VQAERR_READ);
				}

				/* Flag this frame as being key. */
				curframe->Flags |= VQAFRMF_KEY;
				frame_loaded = 1;
				break;

			/* Full uncompressed codebook */
			case ID_CBF0:
				if (Load_CBF0(vqap, iffsize)) {
					return (VQAERR_READ);
				}
				break;

			/* Full compressed codebook */
			case ID_CBFZ:
				if (Load_CBFZ(vqap, iffsize)) {
					return (VQAERR_READ);
				}
				break;

			/* Partial uncompressed codebook */
			case ID_CBP0:
				if (Load_CBP0(vqap, iffsize)) {
					return (VQAERR_READ);
				}
				break;

			/* Partial compressed codebook */
			case ID_CBPZ:
				if (Load_CBPZ(vqap, iffsize)) {
					return (VQAERR_READ);
				}
				break;

			/* Uncompressed palette */
			case ID_CPL0:
				if (Load_CPL0(vqap, iffsize)) {
					return (VQAERR_READ);
				}

				/* If this is the first occurance of a palette then store it now.
				 * This functionality is needed for Monopoly!
				 */
				if (drawer->CurPalSize == 0) {
					memcpy(drawer->Palette_24,curframe->Palette,curframe->PaletteSize);
					drawer->CurPalSize = curframe->PaletteSize;
				}

				/* Flag this frame as having a palette. */
				curframe->Flags |= VQAFRMF_PALETTE;
				break;

			/* Compressed palette */
			case ID_CPLZ:
				if (Load_CPLZ(vqap, iffsize)) {
					return (VQAERR_READ);
				}

				/* If this is the first occurance of a palette then store it now.
				 * This functionality is needed for Monopoly!
				 */
				if (drawer->CurPalSize == 0) {
					drawer->CurPalSize = LCW_Uncompress((char *)curframe->Palette
							+ curframe->PalOffset, (char *)drawer->Palette_24,
							vqabuf->Max_Pal_Size);
				}

				/* Flag this frame as having a palette. */
				curframe->Flags |= VQAFRMF_PALETTE;
				break;

			/* Uncompressed pointer data */
			case ID_VPT0:
				if (Load_VPT0(vqap, iffsize)) {
					return (VQAERR_READ);
				}

				frame_loaded = 1;
				break;

			/* Compressed pointer data */
			case ID_VPTZ:
			case ID_VPTD:
				if (Load_VPTZ(vqap, iffsize)) {
					return (VQAERR_READ);
				}

				frame_loaded = 1;
				break;

			/* Pointer data Key (Must draw) */
			case ID_VPTK:
				if (Load_VPTZ(vqap, iffsize)) {
					return (VQAERR_READ);
				}

				/* Flag this frame as being key. */
				curframe->Flags |= VQAFRMF_KEY;
				frame_loaded = 1;
				break;

			/* Uncompressed audio frame.
			 *
			 *  - Make sure the sound load buffer (Audio.TempBuf) is empty; if not
			 *    go into a sleep state.
			 *  - Load the data into TempBuf.
			 */
			#if(VQAAUDIO_ON)
			case ID_SND0:
				if (!(vqap->Config.OptionFlags & VQAOPTF_ALTAUDIO)) {

					/* Move the last audio frame to the play buffer. */
					if (CopyAudio(vqap) == VQAERR_SLEEPING) {
						vqabuf->Flags |= VQADATF_LSLEEP;
						return (VQAERR_SLEEPING);
					} else {
						vqabuf->Flags &= (~VQADATF_LSLEEP);
					}

					/* Load an uncompressed audio frame. */
					if (Load_SND0(vqap, iffsize) != 0) {
						return (VQAERR_READ);
					}
				} else {
					if (vqap->IOHandler(vqa, VQACMD_SEEK, (void *)SEEK_CUR,
							PADSIZE(iffsize))) {
						return (VQAERR_SEEK);
					}
				}
				break;

			case ID_SNA0:
				if (vqap->Config.OptionFlags & VQAOPTF_ALTAUDIO) {

					/* Move the last audio frame to the play buffer. */
					if (CopyAudio(vqap) == VQAERR_SLEEPING) {
						vqabuf->Flags |= VQADATF_LSLEEP;
						return (VQAERR_SLEEPING);
					} else {
						vqabuf->Flags &= (~VQADATF_LSLEEP);
					}

					/* Load an uncompressed audio frame. */
					if (Load_SND0(vqap, iffsize) != 0) {
						return (VQAERR_READ);
					}
				} else {
					if (vqap->IOHandler(vqa, VQACMD_SEEK, (void *)SEEK_CUR,
							PADSIZE(iffsize))) {
						return (VQAERR_SEEK);
					}
				}
				break;

			/* Compressed audio frame.
			 *
			 *  - Make sure the sound load buffer (Audio.TempBuf) is empty; if not
			 *    go into a sleep state.
			 *  - Load the data into TempBuf.
			 */
			case ID_SND1:
				if (!(vqap->Config.OptionFlags & VQAOPTF_ALTAUDIO)) {

					/* Move the last audio frame to the play buffer. */
					if (CopyAudio(vqap) == VQAERR_SLEEPING) {
						vqabuf->Flags |= VQADATF_LSLEEP;
						return (VQAERR_SLEEPING);
					} else {
						vqabuf->Flags &= (~VQADATF_LSLEEP);
					}

					/* Load a compressed audio frame. */
					if (Load_SND1(vqap, iffsize) != 0) {
						return (VQAERR_READ);
					}
				} else {
					if (vqap->IOHandler(vqa, VQACMD_SEEK, (void *)SEEK_CUR,
							PADSIZE(iffsize))) {
						return (VQAERR_SEEK);
					}
				}
				break;

			case ID_SNA1:
				if (vqap->Config.OptionFlags & VQAOPTF_ALTAUDIO) {

					/* Move the last audio frame to the play buffer. */
					if (CopyAudio(vqap) == VQAERR_SLEEPING) {
						vqabuf->Flags |= VQADATF_LSLEEP;
						return (VQAERR_SLEEPING);
					} else {
						vqabuf->Flags &= (~VQADATF_LSLEEP);
					}

					/* Load a compressed audio frame. */
					if (Load_SND1(vqap, iffsize) != 0) {
						return (VQAERR_READ);
					}
				} else {
					if (vqap->IOHandler(vqa, VQACMD_SEEK, (void *)SEEK_CUR,
							PADSIZE(iffsize))) {
						return (VQAERR_SEEK);
					}
				}
				break;

			/* HMI ADPCM compressed audio frame.
			 *
			 *  - Make sure the sound load buffer (Audio.TempBuf) is empty; if not
			 *    go into a sleep state.
			 *  - Load the data into TempBuf.
			 */
			case ID_SND2:
				if (!(vqap->Config.OptionFlags & VQAOPTF_ALTAUDIO)) {

					/* Move the last audio frame to the play buffer. */
					if (CopyAudio(vqap) == VQAERR_SLEEPING) {
						vqabuf->Flags |= VQADATF_LSLEEP;
						return (VQAERR_SLEEPING);
					} else {
						vqabuf->Flags &= (~VQADATF_LSLEEP);
					}

					/* Load a compressed audio frame. */
					if (Load_SND2(vqap, iffsize) != 0) {
						return (VQAERR_READ);
					}
				} else {
					if (vqap->IOHandler(vqa, VQACMD_SEEK, (void *)SEEK_CUR,
							PADSIZE(iffsize))) {
						return (VQAERR_SEEK);
					}
				}
				break;

			case ID_SNA2:
				if (vqap->Config.OptionFlags & VQAOPTF_ALTAUDIO) {

					/* Move the last audio frame to the play buffer. */
					if (CopyAudio(vqap) == VQAERR_SLEEPING) {
						vqabuf->Flags |= VQADATF_LSLEEP;
						return (VQAERR_SLEEPING);
					} else {
						vqabuf->Flags &= (~VQADATF_LSLEEP);
					}

					/* Load a compressed audio frame. */
					if (Load_SND2(vqap, iffsize) != 0) {
						return (VQAERR_READ);
					}
				} else {
					if (vqap->IOHandler(vqa, VQACMD_SEEK, (void *)SEEK_CUR,
							PADSIZE(iffsize))) {
						return (VQAERR_SEEK);
					}
				}
				break;
			#endif

			/* Skip any unknown chunks. */
			default:
				if (vqap->IOHandler(vqa, VQACMD_SEEK, (void *)SEEK_CUR,
						PADSIZE(iffsize))) {
					return (VQAERR_SEEK);
				}
				break;
		}
	}

	/* Update maximum frame size stat. */
	if ((loader->CurFrameNum>0) && (loader->FrameSize>loader->MaxFrameSize)) {
		loader->MaxFrameSize = loader->FrameSize;
	}

	/*-------------------------------------------------------------------------
	 * SET UP THE FRAME FOR DRAWING.
	 *-----------------------------------------------------------------------*/

	/* Set the frame # */
	curframe->FrameNum = loader->CurFrameNum;
	loader->CurFrameNum++;

	/* Update data for mono output */
	loader->LastFrameNum = loader->CurFrameNum;

	/* Loader is finished with this frame; tell Drawer to draw it */
	curframe->Flags |= VQAFRMF_LOADED;
	loader->CurFrame = curframe->Next;

	return (0);
}


/****************************************************************************
*
* NAME
*     VQA_SeekFrame - Position the movie stream to the specified frame.
*
* SYNOPSIS
*     Frame = VQA_SeekFrame(VQA, Frame, FromWhere)
*
*     long VQA_SeekFrame(VQAHandle *, long, long);
*
* FUNCTION
*     This function sets the movie stream to the new frame specified by
*     the 'offset' parameter. 'FromWhere' is a symbolic constant that is used
*     to specify from where in the stream offset should be applied.
*
* INPUTS
*     VQA       - Pointer to VQAHandle of movie to seek into.
*     Frame     - Frame to seek to.
*     FromWhere - Relative position indicator.
*
* RESULT
*     Frame - New frame position or -1 if error.
*
****************************************************************************/

long VQA_SeekFrame(VQAHandle *vqa, long framenum, long fromwhere)
{
	VQAHandleP   *vqap;
	VQAData      *vqabuf;
	VQALoader    *loader;
	VQAHeader    *header;
	VQAFrameNode *frame;
	VQAConfig    *config;
	long         group;
	long         i;
	long         rc = VQAERR_NONE;

	#if(VQAAUDIO_ON)
	VQAAudio     *audio;
	long         audio_on;
 	#endif

	/* Dereference commonly used data members for quick access. */
	vqap = (VQAHandleP *)vqa;
	vqabuf = vqap->VQABuf;
	loader = &vqabuf->Loader;
	header = &vqap->Header;
	config = &vqap->Config;

	fromwhere = fromwhere;

	#if(VQAAUDIO_ON)
	audio = &vqabuf->Audio;

	/* Stop audio playback. */
	audio_on = (audio->Flags & VQAAUDF_ISPLAYING);
	VQA_StopAudio(vqap);
	#endif

	/* Make sure the requested frame is valid and the frame information
	 * array is allocated before continuing.
	 */
	if ((framenum < header->Frames) && (vqabuf->Foff != NULL)) {

		/* Find and load the most recent palette. */
		if (!(config->OptionFlags & VQAOPTF_PALOFF)) {

			/* Get the current frame. */
			frame = loader->CurFrame;

			for (i = framenum; i >= 0; i--) {
				if (vqabuf->Foff[i] & VQAFINF_PAL) {

					/* Seek to the palette frame. */
					rc = vqap->IOHandler(vqa, VQACMD_SEEK, (void *)SEEK_SET,
							VQAFRAME_OFFSET(vqabuf->Foff[i]));

					/* Fool the loader into thinking this frame is empty. */
					if (!rc) {
						loader->NumPartialCB = 0;
						loader->PartialCBSize = 0;
						loader->FullCB = vqabuf->CBData;
						loader->CurCB = vqabuf->CBData;
						loader->CurFrameNum = 0;
						frame->Flags = 0;

						/* Load the frame with the palette. */
						if (VQA_LoadFrame(vqa) == 0) {

							/* Decompress the palette if neccessary.*/
							if (frame->Flags & VQAFRMF_PALCOMP) {
								frame->PaletteSize = LCW_Uncompress((char *)frame->Palette
										+ frame->PalOffset, (char *)frame->Palette,
 										vqabuf->Max_Pal_Size);
							}

							SetPalette(frame->Palette, frame->PaletteSize, 0);
						}
					} else {
						rc = VQAERR_SEEK;
					}
					break;
				}
			}
		}

		/* Build the codebook for the frame we are seeking to. */
		if (!rc) {

			/* Compute the starting group frame of the requested frame. */
			group = (framenum / header->Groupsize);
			group = (group * header->Groupsize);

			/* The codebook for the group we want to goto is found in the previous
			 * group, with the exception of the very first group.
			 */
			if (group >= header->Groupsize) {
				group -= header->Groupsize;
			}

			/* Seek to the start of the group containing the partial codebooks for
			 * the target frame.
			 */
			if (!vqap->IOHandler(vqa, VQACMD_SEEK, (void *)SEEK_SET,
					VQAFRAME_OFFSET(vqabuf->Foff[group]))) {

				/* Throw away any audio frames that were loaded. */
				#if(VQAAUDIO_ON)
				if ((config->OptionFlags & VQAOPTF_AUDIO)
						&& (audio->Buffer != NULL)) {
					memset(audio->IsLoaded, 0, audio->NumAudBlocks * sizeof(short));
					memset(audio->Buffer, 0, config->AudioBufSize);

					/* Position the audio buffer to 1/2 second. */
					audio->AudBufPos = (long)(((audio->SampleRate * audio->Channels)
							* (audio->BitsPerSample >> 3)) / 2);

					/* Mark 1/2 second of the audio buffer as loaded. */
					for (i = 0; i < (audio->AudBufPos / config->HMIBufSize); i++) {
						audio->IsLoaded[i] = 1;
					}
				}
				#endif

				/* Force the loader to the desired frame. */
				loader->NumPartialCB = 0;
				loader->PartialCBSize = 0;
				loader->FullCB = vqabuf->CBData;
				loader->CurCB = vqabuf->CBData;
				loader->CurFrameNum = group;

				/* Load frames up to the target frame collecting partial codebooks
				 * along the way.
				 */
				for (i = 0; i < (framenum - group); i++) {

					/* Fool the loader into thinking the frame has been drawn. */
					loader->CurFrame->Flags = 0;

					#if(VQAAUDIO_ON)
					audio->TempBufLen = 0;
					#endif

					/* Load the frame. */
					if ((rc = VQA_LoadFrame(vqa)) != 0) {
						if ((rc != VQAERR_NOBUFFER) && (rc != VQAERR_SLEEPING)) {
							break;
						} else {
							rc = 0;
						}
					}
				}

				/* If everything is okay, then re-prime the buffers. */
				if (!rc) {

					/* Mark all the frames except the current one as empty. */
					loader->CurFrame->Flags = 0;
					frame = loader->CurFrame->Next;

					while (frame != loader->CurFrame) {
						frame->Flags = 0;
						frame = frame->Next;
					}

					/* Set the drawer to the current frame and the loader
					 * to the next.
					 */
					vqabuf->Drawer.CurFrame = loader->CurFrame;

					/* Prime the buffers for the new position. */
					rc = PrimeBuffers(vqa);

					/* An end of file is not considered and error. */
					if ((rc == 0) || (rc == VQAERR_EOF)) {
						rc = framenum;
					}
				}
			} else {
				rc = VQAERR_SEEK;
			}
		}
	}

	/* Restart audio playback. */
	#if(VQAAUDIO_ON)
	if (audio_on) {
		VQA_StartAudio(vqap);
	}
	#endif

	return (rc);
}


/****************************************************************************
*
* NAME
*     AllocBuffers - Allocate VQA play buffers.
*
* SYNOPSIS
*     VQAData = AllocBuffers(Header, Config)
*
*     VQAData *AllocBuffers(VQAHeader *, VQAConfig *);
*
* FUNCTION
*     For those structures that contain buffer pointers (codebook nodes,
*     frame buffer nodes), enough memory is allocated for both the structure
*     and its associated buffers, then the buffer pointers are pointed to
*     the appropriate offset from the structure pointer.  This allows us
*     to perform only one malloc & free for each node.
*
*     Buffers allocated:
*       - vqa
*       - vqa->CBData (list)
*       - vqa->FrameData (list)
*       - vqa->Drawer.ImageBuf
*       - vqa->Audio.Buffer
*       - vqa->Audio.IsLoaded
*       - vqa->Foff
*
* INPUTS
*     Header - Pointer to VQAHeader structure.
*     Config - Pointer to VQA configuration structure.
*
* RESULT
*     VQAData - Pointer to initialized VQAData structure.
*
****************************************************************************/

static VQAData *AllocBuffers(VQAHeader *header, VQAConfig *config)
{
	VQAData      *vqa;
	VQACBNode    *cbnode;
	VQACBNode    *this_cb;
	VQAFrameNode *framenode;
	VQAFrameNode *this_frame;
	long         i;

	/* Check the configuration for valid values. */
	if ((config->NumCBBufs == 0) || (config->NumFrameBufs == 0)) {
		return (NULL);
	}

	/* Allocate the master structure */
	if ((vqa = (VQAData *)malloc(sizeof(VQAData))) == NULL) {
		return (NULL);
	}

	/*-------------------------------------------------------------------------
	 * INITIALIZE THE VQA DATA STRUCTURES.
	 *
	 * Pointers are set to NULL initially, and filled in as the buffers are
	 * allocated.  The Max buffer sizes are computed with 1K of padding,
	 * and'd with 0xFFFC to make the size divisible by 4, to ensure DWORD
	 * alignment.
	 *-----------------------------------------------------------------------*/
#if (!VQADIRECT_SOUND)
	DPMI_Lock(vqa, sizeof(VQAData));
#endif	//VQADIRECT_SOUND

	memset(vqa, 0, sizeof(VQAData));
	vqa->MemUsed = sizeof(VQAData);
	vqa->Drawer.LastTime = (-VQA_TIMETICKS);

	/* Set maximum codebook size. */
	vqa->Max_CB_Size = ((header->CBentries) * header->BlockWidth
			* header->BlockHeight + 250) & 0xFFFC;

	/* Set maximum palette size. */
	vqa->Max_Pal_Size = (768 + 1024) & 0xFFFC;

	/* Set maximum vector pointers size. */
	vqa->Max_Ptr_Size = ((header->ImageWidth / header->BlockWidth)
			* (header->ImageHeight / header->BlockHeight)
			* sizeof(short) + 1024) & 0xFFFC;

	/* Set the frame number of the frame containing the last codebook. */
	vqa->Loader.LastCBFrame = (((header->Frames - 1) / header->Groupsize)
			* header->Groupsize);

	/*-------------------------------------------------------------------------
	 * ALLOCATE THE CODEBOOK BUFFERS.
	 *-----------------------------------------------------------------------*/
	for (i = 0; i < config->NumCBBufs; i++) {

		/* Allocate a codebook node. */
		cbnode = (VQACBNode *)malloc((sizeof(VQACBNode) + vqa->Max_CB_Size));

		/* If failure then clean up and exit. */
		if (cbnode == NULL) {
			FreeBuffers(vqa, config, header);
			return (NULL);
		}

		/* Lock the buffer to prevent page swapping. */
#if (!VQADIRECT_SOUND)
		DPMI_Lock(cbnode, (sizeof(VQACBNode) + vqa->Max_CB_Size));
#else	//!VQADIRECT_SOUND
		/* Make sure the allocated memory is paged in */
		Force_VM_Page_In(cbnode, (sizeof(VQACBNode) + vqa->Max_CB_Size));
#endif	//(!VQADIRECT_SOUND)



		/* Keep count of the memory usage. */
		vqa->MemUsed += (long)(sizeof(VQACBNode) + vqa->Max_CB_Size);

		/* Initialize the node */
		memset(cbnode, 0, sizeof(VQACBNode));
		cbnode->Buffer = (unsigned char *)cbnode + sizeof(VQACBNode);

		/* Install the node */
		if (i == 0) {
			vqa->CBData = cbnode;
			this_cb = cbnode;
		} else {
			this_cb->Next = cbnode;
			this_cb = cbnode;
		}
	}

	/* Make the list circular */
	cbnode->Next = vqa->CBData;

	/* Install the Codebook list */
	vqa->Loader.CurCB = vqa->CBData;
	vqa->Loader.FullCB = vqa->CBData;

	/*-------------------------------------------------------------------------
	 * ALLOCATE THE FRAME BUFFERS.
	 *-----------------------------------------------------------------------*/
	for (i = 0; i < config->NumFrameBufs; i++) {

		/* Allocate a pointer node */
		framenode = (VQAFrameNode *)malloc((sizeof(VQAFrameNode)
				+ vqa->Max_Ptr_Size + vqa->Max_Pal_Size));

		/* If failure then clean up and exit. */
		if (framenode == NULL) {
			FreeBuffers(vqa, config, header);
			return (NULL);
		}

		/* Lock the buffer to prevent page swapping. */
#if (!VQADIRECT_SOUND)
		DPMI_Lock(framenode, sizeof(VQAFrameNode) + vqa->Max_Ptr_Size
				+ vqa->Max_Pal_Size);
#else	//!VQADIRECT_SOUND
		/* Make sure the allocated memory is paged in */
		Force_VM_Page_In(framenode, sizeof(VQAFrameNode) + vqa->Max_Ptr_Size
								+ vqa->Max_Pal_Size);
#endif	//(!VQADIRECT_SOUND)

		/* Keep count of the memory usage. */
		vqa->MemUsed += (long)(sizeof(VQAFrameNode) + vqa->Max_Ptr_Size
				+ vqa->Max_Pal_Size);

		/* Initialize the node */
		memset(framenode, 0, sizeof(VQAFrameNode));
		framenode->Pointers = (unsigned char *)framenode + sizeof(VQAFrameNode);
		framenode->Palette = (unsigned char *)framenode + sizeof(VQAFrameNode)
				+ vqa->Max_Ptr_Size;

		framenode->Codebook = vqa->CBData;

		/* Install the node */
		if (i == 0) {
			vqa->FrameData = framenode;
			this_frame = framenode;
		} else {
			this_frame->Next = framenode;
			this_frame = framenode;
		}
	}

	/* Make the list circular */
	framenode->Next = vqa->FrameData;

	/* Install the Frame Buffer list */
	vqa->Loader.CurFrame = vqa->FrameData;
	vqa->Drawer.CurFrame = vqa->FrameData;
	vqa->Flipper.CurFrame = vqa->FrameData;

	/*-------------------------------------------------------------------------
	 * ALLOCATE THE IMAGE BUFFERS IF ONE IS NOT ALREADY PROVIDED.
	 *-----------------------------------------------------------------------*/
	if (config->ImageBuf == NULL) {

		/* Allocate our own buffer. */
		if (config->DrawFlags & VQACFGF_BUFFER) {
			vqa->Drawer.ImageBuf = (unsigned char *)malloc((header->ImageWidth
					* header->ImageHeight));

			/* If the allocation failed we must free up and exit. */
			if (vqa->Drawer.ImageBuf == NULL) {
				FreeBuffers(vqa, config, header);
				return (NULL);
			}

			/* Lock to prevent page swapping. */
#if (!VQADIRECT_SOUND)
			DPMI_Lock(vqa->Drawer.ImageBuf,header->ImageWidth*header->ImageHeight);
#else	//!VQADIRECT_SOUND
			/* Make sure the allocated memory is paged in */
			Force_VM_Page_In(vqa->Drawer.ImageBuf,header->ImageWidth*header->ImageHeight);
#endif	//(!VQADIRECT_SOUND)

			/* Plugin image buffer information. */
			vqa->Drawer.ImageWidth = header->ImageWidth;
			vqa->Drawer.ImageHeight = header->ImageHeight;
			vqa->MemUsed += (long)(header->ImageWidth * header->ImageHeight);
		} else {
			vqa->Drawer.ImageWidth = config->ImageWidth;
			vqa->Drawer.ImageHeight = config->ImageHeight;
		}
	} else {

		/* Use caller provided buffer */
		vqa->Drawer.ImageBuf = config->ImageBuf;
		vqa->Drawer.ImageWidth = config->ImageWidth;
		vqa->Drawer.ImageHeight = config->ImageHeight;
	}

	/*-------------------------------------------------------------------------
	 * ALLOCATE AND INITIALIZE AUDIO BUFFERS AND STRUCTURES.
	 *-----------------------------------------------------------------------*/
	#if(VQAAUDIO_ON)
	if ((header->Flags & VQAHDF_AUDIO)
			&& (config->OptionFlags & VQAOPTF_AUDIO)) {

		/* Dereference audio structure for quick access. */
		VQAAudio *audio = &vqa->Audio;

		/* Version 1 VQA's only supported 22050 8 bit mono audio. */
		if (header->Version < VQAHD_VER2) {
			audio->SampleRate = 22050U;
			audio->Channels = 1;
			audio->BitsPerSample = 8;
			audio->BytesPerSec = 22050;
		} else {
			if ((config->OptionFlags & VQAOPTF_ALTAUDIO)
					&& (header->Flags & VQAHDF_ALTAUDIO)) {
				audio->SampleRate = header->AltSampleRate;
				audio->Channels = header->AltChannels;
				audio->BitsPerSample = header->AltBitsPerSample;
			} else {
				audio->SampleRate = header->SampleRate;
				audio->Channels = header->Channels;
				audio->BitsPerSample = header->BitsPerSample;
			}

			audio->BytesPerSec = ((audio->SampleRate * audio->Channels)
					* (audio->BitsPerSample >> 3));
		}

		/* Adjust the HMI buffer to accomodate the amount of data. */
		#if(0)
		config->HMIBufSize *= (audio->SampleRate / 22050);
		config->HMIBufSize *= audio->Channels * (audio->BitsPerSample >> 3);
		#endif

		/* The default audio buffer size should be large enough to hold
		 * 1.5 seconds of data.
		 */
		if (config->AudioBufSize == -1) {

			/* Compute the number of HMI buffers that will completly fit into
			 * 1.5 seconds of audio data.
			 */
			i = ((audio->BytesPerSec+(audio->BytesPerSec/2))/config->HMIBufSize);
			config->AudioBufSize = (config->HMIBufSize * i);
		}

		/* Do not allocate anything if the audio buffer is zero length. */
		if (config->AudioBufSize > 0) {

			/* Allocate an audio buffer if the user did not provide one.
			 * Otherwise, use the user supplied buffer.
			 */
			if (config->AudioBuf == NULL) {
				audio->Buffer = (unsigned char *)malloc(config->AudioBufSize);

				/* If failure then clean up and exit. */
				if (audio->Buffer == NULL) {
					FreeBuffers(vqa, config, header);
					return (NULL);
				}

#if (!VQADIRECT_SOUND)
				DPMI_Lock(audio->Buffer, config->AudioBufSize);
#else	//!VQADIRECT_SOUND
				/* Make sure the allocated memory is paged in */
				Force_VM_Page_In(audio->Buffer, config->AudioBufSize);
#endif	//(!VQADIRECT_SOUND)

				/* Add audio buffer size to memory usage. */
				vqa->MemUsed += config->AudioBufSize;
			} else {
				audio->Buffer = config->AudioBuf;
			}

			/* Allocate IsLoaded flags */
			audio->NumAudBlocks = (config->AudioBufSize / config->HMIBufSize);
			audio->IsLoaded = (short *)malloc(audio->NumAudBlocks * sizeof(short));

			/* If failure then clean up and exit. */
			if (audio->IsLoaded == NULL) {
				FreeBuffers(vqa, config, header);
				return (NULL);
			}

			/* Lock to prevent page swapping. */
#if (!VQADIRECT_SOUND)
			DPMI_Lock(audio->IsLoaded, audio->NumAudBlocks * sizeof(short));
#else	//!VQADIRECT_SOUND
			/* Make sure the allocated memory is paged in */
			Force_VM_Page_In(audio->IsLoaded, audio->NumAudBlocks * sizeof(short));
#endif	//(!VQADIRECT_SOUND)

			/* Add IsLoaded flags array to memory usage. */
			vqa->MemUsed += (audio->NumAudBlocks * sizeof(short));

			/* Initialize audio IsLoaded flags to false. */
			memset(audio->IsLoaded, 0, audio->NumAudBlocks * sizeof(short));

			/* Allocate temporary staging buffer for the audio frames. */
			audio->TempBufSize = ((audio->BytesPerSec / header->FPS) * 2) + 100;
			audio->TempBuf = (unsigned char *)malloc(audio->TempBufSize);

			if (audio->TempBuf == NULL) {
				FreeBuffers(vqa, config, header);
				return (NULL);
			}

			/* Lock to prevent page swapping. */
#if (!VQADIRECT_SOUND)
			DPMI_Lock(audio->TempBuf, audio->TempBufSize);
#else	//!VQADIRECT_SOUND
			/* Make sure the allocated memory is paged in */
			Force_VM_Page_In(audio->TempBuf, audio->TempBufSize);
#endif	//(!VQADIRECT_SOUND)

			/* Add temporary buffer size to memory usage. */
			vqa->MemUsed += audio->TempBufSize;
		}
	}
	#endif /* VQAAUDIO_ON */

	/*-------------------------------------------------------------------------
	 * ALLOCATE THE FRAME INFORMATION TABLE IF REQUESTED.
	 *-----------------------------------------------------------------------*/
	vqa->Foff = (long *)malloc(header->Frames * sizeof(long));

	if (vqa->Foff == NULL) {
		FreeBuffers(vqa, config, header);
		return (NULL);
	}

	/* Lock to prevent page swapping. */
#if (!VQADIRECT_SOUND)
	DPMI_Lock(vqa->Foff, header->Frames * sizeof(long));
#else	//!VQADIRECT_SOUND
	/* Make sure the allocated memory is paged in */
	Force_VM_Page_In(vqa->Foff, header->Frames * sizeof(long));
#endif	//(!VQADIRECT_SOUND)

	/* Keep a running total of memory usage. */
	vqa->MemUsed += (header->Frames * sizeof(long));

	return (vqa);
}


/****************************************************************************
*
* NAME
*     FreeBuffers - Free VQA play buffers.
*
* SYNOPSIS
*     FreeBuffers(VQAData, Config, Header)
*
*     void FreeBuffers(VQAData *, VQAConfig *, VQAHeader *);
*
* FUNCTION
*      Free the buffers allocated by AllocBuffers().
*
* INPUTS
*      VQAData - Pointer to VQAData structure.
*      Config  - Pointer to configuration structure.
*      Header  - Pointer to movie header structure.
*
* RESULT
*      NONE
*
****************************************************************************/

static void FreeBuffers(VQAData *vqa, VQAConfig *config, VQAHeader *header)
{
	VQACBNode    *cb_this,
	             *cb_next;
	VQAFrameNode *frame_this,
	             *frame_next;
	long         i;

	/*-------------------------------------------------------------------------
	 * FREE THE FRAME INFORMATION TABLE.
	 *-----------------------------------------------------------------------*/
	if (vqa->Foff) {
#if (!VQADIRECT_SOUND)
		DPMI_Unlock(vqa->Foff, header->Frames * sizeof(long));
#endif	//(!VQADIRECT_SOUND)
		free(vqa->Foff);
	}

	/*-------------------------------------------------------------------------
	 * FREE THE AUDIO BUFFERS.
	 *-----------------------------------------------------------------------*/

	#if(VQAAUDIO_ON)
	if ((config->AudioBuf == NULL) && (vqa->Audio.Buffer)) {
#if (!VQADIRECT_SOUND)
		DPMI_Unlock(vqa->Audio.Buffer, config->AudioBufSize);
#endif	//(!VQADIRECT_SOUND)
		free(vqa->Audio.Buffer);
	}

	/* Free the audio segments loaded flag array. */
	if (vqa->Audio.IsLoaded) {
#if (!VQADIRECT_SOUND)
		DPMI_Unlock(vqa->Audio.IsLoaded,vqa->Audio.NumAudBlocks * sizeof(short));
#endif	//(!VQADIRECT_SOUND)
		free(vqa->Audio.IsLoaded);
	}

	/* Free the temporary audio buffer. */
	if (vqa->Audio.TempBuf) {
#if (!VQADIRECT_SOUND)
		DPMI_Unlock(vqa->Audio.TempBuf, vqa->Audio.TempBufSize);
#endif	//(!VQADIRECT_SOUND)
		free(vqa->Audio.TempBuf);
	}
	#endif /* VQAAUDIO_ON */

	/*-------------------------------------------------------------------------
	 * FREE THE IMAGE BUFFER ONLY IF WE ALLOCATED IT.
	 *-----------------------------------------------------------------------*/
	if ((config->ImageBuf == NULL) && vqa->Drawer.ImageBuf) {
#if (!VQADIRECT_SOUND)
		DPMI_Unlock(vqa->Drawer.ImageBuf,header->ImageWidth*header->ImageHeight);
#endif	//(!VQADIRECT_SOUND)
		free(vqa->Drawer.ImageBuf);
	}

	/*-------------------------------------------------------------------------
	 * FREE THE FRAME BUFFERS.
	 *-----------------------------------------------------------------------*/
	frame_this = vqa->FrameData;

	for (i = 0; i < config->NumFrameBufs; i++) {
		if (frame_this) {
			frame_next = frame_this->Next;
#if (!VQADIRECT_SOUND)
			DPMI_Unlock(frame_this, sizeof(VQAFrameNode) + vqa->Max_Ptr_Size
					+ vqa->Max_Pal_Size);
#endif	//(!VQADIRECT_SOUND)
			free(frame_this);
			frame_this = frame_next;
		} else {
			break;
		}
	}

	/*-------------------------------------------------------------------------
	 * FREE THE CODEBOOK BUFFERS.
	 *-----------------------------------------------------------------------*/
	cb_this = vqa->CBData;

	for (i = 0; i < config->NumCBBufs; i++) {
		if (cb_this) {
			cb_next = cb_this->Next;
#if (!VQADIRECT_SOUND)
			DPMI_Unlock(cb_this, sizeof(VQACBNode) + vqa->Max_CB_Size);
#endif	//(!VQADIRECT_SOUND)
			free(cb_this);
			cb_this = cb_next;
		} else {
			break;
		}
	}

	/*-------------------------------------------------------------------------
	 * FREE THE VQA DATA STRUCTURES.
	 *-----------------------------------------------------------------------*/
#if (!VQADIRECT_SOUND)
	DPMI_Unlock(vqa, sizeof(VQAData));
#endif	//(!VQADIRECT_SOUND)
	free(vqa);
}


/****************************************************************************
*
* NAME
*     PrimeBuffers - Pre-Load the internal buffers.
*
* SYNOPSIS
*     Error = PrimeBuffers(VQA)
*
*     long = PrimeBuffers(VQAHandle *);
*
* FUNCTION
*     Pre-load the internal buffers in order to give the player some slack
*     in the playback of large frames.
*
* INPUTS
*     VQA - Pointer to VQAHandle structure.
*
* RESULT
*     Error - 0 if successful, or VQAERR_??? error code.
*
****************************************************************************/

long PrimeBuffers(VQAHandle *vqa)
{
	VQAData   *vqabuf;
	VQAConfig *config;
	long      rc;
	long      i;

	/* Dereference commonly used data members for quick access. */
	vqabuf = ((VQAHandleP *)vqa)->VQABuf;
	config = &((VQAHandleP *)vqa)->Config;

	/* Pre-load the buffers */
	for (i = 0; i < config->NumFrameBufs; i++) {
		if ((rc = VQA_LoadFrame(vqa)) == 0) {
			vqabuf->LoadedFrames++;
		}
		else if ((rc != VQAERR_NOBUFFER) && (rc != VQAERR_SLEEPING)) {
			printf("PrimeBuffers %i\n", rc);
			return (rc);
		}
	}

	return (0);
}


/****************************************************************************
*
* NAME
*     Load_VQF - Loads a VQ Frame chunk.
*
* SYNOPSIS
*     Error = Load_VQF(VQA, Iffsize)
*
*     long Load_VQF(VQAHandleP *, unsigned long);
*
* FUNCTION
*     The VQ Frame Chunk contains a set of other chunks (codebooks,
*     palettes, pointers).  This routine reads the frame's chunk size,
*     then loops until it's read that many bytes.
*
* INPUTS
*     VQA     - Pointer to private VQA handle.
*     Iffsize - Size of IFF chunk.
*
* RESULT
*     Error - 0 if successful or VQAERR_??? error code.
*
****************************************************************************/

static long Load_VQF(VQAHandleP *vqap, unsigned long frame_iffsize)
{
	VQAData       *vqabuf;
	VQAFrameNode  *curframe;
	ChunkHeader   *chunk;
	unsigned long iffsize;
	unsigned long framesize;
	unsigned long bytes_loaded = 0;
	VQADrawer     *drawer;

	/* Dereference commonly used data members for quicker access. */
	vqabuf = vqap->VQABuf;
	curframe = vqabuf->Loader.CurFrame;
	framesize = PADSIZE(frame_iffsize);
	drawer = &(vqap->VQABuf->Drawer);
	chunk = &vqabuf->Loader.CurChunkHdr;

	/*-------------------------------------------------------------------------
	 * FRAME LOADING LOOP.
	 *-----------------------------------------------------------------------*/
	while (bytes_loaded < framesize) {

		/* Read chunk ID */
		if (vqap->IOHandler((VQAHandle *)vqap, VQACMD_READ, chunk, 8)) {
			return (VQAERR_EOF);
		}

		iffsize = REVERSE_LONG(chunk->size);
		bytes_loaded += 8;
		bytes_loaded += PADSIZE(iffsize);

		/* Handle each chunk type */
		switch (chunk->id) {

			/* Full uncompressed codebook */
			case ID_CBF0:
				if (Load_CBF0(vqap, iffsize)) {
					return (VQAERR_READ);
				}
				break;

			/* Full compressed codebook */
			case ID_CBFZ:
				if (Load_CBFZ(vqap, iffsize)) {
				 	return (VQAERR_READ);
				}
				break;

			/* Partial uncompressed codebook */
			case ID_CBP0:
				if (Load_CBP0(vqap, iffsize)) {
					return (VQAERR_READ);
				}
				break;

			/* Partial compressed codebook */
			case ID_CBPZ:
				if (Load_CBPZ(vqap, iffsize)) {
					return (VQAERR_READ);
				}
				break;

			/* Uncompressed palette */
			case ID_CPL0:
				if (Load_CPL0(vqap, iffsize)) {
					return (VQAERR_READ);
				}

				/* If this is the first occurance of a palette then store it now.
				 * This functionality is needed for Monopoly!
				 */
				if (drawer->CurPalSize == 0) {
					memcpy(drawer->Palette_24,curframe->Palette,curframe->PaletteSize);
					drawer->CurPalSize = curframe->PaletteSize;
				}

				/* Flag this frame as having a palette. */
				curframe->Flags |= VQAFRMF_PALETTE;
				break;

			/* Compressed palette */
			case ID_CPLZ:
				if (Load_CPLZ(vqap, iffsize)) {
					return (VQAERR_READ);
				}

				/* If this is the first occurance of a palette then store it now.
				 * This functionality is needed for Monopoly!
				 */
				if (drawer->CurPalSize == 0) {
					drawer->CurPalSize = LCW_Uncompress((char *)curframe->Palette
							+ curframe->PalOffset, (char *)drawer->Palette_24,
							vqabuf->Max_Pal_Size);
				}

				/* Flag this frame as having a palette. */
				curframe->Flags |= VQAFRMF_PALETTE;
				break;

			/* Uncompressed pointer data */
			case ID_VPT0:
				if (Load_VPT0(vqap, iffsize)) {
					return (VQAERR_READ);
				}
				break;

			/* Compressed pointer data */
			case ID_VPTZ:
			case ID_VPTD:
				if (Load_VPTZ(vqap, iffsize)) {
					return (VQAERR_READ);
				}
				break;

			/* Compressed pointer data */
			case ID_VPTK:
				if (Load_VPTZ(vqap, iffsize)) {
					return (VQAERR_READ);
				}

				/* Flag this frame as being key. */
				curframe->Flags |= VQAFRMF_KEY;
				break;

			/* An unknown chunk in the video frame is an error. */
			default:
				return (VQAERR_READ);
		}
	}

	return (0);
}


/****************************************************************************
*
* NAME
*     Load_FINF - Load Frame Info chunk.
*
* SYNOPSIS
*     Error = Load_FINF(VQA, Iffsize)
*
*     long Load_FINF(VQAHandleP *, unsigned long);
*
* FUNCTION
*     Load FINF chunk if buffer available, otherwise skip it.
*
* INPUTS
*     VQA     - Pointer to private VQA handle.
*     Iffsize - Size of IFF chunk.
*
* RESULT
*     Error - 0 if successful or VQAERR_??? error code.
*
****************************************************************************/

static long Load_FINF(VQAHandleP *vqap, unsigned long iffsize)
{
	VQAData *vqabuf;

	/* Dereference commonly used data members for quicker access. */
	vqabuf = vqap->VQABuf;

	/* Load the frame information table if we need to, otherwise we will
	 * skip it.
	 */
	if (vqabuf->Foff != NULL) {
		if (vqap->IOHandler((VQAHandle *)vqap, VQACMD_READ, vqabuf->Foff,
				PADSIZE(iffsize))) {

			return (VQAERR_READ);
		}
	} else {
		if (vqap->IOHandler((VQAHandle *)vqap, VQACMD_SEEK, (void *)SEEK_CUR,
				PADSIZE(iffsize))) {
			return (VQAERR_SEEK);
		}
	}

	return (0);
}


/****************************************************************************
*
* NAME
*     Load_VQHD - Load VQA header chunk.
*
* SYNOPSIS
*     Error = Load_VQHD(VQA, Iffsize)
*
*     long Load_VQHD(VQAHandleP *, unsigned long);
*
* FUNCTION
*
* INPUTS
*     VQA     - Pointer to private VQA handle.
*     Iffsize - Size of IFF chunk.
*
* RESULT
*     Error - 0 if successful or VQAERR_??? error code.
*
****************************************************************************/

static long Load_VQHD(VQAHandleP *vqap, unsigned long iffsize)
{
	/* Read the header */
	if (vqap->IOHandler((VQAHandle *)vqap, VQACMD_READ, &vqap->Header,
			PADSIZE(iffsize))) {

		return (VQAERR_READ);
	}

	/* Reconfigure the Drawer for the new settings */
	VQA_Configure_Drawer(vqap);

	return (0);
}


/****************************************************************************
*
* NAME
*     Load_CBF0 - Load full uncompressed codebook.
*
* SYNOPSIS
*     Error = Load_CBF0(VQA, Iffsize)
*
*     long Load_CBF0(VQAHandleP *, unsigned long);
*
* FUNCTION
*
* INPUTS
*     VQA     - Pointer to private VQA handle.
*     Iffsize - Size of IFF chunk.
*
* RESULT
*     Error - 0 if successful or VQAERR_??? error code.
*
****************************************************************************/

static long Load_CBF0(VQAHandleP *vqap, unsigned long iffsize)
{
	VQALoader *loader;
	VQACBNode *curcb;

	/* Dereference commonly used data members for quicker access. */
	loader = &vqap->VQABuf->Loader;
	curcb = loader->CurCB;

	/* Read into the start of the buffer */
	if (vqap->IOHandler((VQAHandle *)vqap, VQACMD_READ, curcb->Buffer,
			PADSIZE(iffsize))) {

		return (VQAERR_READ);
	}

	/* Reset the partial codebook counter. */
	loader->NumPartialCB = 0;

	/* Flag this codebook as uncompressed. */
	curcb->Flags &= (~VQACBF_CBCOMP);
	curcb->CBOffset = 0;

	/* Clock pointers to next CB Buffer. */
	loader->FullCB = curcb;
	loader->FullCB->Flags &= (~VQACBF_DOWNLOADED);
	loader->CurCB = curcb->Next;

	return (0);
}


/****************************************************************************
*
* NAME
*     Load_CBFZ - Load full compressed codebook.
*
* SYNOPSIS
*     Error = Load_CBFZ(VQA, Iffsize)
*
*     long Load_CBFZ(VQAHandleP *, unsigned long);
*
* FUNCTION
*
* INPUTS
*     VQA     - Pointer to private VQA handle.
*     Iffsize - Size of IFF chunk.
*
* RESULT
*     Error - 0 if successful or VQAERR_??? error code.
*
****************************************************************************/

static long Load_CBFZ(VQAHandleP *vqap, unsigned long iffsize)
{
	VQALoader     *loader;
	VQACBNode     *curcb;
	void          *buffer;
	unsigned long padsize;
	unsigned long lcwoffset;

	/* Dereference commonly used data members for quicker access. */
	loader = &vqap->VQABuf->Loader;
	curcb = loader->CurCB;
	padsize = PADSIZE(iffsize);

	/* Load the codebook into the end of the buffer. */
	lcwoffset = vqap->VQABuf->Max_CB_Size - padsize;
	buffer = curcb->Buffer + lcwoffset;

	if (vqap->IOHandler((VQAHandle *)vqap, VQACMD_READ, buffer, padsize)) {
		return (VQAERR_READ);
	}

	/* Reset the partial codebook counter. */
	loader->NumPartialCB = 0;

	/* Flag this codebook as compressed */
	curcb->Flags |= VQACBF_CBCOMP;
	curcb->CBOffset = lcwoffset;

	/* Clock pointers to next CB Buffer */
	loader->FullCB = curcb;
	loader->FullCB->Flags &= (~VQACBF_DOWNLOADED);
	loader->CurCB = curcb->Next;

	return (0);
}


/****************************************************************************
*
* NAME
*     Load_CBP0 - Load partial uncompressed codebook.
*
* SYNOPSIS
*     Error = Load_CBP0(VQA, Iffsize)
*
*     long Load_CBP0(VQAHandleP *, unsigned long);
*
* FUNCTION
*
* INPUTS
*     VQA     - Pointer to private VQA handle.
*     Iffsize - Size of IFF chunk.
*
* RESULT
*     Error - 0 if successful or VQA_??? error code.
*
****************************************************************************/

static long Load_CBP0(VQAHandleP *vqap, unsigned long iffsize)
{
	VQAData   *vqabuf;
	VQALoader *loader;
	VQACBNode *curcb;
	void      *buffer;

	/* Dereference commonly used data members for quicker access. */
	vqabuf = vqap->VQABuf;
	loader = &vqabuf->Loader;
	curcb = loader->CurCB;

	/*-------------------------------------------------------------------------
	 * ASSEMBLY PARTIAL CODEBOOKS.
	 *-----------------------------------------------------------------------*/

	/* Read the partial codebook into the next position in the buffer. */
	buffer = curcb->Buffer + loader->PartialCBSize;

	if (vqap->IOHandler((VQAHandle *)vqap, VQACMD_READ, buffer,
			PADSIZE(iffsize))) {

		return (VQAERR_READ);
	}

	/* Accumulate the partial codebook values. */
	loader->PartialCBSize += iffsize;
	loader->NumPartialCB++;

	/*-------------------------------------------------------------------------
	 * PROCESS FULL CODEBOOK.
	 *-----------------------------------------------------------------------*/
	if (loader->NumPartialCB == vqap->Header.Groupsize) {

		/* Reset the codebook accumulator values */
		loader->NumPartialCB = 0;
		loader->PartialCBSize = 0;

		/* Flag this codebook as uncompressed */
		curcb->Flags &= (~VQACBF_CBCOMP);
		curcb->CBOffset = 0;

		/* Go to the next codebook buffer */
		loader->FullCB = curcb;
		loader->FullCB->Flags &= (~VQACBF_DOWNLOADED);
		loader->CurCB = curcb->Next;

	}

	return (0);
}


/****************************************************************************
*
* NAME
*     Load_CBPZ - Load partial compressed codebook.
*
* SYNOPSIS
*     Error = Load_CBPZ(VQA, Iffsize)
*
*     long Load_CBPZ(VQAHandleP *, unsigned long);
*
* FUNCTION
*
* INPUTS
*     VQA     - Pointer to private VQA handle.
*     Iffsize - Size of IFF chunk.
*
* RESULT
*     Error - 0 if successful or VQAERR_??? error code.
*
****************************************************************************/

static long Load_CBPZ(VQAHandleP *vqap, unsigned long iffsize)
{
	VQAData       *vqabuf;
	VQALoader     *loader;
	VQACBNode     *curcb;
	void          *buffer;
	unsigned long padsize;

	/* Dereference commonly used data members for quicker access */
	vqabuf = vqap->VQABuf;
	loader = &vqabuf->Loader;
	curcb = loader->CurCB;
	padsize = PADSIZE(iffsize);

	/* Attempt to compute the LCW offset into the codebook buffer by
	 * multiplying the size of this chunk by the # frames/group, and adding
	 * a small fudge factor on, then subtracting that from the CB buffer size.
	 */
	if (loader->PartialCBSize == 0) {
		curcb->CBOffset = (vqabuf->Max_CB_Size
				- (padsize * vqap->Header.Groupsize + 100));
	}

	/*-------------------------------------------------------------------------
	 * ASSEMBLE PARTIAL CODEBOOKS.
	 *-----------------------------------------------------------------------*/

	/* Read the partial codebook into the next position in the buffer. */
	buffer = ((curcb->Buffer + curcb->CBOffset) + loader->PartialCBSize);

	if (vqap->IOHandler((VQAHandle *)vqap, VQACMD_READ, buffer, padsize)) {
		return (VQAERR_READ);
	}

	/* Accumulate partial codebook values */
	loader->PartialCBSize += iffsize;
	loader->NumPartialCB++;

	/*-------------------------------------------------------------------------
	 * PROCESS FULL CODEBOOK.
	 *-----------------------------------------------------------------------*/
	if (loader->NumPartialCB == vqap->Header.Groupsize) {

		/* Reset the codebook accumulator values. */
		loader->NumPartialCB = 0;
		loader->PartialCBSize = 0;

		/* Flag this codebook as compressed. */
		curcb->Flags |= VQACBF_CBCOMP;

		/* Go to the next codebook buffer */
		loader->FullCB = curcb;
		loader->FullCB->Flags &= (~VQACBF_DOWNLOADED);
		loader->CurCB = curcb->Next;
	}

	return (0);
}


/****************************************************************************
*
* NAME
*     Load_CPL0 - Load an uncompressed palette.
*
* SYNOPSIS
*     Error = Load_CPL0(VQA, Iffsize)
*
*     long Load_CPL0(VQAHandleP *, unsigned long);
*
* FUNCTION
*
* INPUTS
*     VQA     - Pointer to private VQA handle.
*     Iffsize - Size of IFF chunk.
*
* RESULT
*     Error - 0 if successful or VQAERR_??? error code.
*
****************************************************************************/

static long Load_CPL0(VQAHandleP *vqap, unsigned long iffsize)
{
	VQAFrameNode *curframe;

	/* Dereference commonly used data members for quicker access. */
	curframe = vqap->VQABuf->Loader.CurFrame;

	/* Read the palette into the palette buffer */
	if (vqap->IOHandler((VQAHandle *)vqap, VQACMD_READ, curframe->Palette,
			PADSIZE(iffsize))) {

		return (VQAERR_READ);
	}

	/* Flag the palette as uncompressed. */
	curframe->Flags &= ~VQAFRMF_PALCOMP;
	curframe->PalOffset = 0;
	curframe->PaletteSize = iffsize;

	return (0);
}


/****************************************************************************
*
* NAME
*     Load_CPLZ - Load compressed palette.
*
* SYNOPSIS
*     Error = Load_CPLZ(VQA, Iffsize)
*
*     long Load_CPLZ(VQAHandleP *, unsigned long);
*
* FUNCTION
*
* INPUTS
*     VQA     - Pointer to private VQA handle.
*     Iffsize - Size of IFF chunk.
*
* RESULT
*     Error - 0 if successful or VQAERR_??? error code.
*
****************************************************************************/

static long Load_CPLZ(VQAHandleP *vqap, unsigned long iffsize)
{
	VQAFrameNode  *curframe;
	void          *buffer;
	unsigned long padsize;
	unsigned long lcwoffset;

	/* Dereference commonly used data members for quicker access. */
	curframe = vqap->VQABuf->Loader.CurFrame;
	padsize = PADSIZE(iffsize);

 	/* Read the palette into the end of the palette buffer. */
	lcwoffset = vqap->VQABuf->Max_Pal_Size - padsize;
	buffer = curframe->Palette + lcwoffset;

	if (vqap->IOHandler((VQAHandle *)vqap, VQACMD_READ, buffer, padsize)) {
		return (VQAERR_READ);
	}

	/* Flag this palette as compressed. */
	curframe->Flags |= VQAFRMF_PALCOMP;
	curframe->PalOffset = lcwoffset;
	curframe->PaletteSize = iffsize;

	return (0);
}


/****************************************************************************
*
* NAME
*     Load_VPT0 - Load uncompressed pointers.
*
* SYNOPSIS
*     Error = Load_VPT0(VQA, Iffsize)
*
*     long Load_VPT0(VQAHandleP *, unsigned long);
*
* FUNCTION
*
* INPUTS
*     VQA     - Pointer to private VQA handle.
*     Iffsize - Size of IFF chunk.
*
* RESULT
*     Error - 0 if successful or VQAERR_??? error code.
*
****************************************************************************/

static long Load_VPT0(VQAHandleP *vqap, unsigned long iffsize)
{
	VQAFrameNode *curframe;

	/* Dereference commonly used data members for quicker access. */
	curframe = vqap->VQABuf->Loader.CurFrame;

	/* Read the pointers into start of the pointer buffer. */
	if (vqap->IOHandler((VQAHandle *)vqap, VQACMD_READ, curframe->Pointers,
			PADSIZE(iffsize))) {

		return (VQAERR_READ);
	}

	/* Flag this frame as uncompressed */
	curframe->Flags &= ~VQAFRMF_PTRCOMP;
	curframe->PtrOffset = 0;

	return (0);
}


/****************************************************************************
*
* NAME
*     Load_VPTZ - Load compressed pointers.
*
* SYNOPSIS
*     Error = Load_VPTZ(VQA, Iffsize)
*
*     long Load_VPTZ(VQAHandleP *, unsigned long);
*
* FUNCTION
*
* INPUTS
*     VQA     - Pointer to private VQA handle.
*     Iffsize - Size of IFF chunk.
*
* RESULT
*     Error - 0 if successful or VQAERR_??? error code.
*
****************************************************************************/

static long Load_VPTZ(VQAHandleP *vqap, unsigned long iffsize)
{
	VQAFrameNode  *curframe;
	void          *buffer;
	unsigned long padsize;
	unsigned long lcwoffset;

	/* Dereference commonly used data members for quicker access. */
	curframe = vqap->VQABuf->Loader.CurFrame;
	padsize = PADSIZE(iffsize);
	lcwoffset = vqap->VQABuf->Max_Ptr_Size - padsize;

	/* Read the pointers into end of the pointer buffer. */
	buffer = curframe->Pointers + lcwoffset;

	if (vqap->IOHandler((VQAHandle *)vqap, VQACMD_READ, buffer, padsize)) {
		return (VQAERR_READ);
	}

	/* Flag this frame as compressed. */
	curframe->Flags |= VQAFRMF_PTRCOMP;
	curframe->PtrOffset = lcwoffset;

	return (0);
}


#if(VQAAUDIO_ON)
/****************************************************************************
*
* NAME
*     Load_SND0 - Load uncompressed sound chunk.
*
* SYNOPSIS
*     Error = Load_SND0(VQA, Iffsize)
*
*     long Load_SND0(VQAHandleP *, unsigned long);
*
* FUNCTION
*     This routine normally loads the chunk into the TempBuf, unless the
*     chunk is larger than the temp buffer size, in which case it puts it
*     directly into the audio buffer itself.  This assumes that the only
*     such chunk will be the first audio chunk!
*
* INPUTS
*     VQA     - Pointer to private VQA handle.
*     Iffsize - Size of IFF chunk.
*
* RESULT
*     Error - 0 if successful or VQAERR_??? error code.
*
****************************************************************************/

static long Load_SND0(VQAHandleP *vqap, unsigned long iffsize)
{
	VQAData       *vqabuf;
	VQALoader     *loader;
	VQAAudio      *audio;
	VQAConfig     *config;
	unsigned long padsize;
	long          i;

	/* Dereference commonly used data members for quicker access. */
	vqabuf = vqap->VQABuf;
	loader = &vqabuf->Loader;
	audio = &vqabuf->Audio;
	config = &vqap->Config;
	padsize = PADSIZE(iffsize);

	/* If sound is disabled, or if we're playing from a VOC file, or if
	 * there's no Audio Buffer, just skip the chunk.
	 */
	#if(VQAVOC_ON && VQAAUDIO_ON)
	if (((config->OptionFlags & VQAOPTF_AUDIO) == 0)
			|| (vqap->vocfh != -1) || (audio->Buffer == NULL)) {
	#else  /* VQAVOC_ON */
	if (((config->OptionFlags & VQAOPTF_AUDIO) == 0)
			|| (audio->Buffer == NULL)) {
	#endif /* VQAVOC_ON */

		if (vqap->IOHandler((VQAHandle *)vqap, VQACMD_SEEK, (void *)SEEK_CUR,
				padsize)) {
			return (VQAERR_SEEK);
		} else {
			return (0);
		}
	}

	/* Read large startup chunk directly into AudioBuf */
	if ((padsize > audio->TempBufSize) && (audio->AudBufPos == 0)) {
		if (vqap->IOHandler((VQAHandle *)vqap, VQACMD_READ, audio->Buffer,
				padsize)) {

			return (VQAERR_READ);
		}

		audio->AudBufPos += iffsize;

		/* Flag the audio frame flags as loaded for the initial audio frame. */
		for (i = 0; i < (iffsize / config->HMIBufSize); i++) {
			audio->IsLoaded[i] = 1;
		}

		return (0);
	} else {

		/*  Read data into TempBuf */
		if (vqap->IOHandler((VQAHandle *)vqap, VQACMD_READ, audio->TempBuf,
				padsize)) {

			return (VQAERR_READ);
		}
	}

	/* Set the TempBufLen */
	audio->TempBufLen = iffsize;

	return (0);
}


/****************************************************************************
*
* NAME
*     Load_SND1 - Load compressed sound chunk.
*
* SYNOPSIS
*     Error = Load_SND1(VQA, Iffsize)
*
*     long Load_SND1(VQAHandleP *, unsigned long);
*
* FUNCTION
*     This routine normally loads the chunk into the TempBuf, unless the
*     chunk is larger than the temp buffer size, in which case it puts it
*     directly into the audio buffer itself.  This assumes that the only
*     such chunk will be the first audio chunk!
*
* INPUTS
*     VQA     - Pointer to private VQA handle.
*     Iffsize - Size of IFF chunk.
*
* RESULT
*     Error - 0 if successful or VQAERR_??? error code.
*
****************************************************************************/

static long Load_SND1(VQAHandleP *vqap, unsigned long iffsize)
{
	VQAData       *vqabuf;
	VQALoader     *loader;
	VQAAudio      *audio;
	VQAConfig     *config;
	unsigned char *loadbuf;
	unsigned long padsize;
	ZAPHeader     zap;
	long          i;

	/* Dereference commonly used data members for quicker access. */
	vqabuf = vqap->VQABuf;
	loader = &vqabuf->Loader;
	audio = &vqabuf->Audio;
	config = &vqap->Config;
	padsize = PADSIZE(iffsize);

	/* If sound is disabled, or if we're playing from a VOC file, or if
	 * there's no Audio Buffer, just skip the chunk
	 */
	#if(VQAVOC_ON && VQAAUDIO_ON)
	if (((config->OptionFlags & VQAOPTF_AUDIO) == 0) || (vqap->vocfh != -1)
			|| (audio->Buffer == NULL)) {
	#else
	if (((config->OptionFlags & VQAOPTF_AUDIO)==0) || (audio->Buffer==NULL)) {
	#endif /* VQAVOC_ON */

		if (vqap->IOHandler((VQAHandle *)vqap, VQACMD_SEEK, (void *)SEEK_CUR,
				padsize)) {
			return (VQAERR_SEEK);
		} else {
			return (0);
		}
	}

	/* Read the ZAP audio frame header. */
	if (vqap->IOHandler((VQAHandle *)vqap, VQACMD_READ, &zap,
			sizeof(ZAPHeader))) {

		return (VQAERR_READ);
	}

	/* Adjust chunk size */
	padsize -= sizeof(ZAPHeader);

	/* Read large startup chunk directly into AudioBuf */
	if ((zap.UnCompSize > audio->TempBufSize) && (audio->AudBufPos == 0)) {

		/* Load RAW uncompressed data. */
		if (zap.UnCompSize == zap.CompSize) {
			if (vqap->IOHandler((VQAHandle *)vqap, VQACMD_READ, audio->Buffer,
					padsize)) {

				return (VQAERR_READ);
			}
		} else {

			/* Load compressed data into the end of the buffer. */
			loadbuf = (audio->Buffer + config->AudioBufSize) - padsize;

			if (vqap->IOHandler((VQAHandle *)vqap, VQACMD_READ, loadbuf,
					padsize)) {

				return (VQAERR_READ);
			}

			/* Uncompress the audio frame. */
			AudioUnzap(loadbuf, audio->Buffer, zap.UnCompSize);
		}

		/* Set buffer positions & flags */
		audio->AudBufPos += zap.UnCompSize;

		for (i = 0; i < (zap.UnCompSize / config->HMIBufSize); i++) {
			audio->IsLoaded[i] = 1;
		}

		return (0);
	}

	/* Load an audio frame. */
	if (zap.UnCompSize == zap.CompSize) {

		/* If the frame is uncompressed the load it in directly. */
		if (vqap->IOHandler((VQAHandle *)vqap, VQACMD_READ, audio->TempBuf,
				padsize)) {

			return (VQAERR_READ);
		}
	} else {

		/* Load the audio frame into the end of the buffer. */
		loadbuf = ((audio->TempBuf + audio->TempBufSize) - padsize);

		if (vqap->IOHandler((VQAHandle *)vqap, VQACMD_READ, loadbuf, padsize)) {
			return (VQAERR_READ);
		}

		/* Uncompress the audio frame. */
		AudioUnzap(loadbuf, audio->TempBuf, zap.UnCompSize);
	}

	/* Set the TempBufLen */
	audio->TempBufLen = zap.UnCompSize;

	return (0);
}


/****************************************************************************
*
* NAME
*     Load_SND2 - Load ADPCM compressed sound chunk.
*
* SYNOPSIS
*     Error = Load_SND2(VQA, Iffsize)
*
*     long Load_SND2(VQAHandleP *, unsigned long);
*
* FUNCTION
*     This routine normally loads the chunk into the TempBuf, unless the
*     chunk is larger than the temp buffer size, in which case it puts it
*     directly into the audio buffer itself.  This assumes that the only
*     such chunk will be the first audio chunk!
*
* INPUTS
*     VQA     - Pointer to private VQA handle.
*     Iffsize - Size of IFF chunk.
*
* RESULT
*     Error - 0 if successful or VQAERR_??? error code.
*
****************************************************************************/

static long Load_SND2(VQAHandleP *vqap, unsigned long iffsize)
{
	VQAData       *vqabuf;
	VQALoader     *loader;
	VQAAudio      *audio;
	VQAConfig     *config;
	unsigned char *loadbuf;
	unsigned long padsize;
	unsigned long uncomp_size;
	long          i;

	/* Dereference commonly used data members for quicker access. */
	vqabuf = vqap->VQABuf;
	loader = &vqabuf->Loader;
	audio = &vqabuf->Audio;
	config = &vqap->Config;
	padsize = PADSIZE(iffsize);

	/* If sound is disabled, or if we're playing from a VOC file, or if
	 * there's no Audio Buffer, just skip the chunk
	 */
	#if(VQAVOC_ON && VQAAUDIO_ON)
	if (((config->OptionFlags & VQAOPTF_AUDIO) == 0) || (vqap->vocfh != -1)
			|| (audio->Buffer == NULL)) {
	#else  /* VQAVOC_ON */
	if (((config->OptionFlags & VQAOPTF_AUDIO)==0) || (audio->Buffer==NULL)) {
	#endif /* VQAVOC_ON */

		if (vqap->IOHandler((VQAHandle *)vqap, VQACMD_SEEK, (void *)SEEK_CUR,
				padsize)) {
			return (VQAERR_SEEK);
		} else {
			return (0);
		}
	}

	uncomp_size = iffsize * (audio->BitsPerSample / 4);

	/* Read large startup chunk directly into AudioBuf */
	if ((uncomp_size > audio->TempBufSize) && (audio->AudBufPos == 0)) {

		/* Load compressed data into the end of the buffer. */
		loadbuf = (audio->Buffer + config->AudioBufSize) - padsize;

		if (vqap->IOHandler((VQAHandle *)vqap, VQACMD_READ, loadbuf, padsize)) {
			return (VQAERR_READ);
		}

		/* Uncompress the audio frame. */
		audio->ADPCM_Info.lpSource = (char *)loadbuf;
		audio->ADPCM_Info.lpDest = (char *)audio->Buffer;
		VQA_sosCODECDecompressData(&audio->ADPCM_Info, uncomp_size);

		/* Set buffer positions & flags */
		audio->AudBufPos += uncomp_size;

		for (i = 0; i < (uncomp_size / config->HMIBufSize); i++) {
			audio->IsLoaded[i] = 1;
		}

		return (0);
	}

	/* Load an audio frame. */
	loadbuf = ((audio->TempBuf + audio->TempBufSize) - padsize);

	if (vqap->IOHandler((VQAHandle *)vqap, VQACMD_READ, loadbuf, padsize)) {
		return (VQAERR_READ);
	}

	/* Uncompress the audio frame. */
	audio->ADPCM_Info.lpSource = (char *)loadbuf;
	audio->ADPCM_Info.lpDest = (char *)audio->TempBuf;
	VQA_sosCODECDecompressData(&audio->ADPCM_Info, uncomp_size);

	/* Set the TempBufLen */
	audio->TempBufLen = uncomp_size;

	return (0);
}


#if(VQAVOC_ON)
/****************************************************************************
*
* NAME
*     Load_AudFrame - Loads blocks from seperate VOC file.
*
* SYNOPSIS
*     Load_AudFrame(VQA)
*
*     void Load_AudFrame(VQAHandleP *);
*
* FUNCTION
*
* INPUTS
*     VQA - Pointer to private VQA handle.
*
* RESULT
*     NONE
*
****************************************************************************/

static void Load_AudFrame(VQAHandleP *vqap)
{
	VQAData     *vqabuf;
	VQALoader   *loader;
	VQAAudio    *audio;
	VQAConfig   *config;
	static long lastplayblock = -1;
	static long myblock = 0;
	static long firsttime = 1;
	long        numblocks;
	long        i;

	/* Dereference commonly used data members for quicker access. */
	vqabuf = vqap->VQABuf;
	loader = &vqabuf->Loader;
	audio = &vqabuf->Audio;
	config = &vqap->Config;

	/* Do nothing if no buffer */
	if (audio->Buffer == NULL) {
		return;
	}

	/* If this is the first time we're called, pre-load the 1st 'n' audio
	 * blocks, where 'n' is half the total audio buffer size; this way, we'll
	 * always stay ahead of HMI.
	 */
	if (firsttime) {
		numblocks = (audio->NumAudBlocks / 2);
		read(vqap->vocfh, audio->Buffer, config->HMIBufSize * numblocks);
		audio->AudBufPos += config->HMIBufSize * numblocks;

		if (audio->AudBufPos >= config->AudioBufSize) {
			audio->AudBufPos = 0;
		}

		for (i = 0; i < numblocks; i++) {
			audio->IsLoaded[i] = 1;
		}

		myblock += numblocks;

		if (myblock >= audio->NumAudBlocks) {
			myblock = 0;
		}

		firsttime = 0;
	}

	/* If HMI's block has changed, load the next block & mark it as loaded */
	if (audio->PlayPosition / config->HMIBufSize != lastplayblock) {

		/* update HMI's last known block position */
		lastplayblock = audio->PlayPosition / config->HMIBufSize;

		/* read the VOC data */
		read(vqap->vocfh, (audio->Buffer + myblock * config->HMIBufSize),
				config->HMIBufSize);

		audio->AudBufPos += config->HMIBufSize;

		if (audio->AudBufPos >= config->AudioBufSize) {
			audio->AudBufPos = 0;
		}

		/* set the IsLoaded flags */
		audio->IsLoaded[myblock] = 1;

		/* increment my block counter */
		myblock++;

		if (myblock >= audio->NumAudBlocks) {
			myblock = 0;
		}
	}
}
#endif /* VQAVOC_ON */
#endif /* VQAAUDIO_ON */

