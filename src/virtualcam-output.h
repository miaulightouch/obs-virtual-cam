#pragma once

#include <obs-module.h>

// define as same as old name.
#define VCAM_OUTPUT_ID "virtual_output"
#define VCAM_OUTPUT_NAME "VirtualOutput"

#ifdef __cplusplus
extern "C" {
#endif

extern struct obs_output_info virtualcam_info;

#ifdef __cplusplus
}
#endif
