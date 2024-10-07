# SPDX-License-Identifier: Apache-2.0

zephyr_library()
zephyr_library_compile_options(-Wall -Werror)
zephyr_library_sources(ControlService.cpp)
