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
#include "event_reactor.h"
#include "event_handler.h"
#include "event_demultiplexer.h"
#include "timer_event_handler.h"
#include "common_timer_errors.h"
#include "utils_log.h"

namespace OHOS {
namespace Utils {

EventReactor::EventReactor()
    :loopReady_(false), switch_(false), demultiplexer_(new EventDemultiplexer())
{
}

EventReactor::~EventReactor()
{
}

void EventReactor::UpdateEventHandler(EventHandler* handler)
{
    if ((handler != nullptr) && (handler->GetEventReactor() == this) && (demultiplexer_ != nullptr)) {
        if (demultiplexer_->UpdateEventHandler(handler) != 0) {
            UTILS_LOGE("updateEventHandler failed.");
        }
    }
}

uint32_t EventReactor::SetUp()
{
    if (demultiplexer_ == nullptr) {
        return TIMER_ERR_INVALID_VALUE;
    }

    uint32_t ret = demultiplexer_->StartUp(); // return TIME_ERR_OK, if demultiplexer has been started.
    if (ret != 0) {
        UTILS_LOGE("demultiplexer startUp failed.");
        return ret;
    }

    loopReady_ = true;
    return TIMER_ERR_OK;
}

void EventReactor::CleanUp()
{
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    for (auto &itor : timerEventHandlers_) {
        itor->Uninitialize();
    }
}

void EventReactor::RunLoop(int timeout) const
{
    if (demultiplexer_ == nullptr) {
        UTILS_LOGE("demultiplexer_ is nullptr.");
        return;
    }

    std::vector<epoll_event> epollEvents(demultiplexer_->GetMaxEvents());
    while (loopReady_ && switch_) {
        if (demultiplexer_->Polling(timeout, epollEvents) == EPOLL_CRITICAL_ERROR) {
            UTILS_LOGD("polling critical error occure: %{public}d", timeout);
            break;
        }
    }

    loopReady_ = false;
}

void EventReactor::SwitchOn()
{
    switch_ = true;
}

void EventReactor::SwitchOff()
{
    switch_ = false;
}

uint32_t EventReactor::ScheduleTimer(const TimerCallback& cb, uint32_t interval, int& timerFd, bool once)
{
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    std::shared_ptr<TimerEventHandler> handler = std::make_shared<TimerEventHandler>(this, interval, once);
    handler->SetTimerCallback(cb);
    uint32_t ret = handler->Initialize();
    if (ret != TIMER_ERR_OK) {
        UTILS_LOGD("ScheduleTimer %{public}d initialize failed", interval);
        return ret;
    }

    timerFd = handler->GetHandle();
    timerEventHandlers_.push_back(handler);
    return TIMER_ERR_OK;
}

void EventReactor::CancelTimer(int timerFd)
{
    UTILS_LOGD("Cancel timer, timerFd: %{public}d.", timerFd);
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    auto itor = timerEventHandlers_.begin();
    for (; itor != timerEventHandlers_.end(); ++itor) {
        if ((*itor)->GetHandle() == timerFd) {
            (*itor)->Uninitialize();
            timerEventHandlers_.erase(itor);
            return;
        }
    }
}

} // namespace Utils
} // namespace OHOS
