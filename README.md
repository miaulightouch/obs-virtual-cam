# OBS-VirtualCam

![CI Windows On-Push](https://github.com/miaulightouch/obs-virtual-cam/actions/workflows/main.yml/badge.svg)

obs-virutalcam is a plugin for obs-studio , transforming the output video to a virtual directshow device.

It's modified from official win-dshow plugin, but with up to 4 cameras for multiple obs instance.

The audio is not supported yet.

**Supported Platforms** : Windows 10, Windows 11

**Supported OBS Studio version** : 30.0.0+

## Features

* **virtual output** : A output plugin sink raw video to directshow interface.

## Install

The installer and compressed file can be found in [Release Page](https://github.com/miaulightouch/obs-virtual-cam/releases). Using installer is recommended, but if you want to use compressed file to install manually , please follow these instructions.

1. Unzip obs-virtualcam-*-windows-x64.zip and put it to your obs-studio install folder.
2. Run CMD as Administrator and register 32bit directshow source

```batch
regsvr32 "C:\Program Files\obs-studio\data\obs-plugins\obs-virtualcam\obs-virtualcam-module32.dll"
```

3. Do it again to register 64bit directshow source

```batch
regsvr32 "C:\Program Files\obs-studio\data\obs-plugins\obs-virtualcam\obs-virtualcam-module64.dll"
```

- If you want to Remove the directshow filter , you can also use regsvr32 to do this

```batch
regsvr32 /u "C:\Program Files\obs-studio\data\obs-plugins\obs-virtualcam\obs-virtualcam-module32.dll"
regsvr32 /u "C:\Program Files\obs-studio\data\obs-plugins\obs-virtualcam\obs-virtualcam-module64.dll"
```

## Register specific number of virtual cameras

Unregister then register 2 directshow camera (up to 4)

```batch
regsvr32 /u "C:\Program Files\obs-studio\data\obs-plugins\obs-virtualcam\obs-virtualcam-module64.dll"
regsvr32 /n /i:"2" "C:\Program Files\obs-studio\data\obs-plugins\obs-virtualcam\obs-virtualcam-module64.dll"
```

## Build

this repo is updated to the latest OBS Plugin Template, please follow [the guide](https://github.com/obsproject/obs-plugintemplate).
