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

// Based on the examples from the LLVM thread safety analysis documentation,
// this file defines annotated interfaces for Mutex and MutexLocker, and adds
// simple state fields to make it easy for unit tests to verify that the
// annotations do not change runtime behaviour while still compiling correctly.

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

// Demonstrates PT_GUARDED_BY: the data pointed to by ptr_ is protected by mu_.
class DataWithPtrGuardedBy {
public:
    DataWithPtrGuardedBy() : storage_(0), ptr_(&storage_) {}

    void SetValue(int v) REQUIRES(mu_)
    {
        *ptr_ = v;
    }

    int GetValue() REQUIRES_SHARED(mu_)
    {
        return *ptr_;
    }

    Mutex &GetMutex() RETURN_CAPABILITY(mu_)
    {
        return mu_;
    }

private:
    Mutex mu_;
    int storage_;
    int *ptr_ PT_GUARDED_BY(mu_);
};

// Lock ordering: four mutexes in a chain (mu1 before mu2 before mu3 before mu4)
// to demonstrate ACQUIRED_BEFORE without extern variables. Declared in reverse
// order so each ACQUIRED_BEFORE references an already-declared member.
struct LockOrderingDemo {
    Mutex mu4;
    Mutex mu3 ACQUIRED_BEFORE(mu4);
    Mutex mu2 ACQUIRED_BEFORE(mu3);
    Mutex mu1 ACQUIRED_BEFORE(mu2);
};

// Lock ordering using ACQUIRED_AFTER: a mirror of the above chain. Each mutex
// must be acquired after the one that follows it (mu4 first, then mu3, mu2, mu1).
// Members are again declared so that each ACQUIRED_AFTER references an
// already-declared member.
struct LockOrderingAfterDemo {
    Mutex mu4;
    Mutex mu3 ACQUIRED_AFTER(mu4);
    Mutex mu2 ACQUIRED_AFTER(mu3);
    Mutex mu1 ACQUIRED_AFTER(mu2);
};

// Example of an annotated data structure demonstrating GUARDED_BY /
// REQUIRES / REQUIRES_SHARED combined together.
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
        // Explicitly disable analysis to suppress potential false positives.
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

    // Default constructor acquires the lock and the destructor releases it
    // automatically (RAII behaviour).
    {
        MutexLocker locker(&mu);
        EXPECT_TRUE(mu.IsLocked());
        EXPECT_TRUE(locker.IsLocked());
    }
    EXPECT_FALSE(mu.IsLocked());

    // adopt_lock: assume the mutex was already locked by the caller, and let
    // the MutexLocker destructor be responsible for unlocking it.
    mu.Lock();
    {
        MutexLocker locker(&mu, ADOPT_LOCK);
        EXPECT_TRUE(mu.IsLocked());
        EXPECT_TRUE(locker.IsLocked());
    }
    // After the adopt_lock-based locker is destroyed, the mutex should be
    // released.
    EXPECT_FALSE(mu.IsLocked());

    // defer_lock: do not lock in the constructor; the mutex is only locked
    // when Lock() is explicitly called.
    {
        MutexLocker locker(&mu, DEFER_LOCK);
        EXPECT_FALSE(mu.IsLocked());
        EXPECT_FALSE(locker.IsLocked());
        locker.Lock();
        EXPECT_TRUE(mu.IsLocked());
        EXPECT_TRUE(locker.IsLocked());
    }
    EXPECT_FALSE(mu.IsLocked());

    // shared_lock: acquire the mutex in shared (reader) mode in the constructor.
    {
        MutexLocker locker(&mu, SHARED_LOCK);
        EXPECT_TRUE(mu.IsLocked());
        EXPECT_TRUE(mu.IsSharedLocked());
        EXPECT_TRUE(locker.IsLocked());
    }
    EXPECT_FALSE(mu.IsLocked());

    // ReaderLock / ReaderUnlock helpers used with a deferred locker.
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

    // UnsafeIncrement disables analysis, but runtime behaviour should remain
    // correct.
    counter.UnsafeIncrement();
    {
        MutexLocker locker(&counter.GetMutex());
        EXPECT_EQ(counter.Get(), 3);
    }
}

/*
 * @tc.name: PtGuardedByAccess
 * @tc.desc: Verify PT_GUARDED_BY annotation: data pointed to by the pointer
 *           is protected by the given mutex. Accesses through the pointer
 *           must hold the lock.
 * @tc.type: FUNC
 * @tc.level: Level0
 */
HWTEST_F(UtilsThreadSafetyAnalysisTest, PtGuardedByAccess, TestSize.Level0)
{
    DataWithPtrGuardedBy data;

    {
        MutexLocker locker(&data.GetMutex());
        data.SetValue(42);
        EXPECT_EQ(data.GetValue(), 42);
    }

    {
        MutexLocker locker(&data.GetMutex());
        data.SetValue(100);
        EXPECT_EQ(data.GetValue(), 100);
    }
}

/*
 * @tc.name: AcquiredBeforeOrder
 * @tc.desc: Verify ACQUIRED_BEFORE lock ordering using four mutexes
 *           (mu1 before mu2 before mu3 before mu4). Locks must be acquired
 *           in the declared order.
 * @tc.type: FUNC
 * @tc.level: Level0
 */
HWTEST_F(UtilsThreadSafetyAnalysisTest, AcquiredBeforeOrder, TestSize.Level0)
{
    LockOrderingDemo order;

    EXPECT_FALSE(order.mu1.IsLocked());
    EXPECT_FALSE(order.mu2.IsLocked());
    EXPECT_FALSE(order.mu3.IsLocked());
    EXPECT_FALSE(order.mu4.IsLocked());

    // Correct order: mu1 -> mu2 -> mu3 -> mu4.
    order.mu1.Lock();
    order.mu2.Lock();
    order.mu3.Lock();
    order.mu4.Lock();
    EXPECT_TRUE(order.mu1.IsLocked());
    EXPECT_TRUE(order.mu2.IsLocked());
    EXPECT_TRUE(order.mu3.IsLocked());
    EXPECT_TRUE(order.mu4.IsLocked());

    order.mu4.Unlock();
    order.mu3.Unlock();
    order.mu2.Unlock();
    order.mu1.Unlock();
    EXPECT_FALSE(order.mu1.IsLocked());
    EXPECT_FALSE(order.mu2.IsLocked());
    EXPECT_FALSE(order.mu3.IsLocked());
    EXPECT_FALSE(order.mu4.IsLocked());
}

/*
 * @tc.name: AcquiredAfterOrder
 * @tc.desc: Verify ACQUIRED_AFTER lock ordering using four mutexes mirrored
 *           from the ACQUIRED_BEFORE chain. Locks must be acquired from mu4
 *           to mu1 to satisfy the \"acquired after\" constraints.
 * @tc.type: FUNC
 * @tc.level: Level0
 */
HWTEST_F(UtilsThreadSafetyAnalysisTest, AcquiredAfterOrder, TestSize.Level0)
{
    LockOrderingAfterDemo order;

    EXPECT_FALSE(order.mu1.IsLocked());
    EXPECT_FALSE(order.mu2.IsLocked());
    EXPECT_FALSE(order.mu3.IsLocked());
    EXPECT_FALSE(order.mu4.IsLocked());

    // Correct order w.r.t ACQUIRED_AFTER: mu4 -> mu3 -> mu2 -> mu1.
    order.mu4.Lock();
    order.mu3.Lock();
    order.mu2.Lock();
    order.mu1.Lock();
    EXPECT_TRUE(order.mu1.IsLocked());
    EXPECT_TRUE(order.mu2.IsLocked());
    EXPECT_TRUE(order.mu3.IsLocked());
    EXPECT_TRUE(order.mu4.IsLocked());

    order.mu1.Unlock();
    order.mu2.Unlock();
    order.mu3.Unlock();
    order.mu4.Unlock();
    EXPECT_FALSE(order.mu1.IsLocked());
    EXPECT_FALSE(order.mu2.IsLocked());
    EXPECT_FALSE(order.mu3.IsLocked());
    EXPECT_FALSE(order.mu4.IsLocked());
}

} // namespace
} // namespace OHOS

