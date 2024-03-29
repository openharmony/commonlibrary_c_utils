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
#include "datetime_ex.h"
#include <unistd.h>
#include <iostream>
#include "benchmark_log.h"
#include "benchmark_assert.h"
using namespace std;

namespace OHOS {
namespace {

class BenchmarkDateTimeTest : public benchmark::Fixture {
public:
    void SetUp(const ::benchmark::State& state) override
    {
    }

    void TearDown(const ::benchmark::State& state) override
    {
    }

    BenchmarkDateTimeTest()
    {
        Iterations(iterations);
        Repetitions(repetitions);
        ReportAggregatesOnly();
    }

    ~BenchmarkDateTimeTest() override = default;

protected:
    const int32_t repetitions = 3;
    const int32_t iterations = 1000;
};

const int64_t SLEEP_DURATION_MICROSECONDS = 100;

/*
 * @tc.name: testTimecover001
 * @tc.desc: convert all letters of str to uppercase
 */
BENCHMARK_F(BenchmarkDateTimeTest, testTimecover001)(benchmark::State& state)
{
    BENCHMARK_LOGD("DateTimeTest testTimecover001 start.");
    while (state.KeepRunning()) {
        const int64_t second = 20;
        const int64_t nanosecondsInASecond = 20000000000;
        AssertEqual(SecToNanosec(second), nanosecondsInASecond,
            "SecToNanosec(second) did not equal nanosecondsInASecond as expected.", state);
        const int64_t millsec = 10;
        const int64_t nanosecondsInAMillisecond = 10000000;
        AssertEqual(MillisecToNanosec(millsec), nanosecondsInAMillisecond,
            "MillisecToNanosec(millsec) did not equal nanosecondsInAMillisecond as expected.", state);
        const int64_t microsec = 5;
        const int64_t nanosecondsInAMicrosecond = 5000;
        AssertEqual(MicrosecToNanosec(microsec), nanosecondsInAMicrosecond,
            "MicrosecToNanosec(microsec) did not equal nanosecondsInAMicrosecond as expected.", state);

        const int64_t nanoces = 1000000000;
        const int64_t secondsInANanosecond = 1;
        const int64_t millisecondsInANanosecond = 1000;
        const int64_t microsecondsInANanosecond = 1000000;
        AssertEqual(NanosecToSec(nanoces), secondsInANanosecond,
            "NanosecToSec(nanoces) did not equal secondsInANanosecond as expected.", state);
        AssertEqual(NanosecToMillisec(nanoces), millisecondsInANanosecond,
            "NanosecToMillisec(nanoces) did not equal millisecondsInANanosecond as expected.", state);
        AssertEqual(NanosecToMicrosec(nanoces), microsecondsInANanosecond,
            "NanosecToMicrosec(nanoces) did not equal microsecondsInANanosecond as expected.", state);
    }
    BENCHMARK_LOGD("DateTimeTest testTimecover001 end.");
}

/*
 * @tc.name: testTime001
 * @tc.desc: datetime unit
 */
BENCHMARK_F(BenchmarkDateTimeTest, testTime001)(benchmark::State& state)
{
    BENCHMARK_LOGD("DateTimeTest testTime001 start.");
    while (state.KeepRunning()) {
        int64_t second = GetSecondsSince1970ToNow();

        struct tm curTime = {0};
        bool ret = GetSystemCurrentTime(&curTime);
        AssertEqual(ret, true, "ret did not equal true as expected.", state);
        ret = GetSystemCurrentTime(nullptr);
        AssertEqual(ret, false, "ret did not equal false as expected.", state);

        int64_t second2 = GetSecondsSince1970ToPointTime(curTime);
        AssertEqual(second, second2, "second did not equal second2 as expected.", state);
        struct tm info;
        info.tm_year = INT32_MIN;
        info.tm_mon = 0;
        info.tm_mday = 0;
        info.tm_hour = 0;
        info.tm_min = 0;
        info.tm_sec = 0;
        info.tm_isdst = 0;
        const int64_t invalidReturnValue = -1;
        second2 = GetSecondsSince1970ToPointTime(info);
        AssertEqual(invalidReturnValue, second2, "invalidReturnValue did not equal second2 as expected.", state);
        int64_t ret2 = GetSecondsBetween(curTime, info);
        AssertTrue((ret2 = invalidReturnValue), "ret2 = invalidReturnValue did not equal true as expected.", state);
    }
    BENCHMARK_LOGD("DateTimeTest testTime001 end.");
}

/*
 * @tc.name: testTime002
 * @tc.desc: datetime unit
 */
BENCHMARK_F(BenchmarkDateTimeTest, testTime002)(benchmark::State& state)
{
    BENCHMARK_LOGD("DateTimeTest testTime002 start.");
    while (state.KeepRunning()) {
        int64_t days = GetDaysSince1970ToNow();
        int64_t seconds = GetSecondsSince1970ToNow();
        const int64_t secondsInAnHour = 3600;
        const int64_t hoursInADay = 24;
        int64_t resultdays = seconds / (secondsInAnHour * hoursInADay);
        AssertEqual(days, resultdays, "days did not equal resultdays as expected.", state);
    }
    BENCHMARK_LOGD("DateTimeTest testTime002 end.");
}

/*
 * @tc.name: testTime003
 * @tc.desc: datetime unit
 */
BENCHMARK_F(BenchmarkDateTimeTest, testTime003)(benchmark::State& state)
{
    BENCHMARK_LOGD("DateTimeTest testTime003 start.");
    while (state.KeepRunning()) {
        struct tm curTime = { 0 };
        bool ret = GetSystemCurrentTime(&curTime);
        AssertEqual(ret, true, "ret did not equal true as expected.", state);

        struct tm curTime2 = { 0 };
        ret = GetSystemCurrentTime(&curTime2);
        AssertEqual(ret, true, "ret did not equal true as expected.", state);
        int64_t betweensec = GetSecondsBetween(curTime, curTime2);
        AssertGreaterThanOrEqual(betweensec, 0, "betweensec >= 0 did not equal true as expected.", state);
    }
    BENCHMARK_LOGD("DateTimeTest testTime003 end.");
}

/*
 * @tc.name: testTime004
 * @tc.desc: datetime unit
 */
BENCHMARK_F(BenchmarkDateTimeTest, testTime004)(benchmark::State& state)
{
    BENCHMARK_LOGD("DateTimeTest testTime004 start.");
    while (state.KeepRunning()) {
        int timezone = 0;
        bool ret = GetLocalTimeZone(timezone);
        AssertEqual(ret, true, "ret did not equal true as expected.", state);
    }
    BENCHMARK_LOGD("DateTimeTest testTime004 end.");
}

/*
 * @tc.name: testGetTickCount001
 * @tc.desc: datetime unit
 */
BENCHMARK_F(BenchmarkDateTimeTest, testGetTickCount001)(benchmark::State& state)
{
    BENCHMARK_LOGD("DateTimeTest testGetTickCount001 start.");
    while (state.KeepRunning()) {
        int64_t begin = GetTickCount();
        usleep(SLEEP_DURATION_MICROSECONDS);
        int64_t end = GetTickCount();

        AssertGreaterThanOrEqual(end - begin, 0, "end - begin >= 0 did not equal true as expected.", state);
    }
    BENCHMARK_LOGD("DateTimeTest testGetTickCount001 end.");
}

/*
 * @tc.name: testGetMicroTickCount001
 * @tc.desc: datetime unit
 */
BENCHMARK_F(BenchmarkDateTimeTest, testGetMicroTickCount001)(benchmark::State& state)
{
    BENCHMARK_LOGD("DateTimeTest testGetMicroTickCount001 start.");
    while (state.KeepRunning()) {
        int64_t begin = GetMicroTickCount();
        usleep(SLEEP_DURATION_MICROSECONDS);
        int64_t end = GetMicroTickCount();

        AssertGreaterThanOrEqual(end - begin, 0, "end - begin >= 0 did not equal true as expected.", state);
    }
    BENCHMARK_LOGD("DateTimeTest testGetMicroTickCount001 end.");
}
}  // namespace
}  // namespace OHOS
// Run the benchmark
BENCHMARK_MAIN();