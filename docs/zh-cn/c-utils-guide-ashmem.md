# 使用匿名共享内存(Ashmem, Anonymous Shared Memory)

## 概述

使用linux内核提供的驱动`dev/ashmem`，申请内核空间中的一块内存区域并映射至用户空间。利用其共享机制，可以实现较大数据量的进程间通信。

## 涉及功能
### OHOS::Ashmem

C++公共基础库中的Ashmem实现，用于操作匿名共享内存(Ashmem)。 

#### 具体描述

```cpp
class OHOS::Ashmem;
```
具体操作包括创建并映射一块ashmem内存区域并对其进行读写等。
**提示**: 尽管被智能指针管理，Ashmem对象仍需手动解除映射并关闭。 


`#include <ashmem.h>`

继承自 OHOS::RefBase

#### Public Functions

| 返回类型       | 名称           |
| -------------- | -------------- |
| sptr< Ashmem > | **CreateAshmem**(const char* name, int32_t size)<br>使用指定名称及大小创建Ashmem对象。  |
| | **Ashmem**(int fd, int32_t size)<br>构造Ashmem对象。  |
| | **~Ashmem**() override |
| void | **CloseAshmem**()<br>通过文件描述符关闭当前ashmem。  |
| int | **GetAshmemFd**() const<br>获取内核中对应ashmem的文件描述符。  |
| int32_t | **GetAshmemSize**()<br>获取内核中ashmem区域的大小。  |
| int | **GetProtection**()<br>获取内核中的ashmem区域的保护权限值。  |
| bool | **MapAshmem**(int mapType)<br>将内核中的ashmem内存区域映射至用户空间。  |
| bool | **MapReadAndWriteAshmem**()<br>以读/写模式映射ashmem内存区域。  |
| bool | **MapReadOnlyAshmem**()<br>以只读模式映射ashmem内存区域。  |
| const void * | **ReadFromAshmem**(int32_t size, int32_t offset)<br>从ashmem内存区域`offset`处读出数据。  |
| bool | **SetProtection**(int protectionType)<br>设置内核中的ashmem区域的保护权限。  |
| void | **UnmapAshmem**()<br>解除ashmem映射。  |
| bool | **WriteToAshmem**(const void* data, int32_t size, int32_t offset)<br>在ashmem内存区域`offset`处写入数据。  |


## 使用示例

1. 使用方法(伪代码)

```c++
sptr<Ashmem> ashmem = Ashmem::CreateAshmem(MEMORY_NAME.c_str(), MEMORY_SIZE);
if (ashmem != nullptr) {
    bool ret = ashmem->MapAshmem(PROT_READ | PROT_WRITE);
}

...

// 当使用结束时不要忘记解映射和关闭ashmem
ashmem->UnmapAshmem();
ashmem->CloseAshmem();
```

2. 测试用例编译运行方法

- 测试用例代码参见 base/test/unittest/common/utils_ashmem_test.cpp

- 使用开发者自测试框架，使用方法参见：[开发自测试执行框架-测试用例执行](https://gitee.com/openharmony/testfwk_developer_test#%E6%B5%8B%E8%AF%95%E7%94%A8%E4%BE%8B%E6%89%A7%E8%A1%8C)

- 使用以下具体命令以运行`ashmem.h`对应测试用例

```bash
run -t UT -tp utils -ts UtilsAshmemTest
```

## 常见问题

1. Ashmem对象使用结束后**需要手动解除映射并关闭**
    * 智能指针仅管理Ashmem对象的析构，而Ashmem对象析构时不会解除内存空间的映射并关闭。
    * 使用`UnmapAshmem()`解除映射并使用`CloseAshmem()`关闭。
