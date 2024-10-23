# SPDX-License-Identifier: Apache-2.0

zephyr_library()
zephyr_library_compile_options(-Wall -Werror)
zephyr_library_sources(HmiService.cpp)
zephyr_library_sources_ifdef(CONFIG_IB_HMI_VIRTUAL VirtualHmiDevice.cpp)

if(CONFIG_IB_HMI_VIRTUAL)
  message(WARNING "CONFIG_IB_HMI_VIRTUAL is enabled. 
  This option is not intended for production use, as it accepts BT pairing requests without displaying a password.")
endif()
