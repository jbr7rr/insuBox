#!/bin/bash

# Script used to run the Kaleido simulator on local host
# If a bluetooth device is present it will be powered off and used

if [ ! -d "build_kaleido" ]; then
    west build -d build_kaleido -b native_sim/native/64 app -DOVERLAY_CONFIG="kaleido.conf"
else
    west build -d build_kaleido
fi

if [ $? -eq 0 ]; then
    bluetoothctl power off
    sudo ./build_kaleido/zephyr/zephyr.exe --bt-dev=hci0
fi
