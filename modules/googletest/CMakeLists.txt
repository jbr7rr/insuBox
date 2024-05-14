if(CONFIG_GOOGLETEST)
  cmake_minimum_required(VERSION 3.13.1)

  set(CMAKE_C_FLAGS -m32)
  set(CMAKE_CXX_FLAGS -m32)

  set(GOOGLETEST_DIR "${CMAKE_CURRENT_SOURCE_DIR}/_gtestCache")
  if(NOT EXISTS "${GOOGLETEST_DIR}/googletest")
    message(STATUS "Fetching GoogleTest into ${GOOGLETEST_DIR}")

    include(FetchContent)
    FetchContent_Declare(
      googletest
      GIT_REPOSITORY https://github.com/google/googletest.git
      GIT_TAG main
      SOURCE_DIR "${GOOGLETEST_DIR}/googletest"  # Specify where to download the source
    )

    FetchContent_MakeAvailable(googletest)
  else()
    message(STATUS "Using cached GoogleTest from ${GOOGLETEST_DIR}")
    add_subdirectory("${GOOGLETEST_DIR}/googletest" "${CMAKE_CURRENT_BINARY_DIR}/googletest-build")
  endif()
endif()
