#pragma once
#include <cstdint>
#include <vector>

class SHPFile
{
public:
	struct Header
	{
		uint16_t frames;
		uint16_t x;
		uint16_t y;
		uint16_t width;
		uint16_t height;
		uint16_t largest_frame_size;
		uint16_t flags;
	};
	struct Frame
	{
		uint8_t *data = nullptr;
	};

	~SHPFile();

	bool load(const uint8_t *data, uint32_t length);

    void save_lz(uint8_t *data, uint32_t &length);

	unsigned get_width() const {return width;}
	unsigned get_height() const {return height;}
	unsigned get_frame_size() const {return width * height;}

	unsigned get_num_frames() const {return num_frames;}

    unsigned get_x() const {return x;}
    unsigned get_y() const {return y;}

	const Frame *get_frame(unsigned index) const;

private:
	unsigned width = 0, height = 0, num_frames = 0;
	unsigned x = 0, y = 0;

	std::vector<Frame> frames;
};