#pragma once

// ----------------------------------------------------------------------------
// DVI constants

#define TMDS_CTRL_00 0x354u
#define TMDS_CTRL_01 0x0abu
#define TMDS_CTRL_10 0x154u
#define TMDS_CTRL_11 0x2abu

#define TMDS_BALANCED_LOW  0x307u
#define TMDS_BALANCED_HIGH 0x2f0u

#define TMDS_BLACK_A 0x100u
#define TMDS_BLACK_B 0x1ffu

#define SYNC_V0_H0 (TMDS_CTRL_00 | (TMDS_CTRL_00 << 10) | (TMDS_CTRL_00 << 20))
#define SYNC_V0_H1 (TMDS_CTRL_01 | (TMDS_CTRL_00 << 10) | (TMDS_CTRL_00 << 20))
#define SYNC_V1_H0 (TMDS_CTRL_10 | (TMDS_CTRL_00 << 10) | (TMDS_CTRL_00 << 20))
#define SYNC_V1_H1 (TMDS_CTRL_11 | (TMDS_CTRL_00 << 10) | (TMDS_CTRL_00 << 20))
#define MISSING_PIXEL (TMDS_BALANCED_LOW | (TMDS_BALANCED_LOW << 10) | (TMDS_BALANCED_HIGH << 20))
#define BLACK_PIXEL (TMDS_BALANCED_LOW | (TMDS_BALANCED_LOW << 10) | (TMDS_BALANCED_LOW << 20))
#define BLACK_PIXEL_A (TMDS_BLACK_A | (TMDS_BLACK_A << 10) | (TMDS_BLACK_A << 20))
#define BLACK_PIXEL_B (TMDS_BLACK_B | (TMDS_BLACK_B << 10) | (TMDS_BLACK_B << 20))

#define HSTX_CMD_RAW         (0x0u << 12)
#define HSTX_CMD_RAW_REPEAT  (0x1u << 12)
#define HSTX_CMD_TMDS        (0x2u << 12)
#define HSTX_CMD_TMDS_REPEAT (0x3u << 12)
#define HSTX_CMD_NOP         (0xfu << 12)

struct dvi_timing {
	bool h_sync_polarity;
	int h_front_porch;
	int h_sync_width;
	int h_back_porch;
	int h_active_pixels;

	bool v_sync_polarity;
	int v_front_porch;
	int v_sync_width;
	int v_back_porch;
	int v_active_lines;

	uint bit_clk_khz;
};

extern const struct dvi_timing dvi_timing_640x480p_60hz;
extern const struct dvi_timing dvi_timing_720x480p_60hz;
extern const struct dvi_timing dvi_timing_720x576p_50hz;
extern const struct dvi_timing dvi_timing_720x400p_60hz;
extern const struct dvi_timing dvi_timing_720x400p_70hz;
extern const struct dvi_timing dvi_timing_800x450p_60hz;
extern const struct dvi_timing dvi_timing_800x480p_60hz;
extern const struct dvi_timing dvi_timing_800x600p_60hz;
extern const struct dvi_timing dvi_timing_960x540p_60hz;
extern const struct dvi_timing dvi_timing_960x540p_50hz;
extern const struct dvi_timing dvi_timing_1024x768_rb_60hz;
extern const struct dvi_timing dvi_timing_1280x720p_rb_50hz;
extern const struct dvi_timing dvi_timing_1280x720p_rb_60hz;
extern const struct dvi_timing dvi_timing_1920x1080i_50hz;
extern const struct dvi_timing dvi_timing_1920x1080i_60hz;
extern const struct dvi_timing dvi_timing_1920x1080p_rb2_30hz;
extern const struct dvi_timing dvi_timing_1920x1080p_30hz;
extern const struct dvi_timing dvi_timing_1920x1080p_yolo_48hz;
extern const struct dvi_timing dvi_timing_2560x1440p_yolo_24hz;
