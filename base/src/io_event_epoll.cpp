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
#include <cstdio>
#include <cstring>
#include <queue>
#include <vector>
#include <sys/epoll.h>
#include <unistd.h>
#include "utils_log.h"
#include "io_event_common.h"
#include "common_event_sys_errors.h"
#include "io_event_epoll.h"

namespace OHOS {
namespace Utils {
IOEventEpoll::IOEventEpoll()
    : epollFd_(epoll_create1(EPOLL_CLOEXEC)), maxEvents_(EPOLL_MAX_EVENTS_INIT) {}

IOEventEpoll::~IOEventEpoll()
{
    CleanUp();
}

ErrCode IOEventEpoll::SetUp()
{
    if (epollFd_ < 0) {
        epollFd_ = epoll_create1(EPOLL_CLOEXEC);
        if (epollFd_ < 0) {
            return EVENT_SYS_ERR_BADF;
        }
    }
    return EVENT_SYS_ERR_OK;
}

void IOEventEpoll::CleanUp()
{
    if (epollFd_ != IO_EVENT_INVALID_FD) {
        if (close(epollFd_) != 0) {
            UTILS_LOGW("%{public}s: Failed, cannot close fd: %{public}s.", __FUNCTION__, strerror(errno));
        }
        epollFd_ = IO_EVENT_INVALID_FD;
    }
}

bool IOEventEpoll::OperateEpoll(int op, int fd, EPEventId epollEvents)
{
    struct epoll_event event;
    bzero(&event, sizeof(event));
    event.events = epollEvents;
    event.data.fd = fd;

    if (epoll_ctl(epollFd_, op, fd, &event) != 0) {
        UTILS_LOGE("%{public}s: Operate on epoll failed, %{public}s. epoll_fd: %{public}d , operation: %{public}d, \
                   target fd: %{public}d", __FUNCTION__, strerror(errno), epollFd_, op, fd);
        return false;
    }

    switch (op) {
        case EPOLL_CTL_ADD:
            interestFds_.insert(fd);
            break;
        case EPOLL_CTL_DEL:
            interestFds_.erase(fd);
            break;
        default:
            break;
    }
    return true;
}

ErrCode IOEventEpoll::ModifyEvents(int fd, REventId events)
{
    if (fd == -1) {
        UTILS_LOGE("%{public}s: Failed, bad fd.", __FUNCTION__);
        return EVENT_SYS_ERR_BADF;
    }

    int op = EPOLL_CTL_ADD;
    if (interestFds_.find(fd) != interestFds_.end()) {
        if (events == Events::EVENT_NONE) {
            op = EPOLL_CTL_DEL;
        } else {
            op = EPOLL_CTL_MOD;
        }
    }

    if (!OperateEpoll(op, fd, Reactor2Epoll(events))) {
        UTILS_LOGE("%{public}s: Modify events failed.", __FUNCTION__);
        return EVENT_SYS_ERR_FAILED;
    }
    return EVENT_SYS_ERR_OK;
}

ErrCode IOEventEpoll::Polling(int timeout /* ms */, std::vector<std::pair<int, REventId>>& res)
{
    struct epoll_event epollEvents[maxEvents_];
    int nfds = epoll_wait(epollFd_, &epollEvents[0], maxEvents_, timeout);
    if (nfds == 0) {
        return EVENT_SYS_ERR_NOEVENT;
    }
    if (nfds == -1) {
        UTILS_LOGE("%{public}s: epoll_wait() failed, %{public}s", __FUNCTION__, strerror(errno));
        return EVENT_SYS_ERR_FAILED;
    }
    for (int idx = 0; idx < nfds; ++idx) {
        res.emplace_back(std::make_pair(epollEvents[idx].data.fd, Epoll2Reactor(epollEvents[idx].events)));
    }

    if (nfds == maxEvents_) {
        maxEvents_ *= EXPANSION_COEFF;
    }
    return EVENT_SYS_ERR_OK;
}

REventId IOEventEpoll::Epoll2Reactor(EPEventId epollEvents)
{
    REventId res = Events::EVENT_NONE;
    if ((epollEvents & EPOLLHUP) && !(epollEvents & EPOLLIN)) {
        res |= Events::EVENT_CLOSE;
    }

    if (epollEvents & EPOLLERR) {
        res |= Events::EVENT_ERROR;
    }

    if (epollEvents & (EPOLLIN | EPOLLPRI | EPOLLRDHUP)) {
        res |= Events::EVENT_READ;
    }

    if (epollEvents & EPOLLOUT) {
        res |= Events::EVENT_WRITE;
    }

    return res;
}

EPEventId IOEventEpoll::Reactor2Epoll(REventId reactorEvent)
{
    EPEventId res = 0u;

    if (reactorEvent & Events::EVENT_READ) {
        res |= EPOLLIN | EPOLLPRI;
    }

    if (reactorEvent & Events::EVENT_WRITE) {
        res |= EPOLLOUT;
    }

    if (reactorEvent & Events::EVENT_ERROR) {
        res |= EPOLLERR;
    }

    return res;
}

} // namespace Utils
} // namespace OHOS
