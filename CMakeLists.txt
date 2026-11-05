cmake_minimum_required(VERSION 3.23)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED YES)

find_package(Git REQUIRED)

execute_process(
  COMMAND ${GIT_EXECUTABLE} describe --tags --always --match "v[0-9]*"
  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
  OUTPUT_VARIABLE GIT_TAG_VERSION
  OUTPUT_STRIP_TRAILING_WHITESPACE
  ERROR_QUIET
)

if(GIT_TAG_VERSION MATCHES "^v([0-9]+\.[0-9]+\.[0-9]*).*")
  set(PROJECT_VERSION_CLEAN "${CMAKE_MATCH_1}")
else()
  set(PROJECT_VERSION_CLEAN "0.0.1")
  set(GIT_TAG_VERSION "v0.0.1-testbuild")
endif()

project(ts_cRIOcpp
  VERSION ${PROJECT_VERSION_CLEAN}
  LANGUAGES CXX
)

configure_file(
  "${CMAKE_CURRENT_SOURCE_DIR}/include/cRIO/version.h.in"
  "${CMAKE_CURRENT_BINARY_DIR}/include/cRIO/version.h"
)

include(GNUInstallDirs)
install(
  FILES "${CMAKE_CURRENT_BINARY_DIR}/include/cRIO/version.h"
  DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

add_library(cRIOcpp)

find_package(spdlog REQUIRED)
target_link_libraries(cRIOcpp PRIVATE spdlog::spdlog)

target_compile_options(cRIOcpp
  PRIVATE
  -Wall -Wl,--gc-sections -fPIC -fdata-sections -ffunction-sections
)

add_subdirectory(include)
add_subdirectory(src)

add_subdirectory(tests)

find_package(Doxygen)
if(Doxygen_FOUND)
  configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile.in"
    "${CMAKE_CURRENT_BINARY_DIR}/Doxyfile"
  )

  doxygen_add_docs(docs
    CONFIG_FILE "${CMAKE_CURRENT_BINARY_DIR}/Doxyfile"
    COMMENT "Generating docs using explicit Doxyfile template"
  )
else()
  message(WARNING "Doxygen not found, cannot generate docs.")
endif()
