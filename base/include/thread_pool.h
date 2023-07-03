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
#ifndef THREAD_POOL_H
#define THREAD_POOL_H

#include "nocopyable.h"

#include <thread>
#include <mutex>
#include <functional>
#include <string>
#include <condition_variable>
#include <deque>
#include <vector>

namespace OHOS {
/**
 * @brief Provides interfaces for thread-safe thread pool operations.
 *
 * The thread-safe is for the thread pool, but not for the threads in the pool.
 * A task queue and a thread group are under control. Users add tasks to the
 * task queue, and the thread group executes the tasks in the task queue.
 */
class ThreadPool : public NoCopyable {
public:
    typedef std::function<void()> Task;

    /**
     * @brief Creates a thread pool and names the threads in the pool.
     *
     * @param name Indicates the prefix of the names of the threads in pool.
     * The names of threads in the pool are in the <b>name</b> + No format.
     * The thread name is a meaningful C language string, whose length is
     * restricted to 16 characters including the terminating null byte ('\0').
     * Pay attention to the name length when setting this parameter.
     * For example, if the number of threads in the pool is less than 10,
     * the name length cannot exceed 14 characters.
     */
    explicit ThreadPool(const std::string &name = std::string());
    ~ThreadPool() override;

    /**
     * @brief Starts a given number of threads, which will execute
     * the tasks in a task queue.
     *
     * @param threadsNum Indicates the number of threads to start.
     */
    uint32_t Start(int threadsNum);
    /**
     * @brief Stops the thread pool.
     */
    void Stop();
    /**
     * @brief Adds a task to the task queue.
     *
     * If <b>Start()</b> has never been called, the task will be executed
     * immediately.
     *
     * @param f Indicates the task to add.
     */
    void AddTask(const Task& f);
    /**
     * @brief Sets the maximum number of tasks in the task queue.
     *
     * @param maxSize Indicates the maximum number of tasks to set.
     */
    void SetMaxTaskNum(size_t maxSize) { maxTaskNum_ = maxSize; }

    // for testability
    /**
     * @brief Obtains the maximum number of tasks in the task queue.
     */
    size_t GetMaxTaskNum() const { return maxTaskNum_; }
    /**
     * @brief Obtains the number of tasks in the task queue.
     */
    size_t GetCurTaskNum();
    /**
     * @brief Obtains the number of threads in the pool.
     */
    size_t GetThreadsNum() const { return threads_.size(); }
    /**
     * @brief Obtains the name of the thread pool.
     */
    std::string GetName() const { return myName_; }

private:
    // If the number of tasks in the queue reaches the maximum set by maxQueueSize, the thread pool is full load.
    bool Overloaded() const;
    void WorkInThread(); // main function in each thread.
    Task ScheduleTask(); // fetch a task from the queue and execute it

private:
    std::string myName_;
    std::mutex mutex_;
    std::condition_variable hasTaskToDo_;
    std::condition_variable acceptNewTask_;
    std::vector<std::thread> threads_;
    std::deque<Task> tasks_;
    size_t maxTaskNum_;
    bool running_;
};

} // namespace OHOS

#endif

