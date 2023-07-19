#pragma once

// #include <obs.hpp>
#include <QDialog>
// #include <memory>

#include <obs-module.h>
#include <obs-frontend-api.h>
#include <util/util.hpp>
#include <util/platform.h>

#include "window-vcam.hpp"
#include "plugin-support.h"
#include "virtualcam.hpp"

#include "ui_OBSBasicVCamConfig.h"

struct VCamConfig;

class OBSBasicVCamConfig : public QDialog {
	Q_OBJECT

public:
	explicit OBSBasicVCamConfig(VCam *_vcam, QWidget *parent = 0);
	void UpdateUIActive(bool active);

private slots:
	void OutputTypeChanged(int type);
	void Accept();
	void Stream();

private:
	std::unique_ptr<Ui::OBSBasicVCamConfig> ui;
	VCam *vcam;
	VCamConfig config;

	void SaveConfig();
	void StartVirtualCam();
	void StopVirtualCam();
};
