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

const int INVALID_SEMA_VALUE = -1;

/**
 * @brief NamedSemaphore.
 *
 * The only difference between the NamedSemaphore and the Semaphore
 * lies in the form of creation and destruction.
 * Generally, the name of the specified semaphore is explicitly specified
 * at the time of creation, and can be accessed directly by name,
 * so it can be used in any process/thread that knows its name.
 * The characteristic of the NamedSemaphore is that they are saved in a file,
 * so it is necessary to close the semaphore at the end like closing the file.
 */
class NamedSemaphore : public NoCopyable {
public:

/**
 * @brief Construct a NamedSemaphore object with a default name,
 * and initial value of the semaphore.
 *
 * The constructor only creates a NamedSemaphore object and does not
 * call sem_open(), so you need to call Create() to create semaphores
 * after construction.
 *
 * @param size the initial value of semaphore.
 */
    explicit NamedSemaphore(size_t);

/**
 * @brief Construct a NamedSemaphore object,
 * specify the name and initial value of the semaphore.
 *
 * The constructor only creates a NamedSemaphore object and does not
 * call sem_open(), so you need to call Create() to create semaphores
 * after construction.
 *
 * @param name the name of NamedSemaphore; NamedSemaphore objects constructed
 * by constructors that do not specify a name are assigned
 * a unique name by default.
 * @param size the initial value of semaphore.
 */
    NamedSemaphore(const std::string&, size_t);

/**
 * @brief Destroy a NamedSemaphore object.
 */
    ~NamedSemaphore() override;

/**
 * @brief Create and initialize a named semaphore.
 *
 * When a Namedsemaphore is created, a sem.xxx file is generated under
 * /dev/shm, and all processes/threads that open the semaphore
 * (including the processes/threads that created it) increment the reference
 * counter for that file managed by the kernel.
 *
 * @return Success returns ture, failure returns false.
 */
    bool Create();

/**
 * @brief Remove the semaphore from the system.
 *
 * When Unlink() is called, the sem.xxx file under /dev/shm rather than
 * the semaphore itself will be deleted immediately, and all processes that
 * have opened the semaphore can still use it normally (because the semaphore
 * file that has been opened is in memory), even if Unlink() is called
 * immediately after opening the file, Wait(), Post() can also operate
 * normally on the semaphore.
 * Each semaphore has a reference counter that records the number of times
 * it has been opened, and when the reference counter is 0,
 * the named semaphore can be deleted from the file system by Unlink().
 * That is, the kernel does not actually delete the semaphore until
 * all processes that open the semaphore have closed the semaphore
 * (the last sem_close call completes).
 *
 * @return Success returns ture, failure returns false.
 */
    bool Unlink();

/**
 * @brief Open a Namedsemaphore file that has already been created.
 *
 * @return Success returns ture, failure returns false.
 */
    bool Open();

/**
 * @brief Turn off the NamedSemaphore.
 *
 * Turn off the semaphore and do not remove it from the system.
 *
 * @return Success returns ture, failure returns false.
 */
    bool Close();

/**
 * @brief Interface for acquiring semaphore (the count of semaphore -1).
 *
 * If the current count value is 0, the call blocks util another call of Post()
 * makes the count greater than 0.
 *
 * @return Success returns 0; Failure returns -1, the semaphore will not be
 * changed by the interface, and errno is set to indicate a specific error.
 */
    bool Wait();

/**
 * @brief Interface for acquiring semaphore (the count of semaphore -1);
 * Non-blocking version.
 *
 * The non-blocking version of Wait() returns an error directly
 * when the current semaphore = 0, not blocking.
 *
 * @return Success returns 0; Failure returns -1, the signal will
 * not be changed by the interface,
 * and errno is set to indicate a specific error.
 */
    bool TryWait();

/**
 * @brief Interface for acquiring semaphore (the count of semaphore -1);
 * Specifies the blocking time version.
 *
 * The version of Wait() that specifies the blocking time,
 * if the value of the semaphore is great than 0,
 * the -1 operation is performed, and the function returns immediately;
 * When the -1 operation cannot be executed, the call will block
 * (specify the blocking time).
 *
 * @param ts Specifies the blocking time of the call.
 * @return Success returns 0; Failure returns -1, the semaphore will
 * not be changed by the interface,
 * and errno is set to indicate a specific error.
 */
    bool TimedWait(const struct timespec& ts);

/**
 * @brief Interface for releasing semaphore (the count of semaphore +1).
 *
 * If the value of the semaphore thus becomes greater than 0,
 * another process/thread that is blocked due to the sem_wait of the call
 * will be woken up and will lock the semaphore.
 *
 * @return Success returns 0; Failure returns -1, the semaphore will not be
 * changed by the interface, and errno is set to indicate a specific error.
 */
    bool Post();

/**
 * @brief Get the value of the semaphore.
 *
 * @return The value of the semaphore is successfully returned;
 * Failure returns INVALID_SEMA_VALUE, which is -1.
 */
    int GetValue() const;

private:
    std::string name_;  // The name of the NamedSemaphore object
    int maxCount_;  // The initial value of the NamedSemaphore object
    void* sema_;  // The pointer to the sem_t structure, which is essentially a long integer.
    bool named_;  // The flag of a Namedsemaphore that is false when no named semaphore name is specified.

};

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

