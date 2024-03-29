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

#include <gtest/gtest.h>
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

using namespace testing::ext;
using namespace OHOS::Utils;

namespace OHOS {
namespace {

class UtilsEventTest : public testing::Test {
public:
    static void SetUpTestCase(void) {}
    static void TearDownTestCase(void) {}
    void SetUp() {}
    void TearDown() {}
};

int g_data = 0;
void TimerCallback1()
{
    std::cout << "timer callback1 invoked." << std::endl;
    g_data++;
}

static const int MILLI_TO_BASE = 1000;
static const int NANO_TO_BASE = 1000000000;
static constexpr int MILLI_TO_NANO = NANO_TO_BASE / MILLI_TO_BASE;
class TimerFdHandler : public IOEventHandler {
public:
    using TimerEventCallback = std::function<void()>;
    TimerFdHandler(int fd, const TimerEventCallback& cb);
    ~TimerFdHandler() {}
    void TimeOut();
    bool Initialize(uint32_t interval);
    void Uninitialize();

private:
    TimerEventCallback timerCallback_;
};

void TestCallback() {}

/*
 * @tc.name: testIOEventHandler001
 * @tc.desc: test basic interfaces of IOEventHandler.
 */
HWTEST_F(UtilsEventTest, testIOEventHandler001, TestSize.Level0)
{
    g_data = 0;
    // 1. Create io event handler
    std::shared_ptr<IOEventHandler> handler = std::make_shared<IOEventHandler>(-1);

    // 2. Set fd
    int fd = timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK | TFD_CLOEXEC);
    ASSERT_NE(fd, -1);
    handler->SetFd(fd);
    EXPECT_EQ(handler->GetFd(), fd);

    // 3. Set callback
    handler->SetCallback(&TestCallback);
    EXPECT_NE(handler->GetCallback(), nullptr);

    // 4. Set interest events
    handler->SetEvents(Events::EVENT_READ | Events::EVENT_WRITE);
    EXPECT_EQ(handler->GetEvents(), Events::EVENT_READ | Events::EVENT_WRITE);

    // 5. Check status
    EXPECT_EQ(handler->Prev(), nullptr);
    EXPECT_EQ(handler->Next(), nullptr);
    EXPECT_EQ(handler->IsActive(), false);

    // 6. disable events
    handler->DisableAll();
    EXPECT_EQ(handler->GetEvents(), Events::EVENT_NONE);

    // 7. enable events
    handler->EnableRead();
    handler->EnableWrite();
    EXPECT_EQ(handler->GetEvents(), Events::EVENT_READ | Events::EVENT_WRITE);

    // 8. disable one of the events
    handler->DisableWrite();
    EXPECT_EQ(handler->GetEvents(), Events::EVENT_READ);
}

/*
 * @tc.name: testIOEventHandler002
 * @tc.desc: test reactor-related interfaces of IOEventHandler.
 */
HWTEST_F(UtilsEventTest, testIOEventHandler002, TestSize.Level0)
{
    g_data = 0;
    // 1. Create io event handler
    std::shared_ptr<IOEventHandler> handler = std::make_shared<IOEventHandler>(-1);

    // 2. Set fd
    int fd = timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK | TFD_CLOEXEC);
    ASSERT_NE(fd, -1);
    handler->SetFd(fd);
    EXPECT_EQ(handler->GetFd(), fd);

    // 3. Set callback
    handler->SetCallback(&TestCallback);
    EXPECT_NE(handler->GetCallback(), nullptr);

    // 4. Set interest events
    handler->EnableRead();
    EXPECT_EQ(handler->GetEvents(), Events::EVENT_READ);

    // 5. Create a reactor but not run
    std::shared_ptr<IOEventReactor> reactor = std::make_shared<IOEventReactor>();
    ASSERT_EQ(reactor->SetUp(), EVENT_SYS_ERR_OK);

    // 6. Start handler
    handler->Start(reactor.get());
    EXPECT_EQ(reactor->FindHandler(handler.get()), EVENT_SYS_ERR_OK);

    // 7. Change setting and update handler to the reactor
    handler->EnableWrite();
    EXPECT_TRUE(handler->Update(reactor.get()));

    // 8. Remove the handler
    handler->Stop(reactor.get());
    EXPECT_EQ(reactor->FindHandler(handler.get()), EVENT_SYS_ERR_NOT_FOUND);

    // 9. Add handler, then delete handler. handler will remove itself from the reactor during deconstruction.
    ASSERT_TRUE(handler->Start(reactor.get()));
    handler.reset();
}

/*
 * @tc.name: testIOEventReactor001
 * @tc.desc: test basic interfaces of IOEventReactor.
 */
HWTEST_F(UtilsEventTest, testIOEventReactor001, TestSize.Level0)
{
    g_data = 0;
    // Get fd
    int fd = timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK | TFD_CLOEXEC);
    ASSERT_NE(fd, -1);

    // 1. Create io event handlers
    std::shared_ptr<IOEventHandler> handler1 = std::make_shared<IOEventHandler>(fd);
    std::shared_ptr<IOEventHandler> handler2 = std::make_shared<IOEventHandler>(fd);
    std::shared_ptr<IOEventHandler> handler3 = std::make_shared<IOEventHandler>(-1); // -1: invalid fd
    std::shared_ptr<IOEventHandler> handler4 = std::make_shared<IOEventHandler>(fd);

    // 2. Create a reactor but not run
    std::shared_ptr<IOEventReactor> reactor = std::make_shared<IOEventReactor>();
    ASSERT_EQ(reactor->SetUp(), EVENT_SYS_ERR_OK);

    // 3. Add handler
    EXPECT_EQ(reactor->AddHandler(handler1.get()), EVENT_SYS_ERR_OK);
    EXPECT_EQ(reactor->AddHandler(handler2.get()), EVENT_SYS_ERR_OK);
    EXPECT_NE(reactor->AddHandler(handler3.get()), EVENT_SYS_ERR_OK);
    EXPECT_NE(reactor->AddHandler(nullptr), EVENT_SYS_ERR_OK);

    // 4. Add handler from the handler side.
    EXPECT_NE(handler1->Start(reactor.get()), EVENT_SYS_ERR_OK); // already started.
    EXPECT_NE(handler3->Start(reactor.get()), EVENT_SYS_ERR_OK); // Bad fd.

    // 5. Remove handler
    EXPECT_NE(reactor->RemoveHandler(nullptr), EVENT_SYS_ERR_OK);
    EXPECT_NE(reactor->RemoveHandler(handler3.get()), EVENT_SYS_ERR_OK); // Bad fd.
    EXPECT_NE(reactor->RemoveHandler(handler4.get()), EVENT_SYS_ERR_OK);
    EXPECT_EQ(reactor->RemoveHandler(handler2.get()), EVENT_SYS_ERR_OK);

    // 6. Remove handler from the handler side.
    EXPECT_NE(handler2->Stop(reactor.get()), EVENT_SYS_ERR_OK); // Not found.

    // 7. Update handler
    EXPECT_NE(reactor->UpdateHandler(nullptr), EVENT_SYS_ERR_OK);
    EXPECT_NE(reactor->UpdateHandler(handler3.get()), EVENT_SYS_ERR_OK);
    EXPECT_EQ(reactor->UpdateHandler(handler1.get()), EVENT_SYS_ERR_OK);
    EXPECT_EQ(reactor->UpdateHandler(handler4.get()), EVENT_SYS_ERR_OK);

    // 8. Update handler from the handler side.
    EXPECT_NE(handler2->Update(reactor.get()), EVENT_SYS_ERR_OK); // Not found.
    EXPECT_NE(handler3->Update(reactor.get()), EVENT_SYS_ERR_OK); // Bad fd.

    // 9. Find handler
    EXPECT_NE(reactor->FindHandler(nullptr), EVENT_SYS_ERR_OK);
    EXPECT_NE(reactor->FindHandler(handler3.get()), EVENT_SYS_ERR_OK);

    // 10. Clean handler
    EXPECT_NE(reactor->Clean(-1), EVENT_SYS_ERR_OK);
    EXPECT_EQ(reactor->Clean(fd), EVENT_SYS_ERR_OK);
}

/*
 * @tc.name: testIOEventReactor002
 * @tc.desc: test change event but not update.
 */
HWTEST_F(UtilsEventTest, testIOEventReactor002, TestSize.Level0)
{
    g_data = 0;
    // 1. Open timer
    int fd = timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK | TFD_CLOEXEC);
    ASSERT_NE(fd, -1);

    // 2. Create io event handlers
    std::shared_ptr<IOEventHandler> handler1 = std::make_shared<IOEventHandler>(fd);
    std::shared_ptr<IOEventHandler> handler2 = std::make_shared<IOEventHandler>(fd);

    // 3. Create a reactor but not run
    std::unique_ptr<IOEventReactor> reactor = std::make_unique<IOEventReactor>();
    ASSERT_EQ(reactor->SetUp(), EVENT_SYS_ERR_OK);

    // 4. Add handler
    EXPECT_EQ(reactor->AddHandler(handler1.get()), EVENT_SYS_ERR_OK);
    EXPECT_EQ(reactor->AddHandler(handler2.get()), EVENT_SYS_ERR_OK);

    // 5. release one handler
    handler2.reset(); // will be removed from the inner list.
}

TimerFdHandler::TimerFdHandler(int fd, const TimerEventCallback& cb)
    : IOEventHandler(fd),  timerCallback_(cb) {}

bool TimerFdHandler::Initialize(uint32_t interval)
{
    if ((GetFd() == -1)) {
        return false;
    }

    struct itimerspec newValue = {{0, 0}, {0, 0}};
    timespec now{0, 0};
    if (clock_gettime(CLOCK_MONOTONIC, &now) == -1) {
        return false;
    }

    // next time out time is now + interval
    newValue.it_value.tv_sec = now.tv_sec + interval / MILLI_TO_BASE;
    newValue.it_value.tv_nsec = now.tv_nsec + (interval % MILLI_TO_BASE) * MILLI_TO_NANO;
    if (newValue.it_value.tv_nsec >= NANO_TO_BASE) {
        newValue.it_value.tv_sec = newValue.it_value.tv_sec + 1;
        newValue.it_value.tv_nsec = newValue.it_value.tv_nsec % NANO_TO_BASE;
    }

    // interval
    newValue.it_interval.tv_sec  = interval / MILLI_TO_BASE;
    newValue.it_interval.tv_nsec = (interval % MILLI_TO_BASE) * MILLI_TO_NANO;

    if (timerfd_settime(GetFd(), TFD_TIMER_ABSTIME, &newValue, nullptr) == -1) {
        std::cout << "Set timerFd failed-" << strerror(errno) << "timer_fd:" << GetFd() << ", next_time:" <<
                  newValue.it_value.tv_sec << ", interval:" << newValue.it_interval.tv_sec << std::endl;
        return false;
    }

    EnableRead();
    SetCallback(std::bind(&TimerFdHandler::TimeOut, this));

    return true;
}

void TimerFdHandler::Uninitialize()
{
    DisableAll();
}

void TimerFdHandler::TimeOut()
{
    if (GetFd() == -1) {
        std::cout << "Invalid timer_fd." << std::endl;
        return;
    }
    uint64_t expirations = 0;
    ssize_t n = ::read(GetFd(), &expirations, sizeof(expirations));
    if (n != sizeof(expirations)) {
        std::cout << "reads " << static_cast<int>(n) << "bytes instead of 8." << std::endl;
    }

    if (timerCallback_) {
        timerCallback_();
    }
}

static void InitAndRun(std::shared_ptr<TimerFdHandler>& handler, const uint32_t interval,
                       std::unique_ptr<IOEventReactor>& reactor, std::thread& loopThread)
{
    // Initialize timer handler and add it to reactor
    ASSERT_TRUE(handler->Initialize(interval));
    ASSERT_TRUE(handler->Start(reactor.get()));

    // Run event loop
    loopThread = std::thread([&reactor] {
        reactor->Run(-1);
    }
    );
}

/*
 * @tc.name: testEvent001
 * @tc.desc: test handling event of timerfd.
 */
HWTEST_F(UtilsEventTest, testEvent001, TestSize.Level0)
{
    g_data = 0;
    // 1. Open timer
    int fd = timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK | TFD_CLOEXEC);
    ASSERT_NE(fd, -1);
    // 2. Create timer event handler
    std::shared_ptr<TimerFdHandler> handler = std::make_shared<TimerFdHandler>(fd, &TimerCallback1);

    // 3. Create reactor for event loop
    std::unique_ptr<IOEventReactor> reactor = std::make_unique<IOEventReactor>();
    ASSERT_EQ(reactor->SetUp(), EVENT_SYS_ERR_OK);
    reactor->EnableHandling();

    // 4. Initialize timer handler and add it to reactor. Run event loop
    uint32_t interval = 10;
    std::thread loopThread;
    InitAndRun(handler, interval, reactor, loopThread);

    // 5. Wait for event handling
    std::this_thread::sleep_for(std::chrono::milliseconds(16));

    // 6. Check result, execute once at least
    EXPECT_GE(g_data, 1);

    // 7. terminate the event-loop (aka Run())
    reactor->Terminate();
    loopThread.join();
}

/*
 * @tc.name: testEvent002
 * @tc.desc: test changing event to EVENT_NONE.
 */
HWTEST_F(UtilsEventTest, testEvent002, TestSize.Level0)
{
    g_data = 0;
    // 1. Open timer
    int fd = timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK | TFD_CLOEXEC);
    ASSERT_NE(fd, -1);
    // 2. Create timer event handler
    std::shared_ptr<TimerFdHandler> handler = std::make_shared<TimerFdHandler>(fd, &TimerCallback1);

    // 3. Create reactor for event loop
    std::unique_ptr<IOEventReactor> reactor = std::make_unique<IOEventReactor>();
    ASSERT_EQ(reactor->SetUp(), EVENT_SYS_ERR_OK);

    // 4. Initialize timer handler and add it to reactor. Run event loop
    uint32_t interval = 10;
    std::thread loopThread;
    InitAndRun(handler, interval, reactor, loopThread);

    // 5. Change settings
    reactor->DisableHandling();
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
    reactor->EnableHandling();
    handler->SetEvents(Events::EVENT_NONE);

    // 6. Wait for event handling
    std::this_thread::sleep_for(std::chrono::milliseconds(16));

    // 7. Check result, no execution
    EXPECT_EQ(g_data, 0);

    // 8. terminate the event-loop (aka Run())
    reactor->Terminate();
    loopThread.join();
}

/*
 * @tc.name: testEvent003
 * @tc.desc: test disable single event.
 */
HWTEST_F(UtilsEventTest, testEvent003, TestSize.Level0)
{
    g_data = 0;
    // 1. Open timer
    int fd = timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK | TFD_CLOEXEC);
    ASSERT_NE(fd, -1);
    // 2. Create timer event handler
    std::shared_ptr<TimerFdHandler> handler = std::make_shared<TimerFdHandler>(fd, &TimerCallback1);

    // 3. Create reactor for event loop
    std::unique_ptr<IOEventReactor> reactor = std::make_unique<IOEventReactor>();
    ASSERT_EQ(reactor->SetUp(), EVENT_SYS_ERR_OK);

    // 4. Initialize timer handler and add it to reactor. Run event loop
    uint32_t interval = 10;
    std::thread loopThread;
    InitAndRun(handler, interval, reactor, loopThread);

    // 5. Change settings
    reactor->EnableHandling();
    ASSERT_TRUE(handler->Stop(reactor.get())); // block to get lock, so no need to wait.

    // 6. Check result, no execution
    EXPECT_EQ(g_data, 0);

    // 7. terminate the event-loop (aka Run())
    reactor->Terminate();
    loopThread.join();
}

/*
 * @tc.name: testEvent004
 * @tc.desc: test removing callback.
 */
HWTEST_F(UtilsEventTest, testEvent004, TestSize.Level0)
{
    g_data = 0;
    // 1. Open timer
    int fd = timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK | TFD_CLOEXEC);
    ASSERT_NE(fd, -1);
    // 2. Create timer event handler
    std::shared_ptr<TimerFdHandler> handler = std::make_shared<TimerFdHandler>(fd, &TimerCallback1);

    // 3. Create reactor for event loop
    std::unique_ptr<IOEventReactor> reactor = std::make_unique<IOEventReactor>();
    ASSERT_EQ(reactor->SetUp(), EVENT_SYS_ERR_OK);

    // 4. Initialize timer handler and add it to reactor. Run event loop
    uint32_t interval = 10;
    std::thread loopThread;
    InitAndRun(handler, interval, reactor, loopThread);

    // 5. Change settings
    reactor->EnableHandling();
    handler->SetCallback(nullptr);

    // 6. Wait for event handling
    std::this_thread::sleep_for(std::chrono::milliseconds(16));

    // 7. Check result, no execution
    EXPECT_EQ(g_data, 0);

    // 8. terminate the event-loop (aka Run())
    reactor->Terminate();
    loopThread.join();
}

/*
 * @tc.name: testEvent005
 * @tc.desc: test change event but not update.
 */
HWTEST_F(UtilsEventTest, testEvent005, TestSize.Level0)
{
    g_data = 0;
    // 1. Open timer
    int fd = timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK | TFD_CLOEXEC);
    ASSERT_NE(fd, -1);
    // 2. Create timer event handler
    std::shared_ptr<TimerFdHandler> handler = std::make_shared<TimerFdHandler>(fd, &TimerCallback1);

    // 3. Create reactor for event loop
    std::unique_ptr<IOEventReactor> reactor = std::make_unique<IOEventReactor>();
    ASSERT_EQ(reactor->SetUp(), EVENT_SYS_ERR_OK);

    // 4. Initialize timer handler and add it to reactor. Run event loop
    uint32_t interval = 15;
    std::thread loopThread;
    InitAndRun(handler, interval, reactor, loopThread);

    // 5. Change settings but not update
    handler->SetEvents(Events::EVENT_WRITE);
    reactor->EnableHandling();

    // 6. Wait for event handling
    std::this_thread::sleep_for(std::chrono::milliseconds(16));

    // 7. Check result, no execution
    EXPECT_EQ(g_data, 0);

    // 8. terminate the event-loop (aka Run())
    reactor->Terminate();
    loopThread.join();
}

/*
 * @tc.name: testEvent006
 * @tc.desc: test release the handler when started.
 */
HWTEST_F(UtilsEventTest, testEvent006, TestSize.Level0)
{
    g_data = 0;
    // 1. Open timer
    int fd = timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK | TFD_CLOEXEC);
    ASSERT_NE(fd, -1);
    // 2. Create timer event handler
    std::shared_ptr<TimerFdHandler> handler = std::make_shared<TimerFdHandler>(fd, &TimerCallback1);

    // 3. Create reactor for event loop
    std::unique_ptr<IOEventReactor> reactor = std::make_unique<IOEventReactor>();
    ASSERT_EQ(reactor->SetUp(), EVENT_SYS_ERR_OK);

    // 4. Initialize timer handler and add it to reactor. Run event loop
    uint32_t interval = 15;
    std::thread loopThread;
    InitAndRun(handler, interval, reactor, loopThread);

    // 5. release eventhandler
    handler.reset();
    reactor->EnableHandling();

    // 6. Wait for event handling
    std::this_thread::sleep_for(std::chrono::milliseconds(16));

    // 7. Check result, no execution
    EXPECT_EQ(g_data, 0);

    // 8. terminate the event-loop (aka Run())
    reactor->Terminate();
    loopThread.join();
}

// Try to substitue underlying implementation of OHOS::UTILS::TIMER
class TimerEventHandler {
public:
    using TimerEventCallback = std::function<void(TimerEventHandler*)>;
    TimerEventHandler(int timerFd, uint32_t timeout, bool once);
    TimerEventHandler(uint32_t timeout /* ms */, bool once);
    ~TimerEventHandler();

    TimerEventHandler(const TimerEventHandler&&) = delete;
    TimerEventHandler& operator=(const TimerEventHandler&&) = delete;
    TimerEventHandler(const TimerEventHandler&) = delete;
    TimerEventHandler& operator=(const TimerEventHandler&) = delete;

    ErrCode Initialize();
    void Uninitialize();

    bool Start(IOEventReactor* reactor);
    bool Stop(IOEventReactor* reactor);

    inline void SetTimerId(const uint32_t& id) { timerId_ = id; }

    inline void SetTimerEventCallback(const TimerEventCallback& callback) { timerEventCallback_ = callback; }

    inline uint32_t GetInterval() const { return interval_; }
    inline int GetTimerFd() const { return handler_->GetFd(); }
    inline uint32_t GetTimerId() const { return timerId_; }

private:
    void TimeOut();

private:
    bool           once_;
    uint32_t       timerId_;
    uint32_t       interval_;
    std::unique_ptr<IOEventHandler> handler_;
    TimerEventCallback  timerEventCallback_;

    friend class Timer;
};

class Timer {
public:
    using TimerCallback = std::function<void ()>;
    using TimerCallbackPtr = std::shared_ptr<TimerCallback>;
    using TimerEventCallback = TimerEventHandler::TimerEventCallback;

    explicit Timer(const std::string& name, int timeoutMs = 1000);
    virtual ~Timer()
    {
    }
    virtual uint32_t Setup();
    virtual void Shutdown(bool useJoin = true);
    uint32_t Register(const TimerCallback& callback, uint32_t interval /* ms */, bool once = false);
    void Unregister(uint32_t timerId);

private:
    void MainLoop();
    void OnTimer(TimerEventHandler* handler, const TimerCallback& callback);
    int GetTimerFd(uint32_t interval /* ms */);
    uint32_t GetValidId() const;
    void EraseUnusedTimerId(uint32_t interval, const std::vector<uint32_t>& unusedIds);

private:
    using TimerHandlerPtr = std::shared_ptr<TimerEventHandler>;
    using TimerHandlerList = std::list<TimerHandlerPtr>;

    ErrCode ScheduleTimer(const TimerEventCallback& callback, uint32_t interval, uint32_t timerId, int& timerFd,
                          bool once);
    void EraseExistNode(TimerHandlerPtr target);
    ErrCode CancelTimer(TimerHandlerPtr target);

    std::map<uint32_t, TimerHandlerList> intervalToTimers_;
    std::map<uint32_t, TimerHandlerPtr> timerHandlers_;

    std::string name_;
    int timeoutMs_;
    std::thread thread_;
    std::unique_ptr<IOEventReactor> reactor_;
    std::mutex mutex_;
};

Timer::Timer(const std::string& name, int timeoutMs) : name_(name), timeoutMs_(timeoutMs),
    reactor_(new IOEventReactor()) {}

void Timer::MainLoop()
{
    prctl(PR_SET_NAME, name_.c_str(), 0, 0, 0);

    reactor_->Run(timeoutMs_);
    std::cout << "||" << gettid() << "||" << "Loop finished" << std::endl;

    if (reactor_->CleanUp() != EVENT_SYS_ERR_OK) {
        std::cout << "||" << gettid() << "||" <<
                  "Reactor Clean Failed. It will clean during deconstruction" << std::endl;
    }
}

uint32_t Timer::Setup()
{
    if (thread_.joinable()) { // avoid double assign to an active thread
        return TIMER_ERR_INVALID_VALUE;
    }

    if (reactor_->SetUp() != EVENT_SYS_ERR_OK) {
        std::cout << "||" << gettid() << "||" << "Setup reactor failed." << std::endl;
        return TIMER_ERR_DEAL_FAILED;
    }

    reactor_->EnableHandling();

    std::thread loopThread(std::bind(&Timer::MainLoop, this));
    thread_.swap(loopThread);

    return TIMER_ERR_OK;
}

void Timer::Shutdown(bool useJoin)
{
    if (!thread_.joinable()) {
        std::cout << "||" << gettid() << "||" << "Invalid operation. Already shutdown." << std::endl;
        return;
    }

    std::cout << "||" << gettid() << "||" << "Stop reactor." << std::endl;
    reactor_->Terminate();

    if (!useJoin) {
        thread_.detach();
        return;
    }
    thread_.join();
}

ErrCode Timer::ScheduleTimer(const TimerEventCallback& callback, uint32_t interval, uint32_t timerId,
                             int& timerFd, bool once)
{
    std::shared_ptr<TimerEventHandler> handler = std::make_shared<TimerEventHandler>(timerFd, interval, once);

    handler->SetTimerId(timerId);
    handler->SetTimerEventCallback(callback);

    uint32_t ret = handler->Initialize();
    if (ret != TIMER_ERR_OK) {
        std::cout << "||" << gettid() << "||" << "Init timer handler failed." << std::endl;
        return ret;
    }
    if (!handler->Start(reactor_.get())) {
        std::cout << "||" << gettid() << "||" << "Start timer handler failed." << std::endl;
        return TIMER_ERR_DEAL_FAILED;
    }
    timerHandlers_.emplace(timerId, handler); // Add to the id2handlers map
    intervalToTimers_[interval].push_back(handler); // Add to interval2handlerlist map
    timerFd = handler->GetTimerFd();
    return TIMER_ERR_OK;
}


uint32_t Timer::Register(const TimerCallback& callback, uint32_t interval /* ms */, bool once)
{
    std::lock_guard<std::mutex> lock(mutex_);

    // wrap the callback in OnTiner
    TimerEventCallback wrappedCb = std::bind(&Timer::OnTimer, this, std::placeholders::_1, callback);
    int timerFd = once ? IO_EVENT_INVALID_FD : GetTimerFd(interval); // Get timerFd
    uint32_t timerId = GetValidId();                        // Get timerId

    uint32_t ret = ScheduleTimer(wrappedCb, interval, timerId, timerFd, once);
    if (ret != TIMER_ERR_OK) {
        std::cout << "||" << gettid() << "||" << "Try schedule task failed. timer-id:" <<
                  timerId << ", interval:" << interval << "timer-fd:" << timerFd << std::endl;
        return TIMER_ERR_DEAL_FAILED;
    }

    return timerId;
}

void Timer::EraseExistNode(TimerHandlerPtr target)
{
    auto handlerList = intervalToTimers_[target->interval_];
    auto itor = std::find(handlerList.begin(), handlerList.end(), target);
    if (itor != handlerList.end()) {
        handlerList.erase(itor);
    }

    if (handlerList.empty()) {
        intervalToTimers_.erase(target->interval_);
    }
}

ErrCode Timer::CancelTimer(TimerHandlerPtr target)
{
    std::cout << "||" << gettid() << "||" << "Cancle timer handler with fd:" <<  target->GetTimerFd() << std::endl;
    target->Uninitialize();
    if (!target->Stop(reactor_.get())) {
        std::cout << "||" << gettid() << "||" << "Stop timer handler failed." << std::endl;
        return TIMER_ERR_DEAL_FAILED;
    }
    timerHandlers_.erase(target->timerId_);
    EraseExistNode(target);
    return TIMER_ERR_OK;
}

void Timer::Unregister(uint32_t timerId)
{
    std::lock_guard<std::mutex> lock(mutex_);

    if (timerHandlers_.find(timerId) == timerHandlers_.end()) {
        std::cout << "||" << gettid() << "||" <<
                  "Unregister failed. timer-id:" << timerId << " not found." << std::endl;
        return;
    }

    auto entry = timerHandlers_[timerId];
    std::cout << "||" << gettid() << "||" << "Try remove timer handler from reactor. timerId:" << timerId <<
              ", interval:" << entry->interval_ << std::endl;

    if (CancelTimer(entry) != TIMER_ERR_OK) {
        std::cout << "||" << gettid() << "||" << "Unregister timer handler failed." << std::endl;
    }
}

void Timer::OnTimer(TimerEventHandler* handler, const TimerCallback& callback)
{
    callback();

    if (handler->once_) {
        Unregister(handler->timerId_);
    }
}

uint32_t Timer::GetValidId() const
{
    static std::atomic_uint32_t timerId = 1;

    while (timerHandlers_.find(timerId) != timerHandlers_.end()) {
        timerId = timerId + 1;
        if (timerId == UINT32_MAX) {
            timerId = 1;
        }

        if (timerId == TIMER_ERR_DEAL_FAILED) {
            timerId = timerId + 1;
        }
    }

    return timerId;
}

int Timer::GetTimerFd(uint32_t interval /* ms */)
{
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
    if (timerFd == IO_EVENT_INVALID_FD) {
        handler_ = std::make_unique<IOEventHandler>(timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK | TFD_CLOEXEC));
    } else {
        handler_ = std::make_unique<IOEventHandler>(timerFd);
    }
}

TimerEventHandler::~TimerEventHandler()
{
    if (close(handler_->GetFd()) != 0) {
        std::cout << "||" << gettid() << "||" << "Close timer-fd failed. fd:" << handler_->GetFd() << ", interval:" <<
                  interval_ << ", once:" << once_ << std::endl;
    }
    handler_->SetFd(IO_EVENT_INVALID_FD);
}

ErrCode TimerEventHandler::Initialize()
{
    if (handler_->GetFd() == IO_EVENT_INVALID_FD) {
        std::cout << "||" << gettid() << "||" << "Invalid timer-fd:" << handler_->GetFd() << ", interval:" <<
                  interval_ << ", once:" << once_ << std::endl;
        return TIMER_ERR_INVALID_VALUE;
    }

    struct itimerspec newValue = {{0, 0}, {0, 0}};
    timespec now{0, 0};
    if (clock_gettime(CLOCK_MONOTONIC, &now) == -1) {
        std::cout << "||" << gettid() << "||" << "Get current time failed." << std::endl;
        return TIMER_ERR_DEAL_FAILED;
    }

    // next time out time is now + interval
    newValue.it_value.tv_sec = now.tv_sec + interval_ / MILLI_TO_BASE;
    newValue.it_value.tv_nsec = now.tv_nsec + (interval_ % MILLI_TO_BASE) * MILLI_TO_NANO;
    if (newValue.it_value.tv_nsec >= NANO_TO_BASE) {
        newValue.it_value.tv_sec = newValue.it_value.tv_sec + 1;
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

    if (timerfd_settime(handler_->GetFd(), TFD_TIMER_ABSTIME, &newValue, nullptr) == -1) {
        std::cout << "||" << gettid() << "||" << "Set timer-fd failed. next:" <<
                  static_cast<long long>(newValue.it_value.tv_sec) << "interval:" <<
                  static_cast<long long>(newValue.it_interval.tv_sec) << std::endl;
        return TIMER_ERR_DEAL_FAILED;
    }

    handler_->SetCallback(std::bind(&TimerEventHandler::TimeOut, this));
    handler_->EnableRead();

    return TIMER_ERR_OK;
}

void TimerEventHandler::Uninitialize()
{
    handler_->DisableAll();
}

bool TimerEventHandler::Start(IOEventReactor* reactor)
{
    if (handler_ == nullptr || !handler_->Start(reactor)) {
        return false;
    }

    return true;
}

bool TimerEventHandler::Stop(IOEventReactor* reactor)
{
    if (handler_ == nullptr || !handler_->Stop(reactor)) {
        return false;
    }

    return true;
}

void TimerEventHandler::TimeOut()
{
    if (handler_->GetFd() == IO_EVENT_INVALID_FD) {
        std::cout << "||" << gettid() << "||" << "Invalid timerfd." << std::endl;
        return;
    }
    uint64_t expirations = 0;
    ssize_t n = ::read(handler_->GetFd(), &expirations, sizeof(expirations));
    if (n != sizeof(expirations)) {
        std::cout << "||" << gettid() << "||" << "Reads " << static_cast<int>(n) <<
                  " bytes instead of 8 from timer fd." << std::endl;
    }

    if (timerEventCallback_) {
        timerEventCallback_(this);
    }
}

std::atomic<int> g_data1(0);
void TimeOutCallback1()
{
    g_data1 += 1;
}

std::atomic<int> g_data2(0);
void TimeOutCallback2()
{
    g_data2 = g_data2 + 1;
}

int64_t CurMs()
{
    struct timeval tpend;
    gettimeofday(&tpend, nullptr);
    return (tpend.tv_sec * 1000000 + tpend.tv_usec) / 1000; // 1000000: s to us, 1000: us to ms
}

/*
 * @tc.name: testNewTimer001
 * @tc.desc: test basic function of timer implemented by new event-system.
 */
HWTEST_F(UtilsEventTest, testNewTimer001, TestSize.Level0)
{
    g_data1 = 0;
    Timer timer("test_timer");
    uint32_t ret = timer.Setup();
    EXPECT_EQ(Utils::TIMER_ERR_OK, ret);
    timer.Register(TimeOutCallback1, 1, true);
    std::this_thread::sleep_for(std::chrono::milliseconds(15));
    timer.Shutdown();
    EXPECT_EQ(1, g_data1);
}

/*
 * @tc.name: testNewTimer002
 * @tc.desc: test basic function of timer implemented by new event-system.
 */
HWTEST_F(UtilsEventTest, testNewTimer002, TestSize.Level0)
{
    g_data1 = 0;
    g_data2 = 0;
    Timer timer("test_timer");
    uint32_t ret = timer.Setup();
    EXPECT_EQ(Utils::TIMER_ERR_OK, ret);
    timer.Register(TimeOutCallback1, 1);
    timer.Register(TimeOutCallback2, 50);
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    timer.Shutdown();
    EXPECT_GE(g_data1, 8);
    EXPECT_GE(g_data2, 2);
}

static void TestTimerEvent(Timer& timer)
{
    uint32_t interval = 1;
    timer.Register(TimeOutCallback1, interval);
    uint32_t interval2 = 2;
    timer.Register(TimeOutCallback1, interval2);
    int sleepTime = 30;
    std::this_thread::sleep_for(std::chrono::milliseconds(sleepTime));
    timer.Shutdown();
}

/*
 * @tc.name: testNewTimer003
 * @tc.desc: test basic function of timer implemented by new event-system.
 */
HWTEST_F(UtilsEventTest, testNewTimer003, TestSize.Level0)
{
    g_data1 = 0;
    Timer timer("test_timer");
    uint32_t ret = timer.Setup();
    EXPECT_EQ(Utils::TIMER_ERR_OK, ret);
    TestTimerEvent(timer);
    EXPECT_GE(g_data1, 5);
}

class A {
public:
    explicit A(int data) : data_(data), timer_("ATimer") {}
    ~A() = default;
    bool Init();
    int GetData() const {return data_;}
    bool StartTimer(int milliseconds, bool once);
    void StopTimer();
private:
    void TimeOutProc()
    {
        data_ -= 1;
    };
    int data_;
    Timer timer_;
};

bool A::Init()
{
    return timer_.Setup() == Utils::TIMER_ERR_OK;
}

void A::StopTimer()
{
    timer_.Shutdown();
}

bool A::StartTimer(int milliseconds, bool once)
{
    uint32_t timerId = timer_.Register(std::bind(&A::TimeOutProc, this), milliseconds, once);
    return timerId != Utils::TIMER_ERR_DEAL_FAILED;
}

/*
 * @tc.name: testNewTimer004
 * @tc.desc: test wrapper of the timer implemented by new event-system.
 */
HWTEST_F(UtilsEventTest, testNewTimer004, TestSize.Level0)
{
    A a(10);
    EXPECT_TRUE(a.Init());
    EXPECT_TRUE(a.StartTimer(1, true));
    std::this_thread::sleep_for(std::chrono::milliseconds(20));
    a.StopTimer();
    EXPECT_EQ(9, a.GetData());
}

static void SleepLoop()
{
    int loops = 11;
    int64_t desiredVal = 10;
    int sleepTime = 10;
    for (int i = 0; i < loops; i++) {
        int64_t pre = CurMs();
        std::this_thread::sleep_for(std::chrono::milliseconds(sleepTime));
        int64_t cur = CurMs();
        EXPECT_GE(cur - pre, desiredVal);
    }
}

static void TimerEvent(Timer& timer)
{
    uint32_t timerId = 0;
    uint32_t loops = 10;
    uint32_t interval = 7;
    int sleepTime = 10;
    for (uint32_t i = 0; i < loops; i++) {
        timerId = timer.Register(TimeOutCallback1, interval, true);
        std::this_thread::sleep_for(std::chrono::milliseconds(sleepTime));
    }
    timer.Unregister(timerId);
    timer.Unregister(timerId);
}

/*
 * @tc.name: testNewTimer005
 * @tc.desc: test abnormal case of timer implemented by new event-system.
 */
HWTEST_F(UtilsEventTest, testNewTimer005, TestSize.Level0)
{
    g_data1 = 0;
    Timer timer("test_timer", -1);
    uint32_t ret = timer.Setup();
    EXPECT_EQ(Utils::TIMER_ERR_OK, ret);
    TimerEvent(timer);
    timer.Shutdown();
    timer.Shutdown(false);
    EXPECT_GE(g_data1, 5);
}

/*
 * @tc.name: testNewTimer006
 * @tc.desc: sleep test for ivi of timer implemented by new event-system.
 */
HWTEST_F(UtilsEventTest, testNewTimer006, TestSize.Level0)
{
    g_data1 = 0;
    Timer timer("test_timer");
    uint32_t ret = timer.Setup();
    EXPECT_EQ(Utils::TIMER_ERR_OK, ret);
    timer.Register(TimeOutCallback1, 10);
    
    SleepLoop();
    timer.Shutdown();
    EXPECT_GE(g_data1, 10);
}

/*
 * @tc.name: testNewTimer007
 * @tc.desc: recursive test of timer implemented by new event-system.
 */
void DoFunc(Timer &timer, int &count)
{
    (void)timer.Register(
        [&timer, &count]() {
            count = count + 1;
            if (count > 9) { // 9: recursion depth
                return;
            }
            DoFunc(timer, count);
        },
        10, true); // 10: interval
    g_data1++;
}

void DoFunc2(Timer &timer, int &count)
{
    (void)timer.Register(
        [&timer, &count]() {
            count = count + 1;
            if (count > 9) { // 9: recursion depth
                return;
            }
            DoFunc2(timer, count);
        },
        10, true); // 10: interval
    g_data1++;
}

HWTEST_F(UtilsEventTest, testNewTimer007, TestSize.Level0)
{
    g_data1 = 0;
    Timer timer("test_timer");
    uint32_t ret = timer.Setup();
    EXPECT_EQ(Utils::TIMER_ERR_OK, ret);

    int cnt = 0;
    int cnt1 = 0;
    DoFunc(timer, cnt);
    DoFunc2(timer, cnt1);
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    EXPECT_GE(g_data1, 5);  /* 8 for max */
    EXPECT_GE(14, g_data1); /* 10 for min */
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    timer.Shutdown();
    EXPECT_GE(g_data1, 10); /* 18 for max */
}

static void TimerRegisterMechanism(Timer& timer)
{
    uint32_t interval = 10;
    timer.Register(TimeOutCallback1, interval, true);
    timer.Register(TimeOutCallback1, interval);
    timer.Register(TimeOutCallback1, interval, true);
    timer.Register(TimeOutCallback1, interval);
}

/*
 * @tc.name: testNewTimer008
 * @tc.desc: test execute-once and execute-periodly tasks.
 */
HWTEST_F(UtilsEventTest, testNewTimer008, TestSize.Level0)
{
    g_data1 = 0;
    Timer timer("test_timer");
    uint32_t ret = timer.Setup();
    EXPECT_EQ(Utils::TIMER_ERR_OK, ret);
    TimerRegisterMechanism(timer);
    std::this_thread::sleep_for(std::chrono::milliseconds(52));
    timer.Shutdown();
    EXPECT_GE(g_data1, 8); /* 12 for max */
}


}  // namespace
}  // namespace OHOS