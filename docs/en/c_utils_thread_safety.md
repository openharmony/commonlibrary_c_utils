# Thread Safety Analysis Macros
## Overview


### Introduction
Provides clang-based compile-time Thread Safety Analysis (TSA) annotation macros. These macros annotate mutexes, protected data, function preconditions/postconditions, etc., helping the compiler detect data races and lock usage errors at compile time. On non-clang compilers, these macros expand to no-ops for compatibility.

```
#include "thread_safety_analysis_macros.h"
```


## Related Interfaces
### Capability Macros
Used to annotate locks or lockable objects.

| Macro | Description |
| --- | --- |
| **CAPABILITY(x)** | Annotates a class/struct as a lockable capability (e.g., mutex). Usage: `class Foo CAPABILITY("mutex") { ... };` |
| **REENTRANT_CAPABILITY** | Annotates a reentrant (recursive lock allowed) lockable class |
| **SCOPED_CAPABILITY** | Annotates an RAII class (e.g., lock guard) that acquires a capability in its constructor and releases it in its destructor |

### Data Protection Macros
Used to annotate variables protected by a lock.

| Macro | Description |
| --- | --- |
| **GUARDED_BY(x)** | Annotates a member variable as guarded by a mutex; the lock must be held when accessing the variable |
| **PT_GUARDED_BY(x)** | Similar to GUARDED_BY, but for pointer variables; indicates that the data pointed to is protected by the specified mutex |

### Lock Ordering Macros
Used to declare lock acquisition order to avoid deadlocks.

| Macro | Description |
| --- | --- |
| **ACQUIRED_BEFORE(...)** | Declares that this capability must be acquired before the listed one(s) |
| **ACQUIRED_AFTER(...)** | Declares that this capability must be acquired after the listed one(s) |

### Function Precondition Macros
Describe the lock state required before calling a function.

| Macro | Description |
| --- | --- |
| **REQUIRES(...)** | Caller must hold the specified mutex(es) (exclusive lock) before calling |
| **REQUIRES_SHARED(...)** | Caller must hold the specified shared lock(s) (reader lock) before calling |
| **EXCLUDES(...)** | Caller must NOT hold the specified mutex(es) when calling; avoids deadlocks from calling such functions while holding locks |

### Function Lock Operation Macros
Describe lock acquisition and release within functions.

| Macro | Description |
| --- | --- |
| **ACQUIRE(...)** | Function acquires (locks) the specified mutex(es) (exclusive) |
| **ACQUIRE_SHARED(...)** | Function acquires the specified mutex(es) in shared mode (reader lock) |
| **RELEASE(...)** | Function releases (unlocks) the specified mutex(es) (previously held exclusively) |
| **RELEASE_SHARED(...)** | Function releases mutex(es) held in shared mode |
| **RELEASE_GENERIC(...)** | Function releases mutex(es) (works for both exclusive and shared mode); used for generic unlock functions |
| **TRY_ACQUIRE(...)** | Function tries to acquire the mutex(es); first parameter is a boolean indicating success when the return value is true |
| **TRY_ACQUIRE_SHARED(...)** | Function tries to acquire the mutex(es) in shared mode |

### Assertion and Return Macros
Used for assertions and return value annotations.

| Macro | Description |
| --- | --- |
| **ASSERT_CAPABILITY(x)** | Asserts that the current thread holds the specified lock |
| **ASSERT_SHARED_CAPABILITY(x)** | Asserts that the current thread holds the specified lock in shared mode |
| **RETURN_CAPABILITY(x)** | Annotates that the function returns a reference or pointer to a lock it holds (commonly used for getters returning a capability) |
| **NO_THREAD_SAFETY_ANALYSIS** | Disables thread safety analysis for the annotated function or code block; used to suppress false positives or cases the analyzer cannot deduce |

## Examples

### 1. Defining an Annotated Mutex and RAII Lock Guard

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
    MutexLocker(Mutex *mu) ACQUIRE(mu) : mu_(mu) { mu->Lock(); }
    ~MutexLocker() RELEASE() { if (mu_) mu_->Unlock(); }
private:
    Mutex *mu_;
};
```

### 2. Annotating Protected Data and Member Functions

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

### 3. Using Standard Library Locks

> To use standard library `std::mutex` and `std::lock_guard`, add the compile option `-D_LIBCPP_ENABLE_THREAD_SAFETY_ANNOTATIONS`.

```cpp
#include <mutex>
#include <vector>

std::mutex mut;
std::vector<int> data{0,1} GUARDED_BY(mut);

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

Build: `clang++ test.cpp -Wthread-safety -stdlib=libc++ -D_LIBCPP_ENABLE_THREAD_SAFETY_ANNOTATIONS -I base/include/ -o a.out`. Note: `-I base/include` is an example path; adjust as needed for your project. To include TSA headers in the OH build system, add the following to the component's `BUILD.gn`:

```gn
external_deps += [ c_utils:utils ]
```

### 4. Test Cases
- Correct usage example: [utils_thread_safety_analysis_test.cpp](https://gitcode.com/openharmony/commonlibrary_c_utils/blob/master/base/test/unittest/common/utils_thread_safety_analysis_test.cpp).

Brief guide for writing test cases:

1. Define a mutex class with capability:

```cpp
// Use CAPABILITY macro to asign capability
class CAPABILITY("mutex") Mutex {
public:
    // Declare methods a mutuable exclusive lock should have.
    void Lock() ACQUIRE();

    // Acquire/lock this mutex for read operations.
    void ReaderLock() ACQUIRE_SHARED();

    // Release/unlock an exclusive mutex.
    void Unlock() RELEASE();

    // Release/unlock a shared mutex.
    void ReaderUnlock() RELEASE_SHARED();

    // Generic unlock, can unlock exclusive and shared mutexes.
    void GenericUnlock() RELEASE_GENERIC();

    // Try to acquire the mutex.  Returns true on success, and false on failure.
    bool TryLock() TRY_ACQUIRE(true);

    // Try to acquire the mutex for read operations.
    bool ReaderTryLock() TRY_ACQUIRE_SHARED(true);

    // Assert that this mutex is currently held by the calling thread.
    void AssertHeld() ASSERT_CAPABILITY(this) {}

    // Assert that this mutex is currently held for read operations.
    void AssertReaderHeld() ASSERT_SHARED_CAPABILITY(this) {}

    // For negative capabilities.
    const Mutex& operator!() const;

    bool IsLocked() const;

    bool IsSharedLocked() const;

private:
    bool locked_;
    bool sharedLocked_;
};
```

2. Use the `HWTEST_F` test framework for concrete test cases:

```cpp
HWTEST_F(UtilsThreadSafetyAnalysisTest, MutexBasicOperations, TestSize.Level0)
{
    Mutex mu;

    EXPECT_FALSE(mu.IsLocked());

    // Exclusive lock / unlock.
    mu.Lock();
    EXPECT_TRUE(mu.IsLocked());
    EXPECT_FALSE(mu.IsSharedLocked());

    mu.Unlock();
    EXPECT_FALSE(mu.IsLocked());

    // Shared lock / unlock.
    mu.ReaderLock();
    EXPECT_TRUE(mu.IsLocked());
    EXPECT_TRUE(mu.IsSharedLocked());

    mu.ReaderUnlock();
    EXPECT_FALSE(mu.IsLocked());

    // TryLock should fail when the mutex is already locked.
    bool locked = mu.TryLock();
    EXPECT_TRUE(locked);
    EXPECT_TRUE(mu.IsLocked());

    // The second TryLock is expected to fail; in that case we cannot assume
    // the lock is held again, so we must not call GenericUnlock for it.
    bool lockedAgain = mu.TryLock();
    EXPECT_FALSE(lockedAgain);

    if (locked) {
        mu.GenericUnlock();
    }
    EXPECT_FALSE(mu.IsLocked());

    // Try shared lock when currently unlocked.
    bool sharedLocked = mu.ReaderTryLock();
    EXPECT_TRUE(sharedLocked);
    EXPECT_TRUE(mu.IsLocked());
    EXPECT_TRUE(mu.IsSharedLocked());
    if (sharedLocked) {
        mu.GenericUnlock();
    }
    EXPECT_FALSE(mu.IsLocked());
}
```

TSA macros are special: the above test case effectively tests the `Mutex` class's lock/unlock behavior, while TSA macros take effect at compile time—a successful compile means the checks are active and the code uses TSA macros correctly.

### 5. Build and Run
- When compiling with clang, enable `-Wthread-safety` for thread safety analysis. For details, see [How to Enable Analysis](#how-to-enable-analysis).

## FAQ

For details, see [LLVM documentation](https://clang.llvm.org/docs/ThreadSafetyAnalysis.html).

1. **Applying to `std::mutex`, `std::lock_guard`**: To use with standard library mutexes, add the compile option macro `-D_LIBCPP_ENABLE_THREAD_SAFETY_ANNOTATIONS`, and use `libc++` rather than `libstdc++`. Note: This macro enables clang to perform thread-safety analysis on all code using annotated library mutexes and lock_guard, which may introduce new warnings.

2. **Compiler Support**: Thread safety analysis only takes effect with clang. With GCC, MSVC, and other compilers, these macros expand to no-ops; they will not produce errors but will not perform static checking either.

3. <a name="how-to-enable-analysis"></a>**How to Enable Analysis**: When using clang, add the compile option `-Wthread-safety` (which includes `-Wthread-safety-analysis`, `-Wthread-safety-attributes`, `-Wthread-safety-precise`, and `-Wthread-safety-reference`; these four can also be enabled separately) or `-Wthread-safety-pointer` (analyzes pointers to protected variables and pointers that refer to protected data); otherwise annotations will not trigger diagnostics (OH build system enables this by default). The OH build system promotes thread-safety warnings to errors by default; use `-Wno-error=thread-safety` to downgrade to warnings.

4. **Usage of NO_THREAD_SAFETY_ANALYSIS**: When the analyzer produces false positives or when complex lock logic cannot be deduced correctly, use `NO_THREAD_SAFETY_ANALYSIS` on the relevant function to skip analysis. Use with caution to avoid masking real issues.

5. **SCOPED_CAPABILITY and RAII**: For RAII types such as lock guards, the constructor should use `ACQUIRE` or `ACQUIRE_SHARED` to annotate the locking operation, and the destructor should use `RELEASE` or `RELEASE_GENERIC` to annotate the unlocking operation.

6. **REENTRANT_CAPABILITY Support**: The clang version in the current OpenHarmony build toolchain does not yet support `REENTRANT_CAPABILITY`; please wait for a toolchain upgrade.
