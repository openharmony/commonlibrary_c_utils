# 单例模式
## 概述
### 简介
根据不同的需求提供了三种不同的单例模式。class DelayedSingleton 是一个线程安全、内存安全的懒汉式单例(用到了智能指针和锁)。class DelayedRefSingleton 是一个函数线程安全的懒汉式单例(用到了普通指针和锁)。class Singleton 是一个饿汉式单例（没有用指针和锁）。

## 涉及功能
### class DelayedSingleton
#### 接口说明
|返回类型            | 名称           |
| -------------- | -------------- |
| void | **DestroyInstance**()<br>释放智能指针托管对象的管理权。  |
| std::shared_ptr< T > | **GetInstance**()<br>创建唯一的实例对象并返回。  |

### class DelayedRefSingleton
#### 接口说明
|返回类型            | 名称           |
| -------------- | -------------- |
| T& | **GetInstance**()<br>创建唯一的实例对象并返回。  |
### class Singleton
#### 接口说明
|返回类型            | 名称           |
| -------------- | -------------- |
| T& | **GetInstance**()<br>返回唯一的实例对象。  |

## 使用示例

1. 示例代码(伪代码)

```c++
#include <iostream>
#include "../include/singleton.h"

using namespace OHOS;
using namespace std;

class DemoDelayedSingleton
{
    DECLARE_DELAYED_SINGLETON(DemoDelayedSingleton);
};

DemoDelayedSingleton :: DemoDelayedSingleton() {}
DemoDelayedSingleton :: ~DemoDelayedSingleton() {}

int main()
{
    shared_ptr<DemoDelayedSingleton> ds1 = DelayedSingleton<DemoDelayedSingleton>::GetInstance();
    shared_ptr<DemoDelayedSingleton> ds2 = DelayedSingleton<DemoDelayedSingleton>::GetInstance();

    if (ds1 == ds2) {
        cout << "Delayed singleton instances construct success!" << endl;
    }


    ds1.reset();
    ds2.reset();
    DelayedSingleton<DemoDelayedSingleton>::DestroyInstance();
    cout << "Delayed singleton instances destroy success!" << endl;

}
```

2. 测试用例编译运行方法

- 测试用例代码参见base/test/unittest/common/utils_singleton_test.cpp

- 使用开发者自测试框架，使用方法参见：[开发自测试执行框架-测试用例执行](https://gitee.com/openharmony/testfwk_developer_test#%E6%B5%8B%E8%AF%95%E7%94%A8%E4%BE%8B%E6%89%A7%E8%A1%8C)

- 使用以下具体命令以运行`singleton.h`对应测试用例

```bash
run -t UT -tp utils -ts UtilsSingletonTest
```

## 常见问题
调用`DestroyInstance()`方法后，`GetInstance()`方法将创建新的对象, 旧对象若存在外部的`std::shared_ptr`引用，则需要开发者自行释放以保证单例。