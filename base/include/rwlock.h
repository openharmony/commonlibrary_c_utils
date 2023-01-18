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
 * @brief A file contains interfaces of RWLock in c_utils.
 */

#ifndef UTILS_RWLOCK_H
#define UTILS_RWLOCK_H

#include <atomic>
#include <thread>

#include "nocopyable.h"

namespace OHOS {
namespace Utils {

/**
 * @brief RWLock promise reading and writing thread-safe.
 *
 * Under RWLock, writing and writing are mutually exclusive,
 * writing and reading are mutually exclusive.
 * However, reading and reading are not mutually exclusive.
 */
class RWLock : NoCopyable {
public:
/**
 * @brief Enumeration of lock status.
 */
    enum LockStatus {
        LOCK_STATUS_WRITE = -1,
        LOCK_STATUS_FREE = 0,
    };

/**
 * @brief Construct a RWLock object.
 *
 * @param writeFirst Specifies the mode of RWLock, whether it is write-first.
 */
    RWLock() : RWLock(true) {}
    explicit RWLock(bool writeFirst);

/**
 * @brief Destroy a RWLock object.
 */
    ~RWLock() override {}

/**
 * @brief Acquire a read lock
 *
 * If the thread has acquired the write lock, return directly.
 * In 'write-first' mode, the state must be non-write locked
 * and no other threads are waiting to write to acquire a read lock.
 * If it is not write priority, you only need the current state to be
 * non-write-locked to acquire a read lock.
 */
    void LockRead();

/**
 * @brief Release the read lock.
 *
 * If the "write lock" has been acquired before,
 * LockRead() will return directly, thus,
 * this method will also be returned directly when called.
 */
    void UnLockRead();

/**
 *@brief Acquire a write lock
 *
 * If the thread has acquired a "write lock", LockWrite() will return directly
 * to avoid acquiring a lock, because write locks are "exclusive locks".
 * Only when no other thread has acquired a read lock or a write lock,
 * the write lock can be acquired; otherwise wait.
 */
    void LockWrite();

/**
 * @brief Release the write lock.
 *
 * If the thread does not obtain a "write lock" , it returns directly.
 */
    void UnLockWrite();

private:
    bool writeFirst_;  // The flag of write mode, true means write priority mode
    std::thread::id writeThreadID_;  // The ID of write Thread

    // Resource lock counter, -1 is write state, 0 is free state, and greater than 0 is shared read state
    std::atomic_int lockCount_;

    // Thread counter waiting for write lock
    std::atomic_uint writeWaitCount_;
};

/**
 * @brief UniqueWriteGuard object controls the ownership of a lockable object
 * within a scope, and is used only as acquisition
 * and release of "write locks".
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
 * and release of "read locks".
 * It is actually a encapsulation of the RWLock class, which can be locked
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

