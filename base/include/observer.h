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
 * @brief Provides the parameters and data required to call the update method.
 */
struct ObserverArg {
public:
    virtual ~ObserverArg() = default;
};

/**
 * @brief Implements the <b>Observer</b> class.
 */
class Observer;

/**
 * @brief Implements the observed class.
 */
class Observable {
public:
    virtual ~Observable() = default;
    /**
     * @brief Adds the specified observer to the set of observers.
     *
     * If `o` is valid and does not exist in the observer set, the observer
     * will be added; otherwise, this function will return directly.
     */
    void AddObserver(const std::shared_ptr<Observer>& o);

    /**
     * @brief Removes the specified observer.
     */
    void RemoveObserver(const std::shared_ptr<Observer>& o);

    /**
     * @brief Removes all observers.
     */
    void RemoveAllObservers();

    /**
     * @brief Notifies all observers with no data passed.
     *
     * This function is equivalent to <b>NotifyObservers(nullptr)</b>.
     */
    void NotifyObservers();

    /**
     * @brief Notifies all observers, with the data 'arg' passed to
	 * the observers.
     *
     * If `changed_` is true, call the `Update()` function to notify all
     * observers to respond.
     *
     * @param arg Indicates the parameters and data to be used for
	 * <b>Observer::Update()</b>.
     * @see ObserverArg.
     */
    void NotifyObservers(const ObserverArg* arg);

    /**
     * @brief Obtains the number of observers.
     */
    int GetObserversCount();

protected:

    /**
     * @brief Obtains the state of this <b>Observable</b> object.
     *
     * @return Returns the value of `changed_`.
     */
    bool HasChanged();

    /**
     * @brief Sets the state of this <b>Observable</b> object to true.
     */
    void SetChanged();

    /**
     * @brief Set the state of this <b>Observable</b> object to false.
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
     * @brief Updates this observer.
     *
     * It will be called when this observer is notified by an
	 * <b>Observable</b> object.
     */
    virtual void Update(const Observable* o, const ObserverArg* arg) = 0;
};
}
#endif