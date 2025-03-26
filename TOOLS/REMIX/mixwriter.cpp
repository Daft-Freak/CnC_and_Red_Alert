#include <fstream>

#include "mixwriter.h"

MixWriter::~MixWriter()
{
	for(auto &ent : entries)
		delete[] ent.data;
}

void MixWriter::addEntry(int32_t crc, int32_t length, uint8_t *data)
{
	entries.emplace_back(Entry{crc, length, data});
}

void MixWriter::write(const char *filename)
{
	std::ofstream file(filename, std::ios::binary);

	auto num_entries = entries.size();
	uint32_t total_size = 0;

	for(auto &ent : entries)
		total_size += ent.length;

	// the entries need to be sorted
	entries.sort([](Entry &a, Entry &b){return a.crc < b.crc;});

	int16_t head[3];

	head[0] = 0; // extended header
	head[1] = 0; // flags (no encryption/digest yet)
	head[2] = num_entries;

	file.write((char *)head, sizeof(head));
	file.write((char *)&total_size, 4);

	uint32_t offset = 0;

	for(auto &ent : entries)
	{
		int32_t block[3];
		block[0] = ent.crc;
		block[1] = offset;
		block[2] = ent.length;

		offset += ent.length;

		file.write((char *)block, sizeof(block));
	}

	for(auto &ent : entries)
		file.write((char *)ent.data, ent.length);
}