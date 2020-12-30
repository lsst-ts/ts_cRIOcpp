# ts_cRIOcpp

Common functions for C++ development.

## Command hierarchy, Controller processing

Provides abstract Command class. This shall be enqued into ControllerThread
internal Command queue. ControllerThread command queue is thread safe (access
to it is guarded by mutex and condition_variable). Please see [ControllerThread
tests](tests/test_ControllerThread.cpp) for examples.

## ILC communication

ILC (Inner-Loop Controllers) are boards used on various M1M3 subsystems. ILCs
are commanded through serial Modbus connection. The connections are organized
on buses, operating on master-worker bus. Either unicasts or broadcasts
commands are supported. Broadcasts commands obviously doesn't return anything.

ILCs are commanded using custom Modbus functions, which is assigning the
following codes (both decimal and hexadecimal code values are shown):

### Generic functions

ILCs provides those generic functions:

* Report Server ID (17 0x11)
* Report Server Status (18 0x12)
* Change ILC Mode (65 0x41)
* Set Temp ILC Address (72 0x48)
* Reset Server (107 0x6b)

### Firmware flash functions

* Write Application Stats (100 0x64) - writes CRC etc. after flashing)
* Erase ILC application (101 0x65)
* Write Application Page (102 0x66)
* Write Verify Application (103 0x67)

### Pneumatic (FA) ILC functions

* Freeze Sensor Values (68 0x44)
* Set Boost Value DCA Gains (73 0x49)
* Read Boost Value DCA Gains (74 0x4a)
* Force Demand (75 0x4b)
* Force & Status (76 0x4c)
* Set ADC scanrate (80 0x50)
* Read callibration (110 0x6e)
* Read mezzanine pressure sensors (119 0x77)
* Report DCA ID (120 0x78)
* Report DCA Status (121 0x79)

### Electromechanical (Hardpoint) ILC functions

* Step motor move (66 0x42)
* Force & Status (67 0x4c)
* Freeze Sensor Values (68 0x44)
* Set ADC scanrate (80 0x50)
* Set ADC Ch Offsets & Sensors (81 0x51)
* Read callibration (110 0x6e)

### Hardpoint monitoring ILC functions

* Read mezzanine pressure sensor (119 0x77)
* Report mezzanine DCP ID (120 0x78)
* Report mezzanine DCP Status (121 0x79)
* Read DCP mezzanine board LVDT instruments (122 0x7a)

### Thermal ILC functions

* Thermal demand (88 0x58)
* Thermal status (89 0x59)

