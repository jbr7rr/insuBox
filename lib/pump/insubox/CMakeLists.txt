
zephyr_library()
zephyr_library_compile_options(-Wall -Werror)
zephyr_library_sources(InsuBoxDevice.cpp)
zephyr_library_sources(motor/Motor.cpp)
zephyr_library_sources(posSensor/PosSensor.cpp)
