/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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

#ifndef UTILS_BASE_OBSERVER_H
#define UTILS_BASE_OBSERVER_H

#include <memory>
#include <vector>
#include <set>
#include <mutex>

namespace OHOS {

/**
 * @brief Parameters and data required to call the update method.
 */
struct ObserverArg {
public:
    virtual ~ObserverArg() = default;
};

/**
 * @brief Observer class.
 */
class Observer;

/**
 * @brief Observed class.
 */
class Observable {
public:
    virtual ~Observable() = default;
    /**
     * @brief Add the specified observer to the set of observers.
     *
     * If `o` is valid and does not exist in the observer set, the observer 
     * will be added, otherwise this function will return directly.
     */
    void AddObserver(const std::shared_ptr<Observer>& o);

    /**
     * @brief Remove the observer passed in from the parameter.
     */
    void RemoveObserver(const std::shared_ptr<Observer>& o);

    /**
     * @brief Remove all observers.
     */
    void RemoveAllObservers();

    /**
     * @brief Notify all observers without data transmission.
     *
     * Equivalent to NotifyObservers(nullptr).
     */
    void NotifyObservers();

    /**
     * @brief Notify all observers, and pass the data 'arg' to the observer.
     *
     * If the `changed_` is true, call the `Update()` function to notify all 
     * observers to respond.
     *
     * @param arg Parameters and data maybe used for Observer::Update().
     * @see ObserverArg.
     */
    void NotifyObservers(const ObserverArg* arg);

    /**
     * @brief Get the number of observers.
     */
    int GetObserversCount();

protected:

    /**
     * @brief Get the state of this Observable object.
     *
     * @return The value of `changed_`.
     */
    bool HasChanged();

    /**
     * @brief Set the state of this Observable object to true.
     */
    void SetChanged();

    /**
     * @brief Set the state of this Observable object to false.
     */
    void ClearChanged();

protected:
    std::set<std::shared_ptr<Observer>> obs; // A collection of observers.
    std::mutex mutex_;

private:
    bool changed_ = false; // The state of this Observable object.
};

class Observer {
public:
    virtual ~Observer() = default;
    /**
     * @brief The observer's interface to update itself.
     *
     * It will be called when this object is notified by an Observable object.
     */
    virtual void Update(const Observable* o, const ObserverArg* arg) = 0;
};


}

#endif

