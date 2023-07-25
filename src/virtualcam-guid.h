#pragma once

#include <windows.h>
#include <initguid.h>
#include <strmif.h>

#define MAX_CAMERA 4

EXTERN_C const GUID CLSID_OBS_VirtualVideo;
EXTERN_C const GUID CLSID_OBS_VirtualVideo2;
EXTERN_C const GUID CLSID_OBS_VirtualVideo3;
EXTERN_C const GUID CLSID_OBS_VirtualVideo4;

EXTERN_C const GUID CLSID_OBS_VirtualAudio;
EXTERN_C const GUID CLSID_OBS_VirtualAudio2;
EXTERN_C const GUID CLSID_OBS_VirtualAudio3;
EXTERN_C const GUID CLSID_OBS_VirtualAudio4;

EXTERN_C const wchar_t *video_device_name[];
EXTERN_C const GUID *video_device_guid[];
EXTERN_C const wchar_t *video_device_guid_wchar[];
