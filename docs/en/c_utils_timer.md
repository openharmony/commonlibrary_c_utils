# Timer
## Overview
### Introduction
This is a timer manager. After "Timer" started, users can register several timed events, which can be continuous or once, to it. 

- Timer is a millisecond-level high-precision timer, which is generally used for short-term timing tasks. It is not recommended for long-term timing tasks, otherwise it will bring a certain performance burden.

- As a timer in user mode, Timer does not have the ability to wake up in the sleep state, and cannot perform normal counting functions in the sleep state.

`#include <timer.h>`

## Related Interfaces
### OHOS::Utils::Timer
#### Public Functions
| Return Type    | Name           |
| -------------- | -------------- |
| | **Timer**(const std::string& name, int timeoutMs = 1000)<br>Construct Timer. If performance-sensitive, change "timeoutMs" larger before Setup. "timeoutMs" default-value(1000ms), performance-estimate: occupy fixed-100us in every default-value(1000ms).  |
| virtual | **~Timer**() |
| uint32_t | **Register**(const TimerCallback& callback, uint32_t interval, bool once = false)<br>Regist timed events.  |
| virtual uint32_t | **Setup**()<br>Set up "Timer". Do not set up repeatly before shutdown.  |
| virtual void | **Shutdown**(bool useJoin = true)<br>Shut down "Timer". There are two modes to shut the "Timer" down: blocking and unblocking. Blocking mode will shut "Timer" down until all running events in "Timer" finished. If "timeoutMs" is set as -1, use unblocking mode to avoid deadloop.  |
| void | **Unregister**(uint32_t timerId)<br>Delete a timed events.  |
## Examples
1. Examples can be seen in base/test/unittest/common/utils_timer_test.cpp
2. Running unit test：

- Start developer test framework：[Developer Test - Using Test Framework](https://gitee.com/openharmony/testfwk_developer_test#using-test-framework)

- Run this command in the test framework to run the tests of `timer.h`

```bash
run -t UT -tp utils -ts UtilsTimerTest
```
## FAQ
1. Timer should be set up(via Setup()) before use, and shutdown(via Shutdown()) before its deconstruction.

1. Timer should be set up first and then shutdown. Avoid delegating them to different threads since it may cause multithreading problem.

1. Set up Timer again would not reset this Timer, but return `TIMER_ERR_INVALID_VALUE`. If a reset operation is required, shut Timer down first and then set it up.

1. Parameter in Shutdown() determines whether the thread in Timer would be detach or join. (True(default) --> join; False --> detach). Detach operation would cause possible multithreading problems, thus is not recommended. If a detach operation is required, availability of related objects used in `thread_` should be guaranteed.

1. If the system sleeps during the scheduled task, the Timer cannot wake up automatically during the sleep phase, and will not perform counting operations, which will cause abnormal timing results.

## Typical Cases
1. The unregister of Timer has a critical situation. The deregistration of the Timer may trigger the deletion of the corresponding event at the time when the event is responded. In this case, an additional callback response may be triggered.

```cpp
// pseudocode
Timer timer("timer_test");
CallBack func;
timer.Setup();
uint32_t timerId = timer.Register(func, 1000); // The scheduled response is called every one minute. Assume that the scheduled response takes effect at 00:00.
......
/*
Event deletion is triggered at 1:00, which is in the critical state. If the polling thread obtains the response event earlier than the deletion action, it will find that the response event is triggered once more after deregistration. If the polling thread responds later than the deletion, the polling thread is not aware of the event response. After the deregistration, the corresponding event is not triggered again.
*/
timer.Unregister(timerId);

```

2. To save resources and improve performance, timerFd is reused for events with the same interval. As a result, the response time of some events may be different from the expected time. If developers have strong requirements on the response time, we advise to set interval to a slightly different time (for example, 1 ms).

```cpp
// pseudocode
Timer timer("timer_test");
CallBack func1;
CallBack func2;
timer.Setup();

// Assume that the start time is 00:00, the subsequent responses of func1 are 01:00, 02:00, 03:00, 04:00...
uint32_t timerId_1 = timer.Register(func1, 1000); // One-minute cyclic response callback.

// Assume that the start time of func2 is 0:30 and the expected follow-up responses are 1:30, 2:30, 3:30, 4:30...
// However, timer fd is multiplexed and shared with func1. The subsequent time response is the same as that of func1, which is 1:00, 2:00, 3:00, 4:00...
uint32_t timerId_2 = timer.Register(func2, 1000); // The timerfd is multiplexed with the timerfd of func1.

// Assume that the start time of func2 is 0:30, the subsequent responses of func2 are about (with 1ms deviation) 1:30, 2:30, 3:30, 4:30...
uint32_t timerId_3 = timer.Register(func2, 1001); // Func2 has its own timerfd.
```