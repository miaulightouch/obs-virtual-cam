#include "virtualcam.hpp"
#include "virtualcam-output.h"
#include "window-vcam-config.hpp"

OBSBasicVCamConfig *dialog;

inline void VCam::OnStart(void *, calldata_t * /* params */)
{
	if (dialog)
		dialog->UpdateUIActive(true);
}

inline void VCam::OnStop(void *data, calldata_t * /* params */)
{
	VCam *vcam = static_cast<VCam *>(data);

	if (dialog)
		dialog->UpdateUIActive(false);

	// obs_output_set_media(vcam->output, nullptr, nullptr);
	vcam->DestroyVirtualCamView();
}

inline void VCam::OnHostShutdown(obs_frontend_event event, void *private_data)
{
	if (event == OBS_FRONTEND_EVENT_SCRIPTING_SHUTDOWN) {
		VCam *vcam = static_cast<VCam *>(private_data);
		if (vcam->VirtualCamActive())
			vcam->StopVirtualCam();
	}
}

bool VCam::UIinit()
{
	QAction *action = (QAction *)obs_frontend_add_tools_menu_qaction(
		obs_module_text("Basic.VCam.VirtualCamera"));

	auto showDialog = [&]() {
		QMainWindow *main_window = static_cast<QMainWindow *>(
			obs_frontend_get_main_window());

		obs_frontend_push_ui_translation(obs_module_get_string);
		dialog = new OBSBasicVCamConfig(this, main_window);
		obs_frontend_pop_ui_translation();

		dialog->exec();
	};

	action->connect(action, &QAction::triggered, showDialog);
	return true;
}

bool VCam::init()
{
	obs_register_output(&virtualcam_info);

	if (!IsConfigExist()) {
		SaveConfig(config);
	} else {
		LoadConfig();
	}

	if (config.autoStart)
		StartVirtualCam();

	obs_frontend_add_event_callback(OnHostShutdown, this);

	return UIinit();
}

bool VCam::VirtualCamActive()
{
	if (!output)
		return false;

	return obs_output_active(output);
}

void VCam::UpdateVirtualCamOutputSource()
{
	if (!virtualCamView)
		return;

	// FIXME: it may need release
	obs_source_t *source;

	switch (config.type) {
	case VCamOutputType::InternalOutput:
		DestroyVirtualCameraScene();
		switch (config.internal) {
		case VCamInternalType::Default:
			source = obs_get_output_source(0);
			break;
		case VCamInternalType::Preview:
			source = obs_frontend_get_current_scene();
			break;
		}
		break;
	case VCamOutputType::SceneOutput:
		DestroyVirtualCameraScene();
		source = obs_get_source_by_name(config.scene.c_str());
		break;
	case VCamOutputType::SourceOutput:
		// FIXME: it may need release
		obs_source_t *s =
			obs_get_source_by_name(config.source.c_str());

		if (!vCamSourceScene)
			vCamSourceScene =
				obs_scene_create_private("vcam_source");
		source = obs_source_get_ref(
			obs_scene_get_source(vCamSourceScene));

		if (vCamSourceSceneItem &&
		    (obs_sceneitem_get_source(vCamSourceSceneItem) != s)) {
			obs_sceneitem_remove(vCamSourceSceneItem);
			vCamSourceSceneItem = nullptr;
		}

		if (!vCamSourceSceneItem) {
			vCamSourceSceneItem = obs_scene_add(vCamSourceScene, s);

			obs_sceneitem_set_bounds_type(vCamSourceSceneItem,
						      OBS_BOUNDS_SCALE_INNER);
			obs_sceneitem_set_bounds_alignment(vCamSourceSceneItem,
							   OBS_ALIGN_CENTER);

			const struct vec2 size = {
				(float)obs_source_get_width(source),
				(float)obs_source_get_height(source),
			};
			obs_sceneitem_set_bounds(vCamSourceSceneItem, &size);
		}
		// obs_source_release(s);
		break;
	}

	// FIXME: it may need release
	obs_source_t *current = obs_view_get_source(virtualCamView, 0);
	if (source != current)
		obs_view_set_source(virtualCamView, 0, source);

	// obs_source_release(source);
	// obs_source_release(current);
}

void VCam::Stream()
{
	if (VirtualCamActive()) {
		StopVirtualCam();
	} else {
		StartVirtualCam();
	}
}

void VCam::SaveConfig(VCamConfig _config)
{
	config = _config;

	config_t *profile = obs_frontend_get_profile_config();
	config_set_bool(profile, VCAM_OUTPUT_ID, "autoStart", config.autoStart);
	config_set_uint(profile, VCAM_OUTPUT_ID, "vcamIndex", config.vcamIndex);
	config_set_uint(profile, VCAM_OUTPUT_ID, "type", config.type);
	config_set_uint(profile, VCAM_OUTPUT_ID, "internal", config.internal);
	config_set_string(profile, VCAM_OUTPUT_ID, "scene", config.scene.c_str());
	config_set_string(profile, VCAM_OUTPUT_ID, "source", config.source.c_str());

	config_save(profile);
}

bool VCam::IsConfigExist()
{
	config_t *profile = obs_frontend_get_profile_config();
	return config_has_user_value(profile, VCAM_OUTPUT_ID, "autoStart");
}

void VCam::LoadConfig()
{
	config_t *profile = obs_frontend_get_profile_config();

	config.autoStart = config_get_bool(profile, VCAM_OUTPUT_ID, "autoStart");
	config.vcamIndex = config_get_uint(profile, VCAM_OUTPUT_ID, "vcamIndex");
	config.type = (VCamOutputType)config_get_uint(profile, VCAM_OUTPUT_ID, "type");
	config.internal = (VCamInternalType)config_get_uint(profile, VCAM_OUTPUT_ID, "internal");
	config.scene = config_get_string(profile, VCAM_OUTPUT_ID, "scene");
	config.source = config_get_string(profile, VCAM_OUTPUT_ID, "source");

	if (VirtualCamActive()) {
		StopVirtualCam();
		StartVirtualCam();
	}
}

bool VCam::StartVirtualCam()
{
	if (output)
		StopVirtualCam();

	obs_data_t *option = obs_data_create();
	obs_data_set_int(option, "vcamIndex", config.vcamIndex);

	output = obs_output_create(VCAM_OUTPUT_ID, VCAM_OUTPUT_NAME,
				option, nullptr);
	signal_handler_t *handler = obs_output_get_signal_handler(output);
	signal_handler_connect(handler, "start", OnStart, nullptr);
	signal_handler_connect(handler, "stop", OnStop, this);
	obs_data_release(option);

	if (!virtualCamView)
		virtualCamView = obs_view_create();

	UpdateVirtualCamOutputSource();

	if (!virtualCamVideo) {
		virtualCamVideo = obs_view_add(virtualCamView);

		if (!virtualCamVideo)
			return false;
	}

	obs_output_set_media(output, virtualCamVideo, obs_get_audio());

	bool success = obs_output_start(output);
	if (!success)
		DestroyVirtualCamView();

	return success;
}
void VCam::StopVirtualCam()
{
	if (!output)
		return;

	obs_output_stop(output);
	obs_output_release(output);
	output = nullptr;
}

void VCam::DestroyVirtualCamView()
{
	obs_view_remove(virtualCamView);
	obs_view_set_source(virtualCamView, 0, nullptr);
	virtualCamVideo = nullptr;

	obs_view_destroy(virtualCamView);
	virtualCamView = nullptr;

	DestroyVirtualCameraScene();
}

void VCam::DestroyVirtualCameraScene()
{
	if (!vCamSourceScene)
		return;

	obs_scene_release(vCamSourceScene);
	vCamSourceScene = nullptr;
	vCamSourceSceneItem = nullptr;
}

void VCam::OnDialogClosed()
{
	dialog = nullptr;
}

/* ----------------------------------------------- */

OBS_DECLARE_MODULE()
OBS_MODULE_USE_DEFAULT_LOCALE(PLUGIN_NAME, "en-US")

bool obs_module_load(void)
{
	VCam *vcam = new VCam();

	return vcam->init();
}
