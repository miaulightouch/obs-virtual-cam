#include <wchar.h>
#include <util/platform.h>
#include <util/threading-windows.h>
#include "shared-memory-queue.h"
#include "virtualcam-output.h"
#include "virtualcam-guid.h"
#include "plugin-support.h"

struct virtualcam_data {
	obs_output_t *output;
	video_queue_t *vq;
	wchar_t *vcid;
	volatile bool active;
	volatile bool stopping;
};

static const char *virtualcam_name(void *unused)
{
	UNUSED_PARAMETER(unused);
	return obs_module_text("Basic.VCam.VirtualCamera");
}

static void virtualcam_destroy(void *data)
{
	struct virtualcam_data *vcam = (struct virtualcam_data *)data;
	video_queue_close(vcam->vq);
	bfree(data);

	obs_log(LOG_INFO, "Virtual output destroyed");
}

static void *virtualcam_create(obs_data_t *settings, obs_output_t *output)
{
	struct virtualcam_data *vcam =
		(struct virtualcam_data *)bzalloc(sizeof(*vcam));
	vcam->output = output;

	int64_t vcamIndex = obs_data_get_int(settings, "vcamIndex");
	vcam->vcid = (wchar_t *)video_device_guid_wchar[vcamIndex];

	return vcam;
}

static bool virtualcam_start(void *data)
{
	struct virtualcam_data *vcam = (struct virtualcam_data *)data;
	uint32_t width = obs_output_get_width(vcam->output);
	uint32_t height = obs_output_get_height(vcam->output);

	struct obs_video_info ovi;
	obs_get_video_info(&ovi);

	uint64_t interval = ovi.fps_den * 10000000ULL / ovi.fps_num;

	char res[64];
	snprintf(res, sizeof(res), "%dx%dx%lld", (int)width, (int)height,
		 (long long)interval);

	obs_log(LOG_INFO, "Virtual output starting");

	char narrow_vcid[CHARS_IN_GUID];
	char stripped_vcid[CHARS_IN_GUID];
	wcstombs(narrow_vcid, vcam->vcid, sizeof(narrow_vcid));
	size_t vcidLen = strlen(narrow_vcid);
	strncpy(stripped_vcid, narrow_vcid + 1, vcidLen - 2);
	stripped_vcid[vcidLen - 2] = '\0';

	size_t filenameLen = strlen(stripped_vcid) + strlen(".txt") + 1;
	char *filename = (char *)malloc(filenameLen);
	snprintf(filename, filenameLen, "%s.txt", stripped_vcid);

	char *res_file = os_get_config_path_ptr(filename);
	os_quick_write_utf8_file_safe(res_file, res, strlen(res), false, "tmp",
				      NULL);
	bfree(res_file);

	vcam->vq = video_queue_create(width, height, interval, vcam->vcid);
	if (!vcam->vq) {
		obs_log(LOG_WARNING, "starting virtual-output failed");
		return false;
	}

	struct video_scale_info vsi = {0};
	vsi.format = VIDEO_FORMAT_NV12;
	vsi.width = width;
	vsi.height = height;
	obs_output_set_video_conversion(vcam->output, &vsi);

	os_atomic_set_bool(&vcam->active, true);
	os_atomic_set_bool(&vcam->stopping, false);
	obs_log(LOG_INFO, "Virtual output started");
	obs_output_begin_data_capture(vcam->output, 0);
	return true;
}

static void virtualcam_deactive(struct virtualcam_data *vcam)
{
	obs_output_end_data_capture(vcam->output);
	video_queue_close(vcam->vq);
	vcam->vq = NULL;

	os_atomic_set_bool(&vcam->active, false);
	os_atomic_set_bool(&vcam->stopping, false);

	obs_log(LOG_INFO, "Virtual output stopped");
}

static void virtualcam_stop(void *data, uint64_t ts)
{
	struct virtualcam_data *vcam = (struct virtualcam_data *)data;
	os_atomic_set_bool(&vcam->stopping, true);

	obs_log(LOG_INFO, "Virtual output stopping");

	UNUSED_PARAMETER(ts);
}

static void virtual_video(void *param, struct video_data *frame)
{
	struct virtualcam_data *vcam = (struct virtualcam_data *)param;

	if (!vcam->vq) {
		obs_log(LOG_WARNING, "virtualcam not initialized");
		return;
	}

	if (!os_atomic_load_bool(&vcam->active)) {
		obs_log(LOG_WARNING, "virtualcam not active");
		return;
	}

	if (os_atomic_load_bool(&vcam->stopping)) {
		virtualcam_deactive(vcam);
		return;
	}

	video_queue_write(vcam->vq, frame->data, frame->linesize,
			  frame->timestamp);
}

struct obs_output_info virtualcam_info = {
	.id = VCAM_OUTPUT_ID,
	.flags = OBS_OUTPUT_VIDEO,
	.get_name = virtualcam_name,
	.create = virtualcam_create,
	.destroy = virtualcam_destroy,
	.start = virtualcam_start,
	.stop = virtualcam_stop,
	.raw_video = virtual_video,
};
