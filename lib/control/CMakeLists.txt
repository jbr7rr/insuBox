# SPDX-License-Identifier: Apache-2.0

zephyr_library()
zephyr_library_compile_options(-Wall -Werror)
zephyr_library_sources(ControlService.cpp)
zephyr_library_sources(bt_cts/bt_cts.cpp)
zephyr_library_sources(bt_ids/bt_ids.cpp)
zephyr_library_sources(bt_ids/InsulinDeliveryDevice.cpp)
