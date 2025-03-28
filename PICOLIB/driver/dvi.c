#include <pico/stdlib.h>

#include "dvi.h"

// VGA -- we do this mode properly, with a pretty comfortable clk_sys (252 MHz)
const struct dvi_timing dvi_timing_640x480p_60hz = {
	.h_sync_polarity   = false,
	.h_front_porch     = 16,
	.h_sync_width      = 96,
	.h_back_porch      = 48,
	.h_active_pixels   = 640,

	.v_sync_polarity   = false,
	.v_front_porch     = 10,
	.v_sync_width      = 2,
	.v_back_porch      = 33,
	.v_active_lines    = 480,

	.bit_clk_khz       = 252000
};

// SVGA -- completely by-the-book but requires 400 MHz clk_sys
const struct dvi_timing dvi_timing_800x600p_60hz = {
	.h_sync_polarity   = false,
	.h_front_porch     = 44,
	.h_sync_width      = 128,
	.h_back_porch      = 88,
	.h_active_pixels   = 800,

	.v_sync_polarity   = false,
	.v_front_porch     = 1,
	.v_sync_width      = 4,
	.v_back_porch      = 23,
	.v_active_lines    = 600,

	.bit_clk_khz       = 400000
};

// 720x480 - timings from dumping the EDID of my monitor
const struct dvi_timing dvi_timing_720x480p_60hz = {
	.h_sync_polarity   = false,
	.h_front_porch     = 16,
	.h_sync_width      = 62,
	.h_back_porch      = 60,
	.h_active_pixels   = 720,

	.v_sync_polarity   = false,
	.v_front_porch     = 9,
	.v_sync_width      = 6,
	.v_back_porch      = 30,
	.v_active_lines    = 480,

	.bit_clk_khz       = 270000
};

// 720x576@50Hz - CEA timing
const struct dvi_timing dvi_timing_720x576p_50hz = {
	.h_sync_polarity   = false,
	.h_front_porch     = 12,
	.h_sync_width      = 64,
	.h_back_porch      = 68,
	.h_active_pixels   = 720,

	.v_sync_polarity   = false,
	.v_front_porch     = 5,
	.v_sync_width      = 5,
	.v_back_porch      = 39,
	.v_active_lines    = 576,

	.bit_clk_khz       = 270000
};

// 720x400@70Hz - "IBM" timing
const struct dvi_timing dvi_timing_720x400p_70hz = {
	.h_sync_polarity   = false,
	.h_front_porch     = 18,
	.h_sync_width      = 108,
	.h_back_porch      = 54,
	.h_active_pixels   = 720,

	.v_sync_polarity   = true,
	.v_front_porch     = 13,
	.v_sync_width      = 2,
	.v_back_porch      = 34,
	.v_active_lines    = 400,

	.bit_clk_khz       = 283200
};

// 720x400@60Hz - Made up, seems to work.  Nice long vblank for updating the frame buffer.
const struct dvi_timing dvi_timing_720x400p_60hz = {
	.h_sync_polarity   = false,
	.h_front_porch     = 18,
	.h_sync_width      = 108,
	.h_back_porch      = 69,
	.h_active_pixels   = 720,

	.v_sync_polarity   = true,
	.v_front_porch     = 13,
	.v_sync_width      = 2,
	.v_back_porch      = 44,
	.v_active_lines    = 400,

	.bit_clk_khz       = 252000
};

// 800x480p 60 Hz (note this doesn't seem to be a CEA mode, I just used the
// output of `cvt 800 480 60`), 295 MHz bit clock
const struct dvi_timing dvi_timing_800x480p_60hz = {
	.h_sync_polarity = false,
	.h_front_porch   = 24,
	.h_sync_width    = 72,
	.h_back_porch    = 96,
	.h_active_pixels = 800,

	.v_sync_polarity = true,
	.v_front_porch   = 3,
	.v_sync_width    = 10,
	.v_back_porch    = 7,
	.v_active_lines  = 480,

	.bit_clk_khz     = 295200
};

// 800x450p 60 Hz Similarly not a CEA mode, but is 16:9
const struct dvi_timing dvi_timing_800x450p_60hz = {
	.h_sync_polarity = false,
	.h_front_porch   = 24,
	.h_sync_width    = 72,
	.h_back_porch    = 96,
	.h_active_pixels = 800,

	.v_sync_polarity = true,
	.v_front_porch   = 3,
	.v_sync_width    = 5,
	.v_back_porch    = 10,
	.v_active_lines  = 450,

	.bit_clk_khz     = 278400
};

// SVGA reduced blanking (355 MHz bit clock) -- valid CVT mode, less common
// than fully-blanked SVGA, but doesn't require such a high system clock
const struct dvi_timing dvi_timing_800x600p_reduced_60hz = {
	.h_sync_polarity   = true,
	.h_front_porch     = 48,
	.h_sync_width      = 32,
	.h_back_porch      = 80,
	.h_active_pixels   = 800,

	.v_sync_polarity   = false,
	.v_front_porch     = 3,
	.v_sync_width      = 4,
	.v_back_porch      = 11,
	.v_active_lines    = 600,

	.bit_clk_khz       = 354000
};

// Also known as qHD, bit uncommon, but it's a nice modest-resolution 16:9
// aspect mode. Pixel clock 40.75 MHz for full CVT mode (no reduced blanking)
const struct dvi_timing dvi_timing_960x540p_60hz = {
	.h_sync_polarity   = false,
	.h_front_porch     = 32,
	.h_sync_width      = 96,
	.h_back_porch      = 128,
	.h_active_pixels   = 960,

	.v_sync_polarity   = true,
	.v_front_porch     = 3,
	.v_sync_width      = 5,
	.v_back_porch      = 14,
	.v_active_lines    = 540,

	.bit_clk_khz       = 408000
};

// Also known as qHD, bit uncommon, but it's a nice modest-resolution 16:9
// aspect mode. Pixel clock 33.5 MHz for 50Hz CVT mode (no reduced blanking)
const struct dvi_timing dvi_timing_960x540p_50hz = {
	.h_sync_polarity   = false,
	.h_front_porch     = 24,
	.h_sync_width      = 96,
	.h_back_porch      = 120,
	.h_active_pixels   = 960,

	.v_sync_polarity   = true,
	.v_front_porch     = 3,
	.v_sync_width      = 5,
	.v_back_porch      = 11,
	.v_active_lines    = 540,

	.bit_clk_khz       = 336000
};

// 1024x768, CVT-RB
const struct dvi_timing dvi_timing_1024x768_rb_60hz = {
	.h_sync_polarity   = true,
	.h_front_porch     = 48,
	.h_sync_width      = 32,
	.h_back_porch      = 80,
	.h_active_pixels   = 1024,

	.v_sync_polarity   = false,
	.v_front_porch     = 3,
	.v_sync_width      = 4,
	.v_back_porch      = 15,
	.v_active_lines    = 768,

	.bit_clk_khz       = 560000
};

// 720p50, this is a standard HD mode, the CVT-RB variant
// should be widely supported
const struct dvi_timing dvi_timing_1280x720p_rb_50hz = {
	.h_sync_polarity   = true,
	.h_front_porch     = 48,
	.h_sync_width      = 32,
	.h_back_porch      = 80,
	.h_active_pixels   = 1280,

	.v_sync_polarity   = false,
	.v_front_porch     = 3,
	.v_sync_width      = 5,
	.v_back_porch      = 9,
	.v_active_lines    = 720,

	.bit_clk_khz       = 528000
};

// 720p60, this is the CVT-RB variant, again should be widely supported
const struct dvi_timing dvi_timing_1280x720p_rb_60hz = {
	.h_sync_polarity   = true,
	.h_front_porch     = 48,
	.h_sync_width      = 32,
	.h_back_porch      = 80,
	.h_active_pixels   = 1280,

	.v_sync_polarity   = false,
	.v_front_porch     = 3,
	.v_sync_width      = 5,
	.v_back_porch      = 13,
	.v_active_lines    = 720,

	.bit_clk_khz       = 640000
};

// 1080p30 RB2
// Strictly speaking RB2 should have a clock speed matching the target frequency more closely
// but it seems to work!
const struct dvi_timing dvi_timing_1920x1080p_rb2_30hz = {
	.h_sync_polarity   = true,
	.h_front_porch     = 8,
	.h_sync_width      = 32,
	.h_back_porch      = 40,
	.h_active_pixels   = 1920,

	.v_sync_polarity   = false,
	.v_front_porch     = 7,
	.v_sync_width      = 8,
	.v_back_porch      = 6,
	.v_active_lines    = 1080,

	.bit_clk_khz       = 660000
};

// 1080p30 - format 34, by the book
const struct dvi_timing dvi_timing_1920x1080p_30hz = {
	.h_sync_polarity   = true,
	.h_front_porch     = 88,
	.h_sync_width      = 44,
	.h_back_porch      = 148,
	.h_active_pixels   = 1920,

	.v_sync_polarity   = true,
	.v_front_porch     = 4,
	.v_sync_width      = 5,
	.v_back_porch      = 36,
	.v_active_lines    = 1080,

	.bit_clk_khz       = 744000
};

// 1080p48 YOLO - may require more than 1.3V, and sorting through your 
// devboards to find one that gives a clean signal.
const struct dvi_timing dvi_timing_1920x1080p_yolo_48hz = {
        .h_sync_polarity   = true,
        .h_front_porch     = 8,
        .h_sync_width      = 16,
        .h_back_porch      = 24,
        .h_active_pixels   = 1920,

        .v_sync_polarity   = false,
        .v_front_porch     = 4,
        .v_sync_width      = 2,
        .v_back_porch      = 6,
        .v_active_lines    = 1080,

        .bit_clk_khz       = 1032000
};

// 1440p24 YOLO - works on my Dell Ultrasharp, that most forgiving of monitors.  May require a little more than 1.3V
const struct dvi_timing dvi_timing_2560x1440p_yolo_24hz = {
	.h_sync_polarity   = true,
	.h_front_porch     = 8,
	.h_sync_width      = 32,
	.h_back_porch      = 40,
	.h_active_pixels   = 2560,

	.v_sync_polarity   = false,
	.v_front_porch     = 7,
	.v_sync_width      = 8,
	.v_back_porch      = 9,
	.v_active_lines    = 1440,

	.bit_clk_khz       = 928000
};
