#include <obs-module.h>
#include <strsafe.h>
#include <strmif.h>
#include "virtualcam-guid.h"
#include "plugin-support.h"

OBS_DECLARE_MODULE()
OBS_MODULE_USE_DEFAULT_LOCALE(PLUGIN_NAME, "en-US")
MODULE_EXPORT const char *obs_module_description(void)
{
	return "OBS Virtual Camera";
}

extern "C" struct obs_output_info virtualcam_info;

bool obs_module_load(void)
{
	obs_register_output(&virtualcam_info);

	return true;
}
