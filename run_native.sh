# Script used to run zepher on local host
# If a bluetooth device is present it will be powered off and used

# To use this first build the zephyr project with the native_sim target:
# west build -b native_sim app -DOVERLAY_CONFIG="boards/native_sim.conf"

west build

if [ $? -eq 0 ]; then
    bluetoothctl power off
    sudo ./build/zephyr/zephyr.exe --bt-dev=hci0
fi
