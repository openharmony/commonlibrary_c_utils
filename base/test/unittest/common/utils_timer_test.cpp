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

#include <gtest/gtest.h>
#include "timer.h"
#include "common_timer_errors.h"
#include <iostream>
#include <thread>
#include <chrono>
#include <stdatomic.h>
#include <sys/time.h>
using namespace testing::ext;
using namespace std;

namespace OHOS {
namespace {
int64_t CurMs()
{
    struct timeval tpend;
    gettimeofday(&tpend, nullptr);
    return (tpend.tv_sec * 1000000 + tpend.tv_usec) / 1000;
}

class UtilsTimerTest : public testing::Test {
public :
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void UtilsTimerTest::SetUpTestCase(void)
{
}

void UtilsTimerTest::TearDownTestCase(void)
{
}

void UtilsTimerTest::SetUp(void)
{
}

void UtilsTimerTest::TearDown(void)
{
}

std::atomic<int> g_data1(0);
void TimeOutCallback1()
{
    g_data1 = g_data1 + 1;
}

std::atomic<int> g_data2(0);
void TimeOutCallback2()
{
    g_data2 = g_data2 + 1;
}

/*
 * @tc.name: testTimer001
 * @tc.desc: timer unit test
 *
 * temporarily offline for kernel difference
HWTEST_F(UtilsTimerTest, testTimer001, TestSize.Level0)
{
    g_data1 = 0;
    Utils::Timer timer("test_timer");
    uint32_t ret = timer.Setup();
    EXPECT_EQ(Utils::TIMER_ERR_OK, ret);
    uint32_t timerId = timer.Register(TimeOutCallback1, 1);
    std::this_thread::sleep_for(std::chrono::milliseconds(8));
    timer.Unregister(timerId);
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    timer.Shutdown();
    EXPECT_GE(g_data1, 2);
    EXPECT_GE(10, g_data1);
*/

/*
 * @tc.name: testTimer002
 * @tc.desc: timer unit test
 */
HWTEST_F(UtilsTimerTest, testTimer002, TestSize.Level0)
{
    g_data1 = 0;
    Utils::Timer timer("test_timer");
    uint32_t ret = timer.Setup();
    EXPECT_EQ(Utils::TIMER_ERR_OK, ret);
    timer.Register(TimeOutCallback1, 1, true);
    std::this_thread::sleep_for(std::chrono::milliseconds(15));
    timer.Shutdown();
    EXPECT_EQ(1, g_data1);
}

/*
 * @tc.name: testTimer003
 * @tc.desc: timer unit test
 */
HWTEST_F(UtilsTimerTest, testTimer003, TestSize.Level0)
{
    g_data1 = 0;
    g_data2 = 0;
    Utils::Timer timer("test_timer");
    uint32_t ret = timer.Setup();
    EXPECT_EQ(Utils::TIMER_ERR_OK, ret);
    timer.Register(TimeOutCallback1, 1);
    timer.Register(TimeOutCallback2, 50);
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    timer.Shutdown();
    EXPECT_GE(g_data1, 8);
    EXPECT_GE(g_data2, 2);
}

static void TestTimerEvent(Utils::Timer& timer)
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
 * @tc.name: testTimer004
 * @tc.desc: timer unit test
 */
HWTEST_F(UtilsTimerTest, testTimer004, TestSize.Level0)
{
    g_data1 = 0;
    Utils::Timer timer("test_timer");
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
    bool StartTimer(int milliseconds, bool once);
    void StopTimer();
    int GetData() const
    {
        return data_;
    }
private:
    void TimeOutProc()
    {
        data_ -= 1;
    };
    int data_;
    Utils::Timer timer_;
};

bool A::Init()
{
    return timer_.Setup() == Utils::TIMER_ERR_OK;
}

bool A::StartTimer(int milliseconds, bool once)
{
    uint32_t timerId = timer_.Register(std::bind(&A::TimeOutProc, this), milliseconds, once);
    return timerId != Utils::TIMER_ERR_DEAL_FAILED;
}

void A::StopTimer()
{
    timer_.Shutdown();
}

/*
 * @tc.name: testTimer005
 * @tc.desc: timer unit test
 *
 * temporarily offline for kernel difference
HWTEST_F(UtilsTimerTest, testTimer005, TestSize.Level0)
{
    A a(10);
    EXPECT_TRUE(a.Init());
    EXPECT_TRUE(a.StartTimer(1, false));
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    a.StopTimer();
    EXPECT_GE(8, a.GetData());
}
*/

/*
 * @tc.name: testTimer006
 * @tc.desc: timer unit test
 */
HWTEST_F(UtilsTimerTest, testTimer006, TestSize.Level0)
{
    A a(10);
    EXPECT_TRUE(a.Init());
    EXPECT_TRUE(a.StartTimer(1, true));
    std::this_thread::sleep_for(std::chrono::milliseconds(20));
    a.StopTimer();
    EXPECT_EQ(9, a.GetData());
}

static void TimerEventFun(Utils::Timer& timer)
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
 * @tc.name: testTimer007
 * @tc.desc: abnormal case
 */
HWTEST_F(UtilsTimerTest, testTimer007, TestSize.Level0)
{
    g_data1 = 0;
    Utils::Timer timer("test_timer");
    uint32_t ret = timer.Setup();
    EXPECT_EQ(Utils::TIMER_ERR_OK, ret);
    TimerEventFun(timer);
    timer.Shutdown();
    timer.Shutdown(false);
    EXPECT_GE(g_data1, 5);
}

static void SleepLoopFunc()
{
    int loops = 11;
    int sleepTime = 10;
    int64_t desiredVal = 10;
    for (int i = 0; i < loops; i++) {
        int64_t pre = CurMs();
        std::this_thread::sleep_for(std::chrono::milliseconds(sleepTime));
        int64_t cur = CurMs();
        EXPECT_GE(cur - pre, desiredVal);
    }
}

/*
 * @tc.name: testTimer008
 * @tc.desc: timer sleep test for ivi
 */
HWTEST_F(UtilsTimerTest, testTimer008, TestSize.Level0)
{
    g_data1 = 0;
    Utils::Timer timer("test_timer");
    uint32_t ret = timer.Setup();
    EXPECT_EQ(Utils::TIMER_ERR_OK, ret);
    timer.Register(TimeOutCallback1, 10);

    SleepLoopFunc();
    timer.Shutdown();
    EXPECT_GE(g_data1, 10);
}

/*
 * @tc.name: testTimer009
 * @tc.desc: recursive test
 */
void DoFunc(Utils::Timer &timer, int &count)
{
    (void)timer.Register(
        [&timer, &count]() {
            count += 1;
            if (count > 9) {
                return;
            }
            DoFunc(timer, count);
        },
        10, true);
    g_data1++;
}

void DoFunc2(Utils::Timer &timer, int &count)
{
    (void)timer.Register(
        [&timer, &count]() {
            count += 1;
            if (count > 9) {
                return;
            }
            DoFunc2(timer, count);
        },
        10, true);
    g_data1++;
}

HWTEST_F(UtilsTimerTest, testTimer009, TestSize.Level0)
{
    g_data1 = 0;
    Utils::Timer timer("test_timer");
    uint32_t ret = timer.Setup();
    EXPECT_EQ(Utils::TIMER_ERR_OK, ret);

    int cnt = 0, cnt1 = 0;
    DoFunc(timer, cnt);
    DoFunc2(timer, cnt1);
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    EXPECT_GE(g_data1, 5);  /* 8 for max */
    EXPECT_GE(14, g_data1); /* 10 for min */
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    timer.Shutdown();
    EXPECT_GE(g_data1, 10); /* 18 for max */
}

static void TimerRegisterMechanism(Utils::Timer& timer, bool once)
{
    uint32_t interval = 10;
    timer.Register(TimeOutCallback1, interval, once);
    timer.Register(TimeOutCallback1, interval, !once);
    timer.Register(TimeOutCallback1, interval, once);
    timer.Register(TimeOutCallback1, interval, !once);
}

/*
 * @tc.name: testTimer010
 * @tc.desc: once timer register
 */
HWTEST_F(UtilsTimerTest, testTimer010, TestSize.Level0)
{
    g_data1 = 0;
    Utils::Timer timer("test_timer");
    uint32_t ret = timer.Setup();
    EXPECT_EQ(Utils::TIMER_ERR_OK, ret);
    bool once = true;
    TimerRegisterMechanism(timer, once);
    std::this_thread::sleep_for(std::chrono::milliseconds(52));
    timer.Shutdown();
    EXPECT_GE(g_data1, 8); /* 12 for max */
}

/*
 * @tc.name: testTimer011
 * @tc.desc: once timer register
 */
HWTEST_F(UtilsTimerTest, testTimer011, TestSize.Level0)
{
    g_data1 = 0;
    Utils::Timer timer("test_timer");
    uint32_t ret = timer.Setup();
    EXPECT_EQ(Utils::TIMER_ERR_OK, ret);
    bool once = false;
    TimerRegisterMechanism(timer, once);
    std::this_thread::sleep_for(std::chrono::milliseconds(52));
    timer.Shutdown();
    EXPECT_GE(g_data1, 8); /* 12 for max */
}

/*
 * @tc.name: testTimer012
 * @tc.desc: Test double setup.
 */
HWTEST_F(UtilsTimerTest, testTimer012, TestSize.Level0)
{
    g_data1 = 0;
    Utils::Timer timer("test_timer");
    uint32_t ret = timer.Setup();
    EXPECT_EQ(Utils::TIMER_ERR_OK, ret);
    ret = timer.Setup();
    EXPECT_EQ(Utils::TIMER_ERR_INVALID_VALUE, ret);

    timer.Shutdown();
}

/*
 * @tc.name: testTimer013
 * @tc.desc: Test uncommon operations.
 */
HWTEST_F(UtilsTimerTest, testTimer013, TestSize.Level0)
{
    g_data1 = 0;
    Utils::Timer timer("test_timer", -1);
    uint32_t ret = timer.Setup();
    EXPECT_EQ(Utils::TIMER_ERR_OK, ret);
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
    timer.Shutdown();

    Utils::Timer timer1("test_timer_1");
    ret = timer1.Setup();
    EXPECT_EQ(Utils::TIMER_ERR_OK, ret);
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
    timer1.Shutdown(false);
}
}  // namespace
}  // namespace OHOS