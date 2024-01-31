# 线程安全map

## 概述

### 简介
提供了一个线程安全的map实现。SafeMap在STL map基础上封装互斥锁，以确保对map的操作安全。

## 涉及功能
### class SafeMap
#### 接口说明


|返回类型           |名称           |
| -------------- | -------------- |
| | **SafeMap**() |
| | **SafeMap**(const SafeMap& rhs) |
| | **~SafeMap**() |
| void | **Clear**()<br>删除map中存储的所有键值对。  |
| void | **EnsureInsert**(const K& key, const V& value)<br>在map中插入元素。  |
| void | **Erase**(const K& key)<br>删除map中键为key的键值对。  |
| bool | **Find**(const K& key, V& value)<br>在map中查找元素。  |
| bool | **FindOldAndSetNew**(const K& key, V& oldValue, const V& newValue)<br>在map中查找元素并将key对应的`oldValue`替换为`newValue`。  |
| bool | **Insert**(const K& key, const V& value)<br>在map中插入新元素。  |
| bool | **IsEmpty**()<br>判断map是否为空。  |
| void | **Iterate**(const SafeMapCallBack& callback)<br>遍历map中的元素。  |
| SafeMap& | **operator=**(const SafeMap& rhs) |
| V | **ReadVal**(const K& key) <br> 线程安全地读map内元素|
| void | **ChangeValueByLambda**(const K& key, LambdaCallback callback) <br> 线程安全地操作safemap内元素，操作行为需要自定义|
| int | **Size**()<br>获取map的size大小。  |

## 使用示例

1. 示例代码(伪代码)

```c++
#include <thread>
#include <iostream>
#include <functional>
#include "../include/safe_map.h"

using namespace OHOS;
using namespace std;

constexpr int THREAD_NUM = 5;
thread threads[THREAD_NUM * 2];
SafeMap<int, string> sm;

bool InsertHandler(const int& key, const string& value)
{
    if (sm.Insert(key,value)) {
        cout << "Insert key: " << key << endl;
        return true;
    } else {
        return false;
    }
}

bool FindHandler(const int& key, string& value)
{
    if (sm.Find(key,value)) {
        cout << "Find key: " << key << " with value: " << value << endl;
        return true;
    } else {
        return false;
    }
}


int main()
{
    for (int i = 0; i < THREAD_NUM; i++) {
        string name = "Thread" + to_string(i);
        threads[i] = thread(&InsertHandler, i, name);
    }

    for (int i = 0; i < THREAD_NUM; i++) {
        string out;
        threads[i + THREAD_NUM] = thread(&FindHandler, i, ref(out));
    }

    this_thread::sleep_for(chrono::milliseconds(300));

    for (int i = 0; i < 2 * THREAD_NUM; i++) {
        threads[i].join();
    }


    if (sm.Size() == THREAD_NUM) {
        cout << "Insert to SafeMap success!" << endl;
    }
}
```

2. 测试用例编译运行方法

- 测试用例代码参见base/test/unittest/common/utils_safe_map_test.cpp

- 使用开发者自测试框架，使用方法参见：[开发自测试执行框架-测试用例执行](https://gitee.com/openharmony/testfwk_developer_test#%E6%B5%8B%E8%AF%95%E7%94%A8%E4%BE%8B%E6%89%A7%E8%A1%8C)

- 使用以下具体命令以运行`safe_map.h`对应测试用例

```bash
run -t UT -tp utils -ts UtilsSafeMapTest
```

3. 接口使用变更

- operator[]接口被废弃：该接口返回类型为引用，可以使用于读写场景，但在写场景情况下，该接口无法进行线程安全防护，因写行为本身基于引用由调用方触发，而非接口内部行为，接口内部持锁无法控制，因此失去线程安全的意义。

```cpp
SafeMap<int, string> sm;
// Thread 1:
sm[1] = "abc";

// Thread 2:
sm[2] = "def";
```

- 提供专用的线程安全读接口：ReadVal，该接口只具有safemap的读取能力，但保证线程安全

```cpp
SafeMap<string, int> sm;
// Thread 1:
sm.Insert("A", 1);
int val = sm.ReadVal("A");
```

- 针对之前利用operator[]返回引用进行的特殊元素操作行为，提供了接口ChangeValueByLambda进行替换，调用者自定义元素操作函数，可保证在线程安全的情况下操作safemap元素。

```cpp
SafeMap<string, std::set<int>> sm;
int val = 1;
auto fn = [&](std::set<int> &value) -> void { // 自定义callback
    value.emplace(val);
}
sm.ChangeValueByLambda("A", fn);
```