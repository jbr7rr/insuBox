#!/bin/bash

# Script used to run on local host
# If a bluetooth device is present it will be powered off and used

if [ ! -d "build_native" ]; then
    west build -d build_native -b native_sim/native/64 app -DOVERLAY_CONFIG="boards/native_sim.conf" 
else
    west build -d build_native
fi

if [ $? -eq 0 ]; then
    bluetoothctl power off
    sudo ./build_native/zephyr/zephyr.exe --bt-dev=hci0
fi
