#pragma once

// The FFmpeg DLLs this DirectShow source needs are delay-loaded from a
// private "obs-virtualcam" folder next to it. They must not live directly in
// OBS's bin folder: they share names with DLLs that older OBS releases
// shipped, and the OBS updater deletes those names on every update.
#define FFMPEG_PRIVATE_DIR L"obs-virtualcam"

// Loads the delay-loaded FFmpeg DLLs; returns false if they are missing.
bool ffmpeg_loaded();
