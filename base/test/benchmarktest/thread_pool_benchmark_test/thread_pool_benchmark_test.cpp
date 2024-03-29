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
#include <chrono>
#include <cstdio>
#include <sys/prctl.h>
#include "thread_pool.h"
#include "benchmark_log.h"
#include "benchmark_assert.h"
#include <unistd.h>

namespace OHOS {
namespace {

int g_times = 0;
bool g_ready = false;
std::mutex g_mutex;
std::condition_variable g_cv;
const int SLEEP_FOR_ONE_SECOND = 1;
static bool g_flagTestFuncGetName = true;

class BenchmarkThreadPoolTest : public benchmark::Fixture {
public:
    void SetUp(const ::benchmark::State& state) override
    {
    }

    void TearDown(const ::benchmark::State& state) override
    {
    }

    BenchmarkThreadPoolTest()
    {
        Iterations(iterations);
        Repetitions(repetitions);
        ReportAggregatesOnly();
    }

    ~BenchmarkThreadPoolTest() override = default;

protected:
    const int32_t repetitions = 3;
    const int32_t iterations = 100;
};

BENCHMARK_F(BenchmarkThreadPoolTest, test_01)(benchmark::State& state)
{
    BENCHMARK_LOGD("ThreadPoolTest test_01 start.");
    const int maxTaskNum = 0;
    const int threadsNum = 0;
    const int curTaskNum = 0;
    while (state.KeepRunning()) {
        ThreadPool pool;
        AssertEqual(pool.GetName(), "", "pool.GetName() did not equal \"\" as expected.", state);
        AssertEqual((int)pool.GetMaxTaskNum(), maxTaskNum,
            "(int)pool.GetMaxTaskNum() did not equal maxTaskNum as expected.", state);
        AssertEqual((int)pool.GetThreadsNum(), threadsNum,
            "(int)pool.GetThreadsNum() did not equal threadsNum as expected.", state);
        AssertEqual((int)pool.GetCurTaskNum(), curTaskNum,
            "(int)pool.GetCurTaskNum() did not equal curTaskNum as expected.", state);
    }
    BENCHMARK_LOGD("ThreadPoolTest test_01 end.");
}

BENCHMARK_F(BenchmarkThreadPoolTest, test_02)(benchmark::State& state)
{
    BENCHMARK_LOGD("ThreadPoolTest test_02 start.");
    const int maxTaskNum = 0;
    const int threadsNum = 0;
    const int curTaskNum = 0;
    while (state.KeepRunning()) {
        ThreadPool pool("test_02_pool");
        AssertEqual(pool.GetName(), "test_02_pool",
            "pool.GetName() did not equal \"test_02_pool\" as expected.", state);
        AssertEqual((int)pool.GetMaxTaskNum(), maxTaskNum,
            "(int)pool.GetMaxTaskNum() did not equal maxTaskNum as expected.", state);
        AssertEqual((int)pool.GetThreadsNum(), threadsNum,
            "(int)pool.GetThreadsNum() did not equal threadsNum as expected.", state);
        AssertEqual((int)pool.GetCurTaskNum(), curTaskNum,
            "(int)pool.GetCurTaskNum() did not equal curTaskNum as expected.", state);
    }
    BENCHMARK_LOGD("ThreadPoolTest test_02 end.");
}

BENCHMARK_F(BenchmarkThreadPoolTest, test_03)(benchmark::State& state)
{
    BENCHMARK_LOGD("ThreadPoolTest test_03 start.");
    const int maxTaskNum = 10;
    const int threadsNum = 0;
    const int curTaskNum = 0;
    while (state.KeepRunning()) {
        ThreadPool pool("test_02_pool");
        pool.SetMaxTaskNum(maxTaskNum);
        AssertEqual(pool.GetName(),
            "test_02_pool", "pool.GetName() did not equal \"test_02_pool\" as expected.", state);
        AssertEqual((int)pool.GetMaxTaskNum(), maxTaskNum,
            "(int)pool.GetMaxTaskNum() did not equal maxTaskNum as expected.", state);
        AssertEqual((int)pool.GetThreadsNum(), threadsNum,
            "(int)pool.GetThreadsNum() did not equal threadsNum as expected.", state);
        AssertEqual((int)pool.GetCurTaskNum(), curTaskNum,
            "(int)pool.GetCurTaskNum() did not equal curTaskNum as expected.", state);
    }
    BENCHMARK_LOGD("ThreadPoolTest test_03 end.");
}

BENCHMARK_F(BenchmarkThreadPoolTest, test_04)(benchmark::State& state)
{
    BENCHMARK_LOGD("ThreadPoolTest test_04 start.");
    const int startThreads = 4;
    const int maxTaskNum = 0;
    const int threadsNum = 4;
    const int curTaskNum = 0;
    while (state.KeepRunning()) {
        ThreadPool pool;
        pool.Start(startThreads);
        AssertEqual(pool.GetName(), "", "pool.GetName() did not equal \"\" as expected.", state);
        AssertEqual((int)pool.GetMaxTaskNum(), maxTaskNum,
            "(int)pool.GetMaxTaskNum() did not equal maxTaskNum as expected.", state);
        AssertEqual((int)pool.GetThreadsNum(), threadsNum,
            "(int)pool.GetThreadsNum() did not equal threadsNum as expected.", state);
        AssertEqual((int)pool.GetCurTaskNum(), curTaskNum,
            "(int)pool.GetCurTaskNum() did not equal curTaskNum as expected.", state);

        // add no task, g_times has no change
        AssertEqual(g_times, 0, "g_times did not equal 0 as expected.", state);
        pool.Stop();
    }
    BENCHMARK_LOGD("ThreadPoolTest test_04 end.");
}

void TestFuncAddOneTime(int& i)
{
    std::lock_guard<std::mutex> lock(g_mutex);
    ++g_times;
    BENCHMARK_LOGD("ThreadPoolTest TestFuncAddOneTime g_times:%{public}d", g_times);
}

void TestFuncSubOneTime(int& i)
{
    std::lock_guard<std::mutex> lock(g_mutex);
    --g_times;
    BENCHMARK_LOGD("ThreadPoolTest TestFuncSubOneTime g_times:%{public}d", g_times);
}

static void AddPoolTaskByForLoop(ThreadPool& pool, const int loopIterations, void(*p)(int&))
{
    for (int i = 0; i < loopIterations; ++i) {
        auto task = std::bind(p, i);
        pool.AddTask(task);
    }
}

// simple task, total task num less than the MaxTaskNum
BENCHMARK_F(BenchmarkThreadPoolTest, test_05)(benchmark::State& state)
{
    BENCHMARK_LOGD("ThreadPoolTest test_05 start.");
    const int startThreads = 5;
    const int threadsNum = 5;
    const int curTaskNum = 0;
    const int loopIterations1 = 3;
    const int loopIterations2 = 2;
    const int expectedTimesValue = 1;
    while (state.KeepRunning()) {
        BENCHMARK_LOGD("ThreadPoolTest test_05 in");
        ThreadPool pool;
        pool.Start(startThreads);
        AssertEqual(pool.GetName(), "", "pool.GetName() did not equal \"\" as expected.", state);
        AssertEqual((int)pool.GetThreadsNum(), threadsNum,
            "(int)pool.GetThreadsNum() did not equal threadsNum as expected.", state);
        AssertEqual((int)pool.GetCurTaskNum(), curTaskNum,
            "(int)pool.GetCurTaskNum() did not equal curTaskNum as expected.", state);

        AddPoolTaskByForLoop(pool, loopIterations1, TestFuncAddOneTime);
        AddPoolTaskByForLoop(pool, loopIterations2, TestFuncSubOneTime);

        sleep(SLEEP_FOR_ONE_SECOND);
        AssertEqual((int)pool.GetCurTaskNum(), curTaskNum,
            "(int)pool.GetCurTaskNum() did not equal curTaskNum as expected.", state);
        // add 5 tasks, add 3 times and sub 2 times
        BENCHMARK_LOGD("ThreadPoolTest test_05 g_times:%{public}d", g_times);
        AssertEqual(g_times, expectedTimesValue, "g_times did not equal expectedTimesValue as expected.", state);
        pool.Stop();
        g_times = 0;
        BENCHMARK_LOGD("ThreadPoolTest test_05 off");
    }
    BENCHMARK_LOGD("ThreadPoolTest test_05 end.");
}

// simaple task, total task num exceed the MaxTaskNum and the threads num
BENCHMARK_F(BenchmarkThreadPoolTest, test_06)(benchmark::State& state)
{
    BENCHMARK_LOGD("ThreadPoolTest test_06 start.");
    const int startThreads = 5;
    const int threadsNum = 5;
    const int curTaskNum = 0;
    const int maxTaskNum = 10;
    const int loopIterations1 = 8;
    const int loopIterations2 = 7;
    const int expectedTimesValue = 1;
    while (state.KeepRunning()) {
        ThreadPool pool;
        pool.Start(startThreads);
        AssertEqual(pool.GetName(), "", "pool.GetName() did not equal \"\" as expected.", state);
        AssertEqual((int)pool.GetCurTaskNum(), curTaskNum,
            "(int)pool.GetCurTaskNum() did not equal curTaskNum as expected.", state);
        AssertEqual((int)pool.GetThreadsNum(), threadsNum,
            "(int)pool.GetThreadsNum() did not equal threadsNum as expected.", state);

        pool.SetMaxTaskNum(maxTaskNum);

        AddPoolTaskByForLoop(pool, loopIterations1, TestFuncAddOneTime);
        AddPoolTaskByForLoop(pool, loopIterations2, TestFuncSubOneTime);

        sleep(SLEEP_FOR_ONE_SECOND);
        // 1 second should be enough to complete these tasks. if not, this case would be fail
        AssertEqual((int)pool.GetCurTaskNum(), curTaskNum,
            "(int)pool.GetCurTaskNum() did not equal curTaskNum as expected.", state);
        // add 5 task, add 3 times and sub 2 times
        AssertEqual(g_times, expectedTimesValue, "g_times did not equal expectedTimesValue as expected.", state);
        pool.Stop();
        g_times = 0;
    }
    BENCHMARK_LOGD("ThreadPoolTest test_06 end.");
}

void TestFuncAddWait(int& i)
{
    BENCHMARK_LOGD("ThreadPoolTest void TestFuncAddWait is called.");
    std::unique_lock<std::mutex> lk(g_mutex);
    ++g_times;
    BENCHMARK_LOGD("ThreadPoolTest TestFuncAddWait i:%{public}d g_times:%{public}d", i, g_times);
    g_cv.wait(lk, [] {return g_ready;});
    BENCHMARK_LOGD("ThreadPoolTest TestFuncAddWait received i:%{public}d g_ready:%{public}d", i, g_ready);
}

void TestFuncSubWait(int& i)
{
    std::unique_lock<std::mutex> lk(g_mutex);
    --g_times;
    BENCHMARK_LOGD("ThreadPoolTest TestFuncSubWait i:%{public}d g_times:%{public}d", i, g_times);
    g_cv.wait(lk, [] {return g_ready;});
    BENCHMARK_LOGD("ThreadPoolTest TestFuncSubWait received i:%{public}d g_ready:%{public}d", i, g_ready);
}

// complex task, wait for notify by the main thread
// total task num less than the threads num and the MaxTaskNum
BENCHMARK_F(BenchmarkThreadPoolTest, test_07)(benchmark::State& state)
{
    BENCHMARK_LOGD("ThreadPoolTest test_07 start.");
    const int startThreads = 5;
    const int threadsNum = 5;
    const int curTaskNum = 0;
    const int loopIterations1 = 3;
    const int loopIterations2 = 2;
    const int expectedTimesValue = 1;
    while (state.KeepRunning()) {
        ThreadPool pool;
        pool.Start(startThreads);
        AssertEqual(pool.GetName(), "", "pool.GetName() did not equal \"\" as expected.", state);
        AssertEqual((int)pool.GetThreadsNum(), threadsNum,
            "(int)pool.GetThreadsNum() did not equal threadsNum as expected.", state);
        AssertEqual((int)pool.GetCurTaskNum(), curTaskNum,
            "(int)pool.GetCurTaskNum() did not equal curTaskNum as expected.", state);

        AddPoolTaskByForLoop(pool, loopIterations1, TestFuncAddWait);
        AddPoolTaskByForLoop(pool, loopIterations2, TestFuncSubWait);

        // release cpu proactively, let the task threads go into wait
        std::this_thread::sleep_for(std::chrono::seconds(SLEEP_FOR_ONE_SECOND));
        {
            std::lock_guard<std::mutex> lk(g_mutex);
            g_ready = true;
        }

        g_cv.notify_all();

        // these tasks are endless Loop, 5 threads process 5 tasks, zero task remains in the task queue
        AssertEqual((int)pool.GetCurTaskNum(), curTaskNum,
            "(int)pool.GetCurTaskNum() did not equal curTaskNum as expected.", state);
        // add 5 task, add 3 times and sub 2 times
        BENCHMARK_LOGD("ThreadPoolTest test_07 g_times:%{public}d", g_times);
        AssertEqual(g_times, expectedTimesValue, "g_times did not equal expectedTimesValue as expected.", state);
        pool.Stop();
        g_times = 0;
        g_ready = false;
    }
    BENCHMARK_LOGD("ThreadPoolTest test_07 end.");
}

BENCHMARK_F(BenchmarkThreadPoolTest, test_08)(benchmark::State& state)
{
    BENCHMARK_LOGD("ThreadPoolTest test_08 start.");
    const int startThreads = 5;
    const int threadsNum = 5;
    const int curTaskNum = 0;
    const int maxTaskNum = 10;
    const int loopIterations1 = 8;
    const int loopIterations2 = 7;
    const int curTaskNum2 = 10;
    const int expectedTimesValue = 5;
    const int expectedTimesValue2 = 1;
    while (state.KeepRunning()) {
        ThreadPool pool;
        pool.Start(startThreads);
        AssertEqual(pool.GetName(), "", "pool.GetName() did not equal \"\" as expected.", state);
        AssertEqual((int)pool.GetThreadsNum(), threadsNum,
            "(int)pool.GetThreadsNum() did not equal threadsNum as expected.", state);
        AssertEqual((int)pool.GetCurTaskNum(), curTaskNum,
            "(int)pool.GetCurTaskNum() did not equal curTaskNum as expected.", state);

        pool.SetMaxTaskNum(maxTaskNum);

        // ADD 15 tasks
        AddPoolTaskByForLoop(pool, loopIterations1, TestFuncAddWait);
        AddPoolTaskByForLoop(pool, loopIterations2, TestFuncSubWait);

        sleep(SLEEP_FOR_ONE_SECOND);
        // at this time, the first 5 tasks execute and wait for notify, the rest 10 tasks stay in the task queue.
        BENCHMARK_LOGD("ThreadPoolTest test_08 g_times:%{tastNum}d", (int)pool.GetCurTaskNum());
        AssertEqual((int)pool.GetCurTaskNum(), curTaskNum2,
            "(int)pool.GetCurTaskNum() did not equal curTaskNum2 as expected.", state);
        // FIFO,
        AssertEqual(g_times, expectedTimesValue, "g_times did not equal expectedTimesValue as expected.", state);

        // notify_all
        {
            std::lock_guard<std::mutex> lk(g_mutex);
            g_ready = true;
        }
        g_cv.notify_all();

        // after noity, task thread wake up, and g_ready is true, new tasks didn't need to wait
        sleep(SLEEP_FOR_ONE_SECOND);
        // these tasks are endless Loop, and total num of task exceed the MaxTaskNum
        AssertEqual((int)pool.GetCurTaskNum(), curTaskNum,
            "(int)pool.GetCurTaskNum() did not equal curTaskNum as expected.", state);
        AssertEqual(g_times, expectedTimesValue2, "g_times did not equal expectedTimesValue2 as expected.", state);
        pool.Stop();
        g_times = 0;
        g_ready = false;
    }
    BENCHMARK_LOGD("ThreadPoolTest test_08 end.");
}

void TestFuncGetName(const std::string& poolName)
{
    BENCHMARK_LOGD("ThreadPoolTest void TestFuncGetName is called.");
    char name[16];
    prctl(PR_GET_NAME, name);
    std::string nameStr(name);
    size_t found = nameStr.find(poolName);
    if (found != 0) {
        g_flagTestFuncGetName = false;
    }
}

/*
 *  Test_09 is used to verify the name set to ThreadPool will be set as the real name of threads in pool.
 */
BENCHMARK_F(BenchmarkThreadPoolTest, test_09)(benchmark::State& state)
{
    BENCHMARK_LOGD("ThreadPoolTest test_09 start.");
    const int startThreads = 5;
    const int threadsNum = 5;
    const int curTaskNum = 0;
    const int loopIterations = 5;
    while (state.KeepRunning()) {
        std::string poolName("test_09_pool");
        ThreadPool pool(poolName);
        pool.Start(startThreads);
        AssertEqual(pool.GetName(), poolName, "pool.GetName() did not equal poolName as expected.", state);
        AssertEqual((int)pool.GetThreadsNum(), threadsNum,
            "(int)pool.GetThreadsNum() did not equal threadsNum as expected.", state);
        AssertEqual((int)pool.GetCurTaskNum(), curTaskNum,
            "(int)pool.GetCurTaskNum() did not equal curTaskNum as expected.", state);

        for (int i = 0; i < loopIterations; ++i) {
            auto task = std::bind(TestFuncGetName, poolName);
            AssertTrue(g_flagTestFuncGetName, "found did not equal true as expected.", state);
            pool.AddTask(task);
        }

        sleep(SLEEP_FOR_ONE_SECOND);
        // these tasks are endless Loop, 5 threads process 5 tasks, zero task remains in the task queue
        AssertEqual((int)pool.GetCurTaskNum(), curTaskNum,
            "(int)pool.GetCurTaskNum() did not equal curTaskNum as expected.", state);
        pool.Stop();
    }
    BENCHMARK_LOGD("ThreadPoolTest test_09 end.");
}
}  // namespace
}  // namespace OHOS
// Run the benchmark
BENCHMARK_MAIN();