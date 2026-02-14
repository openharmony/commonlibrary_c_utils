/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <gtest/gtest.h>

#include "thread_safety_analysis_macros.h"

using namespace testing::ext;

namespace OHOS {
namespace {

// 参考 LLVM 官方线程安全分析文档中的示例，为 Mutex 和 MutexLocker
// 定义带注解的接口，同时在这里加一点简单的状态字段，方便 UT 验证
// 这些宏在编译及运行期的行为（运行期只关心不被宏改变语义）。

// Defines an annotated interface for mutexes.
class CAPABILITY("mutex") Mutex {
public:
    // Acquire/lock this mutex exclusively.
    void Lock() ACQUIRE()
    {
        locked_ = true;
        sharedLocked_ = false;
    }

    // Acquire/lock this mutex for read operations.
    void ReaderLock() ACQUIRE_SHARED()
    {
        locked_ = true;
        sharedLocked_ = true;
    }

    // Release/unlock an exclusive mutex.
    void Unlock() RELEASE()
    {
        locked_ = false;
        sharedLocked_ = false;
    }

    // Release/unlock a shared mutex.
    void ReaderUnlock() RELEASE_SHARED()
    {
        locked_ = false;
        sharedLocked_ = false;
    }

    // Generic unlock, can unlock exclusive and shared mutexes.
    void GenericUnlock() RELEASE_GENERIC()
    {
        locked_ = false;
        sharedLocked_ = false;
    }

    // Try to acquire the mutex.  Returns true on success, and false on failure.
    bool TryLock() TRY_ACQUIRE(true)
    {
        if (locked_) {
            return false;
        }
        locked_ = true;
        sharedLocked_ = false;
        return true;
    }

    // Try to acquire the mutex for read operations.
    bool ReaderTryLock() TRY_ACQUIRE_SHARED(true)
    {
        if (locked_) {
            return false;
        }
        locked_ = true;
        sharedLocked_ = true;
        return true;
    }

    // Assert that this mutex is currently held by the calling thread.
    void AssertHeld() ASSERT_CAPABILITY(this) {}

    // Assert that this mutex is currently held for read operations.
    void AssertReaderHeld() ASSERT_SHARED_CAPABILITY(this) {}

    // For negative capabilities.
    const Mutex& operator!() const
    {
        return *this;
    }

    bool IsLocked() const
    {
        return locked_;
    }

    bool IsSharedLocked() const
    {
        return sharedLocked_;
    }

private:
    bool locked_ = false;
    bool sharedLocked_ = false;
};

// Tag types for selecting a constructor.
struct AdoptLockTag { };
struct DeferLockTag { };
struct SharedLockTag { };

inline constexpr AdoptLockTag ADOPT_LOCK = {};
inline constexpr DeferLockTag DEFER_LOCK = {};
inline constexpr SharedLockTag SHARED_LOCK = {};

// MutexLocker is an RAII class that acquires a mutex in its constructor, and
// releases it in its destructor.
class SCOPED_CAPABILITY MutexLocker {
private:
    Mutex *mut_ = nullptr;
    bool locked_ = false;

public:
    // Acquire mu, implicitly acquire *this and associate it with mu.
    MutexLocker(Mutex *mu) ACQUIRE(mu) : mut_(mu), locked_(true)
    {
        if (mut_ != nullptr) {
            mut_->Lock();
        }
    }

    // Assume mu is held, implicitly acquire *this and associate it with mu.
    MutexLocker(Mutex *mu, AdoptLockTag) REQUIRES(mu) : mut_(mu), locked_(true)
    {}

    // Acquire mu in shared mode, implicitly acquire *this and associate it with mu.
    MutexLocker(Mutex *mu, SharedLockTag) ACQUIRE_SHARED(mu) : mut_(mu), locked_(true)
    {
        if (mut_ != nullptr) {
            mut_->ReaderLock();
        }
    }

    // Assume mu is held in shared mode, implicitly acquire *this and associate it with mu.
    MutexLocker(Mutex *mu, AdoptLockTag, SharedLockTag) REQUIRES_SHARED(mu)
        : mut_(mu), locked_(true)
    {}

    // Assume mu is not held, implicitly acquire *this and associate it with mu.
    MutexLocker(Mutex *mu, DeferLockTag) EXCLUDES(mu) : mut_(mu), locked_(false)
    {}

    // Same as constructors, but without tag types. (Requires C++17 copy elision.)
    // Helper that forwards to the annotated constructor. No additional
    // thread-safety annotation here to avoid confusing the analysis when
    // returning a scoped capability by value.
    static MutexLocker Lock(Mutex *mu)
    {
        return MutexLocker(mu);
    }

    static MutexLocker Adopt(Mutex *mu) REQUIRES(mu)
    {
        return MutexLocker(mu, ADOPT_LOCK);
    }

    // Shared-mode helper similar to Lock().
    static MutexLocker ReaderLock(Mutex *mu)
    {
        return MutexLocker(mu, SHARED_LOCK);
    }

    static MutexLocker AdoptReaderLock(Mutex *mu) REQUIRES_SHARED(mu)
    {
        return MutexLocker(mu, ADOPT_LOCK, SHARED_LOCK);
    }

    static MutexLocker DeferLock(Mutex *mu) EXCLUDES(mu)
    {
        return MutexLocker(mu, DEFER_LOCK);
    }

    // Release *this and all associated mutexes, if they are still held.
    ~MutexLocker() RELEASE_GENERIC()
    {
        if (locked_ && mut_ != nullptr) {
            mut_->GenericUnlock();
        }
        locked_ = false;
    }

    // Acquire all associated mutexes exclusively.
    void Lock() ACQUIRE()
    {
        if (mut_ != nullptr && !locked_) {
            mut_->Lock();
            locked_ = true;
        }
    }

    // Try to acquire all associated mutexes exclusively.
    bool TryLock() TRY_ACQUIRE(true)
    {
        if (mut_ == nullptr) {
            return false;
        }
        if (locked_) {
            return false;
        }
        locked_ = mut_->TryLock();
        return locked_;
    }

    // Acquire all associated mutexes in shared mode.
    void ReaderLock() ACQUIRE_SHARED()
    {
        if (mut_ != nullptr && !locked_) {
            mut_->ReaderLock();
            locked_ = true;
        }
    }

    // Try to acquire all associated mutexes in shared mode.
    bool ReaderTryLock() TRY_ACQUIRE_SHARED(true)
    {
        if (mut_ == nullptr) {
            return false;
        }
        if (locked_) {
            return false;
        }
        locked_ = mut_->ReaderTryLock();
        return locked_;
    }

    // Release all associated mutexes.
    void Unlock() RELEASE()
    {
        if (mut_ != nullptr && locked_) {
            mut_->Unlock();
            locked_ = false;
        }
    }

    // Release all associated mutexes (shared mode).
    void ReaderUnlock() NO_THREAD_SAFETY_ANALYSIS
    {
        if (mut_ != nullptr && locked_) {
            mut_->ReaderUnlock();
            locked_ = false;
        }
    }

    bool IsLocked() const
    {
        return locked_;
    }
};

// 一个带注解的数据结构，演示 GUARDED_BY / REQUIRES / REQUIRES_SHARED。
class AnnotatedCounter {
public:
    AnnotatedCounter() : value_(0) {}

    void Increment() REQUIRES(mu_)
    {
        ++value_;
    }

    int Get() REQUIRES_SHARED(mu_)
    {
        return value_;
    }

    void UnsafeIncrement() NO_THREAD_SAFETY_ANALYSIS
    {
        // 显式关闭分析，避免假阳性。
        ++value_;
    }

    Mutex &GetMutex() RETURN_CAPABILITY(mu_)
    {
        return mu_;
    }

private:
    Mutex mu_;
    int value_ GUARDED_BY(mu_) = 0;
};

class UtilsThreadSafetyAnalysisTest : public testing::Test {
public:
    static void SetUpTestCase(void) {}
    static void TearDownTestCase(void) {}
    void SetUp() {}
    void TearDown() {}
};

/*
 * @tc.name: MutexBasicOperations
 * @tc.desc: Verify basic Mutex and annotated methods follow expected runtime
 *           behaviour and compile with LLVM-style thread safety annotations.
 * @tc.type: FUNC
 * @tc.level: Level0
 */
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

    // TryLock should fail when already locked.
    bool locked = mu.TryLock();
    EXPECT_TRUE(locked);
    EXPECT_TRUE(mu.IsLocked());

    // 第二次 TryLock 预期失败，此时不能假定已经持有锁，因此不调用 GenericUnlock。
    bool lockedAgain = mu.TryLock();
    EXPECT_FALSE(lockedAgain);

    if (locked) {
        mu.GenericUnlock();
    }
    EXPECT_FALSE(mu.IsLocked());

    // Try shared lock when unlocked.
    bool sharedLocked = mu.ReaderTryLock();
    EXPECT_TRUE(sharedLocked);
    EXPECT_TRUE(mu.IsLocked());
    EXPECT_TRUE(mu.IsSharedLocked());
    if (sharedLocked) {
        mu.GenericUnlock();
    }
    EXPECT_FALSE(mu.IsLocked());
}

/*
 * @tc.name: MutexLockerRaiiAndTags
 * @tc.desc: Verify MutexLocker RAII behaviour and LLVM-style tag constructors
 *           (adopt_lock / defer_lock / shared_lock).
 * @tc.type: FUNC
 * @tc.level: Level0
 */
HWTEST_F(UtilsThreadSafetyAnalysisTest, MutexLockerRaiiAndTags, TestSize.Level0)
{
    Mutex mu;

    // 普通构造 + 析构自动解锁。
    {
        MutexLocker locker(&mu);
        EXPECT_TRUE(mu.IsLocked());
        EXPECT_TRUE(locker.IsLocked());
    }
    EXPECT_FALSE(mu.IsLocked());

    // adopt_lock：假定外部已经加锁，并在析构时负责解锁。
    mu.Lock();
    {
        MutexLocker locker(&mu, ADOPT_LOCK);
        EXPECT_TRUE(mu.IsLocked());
        EXPECT_TRUE(locker.IsLocked());
    }
    // adopt_lock 析构后应释放锁。
    EXPECT_FALSE(mu.IsLocked());

    // defer_lock：构造时不加锁，调用 Lock 之后才加锁。
    {
        MutexLocker locker(&mu, DEFER_LOCK);
        EXPECT_FALSE(mu.IsLocked());
        EXPECT_FALSE(locker.IsLocked());
        locker.Lock();
        EXPECT_TRUE(mu.IsLocked());
        EXPECT_TRUE(locker.IsLocked());
    }
    EXPECT_FALSE(mu.IsLocked());

    // shared_lock：构造时以共享模式加锁。
    {
        MutexLocker locker(&mu, SHARED_LOCK);
        EXPECT_TRUE(mu.IsLocked());
        EXPECT_TRUE(mu.IsSharedLocked());
        EXPECT_TRUE(locker.IsLocked());
    }
    EXPECT_FALSE(mu.IsLocked());

    // ReaderLock / ReaderUnlock。
    {
        MutexLocker locker = MutexLocker::DeferLock(&mu);
        EXPECT_FALSE(mu.IsLocked());

        locker.ReaderLock();
        EXPECT_TRUE(mu.IsLocked());
        EXPECT_TRUE(mu.IsSharedLocked());

        locker.ReaderUnlock();
        EXPECT_FALSE(mu.IsLocked());
    }
    EXPECT_FALSE(mu.IsLocked());
}

/*
 * @tc.name: AnnotatedCounterWithGuardedBy
 * @tc.desc: Verify GUARDED_BY / REQUIRES / REQUIRES_SHARED / NO_THREAD_SAFETY_ANALYSIS
 *           can be used together and do not change runtime semantics.
 * @tc.type: FUNC
 * @tc.level: Level0
 */
HWTEST_F(UtilsThreadSafetyAnalysisTest, AnnotatedCounterWithGuardedBy, TestSize.Level0)
{
    AnnotatedCounter counter;
    {
        MutexLocker locker(&counter.GetMutex());
        counter.Increment();
        counter.Increment();
        EXPECT_EQ(counter.Get(), 2);
    }

    // UnsafeIncrement 关闭分析，但运行期仍然正常。
    counter.UnsafeIncrement();
    {
        MutexLocker locker(&counter.GetMutex());
        EXPECT_EQ(counter.Get(), 3);
    }
}

} // namespace
} // namespace OHOS

