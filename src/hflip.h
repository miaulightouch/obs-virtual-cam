#pragma once

#include <stdint.h>

// Horizontal mirror for the virtual output, implemented without FFmpeg so the
// OBS-side plugin does not depend on the FFmpeg DLLs shipped with OBS. Those
// change with every OBS release and the OBS updater deletes old versions.
struct FlipContext {
	bool init = false;
	int width = 0;
	int height = 0;
	int format = 0;
	uint8_t *buffer = nullptr;
	uint8_t *data[4] = {};
	uint32_t linesize[4] = {};
};

bool init_flip_filter(FlipContext *ctx, int width, int height, int format);
bool release_flip_filter(FlipContext *ctx);
void flip_frame(FlipContext *ctx, uint8_t **src, uint32_t *linesize);
