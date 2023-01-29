# 线程安全阻塞队列

## 概述

#### 简介

​线程安全阻塞队列SafeBlockQueue类，提供阻塞和非阻塞版的入队入队和出队接口，并提供可最追踪任务完成状态的的SafeBlockQueueTracking类。

`#include <safe_block_queue.h>`
## 涉及功能

### 接口说明

### OHOS::SafeBlockQueue

| 返回值       | 名称                                                         |
| ------------ | ------------------------------------------------------------ |
|              | **SafeBlockQueue**(int capacity)<br/>构造函数                |
|              | virtual **~SafeBlockQueue**()<br/>析构函数                   |
| void         | virtual **Push**(T const& elem)<br/>入队操作(阻塞版)         |
| bool         | virtual **PushNoWait**(T const& elem)<br/>入队操作(非阻塞版) |
| T            | **Pop**()<br/>出队操作(阻塞版)                               |
| bool         | **PopNotWait**(T& outtask)<br/>出队操作(非阻塞版)            |
| unsigned int | **Size**()<br/>获取队列容量                                  |
| bool         | **IsEmpty**()<br/>队列判空                                   |
| bool         | **IsFull**()<br/>队列判满                                    |


### OHOS::SafeBlockQueueTracking
#### class SafeBlockQueueTracking : public SafeBlockQueue

| 返回值   | 名称                                                         |
| -------- | ------------------------------------------------------------ |
| explicit | **SafeBlockQueueTracking**(int capacity)<br/>构造函数        |
|          | virtual **~SafeBlockQueueTracking**()<br/>析构函数           |
| void     | virtual **Push**(T const& elem)<br/>入队操作(阻塞版)         |
| bool     | virtual **PushNoWait**(T const& elem)<br/>入队操作(非阻塞版) |
| bool     | **OneTaskDone**()<br/>一个任务完成时的响应函数               |
| void     | **Join**()<br/>等待未完成队列                                |
| int      | **GetUnfinishTaskNum**()<br/>获取未完成任务数                |


## 使用示例

1. 示例代码(伪代码)

- SafeBlockQueue的示例代码

```c++
#include <thread>
#include <functional>
#include <iostream>
#include "../include/safe_block_queue.h"

using namespace OHOS;
using namespace std;

constexpr int SIZE = 10;

class ProductsLine
{
public:
    ProductsLine(int maxSize) : que(maxSize) {}

    void Produce()
    {
        for (int i = 0; i < SIZE + 1; i++) {
            que.Push(i);
            cout << "Add " << i << " to the line" << endl;
        }
    }

    void Consume()
    {
        for (int i = 0; i < SIZE + 1; i++) {
            int out = que.Pop();
            cout << "Get " << out << " from the line" << endl;
        }
    }

    int remains()
    {
        return que.Size();
    }

private:
    SafeBlockQueue<int> que;
};

int main()
{
    ProductsLine line(SIZE);

    thread producer(bind(&ProductsLine::Produce, ref(line)));
    this_thread::sleep_for(chrono::milliseconds(1));

    thread consumer(bind(&ProductsLine::Consume, ref(line)));
    this_thread::sleep_for(chrono::milliseconds(1));

    producer.join();
    consumer.join();

    if (line.remains()==0) {
         cout << line.remains() << " elements remains in the queue. Synchronizing success." <<endl;
    }
}
```

- SafeBlockQueueTracking的示例代码

```c++
#include <thread>
#include <functional>
#include <iostream>
#include "../include/safe_block_queue.h"

using namespace OHOS;
using namespace std;

constexpr int SIZE = 10;

class ProductsLine
{
public:
    ProductsLine(int maxSize) : que(maxSize) {}

    void Produce()
    {
        for (int i = 0; i < SIZE + 1; i++) {
            que.Push(i);
            cout << "Add " << i << " to the line" << endl;
        }
    }

    void Consume()
    {
        for (int i = 0; i < SIZE + 1; i++) {
            int out = que.Pop();
            cout << "Get " << out << " from the line" << endl;
            que.OneTaskDone();
        }
    }

    void Join()
    {
        que.Join();
    }

    int UnfinishTaskNum()
    {
        return que.GetUnfinishTaskNum();
    }

private:
    SafeBlockQueueTracking<int> que;
};

int main()
{
    ProductsLine line(SIZE);

    thread producer(bind(&ProductsLine::Produce, ref(line)));
    this_thread::sleep_for(chrono::milliseconds(1));

    thread consumer(bind(&ProductsLine::Consume, ref(line)));
    this_thread::sleep_for(chrono::milliseconds(1));

    line.Join();

    producer.join();
    consumer.join();

    if (line.UnfinishTaskNum()==0) {
         cout << line.UnfinishTaskNum() << " elements remains in the queue. Synchronizing success." <<endl;
    }
}
```

2. 测试用例编译运行方法

- 测试用例代码参见base/test/unittest/common/utils_safe_block_queue_test.cpp 和 base/test/unittest/common/utils_safe_block_queue_tracking.cpp

- 使用开发者自测试框架，使用方法参见：[开发自测试执行框架-测试用例执行](https://gitee.com/openharmony/testfwk_developer_test#%E6%B5%8B%E8%AF%95%E7%94%A8%E4%BE%8B%E6%89%A7%E8%A1%8C)

- 使用以下具体命令以运行`safe_block_queue.h`对应测试用例
```bash
run -t UT -tp utils -ts UtilsSafeBlockQueueTest

# or

run -t UT -tp utils -ts UtilsSafeBlockQueueTrackingTest

```

## 常见问题
