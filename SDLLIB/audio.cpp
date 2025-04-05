#include <algorithm>
#include <cassert>
#include <cstring>

#include <SDL.h>

#include "audio.h"
#include "file.h"

// original code has 5 for windows, 4 for dos
// effectively one less as one is used to track streaming from disk
#define	MAX_SFX	4

enum SCompressType : uint8_t
{
	SCOMP_NONE=0,			// No compression -- raw data.
	SCOMP_WESTWOOD=1,		// Special sliding window delta compression.
	SCOMP_SONARC=33,		// Sonarc frame compression.
	SCOMP_SOS=99			// SOS frame compression.
};

static const int8_t ima_adpcm_index_table[] = {
    -1, -1, -1, -1, 2, 4, 6, 8,
    -1, -1, -1, -1, 2, 4, 6, 8
};

static const int16_t ima_adpcm_step_table[89] = { 
    7, 8, 9, 10, 11, 12, 13, 14, 16, 17, 
    19, 21, 23, 25, 28, 31, 34, 37, 41, 45, 
    50, 55, 60, 66, 73, 80, 88, 97, 107, 118, 
    130, 143, 157, 173, 190, 209, 230, 253, 279, 307,
    337, 371, 408, 449, 494, 544, 598, 658, 724, 796,
    876, 963, 1060, 1166, 1282, 1411, 1552, 1707, 1878, 2066, 
    2272, 2499, 2749, 3024, 3327, 3660, 4026, 4428, 4871, 5358,
    5894, 6484, 7132, 7845, 8630, 9493, 10442, 11487, 12635, 13899, 
    15289, 16818, 18500, 20350, 22385, 24623, 27086, 29794, 32767 
};

SFX_Type SoundType;
Sample_Type SampleType;

int StreamLowImpact;

static int ScoreVolume = 255;

static SDL_AudioDeviceID AudioDevice;
static SDL_AudioSpec ObtainedSpec;
static uint8_t *MixBuffer; // temp buffer for mixing
static AudioCallback ExtraCallback = NULL;

struct ChannelState
{
    const void *sample = NULL;
    SDL_AudioStream *stream = NULL;
    bool playing = false;
    int priority = 0;
    int16_t volume = 32767;

    int raw_volume; // input local * global vol
    int fade = 0;
    
    uint8_t channels = 0;
    uint8_t bits = 0;
    uint16_t sample_rate = 0;

    uint32_t offset = 0;
    uint32_t length = 0;
    uint8_t *in_ptr = NULL;

    int file_handle = -1; // if this is a file stream

    // compression state
    SCompressType compression = SCOMP_NONE;
    int8_t step = 0;
    int16_t predictor = 0;
} Channels[MAX_SFX];

static int Calculate_Volume(int vol)
{
    // TODO: improve?
    return vol * (32767) / (255 * 255);
}

static uint8_t *DecodeADPCMBlock(ChannelState &chan, int block_size, uint8_t *in_ptr)
{
    auto clamp = [](int v, int min, int max)
    {
        return v < min ? min : (v > max ? max : v);
    };

    for(int i = 0; i < block_size; i++)
    {
        int16_t samples[2];
        auto b = *in_ptr++;

        int nibble = b & 0xF;
        int step = ima_adpcm_step_table[chan.step];
        chan.step = clamp(chan.step + ima_adpcm_index_table[nibble], 0, 88);

        int diff = ((((nibble & 7) * 2 + 1) * step) >> 3) * (nibble & 8 ? -1 : 1);
        chan.predictor = clamp(chan.predictor + diff, -32768, 32767);

        samples[0] = chan.predictor;

        nibble = b >> 4;
        step = ima_adpcm_step_table[chan.step];
        chan.step = clamp(chan.step + ima_adpcm_index_table[nibble], 0, 88);

        diff = ((((nibble & 7) * 2 + 1) * step) >> 3) * (nibble & 8 ? -1 : 1);
        chan.predictor = clamp(chan.predictor + diff, -32768, 32767);

        samples[1] = chan.predictor;

        SDL_AudioStreamPut(chan.stream, samples, sizeof(samples));
    }

    return in_ptr;
}

static bool RefillStream(ChannelState &chan)
{
    uint32_t max_update = ObtainedSpec.samples; // assume the target rate is not lower

    if(chan.offset == chan.length)
        return false;

    if(chan.compression == SCOMP_SOS)
    {
        // ADPCM
        int samples_to_gen = std::min(max_update, chan.length - chan.offset);
        // read blocks until we have enough samples
        while(samples_to_gen > 0)
        {
            // read a block
            uint16_t block_in_size = *(uint16_t *)chan.in_ptr;
            uint16_t block_out_size = *(uint16_t *)(chan.in_ptr + 2);
            chan.in_ptr += 8; // there's also a 0000DEAF magic value

            // assert(block_out_size == block_in_size * 4);

            chan.in_ptr = DecodeADPCMBlock(chan, block_in_size, chan.in_ptr);

            chan.offset += block_out_size / 2;
            samples_to_gen -= block_out_size / 2;
        }
    }
    else
        return false; // shouldn't happen, but...

    // written all data, flush the stream
    if(chan.offset == chan.length)
        SDL_AudioStreamFlush(chan.stream);

    return true;
}

static void ResetStream(ChannelState &chan, AUDHeaderType *header)
{
    int channels = header->Flags & AUD_FLAG_STEREO ? 2 : 1;
    int bits = header->Flags & AUD_FLAG_16BIT ? 16 : 8;

    // re-allocate stream if needed
    if(channels != chan.channels || bits != chan.bits || header->Rate != chan.sample_rate)
    {
        if(chan.stream)
            SDL_FreeAudioStream(chan.stream);

        chan.stream = SDL_NewAudioStream(bits == 16 ? AUDIO_S16 : AUDIO_S8, channels, header->Rate, ObtainedSpec.format, ObtainedSpec.channels, ObtainedSpec.freq);
    }
    else
        SDL_AudioStreamClear(chan.stream);
}

static void SDL_Audio_Callback(void *userdata, Uint8 *stream, int len)
{
    int samples = len / sizeof(int16_t);
    memset(stream, 0, len);
    auto stream16 = (int16_t *)stream;

    // let VQA do its thing
    if(ExtraCallback)
        ExtraCallback(stream, len);

    for(auto &chan : Channels)
    {
        if(!chan.playing)
            continue;

        // put more data into stream if needed
        // unless it's a file, we do that elsewhere
        if(SDL_AudioStreamAvailable(chan.stream) < len && chan.in_ptr)
        {
            if(!RefillStream(chan) && !SDL_AudioStreamAvailable(chan.stream))
            {
                // no more data, it's finished
                chan.playing = false;
                continue;
            }
        }
        else if(!SDL_AudioStreamAvailable(chan.stream) && !chan.in_ptr && chan.file_handle == -1)
        {
            // if there's no data, pointer or file handle, this is a finished file stream
            chan.playing = false;
            continue;
        }

        if(chan.fade)
        {
            // update fade
            chan.raw_volume -= chan.fade;
            if(chan.raw_volume <= 0)
            {
                chan.playing = false;
                break;
            }
            chan.volume = Calculate_Volume(chan.raw_volume);
        }

        int stream_len = SDL_AudioStreamGet(chan.stream, MixBuffer, len);

        // mix into buffer
        auto mix16 = (int16_t *)MixBuffer;
        for(int s = 0; s < stream_len / sizeof(int16_t); s++)
            stream16[s] += (mix16[s] * chan.volume) >> 15;
    }
}

int File_Stream_Sample_Vol(char const *filename, int volume, bool real_time_start)
{
    int id = Get_Free_Sample_Handle(0xFF);

    if(id == -1)
        return -1;

    // try to open file and get header
    int handle = Open_File(filename, READ);

    if(handle < 0)
        return -1;

    AUDHeaderType header;

    if(Read_File(handle, &header, sizeof(header)) != sizeof(header))
    {
        Close_File(handle);
        return -1;
    }

    int channels = header.Flags & 1 ? 2 : 1;
    int bits = header.Flags & 2 ? 16 : 8;

    if(header.Compression != SCOMP_SOS || channels != 1 || bits != 16)
    {
        Close_File(handle);
        printf("\trate %i size %i/%i channels %i bits %i comp %i\n", header.Rate, header.Size, header.UncompSize, channels, bits, header.Compression);
        return -1;
    }

    // setup channel
    SDL_LockAudioDevice(AudioDevice);
    auto &chan = Channels[id];

    chan.sample = NULL;
    chan.playing = true;
    chan.priority = 0xFF;
    chan.raw_volume = volume * ScoreVolume;
    chan.volume = Calculate_Volume(chan.raw_volume);
    chan.fade = 0;

    ResetStream(chan, &header);

    chan.channels = channels;
    chan.bits = bits;
    chan.sample_rate = header.Rate;

    chan.offset = 0;
    chan.length = header.UncompSize / channels / (bits / 8);
    chan.in_ptr = NULL;

    chan.file_handle = handle;

    chan.compression = (SCompressType)header.Compression;

    if(chan.compression == SCOMP_SOS)
    {
        chan.step = 0;
        chan.predictor = 0;
    }

    SDL_UnlockAudioDevice(AudioDevice);

    return id;
}

void Sound_Callback(void)
{
    // update file stream
    for(auto &chan : Channels)
    {
        if(chan.file_handle == -1)
            continue;

        if(!chan.playing)
        {
            // clean up file
            // (may have stopped playing due to fade)
            Close_File(chan.file_handle);
            chan.file_handle = -1;
            continue;
        }

        // limit how much we buffer so we don't end up with the whole file
        // (not that it's a problem on any modern system, but still)
        int max_buf = (SDL_AUDIO_BITSIZE(ObtainedSpec.format) / 8) * ObtainedSpec.channels * ObtainedSpec.freq;
        if(SDL_AudioStreamAvailable(chan.stream) >= max_buf)
            continue;
        
        uint16_t block_header[4];
        if(Read_File(chan.file_handle, block_header, 8) !=8)
        {
            // must be eof

            SDL_LockAudioDevice(AudioDevice);
            SDL_AudioStreamFlush(chan.stream);
            SDL_UnlockAudioDevice(AudioDevice);

            Close_File(chan.file_handle);
            chan.file_handle = -1;
        }
        else
        {
            // read block
            auto in_size = block_header[0];
            auto out_size = block_header[1];

            uint8_t *buf = new uint8_t[in_size];

            Read_File(chan.file_handle, buf, in_size);

            SDL_LockAudioDevice(AudioDevice);
            DecodeADPCMBlock(chan, in_size, buf);

            delete[] buf;

            SDL_UnlockAudioDevice(AudioDevice);
        }
    }
}

bool Audio_Init(void * window, int bits_per_sample, bool stereo, int rate, int reverse_channels)
{
    SDL_AudioSpec desired;
    desired.freq = rate;
    desired.format = AUDIO_S16; //bits_per_sample == 16 ? AUDIO_S16 : AUDIO_S8;
    desired.channels = stereo ? 2 : 1;
    desired.samples = 2048;
    desired.callback = SDL_Audio_Callback;

    // don't allow format change so I need less mising code
    int changes = SDL_AUDIO_ALLOW_FREQUENCY_CHANGE | SDL_AUDIO_ALLOW_CHANNELS_CHANGE | SDL_AUDIO_ALLOW_SAMPLES_CHANGE;
    AudioDevice = SDL_OpenAudioDevice(NULL, false, &desired, &ObtainedSpec, changes);

    if(!AudioDevice)
    {
        printf("Audio_Init: %s\n", SDL_GetError());
        return false;
    }

    MixBuffer = new uint8_t[ObtainedSpec.size];

    SDL_PauseAudioDevice(AudioDevice, false);

    SoundType = SFX_SDL;
    SampleType = SAMPLE_SDL;
    return true;
}

void Sound_End(void)
{
    SDL_CloseAudioDevice(AudioDevice);

    delete[] MixBuffer;

    for(auto &chan : Channels)
    {
        SDL_FreeAudioStream(chan.stream);
    }
}

void Stop_Sample(int handle)
{
    if(handle < 0 || handle >= MAX_SFX)
        return;

    SDL_LockAudioDevice(AudioDevice);

    Channels[handle].playing = false;

    SDL_UnlockAudioDevice(AudioDevice);

    if(Channels[handle].file_handle != -1)
    {
        Close_File(Channels[handle].file_handle);
        Channels[handle].file_handle = -1;
    }
}

bool Sample_Status(int handle)
{
    if(handle < 0)
        return false;
    return Channels[handle].playing;
}

bool Is_Sample_Playing(void const * sample)
{
    for(int i = 0; i < MAX_SFX; i++)
    {
        if(Channels[i].sample == sample && Sample_Status(i))
            return true;
    }

    return false;
}

void Stop_Sample_Playing(void const * sample)
{
    for(int i = 0; i < MAX_SFX; i++)
    {
        if(Channels[i].sample == sample)
            return Stop_Sample(i);
    }
}

int Play_Sample(void const *sample, int priority, int volume, signed short panloc)
{
    return Play_Sample_Handle(sample, priority, volume, panloc, Get_Free_Sample_Handle(priority));
}

int Play_Sample_Handle(void const *sample, int priority, int volume, signed short panloc, int id)
{
    if(id == -1 || !sample)
        return -1;

    // play it
    auto header = (AUDHeaderType *)sample;
    int channels = header->Flags & AUD_FLAG_STEREO ? 2 : 1;
    int bits = header->Flags & AUD_FLAG_16BIT ? 16 : 8;

    if(header->Compression != SCOMP_SOS || channels != 1 || bits != 16)
    {
        printf("\trate %i size %i/%i channels %i bits %i comp %i\n", header->Rate, header->Size, header->UncompSize, channels, bits, header->Compression);
        return -1;
    }

    // setup channel
    SDL_LockAudioDevice(AudioDevice);
    auto &chan = Channels[id];

    chan.sample = sample;
    chan.playing = true;
    chan.priority = priority;
    chan.raw_volume = volume * 255;
    chan.volume = Calculate_Volume(chan.raw_volume);
    chan.fade = 0;

    ResetStream(chan, header);

    chan.channels = channels;
    chan.bits = bits;
    chan.sample_rate = header->Rate;

    chan.offset = 0;
    chan.length = header->UncompSize / channels / (bits / 8);
    chan.in_ptr = (uint8_t *)sample + sizeof(AUDHeaderType);

    chan.compression = (SCompressType)header->Compression;

    if(chan.compression == SCOMP_SOS)
    {
        chan.step = 0;
        chan.predictor = 0;
    }

    SDL_UnlockAudioDevice(AudioDevice);

    return id;
}

int Set_Score_Vol(int volume)
{
    int old = ScoreVolume;
    ScoreVolume = volume;

    for(auto &chan : Channels)
    {
        if(chan.playing && !chan.in_ptr) // score is a file stream
        {
            if(ScoreVolume)
            {
                chan.raw_volume = chan.raw_volume / old * ScoreVolume;
                chan.volume = Calculate_Volume(chan.raw_volume);
            }
            else
                chan.playing = false;
        }
    }

    return old;
}

void Fade_Sample(int handle, int ticks)
{
    // recalse from game ticks, to audio callbacks
    int fade_time = (1000 / 60) * ticks;
    int callback_interval =  ObtainedSpec.samples * 1000 / ObtainedSpec.freq;

    int num_steps = fade_time / callback_interval;

    if(Sample_Status(handle))
    {
        SDL_LockAudioDevice(AudioDevice);
        Channels[handle].fade = Channels[handle].raw_volume / num_steps;
        SDL_UnlockAudioDevice(AudioDevice);
    }
}

int Get_Free_Sample_Handle(int priority)
{
    // find free channel
    int id;
    for(id = MAX_SFX - 1; id >= 0; id--)
    {
        if(!Channels[id].playing)
            break;
    }

    if(id < 0)
    {
        // look for lower priority channel instead
        for(id = 0; id < MAX_SFX; id++)
        {
            if(Channels[id].priority < priority)
                break;
        }

        // no channel found, give up
        if(id == MAX_SFX)
            return -1;

        Stop_Sample(id);
    }

    return id;
}

int Get_Digi_Handle(void)
{
    // used to check if audio is initialised
    return AudioDevice ? 1 : -1;
}

bool Start_Primary_Sound_Buffer(bool forced)
{
    SDL_PauseAudioDevice(AudioDevice, false);
    return true;
}

void Stop_Primary_Sound_Buffer(void)
{
    SDL_PauseAudioDevice(AudioDevice, true);
}

uint32_t Get_Audio_Device()
{
    return AudioDevice;
}

void *Get_Audio_Spec()
{
    return &ObtainedSpec;
}

AudioCallback *Get_Audio_Callback_Ptr()
{
    return &ExtraCallback;
}

// TD
void *Load_Sample(char const *filename)
{
    printf("%s\n", __func__);
    return NULL;
}

void Free_Sample(void const *sample)
{

}