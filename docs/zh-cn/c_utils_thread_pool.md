# 线程池
## 概述


### 简介
提供线程安全的线程池功能。线程安全是对于线程池本身而非池内线程而言的。 维护一个任务队列，一个线程组。使用者向任务队列中注册需要进行的任务，线程组执行任务队列中的任务。 


`#include <thread_pool.h>`

Inherits from OHOS::NoCopyable


## 涉及功能
### OHOS::ThreadPool
#### 接口说明
|                | Name           |
| -------------- | -------------- |
| | **ThreadPool**(const std::string& name = std::string())<br>构造ThreadPool。为线程池内线程命名。  |
| | **~ThreadPool**() override |
| void | **AddTask**(const Task& f)<br>向任务队列中添加一个Task。若未调用Start()则直接执行Task且不会向任务队列添加该Task.  |
| size_t | **GetCurTaskNum**()<br>获取当前任务数。  |
| size_t | **GetMaxTaskNum**() const<br>获取最大任务数。  |
| std::string | **GetName**() const<br>获取线程池命名。  |
| size_t | **GetThreadsNum**() const<br>获取线程池内线程数。  |
| void | **SetMaxTaskNum**(size_t maxSize)<br>设置任务队列中最大任务数。  |
| uint32_t | **Start**(int threadsNum)<br>启动给定数量threadsNum的线程，执行任务队列中的任务。  |
| void | **Stop**()<br>停止线程池，等待所有线程结束。  |
## 使用示例
1. 使用实例详见base/test/unittest/common/utils_thread_pool_test.cpp
2. 测试用例编译运行方法：

- 启动开发者自测试框架，使用方法参见：[开发自测试执行框架-测试用例执行](https://gitee.com/openharmony/testfwk_developer_test#%E6%B5%8B%E8%AF%95%E7%94%A8%E4%BE%8B%E6%89%A7%E8%A1%8C)

- 在测试框架中使用以下具体命令以运行`thread_pool.h`对应测试用例

```bash
run -t UT -tp utils -ts UtilsThreadPoolTest
```
## 常见问题
1. 构造函数参数**name**会作为线程池内线程真实的线程名。线程名的形式：name + No。 线程名最大长度为16个字符（结束符'\0'计入），所以要格外注意name的长度。 比如，如果线程池内线程数量小于10，那么name的最大长度为14。