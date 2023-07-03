/*
 * Copyright (c) 2021-2023 Huawei Device Co., Ltd.
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

/**
 * @file rwlock.h
 *
 * @brief Provides interfaces of RWLock in c_utils.
 */

#ifndef UTILS_RWLOCK_H
#define UTILS_RWLOCK_H

#include <atomic>
#include <thread>

#include "nocopyable.h"

namespace OHOS {
namespace Utils {

/**
 * @brief Implements the <b>RWLock</b> class to ensure that read and write
 * operations are thread-safe.
 *
 * Under RWLock, write operations are mutually exclusive,
 * and read and write operations are mutually exclusive.
 * However, read operations are not mutually exclusive.
 */
class RWLock : NoCopyable {
public:
/**
 * @brief Enumerates the lock states.
 */
    enum LockStatus {
        LOCK_STATUS_WRITE = -1,
        LOCK_STATUS_FREE = 0,
    };

/**
 * @brief Creates an <b>RWLock</b> object.
 *
 * @param writeFirst Indicates whether the <b>RWLock</b> object is write-first.
 */
    RWLock() : RWLock(true) {}
    explicit RWLock(bool writeFirst);

/**
 * @brief Destroys this <b>RWLock</b> object.
 */
    ~RWLock() override {}

/**
 * @brief Obtains a read lock.
 *
 * If the thread has obtained a write lock, this function returns directly.
 * In write-first mode, a read lock can be obtained only when the state
 * is non-write-locked and no other threads are waiting to write data.
 * In other modes, a read lock can be obtained when the state is
 * non-write-locked.
 */
    void LockRead();

/**
 * @brief Releases the read lock.
 *
 * If the write lock has been obtained before,
 * LockRead() will return directly.
 * This function will also return directly when called.
 */
    void UnLockRead();

/**
 *@brief Obtains a write lock
 *
 * If the thread has obtained a write lock, this function returns directly
 * to avoid acquiring a lock, because write locks are exclusive.
 * The write lock can be obtained only when no other thread has obtained a read
 * lock or a write lock; otherwise, the thread shall wait.
 */
    void LockWrite();

/**
 * @brief Releases the write lock.
 *
 * If the thread has not obtained a write lock, this function returns directly.
 */
    void UnLockWrite();

private:
    bool writeFirst_;  // Whether the thread is write-first. The value true means that the thread is write-first.
    std::thread::id writeThreadID_;  // ID of the write thread.

    // Resource lock counter. -1 indicates the write state, 0 indicates the free state, and a value greater than 0
    // indicates the shared read state.
    std::atomic_int lockCount_;

    // Thread counter waiting for the write lock.
    std::atomic_uint writeWaitCount_;
};

/**
 * @brief UniqueWriteGuard object controls the ownership of a lockable object
 * within a scope, and is used only as acquisition
 * and release of write locks.
 * It is actually an encapsulation of the RWLock class, which can be locked
 * at construction time and unlocked during destruction,
 * providing a convenient RAII mechanism.
 */
template <typename RWLockable>
class UniqueWriteGuard : NoCopyable {
public:
    explicit UniqueWriteGuard(RWLockable &rwLockable)
        : rwLockable_(rwLockable)
    {
        rwLockable_.LockWrite();
    }

    ~UniqueWriteGuard() override
    {
        rwLockable_.UnLockWrite();
    }

private:
    UniqueWriteGuard() = delete;

private:
    RWLockable &rwLockable_;
};


/**
 * @brief UniqueWriteGuard object controls the ownership of a lockable object
 * within a scope, and is used only as acquisition
 * and release of read locks.
 * It is actually an encapsulation of the RWLock class, which can be locked
 * at construction time and unlocked during destruction,
 * providing a convenient RAII mechanism.
 */
template <typename RWLockable>
class UniqueReadGuard : NoCopyable {
public:
    explicit UniqueReadGuard(RWLockable &rwLockable)
        : rwLockable_(rwLockable)
    {
        rwLockable_.LockRead();
    }

    ~UniqueReadGuard() override
    {
        rwLockable_.UnLockRead();
    }

private:
    UniqueReadGuard() = delete;

private:
    RWLockable &rwLockable_;
};

} // namespace Utils
} // namespace OHOS
#endif

