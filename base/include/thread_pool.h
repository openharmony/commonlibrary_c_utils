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
 * @brief Give a thread-safe thread pool.
 *
 * The thread-safe is for threadpool itself not for the threads in pool. A task
 * queue and a thread group are under control. Users add tasks to task queue.
 * The thread group will execute the tasks in task queue.
 */
class ThreadPool : public NoCopyable {
public:
    typedef std::function<void()> Task;

    /**
     * @brief Construct ThreadPool and name the threads in pool.
     *
     * @param name it will be set as a part the real name of threads in pool.
     * The real name of threads in pool will be like: name + No. The thread
     * name is a meaningful C language string, whose length is restricted to
     * 16 characters, including the terminating null byte ('\0'). Please pay
     * attention to the length of name here. For example, if the number of
     * threads in pool is less than 10, the maximum length of name is 14.
     */
    explicit ThreadPool(const std::string &name = std::string());
    ~ThreadPool() override;

    /**
     * @brief Start a given number(threadsNum) of threads, which will execute
     * the tasks in task queue.
     *
     * @param threadsNum A given number of threads to start.
     */
    uint32_t Start(int threadsNum);
    /**
     * @brief Stop ThreadPool and wait all threads in pool to stop.
     */
    void Stop();
    /**
     * @brief Add a Task to task queue.
     *
     * If Start() has never been called, the Task will be executed immediately.
     *
     * @param f A Task to be added to task queue.
     */
    void AddTask(const Task& f);
    /**
     * @brief Set the maximum amount of tasks in task queue.
     *
     * @param maxSize The maximum amount of tasks in task queue.
     */
    void SetMaxTaskNum(size_t maxSize) { maxTaskNum_ = maxSize; }

    // for testability
    /**
     * @brief Get the maximum amount of tasks in task queue.
     */
    size_t GetMaxTaskNum() const { return maxTaskNum_; }
    /**
     * @brief Get the current amount of tasks in task queue.
     */
    size_t GetCurTaskNum();
    /**
     * @brief Get the current amount of threads in pool.
     */
    size_t GetThreadsNum() const { return threads_.size(); }
    /**
     * @brief Get the name of ThreadPool.
     */
    std::string GetName() const { return myName_; }

private:
    // tasks in the queue reach the maximum set by maxQueueSize, means thread pool is full load.
    bool Overloaded() const;
    void WorkInThread(); // main function in each thread.
    Task ScheduleTask(); // fetch a task from the queue and execute

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

