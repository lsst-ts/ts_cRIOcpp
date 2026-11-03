target_sources(cRIOcpp
  PUBLIC
    FILE_SET HEADERS
    FILES
      rapidcsv.h
)

target_include_directories(cRIOcpp PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}/include
)

add_subdirectory(ILC)
add_subdirectory(Modbus)
add_subdirectory(NiFpga)
add_subdirectory(PID)
add_subdirectory(Transports)
add_subdirectory(cRIO)
