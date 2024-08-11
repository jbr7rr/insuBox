# SPDX-License-Identifier: Apache-2.0

if(CONFIG_IB_PUMP)
    add_subdirectory_ifdef(CONFIG_IB_PUMP_MEDTRUM medtrum)

    zephyr_library()
    zephyr_library_compile_options(-Wall -Werror)
    zephyr_library_sources(PumpService.cpp)
endif()
