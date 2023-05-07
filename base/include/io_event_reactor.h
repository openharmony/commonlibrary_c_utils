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

#ifndef UTILS_EVENT_REACTOR_H
#define UTILS_EVENT_REACTOR_H

#include <cstdint>
#include <atomic>
#include <memory>
#include <mutex>
#include <vector>
#include <set>
#include <queue>
#include "io_event_common.h"
#include "errors.h"
#include "io_event_handler.h"

namespace OHOS {
namespace Utils {

class IOEventEpoll;

class IOEventReactor {
public:
    static constexpr uint8_t FLAG_CHANGED = 0x01;
    static constexpr size_t INIT_FD_NUMS = 8;
    static constexpr int EXPANSION_COEFF = 2;

    IOEventReactor();
    IOEventReactor(const IOEventReactor&) = delete;
    IOEventReactor& operator=(const IOEventReactor&) = delete;
    IOEventReactor(const IOEventReactor&&) = delete;
    IOEventReactor& operator=(const IOEventReactor&&) = delete;
    virtual ~IOEventReactor();

    ErrCode SetUp();
    ErrCode CleanUp();
    ErrCode Clean(int fd);

    ErrCode AddHandler(IOEventHandler* target);
    ErrCode RemoveHandler(IOEventHandler* target);
    ErrCode UpdateHandler(IOEventHandler* target);
    ErrCode FindHandler(IOEventHandler* target);

    void Run(int timeout);

    inline void Terminate()
    {
        loopReady_ = false;
    }

    inline void EnableHandling()
    {
        enabled_ = true;
    }

    inline void DisableHandling()
    {
        enabled_ = false;
    }
private:
    struct FdEvents {
        std::shared_ptr<IOEventHandler> head;
        EventId events;
        uint8_t flags;

        FdEvents()
        {
            head = std::make_shared<IOEventHandler>();
            events = Events::EVENT_NONE;
            flags = 0u;
        }
    };

    bool HasHandler(IOEventHandler* target);
    void InsertNodeFront(int fd, IOEventHandler* target);
    void RemoveNode(IOEventHandler* target);

    void HandleAll(const std::vector<std::pair<int, EventId>>&);
    void Execute(const std::vector<EventCallback>& tasks);

    ErrCode HandleEvents(int fd, EventId events);
    bool UpdateToDemultiplexer(int fd);

    bool DoClean(int fd);

    std::mutex mutex_;
    std::atomic<bool> loopReady_;
    std::atomic<bool> enabled_;
    std::atomic<uint32_t> count_;
    std::vector<struct FdEvents> ioHandlers_;
    std::unique_ptr<IOEventEpoll> backend_;
};

} // namespace Utils
} // namespace OHOS
#endif