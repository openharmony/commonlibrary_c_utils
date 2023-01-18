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
 * @file safe_block_queue.h
 *
 * The file contains interfaces of Thread-safe block queues in c_utils.
 * Includes the SafeBlockQueue class, and the SafeBlockQueueTracking class
 * for trackable tasks.
 */

#ifndef UTILS_BASE_BLOCK_QUEUE_H
#define UTILS_BASE_BLOCK_QUEUE_H

#include <climits>
#include <condition_variable>
#include <mutex>
#include <queue>
#include <atomic>

namespace OHOS {

/**
 * @brief Thread-safe blocking queues.
 *
 * Provides blocking and non-blocking push and pop interfaces.
 */
template <typename T>
class SafeBlockQueue {
public:
    explicit SafeBlockQueue(int capacity) : maxSize_(capacity)
    {
    }

/**
 * @brief Insert an element at the end of the queue (blocking).
 *
 * When the queue is full, the thread of the push operation will be blocked.
 * When the queue is not full, the push operation can be executed
 *  and wakes up one of the waiting threads.
 *
 * @param elem Enqueue data.
 */
    virtual void Push(T const& elem)
    {
        std::unique_lock<std::mutex> lock(mutexLock_);
        while (queueT_.size() >= maxSize_) {
            // queue full , waiting for jobs to be taken
            cvNotFull_.wait(lock, [&]() { return (queueT_.size() < maxSize_); });
        }

        // here means not full we can push in
        queueT_.push(elem);
        cvNotEmpty_.notify_one();
    }

/**
 * @brief Get the first element of the queue (blocking).
 *
 * When the queue is empty, the thread of the pop operation will be blocked.
 * When the queue is not empty, the pop operation can be executed
 * and wakes up one of the waiting threads. Then return the first element
 * of the queue.
 */
    T Pop()
    {
        std::unique_lock<std::mutex> lock(mutexLock_);

        while (queueT_.empty()) {
            // queue empty, waiting for tasks to be Push
            cvNotEmpty_.wait(lock, [&] { return !queueT_.empty(); });
        }

        T elem = queueT_.front();
        queueT_.pop();
        cvNotFull_.notify_one();
        return elem;
    }

/**
 * @brief Insert an element at the end of queue (Non-blocking).
 *
 * When the queue is full, the thread of the push operation
 * will directly return false.
 * When the queue is not full, the push operation can be executed
 * and wakes up one of the waiting threads, and return true.
 *
 * @param elem Enqueue data.
 */
    virtual bool PushNoWait(T const& elem)
    {
        std::unique_lock<std::mutex> lock(mutexLock_);
        if (queueT_.size() >= maxSize_) {
            return false;
        }
        // here means not full we can push in
        queueT_.push(elem);
        cvNotEmpty_.notify_one();
        return true;
    }

/**
 * @brief Get the first elements of the queue (Non-blocking).
 *
 * When the queue is empty, the thread of the pop operation
 * will directly return false.
 * When the queue is not empty, the pop operation can be executed
 * and wakes up one of the waiting threads, and return true.
 *
 * @param outtask data of pop.
 */
    bool PopNotWait(T& outtask)
    {
        std::unique_lock<std::mutex> lock(mutexLock_);
        if (queueT_.empty()) {
            return false;
        }
        outtask = queueT_.front();
        queueT_.pop();

        cvNotFull_.notify_one();

        return true;
    }

    unsigned int Size()
    {
        std::unique_lock<std::mutex> lock(mutexLock_);
        return queueT_.size();
    }

    bool IsEmpty()
    {
        std::unique_lock<std::mutex> lock(mutexLock_);
        return queueT_.empty();
    }

    bool IsFull()
    {
        std::unique_lock<std::mutex> lock(mutexLock_);
        return queueT_.size() == maxSize_;
    }

    virtual ~SafeBlockQueue() {}

protected:
    unsigned long maxSize_;  // capacity of queue
    std::mutex mutexLock_;
    std::condition_variable cvNotEmpty_;
    std::condition_variable cvNotFull_;
    std::queue<T> queueT_;
};

/**
 * @brief A thread-safe blocking queue that inherits SafeBlockQueue
 * and tracks the number of outstanding tasks.
 */
template <typename T>
class SafeBlockQueueTracking : public SafeBlockQueue<T> {
public:
    explicit SafeBlockQueueTracking(int capacity) : SafeBlockQueue<T>(capacity)
    {
        unfinishedTaskCount_ = 0;
    }

    virtual ~SafeBlockQueueTracking() {}

/**
 * @brief Insert an element at the end of queue (blocking).
 *
 * When the queue is full, the thread of the push operation will be blocked.
 * When the queue is not full, the push operation can be executed
 * and wakes up one of the waiting threads.
 */
    virtual void Push(T const& elem)
    {
        unfinishedTaskCount_++;
        std::unique_lock<std::mutex> lock(mutexLock_);
        while (queueT_.size() >= maxSize_) {
            // queue full , waiting for jobs to be taken
            cvNotFull_.wait(lock, [&]() { return (queueT_.size() < maxSize_); });
        }

        // here means not full we can push in
        queueT_.push(elem);

        cvNotEmpty_.notify_one();
    }

/**
 * @brief Insert an element at the end of queue (Non-blocking).
 *
 * When the queue is full, the thread of the push operation
 * will directly return false.
 * When the queue is not full, the push operation can be executed
 *  and wakes up one of the waiting threads, and return true.
 */
    virtual bool PushNoWait(T const& elem)
    {
        std::unique_lock<std::mutex> lock(mutexLock_);
        if (queueT_.size() >= maxSize_) {
            return false;
        }
        // here means not full we can push in
        queueT_.push(elem);
        unfinishedTaskCount_++;
        cvNotEmpty_.notify_one();
        return true;
    }

/**
 * @brief A response function when a task completes.
 *
 * If the count of unfinished task < 1, return false directly.
 * If the count of unfinished task = 1, all threads waiting
 * while calling Join() will be woken up;
 * the count of unfinished task decrements by 1 and returns true.
 * If the count of unfinished task > 1,
 * decrements the count by 1 and returns true.
 */
    bool OneTaskDone()
    {
        std::unique_lock<std::mutex> lock(mutexLock_);
        int unfinished = unfinishedTaskCount_ - 1;

        if (unfinished <= 0) {
            if (unfinished < 0) {
                return false; // false mean call elem done too many times
            }
            cvAllTasksDone_.notify_all();
        }

        unfinishedTaskCount_ = unfinished;
        return true;
    }

/**
 * @brief Wait for all tasks to complete.
 *
 * When anyone of the tasks is not completed, the current thread will be
 * blocked even if it is just woken up.
 */
    void Join()
    {
        std::unique_lock<std::mutex> lock(mutexLock_);
        cvAllTasksDone_.wait(lock, [&] { return unfinishedTaskCount_ == 0; });
    }

/**
 * @brief Returns the number of unfinished tasks.
 */
    int GetUnfinishTaskNum()
    {
        return unfinishedTaskCount_;
    }

protected:
    using SafeBlockQueue<T>::maxSize_;
    using SafeBlockQueue<T>::mutexLock_;
    using SafeBlockQueue<T>::cvNotEmpty_;
    using SafeBlockQueue<T>::cvNotFull_;
    using SafeBlockQueue<T>::queueT_;

    std::atomic<int> unfinishedTaskCount_;
    std::condition_variable cvAllTasksDone_;
};

} // namespace OHOS

#endif
