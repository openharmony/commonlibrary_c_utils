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

#ifndef UTILS_TIMER_H
#define UTILS_TIMER_H

#include <sys/types.h>
#include <cstdint>
#include <string>
#include <list>
#include <map>
#include <mutex>
#include <thread>
#include <vector>

#include "../src/event_reactor.h"

namespace OHOS {
namespace Utils {
/**
 * @brief This is a timer manager.
 *
 * After "Timer" started, users can register several timed events, which can be
 * continuous or once, to it. Some points need to be noticed:\n
 * 1. Timer should be set up(via Setup()) before use, and shutdown
 * (via Shutdown()) before its deconstruction.\n
 * 2. Timer should be set up first and then shutdown. Avoid delegating them to
 * different threads since it may cause multithreading problem.\n
 * 3. Set up Timer again would not reset this Timer, but return
 * `TIMER_ERR_INVALID_VALUE`. If a reset operation is required, shut Timer down
 * first and then set it up.\n
 * 4. Parameter in Shutdown() determines whether the thread in Timer would be
 * detach or join(True(default) --> join; False --> detach). Detach operation
 * would cause possible multithreading problems, thus is not recommended. If a
 * detach operation is required, availability of related objects used in
 * `thread_` should be guaranteed.
 */
class Timer {
public:
    using TimerCallback = std::function<void ()>;
    using TimerCallbackPtr = std::shared_ptr<TimerCallback>;
    using TimerListCallback = std::function<void (int timerFd)>;

public:
    /**
     * @brief Construct Timer.
     *
     * If performance-sensitive, change "timeoutMs" larger before Setup.
     * "timeoutMs" default-value(1000ms), performance-estimate: occupy
     * fixed-100us in every default-value(1000ms).
     *
     * @param name Name for Timer. It is used as the name of thread in Timer.
     * @param timeoutMs Time for waiting timer events. It is an integer in
     * [-1, INT32MAX], but -1 and 0 is not recommended. -1 means waiting
     * forever(until event-trigger). 0 means no waiting, which occupies too
     * much cpu time. others means waiting(until event-trigger).
     */
    explicit Timer(const std::string& name, int timeoutMs = 1000);
    virtual ~Timer() {}

    /**
     * @brief Set up "Timer".
     *
     * Do not set up repeatly before shutdown.
     */
    virtual uint32_t Setup();

    /**
     * @brief Shut down "Timer".
     *
     * There are two modes to shut the "Timer" down: blocking and unblocking.
     * Blocking mode will shut "Timer" down until all running events in "Timer"
     * finished. If "timeoutMs" is set as -1, use unblocking mode to avoid
     * deadloop.
     *
     * @param useJoin Shutdown mode. true means blocking. false means
     * unblocking(not recommended).
     */
    virtual void Shutdown(bool useJoin = true);

    /**
     * @brief Regist timed events.
     *
     * @param callback callback function of a timed event.
     * @param interval interval time(ms) of a timed event.
     * @param once continuity of a timed event. true means discontinuous. false
     * means continuous. Default is false.
     * @return return ID of a timed event. You can use it as parameter of
     * Unregister().
     * @see Unregister
     */
    uint32_t Register(const TimerCallback& callback, uint32_t interval /* ms */, bool once = false);
    /**
     * @brief Delete a timed events.
     *
     * @param timerId ID of the timed event need to be deleted. Users can get
     * it by Register().
     * @see Register
     */
    void Unregister(uint32_t timerId);

private:
    void MainLoop();
    void OnTimer(int timerFd);
    virtual uint32_t DoRegister(const TimerListCallback& callback, uint32_t interval, bool once, int &timerFd);
    virtual void DoUnregister(uint32_t interval);
    void DoTimerListCallback(const TimerListCallback& callback, int timerFd);
    uint32_t GetValidId(uint32_t timerId) const;
    int GetTimerFd(uint32_t interval /* ms */);
    void EraseUnusedTimerId(uint32_t interval, const std::vector<uint32_t>& unusedIds);

private:
    struct TimerEntry {
        uint32_t       timerId;  // unique id
        uint32_t       interval;  // million second
        TimerCallback  callback;
        bool           once;
        int            timerFd;
    };

    using TimerEntryPtr = std::shared_ptr<TimerEntry>;
    using TimerEntryList = std::list<TimerEntryPtr>;

    std::map<uint32_t, TimerEntryList> intervalToTimers_;  // interval to TimerEntryList
    std::map<uint32_t, TimerEntryPtr> timerToEntries_;  // timer_id to TimerEntry

    std::string name_;
    int timeoutMs_;
    std::thread thread_;
    std::unique_ptr<EventReactor> reactor_;
    std::map<uint32_t, uint32_t> timers_;  // timer_fd to interval
    std::mutex mutex_;
};

} // namespace Utils
} // namespace OHOS
#endif

