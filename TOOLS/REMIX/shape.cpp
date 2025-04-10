#include <algorithm>
#include <cstring>

#include "compress.h"

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

void SHPFile::save_lz(uint8_t *data, uint32_t &length)
{
	int frame_size = get_frame_size();

	int raw_size = sizeof(SHPFile::Header) + num_frames * 8 + num_frames * frame_size;
	uint32_t new_length = sizeof(SHPFile::Header) + num_frames * 8;

	auto out_header = (SHPFile::Header *)data;

	out_header->frames = num_frames;
	out_header->x = x;
	out_header->y = y;
	out_header->width = width;
	out_header->height = height;
	out_header->largest_frame_size = 0;
	out_header->flags = 0x5500; // special marker

	auto frame_offsets = (uint32_t *)(data + sizeof(SHPFile::Header));
	auto out_frame_data = data + new_length;

	struct SizeData
	{
		int size = 0;
		int delta_from = 0;

		const bool operator<(const SizeData &other)
		{
			// prefer earlier frame
			if(size == other.size)
				return delta_from < other.delta_from;

			return size < other.size;
		}
	};

	auto size_matrix = new SizeData[num_frames * num_frames]();
	auto raw_comp_size = new int[num_frames]();
	auto frame_compression_buf = new uint8_t[ShapeLZ_Compress_Size(frame_size)];
	auto delta_buf = new uint8_t[frame_size];

	// find best sizes
	for(int i = 0; i < num_frames; i++)
	{
		auto frame_data = get_frame(i)->data;
		auto sizes = size_matrix + i * num_frames;

		// raw
		sizes[i].size = raw_comp_size[i] = ShapeLZ_Compress(frame_data, frame_size, frame_compression_buf);
		sizes[i].delta_from = i; // delta == self is raw

		for(int j = i + 1; j < num_frames; j++)
		{
			// delta
			// from every possible frame
			auto prev_frame = get_frame(j)->data;

			for(int b = 0; b < frame_size; b++)
				delta_buf[b] = prev_frame[b] ^ frame_data[b];

			int delta_size = ShapeLZ_Compress(delta_buf, frame_size, frame_compression_buf);
			sizes[j].size = delta_size;
			sizes[j].delta_from = j;

			// it's an xor, so the reverse delta is the same size
			size_matrix[j * num_frames + i].size = delta_size;
			size_matrix[j * num_frames + i].delta_from = i;
		}
	}

	// resolve conflicts (one level of delta)
	struct DeltaInfo
	{
		int frame = 0;

		int uses = 0;
		int saving_self = 0; // how much this frame saves by encoding itself as a delta (vs raw)

		bool operator<(const DeltaInfo &other)
		{
			return other.uses < uses;
		}
	};

	auto delta_info = new DeltaInfo[num_frames];
	auto final_deltas = new int[num_frames];

	for(int i = 0; i < num_frames; i++)
	{
		auto sizes = size_matrix + i * num_frames;

		if(i > 0) // keep raw as first choice for first frame
			std::sort(sizes, sizes + num_frames);
		final_deltas[i] = -1;
	}

	int frames_decided;
	int loss_target = 0; // optimism
	do
	{
		frames_decided = 0;
		memset(delta_info, 0, sizeof(DeltaInfo) * num_frames);

		for(int i = 0; i < num_frames; i++)
		{
			// don't reverse forcing raw
			if(final_deltas[i] == i)
			{
				delta_info[i].frame = i;
				delta_info[i].saving_self = 0;
				frames_decided++;
				continue;
			}

			auto sizes = size_matrix + i * num_frames;

			int delta_from = sizes[0].delta_from;

			// calc stats
			delta_info[i].frame = i;
			delta_info[i].saving_self = raw_comp_size[i] - sizes[0].size;

			if(delta_from != i) // don't mark raw as using self
				delta_info[delta_from].uses++;

			for(int j = 0; j < num_frames; j++)
			{
				delta_from = sizes[j].delta_from;

				int loss = sizes[j].size - sizes[0].size;

				if(loss == 0) // mark any identical options as also used
					delta_info[delta_from].uses++;

				// scan until loss too high
				if(loss > loss_target)
					break;

				// this is a raw frame
				if(delta_from == i)
				{
					final_deltas[i] = i;
					break;
				}

				auto delta_sizes = size_matrix + delta_from * num_frames;
				int delta_delta = delta_sizes[0].delta_from;

				// delta from a raw frame, this is okay
				if(delta_delta == delta_from || final_deltas[delta_from] == delta_from)
				{
					final_deltas[i] = delta_from;
					break;
				}
			}

			// found one
			if(final_deltas[i] != -1)
			{
				frames_decided++;
				continue;
			}
		}

		// undecided frames, mark one as raw
		if(frames_decided != num_frames)
		{
			std::sort(delta_info, delta_info + num_frames);

			int i;

			// find first that isn't already raw
			for(i = 0; i < num_frames; i++)
			{
				if(final_deltas[delta_info[i].frame] == -1)
					break;
			}

			// there are no deltas from this (or any other?) frame
			if(delta_info[i].uses == 0)
			{
				loss_target = delta_info[i].saving_self;
				continue;
			}

			// mark raw
			final_deltas[delta_info[i].frame] = delta_info[i].frame;
			
			// make up some target for next attempt
			loss_target = delta_info[i].saving_self / delta_info[i].uses;
		}
	}
	while(frames_decided != num_frames);

	delete[] delta_info;
    delete[] size_matrix;
	delete[] raw_comp_size;

	// final data
	for(int i = 0; i < num_frames; i++)
	{
		int delta_from = final_deltas[i];

		auto frame_data = get_frame(i)->data;

		// do final compression
		int final_frame_size = 0;

		auto decomp_buf = new uint8_t[frame_size];

		if(delta_from == i)
			final_frame_size = ShapeLZ_Compress(frame_data, frame_size, frame_compression_buf);
		else
		{
			auto prev_frame = get_frame(delta_from)->data;

			for(int b = 0; b < frame_size; b++)
				delta_buf[b] = prev_frame[b] ^ frame_data[b];

			final_frame_size = ShapeLZ_Compress(delta_buf, frame_size, frame_compression_buf);
		}

		delete[] decomp_buf;

		// copy to final output
		memcpy(out_frame_data, frame_compression_buf, final_frame_size);
		out_frame_data += final_frame_size;

		// update offset/sizes
		if(final_frame_size > out_header->largest_frame_size)
			out_header->largest_frame_size = final_frame_size;

		// also encode keyframe/keydelte
		frame_offsets[i * 2] = new_length | (delta_from == i ? 0x80 : 0x40) << 24;

		new_length += final_frame_size;
	}

	// fill in delta offsets
	for(int i = 0; i < num_frames; i++)
	{
		if(final_deltas[i] != i)
			frame_offsets[i * 2 + 1] = frame_offsets[final_deltas[i] * 2] & 0xFFFFFF;
	}

	// cleanup
	delete[] final_deltas;
	delete[] delta_buf;
	delete[] frame_compression_buf;

    length = new_length;
}

const SHPFile::Frame *SHPFile::get_frame(unsigned index) const
{
    if(index > frames.size())
        return nullptr;

    return &frames[index];
}