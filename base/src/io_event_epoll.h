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

#ifndef UTILS_EVENT_DEMULTIPLEXER_H
#define UTILS_EVENT_DEMULTIPLEXER_H

#include <mutex>
#include <cstdint>
#include <vector>
#include <set>
#include "io_event_common.h"
#include "errors.h"

namespace OHOS {
namespace Utils {

class IOEventHandler;

class IOEventEpoll {
public:
    using REventId = uint32_t;
    using EPEventId = uint32_t;

    static constexpr int EPOLL_MAX_EVENTS_INIT = 8;
    static constexpr int EXPANSION_COEFF = 2;

    IOEventEpoll();
    IOEventEpoll(const IOEventEpoll&) = delete;
    IOEventEpoll& operator=(const IOEventEpoll&) = delete;
    virtual ~IOEventEpoll();

    virtual ErrCode SetUp();
    virtual void CleanUp();

    virtual ErrCode Polling(int timeout, std::vector<std::pair<int, REventId>>&);

    virtual ErrCode ModifyEvents(int fd, REventId events);

private:
    EPEventId Reactor2Epoll(REventId reactorEvent);
    REventId Epoll2Reactor(EPEventId epollEvents);
    bool OperateEpoll(int op, int fd, EPEventId epollEvents);

    int epollFd_;
    int maxEvents_;
    std::set<int> interestFds_;
};

} // Utils
} // OHOS
#endif /* UTILS_EVENT_DEMULTIPLEXER_H_ */
