/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#include <atomic>
#include <mutex>
#include <ostream>
#include <queue>
#include <climits>
#include <iostream>
#include "utils_log.h"
#include "common_event_sys_errors.h"
#include "io_event_epoll.h"
#include "io_event_reactor.h"

namespace OHOS {
namespace Utils {

IOEventReactor::IOEventReactor()
    :loopReady_(false), enabled_(false), count_(0), ioHandlers_(INIT_FD_NUMS), backend_(new IOEventEpoll()) {}

IOEventReactor::~IOEventReactor()
{
    CleanUp();
}

ErrCode IOEventReactor::SetUp()
{
    if (backend_ == nullptr) {
        backend_ = std::make_unique<IOEventEpoll>();
    }

    ErrCode res = backend_->SetUp();
    if (res != EVENT_SYS_ERR_OK) {
        UTILS_LOGE("%{public}s: Backend start failed.", __FUNCTION__);
        return res;
    }

    loopReady_ = true;
    return res;
}

void IOEventReactor::InsertNodeFront(int fd, IOEventHandler* target)
{
    IOEventHandler* h = ioHandlers_[fd].head.get();
    target->next_ = h->next_;
    target->prev_ = h;
    if (h->next_ != nullptr) {
        h->next_->prev_ = target;
    }
    h->next_ = target;
}

void IOEventReactor::RemoveNode(IOEventHandler* target)
{
    target->prev_->next_ = target->next_;

    if (target->next_ != nullptr) {
        target->next_->prev_ = target->prev_;
    }

    target->prev_ = nullptr;
    target->next_ = nullptr;
}

ErrCode IOEventReactor::AddHandler(IOEventHandler* target)
{
    if (target == nullptr) {
        return EVENT_SYS_ERR_NOT_FOUND;
    }

    if (target->fd_ == -1) {
        UTILS_LOGE("%{public}s: Failed, Bad fd.", __FUNCTION__);
        return EVENT_SYS_ERR_BADF;
    }
    if (target->prev_!=nullptr) {
        UTILS_LOGW("%{public}s: Warning, already started.", __FUNCTION__);
        return EVENT_SYS_ERR_ALREADY_STARTED;
    }

    std::lock_guard<std::mutex> lock(mutex_);
    int fd = target->fd_;
    if (static_cast<size_t>(fd) > ioHandlers_.size() - 1u) {
        UTILS_LOGD("%{public}s: Resize when fd: %{public}d", __FUNCTION__, fd);
        ioHandlers_.resize(fd * EXPANSION_COEFF);
    }

    InsertNodeFront(fd, target);

    if ((ioHandlers_[fd].events & target->events_) != target->events_) {
        if (backend_ == nullptr || !UpdateToDemultiplexer(target->fd_)) {
            UTILS_LOGE("%{public}s: Update fd: %{public}d to backend failed.", __FUNCTION__, target->fd_);
            return EVENT_SYS_ERR_FAILED;
        }
    }

    target->enabled_ = true;
    count_++;
    return EVENT_SYS_ERR_OK;
}

ErrCode IOEventReactor::UpdateHandler(IOEventHandler* target)
{
    if (target == nullptr) {
        return EVENT_SYS_ERR_NOT_FOUND;
    }

    if (target->fd_ == -1) {
        UTILS_LOGE("%{public}s: Failed, Bad fd.", __FUNCTION__);
        return EVENT_SYS_ERR_BADF;
    }

    if (target->prev_!=nullptr) {
        if (!HasHandler(target)) {
            UTILS_LOGE("%{public}s: Failed, handler not found.", __FUNCTION__);
            return EVENT_SYS_ERR_NOT_FOUND;
        }
        if (backend_ == nullptr || !UpdateToDemultiplexer(target->fd_)) {
            UTILS_LOGE("%{public}s: Update fd: %{public}d to backend failed.", __FUNCTION__, target->fd_);
            return EVENT_SYS_ERR_FAILED;
        }
        return EVENT_SYS_ERR_OK;
    }

    return AddHandler(target);
}

ErrCode IOEventReactor::RemoveHandler(IOEventHandler* target)
{
    if (target == nullptr) {
        return EVENT_SYS_ERR_NOT_FOUND;
    }

    if (target->fd_ == -1) {
        UTILS_LOGE("%{public}s: Failed, Bad fd.", __FUNCTION__);
        return EVENT_SYS_ERR_BADF;
    }

    target->enabled_ = false;
    std::lock_guard<std::mutex> lock(mutex_);

    if (!HasHandler(target)) {
        UTILS_LOGE("%{public}s Failed. Handler not found.", __FUNCTION__);
        target->enabled_=true;
        return EVENT_SYS_ERR_NOT_FOUND;
    }

    RemoveNode(target);

    if (backend_ == nullptr || !UpdateToDemultiplexer(target->fd_)) {
        UTILS_LOGE("%{public}s: Update fd: %{public}d to backend failed.", __FUNCTION__, target->fd_);
        target->enabled_=true;
        return EVENT_SYS_ERR_FAILED;
    }

    count_--;
    return EVENT_SYS_ERR_OK;
}

bool IOEventReactor::HasHandler(IOEventHandler* target)
{
    for (IOEventHandler* cur = ioHandlers_[target->fd_].head.get(); cur != nullptr; cur = cur->next_) {
        if (cur == target) {
            return true;
        }
    }

    return false;
}

ErrCode IOEventReactor::FindHandler(IOEventHandler* target)
{
    if (target == nullptr) {
        return EVENT_SYS_ERR_NOT_FOUND;
    }

    if (target->fd_ == -1) {
        UTILS_LOGD("%{public}s: Failed, Bad fd.", __FUNCTION__);
        return EVENT_SYS_ERR_BADF;
    }

    std::lock_guard<std::mutex> lock(mutex_);

    if (!HasHandler(target)) {
        UTILS_LOGD("%{public}s: Handler not found.", __FUNCTION__);
        return EVENT_SYS_ERR_NOT_FOUND;
    }

    return EVENT_SYS_ERR_OK;
}

bool IOEventReactor::UpdateToDemultiplexer(int fd)
{
    uint32_t emask = 0u;
    for (IOEventHandler* cur = ioHandlers_[fd].head.get(); cur != nullptr; cur = cur->next_) {
        emask |= cur->events_;
    }

    if (emask == ioHandlers_[fd].events) {
        UTILS_LOGW("%{public}s: Warning, Interested events not changed.", __FUNCTION__);
        return true;
    }

    ErrCode res = backend_->ModifyEvents(fd, emask);
    if (res != EVENT_SYS_ERR_OK) {
        UTILS_LOGE("%{public}s: Modify events on backend failed. fd: %{public}d, \
                   new event: %{public}d, error code: %{public}d", __FUNCTION__, fd, emask, res);
        return false;
    }

    ioHandlers_[fd].events = emask;
    return true;
}

void IOEventReactor::Execute(const std::vector<EventCallback>& tasks)
{
    for (const EventCallback& cb : tasks) {
        cb();
    }
}

ErrCode IOEventReactor::HandleEvents(int fd, EventId event)
{
    std::vector<EventCallback> taskQue;
    {
        std::lock_guard<std::mutex> lock(mutex_);
        if (!(ioHandlers_[fd].events & event)) {
            UTILS_LOGD("%{public}s: Non-interested event: %{public}d with fd: %{public}d, interested events: \
                       %{public}d", __FUNCTION__, event, fd, ioHandlers_[fd].events);
            return EVENT_SYS_ERR_BADEVENT;
        }

        for (IOEventHandler* cur = ioHandlers_[fd].head.get()->next_; cur != nullptr; cur = cur->next_) {
            if (cur->events_ != Events::EVENT_NONE && cur->enabled_ && (cur->events_ & event) && cur->cb_) {
                taskQue.push_back(cur->cb_);
                UTILS_LOGD("%{public}s: Handling event success: %{public}d with fd: %{public}d; \
                           handler interested events: %{public}d, active-status: %{public}d", \
                           __FUNCTION__, event, fd, cur->events_, cur->enabled_);
            } else {
                UTILS_LOGD("%{public}s: Handling event ignore: %{public}d with fd: %{public}d; \
                           handler interested events: %{public}d, active-status: %{public}d", \
                           __FUNCTION__, event, fd, cur->events_, cur->enabled_);
            }
        }
    }

    Execute(taskQue);
    return EVENT_SYS_ERR_OK;
}

void IOEventReactor::HandleAll(const std::vector<std::pair<int, EventId>>& events)
{
    for (size_t idx = 0u; idx < events.size(); idx++) {
        int fd = events[idx].first;
        EventId event = events[idx].second;

        UTILS_LOGD("%{public}s: Processing. Handling event: %{public}d, with fd: %{public}d.", \
                   __FUNCTION__, event, fd);

        if (HandleEvents(fd, event) == EVENT_SYS_ERR_BADEVENT) {
            UTILS_LOGD("%{public}s: Received non-interested events-%{public}d.", __FUNCTION__, event);
        }
    }
}

void IOEventReactor::Run(int timeout)
{
    std::vector<std::pair<int, EventId>> gotEvents;
    while (loopReady_) {
        if (!enabled_) {
            continue;
        }
        ErrCode res;
        if (timeout == -1) {
            std::lock_guard<std::mutex> lock(mutex_);
            if (count_ ==0) {
                continue;
            }
            res = backend_->Polling(timeout, gotEvents);
        } else {
            res = backend_->Polling(timeout, gotEvents);
        }

        switch (res) {
            case EVENT_SYS_ERR_OK:
                HandleAll(gotEvents);
                gotEvents.clear();
                break;
            case EVENT_SYS_ERR_NOEVENT:
                UTILS_LOGD("%{public}s: No events captured.", __FUNCTION__);
                break;
            case EVENT_SYS_ERR_FAILED:
                UTILS_LOGE("%{public}s: Backends failed.", __FUNCTION__);
                break;
            default:
                break;
        }
    }
}

bool IOEventReactor::DoClean(int fd)
{
    if (ioHandlers_[fd].head->next_ == nullptr) {
        return true;
    }

    for (IOEventHandler* cur = ioHandlers_[fd].head->next_; cur != nullptr; cur = cur->next_) {
        cur->prev_->next_ = nullptr;
        cur->prev_ = nullptr;
        cur->enabled_ = false;
    }

    if (!UpdateToDemultiplexer(fd)) {
        UTILS_LOGD("%{public}s: Clear handler list success, while updating backend failed.", __FUNCTION__);
        return false;
    }

    return true;
}

ErrCode IOEventReactor::CleanUp()
{
    std::lock_guard<std::mutex> lock(mutex_);
    ErrCode res = EVENT_SYS_ERR_OK;
    for (size_t fd = 0u; fd < ioHandlers_.size() && fd <= INT_MAX; fd++) {
        if (!DoClean(fd)) {
            UTILS_LOGD("%{public}s Failed.", __FUNCTION__);
            res = EVENT_SYS_ERR_FAILED;
        }
    }

    return res;
}

ErrCode IOEventReactor::Clean(int fd)
{
    if (fd == -1) {
        UTILS_LOGD("%{public}s: Failed, bad fd.", __FUNCTION__);
        return EVENT_SYS_ERR_BADF;
    }

    std::lock_guard<std::mutex> lock(mutex_);
    if (!DoClean(fd)) {
        UTILS_LOGD("%{public}s: Failed.", __FUNCTION__);
        return EVENT_SYS_ERR_FAILED;
    }

    return EVENT_SYS_ERR_OK;
}

} // namespace Utils
} // namespace OHOS
