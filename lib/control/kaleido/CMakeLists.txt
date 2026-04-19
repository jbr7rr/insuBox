zephyr_library()
zephyr_library_compile_options(-Wall -Werror)
zephyr_library_include_directories(${CMAKE_CURRENT_SOURCE_DIR}/..)
zephyr_library_sources(KaleidoDevice.cpp)
