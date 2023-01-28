# Thread Pool
## Overview


### Introduction
Give a thread-safe thread pool. The thread-safe is for threadpool itself not for the threads in pool. An task queue and a thread group are under control. Users add tasks to task queue. The thread group will execute the tasks in task queue. 


`#include <thread_pool.h>`

Inherits from OHOS::NoCopyable


## Related Interfaces
### OHOS::ThreadPool
#### Public Functions
| Return Type    | Name           |
| -------------- | -------------- |
| | **ThreadPool**(const std::string& name = std::string())<br>Construct ThreadPool. Name the threads in pool.  |
| | **~ThreadPool**() override |
| void | **AddTask**(const Task& f)<br>Add a Task to task queue. If Start() has never been called, the Task will be executed immediately.  |
| size_t | **GetCurTaskNum**()<br>Get the current amount of tasks in task queue.  |
| size_t | **GetMaxTaskNum**() const<br>Get the maximum amount of tasks in task queue.  |
| std::string | **GetName**() const<br>Get the name of ThreadPool.  |
| size_t | **GetThreadsNum**() const<br>Get the current amount of threads in pool.  |
| void | **SetMaxTaskNum**(size_t maxSize)<br>Set the maximum amount of tasks in task queue.  |
| uint32_t | **Start**(int threadsNum)<br>Start a given number(threadNum) of threads, which will execute the tasks in task queue.  |
| void | **Stop**()<br>Stop ThreadPool and waiting all threads in pool to stop.  |
## Examples
1. Examples can be seen in base/test/unittest/common/utils_thread_pool_test.cpp
2. Running unit test：

- Start developer test framework：[Developer Test - Using Test Framework](https://gitee.com/openharmony/testfwk_developer_test#using-test-framework)

- Run this command in the test framework to run the tests of `thread_pool.h`

```bash
run -t UT -tp utils -ts UtilsThreadPoolTest
```
## FAQ
1. **name**, the parameter of constructor, will be set as a part the real name of threads in pool. The real name of threads in pool will be like: name + No. The thread name is a meaningful C language string, whose length is restricted to 16 characters, including the terminating null byte ('\0'). Please pay attention to the length of name here. For example, if the number of threads in pool is less than 10, the maximum length of name is 14.