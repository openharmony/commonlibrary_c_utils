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
#include "thread_ex.h"
#include <iostream>
#include <cstdio>
#include <sys/prctl.h>
#include <unistd.h>
#include <sys/resource.h>
#include "benchmark_log.h"
#include "benchmark_assert.h"
using namespace std;

namespace OHOS {
namespace {

class BenchmarkThreadTest : public benchmark::Fixture {
public:
    void SetUp(const ::benchmark::State& state) override
    {
    }

    void TearDown(const ::benchmark::State& state) override
    {
    }

    BenchmarkThreadTest()
    {
        Iterations(iterations);
        Repetitions(repetitions);
        ReportAggregatesOnly();
    }

    ~BenchmarkThreadTest() override = default;

protected:
    const int32_t repetitions = 3;
    const int32_t iterations = 100;
};

static int g_times = 0;
constexpr int DEFAULT_PRIO = 0;
const std::string& DEFAULT_THREAD_NAME = "default";
const int INITIAL_TIMES = 0;
const int INITIAL_TEST_VALUE = 0;
const int THREAD_STACK_SIZE = 1024;
const int INVALID_THREAD_ID = -1;
const int SLEEP_FOR_ONE_SECOND = 1;

using ThreadRunFunc = bool (*)(int& data);

bool TestRun01(int& data)
{
    BENCHMARK_LOGD("ThreadTest bool TestRun01 is called.");
    ++data;
    return false;
}

bool TestRun02(int& data)
{
    BENCHMARK_LOGD("ThreadTest bool TestRun02 is called.");
    ++data;
    return true;
}

bool TestRun03(int& data)
{
    BENCHMARK_LOGD("ThreadTest bool TestRun03 is called.");
    static const int TRY_TIMES = 10;
    if (g_times <= TRY_TIMES) {
        ++data;
        return true;
    }

    return false;
}

class TestThread : public OHOS::Thread {
public:
    TestThread(const int data, const bool readyToWork, ThreadRunFunc runFunc)
        : data_(data),
        priority_(DEFAULT_PRIO),
        name_(DEFAULT_THREAD_NAME),
        readyToWork_(readyToWork),
        runFunc_(runFunc)
        {};

    TestThread() = delete;
    ~TestThread() {}

    bool ReadyToWork() override;

    int data_;
    int priority_;
    std::string name_;

protected:
    bool Run() override;

private:
    bool readyToWork_;
    ThreadRunFunc runFunc_;
};

bool TestThread::ReadyToWork()
{
    BENCHMARK_LOGD("ThreadTest bool TestThread::ReadyToWork is called.");
    return readyToWork_;
}

bool TestThread::Run()
{
    BENCHMARK_LOGD("ThreadTest bool TestThread::Run is called.");
    priority_ = getpriority(PRIO_PROCESS, 0);
    char threadName[MAX_THREAD_NAME_LEN + 1] = {0};
    prctl(PR_GET_NAME, threadName, 0, 0);
    name_ = threadName;

    if (runFunc_ != nullptr) {
        return (*runFunc_)(data_);
    }

    return false;
}

/*
 * @tc.name: testThread001
 * @tc.desc: ThreadTest
 */
BENCHMARK_F(BenchmarkThreadTest, testThread001)(benchmark::State& state)
{
    BENCHMARK_LOGD("ThreadTest testThread001 start.");
    const int expectedDataValue = 0;
    while (state.KeepRunning()) {
        g_times = 0;
        std::unique_ptr<TestThread> test = std::make_unique<TestThread>(INITIAL_TEST_VALUE, false, TestRun01);
        ThreadStatus status = test->Start("test_thread_01", THREAD_PROI_LOW, THREAD_STACK_SIZE);
        AssertEqual((status == ThreadStatus::OK), true,
            "(status == ThreadStatus::OK) did not equal true as expected.", state);

        pthread_t thread = test->GetThread();

        // pthread_equal return non-zero if equal
        AssertEqual((pthread_equal(thread, INVALID_THREAD_ID) != 0), (test->IsRunning() ? false : true),
            "(pthread_equal(thread, INVALID_THREAD_ID) != 0) did not equal \
            (test->IsRunning() ? false : true) as expected.", state);

        // ReadyToWork return false, RUN will not be called!
        AssertEqual(test->priority_, DEFAULT_PRIO, "test->priority_ did not equal DEFAULT_PRIO as expected.", state);
        AssertEqual(test->name_, DEFAULT_THREAD_NAME,
            "test->name_ did not equal DEFAULT_THREAD_NAME as expected.", state);

        // get stacksize of threa, may be different because of system memory align
        AssertEqual(test->data_, expectedDataValue,
            "test->data_ did not equal EXPECTED_DATA_VALUE as expected.", state);
        AssertEqual(g_times, INITIAL_TIMES, "g_times did not equal INITIAL_TIMES as expected.", state);
        test->NotifyExitSync();
        sleep(SLEEP_FOR_ONE_SECOND);
        AssertEqual((pthread_equal(test->GetThread(), INVALID_THREAD_ID) != 0), (test->IsRunning() ? false : true),
            "(pthread_equal(test->GetThread(), INVALID_THREAD_ID) != 0) did not equal \
            (test->IsRunning() ? false : true) as expected.", state);
    }
    BENCHMARK_LOGD("ThreadTest testThread001 end.");
}

/*
 * @tc.name: testThread002
 * @tc.desc: ThreadTest
 */
BENCHMARK_F(BenchmarkThreadTest, testThread002)(benchmark::State& state)
{
    BENCHMARK_LOGD("ThreadTest testThread002 start.");
    const int expectedDataValue = 1;
    while (state.KeepRunning()) {
        g_times = 0;
        std::unique_ptr<TestThread> test = std::make_unique<TestThread>(INITIAL_TEST_VALUE, true, TestRun01);
        ThreadStatus status = test->Start("test_thread_02", THREAD_PROI_LOW, THREAD_STACK_SIZE);

        AssertEqual((status == ThreadStatus::OK), true,
            "status == ThreadStatus::OK did not equal true as expected.", state);

        sleep(SLEEP_FOR_ONE_SECOND); // let the new thread has chance to run

        // pthread_equal return non-zero if equal, RUN return false,may exit already
        AssertEqual((pthread_equal(test->GetThread(), INVALID_THREAD_ID) != 0), (test->IsRunning() ? false : true),
            "(pthread_equal(test->GetThread(), INVALID_THREAD_ID) != 0) did not equal \
            (test->IsRunning() ? false : true) as expected.", state);

        // ReadyToWork return true, RUN will be called!
        AssertEqual(test->priority_, THREAD_PROI_LOW,
            "test->priority_ did not equal THREAD_PROI_LOW as expected.", state);
        AssertEqual(test->name_, "test_thread_02",
            "test->name_ did not equal \"test_thread_02\" as expected.", state);
        AssertEqual(test->data_, expectedDataValue, "test->data_ did not equal expectedDataValue as expected.", state);
        AssertEqual(g_times, INITIAL_TIMES, "g_times did not equal INITIAL_TIMES as expected.", state);

        test->NotifyExitSync();
        sleep(SLEEP_FOR_ONE_SECOND);
        AssertEqual((pthread_equal(test->GetThread(), INVALID_THREAD_ID) != 0), (test->IsRunning() ? false : true),
            "pthread_equal(test->GetThread(), INVALID_THREAD_ID) != 0 did not equal \
            (test->IsRunning() ? false : true) as expected.", state);
    }
    BENCHMARK_LOGD("ThreadTest testThread002 end.");
}

static void ThreadTestProcedure(std::unique_ptr<TestThread>& test, const int expectedDataValue, benchmark::State& state)
{
    pthread_t thread = test->GetThread();

    // pthread_equal return non-zero if equal
    AssertEqual((pthread_equal(thread, INVALID_THREAD_ID) != 0), (test->IsRunning() ? false : true),
        "pthread_equal(thread, INVALID_THREAD_ID) != 0 did not equal \
        (test->IsRunning() ? false : true) as expected.", state);

    // ReadyToWork return false, RUN will not be called!
    AssertEqual(test->priority_, DEFAULT_PRIO,
        "test->priority_ did not equal DEFAULT_PRIO as expected.", state);
    AssertEqual(test->name_, DEFAULT_THREAD_NAME,
        "test->name_ did not equal DEFAULT_THREAD_NAME as expected.", state);
    AssertEqual(test->data_, expectedDataValue, "test->data_ did not equal expectedDataValue as expected.", state);
    AssertEqual(g_times, INITIAL_TIMES, "g_times did not equal INITIAL_TIMES as expected.", state);

    test->NotifyExitSync();
    sleep(SLEEP_FOR_ONE_SECOND);
    AssertEqual((pthread_equal(test->GetThread(), INVALID_THREAD_ID) != 0), (test->IsRunning() ? false : true),
        "pthread_equal(test->GetThread(), INVALID_THREAD_ID) != 0 did not equal \
        (test->IsRunning() ? false : true) as expected.", state);
}

/*
 * @tc.name: testThread003
 * @tc.desc: ThreadTest
 */
BENCHMARK_F(BenchmarkThreadTest, testThread003)(benchmark::State& state)
{
    BENCHMARK_LOGD("ThreadTest testThread003 start.");
    const int expectedDataValue = 0;
    while (state.KeepRunning()) {
        g_times = 0;
        std::unique_ptr<TestThread> test = std::make_unique<TestThread>(INITIAL_TEST_VALUE, false, TestRun02);
        ThreadStatus status = test->Start("test_thread_03", THREAD_PROI_LOW, THREAD_STACK_SIZE);
        AssertEqual((status == ThreadStatus::OK), true,
            "status == ThreadStatus::OK did not equal true as expected.", state);

        ThreadTestProcedure(test, expectedDataValue, state);
    }
    BENCHMARK_LOGD("ThreadTest testThread003 end.");
}

/*
 * @tc.name: testThread004
 * @tc.desc: ThreadTest
 */
BENCHMARK_F(BenchmarkThreadTest, testThread004)(benchmark::State& state)
{
    BENCHMARK_LOGD("ThreadTest testThread004 start.");
    const int comparisonValue = 1;
    while (state.KeepRunning()) {
        g_times = 0;
        std::unique_ptr<TestThread> test = std::make_unique<TestThread>(INITIAL_TEST_VALUE, true, TestRun02);
        ThreadStatus status = test->Start("test_thread_04", THREAD_PROI_LOW, THREAD_STACK_SIZE);

        AssertEqual((status == ThreadStatus::OK), true,
            "status == ThreadStatus::OK did not equal true as expected.", state);

        sleep(SLEEP_FOR_ONE_SECOND); // let the new thread has chance to run

        // pthread_equal return non-zero if equal, RUN return false,may exit already
        AssertEqual((pthread_equal(test->GetThread(), INVALID_THREAD_ID) != 0), (test->IsRunning() ? false : true),
            "pthread_equal(test->GetThread(), INVALID_THREAD_ID) != 0 did not equal \
            (test->IsRunning() ? false : true) as expected.", state);
        // ReadyToWork return true, RUN will be called!
        AssertEqual(test->priority_, THREAD_PROI_LOW,
            "test->priority_ did not equal THREAD_PROI_LOW as expected.", state);
        AssertEqual(test->name_, "test_thread_04",
            "test->name_ did not equal \"test_thread_04\" as expected.", state);
        AssertGreaterThan(test->data_, comparisonValue,
            "test->data_ was not greater than comparisonValue as expected.", state);
        AssertEqual(g_times, INITIAL_TIMES, "g_times did not equal INITIAL_TIMES as expected.", state);

        test->NotifyExitSync();
        sleep(SLEEP_FOR_ONE_SECOND);
        AssertEqual((pthread_equal(test->GetThread(), INVALID_THREAD_ID) != 0), (test->IsRunning() ? false : true),
            "pthread_equal(test->GetThread(), INVALID_THREAD_ID) != 0 did not equal \
            (test->IsRunning() ? false : true) as expected.", state);
    }
    BENCHMARK_LOGD("ThreadTest testThread004 end.");
}

/*
 * @tc.name: testThread005
 * @tc.desc: ThreadTest
 */
BENCHMARK_F(BenchmarkThreadTest, testThread005)(benchmark::State& state)
{
    BENCHMARK_LOGD("ThreadTest testThread005 start.");
    const int expectedDataValue = 0;
    while (state.KeepRunning()) {
        g_times = 0;
        std::unique_ptr<TestThread> test = std::make_unique<TestThread>(INITIAL_TEST_VALUE, false, TestRun03);
        ThreadStatus status = test->Start("test_thread_05", THREAD_PROI_LOW, THREAD_STACK_SIZE);
        AssertEqual((status == ThreadStatus::OK), true,
            "status == ThreadStatus::OK did not equal true as expected.", state);

        ThreadTestProcedure(test, expectedDataValue, state);
    }
    BENCHMARK_LOGD("ThreadTest testThread005 end.");
}

/*
 * @tc.name: testThread006
 * @tc.desc: ThreadTest
 */
BENCHMARK_F(BenchmarkThreadTest, testThread006)(benchmark::State& state)
{
    BENCHMARK_LOGD("ThreadTest testThread006 start.");
    const int comparisonValue = 10;
    while (state.KeepRunning()) {
        g_times = 0;
        std::unique_ptr<TestThread> test = std::make_unique<TestThread>(INITIAL_TEST_VALUE, true, TestRun03);
        ThreadStatus status = test->Start("test_thread_06", THREAD_PROI_LOW, THREAD_STACK_SIZE);
        AssertEqual((status == ThreadStatus::OK), true,
            "status == ThreadStatus::OK did not equal true as expected.", state);

        sleep(SLEEP_FOR_ONE_SECOND); // let the new thread has chance to run

        // pthread_equal return non-zero if equal, RUN return false,may exit already
        AssertEqual((pthread_equal(test->GetThread(), INVALID_THREAD_ID) != 0), (test->IsRunning() ? false : true),
            "pthread_equal(test->GetThread(), INVALID_THREAD_ID) != 0 did not equal \
            (test->IsRunning() ? false : true) as expected.", state);
        // ReadyToWork return true, RUN will be called!
        AssertEqual(test->priority_, THREAD_PROI_LOW,
            "test->priority_ did not equal THREAD_PROI_LOW as expected.", state);
        AssertEqual(test->name_, "test_thread_06",
            "test->name_ did not equal \"test_thread_06\" as expected.", state);
        AssertGreaterThan(test->data_, comparisonValue,
            "test->data_ was not greater than comparisonValue as expected.", state);
        AssertEqual(g_times, INITIAL_TIMES, "g_times did not equal INITIAL_TIMES as expected.", state);

        g_times = 100;
        AssertGreaterThan(test->data_, comparisonValue,
            "test->data_ was not greater than comparisonValue as expected.", state);

        sleep(SLEEP_FOR_ONE_SECOND); // let the new thread has chance to run

        // g_times > 10, TestRun03 return false, thread exit
        AssertEqual((pthread_equal(test->GetThread(), INVALID_THREAD_ID) != 0), (test->IsRunning() ? false : true),
            "pthread_equal(test->GetThread(), INVALID_THREAD_ID) != 0 did not equal \
            (test->IsRunning() ? false : true) as expected.", state);
    }
    BENCHMARK_LOGD("ThreadTest testThread006 end.");
}

/*
 * @tc.name: testThread007
 * @tc.desc: ThreadTest
 */
BENCHMARK_F(BenchmarkThreadTest, testThread007)(benchmark::State& state)
{
    BENCHMARK_LOGD("ThreadTest testThread007 start.");
    while (state.KeepRunning()) {
        g_times = 0;
        std::unique_ptr<TestThread> test = std::make_unique<TestThread>(INITIAL_TEST_VALUE, true, TestRun03);
        ThreadStatus status = test->Start("", THREAD_PROI_LOW, 0);
        AssertEqual((status == ThreadStatus::OK), true,
            "status == ThreadStatus::OK did not equal true as expected.", state);

        if (test->IsRunning()) {
            status = test->Start("", THREAD_PROI_NORMAL, THREAD_STACK_SIZE);
            AssertEqual(status == ThreadStatus::INVALID_OPERATION, true,
                "status == ThreadStatus::INVALID_OPERATION did not equal true as expected.", state);

            test->NotifyExitSync();
            AssertEqual((pthread_equal(test->GetThread(), INVALID_THREAD_ID) != 0), (test->IsRunning() ? false : true),
                "pthread_equal(test->GetThread(), INVALID_THREAD_ID) != 0 did not equal \
                (test->IsRunning() ? false : true) as expected.", state);
        }

        sleep(SLEEP_FOR_ONE_SECOND); // let the new thread has chance to run

        AssertEqual((pthread_equal(test->GetThread(), INVALID_THREAD_ID) != 0), (test->IsRunning() ? false : true),
            "pthread_equal(test->GetThread(), INVALID_THREAD_ID) != 0 did not equal \
            (test->IsRunning() ? false : true) as expected.", state);
    }
    BENCHMARK_LOGD("ThreadTest testThread007 end.");
}

template <typename T>
void ThreadTestStart(std::unique_ptr<T>& test, benchmark::State& state)
{
    ThreadStatus status = test->Start("", THREAD_PROI_NORMAL, THREAD_STACK_SIZE);
    AssertEqual((status == ThreadStatus::OK), true,
        "status == ThreadStatus::OK did not equal true as expected.", state);

    sleep(SLEEP_FOR_ONE_SECOND);
    test->NotifyExitAsync();

    sleep(SLEEP_FOR_ONE_SECOND); // let the new thread has chance to run
    AssertEqual((pthread_equal(test->GetThread(), INVALID_THREAD_ID) != 0), (test->IsRunning() ? false : true),
        "pthread_equal(test->GetThread(), INVALID_THREAD_ID) != 0 did not equal \
        (test->IsRunning() ? false : true) as expected.", state);
}

/*
 * @tc.name: testThread008
 * @tc.desc: ThreadTest
 */
BENCHMARK_F(BenchmarkThreadTest, testThread008)(benchmark::State& state)
{
    BENCHMARK_LOGD("ThreadTest testThread008 start.");
    while (state.KeepRunning()) {
        g_times = 0;
        std::unique_ptr<TestThread> test = std::make_unique<TestThread>(INITIAL_TEST_VALUE, true, TestRun03);

        bool res = test->Thread::ReadyToWork();
        AssertEqual(res, true, "res did not equal true as expected.", state);

        ThreadTestStart<TestThread>(test, state);
    }
    BENCHMARK_LOGD("ThreadTest testThread008 end.");
}

class TestThread2 : public OHOS::Thread {
public:
    TestThread2(const int data, ThreadRunFunc runFunc)
        : data_(data), priority_(DEFAULT_PRIO), name_(DEFAULT_THREAD_NAME), runFunc_(runFunc)
        {};

    TestThread2() = delete;
    ~TestThread2() {}

    int data_;
    int priority_;
    std::string name_;
protected:
    bool Run() override;

private:
    ThreadRunFunc runFunc_;
};

bool TestThread2::Run()
{
    BENCHMARK_LOGD("ThreadTest bool TestThread2::Run is called.");
    priority_ = getpriority(PRIO_PROCESS, 0);
    char threadName[MAX_THREAD_NAME_LEN + 1] = {0};
    prctl(PR_GET_NAME, threadName, 0, 0);
    name_ = threadName;

    if (runFunc_ != nullptr) {
        return (*runFunc_)(data_);
    }

    return false;
}

/*
 * @tc.name: testThread009
 * @tc.desc: ThreadTest
 */
BENCHMARK_F(BenchmarkThreadTest, testThread009)(benchmark::State& state)
{
    BENCHMARK_LOGD("ThreadTest testThread009 start.");
    while (state.KeepRunning()) {
        g_times = 0;
        std::unique_ptr<TestThread2> test = std::make_unique<TestThread2>(0, TestRun03);

        bool res = test->ReadyToWork();
        AssertEqual(res, true, "res did not equal true as expected.", state);

        ThreadTestStart<TestThread2>(test, state);
    }
    BENCHMARK_LOGD("ThreadTest testThread009 end.");
}
}  // namespace
}  // namespace OHOS
// Run the benchmark
BENCHMARK_MAIN();