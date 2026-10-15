#cmake_minimum_required(VERSION 3.23)
#project(ts_cRIOcpp_tests LANGUAGES CXX)

enable_testing()

find_package(Catch2 3 REQUIRED)
find_package(yaml-cpp REQUIRED)

add_executable(cRIOcpp_tests)

target_compile_options(cRIOcpp_tests
  PRIVATE
    -Wl,--gc-sections -fPIC -fdata-sections -ffunction-sections
)

target_link_options(cRIOcpp_tests
  PRIVATE
    -Wl,--gc-sections -fPIC -fdata-sections -ffunction-sections
)

target_sources(cRIOcpp_tests
  PRIVATE
    TestFPGA.cpp
    # test_Alias.cpp
    # test_Application.cpp
    # test_CSC.cpp
    test_CliApp.cpp
    # test_ControllerThread.cpp
    # test_ElectromechanicalPneumaticILC.cpp
    # test_FPGACliApp.cpp
    # test_FirmwareLoad.cpp
    # test_ILCBusList.cpp
    # test_IntelHex.cpp
    test_Join.cpp
    # test_LimitTrigger.cpp
    # test_LimitedPID.cpp
    # test_MPU.cpp
    # test_MPUTelemetry.cpp
    # test_ModbusBuffer.cpp
    # test_Modbus_Buffer.cpp
    # test_Modbus_BusList.cpp
    # test_Modbus_Parser.cpp
    test_NiError.cpp
    test_PID.cpp
    # test_PrintILC.cpp
    # test_SensorMonitor.cpp
    # test_SimulatedILC.cpp
    # test_ThermalILC.cpp
    # test_Thread.cpp
    # test_Timestamp.cpp
    # test_simulated_transport.cpp
    #    test_version.cpp
)

target_sources(cRIOcpp_tests
  PRIVATE
    FILE_SET HEADERS
    FILES
      TestFPGA.h
)

target_link_libraries(cRIOcpp_tests
  PRIVATE
    cRIOcpp
    Catch2::Catch2WithMain
    yaml-cpp
    readline
)

#include(CTest)
#include(Catch)
#catch_discover_tests(cRIOcpp_tests)
