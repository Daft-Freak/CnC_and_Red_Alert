#include <cstring>

#include "shape.h"

extern "C" unsigned long LCW_Uncompress(void *source, void *dest, unsigned long length);
unsigned int Apply_XOR_Delta(char *source_ptr, char *delta_ptr);


SHPFile::~SHPFile()
{
	for(auto &frame : frames)
		delete[] frame.data;
}

bool SHPFile::load(const uint8_t *data, uint32_t length)
{
	auto header = (const Header *)data;

	int frame_size = header->width * header->height;

	frames.resize(header->frames);

    if(header->flags)
        return false;

	num_frames = header->frames;
	x = header->x;
	y = header->y;
	width = header->width;
	height = header->height;

	// nothing to load if the frames are all 0x0
	if(!frame_size)
		return true;

	// decompression buffer
	// with a little margin as LCW writes too much sometimes
	auto buf = new uint8_t[frame_size + 4];

	for(int i = 0; i < header->frames; i++)
	{
		auto frame_head = (uint32_t *)(data + sizeof(Header) + i * 8);
		int next_off = i == header->frames - 1 ? length : frame_head[2] & 0xFFFFFF;
	
		int flags = frame_head[0] >> 24;
		auto offset = frame_head[0] & 0xFFFFFF;
		auto len = next_off - offset;

		if(flags & 0x80) // KEYFRAME
		{
			auto in_ptr = data + offset;
			LCW_Uncompress((void *)in_ptr, buf, frame_size);
		}
		else if(flags & 0x40) // KEYDELTA
		{
			// decompress keyframe
			auto in_ptr = data + (frame_head[1] & 0xFFFFFF);
			LCW_Uncompress((void *)in_ptr, buf, frame_size);

			// apply delta
			Apply_XOR_Delta((char *)buf, (char *)data + offset);
		}
		else if(flags & 0x20) // DELTA
		{
			int dframe = frame_head[1] & 0xFFFF;
			auto delta_head = (uint32_t *)(data + sizeof(Header) + dframe * 8);

			// decompress keyframe
			auto in_ptr = data + (delta_head[1] & 0xFFFFFF);
			LCW_Uncompress((void *)in_ptr, buf, frame_size);

			// apply delta(s)
			Apply_XOR_Delta((char *)buf, (char *)data + (delta_head[0] & 0xFFFFFF));

			dframe++;
			for(int subframe = 2; dframe <= i; subframe += 2, dframe++)
				Apply_XOR_Delta((char *)buf, (char *)data + (delta_head[subframe] & 0xFFFFFF));
		}

		// save raw data
		frames[i].data = new uint8_t[frame_size];
		memcpy(frames[i].data, buf, frame_size);
	}

	delete[] buf;

	return true;
}

const SHPFile::Frame *SHPFile::get_frame(unsigned index) const
{
    if(index > frames.size())
        return nullptr;

    return &frames[index];
}