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

#ifndef UTILS_EVENT_HANDLER_H
#define UTILS_EVENT_HANDLER_H

#include "errors.h"
#include "io_event_common.h"
namespace OHOS {
namespace Utils {

class IOEventReactor;

class IOEventHandler {
public:
    IOEventHandler();
    explicit IOEventHandler(int fd, EventId events = Events::EVENT_NONE, const EventCallback& cb = nullptr);
    IOEventHandler& operator=(const IOEventHandler&) = delete;
    IOEventHandler(const IOEventHandler&) = delete;
    IOEventHandler& operator=(const IOEventHandler&&) = delete;
    IOEventHandler(const IOEventHandler&&) = delete;
    virtual ~IOEventHandler();

    bool Start(IOEventReactor* reactor);
    bool Stop(IOEventReactor* reactor);
    bool Update(IOEventReactor* reactor);

    inline void SetFd(int fd)
    {
        fd_ = fd;
    }

    inline void SetEvents(EventId events)
    {
        events_ = events;
    }

    inline void SetCallback(const EventCallback& cb)
    {
        cb_ = cb;
    }

    inline int GetFd() const
    {
        return fd_;
    }

    inline EventId GetEvents() const
    {
        return events_;
    }

    inline EventCallback GetCallback() const
    {
        return cb_;
    }

    inline IOEventHandler* Prev() const
    {
        return prev_;
    }

    inline IOEventHandler* Next() const
    {
        return next_;
    }

    void EnableRead();
    void EnableWrite();
    void DisableWrite();
    void DisableAll();

    inline bool IsActive()
    {
        return (prev_ != nullptr && enabled_);
    }
private:
    IOEventHandler* prev_;
    IOEventHandler* next_;

    int fd_;
    EventId events_;
    EventCallback cb_;
    bool enabled_;

    friend class IOEventReactor;
};

} // namespace Utils
} // namespace OHOS
#endif /* UTILS_EVENT_HANDLER_H_ */
