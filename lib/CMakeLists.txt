# SPDX-License-Identifier: Apache-2.0

add_subdirectory_ifdef(CONFIG_IB_BLE_COMM ble)
add_subdirectory_ifdef(CONFIG_IB_CONTROL control)
add_subdirectory_ifdef(CONFIG_IB_HMI hmi)
add_subdirectory_ifdef(CONFIG_IB_PUMP pump)

