#include <stdint.h>
#include "gbuffer.h"

#ifndef LORES

#define SIZE_OF_PALETTE	256
extern "C"{
	extern unsigned char PaletteInterpolationTable[SIZE_OF_PALETTE][SIZE_OF_PALETTE];
}

extern "C" void ModeX_Blit(GraphicBufferClass * source)
{
	printf("%s\n", __func__);
}

extern "C" void Asm_Interpolate(unsigned char* src_ptr, unsigned char* dest_ptr, int lines, int src_width, int dest_width)
{
	// this one line doubles by skipping every other line
	auto old_dest = dest_ptr;

	auto interp_table_flat = (uint8_t *)PaletteInterpolationTable;

	do
	{
		int width_counter = 0;
		int pixel_count = (src_width - 2) / 2; // we copy two at a time, so this isn't actually a pixel count

		auto out_ptr = old_dest;

		// convert 2 pixels of source into 4 pixels of destination
		do
		{
			// read four pixels (we use three)
			auto in_pixels = *(uint32_t *)src_ptr;
			src_ptr += 2;

			auto interped = interp_table_flat[in_pixels & 0xFFFF];
			auto interped2 = interp_table_flat[(in_pixels >> 8) & 0xFFFF];
			
			auto out_pixels = (in_pixels & 0xFF) | interped << 8 | (in_pixels & 0xFF00) << 8 | interped2 << 24;

			*(uint32_t *)out_ptr = out_pixels;
			out_ptr += 4;

		}
		while(--pixel_count);

		// do the last three pixels and a blank
		auto in_pixels = *(uint16_t *)src_ptr;
		src_ptr += 2;
		*out_ptr++ = in_pixels & 0xFF;
		*out_ptr++ = interp_table_flat[in_pixels];
		*out_ptr++ = in_pixels >> 8;
		*out_ptr++ = 0;

		old_dest += dest_width; // skip every other line
	}
	while(--lines);
}

extern "C" void Asm_Interpolate_Line_Double(unsigned char* src_ptr, unsigned char* dest_ptr, int lines, int src_width, int dest_width)
{
	auto old_dest = dest_ptr;

	auto interp_table_flat = (uint8_t *)PaletteInterpolationTable;

	do
	{
		int width_counter = 0;
		int pixel_count = (src_width - 2) / 2; // we copy two at a time, so this isn't actually a pixel count

		auto out_ptr = old_dest;
		auto out2_ptr = old_dest + dest_width / 2; // dest width is 2x the actual width...

		// convert 2 pixels of source into 4 pixels of destination
		do
		{
			// read four pixels (we use three)
			auto in_pixels = *(uint32_t *)src_ptr;
			src_ptr += 2;

			auto interped = interp_table_flat[in_pixels & 0xFFFF];
			auto interped2 = interp_table_flat[(in_pixels >> 8) & 0xFFFF];
			
			auto out_pixels = (in_pixels & 0xFF) | interped << 8 | (in_pixels & 0xFF00) << 8 | interped2 << 24;

			*(uint32_t *)out_ptr = out_pixels;
			out_ptr += 4;
			*(uint32_t *)out2_ptr = out_pixels;
			out2_ptr += 4;
		}
		while(--pixel_count);

		// do the last three pixels and a blank
		auto in_pixels = *(uint16_t *)src_ptr;
		src_ptr += 2;
		*out_ptr++ = *out2_ptr++ = in_pixels & 0xFF;
		*out_ptr++ = *out2_ptr++ = interp_table_flat[in_pixels];
		*out_ptr++ = *out2_ptr++ = in_pixels >> 8;
		*out_ptr++ = *out2_ptr++ = 0;

		old_dest += dest_width;
	}
	while(--lines);
}

extern "C" void Asm_Interpolate_Line_Interpolate(unsigned char* src_ptr, unsigned char* dest_ptr, int lines, int src_width, int dest_width)
{
	printf("%s\n", __func__);
}

#endif