#!/bin/bash

# This script is used to flash and monitor the the insubox rev zero board

port="/dev/ttyACM0"

if [ ! -d "build" ]; then
    west build -b insubox_rev_zero/esp32s3/procpu app -DOVERLAY_CONFIG="hw_insubox.conf" 
fi

west flash --esp-device=$port
if [ $? -eq 0 ]; then
    west espressif monitor -p $port
fi
