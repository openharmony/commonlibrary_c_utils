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
#include "timer.h"
#include "common_timer_errors.h"
#include <iostream>
#include <thread>
#include <chrono>
#include <stdatomic.h>
#include <sys/time.h>
#include "benchmark_log.h"
#include "benchmark_assert.h"
using namespace std;

namespace OHOS {
namespace {

const int MICROSECONDS_IN_ONE_SECOND = 1000000;
const int MICROSECONDS_IN_ONE_MILLISECOND = 1000;

int64_t CurMs()
{
    struct timeval tpend;
    gettimeofday(&tpend, nullptr);
    return (tpend.tv_sec * MICROSECONDS_IN_ONE_SECOND + tpend.tv_usec) / MICROSECONDS_IN_ONE_MILLISECOND;
}

class BenchmarkTimerTest : public benchmark::Fixture {
public:
    void SetUp(const ::benchmark::State& state) override
    {
    }

    void TearDown(const ::benchmark::State& state) override
    {
    }

    BenchmarkTimerTest()
    {
        Iterations(iterations);
        Repetitions(repetitions);
        ReportAggregatesOnly();
    }

    ~BenchmarkTimerTest() override = default;

protected:
    const int32_t repetitions = 3;
    const int32_t iterations = 50;
};

std::atomic<int> g_data1(0);
void TimeOutCallback1()
{
    g_data1 += 1;
}

std::atomic<int> g_data2(0);
void TimeOutCallback2()
{
    g_data2 += 1;
}

/*
 * @tc.name: testTimer002
 * @tc.desc: timer unit test
 */
BENCHMARK_F(BenchmarkTimerTest, testTimer002)(benchmark::State& state)
{
    BENCHMARK_LOGD("TimerTest testTimer002 start.");
    while (state.KeepRunning()) {
        BENCHMARK_LOGD("TimerTest testTimer002 in.");
        g_data1 = 0;
        Utils::Timer timer("test_timer");
        uint32_t ret = timer.Setup();
        AssertEqual(Utils::TIMER_ERR_OK, ret, "Utils::TIMER_ERR_OK did not equal ret as expected.", state);
        timer.Register(TimeOutCallback1, 1, true);
        std::this_thread::sleep_for(std::chrono::milliseconds(15));
        timer.Shutdown();
        AssertEqual(1, g_data1, "1 did not equal g_data1 as expected.", state);
    }
    BENCHMARK_LOGD("TimerTest testTimer002 end.");
}

/*
 * @tc.name: testTimer003
 * @tc.desc: timer unit test
 */
BENCHMARK_F(BenchmarkTimerTest, testTimer003)(benchmark::State& state)
{
    BENCHMARK_LOGD("TimerTest testTimer003 start.");
    while (state.KeepRunning()) {
        g_data1 = 0;
        g_data2 = 0;
        Utils::Timer timer("test_timer");
        uint32_t ret = timer.Setup();
        AssertEqual(Utils::TIMER_ERR_OK, ret, "Utils::TIMER_ERR_OK did not equal ret as expected.", state);
        timer.Register(TimeOutCallback1, 1);
        timer.Register(TimeOutCallback2, 50);
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
        timer.Shutdown();
        AssertGreaterThanOrEqual(g_data1, 8, "g_data1 was not greater than or equal to 8 as expected.", state);
        AssertGreaterThanOrEqual(g_data2, 2, "g_data2 was not greater than or equal to 2 as expected.", state);
    }
    BENCHMARK_LOGD("TimerTest testTimer003 end.");
}

/*
 * @tc.name: testTimer004
 * @tc.desc: timer unit test
 */
BENCHMARK_F(BenchmarkTimerTest, testTimer004)(benchmark::State& state)
{
    BENCHMARK_LOGD("TimerTest testTimer004 start.");
    while (state.KeepRunning()) {
        g_data1 = 0;
        Utils::Timer timer("test_timer");
        uint32_t ret = timer.Setup();
        AssertEqual(Utils::TIMER_ERR_OK, ret, "Utils::TIMER_ERR_OK did not equal ret as expected.", state);
        timer.Register(TimeOutCallback1, 1);
        timer.Register(TimeOutCallback1, 2);
        std::this_thread::sleep_for(std::chrono::milliseconds(30));
        timer.Shutdown();
        AssertGreaterThanOrEqual(g_data1, 5, "g_data1 was not greater than or equal to 5 as expected.", state);
    }
    BENCHMARK_LOGD("TimerTest testTimer004 end.");
}

class A {
public:
    explicit A(int data) : data_(data), timer_("ATimer") {}
    ~A() = default;
    bool Init();
    bool StartTimer(int milliseconds, bool once);
    void StopTimer();
    int GetData() const {return data_;}
private:
    void TimeOutProc()
    {
        BENCHMARK_LOGD("TimerTest void TimeOutProc is called.");
        data_ -= 1;
    }
    int data_;
    Utils::Timer timer_;
};

bool A::Init()
{
    BENCHMARK_LOGD("TimerTest bool A::Init is called.");
    return timer_.Setup() == Utils::TIMER_ERR_OK;
}

bool A::StartTimer(int milliseconds, bool once)
{
    BENCHMARK_LOGD("TimerTest bool A::StartTimer is called.");
    uint32_t timerId = timer_.Register(std::bind(&A::TimeOutProc, this), milliseconds, once);
    return timerId != Utils::TIMER_ERR_DEAL_FAILED;
}

void A::StopTimer()
{
    BENCHMARK_LOGD("TimerTest void A::StopTimer is called.");
    timer_.Shutdown();
}

/*
 * @tc.name: testTimer006
 * @tc.desc: timer unit test
 */
BENCHMARK_F(BenchmarkTimerTest, testTimer006)(benchmark::State& state)
{
    BENCHMARK_LOGD("testTimer006 start.");
    while (state.KeepRunning()) {
        A a(10);
        AssertTrue(a.Init(), "a.Init() did not equal true as expected.", state);
        AssertTrue(a.StartTimer(1, true), "a.StartTimer(1, true) did not equal true as expected.", state);
        std::this_thread::sleep_for(std::chrono::milliseconds(20));
        a.StopTimer();
        AssertEqual(9, a.GetData(), "9 did not equal a.GetData() as expected.", state);
    }
    BENCHMARK_LOGD("testTimer006 end.");
}

/*
 * @tc.name: testTimer007
 * @tc.desc: abnormal case
 */
BENCHMARK_F(BenchmarkTimerTest, testTimer007)(benchmark::State& state)
{
    BENCHMARK_LOGD("testTimer007 start.");
    while (state.KeepRunning()) {
        g_data1 = 0;
        Utils::Timer timer("test_timer", -1);
        uint32_t ret = timer.Setup();
        AssertEqual(Utils::TIMER_ERR_OK, ret, "Utils::TIMER_ERR_OK did not equal ret as expected.", state);

        uint32_t timerId = 0;
        for (uint32_t i = 0; i < 10; i++) {
            timerId = timer.Register(TimeOutCallback1, 7, true);
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
        timer.Unregister(timerId);
        timer.Shutdown(false);
        BENCHMARK_LOGD("testTimer007 g_data1:%{public}d", (int)g_data1);
        AssertGreaterThanOrEqual(g_data1, 5, "g_data1 was not greater than or equal to 5 as expected.", state);
    }
    BENCHMARK_LOGD("testTimer007 end.");
}

/*
 * @tc.name: testTimer008
 * @tc.desc: timer sleep test for ivi
 */
BENCHMARK_F(BenchmarkTimerTest, testTimer008)(benchmark::State& state)
{
    BENCHMARK_LOGD("TimerTest testTimer008 start.");
    while (state.KeepRunning()) {
        g_data1 = 0;
        Utils::Timer timer("test_timer");
        uint32_t ret = timer.Setup();
        AssertEqual(Utils::TIMER_ERR_OK, ret, "Utils::TIMER_ERR_OK did not equal ret as expected.", state);
        timer.Register(TimeOutCallback1, 10);

        for (int i = 0; i < 11; i++) {
            int64_t pre = CurMs();
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            int64_t cur = CurMs();
            AssertGreaterThanOrEqual(cur - pre, 10,
                "(cur - pre) was not greater than or equal to 10 as expected.", state);
        }
        timer.Shutdown();
        AssertGreaterThanOrEqual(g_data1, 10, "g_data1 was not greater than or equal to 10 as expected.", state);
    }
    BENCHMARK_LOGD("TimerTest testTimer008 end.");
}

const int MAX_COUNT = 9;
const int TIMER_DELAY = 10;

/*
 * @tc.name: testTimer009
 * @tc.desc: recursive test
 */
void DoFunc(Utils::Timer &timer, int &count)
{
    BENCHMARK_LOGD("TimerTest void DoFunc is called.");
    (void)timer.Register(
        [&timer, &count]() {
            count += 1;
            if (count > MAX_COUNT) {
                return;
            }
            DoFunc(timer, count);
        },
        TIMER_DELAY,
        true);
    g_data1++;
}

void DoFunc2(Utils::Timer &timer, int &count)
{
    BENCHMARK_LOGD("TimerTest void DoFunc2 is called.");
    (void)timer.Register(
        [&timer, &count]() {
            count += 1;
            if (count > MAX_COUNT) {
                return;
            }
            DoFunc2(timer, count);
        },
        TIMER_DELAY,
        true);
    g_data1++;
}

BENCHMARK_F(BenchmarkTimerTest, testTimer009)(benchmark::State& state)
{
    BENCHMARK_LOGD("TimerTest testTimer009 start.");
    while (state.KeepRunning()) {
        g_data1 = 0;
        Utils::Timer timer("test_timer");
        uint32_t ret = timer.Setup();
        AssertEqual(Utils::TIMER_ERR_OK, ret, "Utils::TIMER_ERR_OK did not equal ret as expected.", state);

        int cnt = 0, cnt1 = 0;
        DoFunc(timer, cnt);
        DoFunc2(timer, cnt1);
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
        AssertGreaterThanOrEqual(g_data1, 5, "g_data1 was not greater than or equal to 5 as expected.", state);
        AssertGreaterThanOrEqual(14, g_data1, "g_data1 was not less than or equal to 14 as expected.", state);
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
        timer.Shutdown();
        AssertGreaterThanOrEqual(g_data1, 10, "g_data1 was not greater than or equal to 10 as expected.", state);
    }
    BENCHMARK_LOGD("TimerTest testTimer009 end.");
}

/*
 * @tc.name: testTimer010
 * @tc.desc: once timer register
 */
BENCHMARK_F(BenchmarkTimerTest, testTimer010)(benchmark::State& state)
{
    BENCHMARK_LOGD("TimerTest testTimer010 start.");
    while (state.KeepRunning()) {
        g_data1 = 0;
        Utils::Timer timer("test_timer");
        uint32_t ret = timer.Setup();
        AssertEqual(Utils::TIMER_ERR_OK, ret, "Utils::TIMER_ERR_OK did not equal ret as expected.", state);
        timer.Register(TimeOutCallback1, 10, true);
        timer.Register(TimeOutCallback1, 10);
        timer.Register(TimeOutCallback1, 10, true);
        timer.Register(TimeOutCallback1, 10);
        std::this_thread::sleep_for(std::chrono::milliseconds(52));
        timer.Shutdown();
        AssertGreaterThanOrEqual(g_data1, 8, "g_data1 was not greater than or equal to 8 as expected.", state);
    }
    BENCHMARK_LOGD("TimerTest testTimer010 end.");
}

/*
 * @tc.name: testTimer011
 * @tc.desc: once timer register
 */
BENCHMARK_F(BenchmarkTimerTest, testTimer011)(benchmark::State& state)
{
    BENCHMARK_LOGD("TimerTest testTimer011 start.");
    while (state.KeepRunning()) {
        g_data1 = 0;
        Utils::Timer timer("test_timer");
        uint32_t ret = timer.Setup();
        AssertEqual(Utils::TIMER_ERR_OK, ret, "Utils::TIMER_ERR_OK did not equal ret as expected.", state);
        timer.Register(TimeOutCallback1, 10);
        timer.Register(TimeOutCallback1, 10, true);
        timer.Register(TimeOutCallback1, 10);
        timer.Register(TimeOutCallback1, 10, true);
        std::this_thread::sleep_for(std::chrono::milliseconds(52));
        timer.Shutdown();
        AssertGreaterThanOrEqual(g_data1, 8, "g_data1 was not greater than or equal to 8 as expected.", state);
    }
    BENCHMARK_LOGD("TimerTest testTimer011 end.");
}

/*
 * @tc.name: testTimer012
 * @tc.desc: Test double setup.
 */
BENCHMARK_F(BenchmarkTimerTest, testTimer012)(benchmark::State& state)
{
    BENCHMARK_LOGD("TimerTest testTimer012 start.");
    while (state.KeepRunning()) {
        g_data1 = 0;
        Utils::Timer timer("test_timer");
        uint32_t ret = timer.Setup();
        AssertEqual(Utils::TIMER_ERR_OK, ret, "Utils::TIMER_ERR_OK did not equal ret as expected.", state);
        ret = timer.Setup();
        AssertEqual(Utils::TIMER_ERR_INVALID_VALUE, ret,
            "Utils::TIMER_ERR_INVALID_VALUE did not equal ret as expected.", state);

        timer.Shutdown();
    }
    BENCHMARK_LOGD("TimerTest testTimer012 end.");
}

/*
 * @tc.name: testTimer013
 * @tc.desc: Test uncommon operations.
 */
BENCHMARK_F(BenchmarkTimerTest, testTimer013)(benchmark::State& state)
{
    BENCHMARK_LOGD("TimerTest testTimer013 start.");
    while (state.KeepRunning()) {
        g_data1 = 0;
        Utils::Timer timer("test_timer", -1);
        uint32_t ret = timer.Setup();
        BENCHMARK_LOGD("TimerTest testTimer013 Setup.");
        AssertEqual(Utils::TIMER_ERR_OK, ret, "Utils::TIMER_ERR_OK did not equal ret as expected.", state);
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
        timer.Shutdown(false);

        BENCHMARK_LOGD("TimerTest testTimer013 Shutdown.");

        Utils::Timer timer1("test_timer_1");
        ret = timer1.Setup();
        BENCHMARK_LOGD("TimerTest testTimer013 Setup1.");
        AssertEqual(Utils::TIMER_ERR_OK, ret, "Utils::TIMER_ERR_OK did not equal ret as expected.", state);
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
        timer1.Shutdown();
        BENCHMARK_LOGD("TimerTest testTimer013 Shutdown false.");
    }
    BENCHMARK_LOGD("TimerTest testTimer013 end.");
}
}  // namespace
}  // namespace OHOS
// Run the benchmark
BENCHMARK_MAIN();