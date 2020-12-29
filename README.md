# ts_cRIOcpp

Common functions for C++ development.

## Command hierarchy

Provides abstract Command class. This shall be enqued into ControllerThread
internal Command queue. ControllerThread command queue is thread safe (access
to it is guarded by mutex and condition_variable). Please see [ControllerThread
tests](tests/test_ControllerThread.cpp) for examples.
