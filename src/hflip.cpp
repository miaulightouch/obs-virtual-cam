#include <stdlib.h>
#include <string.h>
#include "hflip.h"

extern "C" {
#include "libavutil/pixfmt.h"
};

struct plane_layout {
	int count;       // elements per row
	int rows;        // rows in the plane
	int elem_size;   // bytes per element
	bool packed_422; // element is a Y0/C/Y1/C macropixel
};

static int get_planes(int format, int width, int height, plane_layout *planes)
{
	int cw = (width + 1) / 2;
	int ch = (height + 1) / 2;

	switch (format) {
	case AV_PIX_FMT_YUV420P:
		planes[0] = {width, height, 1, false};
		planes[1] = {cw, ch, 1, false};
		planes[2] = {cw, ch, 1, false};
		return 3;
	case AV_PIX_FMT_NV12:
		planes[0] = {width, height, 1, false};
		planes[1] = {cw, ch, 2, false};
		return 2;
	case AV_PIX_FMT_YUV444P:
		planes[0] = {width, height, 1, false};
		planes[1] = {width, height, 1, false};
		planes[2] = {width, height, 1, false};
		return 3;
	case AV_PIX_FMT_GRAY8:
		planes[0] = {width, height, 1, false};
		return 1;
	case AV_PIX_FMT_YUYV422:
	case AV_PIX_FMT_UYVY422:
		planes[0] = {cw, height, 4, true};
		return 1;
	case AV_PIX_FMT_RGBA:
	case AV_PIX_FMT_BGRA:
		planes[0] = {width, height, 4, false};
		return 1;
	}
	return 0;
}

bool init_flip_filter(FlipContext *ctx, int width, int height, int format)
{
	if (ctx->init || width <= 0 || height <= 0)
		return false;

	plane_layout planes[4];
	int count = get_planes(format, width, height, planes);
	if (!count)
		return false;

	size_t total = 0;
	for (int i = 0; i < count; i++)
		total += (size_t)planes[i].count * planes[i].elem_size *
			 planes[i].rows;

	ctx->buffer = (uint8_t *)malloc(total);
	if (!ctx->buffer)
		return false;

	uint8_t *dst = ctx->buffer;
	for (int i = 0; i < 4; i++) {
		ctx->data[i] = nullptr;
		ctx->linesize[i] = 0;
	}
	for (int i = 0; i < count; i++) {
		ctx->data[i] = dst;
		ctx->linesize[i] = planes[i].count * planes[i].elem_size;
		dst += (size_t)ctx->linesize[i] * planes[i].rows;
	}

	ctx->width = width;
	ctx->height = height;
	ctx->format = format;
	ctx->init = true;
	return true;
}

bool release_flip_filter(FlipContext *ctx)
{
	if (!ctx->init)
		return false;

	free(ctx->buffer);
	ctx->buffer = nullptr;
	for (int i = 0; i < 4; i++) {
		ctx->data[i] = nullptr;
		ctx->linesize[i] = 0;
	}
	ctx->init = false;
	return true;
}

static void flip_row(uint8_t *dst, const uint8_t *src, int count, int elem_size,
		     bool packed_422, bool y_first)
{
	const uint8_t *s = src + (size_t)(count - 1) * elem_size;

	switch (elem_size) {
	case 1:
		for (int x = 0; x < count; x++)
			dst[x] = src[count - 1 - x];
		break;
	case 2:
		for (int x = 0; x < count; x++, dst += 2, s -= 2)
			memcpy(dst, s, 2);
		break;
	case 4:
		if (!packed_422) {
			for (int x = 0; x < count; x++, dst += 4, s -= 4)
				memcpy(dst, s, 4);
			break;
		}
		// A macropixel holds two pixels; mirroring it also swaps the
		// two luma samples, while the shared chroma stays in place.
		for (int x = 0; x < count; x++, dst += 4, s -= 4) {
			if (y_first) { // Y0 U Y1 V
				dst[0] = s[2];
				dst[1] = s[1];
				dst[2] = s[0];
				dst[3] = s[3];
			} else { // U Y0 V Y1
				dst[0] = s[0];
				dst[1] = s[3];
				dst[2] = s[2];
				dst[3] = s[1];
			}
		}
		break;
	}
}

void flip_frame(FlipContext *ctx, uint8_t **src, uint32_t *linesize)
{
	if (!ctx->init)
		return;

	plane_layout planes[4];
	int count = get_planes(ctx->format, ctx->width, ctx->height, planes);
	bool y_first = ctx->format == AV_PIX_FMT_YUYV422;

	for (int i = 0; i < count; i++) {
		const plane_layout &p = planes[i];
		for (int y = 0; y < p.rows; y++) {
			flip_row(ctx->data[i] + (size_t)ctx->linesize[i] * y,
				 src[i] + (size_t)linesize[i] * y, p.count,
				 p.elem_size, p.packed_422, y_first);
		}
	}
}
