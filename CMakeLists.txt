cmake_minimum_required(VERSION 3.11)

project(ts_cRIOcpp 
    VERSION 1.12.0
    HOMEPAGE_URL https://github.com/lsst-ts/ts_cRIOcpp
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 17)

include(CMakePackageConfigHelpers)
include(CMakeDependentOption)
include(CheckCXXCompilerFlag)
include(GNUInstallDirs)
include(FindPkgConfig)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH}
  ${PROJECT_SOURCE_DIR}/cmake)

configure_file(criocpp.pc.in criocpp.pc @ONLY)
configure_file(src/LSST/cRIO/version.cpp.in src/LSST/cRIO/version.cpp @ONLY)

find_program(CRIOCPP_CPP_CLANG_FORMAT_EXE NAMES clang-format)

find_package(spdlog NO_CMAKE_PACKAGE_REGISTRY)
if(NOT spdlog_FOUND)
  message("-- spdlog not found. Fetching it.")

  FetchContent_Declare(
    spdlog
    GIT_REPOSITORY https://github.com/gabime/spdlog
    GIT_TAG        v1.15.0
  )

  FetchContent_MakeAvailable(spdlog)
else()
  message("-- spdlog found. Using system's spdlog.")

  find_package(fmt NO_CMAKE_PACKAGE_REGISTRY)
  if (NOT fmt_FOUND)
    message("-- fmtlib not found. Fetching it.")

    FetchContent_Declare(
      fmt
      GIT_REPOSITORY https://github.com/fmtlib/fmt
      GIT_TAG        e69e5f977d458f2650bb346dadf2ad30c5320281
    ) # 10.2.1
    FetchContent_MakeAvailable(fmt)
  else()
    message("-- fmt lib found. Using system's fmt.")
  endif()

endif()

find_package(yaml-cpp)
if (NOT yaml-cpp_FOUND)
  message("-- yaml-cpp not found. Fetching it.")

  FetchContent_Declare(
    yaml-cpp
    GIT_REPOSITORY https://github.com/jbeder/yaml-cpp
    GIT_TAG        0.8.0
  )

  FetchContent_MakeAvailable(yaml-cpp)
else()
  message("-- yaml-cpp found. Using system's yaml-cpp.")
endif()

find_package(readline REQUIRED)

option(SHAREDLIB "Build cRIOcpp shared library" ${BUILD_SHARED_LIBS})

cmake_dependent_option(CRIOCPP_INSTALL
    "Enable generation of cRIOcpp install targets" ON
  "CMAKE_SOURCE_DIR STREQUAL PROJECT_SOURCE_DIR" OFF)

set(criocpp-type STATIC)
set(criocpp-label-postfix "static")
if (SHAREDLIB)
  set(criocpp-type SHARED)
  set(criocpp-label-postfix "shared")
endif()

set(build-shared $<BOOL:${SHAREDLIB}>)

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
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/src>
)

target_link_libraries(ts_cRIOcpp
    ${SPDLOG_LIBRARIES}
    ${FMT_LIBRARIES}
    ${YAML-CPP_LIBRARIES}
)

target_compile_options(ts_cRIOcpp
    PUBLIC 
      ${SPDLOG_CFLAGS_OTHER}
      ${FMT_CFLAGS_OTHER}
      ${YAML-CPP_CFLAGS_OTHER}
)

if (NOT DEFINED CMAKE_CXX_STANDARD)
  set(CMAKE_INSTALL_INCLUDE_DIRS 17)
endif()

add_compile_definitions(SPDLOG_FMT_EXTERNAL)

target_compile_options(ts_cRIOcpp
  PRIVATE
  -Wall -Wextra -Wshadow -Weffc++ -Wno-long-long -Wno-vla
    -pedantic -pedantic-errors)

target_sources(ts_cRIOcpp
  PRIVATE
      ${criocpp-sources} ${CMAKE_CURRENT_BINARY_DIR}/src/LSST/cRIO/version.cpp)

if (NOT DEFINED CMAKE_DEBUG_POSTFIX)
  set(CMAKE_DEBUG_POSTFIX "d")
endif()

set_target_properties(ts_cRIOcpp PROPERTIES
  VERSION "${PROJECT_VERSION}"
  SOVERSION "${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}"
  PROJECT_LABEL "criocpp ${criocpp-label-postfix}"
  DEBUG_POSTFIX "${CMAKE_DEBUG_POSTFIX}")

write_basic_package_version_file(
  "${PROJECT_BINARY_DIR}/criocpp-config-version.cmake"
  COMPATIBILITY AnyNewerVersion)

if (CRIOCPP_INSTALL)
    install(TARGETS ts_cRIOcpp
        EXPORT criocpp-targets
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    )

    install(DIRECTORY ${PROJECT_SOURCE_DIR}/include/
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
        FILES_MATCHING PATTERN "*.h")

    install(EXPORT criocpp-targets
        DESTINATION "${CMAKE_INSTALL_DATADIR}/cmake/criocpp")

    install(FILES
        "${PROJECT_BINARY_DIR}/criocpp-config.cmake"
        "${PROJECT_BINARY_DIR}/criocpp-config-version.cmake"
        DESTINATION "${CMAKE_INSTALL_DATADIR}/cmake/ts_cRIocpp")

    install(FILES "${PROJECT_BINARY_DIR}/ts_criocpp.pc"
        DESTINATION ${CMAKE_INSTALL_DATADIR}/pkgconfig)
endif()

if (CRIOCPP_CPP_CLANG_FORMAT_EXE)
  add_custom_target(format
    COMMAND clang-format --style=file -i $<TARGET_PROPERTY:ts_cRIOcpp,SOURCES>
    COMMAND_EXPAND_LISTS
    COMMENT "Running clang-format"
    VERBATIM)
endif()

message("-- installdir ${CMAKE_INSTALL_INCLUDEDIR}")

find_package(Catch2 REQUIRED)

if (NOT Catch2_FOUND)
  message("-- catch2 not found. Tests will be skipped.")
else()
  include(CTest)
  include(Catch)

  add_compile_definitions(CATCH_CONFIG_MAIN)

  file(GLOB test-sources ${PROJECT_SOURCE_DIR}/tests/*.cpp)
  add_executable(tests ${test-sources})
  # add_executable(tests ${PROJECT_SOURCE_DIR}/tests/test_CSC.cpp ${PROJECT_SOURCE_DIR}/tests/TestFPGA.cpp )

  catch_discover_tests(tests
    WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}/tests
  )

  target_link_libraries(tests ts_cRIOcpp yaml-cpp readline spdlog fmt Catch2::Catch2 pthread)
  target_include_directories(tests
    SYSTEM PUBLIC
    PUBLIC
      $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
      $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/tests>
      $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDE_DIRS}>
  )
endif()
