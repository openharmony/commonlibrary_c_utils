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
 * @brief This file contains interfaces of Semaphore in c_utils,
 * including nameless and named semaphores.
 *
 * The semaphore is an atomic counter, which can act as a lock,
 * to achieve mutual exclusion, synchronization and other functions;
 * Used in a multithreaded environment, it is possible to ensure that
 * a critical pieces of code is not called concurrently or maximum number
 * of threads entering the code section is restricted.
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
 * @brief Semaphore. This class is a counter to implement functions such as
 * mutual exclusion between processes/threads, synchronization, etc.
 *
 * The difference between nameless semaphores and named semaphores lies
 * in the form of creation and destruction.
 * The semaphore exists only in memory, requiring that the process/thread
 * using the semaphore must access the memory where the semaphore is located.
 * Therefore, the nameless semaphore can only be in the thread of
 * the same process, or threads in different processes that have mapped
 * the same memory to their address space, that is, the nameless semaphore
 * can only be accessed through shared memory.
 */
class Semaphore : public NoCopyable {
public:
/**
 * @brief Construct a semaphore object.
 *
 * @param Value The initial value of the semaphore object.
 */
    explicit Semaphore(int value = 1) : count_(value) {}

/**
 * @brief Acquire semaphore operation, i.e. semaphore -1.
 *
 * When the semaphore is >= 0 after decrement, the current thread continues.
 * When semaphore is < 0 after decrement, the current thread blocks.
 */
    void Wait();

/**
 * @brief Release semaphore operation, i.e. semaphore +1.
 *
 * When the semaphore is > 0 after increment, it means that
 * there is no blocked thread.
 * When the semaphore is <= 0 after increment, it means that there are still
 * blocked threads. Then this method will wake one of them up.
 */
    void Post();

private:
    int count_;   // The initial value of the Semaphore object
    std::mutex mutex_;
    std::condition_variable cv_;
};

} // OHOS

#endif

