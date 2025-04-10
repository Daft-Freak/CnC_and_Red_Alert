#include "compress.h"

uint32_t ShapeLZ_Compress(uint8_t *in_data, uint32_t in_len, uint8_t *out_data)
{
	uint32_t flag = 0;
	int flag_bits = 0;

	auto in_ptr = in_data;
	auto in_end = in_data + in_len;
	auto out_ptr = out_data;

	auto flag_ptr = (uint32_t *)out_ptr;
	out_ptr += 4;

	static const int min_copy = 3;
	static const int max_copy_len = ((1 << 14) - 1) + min_copy;
	static const int max_copy_offset = 1 << 8;

	while(in_ptr < in_end)
	{
		// special case for run
		int run_len = 0;
		if(in_ptr != in_data && *in_ptr != *(in_ptr - 1))
		{
			auto tmp = in_ptr;
			for(; tmp != in_end && *tmp == *in_ptr; tmp++);
			run_len = tmp - in_ptr;
		}

		// scan for match
		int match_off = max_copy_offset;
		int match_len = 0;

		auto search_ptr = in_ptr - max_copy_offset;
		if(search_ptr < in_data) // clamp
			search_ptr = in_data;

		for(; search_ptr != in_ptr; search_ptr++)
		{
			if(*search_ptr != *in_ptr)
				continue;
			
			// scan for length
			auto match_ptr = search_ptr;
			auto in_tmp = in_ptr;
			for(; in_tmp != in_end && *match_ptr == *in_tmp; *match_ptr++, *in_tmp++);

			int len = in_tmp - in_ptr;
			auto off = in_ptr - search_ptr;

			// clamp length, unless this is the final run as we don't actually encode that
			if(len > max_copy_len && (off > 1 || in_tmp != in_end))
				len = max_copy_len;

			if(len >= match_len)
			{
				match_len = len;
				match_off = off;
			}
		}

		// copy if large enough, but don't if a longer run could be encoded using the raw byte
		if(match_len >= min_copy && run_len <= match_len)
		{
			// skip
			in_ptr += match_len;

			if(match_off == 1 && in_ptr == in_end)
			{
				// flag end
				flag |= 3 << flag_bits;
			}
			else
			{
				// adjust
				match_off--;
				match_len -= min_copy;
				
				// flag copy
				flag |= 1 << flag_bits;

				// len is 6/14 bits, off is 0/8
				uint8_t lo_off_len = (match_len & 0x3F) << 1;
				if(match_off)
					lo_off_len |= 1; // indicates that we have a non-1 offset
				if(match_len > 0x3F)
					lo_off_len |= 0x80;

				*out_ptr++ = lo_off_len;

				if(match_off)
					*out_ptr++ = match_off;

				if(match_len > 0x3F)
					*out_ptr++ = match_len >> 6 ;
			}
		}
		else
		{
			// check if we can find the next two bytes with a 4bit offset
			bool do_raw = true;
			if(in_ptr + 1 != in_end)
			{
				int off0 = 0, off1 = 0;
				auto search_ptr = in_ptr - 16;
				if(search_ptr < in_data) // clamp
					search_ptr = in_data;

				for(; search_ptr != in_ptr; search_ptr++)
				{
					if(*search_ptr == *in_ptr)
						off0 = in_ptr - search_ptr;
					if(*search_ptr == in_ptr[1])
						off1 = in_ptr - search_ptr;
				}

				if(off0 && off1)
				{
					flag |= 2 << flag_bits; // flag two-byte copy

					*out_ptr++ = (off0 - 1) | (off1 - 1) << 4;
					in_ptr += 2;
					do_raw = false;
				}
			}

            // single raw byte
			if(do_raw)
				*out_ptr++ = *in_ptr++;
		}

		// write flag byte if full
		flag_bits += 2;
		if(flag_bits == 32)
		{
			*flag_ptr = flag;
			flag_ptr = (uint32_t *)out_ptr;
			out_ptr += 4;
			flag = 0;
			flag_bits = 0;
		}
	}

	// final flags
	if(flag_bits)
		*flag_ptr = flag;
	else
		out_ptr -= 4;

	return out_ptr - out_data;
}

uint32_t ShapeLZ_Compress_Size(uint32_t size)
{
	// worst-case
	return size + ((size * 2 + 31) / 32) * 4;
}

void ShapeLZ_Decompress(uint8_t *in_data, uint8_t *out_data, uint32_t out_len)
{
    auto out_end = out_data + out_len;

    int flag_bits = 32;
    uint32_t flags = *(uint32_t *)in_data;
    in_data += 4;

    while(out_data != out_end)
    {
        switch(flags & 3)
        {
            case 0: // raw byte
                *out_data++ = *in_data++;
                break;
            case 1: // copy
            {
                // get offset/len
                auto v = *in_data++;
                int offset = 0;
                int length = (v >> 1) & 0x3F;

                // offset > 1
                if(v & 1)
                    offset = *in_data++;

                // len >= 0x3F
                if(v & 0x80)
                    length += (*in_data++) << 6;

                // adjust
                offset++;
                length += 3;

                // do copy
                auto copy_ptr = out_data - offset;

                for(int i = 0; i < length; i++)
                    *out_data++ = *copy_ptr++;
                break;
            }
            case 2: // two byte copy
            {
                auto v = *in_data++;
                int off0 = (v & 0xF) + 1;
                int off1 = (v >> 4) + 1;

                out_data[0] = *(out_data - off0);
                out_data[1] = *(out_data - off1);
                out_data += 2;
                break;
            }
            case 3: // end (fill rest of output)
            {
                auto v = out_data[-1];
                while(out_data != out_end)
                    *out_data++ = v;
                break;
            }
        }

        flags >>= 2;
        flag_bits -= 2;

        // fetch more flags
        if(!flag_bits)
        {
            flag_bits = 32;
            flags = *(uint32_t *)in_data;
            in_data += 4;
        }
    }
}

void ShapeLZ_Decompress_Xor(uint8_t *in_data, uint8_t *out_data, uint32_t out_len)
{
    auto out_end = out_data + out_len;

    int flag_bits = 32;
    uint32_t flags = *(uint32_t *)in_data;
    in_data += 4;

    // the main difference here is that we need to keep a window of the raw xor values
    auto xor_window = new uint8_t[256];
    int xor_window_offset = 0;

    auto xor_window_put = [&xor_window, &xor_window_offset](uint8_t v)
    {
        xor_window[xor_window_offset++] = v;
        xor_window_offset &= 0xFF;
    };

    auto xor_window_get = [&xor_window, &xor_window_offset](int offset)
    {
        offset = (xor_window_offset + 0x100 - offset) & 0xFF;
        return xor_window[offset];
    };

    while(out_data != out_end)
    {
        switch(flags & 3)
        {
            case 0: // raw byte
                xor_window_put(*in_data);
                *out_data++ ^= *in_data++;
                break;
            case 1: // copy
            {
                // get offset/len
                auto v = *in_data++;
                int offset = 0;
                int length = (v >> 1) & 0x3F;

                // offset > 1
                if(v & 1)
                    offset = *in_data++;

                // len >= 0x3F
                if(v & 0x80)
                    length += (*in_data++) << 6;

                // adjust
                offset++;
                length += 3;

                // do copy
                // could optimise this for offset == 1?
                int copy_offset = (xor_window_offset + 0x100 - offset) & 0xFF;
                for(int i = 0; i < length; i++)
                {
                    auto xor_v = xor_window[copy_offset++];
                    copy_offset &= 0xFF;
                    *out_data++ ^= xor_v;
                    xor_window_put(xor_v);
                }
                
                break;
            }
            case 2: // two byte copy
            {
                auto v = *in_data++;
                int off0 = (v & 0xF) + 1;
                int off1 = (v >> 4) + 1;

                auto xor0 = xor_window_get(off0);
                auto xor1 = xor_window_get(off1);

                out_data[0] ^= xor0;
                out_data[1] ^= xor1;
                out_data += 2;

                xor_window_put(xor0);
                xor_window_put(xor1);
                break;
            }
            case 3: // end (fill rest of output)
            {
                auto v = xor_window_get(1);
                while(out_data != out_end)
                    *out_data++ ^= v;
                break;
            }
        }

        flags >>= 2;
        flag_bits -= 2;

        // fetch more flags
        if(!flag_bits)
        {
            flag_bits = 32;
            flags = *(uint32_t *)in_data;
            in_data += 4;
        }
    }

    delete[] xor_window;
}
