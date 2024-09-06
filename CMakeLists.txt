cmake_minimum_required(VERSION 3.11)

project(ts_cRIOcpp VERSION 0.6.3 LANGUAGES CXX)

include(CMakePackageConfigHelpers)
include(CMakeDependentOption)
include(CheckCXXCompilerFlag)
include(GNUInstallDirs)
include(CTest)
include(FindPkgConfig)

find_program(CRIOCPP_CPP_CLANG_FORMAT_EXE NAMES clang-format)

pkg_search_module(SPDLOG REQUIRED spdlog)

option(CRIOCPP_BUILD_SHARED_LIBS "Build cRIOcpp shared library" ${BUILD_SHARED_LIBS})

cmake_dependent_option(CRIOCPP_INSTALL
    "Enable generation of cRIOcpp install targets" ON
  "CMAKE_SOURCE_DIR STREQUAL PROJECT_SOURCE_DIR" OFF)

set(criocpp-type STATIC)
set(criocpp-label-postfix "static")
if (CRIOCPP_BUILD_SHARED_LIBS)
  set(criocpp-type SHARED)
  set(criocpp-label-postfix "shared")
endif()

set(build-shared $<BOOL:${CRIOCPP_BUILD_SHARED_LIBS}>)

set(src-pattern "src/*.cpp")
if (CMAKE_VERSION VERSION_GREATER 3.12)
  list(INSERT src-pattern 0 CONFIGURE_DEPENDS)
endif()

file(GLOB_RECURSE criocpp-sources ${src-pattern})

add_library(ts_cRIOcpp ${criocpp-type} "")
add_library(ts_cRIOcpp::ts_cRIOcpp ALIAS ts_cRIOcpp)

set_property(TARGET ts_cRIOcpp 
  PROPERTY
    CXX_STANDARD_REQUIRED ON)

target_include_directories(ts_cRIOcpp
  SYSTEM PUBLIC
    ${SPDLOG_INCLUDE_DIRS}
  PUBLIC
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDE_DIRS}>
  PRIVATE
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/src>)

target_link_libraries(ts_cRIOcpp
    ${SPDLOG_LIBRARIES}
)

target_compile_options(ts_cRIOcpp
    PUBLIC ${SPDLOG_CFLAGS_OTHER})

if (NOT DEFINED CMAKE_CXX_STANDARD)
  set_target_properties(ts_cRIOcpp
    PROPERTIES
      CXX_STANDARD 17)
endif()

target_compile_options(ts_cRIOcpp
  PRIVATE
    -Wall -Wextra -Wshadow -Weffc++ -Wno-long-long
    -pedantic -pedantic-errors)

target_sources(ts_cRIOcpp
  PRIVATE
    ${criocpp-sources})

if (NOT DEFINED CMAKE_DEBUG_POSTFIX)
  set(CMAKE_DEBUG_POSTFIX "d")
endif()

set_target_properties(ts_cRIOcpp PROPERTIES
  VERSION "${PROJECT_VERSION}"
  SOVERSION "${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}"
  PROJECT_LABEL "criocpp ${criocpp-label-postfix}"
  DEBUG_POSTFIX "${CMAKE_DEBUG_POSTFIX}")

#configure_package_config_file(
#  "${PROJECT_SOURCE_DIR}/criocpp-config.cmake.in"
#  "${PROJECT_BINARY_DIR}/criocpp-config.cmake"
#  INSTALL_DESTINATION "${CMAKE_INSTALL_DATADIR}/cmake/cpp")

write_basic_package_version_file(
  "${PROJECT_BINARY_DIR}/criocpp-config-version.cmake"
  COMPATIBILITY AnyNewerVersion)

configure_file(criocpp.pc.in criocpp.pc @ONLY)

if (CRIOCPP_INSTALL)
	install(TARGETS ts_cRIOcpp
    EXPORT criocpp-targets
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR})
	install(DIRECTORY ${PROJECT_SOURCE_DIR}/include/
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
		FILES_MATCHING PATTERN "*.h")
            install(EXPORT criocpp-targets
    DESTINATION "${CMAKE_INSTALL_DATADIR}/cmake/criocpp")
#	install(FILES
#		"${PROJECT_BINARY_DIR}/criocpp-config.cmake"
#		"${PROJECT_BINARY_DIR}/yaml-cpp-config-version.cmake"
#    DESTINATION "${CMAKE_INSTALL_DATADIR}/cmake/yaml-cpp")
  install(FILES "${PROJECT_BINARY_DIR}/yaml-cpp.pc"
    DESTINATION ${CMAKE_INSTALL_DATADIR}/pkgconfig)
endif()

if (CRIOCPP_CPP_CLANG_FORMAT_EXE)
  add_custom_target(format
    COMMAND clang-format --style=file -i $<TARGET_PROPERTY:ts_cRIOcpp,SOURCES>
    COMMAND_EXPAND_LISTS
    COMMENT "Running clang-format"
    VERBATIM)
endif()
