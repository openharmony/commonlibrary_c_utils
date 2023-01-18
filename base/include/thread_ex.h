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
 * @file thread_ex.h
 *
 * @brief The file contains interfaces of the Thread class
 * implemented in c_utils.
 */

#ifndef UTILS_THREAD_EX_H
#define UTILS_THREAD_EX_H

#include <pthread.h>
#include <string>
#include <mutex>
#include <condition_variable>

namespace OHOS {

enum class ThreadStatus {
    OK,
    WOULD_BLOCK,
    INVALID_OPERATION,
    UNKNOWN_ERROR,
};

enum ThreadPrio {
    THREAD_PROI_NORMAL = 0,
    THREAD_PROI_LOW = 10,
    THREAD_PROI_LOWEST = 19,
};

constexpr int INVALID_PTHREAD_T = -1;
constexpr int MAX_THREAD_NAME_LEN = 15;

/**
 * @brief A thread class provided by c_utils, include functions such as
 * creating threads and getting thread ID.
 */
class Thread {
public:

/**
 * @brief Construct a Thread object, but does not start the thread.
 */
    Thread();
    virtual ~Thread();

/**
 * @brief Create and start a child thread, and execute Run() in a loop.
 * Loop stops when Run() returns false or notifies to exit by
 * `NotifyExitSync()` or `NotifyExitAsync()` from another thread.
 *
 * @param name The thread name.
 * @param priority Thread priority.
 * @param stack The size of the thread stack.
 * @return Return OK if the call is successful;
 * return INVALID_OPERATION when the thread already exists;
 * return UNKNOWN_ERROR when thread creation fails.
 * @see NotifyExitSync() NotifyExitAsync()
 */
    ThreadStatus Start(const std::string& name, int32_t priority = THREAD_PROI_NORMAL, size_t stack = 0);

/**
 * @brief Synchronously notify this Thread object to exit.
 *
 * The current thread is blocked, waiting for the child thread to finish.
 */
    ThreadStatus NotifyExitSync();

/**
 * @brief Asynchronously notify this Thread object to exit.
 *
 * That is, whether the child thread exits does not block
 * the current thread. Notifies the child thread to stop
 *  and the current thread to continue running.
 */
    virtual void NotifyExitAsync();

/**
 * @brief Determine whether the thread is ready.
 */
    virtual bool ReadyToWork();

/**
 * @brief Get the flag of the thread exitPending.
 *
 * If the flag is true, other waiting threads who have called
 * `NotifyExitSync()` are woken up when the current thread
 * finishes running and exits.
 *
 * @return Returns true, indicating that another thread may be
 * blocking to wait for the current thread to complete;
 * Otherwise return false.
 */
    bool IsExitPending() const;

/**
 * @brief Determine whether the thread is running.
 *
 * @return Return true if the thread is running,
 * otherwise, return false.
 */
    bool IsRunning() const;

/**
 * @brief Get the thread ID.
 */
    pthread_t GetThread() const { return thread_; }

protected:
    virtual bool Run() = 0; // Derived class must implement Run()

private:
    Thread(const Thread&) = delete;
    Thread& operator=(const Thread&) = delete;
    static int ThreadStart(void* args);
    ThreadStatus Join(); // pthread created as detached

private:
    pthread_t thread_;  // Thread ID
    mutable std::mutex lock_;
    std::condition_variable cvThreadExited_;
    ThreadStatus status_;
    volatile bool exitPending_;
    volatile bool running_; // flag of thread runing
};

} // namespace OHOS

#endif

