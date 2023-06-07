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
 * @file semaphore_ex.h
 *
 * @brief Provides interfaces of semaphores in c_utils,
 * including nameless and named semaphores.
 *
 * A semaphore is an atomic counter, which can act as a lock
 * to achieve mutual exclusion, synchronization, and other functions.
 * Used in a multithreaded environment, it prevents concurrent
 * calling of a critical piece of code or restricts the maximum number
 * of threads entering the code section.
 */

#ifndef SEMAPHORE_EX_H
#define SEMAPHORE_EX_H

#include "nocopyable.h"

#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <string>
#include <ctime> // timespec since c11

namespace OHOS {
/**
 * @brief Provides interfaces for operating semaphores.
 *
 * A semaphore is a counter used to implement functions, such as
 * mutual exclusion between processes/threads, synchronization, and more.
 * The difference between nameless semaphores and named semaphores lies
 * in the form of creation and destruction.
 * Semaphores exist only in memory. The process/thread
 * using the semaphore must access the memory where the semaphore is located.
 * Therefore, the nameless semaphore can only be in the thread of
 * the same process, or threads in different processes that have mapped
 * the same memory to their address space, that is, the nameless semaphores
 * can only be accessed through shared memory.
 */
class Semaphore : public NoCopyable {
public:
/**
 * @brief A constructor used to create a semaphore object.
 *
 * @param Value Indicates the initial value of the semaphore object.
 */
    explicit Semaphore(int value = 1) : count_(value) {}

/**
 * @brief Acquires the semaphore.
 *
 * If the current semaphore count >= 0, the current thread continues.
 * If the current semaphore count < 0, the current thread will be blocked.
 */
    void Wait();

/**
 * @brief Releases the semaphore.
 *
 * If the current semaphore count > 0, there is no blocked thread.
 * If the current semaphore count <= 0, there are still
 * blocked threads. Then this method will wake one of them up.
 */
    void Post();

private:
    int count_;   // Initial value of the semaphore object.
    std::mutex mutex_;
    std::condition_variable cv_;
};

} // OHOS

#endif

