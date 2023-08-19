# 定时器
## 概述
### 简介
定时器管理器，Timer启动后，可向定时器中注册多个定时事件，定时事件可以单次触发也可连续执行。

- Timer为毫秒级的高精度定时器，一般应用于短时的定时任务，不推荐应用于长时间的定时任务中，否则会带来一定的性能负担。

- Timer作为用户态的定时器没有在休眠状态下唤醒的能力，在休眠状态下无法进行正常的计数功能。

`#include <timer.h>`

## 涉及功能
### OHOS::Utils::Timer
#### 接口说明
|                | Name           |
| -------------- | -------------- |
| | **Timer**(const std::string& name, int timeoutMs = 1000)<br>Timer构造函数。在性能敏感的场景下，输入更大的timeoutMs。timeoutMs默认值是1000ms，性能消耗预估为每 一个timeoutMs中会消耗固定的100us。  |
| virtual | **~Timer**() |
| uint32_t | **Register**(const TimerCallback& callback, uint32_t interval, bool once = false)<br>注册定时事件。入参分别位定时响应函数，定时事件间隔时间，定时事件连续性。  |
| virtual uint32_t | **Setup**()<br>设置Timer。请勿在停止（Shutdown）前重复设置。  |
| virtual void | **Shutdown**(bool useJoin = true)<br>停止Timer。可配置阻塞式停止或者非阻塞式停止。阻塞式停止会等待Timer所有任务结束后停止Timer。 如果配置了timeoutMs为-1可以使用非阻塞式停止防止当前线程阻塞。  |
| void | **Unregister**(uint32_t timerId)<br>删除定时事件。  |

## 使用示例
1. 使用实例详见base/test/unittest/common/utils_timer_test.cpp
2. 测试用例编译运行方法：

- 启动开发者自测试框架，使用方法参见：[开发自测试执行框架-测试用例执行](https://gitee.com/openharmony/testfwk_developer_test#%E6%B5%8B%E8%AF%95%E7%94%A8%E4%BE%8B%E6%89%A7%E8%A1%8C)

- 在测试框架中使用以下具体命令以运行`timer.h`对应测试用例

```bash
run -t UT -tp utils -ts UtilsTimerTest
```
## 常见问题
1. Timer在使用前需要通过Setup()进行设置，在析构之前需要通过Shutdown()进行关闭。

1. Timer需要先设置再关闭。使用者应该避免分别在不同线程进行设置和关闭，否则会导致线程问题。

1. 重复设置Timer不会重置Timer的状态，Setup()接口会返回TIMER_ERR_INVALID_VALUE。 如果需要重置，那么请先关闭Timer再设置。

1. Shutdown接口的参数决定了Timer中的线程的阻塞与否，默认阻塞（true），若为false则非阻塞。非阻塞选项 可能会导致线程问题，因此不推荐。如果一定要使用非阻塞选项，请自行保证线程中对象的生命周期。

1. 如果定时任务中发生系统休眠，在休眠阶段Timer无法自唤醒，不会执行计数操作，因此会导致计时结果异常。