# 线程与并发知识

本文只记录 Thread、ThreadPool、RWLock、Semaphore 四类的边界约束和常见陷阱。线程安全注解宏的使用见 `thread_safety_analysis_macros.h`。

## 核心模型

```
Thread         — 单线程封装，通过 Run() 循环驱动
ThreadPool     — 线程池 + 任务队列，Start(n) 启动 n 个 worker
RWLock         — 读写锁，写优先模式（默认），写操作互斥，读操作可共享
Semaphore      — 基于 condition_variable 的计数信号量
```

## 边界/分类

| 概念 | 用途 | 常见误用 |
|---|---|---|
| `Thread::Start()` | 创建并启动线程，内部循环调用 `Run()` | 重复 Start 同一个 Thread 对象，会返回 INVALID_OPERATION |
| `Thread::NotifyExitSync()` | 同步通知线程退出，调用者阻塞到线程退出 | 在 Run() 中调用自己，死锁 |
| `Thread::NotifyExitAsync()` | 异步通知退出，调用者立即返回 | 线程退出时序不确定，资源可能还在使用 |
| `ThreadPool::Start(int n)` | 启动 n 个 worker 线程 | Start 前 AddTask 的任务会同步执行，不经过线程池 |
| `ThreadPool::Stop()` | 停止所有线程，清空队列 | Stop 后再 AddTask 行为未定义 |
| `RWLock::LockRead()` | 获取读锁，已持有写锁时直接返回 | 以为读锁一定会阻塞等到释放 |
| `Semaphore(int value)` | value=0 时 Wait 阻塞，>0 时表示可用资源数 | 当作互斥锁使用：Semaphore(1) 只允许一个线程进入 |

## 约束规则

- **不要**在并发测试中使用 `std::this_thread::sleep_for` 做同步，用 `std::latch` / `std::barrier` / condition variable。
- **不要**在 Run() 中调用 NotifyExitSync()，会死锁。
- **不要**对同一个 Thread 对象重复调用 Start()。
- **不要**把 Timer 的 thread_ detach（Shutdown(false)），除非确保相关对象生命周期。
- **必须**在 ThreadPool 析构前调用 Stop()。
- **必须**用 `UniqueWriteGuard` / `UniqueReadGuard` RAII 管理锁，不要手动 Lock/Unlock。
- **线程安全注解宏**是编译期检查，不要绕过或注释掉。

## 修改前检查

- 是否引入了 sleep_for 同步？
- 线程启动/停止的配对是否正确？
- 锁的获取/释放是否成对？
- 是否有多线程同时访问但没有保护的共享数据？

## 代码和测试

| 锚点 | 路径 |
|---|---|
| Thread 头文件 | `base/include/thread_ex.h` |
| ThreadPool 头文件 | `base/include/thread_pool.h` |
| RWLock 头文件 | `base/include/rwlock.h` |
| Semaphore 头文件 | `base/include/semaphore_ex.h` |
| 实现 | `base/src/` 对应 .cpp |
| 单测 | `UtilsThreadTest` `UtilsThreadPoolTest` `UtilsSemaphoreTest` `UtilsRWLockTest` `UtilsThreadSafetyAnalysisTest` |
