# 管理、传递文件描述符

## 概述
使用c_utils提供的文件描述符封装类而非直接使用整型的文件描述符数值。其使得文件描述符在变量、函数间进行传递时能够保持仅被唯一一个封装类对象持有。传递过程中若一个文件描述符未被一个封装类对象持有，其将被关闭，以避免重复关闭等问题。

## 涉及功能
### OHOS::UniqueFdAddDeletor

文件描述符管理封装类。用于实现对文件描述符的唯一管理，以避免重复关闭等问题。 

#### 具体描述

```cpp
template <typename Deleter = DefaultDeleter>
class OHOS::UniqueFdAddDeletor;
```

**模板参数**: 

* **Deleter** 指定Deletor类，以提供具体关闭文件描述符的方法。默认为DefaultDeleter。 

定义上，一个UniqueFdAddDeletor对象唯一管理一个文件描述符。 文件描述符的管理权可以在不同的UniqueFdAddDeletor对象间传递，当文件描述符没有任何UniqueFdAddDeletor接替管理时，其将被关闭。 

`#include <unique_fd.h>`

#### Public Functions

| 返回类型                                                               | 名称                                                                                                                                          |
| ------------------------------------------------------------------ | ------------------------------------------------------------------------------------------------------------------------------------------- |
|                                                                    | **UniqueFdAddDeletor**()<br>无参构造UniqueFdAddDeletor对象，此时待管理文件描述符值为-1。                                                                        |
|                                                                    | **UniqueFdAddDeletor**(const int& value)<br>构造UniqueFdAddDeletor对象，传入待管理文件描述符。                                                             |
|                                                                    | **UniqueFdAddDeletor**(UniqueFdAddDeletor&& rhs)<br>移动构造函数。用于文件描述符在UniqueFdAddDeletor对象间的传递。 |
|                                                                    | **~UniqueFdAddDeletor**()<br>析构函数。将调用`UniqueFdAddDeletor::Reset(-1)`以关闭当前文件描述符并置为-1                                                         |
| int                                                                | **Get**() const<br>获取当前管理的文件描述符，但并不释放对其的管理。                                                                                                 |
|                                                                    | **operator int**() const<br>类型转换运算符。                                                                                                        |
| bool                                                               | **operator!=**(const int& rhs) const<br>不等运算符重载函数。将当前对象管理的文件描述符与输入值比较。                                                                     |
| bool                                                               | **operator<**(const int& rhs) const<br>小于运算符重载函数。将当前对象管理的文件描述符与输入值比较。                                                                      |
| bool                                                               | **operator<=**(const int& rhs) const<br>小于等于运算符重载函数。将当前对象管理的文件描述符与输入值比较。                                                                   |
| UniqueFdAddDeletor& | **operator=**(UniqueFdAddDeletor&& rhs)<br>移动赋值运算符重载函数。用于文件描述符在UniqueFdAddDeletor对象间的传递。     |
| bool                                                               | **operator==**(const int& rhs) const<br>相等运算符重载函数。将当前对象管理的文件描述符与输入值比较。                                                                     |
| bool                                                               | **operator>**(const int& rhs) const<br>大于运算符重载函数。将当前对象管理的文件描述符与输入值比较。                                                                      |
| bool                                                               | **operator>=**(const int& rhs) const<br>大于等于运算符重载函数。将当前对象管理的文件描述符与输入值比较。                                                                   |
| int                                                                | **Release**()<br>释放对当前文件描述符的管理，将当前被管理文件描述符置为-1。                                                                                             |

#### Friends

|      | 名称                                                                                                                                                                                   |
| ---- | ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------ |
| bool | **operator**(const int& lhs, const UniqueFdAddDeletor< Deleter >& rhs)                                                               |
| bool | **operator**(const int& lhs, const UniqueFdAddDeletor< Deleter >& rhs)                                                               |
| bool | **operator!=**(const int& lhs, const UniqueFdAddDeletor< Deleter >& rhs) <br>全局不等运算符重载函数，比较输入具体数值与输入UniqueFdAddDeletor对象所管理的文件描述符。   |
| bool | **operator==**(const int& lhs, const UniqueFdAddDeletor< Deleter >& rhs) <br>全局相等运算符重载函数，比较输入具体数值与输入UniqueFdAddDeletor对象所管理的文件描述符。   |
| bool | **operator>**(const int& lhs, const UniqueFdAddDeletor< Deleter >& rhs) <br>全局大于运算符重载函数，比较输入具体数值与输入UniqueFdAddDeletor对象所管理的文件描述符。    |
| bool | **operator>=**(const int& lhs, const UniqueFdAddDeletor< Deleter >& rhs) <br>全局大于等于运算符重载函数，比较输入具体数值与输入UniqueFdAddDeletor对象所管理的文件描述符。 |

### OHOS::DefaultDeleter

Deleter默认实现类，包含关闭文件描述符的静态方法。 

#### 具体描述

```cpp
class OHOS::DefaultDeleter;
```

Deleter默认实现类，包含关闭文件描述符的静态方法。 

**提示**: 管理封装类UniqueFdAddDeletor在取消对当前描述符的管理，同时又没有其他管理封装类接替管理该描述符时，`Deleter::Close()`方法将被调用。 

Deleter类用于关闭文件描述符，可以重新实现以自定义对文件描述符的关闭行为。 

`#include <unique_fd.h>`

#### Public Functions

| 返回类型 | 名称                                |
| ---- | --------------------------------- |
| void | **Close**(int fd)<br>默认文件描述符关闭方法。 |

## 使用示例

1. 测试用例编译运行方法

- 测试用例代码参见 base/test/unittest/common/utils_unique_fd_test.cpp

- 使用开发者自测试框架，使用方法参见：[开发自测试执行框架-测试用例执行](https://gitee.com/openharmony/testfwk_developer_test#%E6%B5%8B%E8%AF%95%E7%94%A8%E4%BE%8B%E6%89%A7%E8%A1%8C)

- 使用以下具体命令以运行`unique_fd.h`对应测试用例

```bash
run -t UT -tp utils -ts UtilsUniqueFdTest
```
