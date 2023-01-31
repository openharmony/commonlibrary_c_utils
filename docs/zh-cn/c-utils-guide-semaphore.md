# 增强信号量功能

## 概述

### 简介

信号量是一个具有原子性的计数器，可以作一把锁，以实现互斥、同步等功能；在多线程环境下使用，可以实现临界区代码不被并发调用或者限制并发的数量。信号量根据种类又可以分为有名信号量和无名信号量。

`#include <semaphore_ex.h>`

Inherits from OHOS::NoCopyable

## 涉及功能

#### 接口说明

### OHOS::NamedSemaphore

| 返回值 | 名称                                                         |
| ------ | ------------------------------------------------------------ |
|        | **NamedSemaphore**(size_t size)<br/>构造函数(指定信号量初始值) |
|        | **NamedSemaphore**(const std::string&, size_t)<br/>构造函数(指定信号量名字和初始值) |
|        | **~NamedSemaphore**()<br/>析构函数                           |
| bool   | **Create**()<br/>创建并初始化有名信号量                      |
| bool   | **Unlink**()<br/>将有名信号量文件从系统中删除                |
| bool   | **Open**()<br/>打开一个已经创建的有名信号量文件              |
| bool   | **Close**()<br/>关闭有名信号量                               |
| bool   | **Wait**()<br/>等待/获取信号量(信号量 -1)                    |
| bool   | **TryWait**()<br/>等待/获取信号量(信号量 -1)的接口；非阻塞版 |
| bool   | **TimedWait**(const struct timespec& ts)<br/>等待/获取信号量(信号量 -1)；指定阻塞时间版 |
| bool   | **Post**()<br/>释放信号量(信号量 +1)                         |
| int    | **GetValue**() const<br/>获取信号的值                        |

### OHOS::Semaphore 

| 返回值 | 名称                                                     |
| ------ | -------------------------------------------------------- |
|        | **Semaphore**(int value = 1) : count_(value)<br>构造函数 |
| void   | **Wait**()<br/>等待/获取信号量(信号量 -1)                |
| void   | **Post**()<br/>释放信号量(信号量 +1)                     |

## 使用示例

1. 示例代码

```c++
#include <functional>
#include <iostream>
#include <chrono>
#include "../include/semaphore_ex.h"

using namespace OHOS;
using namespace std;

constexpr int COUNT_MAX = 3;
constexpr int THREAD_NUM = 2 * COUNT_MAX;

class WorkSpace
{
public:
    WorkSpace(const string &name, int maxSize) : sema(name, maxSize), count(0), max(0)
    {
    }

    bool CreateSema()
    {
        return sema.Create();
    }

    ~WorkSpace()
    {
        sema.Close();
        sema.Unlink();
    }

    void Enter()
    {
        sema.Wait();
        count++;
        this_thread::sleep_for(chrono::microseconds(1));
        max = max > count ? max : count;
        count--;
        sema.Post();
    }

    bool Check()
    {
        if (COUNT_MAX >= max) {
            return true;
        }

        return false;
    }

    bool CloseAndUnlink()
    {
        return sema.Close() && sema.Unlink();
    }

    int GetCount()
    {
        return count;
    }

    int GetMax()
    {
        return max;
    }

private:
    NamedSemaphore sema;
    int count;
    int max;
};

int main()
{
    WorkSpace ws("mysem", COUNT_MAX);
    if (ws.CreateSema()) {
        cout << "NamedSemaphore created success!" << endl;
    } else {
        return 0;
    }
    thread threads[THREAD_NUM];
    for (int i = 0; i < THREAD_NUM; i++) {
        threads[i] = thread(&WorkSpace::Enter, ref(ws));
    }

    for (int i = 0; i < THREAD_NUM; i++) {
        threads[i].join();
    }

    if (ws.Check()) {
        cout << "Semaphore test success!" <<endl;
    }

    cout << "max: " << ws.GetMax() << " count: " << ws.GetCount() << endl;

    ws.CloseAndUnlink();
}
```

2. 测试用例编译运行方法

- 测试用例代码参见base/test/unittest/common/utils_semaphore_test.cpp

- 使用开发者自测试框架，使用方法参见：[开发自测试执行框架-测试用例执行](https://gitee.com/openharmony/testfwk_developer_test#%E6%B5%8B%E8%AF%95%E7%94%A8%E4%BE%8B%E6%89%A7%E8%A1%8C)

- 使用以下具体命令以运行`semaphore_ex.h`对应测试用例
```bash
run -t UT -tp utils -ts UtilsSemaphoreTest
```

## 常见问题

