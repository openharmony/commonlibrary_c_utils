# 读写锁

## 概述

### 简介

读写锁，即共享独占锁，读时共享，写时独占。读的时候写阻塞，写的时候读阻塞。而且在写优先模式下，读写操作在竞争锁的时候，会优先得到写锁。

`#include <rwlock.h>`

Inherits from OHOS::NoCopyable

## 涉及功能

### 接口说明

### OHOS::RWLOCK

| 返回类型 | 名称                                                     |
| -------- | -------------------------------------------------------- |
|          | **RWLock**() : RWLock(true)<br>构造函数                  |
|          | **RWLock**(bool writeFirst)<br/>构造函数(指定读优先模式) |
|          | **~RWLock**()<br/>析构                                   |
| void     | **LockRead**()<br/>获取读锁                             |
| void     | **UnLockRead**()<br/>释放读锁                           |
| void     | **LockWrite**()<br/>获取写锁                            |
| void     | **UnLockWrite**()<br/>释放写锁                          |

### OHOS::UniqueWriteGuard

| 返回类型 | 名称                                                      |
| -------- | --------------------------------------------------------- |
|          | **UniqueWriteGuard**(RWLockable &rwLockable)<br/>构造函数 |
|          | **~UniqueWriteGuard**()<br/>析构函数                      |

### OHOS::UniqueReadGuard

| 返回类型 | 名称                                                     |
| -------- | -------------------------------------------------------- |
|          | **UniqueReadGuard**(RWLockable &rwLockable)<br/>构造函数 |
|          | **~UniqueReadGuard**()<br/>析构函数                      |

## 使用示例


1. 测试用例编译运行方法

- 测试用例代码参见base/test/unittest/common/utils_rwlock_test.cpp

- 使用开发者自测试框架，使用方法参见：[开发自测试执行框架-测试用例执行](https://gitee.com/openharmony/testfwk_developer_test#%E6%B5%8B%E8%AF%95%E7%94%A8%E4%BE%8B%E6%89%A7%E8%A1%8C)

- 使用以下具体命令以运行`rwlock.h`对应测试用例
```bash
run -t UT -tp utils -ts UtilsRWLockTest
```

## 常见问题
