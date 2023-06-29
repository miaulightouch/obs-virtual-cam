# OBS-VirtualCam

Release: ![CI Windows Release](https://github.com/miaulightouch/obs-virtual-cam/actions/workflows/main.yml/badge.svg?event=release), CI: ![CI Windows On-Push](https://github.com/miaulightouch/obs-virtual-cam/actions/workflows/main.yml/badge.svg?event=push)

obs-virutalcam is a plugin for obs-studio , transforming the output video to a virtual directshow device.

source from [Fenrirthviti/obs-virtual-cam](https://github.com/Fenrirthviti/obs-virtual-cam)

**Supported Platforms** : Windows 10

**Supported OBS Studio version** : 29.0.0+

**Note: 32bit is not tested, but it should work.**

## Features

* **virtual output** : A output plugin sink raw video & audio to directshow interface.
* **virtual filter output** : A filter plugin sink obs source video to directshow interface.
* **virtual source** : Four directshow Interfaces which can use in 3rd party software.

## Install

The installer and compressed file can be found in [Release Page](https://github.com/miaulightouch/obs-virtual-cam/releases). Using installer is recommended, but if you want to use compressed file to install manually , please follow these instructions.

1. Unzip OBS-VirtualCam.zip and put it to your obs-studio install folder.
2. Run CMD as Administrator and register 32bit directshow source

```batch
regsvr32 "C:\Program Files\obs-studio\bin\32bit\obs-virtualsource.dll"
```

3. Do it again to register 64bit directshow source

```batch
regsvr32 "C:\Program Files\obs-studio\bin\64bit\obs-virtualsource.dll"
```

- If you want to Remove the directshow filter , you can also use regsvr32 to do this

```batch
regsvr32 /u "C:\Program Files\obs-studio\bin\32bit\obs-virtualsource.dll"
```

## Register specific number of virtual cameras

Unregister then register 2 directshow camera (up to 4)

```batch
regsvr32 /u "C:\Program Files\obs-studio\bin\64bit\obs-virtualsource.dll"
regsvr32 /n /i:"2" "C:\Program Files\obs-studio\bin\64bit\obs-virtualsource.dll"
```

## Build

this repo is updated to the latest OBS Plugin Template, please follow [the guide](https://github.com/obsproject/obs-plugintemplate).

## Donate

If you like this plugin, you can donate to original author via [Paypal.me](https://www.paypal.me/obsvirtualcam)
