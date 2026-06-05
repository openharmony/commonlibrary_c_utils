# 事件处理知识

本文记录 IOEventHandler、IOEventReactor、Observer/Observable、Timer 的边界约束。

## 核心模型

```
IOEventReactor    — 基于 epoll 的事件循环，管理一组 IOEventHandler
IOEventHandler    — 每个 fd + event mask + callback
Observer/Observable — 观察者模式，Obs 通知所有 Observer
Timer             — 定时器管理器，基于 EventReactor + timerfd
```

## 边界/分类

### IO 事件系统

| 概念 | 用途 | 常见误用 |
|---|---|---|
| `IOEventHandler(fd, events, cb)` | 绑定 fd + 事件类型 + 回调 | fd 已关闭但未 Stop，导致 epoll 监听无效 fd |
| `Start(reactor)` | 注册到 reactor | 重复注册同一个 handler |
| `Stop(reactor)` | 从 reactor 注销 | 在回调中 Stop 自己可能 crash |
| `Update(reactor)` | 修改监听的事件类型 | 与 Start 并发导致竞态 |
| EnableRead/EnableWrite | 动态启用读/写事件 | 不调用 Update 不会生效 |

### 观察者模式

| 概念 | 用途 | 常见误用 |
|---|---|---|
| Observable::NotifyObservers | 遍历 obs 集合调用 Update | 在 Update 回调中 AddObserver/RemoveObserver 导致迭代器失效 |
| Observable::AddObserver | 添加观察者 | 已经在集合中时直接返回，不报错 |
| Observable::SetChanged/ClearChanged | 控制是否真的通知 | NotifyObservers 前忘记 SetChanged，通知被跳过 |

### Timer

| 概念 | 用途 | 常见误用 |
|---|---|---|
| `Timer::Setup()` | 启动定时器线程 | Shutdown 前重复 Setup 返回错误 |
| `Timer::Shutdown(bool useJoin)` | 停止定时器，useJoin=true 阻塞等线程退出 | useJoin=false 时 thread_ detach，相关对象可能已析构 |
| `Timer::Register(cb, interval, once)` | 注册定时事件 | interval=0 高频触发会占满 CPU |
| `Timer::Unregister(timerId)` | 注销定时事件 | 在回调中 Unregister 自己可能造成问题 |
| timeoutMs 参数 | epoll_wait 超时，默认 1000ms | -1 表示无限等待（如果没事件会永久阻塞），0 表示不等待（CPU 空转） |

## 约束规则

- **不要**在 IOEventHandler 的回调中 Stop/销毁自己所在的 reactor。
- **不要**在 NotifyObservers 的 Update 回调中添加或删除观察者。
- **不要**对 Timer 使用 `Shutdown(false)` 除非能保证 detach 后的线程不访问已析构对象。
- **不要**将 interval 设为 0——timerfd 会高频触发，占满 CPU。
- **不要**重复 Setup 同一个 Timer 对象。
- **必须**在 Timer 析构前调用 Shutdown()。

## 修改前检查

- epoll 事件回调中是否有重入风险？
- Observer 的 Update 中是否修改了 obs 集合？
- Timer 的 Shutdown 和析构顺序是否正确？
- timeout=-1 时是否会无限阻塞？

## 代码和测试

| 锚点 | 路径 |
|---|---|
| IOEventHandler 头文件 | `base/include/io_event_handler.h` |
| IOEventReactor 头文件 | `base/include/io_event_reactor.h` |
| IOEventCommon 头文件 | `base/include/io_event_common.h` |
| Observer 头文件 | `base/include/observer.h` |
| Timer 头文件 | `base/include/timer.h` |
| 实现 | `base/src/event_handler.cpp` 等 |
| 单测 | `UtilsEventTest` `UtilsObserverTest` `UtilsTimerTest` |
| 模糊测试 | `base/test/fuzztest/timer_fuzzer/` |
