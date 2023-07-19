#pragma once

#include <QMainWindow>
#include <QAction>
#include <obs-module.h>
#include <obs-frontend-api.h>
#include "window-vcam.hpp"

class VCam {
public:
    bool init();
    VCamConfig config;
    bool VirtualCamActive();
    bool StartVirtualCam();
    void StopVirtualCam();
    void Stream();
    void SaveConfig(VCamConfig _config);
    void OnDialogClosed();
private:
    obs_output_t *output;
    obs_view_t *virtualCamView;
    obs_scene_t *vCamSourceScene;
    obs_sceneitem_t *vCamSourceSceneItem;
    video_t *virtualCamVideo;
    bool UIinit();
    void LoadConfig();
    bool IsConfigExist();
    void DestroyVirtualCamView();
    void DestroyVirtualCameraScene();
    void UpdateVirtualCamOutputSource();
    static inline void OnStart(void *, calldata_t * /* params */);
    static inline void OnStop(void *, calldata_t * /* params */);
    static inline void OnHostShutdown(obs_frontend_event event, void *private_data);
};
