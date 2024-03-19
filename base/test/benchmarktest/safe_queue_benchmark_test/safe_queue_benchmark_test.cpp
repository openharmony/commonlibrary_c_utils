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
#include "safe_queue.h"
#include <array>
#include <future>
#include <iostream>
#include <thread>
#include <chrono>
#include "benchmark_log.h"
#include "benchmark_assert.h"
using namespace std;

namespace OHOS {
namespace {

const unsigned int QUEUE_SLOTS = 10;
const unsigned int THREAD_NUM = QUEUE_SLOTS + 1;
const int TIME_INCREMENT = 2;
const int SLEEP_FOR_TWO_SECONDS = 2;
const int SLEEP_FOR_THREE_SECONDS = 3;
const int SLEEP_FOR_FOUR_SECONDS = 4;

class BenchmarkSafeQueue : public benchmark::Fixture {
public:
    void SetUp(const ::benchmark::State& state) override
    {
    }

    void TearDown(const ::benchmark::State& state) override
    {
    }

    BenchmarkSafeQueue()
    {
        Iterations(iterations);
        Repetitions(repetitions);
        ReportAggregatesOnly();
    }

    ~BenchmarkSafeQueue() override = default;

protected:
    const int32_t repetitions = 3;
    const int32_t iterations = 50;
};

class DemoThreadData {
public:
    DemoThreadData()
    {
        putStatus = false;
        getStatus = false;
        eraseStatus = false;
        emptyStatus = false;
    }
    static SafeQueue<int> shareQueue;
    bool putStatus;
    bool getStatus;
    bool eraseStatus;
    bool emptyStatus;

    void Put(int i)
    {
        shareQueue.Push(i);
        putStatus = true;
    }

    void Get(int &i)
    {
        shareQueue.Pop(i);
        getStatus = true;
    }

    void Erase(int &i)
    {
        shareQueue.Erase(i);
        eraseStatus = true;
    }

    void Empty()
    {
        shareQueue.Empty();
        emptyStatus = true;
    }
};

SafeQueue<int> DemoThreadData::shareQueue;

void PutHandleThreadDataTime(DemoThreadData &q, int i, std::chrono::system_clock::time_point absTime)
{
    BENCHMARK_LOGD("SafeQueue PutHandleThreadDataTime is called i:%{public}d .", i);
    std::this_thread::sleep_until(absTime);

    q.Put(i);
}

void GetHandleThreadDataTime(DemoThreadData &q, int i, std::chrono::system_clock::time_point absTime)
{
    BENCHMARK_LOGD("SafeQueue GetHandleThreadDataTime is called i:%{public}d.", i);
    std::this_thread::sleep_until(absTime);
    int t = 0;
    q.Get(t);
}

void EraseHandleThreadDataTime(DemoThreadData &q, int i, std::chrono::system_clock::time_point absTime)
{
    BENCHMARK_LOGD("SafeQueue EraseHandleThreadDataTime is called i:%{public}d.", i);
    std::this_thread::sleep_until(absTime);

    q.Erase(i);
}

void EmptyHandleThreadDataTime(DemoThreadData &q, std::chrono::system_clock::time_point absTime)
{
    BENCHMARK_LOGD("SafeQueue EmptyHandleThreadDataTime is called.");
    std::this_thread::sleep_until(absTime);

    q.Empty();
}

class TestThreading {
public:
    TestThreading()
    {
        demoDatas.fill(DemoThreadData());
    }

    void AllThreadPut(std::time_t &timeT)
    {
        using std::chrono::system_clock;
        for (unsigned int i = 0; i < THREAD_NUM; i++) {
            threads[i] = std::thread(PutHandleThreadDataTime,
                std::ref(demoDatas[i]), i, system_clock::from_time_t(timeT));
        }
    }
    void AllThreadGet(std::time_t &timeT)
    {
        using std::chrono::system_clock;
        for (unsigned int i = 0; i < THREAD_NUM; i++) {
            threads[i] = std::thread(GetHandleThreadDataTime,
                std::ref(demoDatas[i]), i, system_clock::from_time_t(timeT));
        }
    }

    void AllThreadErase(std::time_t &timeT)
    {
        using std::chrono::system_clock;
        for (unsigned int i = 0; i < THREAD_NUM; i++) {
            threads[i] = std::thread(EraseHandleThreadDataTime,
                std::ref(demoDatas[i]), i, system_clock::from_time_t(timeT));
        }
    }

    void AllThreadEmpty(std::time_t &timeT)
    {
        using std::chrono::system_clock;
        for (unsigned int i = 0; i < THREAD_NUM; i++) {
            threads[i] = std::thread(EmptyHandleThreadDataTime,
                std::ref(demoDatas[i]), system_clock::from_time_t(timeT));
        }
    }

    void GetThreadDatePushedStatus(unsigned int &pushedIn, unsigned int &unpushedIn)
    {
        pushedIn = 0;
        unpushedIn = 0;
        for (auto &t : demoDatas) {
            if (t.putStatus) {
                pushedIn++;
            } else {
                unpushedIn++;
            }
        }
        BENCHMARK_LOGD("SafeQueue GetThreadDatePushedStatus pIn:%{public}d upIn:%{public}d.", pushedIn, unpushedIn);
    }

    void GetThreadDateGetedStatus(unsigned int &getedOut, unsigned int &ungetedOut)
    {
        BENCHMARK_LOGD("SafeQueue void GetThreadDateGetedStatus is called.");
        getedOut = 0;
        ungetedOut = 0;
        for (auto &t : demoDatas) {
            if (t.getStatus) {
                getedOut++;
            } else {
                ungetedOut++;
            }
        }
        BENCHMARK_LOGD("SafeQueue GetThreadDateGetedStatus gOut:%{public}d uOut:%{public}d.", getedOut, ungetedOut);
    }

    void GetThreadDateEraseStatus(unsigned int &erase, unsigned int &unErase)
    {
        erase = 0;
        unErase = 0;
        for (auto &t : demoDatas) {
            if (t.eraseStatus) {
                erase++;
            } else {
                unErase++;
            }
        }
        BENCHMARK_LOGD("SafeQueue GetThreadDateEraseStatus erase:%{public}d unErase:%{public}d.", erase, unErase);
    }

    void GetThreadDateEmptyStatus(unsigned int &empty, unsigned int &unEmpty)
    {
        empty = 0;
        unEmpty = 0;
        for (auto &t : demoDatas) {
            if (t.emptyStatus) {
                empty++;
            } else {
                unEmpty++;
            }
        }
        BENCHMARK_LOGD("SafeQueue GetThreadDateEmptyStatus empty:%{public}d unEmpty:%{public}d.", empty, unEmpty);
    }

    void ResetStatus()
    {
        BENCHMARK_LOGD("SafeQueue void ResetStatus is called.");
        for (auto &t : threads) {
            t.join();
        }

        DemoThreadData::shareQueue.Clear();
    }

    std::thread threads[THREAD_NUM];
    std::array<DemoThreadData, THREAD_NUM> demoDatas;
};

/*
* Feature: SafeBlockQueue
* Function:put
* SubFunction: NA
* FunctionPoints:
* EnvConditions: NA
* CaseDescription: Multiple threads put, one thread gets, all threads finish running normally
*/
BENCHMARK_F(BenchmarkSafeQueue, testMutilthreadPutAndOneThreadGetOnemptyQueue)(benchmark::State& state)
{
    BENCHMARK_LOGD("SafeQueue testMutilthreadPutAndOneThreadGetOnemptyQueue start.");
    while (state.KeepRunning()) {
        TestThreading testThread;
        using std::chrono::system_clock;
        std::time_t timeT = system_clock::to_time_t(system_clock::now());
        timeT += TIME_INCREMENT;
        testThread.AllThreadPut(timeT);

        // 1. queue is full and some threads is blocked
        std::this_thread::sleep_for(std::chrono::seconds(SLEEP_FOR_THREE_SECONDS));
        AssertTrue((DemoThreadData::shareQueue.Size() > 0),
            "DemoThreadData::shareQueue.Size() > 0 did not equal true as expected.", state);

        unsigned int pushedIn = 0;
        unsigned int unpushedIn = 0;

        testThread.GetThreadDatePushedStatus(pushedIn, unpushedIn);
        AssertEqual(pushedIn, THREAD_NUM, "pushedIn did not equal THREAD_NUM as expected.", state);

        //2.  get one out  and wait some put in
        for (unsigned int i = 0; i < THREAD_NUM; i++) {
            int t = 0;
            testThread.demoDatas[0].Get(t);
        }

        std::this_thread::sleep_for(std::chrono::seconds(SLEEP_FOR_TWO_SECONDS));
        // queue is full and some threads is blocked and is not joined
        AssertTrue((DemoThreadData::shareQueue.Size() == 0),
            "DemoThreadData::shareQueue.Size() == 0 did not equal true as expected.", state);

        // here means all thread end ok or if some operation blocked and the testcase blocked
        testThread.ResetStatus();
    }
    BENCHMARK_LOGD("SafeQueue testMutilthreadPutAndOneThreadGetOnemptyQueue end.");
}

/*
* Feature: SafeBlockQueue
* Function:put
* SubFunction: NA
* FunctionPoints:
* EnvConditions: NA
* CaseDescription: Multi-threaded put() and Multi-threaded get() on the empty queue.
* When all threads are waiting to reach a certain
* time-point, everyone run concurrently to see the status of the queue and the state of the thread.
*/
BENCHMARK_F(BenchmarkSafeQueue, testMutilthreadPutAndGetConcurrently)(benchmark::State& state)
{
    BENCHMARK_LOGD("SafeQueue testMutilthreadPutAndGetConcurrently start.");
    while (state.KeepRunning()) {
        using std::chrono::system_clock;
        std::time_t timeT = system_clock::to_time_t(system_clock::now());
        timeT += TIME_INCREMENT;

        TestThreading putInTestThread;
        putInTestThread.AllThreadPut(timeT);

        TestThreading getOutTestThread;
        getOutTestThread.AllThreadGet(timeT);

        // 1. queue is full and some threads is blocked
        std::this_thread::sleep_for(std::chrono::seconds(SLEEP_FOR_FOUR_SECONDS));

        unsigned int pushedIn = 0;
        unsigned int unpushedIn = 0;
        unsigned int getedOut = 0;
        unsigned int ungetedOut = 0;
        putInTestThread.GetThreadDatePushedStatus(pushedIn, unpushedIn);
        getOutTestThread.GetThreadDateGetedStatus(getedOut, ungetedOut);
        AssertEqual(pushedIn, THREAD_NUM, "pushedIn did not equal THREAD_NUM as expected.", state);
        AssertEqual(getedOut, THREAD_NUM, "getedOut did not equal THREAD_NUM as expected.", state);

        putInTestThread.ResetStatus();
        getOutTestThread.ResetStatus();
    }
    BENCHMARK_LOGD("SafeQueue testMutilthreadPutAndGetConcurrently end.");
}

/*
* Feature: SafeBlockQueue
* Function:put
* SubFunction: NA
* FunctionPoints:
* EnvConditions: NA
* CaseDescription: Multi-threaded put() and Multi-threaded get() on the not empty queue.
* When all threads are waiting to reach a certain
* time-point, everyone run concurrently to see the status of the queue and the state of the thread.
*/
BENCHMARK_F(BenchmarkSafeQueue, testMutilthreadConcurrentGetAndPopInNotEmptyQueue)(benchmark::State& state)
{
    BENCHMARK_LOGD("SafeQueue testMutilthreadConcurrentGetAndPopInNotEmptyQueue start.");
    while (state.KeepRunning()) {
        //1. prepare
        using std::chrono::system_clock;
        std::time_t timeT = system_clock::to_time_t(system_clock::now());
        timeT += TIME_INCREMENT;

        AssertTrue((DemoThreadData::shareQueue.Size() == 0),
            "DemoThreadData::shareQueue.Size() == 0 did not equal true as expected.", state);

        int t = 1;
        for (unsigned int i = 0; i < THREAD_NUM; i++) {
            DemoThreadData::shareQueue.Push(t);
        }

        AssertTrue((DemoThreadData::shareQueue.Size() == THREAD_NUM),
            "DemoThreadData::shareQueue.Size() == THREAD_NUM did not equal true as expected.", state);

        //2. start thread put in not full queue
        TestThreading putInTestThread;
        putInTestThread.AllThreadPut(timeT);

        TestThreading getOutTestThread;
        getOutTestThread.AllThreadGet(timeT);

        std::this_thread::sleep_for(std::chrono::seconds(SLEEP_FOR_THREE_SECONDS));
        AssertTrue((DemoThreadData::shareQueue.Size() == THREAD_NUM),
            "DemoThreadData::shareQueue.Size() == THREAD_NUM did not equal true as expected.", state);

        unsigned int getedOut = 0;
        unsigned int ungetedOut = 0;
        unsigned int pushedIn = 0;
        unsigned int unpushedIn = 0;
        getOutTestThread.GetThreadDateGetedStatus(getedOut, ungetedOut);
        putInTestThread.GetThreadDatePushedStatus(pushedIn, unpushedIn);
        AssertEqual(pushedIn, THREAD_NUM, "pushedIn did not equal THREAD_NUM as expected.", state);
        AssertEqual(getedOut, THREAD_NUM, "getedOut did not equal THREAD_NUM as expected.", state);

        // 3. reset status
        putInTestThread.ResetStatus();
        getOutTestThread.ResetStatus();
    }
    BENCHMARK_LOGD("SafeQueue testMutilthreadConcurrentGetAndPopInNotEmptyQueue end.");
}

/*
* Feature: SafeBlockQueue
* Function:erase empty
* SubFunction: NA
* FunctionPoints:
* EnvConditions: NA
* CaseDescription: Multi-threaded erase() and Multi-threaded empty() on the empty queue.
* When all threads are waiting to reach a certain
* time-point, everyone run concurrently to see the status of the queue and the state of the thread.
*/
BENCHMARK_F(BenchmarkSafeQueue, testMutilthreadEraseAndEmptyConcurrently)(benchmark::State& state)
{
    BENCHMARK_LOGD("SafeQueue testMutilthreadEraseAndEmptyConcurrently start.");
    while (state.KeepRunning()) {
        using std::chrono::system_clock;
        std::time_t timeT = system_clock::to_time_t(system_clock::now());
        timeT += TIME_INCREMENT;

        TestThreading putThread;
        putThread.AllThreadPut(timeT);

        TestThreading eraseThread;
        eraseThread.AllThreadErase(timeT);

        TestThreading emptyThread;
        emptyThread.AllThreadEmpty(timeT);
        std::this_thread::sleep_for(std::chrono::seconds(SLEEP_FOR_FOUR_SECONDS));

        unsigned int pushedIn = 0;
        unsigned int unpushedIn = 0;
        unsigned int erase = 0;
        unsigned int unerase = 0;
        unsigned int empty = 0;
        unsigned int unempty = 0;
        putThread.GetThreadDatePushedStatus(pushedIn, unpushedIn);
        eraseThread.GetThreadDateEraseStatus(erase, unerase);
        emptyThread.GetThreadDateEmptyStatus(empty, unempty);
        AssertEqual(pushedIn, THREAD_NUM, "pushedIn did not equal THREAD_NUM as expected.", state);
        AssertEqual(erase, THREAD_NUM, "erase did not equal THREAD_NUM as expected.", state);
        AssertEqual(empty, THREAD_NUM, "empty did not equal THREAD_NUM as expected.", state);

        putThread.ResetStatus();
        eraseThread.ResetStatus();
        emptyThread.ResetStatus();
    }
    BENCHMARK_LOGD("SafeQueue testMutilthreadEraseAndEmptyConcurrently end.");
}
}  // namespace
}  // namespace OHOS
// Run the benchmark
BENCHMARK_MAIN();