#pragma once

#include <string>

enum VCamOutputType {
	InternalOutput,
	SceneOutput,
	SourceOutput,
};

enum VCamInternalType {
	Default,
	Preview,
};

struct VCamConfig {
	bool autoStart = false;
	uint16_t vcamIndex = 0;
	VCamOutputType type = VCamOutputType::InternalOutput;
	VCamInternalType internal = VCamInternalType::Default;
	std::string scene;
	std::string source;
};
