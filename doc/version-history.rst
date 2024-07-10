###############
Version History
###############


v1.11.1
-------
* SensorMonitor ILC
* broadcast commands to step and freeze
* SimpleFPGACliApp class

v1.11.0
-------
* Modbus::, ILC::ILCBusList classes. Communication code moved from ModbusBuffer
  to FPGA class.
* Improved documentation.
* Public constants to explain meaning of the numbers
* mpuCommands redesign

v1.10.1
-------

* Set and read DCA Gain commands (ILC code 73 and 74)
* Add NiWarning class, improve Ni error reporting
* FPGA communication debugging
* support for M2 command line client

v1.10.0
-------

* getStringStatus function
* handleMissingReply, MissingReply exception
* IRQTimeout handling

v1.9.0
------

* MPU changed to SerialMultiplex

v1.8.0
------

* LVDT ElectroMechanical readout

v1.7.0
------

* offsets,.. ILC commands
* improved re-heater gain management
