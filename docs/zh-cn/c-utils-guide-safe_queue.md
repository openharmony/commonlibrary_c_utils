# 线程安全栈与队列

## 概述

### 简介

线程安全队列，是在dequeue的基础上封装std::lock_guard，以此实现线程的相关操作。根据继承SafeQueueInner抽象类，并对dequeue的pop方法的重写，可以实现SafeStack和SafeQueue的相关方法。

`#include <safe_queue.h>`

## 涉及功能

### 接口说明

### OHOS::SafeQueueInner 

| 返回值 | 名称                                                         |
| ------ | ------------------------------------------------------------ |
|        | **SafeQueueInner**()<br/>构造函数                            |
|        | virtual **~SafeQueueInner**()<br/>析构函数                   |
| void   | **Erase**(T& object)<br/>移除某个元素                        |
| bool   | **Empty**()<br/>队列判空                                     |
| void   | **Clear**()<br/>清空队列元素                                 |
| int    | **Size**()<br/>获取队列的容量                                |
| void   | **Push**(const T& pt)<br/>入队操作                           |
| void   | virtual void **DoPush**(const T& pt) = 0<br/>Push底层调用DoPush，需要重写 |
| bool   | **Pop**(T& pt)<br/>出队操作                                  |
| bool   | virtual **DoPop**(T& pt) = 0<br/>Push底层调用DoPop，需要重写 |

### OHOS::SafeQueue 
#### class SafeQueue : public SafeQueueInner 

| 返回值 | 名称                        |
| ------ | ------------------------------------ |
| void   | **DoPush**(const T& pt)<br/>入队操作 |
| bool   | **DoPop**(T& pt)<br/>出队操作        |

### OHOS::SafeStack 
#### class SafeStack : public SafeQueueInner

| 返回值 | 名称                                 |
| ------ | ------------------------------------ |
| void   | **DoPush**(const T& pt)<br/>入栈操作 |
| bool   | **DoPop**(T& pt)<br/>出栈操作        |

## 使用示例

1. 示例代码

```c++
#include <thread>
#include <iostream>
#include <functional>
#include "../include/safe_queue.h"

using namespace OHOS;
using namespace std;

constexpr int SIZE = 4;

int main() {
    SafeQueue<int> sq;
    SafeStack<int> st;

    for (int i = 0; i < SIZE; i++) {
        sq.Push(i);
        st.Push(i);
    }

    for (int i = 0; i < SIZE; i++) {
        int queOut;
        int stackOut;
        sq.Pop(queOut);
        st.Pop(stackOut);

        cout << "SafeQueue pop: " << queOut << endl;
        cout << "SafeStack pop: " << stackOut <<endl;
    }
}
```

2. 测试用例编译运行方法

- 测试用例代码参见base/test/unittest/common/utils_safe_queue_test.cpp

- 使用开发者自测试框架，使用方法参见：[开发自测试执行框架-测试用例执行](https://gitee.com/openharmony/testfwk_developer_test#%E6%B5%8B%E8%AF%95%E7%94%A8%E4%BE%8B%E6%89%A7%E8%A1%8C)

- 使用以下具体命令以运行`safe_queue.h`对应测试用例
```bash
run -t UT -tp utils -ts UtilsSafeQueueTest
```

## 常见问题
