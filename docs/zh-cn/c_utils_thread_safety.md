# 线程安全分析宏
## 概述


### 简介
提供基于 clang 的编译期线程安全分析（Thread Safety Analysis，TSA）注解宏。这些宏用于标注互斥锁、受保护数据、函数的前置/后置条件等，帮助编译器在编译阶段发现数据竞争和锁使用错误。在非 clang 编译器下，这些宏会被展开为空操作，以保持兼容性。


`#include <thread_safety_analysis_macros.h>`


## 涉及功能
### 能力（Capability）类宏
用于标注锁或可锁对象。

| 宏 | 说明 |
| --- | --- |
| **CAPABILITY(x)** | 标注类/结构体为可锁能力（如互斥锁）。用法：`class Foo CAPABILITY("mutex") { ... };` |
| **REENTRANT_CAPABILITY** | 标注可重入（允许递归加锁）的可锁类 |
| **SCOPED_CAPABILITY** | 标注 RAII 类（如锁守卫），在构造函数中获取能力，在析构函数中释放 |

### 数据保护宏
用于标注受锁保护的变量。

| 宏 | 说明 |
| --- | --- |
| **GUARDED_BY(x)** | 标注成员变量由某互斥锁保护，访问该变量时需持有该锁 |
| **PT_GUARDED_BY(x)** | 类似 GUARDED_BY，用于指针变量，表示指针所指向的数据由指定互斥锁保护 |

### 锁序宏
用于声明锁的获取顺序，避免死锁。

| 宏 | 说明 |
| --- | --- |
| **ACQUIRED_BEFORE(...)** | 声明该能力必须在其后列出的能力之前获取 |
| **ACQUIRED_AFTER(...)** | 声明该能力必须在其后列出的能力之后获取 |

### 函数前置条件宏
描述调用函数前需满足的锁状态。

| 宏 | 说明 |
| --- | --- |
| **REQUIRES(...)** | 调用前必须持有指定的互斥锁（独占锁） |
| **REQUIRES_SHARED(...)** | 调用前必须持有指定的共享锁（读锁） |
| **EXCLUDES(...)** | 调用时不得持有指定的互斥锁，避免在持锁时调用此类函数引发死锁 |

### 函数锁操作宏
描述函数内部对锁的获取与释放。

| 宏 | 说明 |
| --- | --- |
| **ACQUIRE(...)** | 函数会获取（加锁）指定的互斥锁（独占） |
| **ACQUIRE_SHARED(...)** | 函数会以共享模式获取（加读锁）指定的互斥锁 |
| **RELEASE(...)** | 函数会释放（解锁）指定的互斥锁（此前以独占模式持有） |
| **RELEASE_SHARED(...)** | 函数会释放（解锁）以共享模式持有的互斥锁 |
| **RELEASE_GENERIC(...)** | 函数会释放互斥锁（兼容独占或共享模式），用于通用解锁函数 |
| **TRY_ACQUIRE(...)** | 函数会尝试获取互斥锁，第一个参数为布尔值，表示返回 true 时表示成功 |
| **TRY_ACQUIRE_SHARED(...)** | 函数会尝试以共享模式获取互斥锁 |

### 断言与返回宏
用于断言和返回值标注。

| 宏 | 说明 |
| --- | --- |
| **ASSERT_CAPABILITY(x)** | 断言当前线程持有指定锁 |
| **ASSERT_SHARED_CAPABILITY(x)** | 断言当前线程以共享模式持有指定锁 |
| **RETURN_CAPABILITY(x)** | 标注函数返回其持有的锁的引用或指针（常用于返回 capability 的 getter） |
| **NO_THREAD_SAFETY_ANALYSIS** | 关闭对当前函数或代码块的线程安全分析，用于抑制误报或分析器无法推导的情况 |

## 使用示例

### 1. 定义带注解的互斥锁与 RAII 锁守卫

```cpp
class CAPABILITY("mutex") Mutex {
public:
    void Lock() ACQUIRE() { /* ... */ }
    void Unlock() RELEASE() { /* ... */ }
    void ReaderLock() ACQUIRE_SHARED() { /* ... */ }
    void ReaderUnlock() RELEASE_SHARED() { /* ... */ }
    bool TryLock() TRY_ACQUIRE(true) { /* ... */ }
};

class SCOPED_CAPABILITY MutexLocker {
public:
    MutexLocker(Mutex *mu) ACQUIRE(mu) : mut_(mu) { mu->Lock(); }
    ~MutexLocker() RELEASE() { if (mut_) mut_->Unlock(); }
private:
    Mutex *mut_;
};
```

### 2. 标注受保护数据与成员函数

```cpp
class Counter {
public:
    void Increment() REQUIRES(mu_) { ++value_; }
    int Get() REQUIRES_SHARED(mu_) { return value_; }
    Mutex &GetMutex() RETURN_CAPABILITY(mu_) { return mu_; }
private:
    Mutex mu_;
    int value_ GUARDED_BY(mu_);
};
```

### 3. 使用标准库锁

> 使用标准库`std::mutex, std::lock_guard`，需要增加`-D_LIBCPP_ENABLE_THREAD_SAFETY_ANNOTATIONS`编译选项。

```cpp
#include <mutex>
#include <vector>

std::mutex mut;
std::vector<int> data{0,1} GUAREDE_BY(mut);

// access with capabilities, no warning.
{
    std::lock_guard<std::mutex> lock(mut);
    int a = data.at(0);
}

// access without capabilities, warning.
{
    int b = data.at(0);
}
```

编译：`clang++ test.cpp -Wthread-safety -stdlib=libc++ -D_LIBCPP_ENABLE_THREAD_SAFETY_ANNOTATIONS -I base/include/ -o a.out`

### 4. 测试用例
- 正确用法示例：base/test/unittest/common/utils_thread_safety_analysis_test.cpp

### 5. 编译与运行
- 使用 clang 编译时，需启用 `-Wthread-safety` 才能进行线程安全分析

## 常见问题

详细信息参考[LLVM官方文档](https://clang.llvm.org/docs/ThreadSafetyAnalysis.html)

1. **应用于`std::mutex, std::lock_guard`**：需要应用于标准库互斥锁，需要添加编译选项宏`-D_LIBCPP_ENABLE_THREAD_SAFETY_ANNOTATIONS`，且使用`libc++`而非`libstdc++`。但注意：该编译选项宏会使能clang编译器对所有使用标注库互斥锁、lock_guard代码进行线程安全性检测，可能引入新的告警。

2. **编译器支持**：线程安全分析仅在 clang 下生效。在 GCC、MSVC 等编译器下，这些宏会展开为空，不会报错，但也不会进行静态检查。

3. **如何启用分析**：使用 clang 时需添加编译选项 `-Wthread-safety`（包含了 `-Wthread-safety-analysis, -Wthread-safety-attributes, -Wthread-safety-precise, -Wthread-safety-reference`，也可单独开启此四个安全检测宏）或 `-Wthread-safety-pointer`（检测受保护变量的指针、指向受保护数据的指针），否则注解不会触发诊断（OH构建系统已默认开启）。OH构建系统默认提升thread-safety warning为error，可以使用`-Wno-error=thread-safety`降级为warning。

4. **NO_THREAD_SAFETY_ANALYSIS 的用法**：当分析器产生误报，或某些复杂的锁逻辑无法被正确推导时，可对相应函数使用 `NO_THREAD_SAFETY_ANALYSIS` 跳过分析。应谨慎使用，避免掩盖真实问题。

5. **SCOPED_CAPABILITY 与 RAII**：用于锁守卫等 RAII 类型时，构造函数应使用 `ACQUIRE` 或 `ACQUIRE_SHARED` 标注加锁操作，析构函数使用 `RELEASE` 或 `RELEASE_GENERIC` 标注解锁操作。
