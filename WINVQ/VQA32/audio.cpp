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
*     audio.c
*
* DESCRIPTION
*     Audio playback and timing.
*
* PROGRAMMER
*     Bill Randolph
*     Denzil E. Long, Jr.
*
* DATE
*     August 4, 1995
*
*
* HISTORY:
*     Modified for Win95 Direct Sound - Steve T 1/2/96 6:35AM
*
*----------------------------------------------------------------------------
*
* PUBLIC
*     VQA_StartTimerInt - Initialize system timer interrupt.
*     VQA_StopTimerInt  - Remove system timer interrupt.
*     VQA_SetTimer      - Resets current time to given tick value.
*     VQA_GetTime       - Return current time.
*     VQA_TimerMethod   - Get timer method being used.
*     VQA_OpenAudio     - Open sound system.
*     VQA_CloseAudio    - Close sound system
*     VQA_StartAudio    - Starts audio playback
*     VQA_StopAudio     - Stop audio playback.
*     CopyAudio         - Copy data from Audio Temp buf into Audio play buf.
*
* PRIVATE
*     TimerCallback - VQA timer event. (Called by HMI)
*     AutoDetect    - Auto detect the sound card.
*     AudioCallback - Sound system callback.
*
****************************************************************************/

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <sys/timeb.h>
#include "vqaplayp.h"
#include <vqm32/all.h>

#ifdef __WATCOMC__
#pragma pack(4);
#endif


/*---------------------------------------------------------------------------
 * AUDIO DEFINITIONS
 *-------------------------------------------------------------------------*/

#define USE_WINDOWS_TIME	1

#define HMI_SAMPLE 0x1000
#define MAKE_LONG(a,b) (((long)(a)<<16)|(long)((b)&0x0000FFFFL))

/*---------------------------------------------------------------------------
 * PROTOTYPES
 *-------------------------------------------------------------------------*/

#if( USE_WINDOWS_TIME )
	extern unsigned long Get_Game_Time( void );
#endif

#if(VQAAUDIO_ON)
#if VQASDL_SOUND
#undef WIN32
#include "SDL.h"
#elif VQAPICO_SOUND
#ifdef PICO_BUILD
#include "pico/stdlib.h" // "DOS" timer fallback
#elif defined(ESP_BUILD)
#include "esp_timer.h"
#endif
#elif(VQADIRECT_SOUND)
void CALLBACK TimerCallback ( UINT event_id, UINT res1 , DWORD user, DWORD  res2, DWORD  res3 );
BOOL Move_HMI_Audio_Block_To_Direct_Sound_Buffer (void);
void CALLBACK AudioCallback ( UINT , UINT , DWORD , DWORD , DWORD );

#else

long AutoDetect(_SOS_CAPABILITIES *digicaps, long bitsize, long channels);
void far TimerCallback(void);
void far __cdecl AudioCallback(WORD wDriverHandle, WORD wAction,
		WORD wSampleID);

/* Dummy functions used to mark the start/end address of the file. */
static void StartAddr(void);
static void EndAddr(void);

#endif
/*---------------------------------------------------------------------------
 * GLOBAL DATA
 *-------------------------------------------------------------------------*/

static VQAHandleP *VQAP = NULL;
static long AudioFlags = 0;
static long TimerSysCount = 0;
static long TimerIntCount = 0;
static uint16_t VQATimer = 0;
static long TimerMethod;
static long VQATickCount = 0;
#endif /* VQAAUDIO_ON */

static long TickOffset = 0;
char *HMIDevName = "<none>";

#if(VQAAUDIO_ON)

#if VQASDL_SOUND

extern int VQAMovieDone;
static bool VQAAudioPaused = false;
static SDL_AudioStream *SDLStream = NULL;
static unsigned StreamConvScale = 1 << 15;

static void VQA_Audio_Callback(uint8_t *stream, int len)
{
	// called from SDL audio callback
	if(!VQAP)
		return;
	auto audio = &VQAP->VQABuf->Audio;
	if(!(audio->Flags & VQAAUDF_ISPLAYING) || VQAAudioPaused || !SDLStream)
		return;

	auto config = &VQAP->Config;

	while(SDL_AudioStreamAvailable(SDLStream) < len)
	{
		SDL_AudioStreamPut(SDLStream, audio->Buffer + audio->PlayPosition, config->HMIBufSize);

		/* Compute the 'NextBlock' index */
		audio->NextBlock = audio->CurBlock + 1;

		if (audio->NextBlock >= audio->NumAudBlocks) {
			audio->NextBlock = 0;
		}

		/* See if the next block has data in it; if so, update the audio
			* buffer play position & the 'CurBlock' value.
			* If not, don't change anything and replay this block.
			*/
		if (audio->IsLoaded[audio->NextBlock] == 1) {

			/* Update this block's status to loadable (0) */
			audio->IsLoaded[audio->CurBlock] = 0;

			/* Update position within audio buffer */
			audio->PlayPosition += config->HMIBufSize;
			audio->CurBlock++;

			if (audio->PlayPosition >= config->AudioBufSize) {
				audio->PlayPosition = 0;
				audio->CurBlock = 0;
			}
			audio->ChunksMovedToAudioBuffer++;
		} else {
			if (VQAMovieDone){
				audio->ChunksMovedToAudioBuffer++;
			}
			audio->NumSkipped++;
			/*
			** Enable frame skipping to prevent this happening again
			*/
			config->DrawFlags &= ~VQACFGF_NOSKIP;
		}
	}

	// output stream
	SDL_AudioStreamGet(SDLStream, stream, len);
}

/****************************************************************************
*
* NAME
*     VQA_StartTimerInt - Initialize system timer interrupt.
*
* SYNOPSIS
*     Error = VQA_StartTimerInt(VQA, Init)
*
*     long VQA_StartTimerInt(VQAHandeP *, long);
*
* FUNCTION
*     Initialize the HMI timer system and add our own timer event. If the
*     system has already been initialized then we are given access to the
*     the timer system.
*
* INPUTS
*     VQA  - Pointer to private VQAHandle structure.
*     Init - Initialize HMI timer system flag. (TRUE = Initialize)
*
* RESULT
*     Error - 0 if successful, -1 if error.
*
****************************************************************************/

long VQA_StartTimerInt(VQAHandleP *vqap, long init)
{
	VQAAudio *audio;

	/* Dereference for quick access. */
	audio = &vqap->VQABuf->Audio;

	/* Register the VQA_TickCount timer event. */
	if ((AudioFlags & VQAAUDF_HMITIMER) == (HMI_UNINIT<<VQAAUDB_HMITIMER)) {
		// TODO: add timer (VQATimer)

		if (VQATimer){

			/* Flag the timer interrupt as being registered. */
			AudioFlags |= (HMI_VQAINIT << VQAAUDB_HMITIMER);
		} else {
			return (-1);
		}
	}

	/* Flag availability of the timer interrupt. */
	audio->Flags |= (HMI_VQAINIT << VQAAUDB_HMITIMER);

	/* Increment the timer interrupt usage count. */
	TimerIntCount++;

	return (0);
}


/****************************************************************************
*
* NAME
*     VQA_StopTimerInt - Remove system timer interrupt.
*
* SYNOPSIS
*     VQA_StopTimerInt()
*
*     void VQA_StopTimerInt(void);
*
* FUNCTION
*     Remove our timer event from the HMI timer system. Uninitialize the
*     HMI timer system if we initialized it.
*
* INPUTS
*     NONE
*
* RESULT
*     NONE
*
****************************************************************************/

void VQA_StopTimerInt(VQAHandleP *vqap)
{
	VQAAudio *audio;

	/* Dereference for quick access. */
	audio = &vqap->VQABuf->Audio;

	/* Decrement the timer interrupt usage count. */
	if (TimerIntCount) {
		TimerIntCount--;
	}

	/* Remove the timer interrrupt if it is initialized and the use count is
	 * zero. Otherwise, clear the callers timer interrupt availability flag.
	 */
	if (((AudioFlags & VQAAUDF_HMITIMER) == (HMI_VQAINIT<<VQAAUDB_HMITIMER))
			&& (TimerIntCount == 0)) {

		// TODO: remove timer
		AudioFlags &= ~VQAAUDF_HMITIMER;
	} else {
		AudioFlags &= ~VQAAUDF_HMITIMER;
	}

}

/****************************************************************************
*
* NAME
*     VQA_OpenAudio - Open sound system.
*
* SYNOPSIS
*     Error = VQA_OpenAudio(VQAHandleP)
*
*     long VQA_OpenAudio(VQAHandleP *);
*
* FUNCTION
*     Initialise the sound system. Create a direct sound object and the
*     direct sound primary sound buffer if they dont already exist.
*
* INPUTS
*     VQAHandleP - Pointer to private VQAHandle.
*
* RESULT
*     Error - 0 if successful, -1 if error.
*
****************************************************************************/

static int OpenCount = 0;

long VQA_OpenAudio(VQAHandleP *vqap, void *window)
{
	VQAData       *vqabuf;
	VQAAudio      *audio;
	VQAConfig     *config;
	VQAHeader     *header;

	/* Dereference data memebers for quicker access. */
	config = &vqap->Config;
	header = &vqap->Header;
	vqabuf = vqap->VQABuf;
	audio = &vqabuf->Audio;

	/* Reset the buffer position to the beginning. */
	audio->CurBlock = 0;

	if(OpenCount)
	{
		// if we've already initialised make sure we're not in the callback
		// (by unsetting it)
		SDL_LockAudioDevice(config->AudioDeviceID);
		*config->AudioCallback = NULL;
		SDL_UnlockAudioDevice(config->AudioDeviceID);
	}

	// setup audio stream
	if(SDLStream)
		SDL_FreeAudioStream(SDLStream);

	auto spec = (SDL_AudioSpec *)config->AudioSpec;

	SDLStream = SDL_NewAudioStream(
		audio->BitsPerSample == 16 ? AUDIO_S16 : AUDIO_S8, audio->Channels, audio->SampleRate,
		spec->format, spec->channels, spec->freq
	);

	// calculate scaling factor
	unsigned bytes_per_second_in = (audio->BitsPerSample / 8) * audio->Channels * audio->SampleRate;
	unsigned bytes_per_second_out = (SDL_AUDIO_BITSIZE(spec->format) / 8) * spec->channels * spec->freq;

	StreamConvScale = (bytes_per_second_in << 15) / bytes_per_second_out;

	// register our audio callback
	*config->AudioCallback = VQA_Audio_Callback;

	audio->Flags |= (HMI_VQAINIT << VQAAUDB_DIGIINIT);
	AudioFlags |= (HMI_VQAINIT << VQAAUDB_DIGIINIT);

	OpenCount++;

	return (0);
}


/****************************************************************************
*
* NAME
*     VQA_CloseAudio - Close sound system
*
* SYNOPSIS
*     VQA_CloseAudio()
*
*     void VQA_CloseAudio(void);
*
* FUNCTION
*     Removes VQA's involvement in the audio system.
*
* INPUTS
*     NONE
*
* RESULT
*     NONE
*
****************************************************************************/

void VQA_CloseAudio(VQAHandleP *vqap)
{
	VQAAudio		*audio;
	VQAConfig	*config;

	/* Dereference for quick access. */
	audio	 = &vqap->VQABuf->Audio;
	config = &vqap->Config;

	/*
	** If the audio is still playing then stop it
	*/
	VQA_StopAudio(vqap);

	audio->Flags &= ~VQAAUDF_TIMERINIT;
	AudioFlags &= ~VQAAUDF_TIMERINIT;

	// don't remove the callback if open was called multiple times
	OpenCount--;
	if(OpenCount)
		return;

	// unregister our audio callback
	// and make sure we're not in it
	SDL_LockAudioDevice(config->AudioDeviceID);
	*config->AudioCallback = NULL;
	SDL_UnlockAudioDevice(config->AudioDeviceID);

	if(SDLStream)
	{
		SDL_FreeAudioStream(SDLStream);
		SDLStream = NULL;
	}

	audio->Flags &= ~VQAAUDF_DIGIINIT;
	AudioFlags &= ~VQAAUDF_DIGIINIT;
	AudioFlags &= ~VQAAUDF_ISPLAYING;

}


/****************************************************************************
*
* NAME
*     VQA_StartAudio - Starts audio playback
*
* SYNOPSIS
*     Error = VQA_StartAudio(VQA)
*
*     long VQA_StartAudio(VQAHandleP *);
*
* FUNCTION
*     Start the audio playback for the movie.
*
* INPUTS
*     VQA - Pointer to private VQA handle.
*
* RESULT
*     Error - 0 if successful, or -1 error code.
*
****************************************************************************/

long VQA_StartAudio(VQAHandleP *vqap)
{
	VQAConfig *config;
	VQAAudio  *audio;

	/* Save buffers for the callback routine */
	VQAP = vqap;

	/* Dereference commonly used data members for quicker access. */
	config = &vqap->Config;
	audio = &vqap->VQABuf->Audio;

	/* Return if already playing */
	if (AudioFlags & VQAAUDF_ISPLAYING) {
		return (-1);
	}

	SDL_LockAudioDevice(config->AudioDeviceID);
	// setup playback
	audio->ChunksMovedToAudioBuffer = 0;

	/*
	** Set the volume
	*/
	long volume = config->Volume << 7;
	//audio->SecondaryBufferPtr->SetVolume(- ( ( (32768 - volume)*1000) >>15 ) );

	audio->Flags |= VQAAUDF_ISPLAYING;
	AudioFlags |= VQAAUDF_ISPLAYING;

	SDL_UnlockAudioDevice(config->AudioDeviceID);

	return (0);
}


/****************************************************************************
*
* NAME
*     VQA_StopAudio - Stop audio playback.
*
* SYNOPSIS
*     VQA_StopAudio(VQA)
*
*     void VQA_StopAudio(VQAHandleP *);
*
* FUNCTION
*     Halts the currently playing audio stream.
*
* INPUTS
*     VQA - Pointer to private VQAHandle.
*
* RESULT
*     NONE
*
****************************************************************************/

void VQA_StopAudio(VQAHandleP *vqap)
{
	VQAAudio *audio;
	VQAConfig *config;

	/* Dereference commonly used data members for quicker access. */
	config = &vqap->Config;
	audio = &vqap->VQABuf->Audio;

	/* Just return if not playing */
	if (AudioFlags & VQAAUDF_ISPLAYING) {


		//audio->TimerHandle = NULL;

		// TODO: stop buffer

		audio->Flags &= ~VQAAUDF_ISPLAYING;
		AudioFlags &= ~VQAAUDF_ISPLAYING;
	}

	VQAP = NULL;
}


/****************************************************************************
*
* NAME
*     CopyAudio - Copy data from Audio Temp buffer into Audio play buffer.
*
* SYNOPSIS
*     Error = CopyAudio(VQA)
*
*     long CopyAudio(VQAHandleP *);
*
* FUNCTION
*     This routine just copies the data in the TempBuf into the correct
*     spots in the audio play buffer.  If there is no room available in the
*     audio play buffer, the routine returns VQAERR_SLEEPING, which will put
*     the whole Loader to "sleep" while it waits for a free buffer.
*
*     If there's no data in the TempBuf to copy, the routine just returns 0.
*
* INPUTS
*     VQA - Pointer to private VQAHandle structure.
*
* RESULT
*     Error - 0 if successful or VQAERR_??? error code.
*
****************************************************************************/

long CopyAudio(VQAHandleP *vqap)
{
	VQAAudio  *audio;
	VQAConfig *config;
	long      startblock;
	long      endblock;
	long      len1,len2;
	long      i;

	/* Dereference commonly used data members for quicker access. */
	audio = &vqap->VQABuf->Audio;
	config = &vqap->Config;

	/* If audio is disabled, or if we're playing from a VOC file, or if
	 * there's no Audio Buffer, or if there's no data to copy, just return 0
	 */
	#if(VQAVOC_ON && VQAAUDIO_ON)
	if (((config->OptionFlags & VQAOPTF_AUDIO) == 0) || (vqap->vocfh != -1)
			|| (audio->Buffer == NULL) || (audio->TempBufLen == 0)) {
	#else  /* VQAVOC_ON */
	if (((config->OptionFlags & VQAOPTF_AUDIO) == 0) || (audio->Buffer == NULL)
			|| (audio->TempBufLen == 0)) {
	#endif /* VQAVOC_ON */

		return (0);
	}

	/* Compute start & end blocks to copy into */
	startblock = (audio->AudBufPos / config->HMIBufSize);
	endblock = (audio->AudBufPos + audio->TempBufLen) / config->HMIBufSize;

	if (endblock >= audio->NumAudBlocks) {
		endblock -= audio->NumAudBlocks;
	}

	/* If 'endblock' hasn't played yet, return VQAERR_SLEEPING */
	if (audio->IsLoaded[endblock] == 1) {
		return (VQAERR_SLEEPING);
	}
	
	SDL_LockAudioDevice(config->AudioDeviceID);

	/* Copy the data:
	 *
	 *  - If 'startblock' < 'endblock', copy the entire buffer
	 *  - Otherwise, fill to the end of the buffer with part of the data, then
	 *    copy the rest to the beginning of the buffer
	 */
	if (startblock <= endblock) {

		
		/* Copy data */
		memcpy((audio->Buffer + audio->AudBufPos), audio->TempBuf,
				audio->TempBufLen);

		/* Adjust current load position */
		audio->AudBufPos += audio->TempBufLen;

		/* Mark buffer as empty */
		audio->TempBufLen = 0;

		/* Set all blocks to loaded */
		for (i = startblock; i < endblock; i++) {
			audio->IsLoaded[i] = 1;
		}

		SDL_UnlockAudioDevice(config->AudioDeviceID);
		return (0);
	} else {

		/* Compute length of each piece */
		len1 = config->AudioBufSize - audio->AudBufPos;
		len2 = audio->TempBufLen - len1;

		/* Copy 1st piece into end of Audio Buffer */
		memcpy((audio->Buffer + audio->AudBufPos), audio->TempBuf, len1);

		/* Copy 2nd piece into start of Audio Buffer */
		memcpy(audio->Buffer, audio->TempBuf + len1, len2);

		/* Adjust load position */
		audio->AudBufPos = len2;

		/* Mark buffer as empty */
		audio->TempBufLen = 0;

		/* Set blocks to loaded */
		for (i = startblock; i < audio->NumAudBlocks; i++) {
			audio->IsLoaded[i] = 1;
		}

		for (i = 0; i < endblock; i++) {
			audio->IsLoaded[i] = 1;
		}

		SDL_UnlockAudioDevice(config->AudioDeviceID);
		return (0);
	}
}

void VQA_PauseAudio(void)
{
	VQAAudio  	*audio;
	if (VQAP && VQAP->VQABuf){

		audio = &VQAP->VQABuf->Audio;

		//if (audio->SecondaryBufferPtr){

			if (AudioFlags & VQAAUDF_ISPLAYING && !VQAAudioPaused) {

				//audio->SecondaryBufferPtr->Stop();
				VQAAudioPaused = true;
			}
		//}
	}
}


void VQA_ResumeAudio(void)
{
	VQAAudio  	*audio;
	if (VQAP && VQAP->VQABuf){

		audio = &VQAP->VQABuf->Audio;

		//if (audio->SecondaryBufferPtr){

			if (AudioFlags & VQAAUDF_ISPLAYING && VQAAudioPaused) {

				// TODO: resume
				VQAAudioPaused = false;
			}
		//}
	}
}


#elif VQAPICO_SOUND

extern int VQAMovieDone;
static bool VQAAudioPaused = false;

static void VQA_Audio_Callback(uint8_t *stream, int len)
{
	// called from main audio callback
	if(!VQAP)
		return;
	auto audio = &VQAP->VQABuf->Audio;
	if(!(audio->Flags & VQAAUDF_ISPLAYING) || VQAAudioPaused)
		return;

	auto config = &VQAP->Config;

	memcpy(stream, audio->Buffer + audio->PlayPosition + audio->BlockOffset, len);
	audio->BlockOffset += len;
	if (audio->BlockOffset == config->HMIBufSize) {
		audio->BlockOffset = 0;

		/* Compute the 'NextBlock' index */
		audio->NextBlock = audio->CurBlock + 1;

		if (audio->NextBlock >= audio->NumAudBlocks) {
			audio->NextBlock = 0;
		}

		/* See if the next block has data in it; if so, update the audio
			* buffer play position & the 'CurBlock' value.
			* If not, don't change anything and replay this block.
			*/
		if (audio->IsLoaded[audio->NextBlock] == 1) {

			/* Update this block's status to loadable (0) */
			audio->IsLoaded[audio->CurBlock] = 0;

			/* Update position within audio buffer */
			audio->PlayPosition += config->HMIBufSize;
			audio->CurBlock++;

			if (audio->PlayPosition >= config->AudioBufSize) {
				audio->PlayPosition = 0;
				audio->CurBlock = 0;
			}
			audio->ChunksMovedToAudioBuffer++;
		} else {
			if (VQAMovieDone){
				audio->ChunksMovedToAudioBuffer++;
			}
			audio->NumSkipped++;
			/*
			** Enable frame skipping to prevent this happening again
			*/
			config->DrawFlags &= ~VQACFGF_NOSKIP;
		}
	}
}

long VQA_StartTimerInt(VQAHandleP *vqap, long init)
{
	return 0;
}

void VQA_StopTimerInt(VQAHandleP *vqap)
{
}

static int OpenCount = 0;

long VQA_OpenAudio(VQAHandleP *vqap, void *window)
{
	VQAData       *vqabuf;
	VQAAudio      *audio;
	VQAConfig     *config;
	VQAHeader     *header;

	/* Dereference data memebers for quicker access. */
	config = &vqap->Config;
	header = &vqap->Header;
	vqabuf = vqap->VQABuf;
	audio = &vqabuf->Audio;

	/* Reset the buffer position to the beginning. */
	audio->CurBlock = 0;

	if(OpenCount)
	{
		// if we've already initialised make sure we're not in the callback
		// (by unsetting it)
		//SDL_LockAudioDevice(config->AudioDeviceID);
		*config->AudioCallback = NULL;
		//SDL_UnlockAudioDevice(config->AudioDeviceID);
	}

	// register our audio callback
	*config->AudioCallback = VQA_Audio_Callback;

	audio->Flags |= (HMI_VQAINIT << VQAAUDB_DIGIINIT);
	AudioFlags |= (HMI_VQAINIT << VQAAUDB_DIGIINIT);

	OpenCount++;

	return (0);
}

void VQA_CloseAudio(VQAHandleP *vqap)
{
	VQAAudio		*audio;
	VQAConfig	*config;

	/* Dereference for quick access. */
	audio	 = &vqap->VQABuf->Audio;
	config = &vqap->Config;

	/*
	** If the audio is still playing then stop it
	*/
	VQA_StopAudio(vqap);

	audio->Flags &= ~VQAAUDF_TIMERINIT;
	AudioFlags &= ~VQAAUDF_TIMERINIT;

	// don't remove the callback if open was called multiple times
	OpenCount--;
	if(OpenCount)
		return;

	// unregister our audio callback
	// and make sure we're not in it
	//SDL_LockAudioDevice(config->AudioDeviceID);
	*config->AudioCallback = NULL;
	//SDL_UnlockAudioDevice(config->AudioDeviceID);

	audio->Flags &= ~VQAAUDF_DIGIINIT;
	AudioFlags &= ~VQAAUDF_DIGIINIT;
	AudioFlags &= ~VQAAUDF_ISPLAYING;

}

long VQA_StartAudio(VQAHandleP *vqap)
{
	VQAConfig *config;
	VQAAudio  *audio;

	/* Save buffers for the callback routine */
	VQAP = vqap;

	/* Dereference commonly used data members for quicker access. */
	config = &vqap->Config;
	audio = &vqap->VQABuf->Audio;

	/* Return if already playing */
	if (AudioFlags & VQAAUDF_ISPLAYING) {
		return (-1);
	}

	//SDL_LockAudioDevice(config->AudioDeviceID);
	// setup playback
	audio->ChunksMovedToAudioBuffer = 0;
	audio->BlockOffset = 0;

	/*
	** Set the volume
	*/
	long volume = config->Volume << 7;
	//audio->SecondaryBufferPtr->SetVolume(- ( ( (32768 - volume)*1000) >>15 ) );

	audio->Flags |= VQAAUDF_ISPLAYING;
	AudioFlags |= VQAAUDF_ISPLAYING;

	//SDL_UnlockAudioDevice(config->AudioDeviceID);

	return (0);
}

void VQA_StopAudio(VQAHandleP *vqap)
{
	VQAAudio *audio;
	VQAConfig *config;

	/* Dereference commonly used data members for quicker access. */
	config = &vqap->Config;
	audio = &vqap->VQABuf->Audio;

	/* Just return if not playing */
	if (AudioFlags & VQAAUDF_ISPLAYING) {


		//audio->TimerHandle = NULL;

		audio->Flags &= ~VQAAUDF_ISPLAYING;
		AudioFlags &= ~VQAAUDF_ISPLAYING;
	}

	VQAP = NULL;
}

long CopyAudio(VQAHandleP *vqap)
{
	VQAAudio  *audio;
	VQAConfig *config;
	long      startblock;
	long      endblock;
	long      len1,len2;
	long      i;

	/* Dereference commonly used data members for quicker access. */
	audio = &vqap->VQABuf->Audio;
	config = &vqap->Config;

	/* If audio is disabled, or if we're playing from a VOC file, or if
	 * there's no Audio Buffer, or if there's no data to copy, just return 0
	 */
	#if(VQAVOC_ON && VQAAUDIO_ON)
	if (((config->OptionFlags & VQAOPTF_AUDIO) == 0) || (vqap->vocfh != -1)
			|| (audio->Buffer == NULL) || (audio->TempBufLen == 0)) {
	#else  /* VQAVOC_ON */
	if (((config->OptionFlags & VQAOPTF_AUDIO) == 0) || (audio->Buffer == NULL)
			|| (audio->TempBufLen == 0)) {
	#endif /* VQAVOC_ON */

		return (0);
	}

	/* Compute start & end blocks to copy into */
	startblock = (audio->AudBufPos / config->HMIBufSize);
	endblock = (audio->AudBufPos + audio->TempBufLen) / config->HMIBufSize;

	if (endblock >= audio->NumAudBlocks) {
		endblock -= audio->NumAudBlocks;
	}

	/* If 'endblock' hasn't played yet, return VQAERR_SLEEPING */
	if (audio->IsLoaded[endblock] == 1) {
		return (VQAERR_SLEEPING);
	}
	
	//SDL_LockAudioDevice(config->AudioDeviceID);

	/* Copy the data:
	 *
	 *  - If 'startblock' < 'endblock', copy the entire buffer
	 *  - Otherwise, fill to the end of the buffer with part of the data, then
	 *    copy the rest to the beginning of the buffer
	 */
	if (startblock <= endblock) {

		
		/* Copy data */
		memcpy((audio->Buffer + audio->AudBufPos), audio->TempBuf,
				audio->TempBufLen);

		/* Adjust current load position */
		audio->AudBufPos += audio->TempBufLen;

		/* Mark buffer as empty */
		audio->TempBufLen = 0;

		/* Set all blocks to loaded */
		for (i = startblock; i < endblock; i++) {
			audio->IsLoaded[i] = 1;
		}

		//SDL_UnlockAudioDevice(config->AudioDeviceID);
		return (0);
	} else {

		/* Compute length of each piece */
		len1 = config->AudioBufSize - audio->AudBufPos;
		len2 = audio->TempBufLen - len1;

		/* Copy 1st piece into end of Audio Buffer */
		memcpy((audio->Buffer + audio->AudBufPos), audio->TempBuf, len1);

		/* Copy 2nd piece into start of Audio Buffer */
		memcpy(audio->Buffer, audio->TempBuf + len1, len2);

		/* Adjust load position */
		audio->AudBufPos = len2;

		/* Mark buffer as empty */
		audio->TempBufLen = 0;

		/* Set blocks to loaded */
		for (i = startblock; i < audio->NumAudBlocks; i++) {
			audio->IsLoaded[i] = 1;
		}

		for (i = 0; i < endblock; i++) {
			audio->IsLoaded[i] = 1;
		}

		//SDL_UnlockAudioDevice(config->AudioDeviceID);
		return (0);
	}
}

void VQA_PauseAudio(void)
{
	if (VQAP && VQAP->VQABuf){
		if (AudioFlags & VQAAUDF_ISPLAYING && !VQAAudioPaused)
			VQAAudioPaused = true;
	}
}

void VQA_ResumeAudio(void)
{
	if (VQAP && VQAP->VQABuf){
		if (AudioFlags & VQAAUDF_ISPLAYING && VQAAudioPaused)
			VQAAudioPaused = false;
	}
}


#elif(!VQADIRECT_SOUND)

/* This is a dummy function that is used to mark the start of the module.
 * It is necessary for locking the memory the module occupies. This prevents
 * the virtual memory manager from swapping out this memory.
 */
static void StartAddr(void)
{
}


/****************************************************************************
*
* NAME
*     VQA_StartTimerInt - Initialize system timer interrupt.
*
* SYNOPSIS
*     Error = VQA_StartTimerInt(VQA, Init)
*
*     long VQA_StartTimerInt(VQAHandeP *, long);
*
* FUNCTION
*     Initialize the HMI timer system and add our own timer event. If the
*     system has already been initialized then we are given access to the
*     the timer system.
*
* INPUTS
*     VQA  - Pointer to private VQAHandle structure.
*     Init - Initialize HMI timer system flag. (TRUE = Initialize)
*
* RESULT
*     Error - 0 if successful, -1 if error.
*
****************************************************************************/

long VQA_StartTimerInt(VQAHandleP *vqap, long init)
{
	VQAAudio *audio;

	/* Dereference for quick access. */
	audio = &vqap->VQABuf->Audio;

	/* Does the caller want me to initialize the timer system? */
	if (init == 0) {

		/* If the timer system is uninitialized then it is the players
		 * responsibility to do it.
		 */
		if ((AudioFlags & VQAAUDF_TIMERINIT)==(HMI_UNINIT<<VQAAUDB_TIMERINIT)) {
			sosTIMERInitSystem(_TIMER_DOS_RATE, _SOS_DEBUG_NORMAL);

			/* Flag the timer system as open. */
			AudioFlags |= (HMI_VQAINIT << VQAAUDB_TIMERINIT);
		}

		/* Flag availability of the timer system. */
		audio->Flags |= (HMI_VQAINIT << VQAAUDB_TIMERINIT);
	} else {
		audio->Flags |= (HMI_APPINIT << VQAAUDB_TIMERINIT);
	}

	/* Increment the timer system usage count. */
	TimerSysCount++;

	/* Register the VQA_TickCount timer event. */
	if ((AudioFlags & VQAAUDF_HMITIMER) == (HMI_UNINIT<<VQAAUDB_HMITIMER)) {
		if (sosTIMERRegisterEvent(VQA_TIMETICKS,TimerCallback,&VQATimer) == 0) {

			/* Flag the timer interrupt as being registered. */
			AudioFlags |= (HMI_VQAINIT << VQAAUDB_HMITIMER);
		} else {
			return (-1);
		}
	}

	/* Flag availability of the timer interrupt. */
	audio->Flags |= (HMI_VQAINIT << VQAAUDB_HMITIMER);

	/* Increment the timer interrupt usage count. */
	TimerIntCount++;

	/* Lock the memory occupied by this module. */
	if ((audio->Flags & VQAAUDF_MODLOCKED) == 0) {
		DPMI_Lock((void *)&StartAddr, (long)&EndAddr - (long)&StartAddr);
		audio->Flags |= VQAAUDF_MODLOCKED;
	}

	return (0);
}


/****************************************************************************
*
* NAME
*     VQA_StopTimerInt - Remove system timer interrupt.
*
* SYNOPSIS
*     VQA_StopTimerInt()
*
*     void VQA_StopTimerInt(void);
*
* FUNCTION
*     Remove our timer event from the HMI timer system. Uninitialize the
*     HMI timer system if we initialized it.
*
* INPUTS
*     NONE
*
* RESULT
*     NONE
*
****************************************************************************/

void VQA_StopTimerInt(VQAHandleP *vqap)
{
	VQAAudio *audio;

	/* Dereference for quick access. */
	audio = &vqap->VQABuf->Audio;

	/* Decrement the timer interrupt usage count. */
	if (TimerIntCount) {
		TimerIntCount--;
	}

	/* Remove the timer interrrupt if it is initialized and the use count is
	 * zero. Otherwise, clear the callers timer interrupt availability flag.
	 */
	if (((AudioFlags & VQAAUDF_HMITIMER) == (HMI_VQAINIT<<VQAAUDB_HMITIMER))
			&& (TimerIntCount == 0)) {

		sosTIMERRemoveEvent(VQATimer);
		AudioFlags &= ~VQAAUDF_HMITIMER;
	} else {
		audio->Flags &= ~VQAAUDF_HMITIMER;
	}

	/* Derement the timer system usage count. */
	if (TimerSysCount) {
		TimerSysCount--;
	}

	/* Close the timer system if it has been opened and the use count is zero.
	 * Otherwise, clear the callers timer system availability flag.
	 */
	if (((AudioFlags & VQAAUDF_TIMERINIT) == (HMI_VQAINIT<<VQAAUDB_TIMERINIT))
			&& (TimerSysCount == 0)) {

		sosTIMERUnInitSystem(0);
		AudioFlags &= ~VQAAUDF_TIMERINIT;
	} else {
		audio->Flags &= ~VQAAUDF_TIMERINIT;
	}

	/* Unlock the memory accupied by this module. */
	if ((audio->Flags & VQAAUDF_MODLOCKED) == 1) {
		DPMI_Unlock(&StartAddr, (long)&EndAddr - (long)&StartAddr);
		audio->Flags &= ~VQAAUDF_MODLOCKED;
	}
}


/****************************************************************************
*
* NAME
*     TimerCallback - VQA timer event. (Called by HMI)
*
* SYNOPSIS
*     TimerCallback()
*
*     void TimerCallback(void);
*
* FUNCTION
*     Our custom timer event. This is the timer event that we register with
*     HMI for system timing.
*
* INPUTS
*     NONE
*
* RESULT
*     NONE
*
****************************************************************************/

void far TimerCallback(void)
{
	VQATickCount++;
}


/****************************************************************************
*
* NAME
*     VQA_OpenAudio - Open sound system.
*
* SYNOPSIS
*     Error = VQA_OpenAudio(VQAHandleP)
*
*     long VQA_OpenAudio(VQAHandleP *);
*
* FUNCTION
*     Initialize the sound system by setting the DigiHandle to and HMI
*     driver handle. The timer must first be initialized before calling
*     this function.
*
* INPUTS
*     VQAHandleP - Pointer to private VQAHandle.
*
* RESULT
*     Error - 0 if successful, -1 if error.
*
****************************************************************************/

long VQA_OpenAudio(VQAHandleP *vqap)
{
	VQAData       *vqabuf;
	VQAAudio      *audio;
	VQAConfig     *config;
	VQAHeader     *header;
	unsigned char *driver_path;
	WORD          port;
	long          rc;

	/* Dereference data memebers for quicker access. */
	config = &vqap->Config;
	header = &vqap->Header;
	vqabuf = vqap->VQABuf;
	audio = &vqabuf->Audio;

	/* Fail if no audio buffer or DigiCard is 0 (no sound) */
	if ((audio->Buffer == NULL) || (config->DigiCard == 0)) {
		return (-1);
	}

	/* Reset the buffer position to the beginning. */
	audio->CurBlock = 0;

	/* Initialize the HMI driver structure. */
	audio->sSOSInitDriver.wAllocateBuffer = _TRUE;
	audio->sSOSInitDriver.wParam = 19;
	audio->sSOSInitDriver.wTimerID = _SOS_NORMAL_TIMER;

	/*-------------------------------------------------------------------------
	 * Compute the playback rate:
	 *
	 * - If Config->AudioRate is -1, use HMI_DATA_RATE scaled to the specified
	 *   frame rate (so the audio plays faster if we're playing faster)
	 * - otherwise, use the specified rate
	 *-----------------------------------------------------------------------*/
	if (config->AudioRate != -1) {
		audio->sSOSInitDriver.wSampleRate = config->AudioRate;
	}
	else if (config->FrameRate != header->FPS) {
		audio->sSOSInitDriver.wSampleRate = ((audio->SampleRate*config->FrameRate)
				/ (unsigned long)header->FPS);

		config->AudioRate = audio->sSOSInitDriver.wSampleRate;
	} else {
		audio->sSOSInitDriver.wSampleRate = audio->SampleRate;
		config->AudioRate = audio->SampleRate;
	}

	/* If the application has already initialized HMI then set the
	 * necessary variables. Otherwise we must setup HMI ourself.
	 */
	if ((config->OptionFlags & VQAOPTF_HMIINIT)
			|| ((AudioFlags & VQAAUDF_DIGIINIT) != 0)) {

		/* The application MUST provide the card type! */
		if (config->DigiCard == -1) {
			return (-1);
		}

		/* Init the detection system */
		driver_path = (unsigned char *)".\\";

		if ((rc = sosDIGIDetectInit(driver_path)) != 0) {
			return (rc);
		}

		/* Get the capabilities of the card being used. */
		rc = sosDIGIDetectGetCaps(config->DigiCard, &audio->DigiCaps);
		sosDIGIDetectUnInit();

		if (rc != 0) {
			return (rc);
		}

		audio->DigiHandle = config->DigiHandle;
		audio->Flags |= (HMI_APPINIT << VQAAUDB_TIMERINIT);
		audio->Flags |= (HMI_APPINIT << VQAAUDB_DIGIINIT);

		if ((AudioFlags & (VQAAUDF_TIMERINIT|VQAAUDF_DIGIINIT)) == 0) {
			HMIDevName = "App-Specific";
			AudioFlags |= (HMI_APPINIT << VQAAUDB_TIMERINIT);
			AudioFlags |= (HMI_APPINIT << VQAAUDB_DIGIINIT);
		}
	} else {

		/* Init the detection system */
		driver_path = (unsigned char *)".\\";

		if ((rc = sosDIGIDetectInit(driver_path)) != 0) {
			return (rc);
		}

		/*-----------------------------------------------------------------------
		 * Initialize DigiHardware with port, IRQ, and DMA, and make sure
		 * Config.DigiCard contains the HMI ID we want to use:
		 *
		 * - If Config.DigiCard is -1, auto-detect the hardware; then, do a
		 *   FindHardware so the GetSettings will work
		 * - If Config.DigiCard is filled in, but port, IRQ or DMA is -1, use
		 *   FindHardware & GetSettings to get the settings
		 * - If all are filled in, just use them as they are
		 *---------------------------------------------------------------------*/

		/* Auto-Detect */
		if (config->DigiCard == -1) {

			/* Version 1 VQA's have only 8 bit mono audio streams. */
			if (header->Version == VQAHD_VER1) {
				config->DigiCard = AutoDetect(&audio->DigiCaps, 8, 1);
			} else {
				config->DigiCard = AutoDetect(&audio->DigiCaps, audio->BitsPerSample,
						audio->Channels);

				/* Resort to 8bit mono */
				if (config->DigiCard == -1) {
					config->DigiCard = AutoDetect(&audio->DigiCaps, 8, 1);
				}
			}

			if (config->DigiCard == -1) {
				sosDIGIDetectUnInit();
				return (-1);
			}
		}

		/* Do a FindHardware & GetSettings */
		if (config->DigiPort == -1) {
		  if (sosDIGIDetectFindHardware(config->DigiCard, &audio->DigiCaps,
					&port)) {

				sosDIGIDetectUnInit();
				return (-1);
			}

			if (sosDIGIDetectGetSettings(&audio->DigiHardware)) {
				sosDIGIDetectUnInit();
				return (-1);
			}

			config->DigiPort = audio->DigiHardware.wPort;
			config->DigiIRQ = audio->DigiHardware.wIRQ;
			config->DigiDMA = audio->DigiHardware.wDMA;
			HMIDevName = (char *)audio->DigiCaps.szDeviceName;
		} else {
			audio->DigiHardware.wPort = config->DigiPort;
			audio->DigiHardware.wIRQ = config->DigiIRQ;
			audio->DigiHardware.wDMA = config->DigiDMA;
			HMIDevName = "App-Specific";
		}

		sosDIGIDetectUnInit();

		/* Initialize the DIGI system & driver */
		sosDIGIInitSystem(driver_path, _SOS_DEBUG_NORMAL);
		audio->sSOSInitDriver.wBufferSize = config->HMIBufSize;
		audio->sSOSInitDriver.lpBuffer = NULL;
		audio->sSOSInitDriver.lpFillHandler = NULL;
		audio->sSOSInitDriver.lpDriverMemory = NULL;
		audio->sSOSInitDriver.lpTimerMemory = NULL;
		audio->DigiHandle = -1;

		if ((rc = sosDIGIInitDriver(config->DigiCard, &audio->DigiHardware,
				&audio->sSOSInitDriver, &audio->DigiHandle)) != 0) {

			return (rc);
		}

		/*-----------------------------------------------------------------------
		 * Register the timer event
		 *---------------------------------------------------------------------*/

		/* If the timer hasn't been init'd, do it now */
		if ((AudioFlags & VQAAUDF_TIMERINIT) == (HMI_UNINIT<<VQAAUDB_TIMERINIT)) {
			sosTIMERInitSystem(_TIMER_DOS_RATE ,_SOS_DEBUG_NORMAL);
			audio->Flags |= (HMI_VQAINIT << VQAAUDB_TIMERINIT);
 			AudioFlags |= (HMI_VQAINIT << VQAAUDB_TIMERINIT);
		}

		/* Register the event */
		rc = sosTIMERRegisterEvent(VQA_TIMETICKS,
				audio->sSOSInitDriver.lpFillHandler, &audio->DigiTimer);

		if (rc) {
			sosDIGIUnInitDriver(audio->DigiHandle, _TRUE, _TRUE);
			sosDIGIUnInitSystem();
			return (rc);
		}

		config->DigiHandle = audio->DigiHandle;
		audio->Flags |= (HMI_VQAINIT << VQAAUDB_DIGIINIT);
		AudioFlags |= (HMI_VQAINIT << VQAAUDB_DIGIINIT);
	}

	if ((audio->Flags & VQAAUDF_MODLOCKED) == 0) {
		DPMI_Lock(&StartAddr, (long)&EndAddr - (long)&StartAddr);
		audio->Flags |= VQAAUDF_MODLOCKED;
	}

	return (0);
}


/****************************************************************************
*
* NAME
*     VQA_CloseAudio - Close sound system
*
* SYNOPSIS
*     VQA_CloseAudio()
*
*     void VQA_CloseAudio(void);
*
* FUNCTION
*     Removes VQA's involvement in the audio system.
*
* INPUTS
*     NONE
*
* RESULT
*     NONE
*
****************************************************************************/

void VQA_CloseAudio(VQAHandleP *vqap)
{
	VQAAudio *audio;

	/* Dereference for quick access. */
	audio = &vqap->VQABuf->Audio;

	/* Remove the Digi event */
	if ((AudioFlags & VQAAUDF_DIGIINIT) == (HMI_VQAINIT<<VQAAUDB_DIGIINIT)) {
		sosTIMERRemoveEvent(audio->DigiTimer);
	}

	/* Un-init timer if necessary */
	if ((AudioFlags & VQAAUDF_TIMERINIT) == (HMI_VQAINIT<<VQAAUDB_TIMERINIT)) {
		sosTIMERUnInitSystem(0);
	}

	audio->Flags &= ~VQAAUDF_TIMERINIT;
	AudioFlags &= ~VQAAUDF_TIMERINIT;

	/* Remove the driver */
	if ((audio->Flags & VQAAUDF_DIGIINIT) == (HMI_VQAINIT<<VQAAUDB_DIGIINIT)) {
		sosDIGIUnInitDriver(audio->DigiHandle, _TRUE, _TRUE);
		sosDIGIUnInitSystem();
	}

	audio->Flags &= ~VQAAUDF_DIGIINIT;
	AudioFlags &= ~VQAAUDF_DIGIINIT;
	AudioFlags &= ~VQAAUDF_ISPLAYING;

	/* Unlock the memory accupied by this module. */
	if ((audio->Flags & VQAAUDF_MODLOCKED) == 1) {
		DPMI_Unlock(&StartAddr, (long)&EndAddr - (long)&StartAddr);
		audio->Flags &= ~VQAAUDF_MODLOCKED;
	}
}


/****************************************************************************
*
* NAME
*     AutoDetect - Auto detect the sound card.
*
* SYNOPSIS
*     CardID = AutoDetect(DigiCaps, BitSize, Channels)
*
*     long AutoDetect(_SOS_CAPABILITIES *, long, long);
*
* FUNCTION
*     Autodetects the type of sound card present in the system.
*
* INPUTS
*     DigiCaps - Pointer to HMI digital card capabilities structure.
*     BitSize  - Bits per sample size.
*     Channels - Number of desired channels.
*
* RESULT
*     CardID - HMI ID of the sound card found, -1 if none.
*
****************************************************************************/

long AutoDetect(_SOS_CAPABILITIES *digicaps, long bitsize, long channels)
{
	long device_id = -1;
	WORD port;
	long i;
	long rc;

	/* Search for an 8-bit mono device */
	if (sosDIGIDetectFindFirst(digicaps, &port)) {
		return (-1);
	}

	channels--;

	i = 0;
	while (i < 6) {
		i++;

		if ((digicaps->wBitsPerSample == bitsize)
				&& (digicaps->wChannels == channels)) {

			break;
		}

		if (sosDIGIDetectFindNext(digicaps, &port)) {
			return (-1);
		}
	}

	/* Exit if failed to find the required device */
	if ((digicaps->wBitsPerSample != bitsize)
			|| (digicaps->wChannels != channels)) {

		return (-1);
	}

	/* Stash the ID */
	device_id = digicaps->wDeviceID;

	/* Now that we have handled the initial pass, verify that if we found an
	 * _ADLIB_GOLD_8_MONO that it is not a Logitech Sound Wave man in disguise.
	 */
	if ((WORD)digicaps->wDeviceID == _ADLIB_GOLD_8_MONO) {
		rc = sosDIGIDetectFindNext(digicaps, &port);

		while ((i < 6) && (rc == 0)) {
			i++;

			if ((digicaps->wBitsPerSample == 8) && (digicaps->wChannels == 0)) {
				break;
			}

			if ((rc = sosDIGIDetectFindNext(digicaps, &port)) != 0) {
				break;
			}
		}

		/* If we don't have an error use the secondary device ID, after all,
		 * anything's better than an Adlib Gold.  If we do have an error or there
		 * is nothing we can use then the device ID is already set to the adlib
		 * gold so just let it rip.
		 */
		if ((rc == 0) && ((WORD)digicaps->wDeviceID == _SBPRO_8_MONO)) {
			return (digicaps->wDeviceID);
		}
	}

	return (device_id);
}


/****************************************************************************
*
* NAME
*     VQA_StartAudio - Starts audio playback
*
* SYNOPSIS
*     Error = VQA_StartAudio(VQA)
*
*     long VQA_StartAudio(VQAHandleP *);
*
* FUNCTION
*     Start the audio playback for the movie.
*
* INPUTS
*     VQA - Pointer to private VQA handle.
*
* RESULT
*     Error - 0 if successful, or -1 error code.
*
****************************************************************************/

long VQA_StartAudio(VQAHandleP *vqap)
{
	VQAConfig *config;
	VQAAudio  *audio;

	/* Save buffers for the callback routine */
	VQAP = vqap;

	/* Dereference commonly used data members for quicker access. */
	config = &vqap->Config;
	audio = &vqap->VQABuf->Audio;

	/* Return if already playing */
	if (AudioFlags & VQAAUDF_ISPLAYING) {
		return (-1);
	}

 	/* Set my driver handle */
	if (config->DigiHandle != -1) {
		audio->DigiHandle = config->DigiHandle;
	}

	if (audio->DigiHandle == (WORD)-1) {
		return (-1);
	}

	/*-------------------------------------------------------------------------
	 * Initialize the sample structure.
	 *-----------------------------------------------------------------------*/
	memset(&audio->sSOSSampleData, 0, sizeof(_SOS_START_SAMPLE));
  audio->sSOSSampleData.lpSamplePtr = (unsigned char *)audio->Buffer;
 	audio->sSOSSampleData.dwSampleSize = config->HMIBufSize;
	audio->sSOSSampleData.wVolume = (config->Volume << 7);
	audio->sSOSSampleData.wSampleID = HMI_SAMPLE;
	audio->sSOSSampleData.lpCallback = AudioCallback;

	/* Set the channel flags for the type of data we have. */
	if (audio->Channels == 2) {
		audio->sSOSSampleData.wChannel = _INTERLEAVED;
	} else {
		audio->sSOSSampleData.wChannel = _CENTER_CHANNEL;
	}

	/* If the card is unable to handle stereo data the we must notify the
	 * sound system to convert the stereo data to mono data during playback.
	 */
	if ((audio->Channels - 1) > audio->DigiCaps.wChannels) {
		audio->sSOSSampleData.wSampleFlags |= _STEREOTOMONO;
	}

	/* If the card is unable to handle the sample size of the audio data
	 * then we must notify the sound system to convert the audio data to
	 * the proper format.
	 */
	if (audio->BitsPerSample != audio->DigiCaps.wBitsPerSample) {
		if (audio->BitsPerSample > audio->DigiCaps.wBitsPerSample) {
			audio->sSOSSampleData.wSampleFlags |= _TRANSLATE16TO8;
		} else {
			audio->sSOSSampleData.wSampleFlags |= _TRANSLATE8TO16;
		}
	}

	/* Adjust the pitch if the driver is setup to playback at a different
	 * rate than what the sample was recorded at.
	 */
	if (audio->sSOSInitDriver.wSampleRate != audio->SampleRate) {
		ldiv_t result;

		result = ldiv(audio->SampleRate, audio->sSOSInitDriver.wSampleRate);
		audio->sSOSSampleData.dwSamplePitchAdd = (long)MAKE_LONG((short)result.quot,
				(short)(((long)result.rem * 0x10000L) / audio->sSOSInitDriver.wSampleRate));
		audio->sSOSSampleData.wSampleFlags |= _PITCH_SHIFT;
	}

	/* Start playback */
 	audio->SampleHandle = sosDIGIStartSample(audio->DigiHandle,
			&audio->sSOSSampleData);

	audio->Flags |= VQAAUDF_ISPLAYING;
	AudioFlags |= VQAAUDF_ISPLAYING;

	return (0);
}


/****************************************************************************
*
* NAME
*     VQA_StopAudio - Stop audio playback.
*
* SYNOPSIS
*     VQA_StopAudio(VQA)
*
*     void VQA_StopAudio(VQAHandleP *);
*
* FUNCTION
*     Halts the currently playing audio stream.
*
* INPUTS
*     VQA - Pointer to private VQAHandle.
*
* RESULT
*     NONE
*
****************************************************************************/

void VQA_StopAudio(VQAHandleP *vqap)
{
	VQAAudio *audio;

	/* Dereference for quick access. */
	audio = &vqap->VQABuf->Audio;

	/* Just return if not playing */
	if (AudioFlags & VQAAUDF_ISPLAYING) {
		sosDIGIStopSample(audio->DigiHandle, audio->SampleHandle);
		audio->Flags &= ~VQAAUDF_ISPLAYING;
		AudioFlags &= ~VQAAUDF_ISPLAYING;
	}

	VQAP = NULL;
}


/****************************************************************************
*
* NAME
*     CopyAudio - Copy data from Audio Temp buffer into Audio play buffer.
*
* SYNOPSIS
*     Error = CopyAudio(VQA)
*
*     long CopyAudio(VQAHandleP *);
*
* FUNCTION
*     This routine just copies the data in the TempBuf into the correct
*     spots in the audio play buffer.  If there is no room available in the
*     audio play buffer, the routine returns VQAERR_SLEEPING, which will put
*     the whole Loader to "sleep" while it waits for a free buffer.
*
*     If there's no data in the TempBuf to copy, the routine just returns 0.
*
* INPUTS
*     VQA - Pointer to private VQAHandle structure.
*
* RESULT
*     Error - 0 if successful or VQAERR_??? error code.
*
****************************************************************************/

long CopyAudio(VQAHandleP *vqap)
{
	VQAAudio  *audio;
	VQAConfig *config;
	long      startblock;
	long      endblock;
	long      len1,len2;
	long      i;

	/* Dereference commonly used data members for quicker access. */
	audio = &vqap->VQABuf->Audio;
	config = &vqap->Config;

	/* If audio is disabled, or if we're playing from a VOC file, or if
	 * there's no Audio Buffer, or if there's no data to copy, just return 0
	 */
	#if(VQAVOC_ON && VQAAUDIO_ON)
	if (((config->OptionFlags & VQAOPTF_AUDIO) == 0) || (vqap->vocfh != -1)
			|| (audio->Buffer == NULL) || (audio->TempBufLen == 0)) {
	#else  /* VQAVOC_ON */
	if (((config->OptionFlags & VQAOPTF_AUDIO) == 0) || (audio->Buffer == NULL)
			|| (audio->TempBufLen == 0)) {
	#endif /* VQAVOC_ON */

		return (0);
	}

	/* Compute start & end blocks to copy into */
	startblock = (audio->AudBufPos / config->HMIBufSize);
	endblock = (audio->AudBufPos + audio->TempBufLen) / config->HMIBufSize;

	if (endblock >= audio->NumAudBlocks) {
		endblock -= audio->NumAudBlocks;
	}

	/* If 'endblock' hasn't played yet, return VQAERR_SLEEPING */
	if (audio->IsLoaded[endblock] == 1) {
		return (VQAERR_SLEEPING);
	}

	/* Copy the data:
	 *
	 *  - If 'startblock' < 'endblock', copy the entire buffer
	 *  - Otherwise, fill to the end of the buffer with part of the data, then
	 *    copy the rest to the beginning of the buffer
	 */
	if (startblock <= endblock) {

		/* Copy data */
		memcpy((audio->Buffer + audio->AudBufPos), audio->TempBuf,
				audio->TempBufLen);

		/* Adjust current load position */
		audio->AudBufPos += audio->TempBufLen;

		/* Mark buffer as empty */
		audio->TempBufLen = 0;

		/* Set all blocks to loaded */
		for (i = startblock; i < endblock; i++) {
			audio->IsLoaded[i] = 1;
		}

		return (0);
	} else {

		/* Compute length of each piece */
		len1 = config->AudioBufSize - audio->AudBufPos;
		len2 = audio->TempBufLen - len1;

		/* Copy 1st piece into end of Audio Buffer */
		memcpy((audio->Buffer + audio->AudBufPos), audio->TempBuf, len1);

		/* Copy 2nd piece into start of Audio Buffer */
		memcpy(audio->Buffer, audio->TempBuf + len1, len2);

		/* Adjust load position */
		audio->AudBufPos = len2;

		/* Mark buffer as empty */
		audio->TempBufLen = 0;

		/* Set blocks to loaded */
		for (i = startblock; i < audio->NumAudBlocks; i++) {
			audio->IsLoaded[i] = 1;
		}

		for (i = 0; i < endblock; i++) {
			audio->IsLoaded[i] = 1;
		}

		return (0);
	}
}


/****************************************************************************
*
* NAME
*     AudioCallback - Sound system callback.
*
* SYNOPSIS
*     AudioCallback(DriverHandle, Action, SampleID)
*
*     void AudioCallback(WORD, WORD, WORD);
*
* FUNCTION
*     Our custom audio callback routine that services HMI.
*
* INPUTS
*     DriverHandle - HMI driver handle.
*     Action       - Action taken.
*     SampleID     - ID of sample.
*
* RESULT
*     NONE
*
****************************************************************************/

void far __cdecl AudioCallback(WORD wDriverHandle,WORD wAction,WORD wSampleID)
{
	VQAAudio  *audio;
	VQAConfig *config;

	/* Dereference commonly used data members for quicker access. */
	audio = &VQAP->VQABuf->Audio;
	config = &VQAP->Config;

	/* Suppress compiler warnings */
	wDriverHandle = wDriverHandle;
	wSampleID = wSampleID;

  /* See if we're called because the buffer is empty */
	if (wAction == _SAMPLE_PROCESSED) {

		/* Compute the 'NextBlock' index */
		audio->NextBlock = audio->CurBlock + 1;

		if (audio->NextBlock >= audio->NumAudBlocks) {
			audio->NextBlock = 0;
		}

		/* See if the next block has data in it; if so, update the audio
		 * buffer play position & the 'CurBlock' value.
		 * If not, don't change anything and replay this block.
		 */
		if (audio->IsLoaded[audio->NextBlock] == 1) {

			/* Update this block's status to loadable (0) */
			audio->IsLoaded[audio->CurBlock] = 0;

			/* Update position within audio buffer */
			audio->PlayPosition += config->HMIBufSize;
			audio->CurBlock++;

			if (audio->PlayPosition >= config->AudioBufSize) {
				audio->PlayPosition = 0;
				audio->CurBlock = 0;
			}
		} else {
			audio->NumSkipped++;
		}

		/* Start the new buffer playing */
	  audio->sSOSSampleData.lpSamplePtr = (unsigned char *)(audio->Buffer)
				+ audio->PlayPosition;

   	sosDIGIContinueSample(audio->DigiHandle, audio->SampleHandle,
				&audio->sSOSSampleData);

		audio->SamplesPlayed += config->HMIBufSize;
	}
}

/* Dummy function used to mark the beginning address of the file. */
static void EndAddr(void)
{
}



#else	//!VQADIRECT_SOUND

/**************************************************************************
*
* Start of Direct Sound code
*
*
* The direct sound implementation works by taking what would have been the
* HMI sound buffer and feeding the contents to a direct sound secondary
* buffer.
*
* Steve T. -  12/15/95
*
*
*
*
***************************************************************************/

BOOL	SuspendAudioCallback = FALSE;

/****************************************************************************
*
* NAME
*     VQA_StartTimerInt - Initialize system timer interrupt.
*
* SYNOPSIS
*     Error = VQA_StartTimerInt(VQA, Init)
*
*     long VQA_StartTimerInt(VQAHandeP *, long);
*
* FUNCTION
*     Initialize the HMI timer system and add our own timer event. If the
*     system has already been initialized then we are given access to the
*     the timer system.
*
* INPUTS
*     VQA  - Pointer to private VQAHandle structure.
*     Init - Initialize HMI timer system flag. (TRUE = Initialize)
*
* RESULT
*     Error - 0 if successful, -1 if error.
*
****************************************************************************/

long VQA_StartTimerInt(VQAHandleP *vqap, long init)
{
	VQAAudio *audio;

	/* Dereference for quick access. */
	audio = &vqap->VQABuf->Audio;

	/* Register the VQA_TickCount timer event. */
	if ((AudioFlags & VQAAUDF_HMITIMER) == (HMI_UNINIT<<VQAAUDB_HMITIMER)) {
		VQATimer = timeSetEvent ( 1000/VQA_TIMETICKS , 1 , TimerCallback , 0 , TIME_PERIODIC);

		if (VQATimer){

			/* Flag the timer interrupt as being registered. */
			AudioFlags |= (HMI_VQAINIT << VQAAUDB_HMITIMER);
		} else {
			return (-1);
		}
	}

	/* Flag availability of the timer interrupt. */
	audio->Flags |= (HMI_VQAINIT << VQAAUDB_HMITIMER);

	/* Increment the timer interrupt usage count. */
	TimerIntCount++;

	return (0);
}


/****************************************************************************
*
* NAME
*     VQA_StopTimerInt - Remove system timer interrupt.
*
* SYNOPSIS
*     VQA_StopTimerInt()
*
*     void VQA_StopTimerInt(void);
*
* FUNCTION
*     Remove our timer event from the HMI timer system. Uninitialize the
*     HMI timer system if we initialized it.
*
* INPUTS
*     NONE
*
* RESULT
*     NONE
*
****************************************************************************/

void VQA_StopTimerInt(VQAHandleP *vqap)
{
	VQAAudio *audio;

	/* Dereference for quick access. */
	audio = &vqap->VQABuf->Audio;

	/* Decrement the timer interrupt usage count. */
	if (TimerIntCount) {
		TimerIntCount--;
	}

	/* Remove the timer interrrupt if it is initialized and the use count is
	 * zero. Otherwise, clear the callers timer interrupt availability flag.
	 */
	if (((AudioFlags & VQAAUDF_HMITIMER) == (HMI_VQAINIT<<VQAAUDB_HMITIMER))
			&& (TimerIntCount == 0)) {

		timeKillEvent(VQATimer);
		AudioFlags &= ~VQAAUDF_HMITIMER;
	} else {
		AudioFlags &= ~VQAAUDF_HMITIMER;
	}

}


/****************************************************************************
*
* NAME
*     TimerCallback - VQA timer event. (Called by Windoze)
*
* SYNOPSIS
*     TimerCallback()
*
*     void TimerCallback(void);
*
* FUNCTION
*     Our custom timer event. This is the timer event that we register with
*     Windows for system timing.
*
* INPUTS
*     NONE
*
* RESULT
*     NONE
*
****************************************************************************/

void CALLBACK TimerCallback (UINT , UINT , DWORD , DWORD , DWORD)
{
	VQATickCount++;
}


/****************************************************************************
*
* NAME
*     VQA_OpenAudio - Open sound system.
*
* SYNOPSIS
*     Error = VQA_OpenAudio(VQAHandleP)
*
*     long VQA_OpenAudio(VQAHandleP *);
*
* FUNCTION
*     Initialise the sound system. Create a direct sound object and the
*     direct sound primary sound buffer if they dont already exist.
*
* INPUTS
*     VQAHandleP - Pointer to private VQAHandle.
*
* RESULT
*     Error - 0 if successful, -1 if error.
*
****************************************************************************/

long VQA_OpenAudio(VQAHandleP *vqap, HWND window)
{
	VQAData       *vqabuf;
	VQAAudio      *audio;
	VQAConfig     *config;
	VQAHeader     *header;
	unsigned char *driver_path;
	WORD          port;
	long          rc;

	/* Dereference data memebers for quicker access. */
	config = &vqap->Config;
	header = &vqap->Header;
	vqabuf = vqap->VQABuf;
	audio = &vqabuf->Audio;

	/* Reset the buffer position to the beginning. */
	audio->CurBlock = 0;


	/*
	**	create the direct sound object if it doesnt already exist
	*/
	if (!config->SoundObject){

		if ( DirectSoundCreate (NULL,&config->SoundObject,NULL) !=DS_OK ) {
			return (-1);
		}else{
			audio->CreatedSoundObject = TRUE;
			/*
			**	Give ourselves exclusive access to the sound card
			*/
			if ( config->SoundObject->SetCooperativeLevel( window, DSSCL_EXCLUSIVE ) != DS_OK){
				config->SoundObject->Release();
				return (-1);
			}
		}


		if (!config->PrimaryBufferPtr){
			/*
			** Define the format of the primary sound buffer
			*/
			memset (&audio->BufferDesc , 0 , sizeof(DSBUFFERDESC));
			audio->BufferDesc.dwSize=sizeof(DSBUFFERDESC);
			audio->BufferDesc.dwFlags=DSBCAPS_PRIMARYBUFFER | DSBCAPS_CTRLVOLUME;

			memset (&audio->DsBuffFormat , 0 , sizeof(WAVEFORMATEX));
			audio->DsBuffFormat.wFormatTag	= WAVE_FORMAT_PCM;
			audio->DsBuffFormat.nChannels		= (unsigned short) 2;


			/*-------------------------------------------------------------------------
	 		* Compute the playback rate:
	 		*
	 		* - If Config->AudioRate is -1, use HMI_DATA_RATE scaled to the specified
	 		*   frame rate (so the audio plays faster if we're playing faster)
	 		* - otherwise, use the specified rate
	 		*-----------------------------------------------------------------------*/
			if (config->AudioRate != -1) {
				audio->DsBuffFormat.nSamplesPerSec = config->AudioRate;
			}
			else if (config->FrameRate != header->FPS) {
				audio->DsBuffFormat.nSamplesPerSec = ((audio->SampleRate*config->FrameRate)
						/ (unsigned long)header->FPS);

				config->AudioRate = audio->DsBuffFormat.nSamplesPerSec;
			} else {
				audio->DsBuffFormat.nSamplesPerSec = audio->SampleRate;
				config->AudioRate = audio->SampleRate;
			}

			audio->DsBuffFormat.wBitsPerSample	= (short) 16;
			audio->DsBuffFormat.nBlockAlign	= (unsigned short)( (audio->DsBuffFormat.wBitsPerSample/8) * audio->DsBuffFormat.nChannels);
			audio->DsBuffFormat.nAvgBytesPerSec= audio->DsBuffFormat.nSamplesPerSec * audio->DsBuffFormat.nBlockAlign;
			audio->DsBuffFormat.cbSize = 0;

			/*
			**  Create the direct sound primary sound buffer object
			*/
			if ( config->SoundObject->CreateSoundBuffer (&audio->BufferDesc ,
																		&config->PrimaryBufferPtr ,
																		NULL ) !=DS_OK ){
				if (audio->CreatedSoundObject){
					config->SoundObject->Release();
					audio->CreatedSoundObject = FALSE;
				}
				return (-1);
			}

			audio->CreatedSoundBuffer = TRUE;
		}
	}

	audio->Flags |= (HMI_VQAINIT << VQAAUDB_DIGIINIT);
	AudioFlags |= (HMI_VQAINIT << VQAAUDB_DIGIINIT);

	return (0);
}


/****************************************************************************
*
* NAME
*     VQA_CloseAudio - Close sound system
*
* SYNOPSIS
*     VQA_CloseAudio()
*
*     void VQA_CloseAudio(void);
*
* FUNCTION
*     Removes VQA's involvement in the audio system.
*
* INPUTS
*     NONE
*
* RESULT
*     NONE
*
****************************************************************************/

void VQA_CloseAudio(VQAHandleP *vqap)
{
	VQAAudio		*audio;
	VQAConfig	*config;

	/* Dereference for quick access. */
	audio	 = &vqap->VQABuf->Audio;
	config = &vqap->Config;

	/*
	** If the audio is still playing then stop it
	*/
	VQA_StopAudio(vqap);

	/* Remove the windows callback event */
	//if ((AudioFlags & VQAAUDF_DIGIINIT) == (HMI_VQAINIT<<VQAAUDB_DIGIINIT)) {
	//	timeKillEvent(audio->TimerHandle);
	//}

	audio->Flags &= ~VQAAUDF_TIMERINIT;
	AudioFlags &= ~VQAAUDF_TIMERINIT;

	/*
	**  Remove the direct sound primary buffer if we created it
	*/
	if (audio->CreatedSoundBuffer){
		config->PrimaryBufferPtr->Stop();
		config->PrimaryBufferPtr->Release();
		config->PrimaryBufferPtr = NULL;
		audio->CreatedSoundBuffer = FALSE;
	}

	/*
	** If we created the sound object then remove that as well
	*/
	if (audio->CreatedSoundObject){
		config->SoundObject->Release();
		config->SoundObject = NULL;
	}

	audio->Flags &= ~VQAAUDF_DIGIINIT;
	AudioFlags &= ~VQAAUDF_DIGIINIT;
	AudioFlags &= ~VQAAUDF_ISPLAYING;

}


/****************************************************************************
*
* NAME
*     VQA_StartAudio - Starts audio playback
*
* SYNOPSIS
*     Error = VQA_StartAudio(VQA)
*
*     long VQA_StartAudio(VQAHandleP *);
*
* FUNCTION
*     Start the audio playback for the movie.
*
* INPUTS
*     VQA - Pointer to private VQA handle.
*
* RESULT
*     Error - 0 if successful, or -1 error code.
*
****************************************************************************/

long VQA_StartAudio(VQAHandleP *vqap)
{
	VQAConfig *config;
	VQAAudio  *audio;

	/* Save buffers for the callback routine */
	VQAP = vqap;

	/* Dereference commonly used data members for quicker access. */
	config = &vqap->Config;
	audio = &vqap->VQABuf->Audio;

	/* Return if already playing */
	if (AudioFlags & VQAAUDF_ISPLAYING) {
		return (-1);
	}

	/*
	** If we already have a direct sound secondary buffer then get rid of it
	*/
	if (audio->SecondaryBufferPtr != NULL){
		audio->SecondaryBufferPtr->Stop();
		audio->SecondaryBufferPtr->Release();
		audio->SecondaryBufferPtr = NULL;
	}
	/*
	** Make it big enough for 4 blocks of HMI data
	*/
	audio->SecondaryBufferSize = config->HMIBufSize*4;

	/*
	** Define the format for the secondary sound buffer
	*/
	memset (&audio->BufferDesc , 0 , sizeof(DSBUFFERDESC));
	audio->BufferDesc.dwSize				= sizeof(DSBUFFERDESC);
	audio->BufferDesc.dwFlags				= DSBCAPS_CTRLVOLUME;
	audio->BufferDesc.dwBufferBytes		= audio->SecondaryBufferSize;
	audio->BufferDesc.lpwfxFormat 		= (LPWAVEFORMATEX) &audio->DsBuffFormat;
	memset (&audio->DsBuffFormat , 0 , sizeof(WAVEFORMATEX));
	audio->DsBuffFormat.wFormatTag		= WAVE_FORMAT_PCM;
	audio->DsBuffFormat.nSamplesPerSec	= audio->SampleRate;
	audio->DsBuffFormat.nChannels			= audio->Channels;
	audio->DsBuffFormat.wBitsPerSample	= audio->BitsPerSample;
	audio->DsBuffFormat.nBlockAlign		= (short) ((audio->DsBuffFormat.wBitsPerSample/8) * audio->DsBuffFormat.nChannels);
	audio->DsBuffFormat.nAvgBytesPerSec	= audio->DsBuffFormat.nSamplesPerSec * audio->DsBuffFormat.nBlockAlign;

	/*
	** Create the secondary sound buffer object
	*/
	config->SoundObject->CreateSoundBuffer (&audio->BufferDesc , &audio->SecondaryBufferPtr , NULL);

	/*
	** Set the format of the primary buffer to the same as the secondary buffer
	*/
	config->PrimaryBufferPtr->Stop();
	config->PrimaryBufferPtr->SetFormat (&audio->DsBuffFormat);

	if (config->PrimaryBufferPtr->Play(0, 0, DSBPLAY_LOOPING) != DS_OK){
		return (-1);
	}

	/* Start playback */
	audio->EndLastAudioChunk = 0;
	audio->ChunksMovedToAudioBuffer = 0;
	Move_HMI_Audio_Block_To_Direct_Sound_Buffer();
	Move_HMI_Audio_Block_To_Direct_Sound_Buffer();
	audio->SecondaryBufferPtr->SetCurrentPosition (0);
	if (audio->SecondaryBufferPtr->Play(0, 0, DSBPLAY_LOOPING) != DS_OK){
		return (-1);
	}

	/*
	** Set the volume
	*/
	long volume = config->Volume << 7;
	audio->SecondaryBufferPtr->SetVolume(- ( ( (32768 - volume)*1000) >>15 ) );

	// Set orf 60hz timer
	audio->TimerHandle = timeSetEvent ( 1000/60 , 1 , AudioCallback , 0 , TIME_PERIODIC);

	audio->Flags |= VQAAUDF_ISPLAYING;
	AudioFlags |= VQAAUDF_ISPLAYING;

	return (0);
}


/****************************************************************************
*
* NAME
*     VQA_StopAudio - Stop audio playback.
*
* SYNOPSIS
*     VQA_StopAudio(VQA)
*
*     void VQA_StopAudio(VQAHandleP *);
*
* FUNCTION
*     Halts the currently playing audio stream.
*
* INPUTS
*     VQA - Pointer to private VQAHandle.
*
* RESULT
*     NONE
*
****************************************************************************/

void VQA_StopAudio(VQAHandleP *vqap)
{
	VQAAudio *audio;
	VQAConfig *config;

	/* Dereference commonly used data members for quicker access. */
	config = &vqap->Config;
	audio = &vqap->VQABuf->Audio;

	/* Just return if not playing */
	if (AudioFlags & VQAAUDF_ISPLAYING) {

		/*
		** Stop the primary buffer so the audio doesnt glitch
		*/
		config->PrimaryBufferPtr->Stop();

		/*
		** Remove the windows timer event
		*/
		timeKillEvent(audio->TimerHandle);
		audio->TimerHandle = NULL;

		/*
		** Kill the secondary sound buffer
		*/
		if (audio->SecondaryBufferPtr){
			audio->SecondaryBufferPtr->Stop();
			audio->SecondaryBufferPtr->Release();
			audio->SecondaryBufferPtr = NULL;
		}
		audio->Flags &= ~VQAAUDF_ISPLAYING;
		AudioFlags &= ~VQAAUDF_ISPLAYING;
	}

	VQAP = NULL;
}


/****************************************************************************
*
* NAME
*     CopyAudio - Copy data from Audio Temp buffer into Audio play buffer.
*
* SYNOPSIS
*     Error = CopyAudio(VQA)
*
*     long CopyAudio(VQAHandleP *);
*
* FUNCTION
*     This routine just copies the data in the TempBuf into the correct
*     spots in the audio play buffer.  If there is no room available in the
*     audio play buffer, the routine returns VQAERR_SLEEPING, which will put
*     the whole Loader to "sleep" while it waits for a free buffer.
*
*     If there's no data in the TempBuf to copy, the routine just returns 0.
*
* INPUTS
*     VQA - Pointer to private VQAHandle structure.
*
* RESULT
*     Error - 0 if successful or VQAERR_??? error code.
*
****************************************************************************/

long CopyAudio(VQAHandleP *vqap)
{
	VQAAudio  *audio;
	VQAConfig *config;
	long      startblock;
	long      endblock;
	long      len1,len2;
	long      i;

	/* Dereference commonly used data members for quicker access. */
	audio = &vqap->VQABuf->Audio;
	config = &vqap->Config;

	/* If audio is disabled, or if we're playing from a VOC file, or if
	 * there's no Audio Buffer, or if there's no data to copy, just return 0
	 */
	#if(VQAVOC_ON && VQAAUDIO_ON)
	if (((config->OptionFlags & VQAOPTF_AUDIO) == 0) || (vqap->vocfh != -1)
			|| (audio->Buffer == NULL) || (audio->TempBufLen == 0)) {
	#else  /* VQAVOC_ON */
	if (((config->OptionFlags & VQAOPTF_AUDIO) == 0) || (audio->Buffer == NULL)
			|| (audio->TempBufLen == 0)) {
	#endif /* VQAVOC_ON */

		return (0);
	}

	/* Compute start & end blocks to copy into */
	startblock = (audio->AudBufPos / config->HMIBufSize);
	endblock = (audio->AudBufPos + audio->TempBufLen) / config->HMIBufSize;

	if (endblock >= audio->NumAudBlocks) {
		endblock -= audio->NumAudBlocks;
	}

	/* If 'endblock' hasn't played yet, return VQAERR_SLEEPING */
	if (audio->IsLoaded[endblock] == 1) {
		return (VQAERR_SLEEPING);
	}

	/* Copy the data:
	 *
	 *  - If 'startblock' < 'endblock', copy the entire buffer
	 *  - Otherwise, fill to the end of the buffer with part of the data, then
	 *    copy the rest to the beginning of the buffer
	 */
	if (startblock <= endblock) {

		/* Copy data */
		memcpy((audio->Buffer + audio->AudBufPos), audio->TempBuf,
				audio->TempBufLen);

		/* Adjust current load position */
		audio->AudBufPos += audio->TempBufLen;

		/* Mark buffer as empty */
		audio->TempBufLen = 0;

		/* Set all blocks to loaded */
		for (i = startblock; i < endblock; i++) {
			audio->IsLoaded[i] = 1;
		}

		return (0);
	} else {

		/* Compute length of each piece */
		len1 = config->AudioBufSize - audio->AudBufPos;
		len2 = audio->TempBufLen - len1;

		/* Copy 1st piece into end of Audio Buffer */
		memcpy((audio->Buffer + audio->AudBufPos), audio->TempBuf, len1);

		/* Copy 2nd piece into start of Audio Buffer */
		memcpy(audio->Buffer, audio->TempBuf + len1, len2);

		/* Adjust load position */
		audio->AudBufPos = len2;

		/* Mark buffer as empty */
		audio->TempBufLen = 0;

		/* Set blocks to loaded */
		for (i = startblock; i < audio->NumAudBlocks; i++) {
			audio->IsLoaded[i] = 1;
		}

		for (i = 0; i < endblock; i++) {
			audio->IsLoaded[i] = 1;
		}

		return (0);
	}
}


BOOL	VQAAudioPaused = FALSE;

void VQA_PauseAudio(void)
{
	VQAAudio  	*audio;
	if (VQAP && VQAP->VQABuf){

		audio = &VQAP->VQABuf->Audio;

		if (audio->SecondaryBufferPtr){

			if (AudioFlags & VQAAUDF_ISPLAYING && !VQAAudioPaused) {

				audio->SecondaryBufferPtr->Stop();
				VQAAudioPaused = TRUE;
			}
		}
	}
}


void VQA_ResumeAudio(void)
{
	VQAAudio  	*audio;
	if (VQAP && VQAP->VQABuf){

		audio = &VQAP->VQABuf->Audio;

		if (audio->SecondaryBufferPtr){

			if (AudioFlags & VQAAUDF_ISPLAYING && VQAAudioPaused) {

				audio->SecondaryBufferPtr->SetCurrentPosition (0);
				audio->LastChunkPosition = 0;
				audio->EndLastAudioChunk = 0;
				Move_HMI_Audio_Block_To_Direct_Sound_Buffer();
				audio->SecondaryBufferPtr->Play(0,0,DSBPLAY_LOOPING);
				VQAAudioPaused = FALSE;
			}
		}
	}
}






/****************************************************************************
*
* NAME
*     AudioCallback - Sound system callback.
*
* SYNOPSIS
*     AudioCallback(DriverHandle, Action, SampleID)
*
*     void AudioCallback(WORD, WORD, WORD);
*
* FUNCTION
*     Our custom audio callback routine that services HMI.
*
* INPUTS
*     DriverHandle - HMI driver handle.
*     Action       - Action taken.
*     SampleID     - ID of sample.
*
* RESULT
*     NONE
*
****************************************************************************/

void CALLBACK AudioCallback ( UINT, UINT, DWORD, DWORD, DWORD )

{
	VQAAudio  	*audio;
	VQAConfig 	*config;
	DWORD			play_cursor;		//Position that direct sound is reading from
	DWORD			write_cursor;		//Position in buffer that we can write to
	HRESULT		return_code;
	BOOL			buffer_stopped = FALSE;
	DWORD			status;


	if (SuspendAudioCallback || VQAAudioPaused) return;

	/* Dereference commonly used data members for quicker access. */
	audio = &VQAP->VQABuf->Audio;
	config = &VQAP->Config;

	if (!audio->SecondaryBufferPtr) return;

	/*
	** See if we are nearing the end of the meaningful data in the direct sound buffer
	*/
	return_code = audio->SecondaryBufferPtr->GetCurrentPosition (&play_cursor , &write_cursor);

	BOOL write_more = FALSE;

	if (return_code == DSERR_BUFFERLOST || config->PrimaryBufferPtr->GetStatus(&status) == DSERR_BUFFERLOST){
		config->PrimaryBufferPtr->Restore();
		audio->SecondaryBufferPtr->Restore();
		audio->SecondaryBufferPtr->Stop();
		buffer_stopped = TRUE;
		audio->SecondaryBufferPtr->SetCurrentPosition (0);
		audio->LastChunkPosition = 0;
		audio->EndLastAudioChunk = 0;
		write_more = TRUE;
	}


	if (play_cursor < audio->EndLastAudioChunk){
		if (audio->EndLastAudioChunk - play_cursor <= audio->SecondaryBufferSize/4){
			write_more = TRUE;
		}
	}else{
		if (( play_cursor > audio->SecondaryBufferSize*3/4) && audio->EndLastAudioChunk==0 ){
			write_more = TRUE;
		}
	}

	/*
	** See if we need to fill the buffer
	*/
	if (write_more){
		if (Move_HMI_Audio_Block_To_Direct_Sound_Buffer()){

			/*
			** Start the buffer playing again if we had to stop it
			*/
			if (buffer_stopped){
				audio->SecondaryBufferPtr->Play(0,0,DSBPLAY_LOOPING);
			}
		}
	}
}









/***********************************************************************************************
 * Move_HMI_Audio_Block_To_Direct_Sound_Buffer -- moves an audio block which would have been   *
 *                                                played by HMI into a direct sound            *
 *                                                secondary buffer                             *
 *                                                                                             *
 * INPUT:    Nothing                                                                           *
 *                                                                                             *
 * OUTPUT:   BOOL was block moved                                                              *
 *                                                                                             *
 * WARNINGS: None                                                                              *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *    12/21/95 9:51AM ST : Created                                                              *
 *=============================================================================================*/
extern int VQAMovieDone;
BOOL Move_HMI_Audio_Block_To_Direct_Sound_Buffer (void)
{
	VQAAudio  	*audio;
	VQAConfig 	*config;

	LPVOID		play_buffer_ptr;	//Beginning of locked area of buffer
	LPVOID		dummy_buffer_ptr;	//Length of locked area in buffer
	DWORD			lock_length1;		//Beginning of second locked area in buffer
	DWORD			lock_length2;		//Length of second locked area in buffer
	unsigned		next_fill_pos;
	HRESULT		return_code;
	DWORD			status;


	/* Dereference commonly used data members for quicker access. */
	audio = &VQAP->VQABuf->Audio;
	config = &VQAP->Config;


	/*************************************************************************
	**
	** Copy the data from the HMI play position into the direct sound buffer
	**
	*/
	next_fill_pos = audio->EndLastAudioChunk;

	/*
	** Lock the buffer to get a pointer to it
	*/
	return_code= audio->SecondaryBufferPtr->Lock ((DWORD)next_fill_pos,
																	(DWORD)config->HMIBufSize,
																	&play_buffer_ptr,
																	&lock_length1,
																	&dummy_buffer_ptr,
																	&lock_length2,
																	0 );

	if (return_code!=DS_OK) return(FALSE);
	if (config->PrimaryBufferPtr->GetStatus(&status) == DSERR_BUFFERLOST) return(FALSE);

	/*
	** Copy the HMI audio buffer to the direct sound buffer
	*/
	memcpy ((char*)play_buffer_ptr , (char*)((unsigned)audio->Buffer + (unsigned)audio->PlayPosition) , config->HMIBufSize);

	/*
	** Unlock the direct sound buffer
	*/
	audio->SecondaryBufferPtr->Unlock(play_buffer_ptr,
												lock_length1,
												dummy_buffer_ptr,
												lock_length2);
	/*
	** Update our audio data pointers
	*/
	audio->LastChunkPosition = next_fill_pos;
	audio->EndLastAudioChunk = next_fill_pos + config->HMIBufSize;
	if (audio->EndLastAudioChunk >= audio->SecondaryBufferSize){
		audio->EndLastAudioChunk = 0;
	}

	/* Compute the 'NextBlock' index */
	audio->NextBlock = audio->CurBlock + 1;

	if (audio->NextBlock >= audio->NumAudBlocks) {
		audio->NextBlock = 0;
	}

	/* See if the next block has data in it; if so, update the audio
		* buffer play position & the 'CurBlock' value.
		* If not, don't change anything and replay this block.
		*/
	if (audio->IsLoaded[audio->NextBlock] == 1) {

		/* Update this block's status to loadable (0) */
		audio->IsLoaded[audio->CurBlock] = 0;

		/* Update position within audio buffer */
		audio->PlayPosition += config->HMIBufSize;
		audio->CurBlock++;

		if (audio->PlayPosition >= config->AudioBufSize) {
			audio->PlayPosition = 0;
			audio->CurBlock = 0;
		}
		audio->ChunksMovedToAudioBuffer++;
		return (TRUE);
	} else {
		if (VQAMovieDone){
			audio->ChunksMovedToAudioBuffer++;
		}
		audio->NumSkipped++;
		/*
		** Enable frame skipping to prevent this happening again
		*/
		config->DrawFlags &= ~VQACFGF_NOSKIP;
		return (FALSE);
	}
}





#endif	//!VQADIRECT_SOUND


#endif /* VQAAUDIO_ON */












/****************************************************************************
*
* NAME
*     VQA_SetTimer - Resets current time to given tick value.
*
* SYNOPSIS
*     VQA_SetTimer(Time, Method)
*
*     void VQA_SetTimer(long, long);
*
* FUNCTION
*     Sets 'TickOffset' to a value that will make the current time look like
*     the time passed in. This function allows the player to be "paused",
*     by recording the time of the pause, and then setting the timer to
*     that time. The timer method used by the player is also set. The method
*     selected is not neccesarily the method that will be used because some
*     timer methods work with only certain playback conditions. (EX: The
*     audio DMA timer method cannot be used if there is not any audio
*     playing.)
*
* INPUTS
*     Time   - Value to set current time to.
*     Method - Timer method to use.
*
* RESULT
*     NONE
*
****************************************************************************/

void VQA_SetTimer(VQAHandleP *vqap, long time, long method)
{
	unsigned long curtime;

	#if(VQAAUDIO_ON)
	VQAAudio      *audio;

	/* Dereference for quick access. */
	audio = &vqap->VQABuf->Audio;

	/* If the client does not have a preferencee then pick a method
	 * based on the state of the player.
	 */
	if (method == VQA_TMETHOD_DEFAULT) {

		/* If we are playing audio, use the audio DMA position. */
		if (AudioFlags & VQAAUDF_ISPLAYING) {
			method = VQA_TMETHOD_AUDIO;
		}

		/* Otherwise use the HMI timer if it is initialized. */
		else if (AudioFlags & VQAAUDF_HMITIMER) {
			method = VQA_TMETHOD_INT;
		}

		/* If all else fails resort the the "jerky" DOS time. */
		else {
			method = VQA_TMETHOD_DOS;
		}
	} else {

		/* We cannot use the DMA position if there isn't any audio playing. */
		if (!(AudioFlags & VQAAUDF_ISPLAYING) && (method == VQA_TMETHOD_AUDIO)) {
			method = VQA_TMETHOD_INT;
		}

		/* We cannot use the timer if it has not been initialized. */
		if (!(AudioFlags & VQAAUDF_HMITIMER) && (method == VQA_TMETHOD_INT)) {
			method = VQA_TMETHOD_DOS;
		}
	}

	TimerMethod = method;
	#else
	method = method;
	#endif

	TickOffset = 0L;
	curtime = VQA_GetTime(vqap);
	TickOffset = (time - curtime);
}


/****************************************************************************
*
* NAME
*     VQA_GetTime - Return current time.
*
* SYNOPSIS
*     Time = VQA_GetTime()
*
*     unsigned long VQA_GetTime(void);
*
* FUNCTION
*     This routine returns timer ticks computed one of 3 ways:
*
*     1) If audio is playing, the timer is based on the DMA buffer position:
*        Compute the number of audio samples that have actually been played.
*        The following internal HMI variables are used:
*
*          _lpSOSDMAFillCount[drv_handle]: current DMA buffer position
*          _lpSOSSampleList[drv_handle][samp_handle]:
*          sampleTotalBytes: total bytes sent by HMI to the DMA buffer
*          sampleLastFill: HMI's last fill position in DMA buffer
*
*        So, the number of samples actually played is:
*
*          sampleTotalBytes - <DMA_diff>
*          where <DMA_diff> is how far ahead sampleLastFill is in front of
*          _lpSOSDMAFillCount: (sampleLastFill - _lpSOSDMAFillCount)
*
*        These values are indices into a circular DMA buffer, so:
*
*          if (sampleLastFill >= _lpSOSDMAFillCount)
*            <DMA_diff> = sampleLastFill - _lpSOSDMAFillCount
*          else
*            <DMA_diff> = (DMA_BUF_SIZE - lpSOSDMAFillCount) + sampleLastFill
*
*        Note that, if using the stereo driver with mono data, you must
*        divide LastFill & FillCount by 2, but not TotalBytes. If using the
*        stereo driver with stereo data, you must divide all 3 variables
*        by 2.
*
*     2) If no audio is playing, but the timer interrupt is running,
*        VQATickCount is used as the timer
*
*     3) If no audio is playing & no timer interrupt is going, the DOS 18.2
*        system timer is used.
*
*     Regardless of the method, TickOffset is used as an offset from the
*     computed time.
*
* INPUTS
*     NONE
*
* RESULT
*     Time - Time in VQA_TIMETICKS
*
****************************************************************************/
unsigned long VQA_GetTime(VQAHandleP *vqap)
{
	#if(VQAAUDIO_ON)
	VQAAudio      *audio;
	VQAConfig     *config;
	unsigned long fillcount;
	unsigned long lastfill;
	unsigned long dma_diff;
	unsigned long totalbytes;
	unsigned long samples;
	uint32_t				play_cursor;		//Position that direct sound is reading from
	uint32_t				write_cursor;		//Position in buffer that we can write to
	unsigned			temp;
	#endif

	// MEG 09.25.95 - changed from long to unsigned long
	unsigned long ticks;

	#if(VQAAUDIO_ON)
	switch (TimerMethod) {

		/* If Audio is playing then timing is based on the audio DMA buffer
		 * position.
		 */
		case VQA_TMETHOD_AUDIO:

			/* Dereference commonly used data members for quicker access. */
			audio = &vqap->VQABuf->Audio;
			config = &vqap->Config;
			//vqabuf = ((VQAHandleP *)vqa)->VQABuf;

			#if VQASDL_SOUND
			SDL_LockAudioDevice(vqap->Config.AudioDeviceID);
			totalbytes = (audio->ChunksMovedToAudioBuffer) * config->HMIBufSize;

			// offset by any bytes still in the stream
			// there will still be samples in the "hardware" queue, but this is the best we can do
			play_cursor = SDL_AudioStreamAvailable(SDLStream);
			totalbytes -= (play_cursor * StreamConvScale) >> 15;
			SDL_UnlockAudioDevice(vqap->Config.AudioDeviceID);

			samples = totalbytes/audio->Channels;
			samples = samples/(audio->BitsPerSample >> 3);

			#elif VQAPICO_SOUND

			totalbytes = (audio->ChunksMovedToAudioBuffer) * config->HMIBufSize + audio->BlockOffset;

			samples = totalbytes/audio->Channels;
			samples = samples/(audio->BitsPerSample >> 3);

			#elif (VQADIRECT_SOUND)

				totalbytes = (audio->ChunksMovedToAudioBuffer) * config->HMIBufSize;

			if (audio->SecondaryBufferPtr &&
				audio->SecondaryBufferPtr->GetCurrentPosition (&play_cursor , &write_cursor) == DS_OK){
				totalbytes = (audio->ChunksMovedToAudioBuffer) * config->HMIBufSize;

				if (audio->LastChunkPosition){
					totalbytes += play_cursor - audio->LastChunkPosition;
				}else {
					if (play_cursor > audio->SecondaryBufferSize*3/4){
						totalbytes -= audio->SecondaryBufferSize - play_cursor;
					} else {
						totalbytes += play_cursor - audio->LastChunkPosition;
					}
				}


			}else{
				totalbytes = (audio->ChunksMovedToAudioBuffer-1) * config->HMIBufSize;
			}


			samples = totalbytes/audio->DsBuffFormat.nChannels;
			samples = samples/(audio->DsBuffFormat.wBitsPerSample >> 3);

			#else		//VQADIRECT_SOUND

			/* Compute our current position in the audio track by getting the
			 * bytes processed by HMI. Then adjust the bytes processed by the
			 * position of the DMA fill handler, this should tell us exactly
			 * where we are in the audio track.
			 */
			fillcount = (unsigned long)(*_lpSOSDMAFillCount[audio->DigiHandle]);
			lastfill = _lpSOSSampleList[audio->DigiHandle][audio->SampleHandle]->sampleLastFill;
			totalbytes = _lpSOSSampleList[audio->DigiHandle][audio->SampleHandle]->sampleTotalBytes;

			if (totalbytes == 0) {
				dma_diff = 0;
			} else {
				if (lastfill > fillcount) {
					dma_diff = lastfill - fillcount;
				} else {
					dma_diff = (config->HMIBufSize - fillcount) + lastfill;
				}

				if (dma_diff > totalbytes) {
					dma_diff = totalbytes;
				}
			}

			/* Calculate the number of samples by taking the total number of
			 * bytes processed and divide it by the number of channels and
			 * bits per sample.
			 */
			samples = totalbytes - dma_diff;
			samples -= (audio->NumSkipped * config->HMIBufSize);
			samples /= (audio->Channels * (audio->BitsPerSample >> 3));

			#endif	//VQADIRECT_SOUND

			/* The elapsed ticks is calculated by the number of samples
			 * processed times the tick resolution per second divided by the
			 * sample rate.
			 */
			ticks = (long)((samples * VQA_TIMETICKS) / audio->SampleRate);
			ticks += TickOffset;
			break;

		/* No audio playing, but timer interrupt is going; use VQATickCount */
		case VQA_TMETHOD_INT:
			ticks = (VQATickCount + TickOffset);
			break;

		/* No interrupts are going at all; use DOS's time */
		default:
		case VQA_TMETHOD_DOS:
			{
#ifdef PICO_BUILD
			ticks = to_ms_since_boot(get_absolute_time());
#elif defined(ESP_BUILD)
			ticks = esp_timer_get_time() / 1000;
#else
			struct timeb mytime;
			ftime(&mytime);
 			ticks = (unsigned long)mytime.time*1000L+(unsigned long)mytime.millitm;
#endif
			ticks = ((ticks * VQA_TIMETICKS) / 1000L);
			ticks += TickOffset;
			}
			break;
	}
	#else
	{
	// MEG 09.23.95 - Use Windows timer.
#if( ! USE_WINDOWS_TIME )

	struct timeb mytime;

	ftime(&mytime);
	ticks = (unsigned long)mytime.time*1000L+(unsigned long)mytime.millitm;

#else

	ticks = Get_Game_Time();

#endif

//	ticks = ((ticks * VQA_TIMETICKS) / 1000L);

	// MEG 09.25.95 - multiply by 3, divide by 50 instead of multiplying by
	// 60 and dividing by 1000.  The reason for this is that multiplying by
	// 60 causes an overflow pretty quickly if the value from Get_Game_Time
	// is high.
	ticks = ((ticks * 3L) / 50L);

	ticks += TickOffset;
	}
	#endif

	return (ticks);
}


/****************************************************************************
*
* NAME
*     VQA_TimerMethod - Get timer method being used.
*
* SYNOPSIS
*     Method = VQA_TimerMethod()
*
*     long VQA_TimerMethod(void);
*
* FUNCTION
*     Returns the ID of the current timer method being used.
*
* INPUTS
*     NONE
*
* RESULT
*     Method - Method used for the timer.
*
****************************************************************************/

long VQA_TimerMethod(void)
{
	#if(VQAAUDIO_ON)
	return (TimerMethod);
	#else
	return (VQA_TMETHOD_DOS);
	#endif
}

#ifdef __WATCOMC__
#pragma pack(1);
#endif
