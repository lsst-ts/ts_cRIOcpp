# ts_cRIOcpp

Common functions for C++ development.

## Command hierarchy

Provides abstract Command class. This shall be enqued into ControllerThread
internal Command queue. ControllerThread command queue is thread safe (access
to it is quarded by mutex and condition_variable).

```cpp
class TestCommand:public Command
{
public:
    void execute() override { ..do something.. }

};
// run controller thread
std::thread controller([] { ControllerThread::get().run(); });

// enqueue command into controller thread
ControllerThread::get().enqueue(new TestCommand());

std::this_thread::sleep_for(100ms);

// stop controller thread
ControllerThread::get().stop();

controller.join();
```
