#ts_cRIOcpp

Common functions for C++ development.

## OSPL setup

To get history data, and faster/lower bandwidth communication, OSPL shall be
run on a machine for multiple users. For that, /tmp/ispddskey_XXXXXX file shall
be accessible to users requiring OSPL access. Please see
[OSPL_SETUP.md](OSPL_SETUP.md) for details.

## Command hierarchy, Controller processing

Provides abstract Command class. This shall be enqued into ControllerThread
internal Command queue. ControllerThread command queue is thread safe (access
to it is guarded by mutex and condition_variable). Please see [ControllerThread
tests](tests/test_ControllerThread.cpp) for examples.

## ILC communication

ILC (Inner-Loop Controller) is a board used on various M1M3 subsystems. ILCs
are commanded through serial Modbus connection. The connections are organized
on buses, operating on master-worker bus. Either unicasts or broadcasts
commands are supported. Broadcasts commands obviously doesn't return anything.

### Receiving data from ModBus

After command is send on ModBus, received data are held in a per bus dedicated
FIFO. Request code is available to copy content of this FIFO into response
FIFO. Synchronization of sending/receiving is done in FPGA class.

Response data are passed to ILC subclass in processResponse call. This method
check if responses matches non-broadcast request functions and addresses stored
in commanded buffer.

Data are send and received via [Common FPGA Modbus
module](https://github.com/lsst-ts/Common_FPGA_Modbus). Please see it for
details how Modbus frames are written as UINT 16 values on command FIFO and how
data are received from response FIFO.

### ILC Modbus Functions

ILC Modbus communication is described in LTS-646. Please see it for detailed
description of ILC functions.

ILCs are commanded using custom Modbus functions, which is assigning the
following codes (both decimal and hexadecimal code values are shown):

### Generic functions

ILCs provides those generic functions:

Code | Hex  | Error | Description
 --- | ---  | ----- | -----------
17   | 0x11 | 145   | Report Server ID
18   | 0x12 | 146   | Report Server Status
65   | 0x41 | 193   | Change ILC Mode
72   | 0x48 | 200   | Set Temp ILC Address
107  | 0x6b | 235   | Reset Server

### Firmware flash functions

Code | Hex  | Description
 --- | ---  | -----------
100  | 0x64 | Write Application Stats - writes CRC etc. after flashing)
101  | 0x65 | Erase ILC application
102  | 0x66 | Write Application Page
103  | 0x67 | Write Verify Application

### Pneumatic (FA) ILC functions

Code | Hex  | Description
 --- | ---  | -----------
68   | 0x44 | Freeze Sensor Values
73   | 0x49 | Set Boost Value DCA Gains
74   | 0x4a | Read Boost Value DCA Gains
75   | 0x4b | Force Demand
76   | 0x4c | Force & Status
80   | 0x50 | Set ADC scanrate
110  | 0x6e | Read callibration
119  | 0x77 | Read mezzanine pressure sensors
120  | 0x78 | Report DCA ID
121  | 0x79 | Report DCA Status

### Electromechanical (Hardpoint) ILC functions

Code | Hex  | Description
 --- | ---  | -----------
66   | 0x42 | Step motor move
67   | 0x4c | Force & Status
68   | 0x44 | Freeze Sensor Values
80   | 0x50 | Set ADC scanrate
81   | 0x51 | Set ADC Ch Offsets & Sensors
110  | 0x6e | Read callibration

### Hardpoint monitoring ILC functions

Code | Hex  | Description
 --- | ---  | -----------
119  | 0x77 | Read mezzanine pressure sensor
120  | 0x78 | Report mezzanine DCP ID
121  | 0x79 | Report mezzanine DCP Status
122  | 0x7a | Read DCP mezzanine board LVDT instruments

### Thermal ILC functions

Code | Hex  | Error | Description
 --- | ---  | ----- | -----------
88   | 0x58 | 216   | Thermal demand
89   | 0x59 | 217   | Thermal status

## FPGA

Commands are written into 2 bytes (16 bits) commandQueue FIFO in the following format:

Offse         | Description
 ------------ | -----------------------------------
0             | Command - see FPGA code for details
1             | payload length
2-data length | payload bytes

Where payload for Modbus uses highest 7 bits for operation code, next 8 bits
for data, and last bit as start bit (shall be always 0). See [Support System
FPGA](https://github.com/lsst-ts/ts_m1m3supportFPGA) and [Thermal System
FPGA](https://github.com/lsst-ts/ts_m1m3thermalFPGA) for
details.
