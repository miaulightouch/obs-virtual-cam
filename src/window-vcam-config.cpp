#include "virtualcam-guid.h"
#include "window-vcam-config.hpp"

OBSBasicVCamConfig::OBSBasicVCamConfig(VCam *_vcam, QWidget *parent)
	: vcam(_vcam),
	  QDialog(parent),
	  ui(new Ui::OBSBasicVCamConfig)
{
	// copy config from parent
	memcpy(&config, &vcam->config, sizeof(VCamConfig));

	setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

	ui->setupUi(this);

	ui->outputType->setCurrentIndex(config.type);
	OutputTypeChanged(config.type);
	connect(ui->outputType, SIGNAL(currentIndexChanged(int)), this,
		SLOT(OutputTypeChanged(int)));

	for (size_t i = 0; i < MAX_CAMERA; i++) {
		ui->vcamSelection->addItem(
			QString::fromStdWString(video_device_name[i]));
	}
	ui->vcamSelection->setCurrentIndex(config.vcamIndex);

	UpdateUIActive(vcam->VirtualCamActive());
	ui->autoStart->setChecked(config.autoStart);

	connect(ui->streamButton, SIGNAL(clicked()), this, SLOT(Stream()));

	connect(ui->buttonBox, SIGNAL(accepted()), this, SLOT(Accept()));
}

void OBSBasicVCamConfig::OutputTypeChanged(int type)
{
	auto list = ui->outputSelection;
	list->clear();

	switch ((VCamOutputType)type) {
	case VCamOutputType::InternalOutput:
		list->addItem(obs_module_text("Basic.VCam.InternalDefault"));
		list->addItem(obs_module_text("Basic.VCam.InternalPreview"));
		list->setCurrentIndex(config.internal);
		break;

	case VCamOutputType::SceneOutput: {
		// Scenes in default order
		BPtr<char *> scenes = obs_frontend_get_scene_names();
		for (char **temp = scenes; *temp; temp++) {
			list->addItem(*temp);

			if (config.scene.compare(*temp) == 0)
				list->setCurrentIndex(list->count() - 1);
		}
		break;
	}

	case VCamOutputType::SourceOutput: {
		// Sources in alphabetical order
		std::vector<std::string> sources;
		auto AddSource = [&](obs_source_t *source) {
			auto name = obs_source_get_name(source);

			if (!(obs_source_get_output_flags(source) &
			      OBS_SOURCE_VIDEO))
				return;

			sources.push_back(name);
		};
		using AddSource_t = decltype(AddSource);

		obs_enum_sources(
			[](void *data, obs_source_t *source) {
				auto &AddSource =
					*static_cast<AddSource_t *>(data);
				if (!obs_source_removed(source))
					AddSource(source);
				return true;
			},
			static_cast<void *>(&AddSource));

		// Sort and select current item
		sort(sources.begin(), sources.end());
		for (auto &&source : sources) {
			list->addItem(source.c_str());

			if (config.source == source)
				list->setCurrentIndex(list->count() - 1);
		}
		break;
	}
	}
}

void OBSBasicVCamConfig::SaveConfig()
{
	VCamOutputType type = (VCamOutputType)ui->outputType->currentIndex();
	switch (type) {
	case VCamOutputType::InternalOutput:
		config.internal =
			(VCamInternalType)ui->outputSelection->currentIndex();
		break;
	case VCamOutputType::SceneOutput:
		config.scene = ui->outputSelection->currentText().toStdString();
		break;
	case VCamOutputType::SourceOutput:
		config.source =
			ui->outputSelection->currentText().toStdString();
		break;
	default:
		// unknown value, don't save type
		return;
	}

	config.type = type;
	config.vcamIndex = ui->vcamSelection->currentIndex();
	config.autoStart = ui->autoStart->isChecked();

	vcam->SaveConfig(config);
}

void OBSBasicVCamConfig::Accept()
{
	SaveConfig();
	vcam->OnDialogClosed();
}

void OBSBasicVCamConfig::Stream()
{
	SaveConfig();

	if (vcam->VirtualCamActive()) {
		StopVirtualCam();
	} else {
		StartVirtualCam();
	}
}

void OBSBasicVCamConfig::StartVirtualCam()
{
	if (!vcam->StartVirtualCam()) {
		ui->streamWarning->setText(obs_module_text("Basic.VCam.Error"));
		ui->streamWarning->setStyleSheet("color: red");
	} else {
		ui->streamWarning->setText("");
		ui->streamWarning->setStyleSheet("");
	}
}

void OBSBasicVCamConfig::StopVirtualCam()
{
	if (vcam->VirtualCamActive())
		vcam->StopVirtualCam();
}

void OBSBasicVCamConfig::UpdateUIActive(bool active)
{
	ui->streamButton->setChecked(!active);
	ui->streamButton->setText(
		obs_module_text(active ? "Basic.Main.StopVirtualCam"
				       : "Basic.Main.StartVirtualCam"));
	ui->autoStart->setEnabled(!active);
	ui->vcamSelection->setEnabled(!active);
	ui->outputType->setEnabled(!active);
	ui->outputSelection->setEnabled(!active);
}
