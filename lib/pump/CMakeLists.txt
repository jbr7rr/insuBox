# SPDX-License-Identifier: Apache-2.0

add_subdirectory_ifdef(CONFIG_IB_PUMP_MEDTRUM_BT medtrum_bt)

zephyr_library()
zephyr_library_compile_options(-Wall -Werror)
zephyr_library_sources(PumpService.cpp)
zephyr_library_sources_ifdef(CONFIG_IB_PUMP_VIRTUAL VirtualPumpDevice.cpp)
