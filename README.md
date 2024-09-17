# insuBox
DIY Loop system contained in a small box

## Description

insuBox is a DIY Loop system which targets embedded systems, so the hardware size can be minimized. It uses ZephyrOS as a base and can run a variety of hardware.

Intended use case is as a stand alone loop system, so one don't need to carry a phone to run the loop.

## Features

WIP
Initial target features:
- Medtrum Nano support
- Dexcom G6 support
- Algorithm based on OpenAPS but simplified (no carb input)
- Some way to control the system? (remote app or whatever)
- Synchronization with Nightscout? (maybe by remote app, if we eliminate the need of wifi, we can make it really small and last longer)

# How to build

This software is still in development, so it's not ready for use yet.

WIP

General developer notes:
- Install zephyr toolchain etc : https://docs.zephyrproject.org/latest/develop/getting_started/index.html#install-the-zephyr-sdk
- Clone this repo in a folder in which you want to have the zephyr project (zephyr will fetch zephyr and other modules in the top dir of the repo)
run in your repo folder:
```
west init -l .
cd ..
west update
``` 

- Build the project for your target board, for example:
https://docs.zephyrproject.org/latest/boards/seeed/xiao_esp32s3/doc/index.html

# Run unit tests

Apply the patch in the `patches` folder to the zephyr repo (only first time needed):
```
cd ../zephyr
git apply ../insuBox/patches/fix_gtest_harness.patch
```

Run the tests (from the repo dir):
```
west twister -T tests --integration
```
