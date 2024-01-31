# 强化线程能力

## 概述

### 简介

线程类提供例如启动线程、同步通知、异步通知等功能的接口

`#include <thread_ex.h>`

## 涉及功能

### 接口说明

### OHOS::Thread


|返回值类型 | 名称                                                       |
| ------------ | ------------------------------------------------------------ |
|              | **Thread**()<br/>构造函数, 构造一个Thread对象，但并不会启动线程。 |
|              | virtual **~Thread**()<br/>析构函数                           |
| ThreadStatus | **Start**(const std::string& name, int32_t priority = THREAD_PROI_NORMAL, size_t stack = 0);<br/>创建并启动一个子线程，循环执行Run()，当Run()返回false或通知退出时停止。 |
| ThreadStatus | **NotifyExitSync**()<br/>同步通知线程退出,即阻塞式停止子线程。<br/>当前线程被阻塞，等待子线程结束。 |
| void         | virtual **NotifyExitAsync**()<br/>异步通知线程退出，即子线程退出与否不阻塞当前线程。<br/>通知子线程停止，当前线程继续运行。 |
| bool         | virtual **ReadyToWork**()<br/>判断线程是否已经准备就绪，始终返回true。 |
| bool         | **IsExitPending**() const<br/>获取线程退出待定标志位。       |
| bool         | **IsRunning**() const<br/>判断线程是否在运行                 |
| pthread_t    | **GetThread**() const<br/>获取线程ID                         |


## 使用示例

1. 测试用例编译运行方法

- 测试用例代码参见base/test/unittest/common/utils_thread_test.cpp

- 使用开发者自测试框架，使用方法参见：[开发自测试执行框架-测试用例执行](https://gitee.com/openharmony/testfwk_developer_test#%E6%B5%8B%E8%AF%95%E7%94%A8%E4%BE%8B%E6%89%A7%E8%A1%8C)

- 使用以下具体命令以运行`thread_ex.h`对应测试用例
```bash
run -t UT -tp utils -ts UtilsThreadTest
```

## 常见问题

- 主线程对象生命周期终止前，一定要调用NotifyExitSync或NotifyExitAsync终止子线程运行，否则子线程在主线程对象消亡后继续工作，而由于Run函数由主线程对象真正实现，此时主线程对象消亡，Run函数会调用虚基类的纯虚函数而报错。

```cpp
class RealThread : public Thread { // 使用方继承虚基类并实现Run函数
    bool Run() override;
};

{
    std::unique_ptr<RealThread> test = std::make_unique<RealThread>();
    ThreadStatus status = test->Start("test_thread_01", THREAD_PROI_LOW, 1024); // 创建并启动子线程对象
    test->NotifyExitSync(); // 在test对象生命周期结束前，一定要终止子线程的继续运行
}

```
