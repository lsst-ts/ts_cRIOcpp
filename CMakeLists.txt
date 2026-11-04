cmake_minimum_required(VERSION 3.23)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED YES)

project(ts_cRIOcpp)

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
    "${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile"
    @ONLY
  )

  doxygen_add_docs(docs
    CONFIG_FILE "${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile"
    COMMENT "Generating docs using explicit Doxyfile template"
  )
else()
  message(WARNING "Doxygen not found, cannot generate docs.")
endif()
