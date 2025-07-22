if(CONFIG_GOOGLETEST)
  # Do this here to avoid issues with the googletest module being
  # built or added when we don't need it.
  # googletest is fetched from the west.yml file.
  add_subdirectory("${PROJECT_SOURCE_DIR}/../googletest" "${CMAKE_CURRENT_BINARY_DIR}/googletest-build")
endif()
