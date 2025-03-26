#pragma once
#include <cstdint>
#include <list>

class MixWriter
{
public:
	~MixWriter();

	void addEntry(int32_t crc, int32_t length, uint8_t *data);

	void write(const char *filename);

private:
	struct Entry
	{
		int32_t crc;
		int32_t length;
		uint8_t *data;
	};

	std::list<Entry> entries;
};