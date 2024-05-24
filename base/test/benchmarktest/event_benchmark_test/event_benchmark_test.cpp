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

#include <benchmark/benchmark.h>
#include <sys/types.h>
#include <sys/timerfd.h>
#include <sys/prctl.h>
#include "unistd.h"
#include <cstdint>
#include <cstring>
#include <string>
#include <thread>
#include <mutex>
#include <memory>
#include <atomic>
#include <algorithm>
#include <list>
#include <map>
#include <functional>
#include <iostream>
#include "common_timer_errors.h"
#include "common_event_sys_errors.h"
#include "io_event_handler.h"
#include "io_event_reactor.h"
#include <sys/time.h>
#include "benchmark_log.h"
#include "benchmark_assert.h"
using namespace OHOS::Utils;

namespace OHOS {
namespace {

class BenchmarkEventTest : public benchmark::Fixture {
public:
    BenchmarkEventTest()
    {
        Iterations(iterations);
        Repetitions(repetitions);
        ReportAggregatesOnly();
    }

    ~BenchmarkEventTest() override = default;
    void SetUp(const ::benchmark::State& state) override
    {
    }

    void TearDown(const ::benchmark::State& state) override
    {
    }

protected:
    const int32_t repetitions = 3;
    const int32_t iterations = 500;
};

int g_data = 0;
void TimerCallback1()
{
    g_data++;
}

static const int MILLI_TO_BASE = 1000;
static const int NANO_TO_BASE = 1000000000;
static constexpr int MILLI_TO_NANO = NANO_TO_BASE / MILLI_TO_BASE;
const int INVALID_FD = -1;
const int SLEEP_SIXTEEN_MILLISECONDS = 16;
const int TIMER_INIT_DELAY = 15;
const int TIMER_INIT_DELAY_TEN = 10;
constexpr uint32_t TIMEOUT_ONE_MS = 1;
constexpr uint32_t TIMEOUT_TWO_MS = 2;


class TimerFdHandler : public IOEventHandler {
public:
    using TimerEventCallback = std::function<void()>;
    TimerFdHandler(int fd, const TimerEventCallback& cb);
    ~TimerFdHandler() {}
    bool Initialize(uint32_t interval);
    void Uninitialize();
    void TimeOut();

private:
    TimerEventCallback timerCallback_;
};

void TestCallback()
{
}

void SetFdAndCallback(std::shared_ptr<IOEventHandler>& handler, benchmark::State& state)
{
    // 1. Set fd
    int fd = timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK | TFD_CLOEXEC);
    AssertUnequal(fd, INVALID_FD, "fd was not different from INVALID_FD as expected.", state);
    handler->SetFd(fd);
    AssertEqual(handler->GetFd(), fd, "handler->GetFd() did not equal fd as expected.", state);

    // 2. Set callback
    handler->SetCallback(&TestCallback);
    AssertUnequal(handler->GetCallback(), nullptr,
        "handler->GetCallback() was not different from nullptr as expected.", state);
}

/*
 * @tc.name: testIOEventHandler001
 * @tc.desc: test basic interfaces of IOEventHandler.
 */
BENCHMARK_F(BenchmarkEventTest, testIOEventHandler001)(benchmark::State& state)
{
    BENCHMARK_LOGD("EventTest testIOEventHandler001 start.");
    while (state.KeepRunning()) {
        g_data = 0;
        // 1. Create io event handler
        std::shared_ptr<IOEventHandler> handler = std::make_shared<IOEventHandler>(INVALID_FD);

        // 2. Set fd and callback
        SetFdAndCallback(handler, state);

        // 3. Set interest events
        handler->SetEvents(Events::EVENT_READ | Events::EVENT_WRITE);
        AssertEqual(handler->GetEvents(), (Events::EVENT_READ | Events::EVENT_WRITE),
            "handler->GetEvents() did not equal (Events::EVENT_READ | Events::EVENT_WRITE) as expected.", state);

        // 4. Check status
        AssertEqual(handler->Prev(), nullptr, "handler->Prev() did not equal nullptr as expected.", state);
        AssertEqual(handler->Next(), nullptr, "handler->Next() did not equal nullptr as expected.", state);
        AssertEqual(handler->IsActive(), false, "handler->IsActive() did not equal false as expected.", state);

        // 5. disable events
        handler->DisableAll();
        AssertEqual(handler->GetEvents(), Events::EVENT_NONE,
            "handler->GetEvents() did not equal Events::EVENT_NONE as expected.", state);

        // 6. enable events
        handler->EnableRead();
        handler->EnableWrite();
        AssertEqual(handler->GetEvents(), (Events::EVENT_READ | Events::EVENT_WRITE),
            "handler->GetEvents() did not equal (Events::EVENT_READ | Events::EVENT_WRITE) as expected.", state);

        // 7. disable one of the events
        handler->DisableWrite();
        AssertEqual(handler->GetEvents(), Events::EVENT_READ,
            "handler->GetEvents() did not equal Events::EVENT_READ as expected.", state);
    }
    BENCHMARK_LOGD("EventTest testIOEventHandler001 end.");
}

/*
 * @tc.name: testIOEventHandler002
 * @tc.desc: test reactor-related interfaces of IOEventHandler.
 */
BENCHMARK_F(BenchmarkEventTest, testIOEventHandler002)(benchmark::State& state)
{
    BENCHMARK_LOGD("EventTest testIOEventHandler002 start.");
    while (state.KeepRunning()) {
        g_data = 0;
        // 1. Create io event handler
        std::shared_ptr<IOEventHandler> handler = std::make_shared<IOEventHandler>(INVALID_FD);

        // 2. Set fd and callback
        SetFdAndCallback(handler, state);

        // 3. Set interest events
        handler->EnableRead();
        AssertEqual(handler->GetEvents(), Events::EVENT_READ,
            "handler->GetEvents() did not equal Events::EVENT_READ as expected.", state);

        // 4. Create a reactor but not run
        std::shared_ptr<IOEventReactor> reactor = std::make_shared<IOEventReactor>();
        AssertEqual(reactor->SetUp(), EVENT_SYS_ERR_OK,
            "reactor->SetUp() did not equal EVENT_SYS_ERR_OK as expected.", state);

        // 5. Start handler
        handler->Start(reactor.get());
        AssertEqual(reactor->FindHandler(handler.get()), EVENT_SYS_ERR_OK,
            "reactor->FindHandler(handler.get()) did not equal EVENT_SYS_ERR_OK as expected.", state);

        // 6. Change setting and update handler to the reactor
        handler->EnableWrite();
        AssertTrue((handler->Update(reactor.get())),
            "handler->Update(reactor.get()) did not equal true as expected.", state);

        // 7. Remove the handler
        handler->Stop(reactor.get());
        AssertEqual(reactor->FindHandler(handler.get()), EVENT_SYS_ERR_NOT_FOUND,
            "reactor->FindHandler(handler.get()) did not equal EVENT_SYS_ERR_NOT_FOUND as expected.", state);

        // 8. Add handler, then delete handler. handler will remove itself from the reactor during deconstruction.
        AssertTrue((handler->Start(reactor.get())),
            "handler->Start(reactor.get()) did not equal true as expected.", state);
        handler.reset();
    }
    BENCHMARK_LOGD("EventTest testIOEventHandler002 end.");
}

static void AddHandler(std::shared_ptr<IOEventReactor>& reactor, std::shared_ptr<IOEventHandler>& handler1,
                       std::shared_ptr<IOEventHandler>& handler2, std::shared_ptr<IOEventHandler>& handler3,
                       benchmark::State& state)
{
    AssertEqual(reactor->AddHandler(handler1.get()), EVENT_SYS_ERR_OK,
        "reactor->AddHandler(handler1.get()) did not equal EVENT_SYS_ERR_OK as expected.", state);
    AssertEqual(reactor->AddHandler(handler2.get()), EVENT_SYS_ERR_OK,
        "reactor->AddHandler(handler2.get()) did not equal EVENT_SYS_ERR_OK as expected.", state);
    AssertUnequal(reactor->AddHandler(handler3.get()), EVENT_SYS_ERR_OK,
        "reactor->AddHandler(handler3.get()) was not different from EVENT_SYS_ERR_OK as expected.", state);
    AssertUnequal(reactor->AddHandler(nullptr), EVENT_SYS_ERR_OK,
        "reactor->AddHandler(nullptr) was not different from EVENT_SYS_ERR_OK as expected.", state);
}

static void RemoveHandler(std::shared_ptr<IOEventReactor>& reactor, std::shared_ptr<IOEventHandler>& handler2,
                          std::shared_ptr<IOEventHandler>& handler3, std::shared_ptr<IOEventHandler>& handler4,
                          benchmark::State& state)
{
    AssertUnequal(reactor->RemoveHandler(nullptr), EVENT_SYS_ERR_OK,
        "reactor->RemoveHandler(nullptr) was not different from EVENT_SYS_ERR_OK as expected.", state);
    AssertUnequal(reactor->RemoveHandler(handler3.get()), EVENT_SYS_ERR_OK,
        "reactor->RemoveHandler(handler3.get()) was not different from EVENT_SYS_ERR_OK as expected.", state);
    AssertUnequal(reactor->RemoveHandler(handler4.get()), EVENT_SYS_ERR_OK,
        "reactor->RemoveHandler(handler4.get()) was not different from EVENT_SYS_ERR_OK as expected.", state);
    AssertEqual(reactor->RemoveHandler(handler2.get()), EVENT_SYS_ERR_OK,
        "reactor->RemoveHandler(handler2.get()) did not equal EVENT_SYS_ERR_OK as expected.", state);
}

static void UpdateHandler(std::shared_ptr<IOEventReactor>& reactor, std::shared_ptr<IOEventHandler>& handler1,
                          std::shared_ptr<IOEventHandler>& handler3, std::shared_ptr<IOEventHandler>& handler4,
                          benchmark::State& state)
{
    // Update handler
    AssertUnequal(reactor->UpdateHandler(nullptr), EVENT_SYS_ERR_OK,
        "reactor->UpdateHandler(nullptr) was not different from EVENT_SYS_ERR_OK as expected.", state);
    AssertUnequal(reactor->UpdateHandler(handler3.get()), EVENT_SYS_ERR_OK,
        "reactor->UpdateHandler(handler3.get()) was not different from EVENT_SYS_ERR_OK as expected.", state);
    AssertEqual(reactor->UpdateHandler(handler1.get()), EVENT_SYS_ERR_OK,
        "reactor->UpdateHandler(handler1.get()) did not equal EVENT_SYS_ERR_OK as expected.", state);
    AssertEqual(reactor->UpdateHandler(handler4.get()), EVENT_SYS_ERR_OK,
        "reactor->UpdateHandler(handler4.get()) did not equal EVENT_SYS_ERR_OK as expected.", state);
}

/*
 * @tc.name: testIOEventReactor001
 * @tc.desc: test basic interfaces of IOEventReactor.
 */
BENCHMARK_F(BenchmarkEventTest, testIOEventReactor001)(benchmark::State& state)
{
    BENCHMARK_LOGD("EventTest testIOEventReactor001 start.");
    while (state.KeepRunning()) {
        g_data = 0;
        // Get fd
        int fd = timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK | TFD_CLOEXEC);
        AssertUnequal(fd, INVALID_FD, "fd was not different from INVALID_FD as expected.", state);

        // 1. Create io event handlers
        std::shared_ptr<IOEventHandler> handler1 = std::make_shared<IOEventHandler>(fd);
        std::shared_ptr<IOEventHandler> handler2 = std::make_shared<IOEventHandler>(fd);
        std::shared_ptr<IOEventHandler> handler3 = std::make_shared<IOEventHandler>(INVALID_FD);
        std::shared_ptr<IOEventHandler> handler4 = std::make_shared<IOEventHandler>(fd);

        // 2. Create a reactor but not run
        std::shared_ptr<IOEventReactor> reactor = std::make_shared<IOEventReactor>();
        AssertEqual(reactor->SetUp(), EVENT_SYS_ERR_OK,
            "reactor->SetUp() did not equal EVENT_SYS_ERR_OK as expected.", state);

        // 3. Add handler
        AddHandler(reactor, handler1, handler2, handler3, state);

        // 4. Add handler from the handler side.
        AssertUnequal(handler1->Start(reactor.get()), true,
            "handler1->Start(reactor.get()) was not different from false as expected.", state);
        AssertUnequal(handler3->Start(reactor.get()), true,
            "handler3->Start(reactor.get()) was not different from false as expected.", state);

        // 5. Remove handler
        RemoveHandler(reactor, handler2, handler3, handler4, state);

        // 6. Remove handler from the handler side.
        AssertUnequal(handler2->Stop(reactor.get()), true,
            "handler2->Stop(reactor.get()) was not different from false as expected.", state);

        // 7. Update handler
        UpdateHandler(reactor, handler1, handler3, handler4, state);

        // Update handler from the handler side.
        AssertEqual(handler2->Update(reactor.get()), true,
            "handler2->Update(reactor.get()) was not different from false as expected.", state);
        AssertUnequal(handler3->Update(reactor.get()), true,
            "handler3->Update(reactor.get()) was not different from true as expected.", state);
    
        // 9. Find handler
        AssertUnequal(reactor->FindHandler(nullptr), EVENT_SYS_ERR_OK,
            "reactor->FindHandler(nullptr) was not different from EVENT_SYS_ERR_OK as expected.", state);
        AssertUnequal(reactor->FindHandler(handler3.get()), EVENT_SYS_ERR_OK,
            "reactor->FindHandler(handler3.get()) was not different from EVENT_SYS_ERR_OK as expected.", state);

        // 10. Clean handler
        AssertUnequal(reactor->Clean(INVALID_FD), EVENT_SYS_ERR_OK,
            "reactor->Clean(INVALID_FD) was not different from EVENT_SYS_ERR_OK as expected.", state);
        AssertEqual(reactor->Clean(fd), EVENT_SYS_ERR_OK,
            "reactor->Clean(fd) did not equal EVENT_SYS_ERR_OK as expected.", state);
    }
    BENCHMARK_LOGD("EventTest testIOEventReactor001 end.");
}

/*
 * @tc.name: testIOEventReactor002
 * @tc.desc: test change event but not update.
 */
BENCHMARK_F(BenchmarkEventTest, testIOEventReactor002)(benchmark::State& state)
{
    BENCHMARK_LOGD("EventTest testIOEventReactor002 start.");
    while (state.KeepRunning()) {
        g_data = 0;
        // 1. Open timer
        int fd = timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK | TFD_CLOEXEC);
        AssertUnequal(fd, INVALID_FD, "fd was not different from INVALID_FD as expected.", state);

        // 2. Create io event handlers
        std::shared_ptr<IOEventHandler> handler1 = std::make_shared<IOEventHandler>(fd);
        std::shared_ptr<IOEventHandler> handler2 = std::make_shared<IOEventHandler>(fd);

        // 3. Create a reactor but not run
        std::unique_ptr<IOEventReactor> reactor = std::make_unique<IOEventReactor>();
        AssertEqual(reactor->SetUp(), EVENT_SYS_ERR_OK,
            "reactor->SetUp() did not equal EVENT_SYS_ERR_OK as expected.", state);

        // 4. Add handler
        AssertEqual(reactor->AddHandler(handler1.get()), EVENT_SYS_ERR_OK,
            "reactor->AddHandler(handler1.get()) did not equal EVENT_SYS_ERR_OK as expected.", state);
        AssertEqual(reactor->AddHandler(handler2.get()), EVENT_SYS_ERR_OK,
            "reactor->AddHandler(handler2.get()) did not equal EVENT_SYS_ERR_OK as expected.", state);

        // 5. release one handler
        handler2.reset(); // will be removed from the inner list.
    }
    BENCHMARK_LOGD("EventTest testIOEventReactor002 end.");
}

TimerFdHandler::TimerFdHandler(int fd, const TimerEventCallback& cb)
    : IOEventHandler(fd),  timerCallback_(cb)
{
    BENCHMARK_LOGD("EventTest TimerFdHandler::TimerFdHandler is called.");
}

bool TimerFdHandler::Initialize(uint32_t interval)
{
    BENCHMARK_LOGD("EventTest bool TimerFdHandler::Initialize is called.");
    if ((GetFd() == INVALID_FD)) {
        return false;
    }

    struct itimerspec newValue = {{0, 0}, {0, 0}};
    timespec now{0, 0};
    if (clock_gettime(CLOCK_MONOTONIC, &now) == INVALID_FD) {
        return false;
    }

    // next time out time is now + interval
    newValue.it_value.tv_sec = now.tv_sec + interval / MILLI_TO_BASE;
    newValue.it_value.tv_nsec = now.tv_nsec + (interval % MILLI_TO_BASE) * MILLI_TO_NANO;
    if (newValue.it_value.tv_nsec >= NANO_TO_BASE) {
        newValue.it_value.tv_sec += 1;
        newValue.it_value.tv_nsec = newValue.it_value.tv_nsec % NANO_TO_BASE;
    }

    // interval
    newValue.it_interval.tv_sec  = interval / MILLI_TO_BASE;
    newValue.it_interval.tv_nsec = (interval % MILLI_TO_BASE) * MILLI_TO_NANO;

    if (timerfd_settime(GetFd(), TFD_TIMER_ABSTIME, &newValue, nullptr) == INVALID_FD) {
        BENCHMARK_LOGD("Set timerFd failed-%{public}s timer_fd:%{public}d", strerror(errno), GetFd());
        return false;
    }

    EnableRead();
    SetCallback(std::bind(&TimerFdHandler::TimeOut, this));

    return true;
}

void TimerFdHandler::Uninitialize()
{
    BENCHMARK_LOGD("EventTest void TimerFdHandler::Uninitialize is called.");
    DisableAll();
}

void TimerFdHandler::TimeOut()
{
    BENCHMARK_LOGD("EventTest void TimerFdHandler::TimeOut is called.");
    if (GetFd() == INVALID_FD) {
        BENCHMARK_LOGD("Invalid timer_fd.");
        return;
    }
    uint64_t expirations = 0;
    ssize_t n = ::read(GetFd(), &expirations, sizeof(expirations));
    if (n != sizeof(expirations)) {
        BENCHMARK_LOGD("reads %{public}d bytes instead of 8.", static_cast<int>(n));
    }

    if (timerCallback_) {
        timerCallback_();
    }
}

/*
 * @tc.name: testEvent001
 * @tc.desc: test handling event of timerfd.
 */
BENCHMARK_F(BenchmarkEventTest, testEvent001)(benchmark::State& state)
{
    BENCHMARK_LOGD("EventTest testEvent001 start.");
    while (state.KeepRunning()) {
        g_data = 0;
        // 1. Open timer
        int fd = timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK | TFD_CLOEXEC);
        AssertUnequal(fd, INVALID_FD, "fd was not different from INVALID_FD as expected.", state);
        // 2. Create timer event handler
        std::shared_ptr<TimerFdHandler> handler = std::make_shared<TimerFdHandler>(fd, &TimerCallback1);

        // 3. Create reactor for event loop
        std::unique_ptr<IOEventReactor> reactor = std::make_unique<IOEventReactor>();
        AssertEqual(reactor->SetUp(), EVENT_SYS_ERR_OK,
            "reactor->SetUp() did not equal EVENT_SYS_ERR_OK as expected.", state);
        reactor->EnableHandling();

        // 4. Initialize timer handler and add it to reactor
        AssertTrue((handler->Initialize(10)), "handler->Initialize(10) did not equal true as expected.", state);
        AssertTrue((handler->Start(reactor.get())),
            "handler->Start(reactor.get()) did not equal true as expected.", state);

        // 5. Run event loop
        std::thread loopThread([&reactor] {
            reactor->Run(INVALID_FD);
        });

        // 6. Wait for event handling
        std::this_thread::sleep_for(std::chrono::milliseconds(SLEEP_SIXTEEN_MILLISECONDS));

        // 7. Check result, execute once at least
        AssertGreaterThanOrEqual(g_data, 1, "g_data was not greater than or equal to 1 as expected.", state);

        // 8. terminate the event-loop (aka Run())
        reactor->Terminate();
        loopThread.join();
    }
    BENCHMARK_LOGD("EventTest testEvent001 end.");
}

std::unique_ptr<IOEventReactor> EventLoop(std::shared_ptr<TimerFdHandler>& handler, benchmark::State& state,
    std::thread& loopThread)
{
    // 1. Open timer
    int fd = timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK | TFD_CLOEXEC);
    AssertUnequal(fd, INVALID_FD, "fd was not different from INVALID_FD as expected.", state);
    // 2. Create timer event handler
    handler = std::make_shared<TimerFdHandler>(fd, &TimerCallback1);

    // 3. Create reactor for event loop
    std::unique_ptr<IOEventReactor> reactor = std::make_unique<IOEventReactor>();
    AssertEqual(reactor->SetUp(), EVENT_SYS_ERR_OK,
        "reactor->SetUp() did not equal EVENT_SYS_ERR_OK as expected.", state);

    // 4. Initialize timer handler and add it to reactor
    AssertTrue((handler->Initialize(TIMER_INIT_DELAY_TEN)),
        "handler->Initialize(TIMER_INIT_DELAY_TEN) did not equal true as expected.", state);
    AssertTrue((handler->Start(reactor.get())),
        "handler->Start(reactor.get()) did not equal true as expected.", state);

    // 5. Run event loop
    loopThread = std::thread([&reactor] {
        reactor->Run(INVALID_FD);
    });

    return reactor;
}

/*
 * @tc.name: testEvent002
 * @tc.desc: test changing event to EVENT_NONE.
 */
BENCHMARK_F(BenchmarkEventTest, testEvent002)(benchmark::State& state)
{
    BENCHMARK_LOGD("EventTest testEvent002 start.");
    while (state.KeepRunning()) {
        g_data = 0;
        std::shared_ptr<TimerFdHandler> handler;
        std::thread loopThread;
        // 1. event loop
        std::unique_ptr<IOEventReactor> reactor = EventLoop(handler, state, loopThread);

        // 2. Change settings
        reactor->DisableHandling();
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
        reactor->EnableHandling();
        handler->SetEvents(Events::EVENT_NONE);

        // 3. Wait for event handling
        std::this_thread::sleep_for(std::chrono::milliseconds(SLEEP_SIXTEEN_MILLISECONDS));

        // 4. Check result, no execution
        AssertEqual(g_data, 0, "g_data did not equal 0 as expected.", state);

        // 5. terminate the event-loop (aka Run())
        reactor->Terminate();
        loopThread.join();
    }
    BENCHMARK_LOGD("EventTest testEvent002 end.");
}

/*
 * @tc.name: testEvent003
 * @tc.desc: test disable single event.
 */
BENCHMARK_F(BenchmarkEventTest, testEvent003)(benchmark::State& state)
{
    BENCHMARK_LOGD("EventTest testEvent003 start.");
    while (state.KeepRunning()) {
        g_data = 0;
        std::shared_ptr<TimerFdHandler> handler;
        std::thread loopThread;
        // 1. event loop
        std::unique_ptr<IOEventReactor> reactor = EventLoop(handler, state, loopThread);

        // 2. Change settings
        reactor->EnableHandling();
        AssertTrue((handler->Stop(reactor.get())),
            "handler->Stop(reactor.get()) did not equal true as expected.", state);

        // 3. Check result, no execution
        AssertEqual(g_data, 0, "g_data did not equal 0 as expected.", state);

        // 4. terminate the event-loop (aka Run())
        reactor->Terminate();
        loopThread.join();
    }
    BENCHMARK_LOGD("EventTest testEvent003 end.");
}

/*
 * @tc.name: testEvent004
 * @tc.desc: test removing callback.
 */
BENCHMARK_F(BenchmarkEventTest, testEvent004)(benchmark::State& state)
{
    BENCHMARK_LOGD("EventTest testEvent004 start.");
    while (state.KeepRunning()) {
        g_data = 0;
        std::shared_ptr<TimerFdHandler> handler;
        std::thread loopThread;
        // 1. event loop
        std::unique_ptr<IOEventReactor> reactor = EventLoop(handler, state, loopThread);

        // 2. Change settings
        reactor->EnableHandling();
        handler->SetCallback(nullptr);

        // 3. Wait for event handling
        std::this_thread::sleep_for(std::chrono::milliseconds(SLEEP_SIXTEEN_MILLISECONDS));

        // 4. Check result, no execution
        AssertEqual(g_data, 0, "g_data did not equal 0 as expected.", state);

        // 5. terminate the event-loop (aka Run())
        reactor->Terminate();
        loopThread.join();
    }
    BENCHMARK_LOGD("EventTest testEvent004 end.");
}

/*
 * @tc.name: testEvent005
 * @tc.desc: test change event but not update.
 */
BENCHMARK_F(BenchmarkEventTest, testEvent005)(benchmark::State& state)
{
    BENCHMARK_LOGD("EventTest testEvent005 start.");
    while (state.KeepRunning()) {
        g_data = 0;
        // 1. Open timer
        int fd = timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK | TFD_CLOEXEC);
        AssertUnequal(fd, INVALID_FD, "fd was not different from INVALID_FD as expected.", state);
        // 2. Create timer event handler
        std::shared_ptr<TimerFdHandler> handler = std::make_shared<TimerFdHandler>(fd, &TimerCallback1);

        // 3. Create reactor for event loop
        std::unique_ptr<IOEventReactor> reactor = std::make_unique<IOEventReactor>();
        AssertEqual(reactor->SetUp(), EVENT_SYS_ERR_OK,
            "reactor->SetUp() did not equal EVENT_SYS_ERR_OK as expected.", state);

        // 4. Initialize timer handler and add it to reactor
        AssertTrue((handler->Initialize(TIMER_INIT_DELAY)),
            "handler->Initialize(TIMER_INIT_DELAY) did not equal true as expected.", state);
        AssertTrue((handler->Start(reactor.get())),
            "handler->Start(reactor.get()) did not equal true as expected.", state);

        // 5. Run event loop
        std::thread loopThread([&reactor] {
            reactor->Run(INVALID_FD);
        });

        // 6. Change settings but not update
        handler->SetEvents(Events::EVENT_WRITE);
        reactor->EnableHandling();

        // 7. Wait for event handling
        std::this_thread::sleep_for(std::chrono::milliseconds(SLEEP_SIXTEEN_MILLISECONDS));

        // 8. Check result, no execution
        AssertEqual(g_data, 0, "g_data did not equal 0 as expected.", state);

        // 9. terminate the event-loop (aka Run())
        reactor->Terminate();
        loopThread.join();
    }
    BENCHMARK_LOGD("EventTest testEvent005 end.");
}

/*
 * @tc.name: testEvent006
 * @tc.desc: test release the handler when started.
 */
BENCHMARK_F(BenchmarkEventTest, testEvent006)(benchmark::State& state)
{
    BENCHMARK_LOGD("EventTest testEvent006 start.");
    while (state.KeepRunning()) {
        g_data = 0;
        // 1. Open timer
        int fd = timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK | TFD_CLOEXEC);
        AssertUnequal(fd, INVALID_FD, "fd was not different from INVALID_FD as expected.", state);
        // 2. Create timer event handler
        std::shared_ptr<TimerFdHandler> handler = std::make_shared<TimerFdHandler>(fd, &TimerCallback1);

        // 3. Create reactor for event loop
        std::unique_ptr<IOEventReactor> reactor = std::make_unique<IOEventReactor>();
        AssertEqual(reactor->SetUp(), EVENT_SYS_ERR_OK,
            "reactor->SetUp() did not equal EVENT_SYS_ERR_OK as expected.", state);

        // 4. Initialize timer handler and add it to reactor
        AssertTrue((handler->Initialize(TIMER_INIT_DELAY)),
            "handler->Initialize(TIMER_INIT_DELAY) did not equal true as expected.", state);
        AssertTrue((handler->Start(reactor.get())),
            "handler->Start(reactor.get()) did not equal true as expected.", state);

        // 5. Run event loop
        std::thread loopThread([&reactor] {
            reactor->Run(INVALID_FD);
        });

        // 6. release eventhandler
        handler.reset();
        reactor->EnableHandling();

        // 7. Wait for event handling
        std::this_thread::sleep_for(std::chrono::milliseconds(SLEEP_SIXTEEN_MILLISECONDS));

        // 8. Check result, no execution
        AssertEqual(g_data, 0, "g_data did not equal 0 as expected.", state);

        // 9. terminate the event-loop (aka Run())
        reactor->Terminate();
        loopThread.join();
    }
    BENCHMARK_LOGD("EventTest testEvent006 end.");
}

// Try to substitue underlying implementation of OHOS::UTILS::TIMER
class TimerEventHandler {
public:
    using TimerEventCallback = std::function<void(TimerEventHandler*)>;
    TimerEventHandler(int timerFd, uint32_t timeout, bool once);
    TimerEventHandler(uint32_t timeout /* ms */, bool once);
    ~TimerEventHandler();

    TimerEventHandler(const TimerEventHandler&) = delete;
    TimerEventHandler& operator=(const TimerEventHandler&) = delete;
    TimerEventHandler(const TimerEventHandler&&) = delete;
    TimerEventHandler& operator=(const TimerEventHandler&&) = delete;

    ErrCode Initialize();
    void Uninitialize();

    bool Start(IOEventReactor* reactor);
    bool Stop(IOEventReactor* reactor);

    inline void SetTimerEventCallback(const TimerEventCallback& callback) { timerEventCallback_ = callback; }

    inline void SetTimerId(const uint32_t& id) { timerId_ = id; }

    inline uint32_t GetInterval() const { return interval_; }
    inline uint32_t GetTimerId() const { return timerId_; }
    inline int GetTimerFd() const { return handler_->GetFd(); }

private:
    void TimeOut();

private:
    bool           once_;
    uint32_t       timerId_;
    uint32_t       interval_;
    TimerEventCallback  timerEventCallback_;

    std::unique_ptr<IOEventHandler> handler_;

    friend class Timer;
};

class Timer {
public:
    using TimerCallback = std::function<void ()>;
    using TimerCallbackPtr = std::shared_ptr<TimerCallback>;
    using TimerEventCallback = TimerEventHandler::TimerEventCallback;

    explicit Timer(const std::string& name, int timeoutMs = 1000);
    virtual ~Timer() {}
    virtual uint32_t Setup();
    virtual void Shutdown(bool useJoin = true);
    uint32_t Register(const TimerCallback& callback, uint32_t interval /* ms */, bool once = false);
    void Unregister(uint32_t timerId);

private:
    void MainLoop();
    void OnTimer(TimerEventHandler* handler, const TimerCallback& callback);
    uint32_t GetValidId() const;
    int GetTimerFd(uint32_t interval /* ms */);
    void EraseUnusedTimerId(uint32_t interval, const std::vector<uint32_t>& unusedIds);

private:
    using TimerHandlerPtr = std::shared_ptr<TimerEventHandler>;
    using TimerHandlerList = std::list<TimerHandlerPtr>;

    ErrCode ScheduleTimer(const TimerEventCallback& callback, uint32_t interval, uint32_t timerId, int& timerFd,
                          bool once);
    ErrCode CancelTimer(TimerHandlerPtr target);

    std::map<uint32_t, TimerHandlerPtr> timerHandlers_;
    std::map<uint32_t, TimerHandlerList> intervalToTimers_;

    std::string name_;
    int timeoutMs_;
    std::thread thread_;
    std::unique_ptr<IOEventReactor> reactor_;
    std::mutex mutex_;
};

Timer::Timer(const std::string& name, int timeoutMs) : name_(name), timeoutMs_(timeoutMs),
    reactor_(new IOEventReactor())
{
    BENCHMARK_LOGD("EventTest Timer::Timer is called.");
}

void Timer::MainLoop()
{
    prctl(PR_SET_NAME, name_.c_str(), 0, 0, 0);

    reactor_->Run(timeoutMs_);
    BENCHMARK_LOGD("||%{public}d||Loop finished", gettid());
    BENCHMARK_LOGD("MainLoop timeoutMs_: %{public}d", timeoutMs_);

    if (reactor_->CleanUp() != EVENT_SYS_ERR_OK) {
        BENCHMARK_LOGD("||%{public}d||Reactor Clean Failed. It will clean during deconstruction", gettid());
    }
}

uint32_t Timer::Setup()
{
    BENCHMARK_LOGD("EventTest uint32_t Timer::Setup is called.");
    if (thread_.joinable()) { // avoid double assign to an active thread
        return TIMER_ERR_INVALID_VALUE;
    }

    if (reactor_->SetUp() != EVENT_SYS_ERR_OK) {
        BENCHMARK_LOGD("||%{public}d||Setup reactor failed.", gettid());
        return TIMER_ERR_DEAL_FAILED;
    }

    reactor_->EnableHandling();

    std::thread loopThread(std::bind(&Timer::MainLoop, this));
    thread_.swap(loopThread);

    return TIMER_ERR_OK;
}

void Timer::Shutdown(bool useJoin)
{
    BENCHMARK_LOGD("EventTest void Timer::Shutdown is called.");
    if (!thread_.joinable()) {
        BENCHMARK_LOGD("||%{public}d||Invalid operation. Already shutdown.", gettid());
        return;
    }

    BENCHMARK_LOGD("||%{public}d||Stop reactor.", gettid());
    reactor_->Terminate();

    if (!useJoin) {
        thread_.detach();
        return;
    }
    thread_.join();
}

ErrCode Timer::ScheduleTimer(const TimerEventCallback& callback, uint32_t interval,
                             uint32_t timerId, int& timerFd, bool once)
{
    BENCHMARK_LOGD("EventTest ErrCode Timer::ScheduleTimer is called.");
    std::shared_ptr<TimerEventHandler> handler = std::make_shared<TimerEventHandler>(timerFd, interval, once);
    BENCHMARK_LOGD("Timer::ScheduleTimer interval: %{public}u -- timerId: %{public}u -- timerFd: %{public}d",
        interval, timerId, timerFd);

    handler->SetTimerId(timerId);
    handler->SetTimerEventCallback(callback);

    uint32_t ret = handler->Initialize();
    if (ret != TIMER_ERR_OK) {
        BENCHMARK_LOGD("||%{public}d||Init timer handler failed.", gettid());
        return ret;
    }
    if (!handler->Start(reactor_.get())) {
        BENCHMARK_LOGD("||%{public}d||Start timer handler failed.", gettid());
        return TIMER_ERR_DEAL_FAILED;
    }
    timerHandlers_.emplace(timerId, handler); // Add to the id2handlers map
    intervalToTimers_[interval].push_back(handler); // Add to interval2handlerlist map
    timerFd = handler->GetTimerFd();
    return TIMER_ERR_OK;
}


uint32_t Timer::Register(const TimerCallback& callback, uint32_t interval /* ms */, bool once)
{
    BENCHMARK_LOGD("EventTest uint32_t Timer::Register is called.");
    std::lock_guard<std::mutex> lock(mutex_);

    // wrap the callback in OnTiner
    TimerEventCallback wrappedCb = std::bind(&Timer::OnTimer, this, std::placeholders::_1, callback);
    int timerFd = once ? IO_EVENT_INVALID_FD : GetTimerFd(interval); // Get timerFd
    uint32_t timerId = GetValidId();                        // Get timerId

    uint32_t ret = ScheduleTimer(wrappedCb, interval, timerId, timerFd, once);
    if (ret != TIMER_ERR_OK) {
        BENCHMARK_LOGD("||%{public}d||Try schedule task failed. timer-id:%{public}d, interval:%{public}d, "\
            "timer-fd:%{public}d", gettid(), timerId, interval, timerFd);
        return TIMER_ERR_DEAL_FAILED;
    }

    return timerId;
}

ErrCode Timer::CancelTimer(TimerHandlerPtr target)
{
    BENCHMARK_LOGD("EventTest ErrCode Timer::CancelTimer is called.");
    BENCHMARK_LOGD("||%{public}d||Cancle timer handler with fd:%{public}d", gettid(), target->GetTimerFd());
    target->Uninitialize();
    if (!target->Stop(reactor_.get())) {
        BENCHMARK_LOGD("||%{public}d||Stop timer handler failed.", gettid());
        return TIMER_ERR_DEAL_FAILED;
    }

    timerHandlers_.erase(target->timerId_);

    auto handlerList = intervalToTimers_[target->interval_];
    auto itor = std::find(handlerList.begin(), handlerList.end(), target);
    if (itor != handlerList.end()) {
        handlerList.erase(itor);
    }

    if (handlerList.empty()) {
        intervalToTimers_.erase(target->interval_);
    }

    return TIMER_ERR_OK;
}

void Timer::Unregister(uint32_t timerId)
{
    BENCHMARK_LOGD("EventTest void Timer::Unregister is called.");
    std::lock_guard<std::mutex> lock(mutex_);

    if (timerHandlers_.find(timerId) == timerHandlers_.end()) {
        BENCHMARK_LOGD("||%{public}d||Unregister failed. timer-id:%{public}d not found.", gettid(), timerId);
        return;
    }

    auto entry = timerHandlers_[timerId];
    BENCHMARK_LOGD("||%{public}d||Try remove timer handler from reactor. timerId:%{public}d, interval:%{public}u",
        gettid(), timerId, entry->interval_);

    if (CancelTimer(entry) != TIMER_ERR_OK) {
        BENCHMARK_LOGD("||%{public}d||Unregister timer handler failed.", gettid());
    }
}

void Timer::OnTimer(TimerEventHandler* handler, const TimerCallback& callback)
{
    BENCHMARK_LOGD("EventTest void Timer::OnTimer is called.");
    callback();

    if (handler->once_) {
        Unregister(handler->timerId_);
    }
}

uint32_t Timer::GetValidId() const
{
    BENCHMARK_LOGD("EventTest uint32_t Timer::GetValidId is called.");
    static std::atomic_uint32_t timerId = 1;

    while (timerHandlers_.find(timerId) != timerHandlers_.end()) {
        timerId++;
        if (timerId == UINT32_MAX) {
            timerId = 1;
        }

        if (timerId == TIMER_ERR_DEAL_FAILED) {
            timerId++;
        }
    }

    return timerId;
}

int Timer::GetTimerFd(uint32_t interval /* ms */)
{
    BENCHMARK_LOGD("EventTest int Timer::GetTimerFd is called.");
    if (intervalToTimers_.find(interval) == intervalToTimers_.end()) {
        return IO_EVENT_INVALID_FD;
    }
    auto &handlerList = intervalToTimers_[interval];
    for (const TimerHandlerPtr &ptr : handlerList) {
        if (!ptr->once_) {
            return ptr->GetTimerFd();
        }
    }
    return IO_EVENT_INVALID_FD;
}

TimerEventHandler::TimerEventHandler(int timerFd, uint32_t timeout /* ms */, bool once)
    : once_(once), interval_(timeout)
{
    BENCHMARK_LOGD("EventTest     : once_(once), interval_ is called.");
    if (timerFd == IO_EVENT_INVALID_FD) {
        handler_ = std::make_unique<IOEventHandler>(timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK | TFD_CLOEXEC));
    } else {
        handler_ = std::make_unique<IOEventHandler>(timerFd);
    }
}

TimerEventHandler::~TimerEventHandler()
{
    BENCHMARK_LOGD("EventTest TimerEventHandler::~TimerEventHandler is called.");
    if (close(handler_->GetFd()) != 0) {
        BENCHMARK_LOGD("||%{public}d||Close timer-fd failed. fd:%{public}d, interval:%{public}u, once:%{public}d",
            gettid(), handler_->GetFd(), interval_, once_);
    }
    handler_->SetFd(IO_EVENT_INVALID_FD);
}

ErrCode TimerEventHandler::Initialize()
{
    BENCHMARK_LOGD("EventTest ErrCode TimerEventHandler::Initialize is called.");
    if (handler_->GetFd() == IO_EVENT_INVALID_FD) {
        BENCHMARK_LOGD("||%{public}d||Invalid timer-fd:%{public}d, interval:%{public}u, once:%{public}d",
            gettid(), handler_->GetFd(), interval_, once_);
        return TIMER_ERR_INVALID_VALUE;
    }

    struct itimerspec newValue = {{0, 0}, {0, 0}};
    timespec now{0, 0};
    if (clock_gettime(CLOCK_MONOTONIC, &now) == INVALID_FD) {
        BENCHMARK_LOGD("||%{public}d||Get current time failed.", gettid());
        return TIMER_ERR_DEAL_FAILED;
    }

    // next time out time is now + interval
    newValue.it_value.tv_sec = now.tv_sec + interval_ / MILLI_TO_BASE;
    newValue.it_value.tv_nsec = now.tv_nsec + (interval_ % MILLI_TO_BASE) * MILLI_TO_NANO;
    if (newValue.it_value.tv_nsec >= NANO_TO_BASE) {
        newValue.it_value.tv_sec += 1;
        newValue.it_value.tv_nsec = newValue.it_value.tv_nsec % NANO_TO_BASE;
    }

    if (once_) {
        // interval, 0 means time out only once
        newValue.it_interval.tv_sec  = 0;
        newValue.it_interval.tv_nsec = 0;
    } else {
        // interval
        newValue.it_interval.tv_sec  = interval_ / MILLI_TO_BASE;
        newValue.it_interval.tv_nsec = (interval_ % MILLI_TO_BASE) * MILLI_TO_NANO;
    }

    if (timerfd_settime(handler_->GetFd(), TFD_TIMER_ABSTIME, &newValue, nullptr) == INVALID_FD) {
        BENCHMARK_LOGD("||%{public}d||Set timer-fd failed. next:%{public}lld, interval:%{public}lld",
            gettid(), static_cast<long long>(newValue.it_value.tv_sec),
            static_cast<long long>(newValue.it_interval.tv_sec));

        return TIMER_ERR_DEAL_FAILED;
    }

    handler_->SetCallback(std::bind(&TimerEventHandler::TimeOut, this));
    handler_->EnableRead();

    return TIMER_ERR_OK;
}

void TimerEventHandler::Uninitialize()
{
    BENCHMARK_LOGD("EventTest void TimerEventHandler::Uninitialize is called.");
    handler_->DisableAll();
}

bool TimerEventHandler::Start(IOEventReactor* reactor)
{
    BENCHMARK_LOGD("EventTest bool TimerEventHandler::Start is called.");
    if (handler_ == nullptr || !handler_->Start(reactor)) {
        return false;
    }

    return true;
}

bool TimerEventHandler::Stop(IOEventReactor* reactor)
{
    BENCHMARK_LOGD("EventTest bool TimerEventHandler::Stop is called.");
    if (handler_ == nullptr || !handler_->Stop(reactor)) {
        return false;
    }

    return true;
}

void TimerEventHandler::TimeOut()
{
    if (handler_->GetFd() == IO_EVENT_INVALID_FD) {
        BENCHMARK_LOGD("||%{public}d||Invalid timerfd.", gettid());
        return;
    }
    uint64_t expirations = 0;
    const size_t expirationSize = sizeof(expirations);
    ssize_t n = ::read(handler_->GetFd(), &expirations, expirationSize);
    if (n != expirationSize) {
        BENCHMARK_LOGD("||%{public}d||Reads %{public}d bytes instead of %{public}zu from timer fd.",
            gettid(), static_cast<int>(n), expirationSize);
    }

    if (timerEventCallback_) {
        timerEventCallback_(this);
    }
}


int64_t CurMs()
{
    struct timeval tpend;
    gettimeofday(&tpend, nullptr);
    return (tpend.tv_sec * 1000000 + tpend.tv_usec) / 1000; // 1000000: s to us, 1000: us to ms
}

std::atomic<int> g_data1(0);
void TimeOutCallback1()
{
    g_data1 = g_data1 + 1;
}

std::atomic<int> g_data2(0);
void TimeOutCallback2()
{
    g_data2 += 1;
}

/*
 * @tc.name: testNewTimer001
 * @tc.desc: test basic function of timer implemented by new event-system.
 */
BENCHMARK_F(BenchmarkEventTest, testNewTimer001)(benchmark::State& state)
{
    BENCHMARK_LOGD("EventTest testNewTimer001 start.");
    const int sleepDurationMs = 15;
    while (state.KeepRunning()) {
        g_data1 = 0;
        Timer timer("test_timer", 100);
        uint32_t ret = timer.Setup();
        AssertEqual(Utils::TIMER_ERR_OK, ret, "Utils::TIMER_ERR_OK did not equal ret as expected.", state);
        timer.Register(TimeOutCallback1, 1, true);
        std::this_thread::sleep_for(std::chrono::milliseconds(sleepDurationMs));
        timer.Shutdown();
        AssertEqual(1, g_data1, "1 did not equal g_data1 as expected.", state);
    }
    BENCHMARK_LOGD("EventTest testNewTimer001 end.");
}

/*
 * @tc.name: testNewTimer002
 * @tc.desc: test basic function of timer implemented by new event-system.
 */
BENCHMARK_F(BenchmarkEventTest, testNewTimer002)(benchmark::State& state)
{
    BENCHMARK_LOGD("EventTest testNewTimer002 start.");
    constexpr uint32_t sleepDurationMs = 200;
    constexpr int expectedData1 = 8;
    constexpr int expectedData2 = 2;
    while (state.KeepRunning()) {
        g_data1 = 0;
        g_data2 = 0;
        Timer timer("test_timer", 50);
        uint32_t ret = timer.Setup();
        AssertEqual(Utils::TIMER_ERR_OK, ret, "Utils::TIMER_ERR_OK did not equal ret as expected.", state);
        timer.Register(TimeOutCallback1, TIMEOUT_ONE_MS);
        timer.Register(TimeOutCallback2, TIMEOUT_TWO_MS);
        std::this_thread::sleep_for(std::chrono::milliseconds(sleepDurationMs));
        timer.Shutdown();
        AssertGreaterThanOrEqual(g_data1, expectedData1,
            "g_data1 was not greater than or equal to expectedData1 as expected.", state);
        AssertGreaterThanOrEqual(g_data2, expectedData2,
            "g_data2 was not greater than or equal to expectedData2 as expected.", state);
    }
    BENCHMARK_LOGD("EventTest testNewTimer002 end.");
}

/*
 * @tc.name: testNewTimer003
 * @tc.desc: test basic function of timer implemented by new event-system.
 */
BENCHMARK_F(BenchmarkEventTest, testNewTimer003)(benchmark::State& state)
{
    BENCHMARK_LOGD("EventTest testNewTimer003 start.");
    constexpr uint32_t sleepDurationMs = 30;
    constexpr int expectedData = 5;
    while (state.KeepRunning()) {
        g_data1 = 0;
        Timer timer("test_timer", 100);
        uint32_t ret = timer.Setup();
        AssertEqual(Utils::TIMER_ERR_OK, ret, "Utils::TIMER_ERR_OK did not equal ret as expected.", state);
        timer.Register(TimeOutCallback1, TIMEOUT_ONE_MS);
        timer.Register(TimeOutCallback1, TIMEOUT_TWO_MS);
        std::this_thread::sleep_for(std::chrono::milliseconds(sleepDurationMs));
        timer.Shutdown();
        AssertGreaterThanOrEqual(g_data1, expectedData,
            "g_data1 was not greater than or equal to expectedData as expected.", state);
    }
    BENCHMARK_LOGD("EventTest testNewTimer003 end.");
}

class A {
public:
    explicit A(int data) : data_(data), timer_("ATimer", 100) {}
    ~A() = default;
    bool Init();
    bool StartTimer(int milliseconds, bool once);
    void StopTimer();
    int GetData() const {return data_;}
private:
    void TimeOutProc()
    {
        data_ = data_ - 1;
    };
    int data_;
    Timer timer_;
};

bool A::Init()
{
    BENCHMARK_LOGD("EventTest bool A::Init is called.");
    return timer_.Setup() == Utils::TIMER_ERR_OK;
}

bool A::StartTimer(int milliseconds, bool once)
{
    BENCHMARK_LOGD("EventTest bool A::StartTimer is called.");
    uint32_t timerId = timer_.Register(std::bind(&A::TimeOutProc, this), milliseconds, once);
    return timerId != Utils::TIMER_ERR_DEAL_FAILED;
}

void A::StopTimer()
{
    BENCHMARK_LOGD("EventTest void A::StopTimer is called.");
    timer_.Shutdown();
}

/*
 * @tc.name: testNewTimer004
 * @tc.desc: test wrapper of the timer implemented by new event-system.
 */
BENCHMARK_F(BenchmarkEventTest, testNewTimer004)(benchmark::State& state)
{
    BENCHMARK_LOGD("EventTest testNewTimer004 start.");
    constexpr int initialValue = 10;
    constexpr int timeoutSeconds = 1;
    constexpr int sleepDurationMs = 20;
    constexpr int expectedData = 9;
    while (state.KeepRunning()) {
        A a(initialValue);
        AssertTrue((a.Init()), "a.Init() did not equal true as expected.", state);
        AssertTrue((a.StartTimer(timeoutSeconds, true)),
            "a.StartTimer(timeoutSeconds, true) did not equal true as expected.", state);
        std::this_thread::sleep_for(std::chrono::milliseconds(sleepDurationMs));
        a.StopTimer();
        AssertEqual(expectedData, a.GetData(), "expectedData did not equal a.GetData() as expected.", state);
    }
    BENCHMARK_LOGD("EventTest testNewTimer004 end.");
}

/*
 * @tc.name: testNewTimer005
 * @tc.desc: test abnormal case of timer implemented by new event-system.
 */
BENCHMARK_F(BenchmarkEventTest, testNewTimer005)(benchmark::State& state)
{
    BENCHMARK_LOGD("EventTest testNewTimer005 start.");
    constexpr uint32_t loopCount = 10;
    constexpr uint32_t timeoutSeconds = 7;
    constexpr uint32_t sleepDurationMs = 10;
    constexpr int expectedData = 5;
    while (state.KeepRunning()) {
        g_data1 = 0;
        Timer timer("test_timer", INVALID_FD);
        uint32_t ret = timer.Setup();
        AssertEqual(Utils::TIMER_ERR_OK, ret, "Utils::TIMER_ERR_OK did not equal ret as expected.", state);

        uint32_t timerId = 0;
        for (uint32_t i = 0; i < loopCount; i++) {
            timerId = timer.Register(TimeOutCallback1, timeoutSeconds, true);
            std::this_thread::sleep_for(std::chrono::milliseconds(sleepDurationMs));
        }
        timer.Unregister(timerId);
        timer.Unregister(timerId);

        timer.Shutdown();
        timer.Shutdown(false);
        AssertGreaterThanOrEqual(g_data1, expectedData,
            "g_data1 was not greater than or equal to expectedData as expected.", state);
    }
    BENCHMARK_LOGD("EventTest testNewTimer005 end.");
}

BENCHMARK_F(BenchmarkEventTest, testNewTimer006)(benchmark::State& state)
{
    BENCHMARK_LOGD("EventTest testNewTimer006 start.");
    constexpr uint32_t timeoutSeconds = 10;
    constexpr int loopCount = 11;
    constexpr int sleepDurationMs = 10;
    constexpr int expectedTimeDiff = 10;
    constexpr int expectedData = 10;
    while (state.KeepRunning()) {
        g_data1 = 0;
        Timer timer("test_timer", 100);
        uint32_t ret = timer.Setup();
        AssertEqual(Utils::TIMER_ERR_OK, ret, "Utils::TIMER_ERR_OK did not equal ret as expected.", state);
        timer.Register(TimeOutCallback1, timeoutSeconds);

        for (int i = 0; i < loopCount; i++) {
            int64_t pre = CurMs();
            std::this_thread::sleep_for(std::chrono::milliseconds(sleepDurationMs));
            int64_t cur = CurMs();
            AssertGreaterThanOrEqual(cur - pre, expectedTimeDiff,
                "cur - pre was not greater than or equal to expectedTimeDiff as expected.", state);
        }
        timer.Shutdown();
        AssertGreaterThanOrEqual(g_data1, expectedData,
            "g_data1 was not greater than or equal to expectedData as expected.", state);
    }
    BENCHMARK_LOGD("EventTest testNewTimer006 end.");
}

constexpr int RECURSION_DEPTH = 9;
constexpr int INTERVAL_SECONDS = 10;

/*
 * @tc.name: testNewTimer007
 * @tc.desc: recursive test of timer implemented by new event-system.
 */
void DoFunc(Timer &timer, int &count)
{
    BENCHMARK_LOGD("EventTest void DoFunc is called.");
    (void)timer.Register(
        [&timer, &count]() {
            count += 1;
            if (count > RECURSION_DEPTH) {
                return;
            }
            DoFunc(timer, count);
        },
        INTERVAL_SECONDS, true);
    g_data1++;
}

void DoFunc2(Timer &timer, int &count)
{
    BENCHMARK_LOGD("EventTest void DoFunc2 is called.");
    (void)timer.Register(
        [&timer, &count]() {
            count += 1;
            if (count > RECURSION_DEPTH) {
                return;
            }
            DoFunc2(timer, count);
        },
        INTERVAL_SECONDS, true);
    g_data1++;
}

BENCHMARK_F(BenchmarkEventTest, testNewTimer007)(benchmark::State& state)
{
    BENCHMARK_LOGD("EventTest testNewTimer007 start.");
    constexpr int expectedMinData = 5;
    constexpr int expectedMaxData = 14;
    constexpr int expectedMinDataAfterShutdown = 10;
    constexpr int sleepDurationMs = 50;
    while (state.KeepRunning()) {
        g_data1 = 0;
        Timer timer("test_timer", 100);
        uint32_t ret = timer.Setup();
        AssertEqual(Utils::TIMER_ERR_OK, ret, "Utils::TIMER_ERR_OK did not equal ret as expected.", state);

        int cnt = 0, cnt1 = 0;
        DoFunc(timer, cnt);
        DoFunc2(timer, cnt1);
        std::this_thread::sleep_for(std::chrono::milliseconds(sleepDurationMs));
        AssertGreaterThanOrEqual(g_data1, expectedMinData,
            "g_data1 was not greater than or equal to expectedMinData as expected.", state);
        AssertGreaterThanOrEqual(expectedMaxData, g_data1,
            "expectedMaxData was not greater than or equal to g_data1 as expected.", state);
        std::this_thread::sleep_for(std::chrono::milliseconds(sleepDurationMs));
        timer.Shutdown();
        AssertGreaterThanOrEqual(g_data1, expectedMinDataAfterShutdown,
            "g_data1 was not greater than or equal to expectedMinDataAfterShutdown as expected.", state);
    }
    BENCHMARK_LOGD("EventTest testNewTimer007 end.");
}

/*
 * @tc.name: testNewTimer008
 * @tc.desc: test execute-once and execute-periodly tasks.
 */
BENCHMARK_F(BenchmarkEventTest, testNewTimer008)(benchmark::State& state)
{
    BENCHMARK_LOGD("EventTest testNewTimer008 start.");
    constexpr int intervalSeconds = 10;
    constexpr int sleepDurationMs = 52;
    constexpr int expectedMinData = 8;
    while (state.KeepRunning()) {
        g_data1 = 0;
        Timer timer("test_timer", 100);
        uint32_t ret = timer.Setup();
        AssertEqual(Utils::TIMER_ERR_OK, ret, "Utils::TIMER_ERR_OK did not equal ret as expected.", state);
        timer.Register(TimeOutCallback1, intervalSeconds, true);
        timer.Register(TimeOutCallback1, intervalSeconds);
        timer.Register(TimeOutCallback1, intervalSeconds, true);
        timer.Register(TimeOutCallback1, intervalSeconds);
        std::this_thread::sleep_for(std::chrono::milliseconds(sleepDurationMs));
        timer.Shutdown();
        AssertGreaterThanOrEqual(g_data1, expectedMinData,
            "g_data1 was not greater than or equal to expectedMinData as expected.", state);
    }
    BENCHMARK_LOGD("EventTest testNewTimer008 end.");
}

/*
 * @tc.name: testIOEventHandlerConstruction001
 * @tc.desc: test construction of IOEventHandler.
 */
BENCHMARK_F(BenchmarkEventTest, testIOEventHandlerConstruction001)(benchmark::State& state)
{
    BENCHMARK_LOGD("EventTest testIOEventHandlerConstruction001 start.");
    while (state.KeepRunning()) {
        // 1. Construct IOEventHandler
        std::shared_ptr<IOEventHandler> handler = std::make_shared<IOEventHandler>();

        // 2. Check initialization
        AssertEqual(handler->GetFd(), IO_EVENT_INVALID_FD,
            "handler->GetFd() did not equal IO_EVENT_INVALID_FD as expected.", state);
        AssertEqual(handler->GetEvents(), Events::EVENT_NONE,
            "handler->GetEvents() did not equal Events::EVENT_NONE as expected.", state);
        AssertEqual(handler->GetCallback(), nullptr,
            "handler->GetCallback() was not equal to nullptr as expected.", state);
        AssertEqual(handler->Prev(), nullptr, "handler->Prev() was not equal to nullptr as expected.", state);
        AssertEqual(handler->Next(), nullptr, "handler->Next() was not equal to nullptr as expected.", state);
        AssertEqual(handler->IsActive(), false, "handler->IsActive() was not equal to false as expected.", state);
    }
    BENCHMARK_LOGD("EventTest testIOEventHandlerConstruction001 end.");
}

/*
 * @tc.name: testIOEventHandlerConstructionWithParameters001
 * @tc.desc: test construction of IOEventHandler with parameters.
 */
BENCHMARK_F(BenchmarkEventTest, testIOEventHandlerConstructionWithParameters001)(benchmark::State& state)
{
    BENCHMARK_LOGD("EventTest testIOEventHandlerConstructionWithParameters001 start.");
    while (state.KeepRunning()) {
        // 1. Define file descriptor and events
        int fd = 10;
        EventId events = Events::EVENT_READ;

        // 2. Construct IOEventHandler with parameters
        std::shared_ptr<IOEventHandler> handler = std::make_shared<IOEventHandler>(fd, events);

        // 3. Check initialization
        AssertEqual(handler->GetFd(), fd, "handler->GetFd() did not equal fd as expected.", state);
        AssertEqual(handler->GetEvents(), events, "handler->GetEvents() did not equal events as expected.", state);
        AssertEqual(handler->Prev(), nullptr, "handler->Prev() was not equal to nullptr as expected.", state);
        AssertEqual(handler->Next(), nullptr, "handler->Next() was not equal to nullptr as expected.", state);
        AssertEqual(handler->IsActive(), false, "handler->IsActive() was not equal to false as expected.", state);
    }
    BENCHMARK_LOGD("EventTest testIOEventHandlerConstructionWithParameters001 end.");
}

/*
 * @tc.name: testIOEventHandlerConstructor001
 * @tc.desc: test construction of IOEventHandler.
 */
BENCHMARK_F(BenchmarkEventTest, testIOEventHandlerConstructor001)(benchmark::State& state)
{
    BENCHMARK_LOGD("EventTest testIOEventHandlerConstructor001 start.");
    while (state.KeepRunning()) {
        // 1. Define file descriptor and events
        int fd = 10;
        EventId events = Events::EVENT_READ;

        // 2. Construct IOEventHandler with parameters and measure the time
        std::shared_ptr<IOEventHandler> handler = std::make_shared<IOEventHandler>(fd, events);

        // 3. Check if the handler is constructed correctly
        AssertEqual(handler->GetFd(), fd, "handler->GetFd() did not equal fd as expected.", state);
        AssertEqual(handler->GetEvents(), events, "handler->GetEvents() did not equal events as expected.", state);
    }
    BENCHMARK_LOGD("EventTest testIOEventHandlerConstructor001 end.");
}
}  // namespace
}  // namespace OHOS
// Run the benchmark
BENCHMARK_MAIN();
