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
 * @file safe_queue.h
 *
 * @brief The file contains interfaces of thread-safe queues in c_utils.
 *
 * The file contains thread-safe abstract class, the SafeQueue
 * and SafeStack that override the virtual methods of the abstract class.
 */

#ifndef UTILS_BASE_SAFE_QUEUE_H
#define UTILS_BASE_SAFE_QUEUE_H

#include <deque>
#include <mutex>

namespace OHOS {

/**
 * @brief An abstract class for thread-safe queues.
 *
 * Encapsulate std::lock_guard locks on the basis of std::deque,
 * so that the interface of the queue becomes thread-safe.
 */
template <typename T>
class SafeQueueInner {

public:
    SafeQueueInner() {}

    virtual ~SafeQueueInner()
    {
        if (!deque_.empty()) {
            deque_.clear();
        }
    }

    void Erase(T& object)
    {
        std::lock_guard<std::mutex> lock(mutex_);
        deque_.remove(object);
    }

    bool Empty()
    {
        std::lock_guard<std::mutex> lock(mutex_);
        return deque_.empty();
    }

    void Push(const T& pt)
    {
        std::lock_guard<std::mutex> lock(mutex_);
        return DoPush(pt);
    }

    void Clear()
    {
        std::lock_guard<std::mutex> lock(mutex_);
        if (!deque_.empty()) {
            deque_.clear();
        }

        return;
    }

    int Size()
    {
        std::lock_guard<std::mutex> lock(mutex_);
        return deque_.size();
    }

    bool Pop(T& pt)
    {
        std::lock_guard<std::mutex> lock(mutex_);
        return DoPop(pt);
    }

protected:
    virtual void DoPush(const T& pt) = 0;
    virtual bool DoPop(T& pt) = 0;

    std::deque<T> deque_;
    std::mutex mutex_;
};

/**
 * @brief Thread-safe Queue.
 *
 * Overrides the DoPush and DoPop methods of abstract classes to implement
 * the push and pop functionality of the "SafeQueue".
 */
template <typename T>
class SafeQueue : public SafeQueueInner<T> {

protected:
    using SafeQueueInner<T>::deque_;
    using SafeQueueInner<T>::mutex_;

    void DoPush(const T& pt) override
    {
        deque_.push_back(pt);
    }

/**
 * @brief Encapsulate the pop_front() of dequeues
 * and implement the pop function of queues.
 */
    bool DoPop(T& pt) override
    {
        if (deque_.size() > 0) {
            pt = deque_.front();
            deque_.pop_front();
            return true;
        }

        return false;
    }
};

/**
 * @brief Thread-safe Stack.
 *
 * Overrides the DoPush and DoPop methods of abstract classes
 * to implement the push and pop functionality of the "SafeStack".
 */
template <typename T>
class SafeStack : public SafeQueueInner<T> {

protected:
    using SafeQueueInner<T>::deque_;
    using SafeQueueInner<T>::mutex_;

    void DoPush(const T& pt) override
    {
        deque_.push_back(pt);
    }

/**
 * @brief Encapsulate the pop_back() of dequeues
 * and implement the pop function of stack.
 */
    bool DoPop(T& pt) override
    {
        if (deque_.size() > 0) {
            pt = deque_.back();
            deque_.pop_back();
            return true;
        }

        return false;
    }
};

} // namespace OHOS
#endif
