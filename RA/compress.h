#pragma once
#include <stdint.h>

// custom compressor for shape data (~1.2x more effective than LCW, (up to ~3.6x))
uint32_t ShapeLZ_Compress(uint8_t *in_data, uint32_t in_len, uint8_t *out_data);
uint32_t ShapeLZ_Compress_Size(uint32_t size);

void ShapeLZ_Decompress(uint8_t *in_data, uint8_t *out_data, uint32_t out_len);
void ShapeLZ_Decompress_Xor(uint8_t *in_data, uint8_t *out_data, uint32_t out_len);
