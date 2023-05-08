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

#include <mutex>
#include "utils_log.h"
#include "common_event_sys_errors.h"
#include "io_event_reactor.h"
#include "io_event_handler.h"

namespace OHOS {
namespace Utils {
IOEventHandler::IOEventHandler()
    :prev_(nullptr), next_(nullptr), fd_(IO_EVENT_INVALID_FD), events_(Events::EVENT_NONE),
    cb_(nullptr), enabled_(false) {}

IOEventHandler::IOEventHandler(int fd, EventId events, const EventCallback& cb)
    :prev_(nullptr), next_(nullptr), fd_(fd), events_(events), cb_(cb), enabled_(false) {}

IOEventHandler::~IOEventHandler()
{
    if (prev_ != nullptr) {
        prev_->next_ = next_;
    }

    if (next_ != nullptr) {
        next_->prev_ = prev_;
    }

    prev_ = nullptr;
    next_ = nullptr;
}

bool IOEventHandler::Start(IOEventReactor* reactor)
{
    UTILS_LOGD("%{public}s: Try add handler-%{public}p to reactor-%{public}p.", \
               __FUNCTION__, reinterpret_cast<void*>(this), reinterpret_cast<void*>(reactor));
    ErrCode res = reactor->AddHandler(this);
    if (res != EVENT_SYS_ERR_OK) {
        UTILS_LOGE("%{public}s: Try add handler failed.", __FUNCTION__);
        return false;
    }

    return true;
}

bool IOEventHandler::Stop(IOEventReactor* reactor)
{
    UTILS_LOGD("%{public}s: Try remove handler-%{public}p from reactor-%{public}p.", \
               __FUNCTION__, reinterpret_cast<void*>(this), reinterpret_cast<void*>(reactor));
    ErrCode res = reactor->RemoveHandler(this);
    if (res != EVENT_SYS_ERR_OK) {
        UTILS_LOGE("%{public}s: Try remove handler failed.", __FUNCTION__);
        return false;
    }

    return true;
}

bool IOEventHandler::Update(IOEventReactor* reactor)
{
    UTILS_LOGD("%{public}s: Try update handler-%{public}p to reactor-%{public}p.", \
               __FUNCTION__, reinterpret_cast<void*>(this), reinterpret_cast<void*>(reactor));
    ErrCode res = reactor->UpdateHandler(this);
    if (res != EVENT_SYS_ERR_OK) {
        UTILS_LOGE("%{public}s: Try update handler failed.", __FUNCTION__);
        return false;
    }

    return true;
}

void IOEventHandler::EnableRead()
{
    events_ |= Events::EVENT_READ;
}

void IOEventHandler::EnableWrite()
{
    events_ |= Events::EVENT_WRITE;
}

void IOEventHandler::DisableWrite()
{
    events_ &= ~Events::EVENT_WRITE;
}
void IOEventHandler::DisableAll()
{
    events_ = Events::EVENT_NONE;
}

} // Utils
} // OHOS