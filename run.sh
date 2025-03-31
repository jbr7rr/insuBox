
# This script is used to flash and monitor the ESP32 board

# port
port="/dev/ttyACM0"

west flash --esp-device=$port
# only execute when previous command is successful
if [ $? -eq 0 ]; then
    west espressif monitor -p $port
fi
