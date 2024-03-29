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
#include "safe_block_queue.h"
#include <array>
#include <future>
#include <iostream>
#include <thread>
#include <chrono>
#include <thread>
#include "benchmark_log.h"
#include "benchmark_assert.h"
using namespace std;

namespace OHOS {
namespace {

class BenchmarkSafeBlockQueue : public benchmark::Fixture {
public:
    void SetUp(const ::benchmark::State& state) override
    {
    }

    void TearDown(const ::benchmark::State& state) override
    {
    }

    BenchmarkSafeBlockQueue()
    {
        Iterations(iterations);
        Repetitions(repetitions);
        ReportAggregatesOnly();
    }

    ~BenchmarkSafeBlockQueue() override = default;

protected:
    const int32_t repetitions = 3;
    const int32_t iterations = 100;
};

const unsigned int QUEUE_SLOTS = 10;
const unsigned int THREAD_NUM = QUEUE_SLOTS + 1;
const int QUEUE_CAPACITY = 10;
const int SLEEP_FOR_TWENTY_MILLISECOND = 20;

class DemoThreadData {
public:
    DemoThreadData()
    {
        putStatus = false;
        getStatus = false;
    }
    static SafeBlockQueue<int> shareQueue;
    bool putStatus;
    bool getStatus;
    void Get()
    {
        shareQueue.Pop();
        getStatus = true;
    }

    void Put(int i)
    {
        shareQueue.Push(i);
        putStatus = true;
    }
};

SafeBlockQueue<int> DemoThreadData::shareQueue(QUEUE_SLOTS);

void PutHandleThreadData(DemoThreadData& q, int i)
{
    q.Put(i);
}

void GetThreadDateGetedStatus(std::array<DemoThreadData, THREAD_NUM>& demoDatas, unsigned int& getedOut,
    unsigned int& ungetedOut)
{
    ungetedOut = 0;
    getedOut = 0;
    for (auto& t : demoDatas) {
        if (t.getStatus) {
            getedOut++;
        } else {
            ungetedOut++;
        }
    }
}

void GetThreadDatePushedStatus(std::array<DemoThreadData, THREAD_NUM>& demoDatas, unsigned int& pushedIn,
    unsigned int& unpushedIn)
{
    unpushedIn = 0;
    pushedIn = 0;
    for (auto& t : demoDatas) {
        if (t.putStatus) {
            pushedIn++;
        } else {
            unpushedIn++;
        }
    }
}

void PutHandleThreadDataTime(DemoThreadData& q, int i,
    std::chrono::time_point<std::chrono::high_resolution_clock> absTime)
{
    std::this_thread::sleep_until(absTime);
    q.Put(i);
}

void GetHandleThreadDataTime(DemoThreadData& q, int i,
    std::chrono::time_point<std::chrono::high_resolution_clock> absTime)
{
    std::this_thread::sleep_until(absTime);
    q.Get();
}

auto GetTimeOfSleepTwentyMillisecond()
{
    using std::chrono::system_clock;
    auto timeT = std::chrono::high_resolution_clock::now();
    timeT += std::chrono::milliseconds(SLEEP_FOR_TWENTY_MILLISECOND);
    return timeT;
}

static void QueuePushFullEquivalent(const int Equivalent, benchmark::State& state)
{
    for (unsigned int i = 0; i < QUEUE_SLOTS; i++) {
        DemoThreadData::shareQueue.Push(Equivalent);
    }
    AssertTrue((DemoThreadData::shareQueue.IsFull()), "shareQueue.IsFull() did not equal true.", state);
}

static void QueuePushInnotfullNotEquivalent(const unsigned int remain)
{
    for (unsigned int i = 0; i < QUEUE_SLOTS - remain; i++) {
        int t = i;
        DemoThreadData::shareQueue.Push(t);
    }
}

/*
 * @tc.name: testPut001
 * @tc.desc: Single-threaded call put and get to determine that the normal scenario
 */
BENCHMARK_F(BenchmarkSafeBlockQueue, testPut001)(benchmark::State& state)
{
    BENCHMARK_LOGD("SafeBlockQueue testPut001 start.");
    while (state.KeepRunning()) {
        SafeBlockQueue<int> qi(QUEUE_CAPACITY);
        int i = 1;
        qi.Push(i);
        const unsigned int expectedQueueSize = 1;
        AssertEqual(static_cast<unsigned>(expectedQueueSize), qi.Size(),
            "static_cast<unsigned>(expectedQueueSize) did not equal qi.Size() as expected.", state);
    }
    BENCHMARK_LOGD("SafeBlockQueue testPut001 end.");
}

/*
 * @tc.name: testGet001
 * @tc.desc: Single-threaded call put and get to determine that the normal scenario
 */
BENCHMARK_F(BenchmarkSafeBlockQueue, testGet001)(benchmark::State& state)
{
    BENCHMARK_LOGD("SafeBlockQueue testGet001 start.");
    const int pushCount = 3;
    const unsigned int expectedQueueSize = 3;
    const int expectedFirstElement = 0;
    while (state.KeepRunning()) {
        SafeBlockQueue<int> qi(QUEUE_CAPACITY);
        for (int i = 0; i < pushCount; i++) {
            qi.Push(i);
        }
        AssertEqual(static_cast<unsigned>(expectedQueueSize), qi.Size(),
            "static_cast<unsigned>(expectedQueueSize) did not equal qi.Size() as expected.", state);
        int t = qi.Pop();
        AssertEqual(t, expectedFirstElement, "t did not equal expectedFirstElement as expected.", state);
    }
    BENCHMARK_LOGD("SafeBlockQueue testGet001 end.");
}

void QueueFullHandler(std::thread (&threads)[THREAD_NUM], std::array<DemoThreadData, THREAD_NUM>& demoDatas,
    benchmark::State& state)
{
    // 1. queue is full and some threads is blocked
    std::this_thread::sleep_for(std::chrono::milliseconds(SLEEP_FOR_TWENTY_MILLISECOND));
    AssertTrue((DemoThreadData::shareQueue.IsFull()),
        "DemoThreadData::shareQueue.IsFull() did not equal true as expected.", state);
    unsigned int pushedIn = 0;
    unsigned int unpushedIn = 0;
    unsigned int getedOut = 0;
    unsigned int ungetedOut = 0;
    GetThreadDatePushedStatus(demoDatas, pushedIn, unpushedIn);
    AssertEqual(pushedIn, QUEUE_SLOTS, "pushedIn did not equal QUEUE_SLOTS as expected.", state);
    AssertEqual(unpushedIn, THREAD_NUM - QUEUE_SLOTS,
        "unpushedIn did not equal THREAD_NUM - QUEUE_SLOTS as expected.", state);
    // 2. get one out  and wait some put in
    for (unsigned int i = 0; i < THREAD_NUM - QUEUE_SLOTS; i++) {
        demoDatas[0].Get();
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(SLEEP_FOR_TWENTY_MILLISECOND));
    // queue is full and some threads is blocked and is not joined
    AssertTrue((DemoThreadData::shareQueue.IsFull()),
        "DemoThreadData::shareQueue.IsFull() did not equal true as expected.", state);
    GetThreadDatePushedStatus(demoDatas, pushedIn, unpushedIn);
    GetThreadDateGetedStatus(demoDatas, getedOut, ungetedOut);
    AssertEqual(pushedIn, THREAD_NUM, "pushedIn did not equal THREAD_NUM as expected.", state);
    AssertEqual(getedOut, THREAD_NUM - QUEUE_SLOTS,
        "getedOut did not equal THREAD_NUM - QUEUE_SLOTS as expected.", state);
    for (auto& t : threads) {
        t.join();
    }
    while (!DemoThreadData::shareQueue.IsEmpty()) {
        demoDatas[0].Get();
    }
}

/*
 * @tc.name: testMutilthreadPutAndBlock001
 * @tc.desc: Multiple threads put until blocking runs, one thread gets, all threads finish running normally
 */
BENCHMARK_F(BenchmarkSafeBlockQueue, testMutilthreadPutAndBlock001)(benchmark::State& state)
{
    BENCHMARK_LOGD("SafeBlockQueue testMutilthreadPutAndBlock001 start.");
    std::thread threads[THREAD_NUM];
    std::array<DemoThreadData, THREAD_NUM> demoDatas;
    while (state.KeepRunning()) {
        demoDatas.fill(DemoThreadData());
        for (unsigned int i = 0; i < THREAD_NUM; i++) {
            threads[i] = std::thread(PutHandleThreadData, std::ref(demoDatas[i]), i);
        }

        QueueFullHandler(threads, demoDatas, state);
    }
    BENCHMARK_LOGD("SafeBlockQueue testMutilthreadPutAndBlock001 end.");
}

/*
 * @tc.name: testMutilthreadConcurrentPutAndBlockInblankqueue001
 * @tc.desc: Multi-threaded put() on the empty queue. When n threads are waiting to reach a certain
 * time-point, everyone puts concurrent to see the status of the queue and the state of the thread.
 */
BENCHMARK_F(BenchmarkSafeBlockQueue, testMutilthreadConcurrentPutAndBlockInblankqueue001)(benchmark::State& state)
{
    BENCHMARK_LOGD("SafeBlockQueue testMutilthreadConcurrentPutAndBlockInblankqueue001 start.");
    std::thread threads[THREAD_NUM];
    std::array<DemoThreadData, THREAD_NUM> demoDatas;
    while (state.KeepRunning()) {
        demoDatas.fill(DemoThreadData());
        auto timeT = GetTimeOfSleepTwentyMillisecond();
        AssertTrue((DemoThreadData::shareQueue.IsEmpty()),
            "DemoThreadData::shareQueue.IsEmpty() did not equal true as expected.", state);
        for (unsigned int i = 0; i < THREAD_NUM; i++) {
            threads[i] = std::thread(PutHandleThreadDataTime,
                                     std::ref(demoDatas[i]), i, timeT);
        }

        QueueFullHandler(threads, demoDatas, state);
    }
    BENCHMARK_LOGD("SafeBlockQueue testMutilthreadConcurrentPutAndBlockInblankqueue001 end.");
}

/*
 * @tc.name: testMutilthreadConcurrentPutAndBlockInfullqueue001
 * @tc.desc: Multi-threaded put() on the full queue. When n threads are waiting to reach a certain
 * time-point, everyone puts concurrent to see the status of the queue and the state of the thread.
 */
BENCHMARK_F(BenchmarkSafeBlockQueue, testMutilthreadConcurrentPutAndBlockInfullqueue001)(benchmark::State& state)
{
    BENCHMARK_LOGD("SafeBlockQueue testMutilthreadConcurrentPutAndBlockInfullqueue001 start.");
    std::thread threads[THREAD_NUM];
    std::array<DemoThreadData, THREAD_NUM> demoDatas;
    while (state.KeepRunning()) {
        demoDatas.fill(DemoThreadData());
        auto timeT = GetTimeOfSleepTwentyMillisecond();
        AssertTrue((DemoThreadData::shareQueue.IsEmpty()),
            "DemoThreadData::shareQueue.IsEmpty() did not equal true as expected.", state);
        for (unsigned int i = 0; i < QUEUE_SLOTS; i++) {
            int t = i;
            DemoThreadData::shareQueue.Push(t);
        }
        AssertTrue((DemoThreadData::shareQueue.IsFull()),
            "DemoThreadData::shareQueue.IsFull() did not equal true as expected.", state);
        for (unsigned int i = 0; i < THREAD_NUM; i++) {
            threads[i] = std::thread(PutHandleThreadDataTime,
                                     std::ref(demoDatas[i]), i, timeT);
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(SLEEP_FOR_TWENTY_MILLISECOND));
        unsigned int pushedIn = 0;
        unsigned int unpushedIn = 0;
        GetThreadDatePushedStatus(demoDatas, pushedIn, unpushedIn);
        AssertEqual(pushedIn, static_cast<unsigned int>(0),
            "pushedIn did not equal static_cast<unsigned int>(0) as expected.", state);
        AssertEqual(unpushedIn, THREAD_NUM, "unpushedIn did not equal THREAD_NUM.", state);
        AssertTrue((DemoThreadData::shareQueue.IsFull()),
            "DemoThreadData::shareQueue.IsFull() did not equal true as expected.", state);
        for (unsigned int i = 0; i < THREAD_NUM; i++) {
            DemoThreadData::shareQueue.Pop();
            std::this_thread::sleep_for(std::chrono::milliseconds(SLEEP_FOR_TWENTY_MILLISECOND));
            AssertTrue((DemoThreadData::shareQueue.IsFull()),
                "DemoThreadData::shareQueue.IsFull() did not equal true as expected.", state);
                GetThreadDatePushedStatus(demoDatas, pushedIn, unpushedIn);
            AssertEqual(pushedIn, i + 1, "pushedIn did not equal i + 1 as expected.", state);
            AssertEqual(unpushedIn, THREAD_NUM - (i + 1), "unpushedIn did not equal THREAD_NUM - (i + 1).", state);
        }
        for (auto& t : threads) {
            t.join();
        }
        while (!DemoThreadData::shareQueue.IsEmpty()) {
            demoDatas[0].Get();
        }
    }
    BENCHMARK_LOGD("SafeBlockQueue testMutilthreadConcurrentPutAndBlockInfullqueue001 end.");
}

/*
 * @tc.name: testMutilthreadConcurrentGetAndBlockInblankqueue001
 * @tc.desc: Multi-threaded get() on the empty queue. When n threads are waiting to reach a certain
 * time-point, everyone gets concurrent to see the status of the queue and the state of the thread.
 */
BENCHMARK_F(BenchmarkSafeBlockQueue, testMutilthreadConcurrentGetAndBlockInblankqueue001)(benchmark::State& state)
{
    BENCHMARK_LOGD("SafeBlockQueue testMutilthreadConcurrentGetAndBlockInblankqueue001 start.");
    std::thread threads[THREAD_NUM];
    std::array<DemoThreadData, THREAD_NUM> demoDatas;
    while (state.KeepRunning()) {
        demoDatas.fill(DemoThreadData());
        auto timeT = GetTimeOfSleepTwentyMillisecond();
        AssertTrue((DemoThreadData::shareQueue.IsEmpty()),
            "DemoThreadData::shareQueue.IsEmpty() did not equal true as expected.", state);
        for (unsigned int i = 0; i < THREAD_NUM; i++) {
            threads[i] = std::thread(GetHandleThreadDataTime,
                std::ref(demoDatas[i]), i, timeT);
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(SLEEP_FOR_TWENTY_MILLISECOND));
        unsigned int getedOut = 0;
        unsigned int ungetedOut = 0;
        GetThreadDateGetedStatus(demoDatas, getedOut, ungetedOut);
        AssertEqual(getedOut, static_cast<unsigned int>(0),
            "getedOut did not equal static_cast<unsigned int>(0) as expected.", state);
        AssertEqual(ungetedOut, THREAD_NUM, "ungetedOut did not equal THREAD_NUM as expected.", state);
        AssertTrue((DemoThreadData::shareQueue.IsEmpty()),
            "DemoThreadData::shareQueue.IsEmpty() did not equal true as expected.", state);

        int value = 1;
        for (unsigned int i = 0; i < THREAD_NUM; i++) {
            DemoThreadData::shareQueue.Push(value);
            std::this_thread::sleep_for(std::chrono::milliseconds(SLEEP_FOR_TWENTY_MILLISECOND));
            AssertTrue((DemoThreadData::shareQueue.IsEmpty()),
                "DemoThreadData::shareQueue.IsEmpty() did not equal true as expected.", state);
                GetThreadDateGetedStatus(demoDatas, getedOut, ungetedOut);
            AssertEqual(getedOut, i + 1, "getedOut did not equal i + 1 as expected.", state);
            AssertEqual(ungetedOut, THREAD_NUM - (i + 1),
                "ungetedOut did not equal THREAD_NUM - (i + 1) as expected.", state);
        }
        for (auto& t : threads) {
            t.join();
        }

        while (!DemoThreadData::shareQueue.IsEmpty()) {
            demoDatas[0].Get();
        }
    }
    BENCHMARK_LOGD("SafeBlockQueue testMutilthreadConcurrentGetAndBlockInblankqueue001 end.");
}

/*
 * @tc.name: testMutilthreadConcurrentGetAndBlockInfullqueue001
 * @tc.desc: Multi-threaded get() on the full queue. When n threads are waiting to reach a certain
 * time-point, everyone gets concurrent to see the status of the queue and the state of the thread.
 */
BENCHMARK_F(BenchmarkSafeBlockQueue, testMutilthreadConcurrentGetAndBlockInfullqueue001)(benchmark::State& state)
{
    BENCHMARK_LOGD("SafeBlockQueue testMutilthreadConcurrentGetAndBlockInfullqueue001 start.");
    std::thread threads[THREAD_NUM];
    std::array<DemoThreadData, THREAD_NUM> demoDatas;
    while (state.KeepRunning()) {
        demoDatas.fill(DemoThreadData());
        auto timeT = GetTimeOfSleepTwentyMillisecond();
        AssertTrue((DemoThreadData::shareQueue.IsEmpty()),
            "DemoThreadData::shareQueue.IsEmpty() did not equal true as expected.", state);
        int t = 1;
        QueuePushFullEquivalent(t, state);
        for (unsigned int i = 0; i < THREAD_NUM; i++) {
            threads[i] = std::thread(GetHandleThreadDataTime,
                                     std::ref(demoDatas[i]), i, timeT);
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(SLEEP_FOR_TWENTY_MILLISECOND));
        AssertTrue((DemoThreadData::shareQueue.IsEmpty()),
            "DemoThreadData::shareQueue.IsEmpty() did not equal true as expected.", state);
        unsigned int getedOut = 0;
        unsigned int ungetedOut = 0;
        GetThreadDateGetedStatus(demoDatas, getedOut, ungetedOut);
        AssertEqual(getedOut, QUEUE_SLOTS, "getedOut did not equal QUEUE_SLOTS as expected.", state);
        AssertEqual(ungetedOut, THREAD_NUM - QUEUE_SLOTS, "ungetedOut did not equal THREAD_NUM - QUEUE_SLOTS.", state);
        for (unsigned int i = 0; i < THREAD_NUM - QUEUE_SLOTS; i++) {
            demoDatas[0].Put(t);
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(SLEEP_FOR_TWENTY_MILLISECOND));
        AssertTrue((DemoThreadData::shareQueue.IsEmpty()),
            "DemoThreadData::shareQueue.IsEmpty() did not equal true as expected.", state);
        GetThreadDateGetedStatus(demoDatas, getedOut, ungetedOut);
        AssertEqual(getedOut, THREAD_NUM, "getedOut did not equal THREAD_NUM as expected.", state);
        AssertEqual(ungetedOut, static_cast<unsigned int>(0),
            "ungetedOut did not equal static_cast<unsigned int>(0) as expected.", state);
        for (auto& singleThread : threads) {
            singleThread.join();
        }
        while (!DemoThreadData::shareQueue.IsEmpty()) {
            demoDatas[0].Get();
        }
    }
    BENCHMARK_LOGD("SafeBlockQueue testMutilthreadConcurrentGetAndBlockInfullqueue001 end.");
}

/*
 * @tc.name: testMutilthreadConcurrentGetAndBlockInnotfullqueue001
 * @tc.desc: Multi-threaded get() on the notfull queue. When n threads are waiting to reach a certain
 * time-point, everyone get concurrent to see the status of the queue and the state of the thread.
 */
BENCHMARK_F(BenchmarkSafeBlockQueue, testMutilthreadConcurrentGetAndBlockInnotfullqueue001)(benchmark::State& state)
{
    BENCHMARK_LOGD("SafeBlockQueue testMutilthreadConcurrentGetAndBlockInnotfullqueue001 start.");
    std::thread threads[THREAD_NUM];
    std::array<DemoThreadData, THREAD_NUM> demoDatas;
    while (state.KeepRunning()) {
        demoDatas.fill(DemoThreadData());
        auto timeT = GetTimeOfSleepTwentyMillisecond();
        const unsigned int REMAIN_SLOTS = 5;
        AssertTrue((DemoThreadData::shareQueue.IsEmpty()), "shareQueue.IsEmpty() did not equal true.", state);
        QueuePushInnotfullNotEquivalent(REMAIN_SLOTS);
        for (unsigned int i = 0; i < THREAD_NUM; i++) {
            threads[i] = std::thread(GetHandleThreadDataTime,
                std::ref(demoDatas[i]), i, timeT);
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(SLEEP_FOR_TWENTY_MILLISECOND));
        unsigned int getedOut = 0;
        unsigned int ungetedOut = 0;
        GetThreadDateGetedStatus(demoDatas, getedOut, ungetedOut);
        AssertEqual(getedOut, QUEUE_SLOTS - REMAIN_SLOTS,
            "getedOut did not equal QUEUE_SLOTS - REMAIN_SLOTS as expected.", state);
        AssertEqual(ungetedOut, THREAD_NUM - (QUEUE_SLOTS - REMAIN_SLOTS),
            "ungetedOut did not equal THREAD_NUM - (QUEUE_SLOTS - REMAIN_SLOTS) as expected.", state);
        for (unsigned int i = 0; i < ungetedOut; i++) {
            int t = i;
            DemoThreadData::shareQueue.Push(t);
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(SLEEP_FOR_TWENTY_MILLISECOND));
        GetThreadDateGetedStatus(demoDatas, getedOut, ungetedOut);
        AssertEqual(getedOut, THREAD_NUM, "getedOut did not equal THREAD_NUM as expected.", state);
        AssertEqual(ungetedOut, static_cast<unsigned int>(0),
            "ungetedOut did not equal static_cast<unsigned int>(0) as expected.", state);
        for (auto& t : threads) {
            t.join();
        }
        while (!DemoThreadData::shareQueue.IsEmpty()) {
            demoDatas[0].Get();
        }
    }
    BENCHMARK_LOGD("SafeBlockQueue testMutilthreadConcurrentGetAndBlockInnotfullqueue001 end.");
}

/*
 * @tc.name: testMutilthreadConcurrentPutAndBlockInnotfullqueue001
 * @tc.desc: Multi-threaded put() on the not full queue. When n threads are waiting to reach a certain
 * time-point, everyone puts concurrent to see the status of the queue and the state of the thread.
 */
BENCHMARK_F(BenchmarkSafeBlockQueue, testMutilthreadConcurrentPutAndBlockInnotfullqueue001)(benchmark::State& state)
{
    BENCHMARK_LOGD("SafeBlockQueue testMutilthreadConcurrentPutAndBlockInnotfullqueue001 start.");
    std::thread threads[THREAD_NUM];
    std::array<DemoThreadData, THREAD_NUM> demoDatas;
    while (state.KeepRunning()) {
        demoDatas.fill(DemoThreadData());
        auto timeT = GetTimeOfSleepTwentyMillisecond();
        const unsigned int REMAIN_SLOTS = 5;
        AssertTrue((DemoThreadData::shareQueue.IsEmpty()), "shareQueue.IsEmpty() did not equal true.", state);
        QueuePushInnotfullNotEquivalent(REMAIN_SLOTS);
        for (unsigned int i = 0; i < THREAD_NUM; i++) {
            threads[i] = std::thread(PutHandleThreadDataTime,
                std::ref(demoDatas[i]), i, timeT);
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(SLEEP_FOR_TWENTY_MILLISECOND));
        unsigned int putedin = 0;
        unsigned int unputedin = 0;
        GetThreadDatePushedStatus(demoDatas, putedin, unputedin);
        AssertEqual(putedin, REMAIN_SLOTS, "putedin did not equal REMAIN_SLOTS as expected.", state);
        AssertEqual(unputedin, THREAD_NUM - REMAIN_SLOTS,
            "unputedin did not equal THREAD_NUM - REMAIN_SLOTS as expected.", state);
        for (unsigned int i = 0; i < unputedin; i++) {
            DemoThreadData::shareQueue.Pop();
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(SLEEP_FOR_TWENTY_MILLISECOND));
        GetThreadDatePushedStatus(demoDatas, putedin, unputedin);
        AssertEqual(putedin, THREAD_NUM, "putedin did not equal THREAD_NUM as expected.", state);
        AssertEqual(unputedin, static_cast<unsigned int>(0),
            "unputedin did not equal static_cast<unsigned int>(0) as expected.", state);
        for (auto& t : threads) {
            t.join();
        }
        while (!DemoThreadData::shareQueue.IsEmpty()) {
            demoDatas[0].Get();
        }
    }
    BENCHMARK_LOGD("SafeBlockQueue testMutilthreadConcurrentPutAndBlockInnotfullqueue001 end.");
}

/*
 * @tc.name: testMutilthreadConcurrentGetAndPopInblankqueue001
 * @tc.desc: Multi-threaded put() and Multi-threaded get() on the empty queue. When all threads are waiting to reach
 * a certain time-point, everyone run concurrently to see the status of the queue and the state of the thread.
 */
BENCHMARK_F(BenchmarkSafeBlockQueue, testMutilthreadConcurrentGetAndPopInblankqueue001)(benchmark::State& state)
{
    BENCHMARK_LOGD("SafeBlockQueue testMutilthreadConcurrentGetAndPopInblankqueue001 start.");
    std::thread threadsout[THREAD_NUM];
    std::array<DemoThreadData, THREAD_NUM> demoDatas;
    while (state.KeepRunning()) {
        demoDatas.fill(DemoThreadData());
        std::thread threadsin[THREAD_NUM];
        auto timeT = GetTimeOfSleepTwentyMillisecond();
        AssertTrue((DemoThreadData::shareQueue.IsEmpty()),
            "DemoThreadData::shareQueue.IsEmpty() did not equal true as expected.", state);
        for (unsigned int i = 0; i < THREAD_NUM; i++) {
            threadsout[i] = std::thread(GetHandleThreadDataTime,
                std::ref(demoDatas[i]), i, timeT);
        }
        for (unsigned int i = 0; i < THREAD_NUM; i++) {
            threadsin[i] = std::thread(PutHandleThreadDataTime,
                std::ref(demoDatas[i]), i, timeT);
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(SLEEP_FOR_TWENTY_MILLISECOND));
        AssertTrue((DemoThreadData::shareQueue.IsEmpty()),
            "DemoThreadData::shareQueue.IsEmpty() did not equal true as expected.", state);
        unsigned int getedOut = 0;
        unsigned int ungetedOut = 0;
        unsigned int pushedIn = 0;
        unsigned int unpushedIn = 0;
        GetThreadDateGetedStatus(demoDatas, getedOut, ungetedOut);
        GetThreadDatePushedStatus(demoDatas, pushedIn, unpushedIn);
        AssertEqual(pushedIn, THREAD_NUM, "pushedIn did not equal THREAD_NUM as expected.", state);
        AssertEqual(getedOut, THREAD_NUM, "getedOut did not equal THREAD_NUM as expected.", state);
        for (auto& t : threadsout) {
            t.join();
        }
        for (auto& t : threadsin) {
            t.join();
        }
        while (!DemoThreadData::shareQueue.IsEmpty()) {
            demoDatas[0].Get();
        }
    }
    BENCHMARK_LOGD("SafeBlockQueue testMutilthreadConcurrentGetAndPopInblankqueue001 end.");
}

/*
 * @tc.name: testMutilthreadConcurrentGetAndPopInfullqueue001
 * @tc.desc: Multi-threaded put() and Multi-threaded get() on the full queue.
 * When all threads are waiting to reach a certain
 * time-point, everyone run concurrently to see the status of the queue and the state of the thread.
 */
BENCHMARK_F(BenchmarkSafeBlockQueue, testMutilthreadConcurrentGetAndPopInfullqueue001)(benchmark::State& state)
{
    BENCHMARK_LOGD("SafeBlockQueue testMutilthreadConcurrentGetAndPopInfullqueue001 start.");
    std::thread threadsout[THREAD_NUM];
    std::array<DemoThreadData, THREAD_NUM> demoDatas;
    while (state.KeepRunning()) {
        demoDatas.fill(DemoThreadData());
        std::thread threadsin[THREAD_NUM];
        auto timeT = GetTimeOfSleepTwentyMillisecond();
        AssertTrue((DemoThreadData::shareQueue.IsEmpty()),
            "DemoThreadData::shareQueue.IsEmpty() did not equal true as expected.", state);
        int t = 1;
        QueuePushFullEquivalent(t, state);
        for (unsigned int i = 0; i < THREAD_NUM; i++) {
            threadsin[i] = std::thread(PutHandleThreadDataTime,
                std::ref(demoDatas[i]), i, timeT);
        }
        for (unsigned int i = 0; i < THREAD_NUM; i++) {
            threadsout[i] = std::thread(GetHandleThreadDataTime,
                std::ref(demoDatas[i]), i, timeT);
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(SLEEP_FOR_TWENTY_MILLISECOND));
        AssertTrue((DemoThreadData::shareQueue.IsFull()),
            "DemoThreadData::shareQueue.IsFull() did not equal true as expected.", state);
        unsigned int getedOut = 0;
        unsigned int ungetedOut = 0;
        unsigned int pushedIn = 0;
        unsigned int unpushedIn = 0;
        GetThreadDateGetedStatus(demoDatas, getedOut, ungetedOut);
        GetThreadDatePushedStatus(demoDatas, pushedIn, unpushedIn);
        AssertEqual(pushedIn, THREAD_NUM, "pushedIn did not equal THREAD_NUM as expected.", state);
        AssertEqual(getedOut, THREAD_NUM, "getedOut did not equal THREAD_NUM as expected.", state);
        for (auto& outThread : threadsout) {
            outThread.join();
        }
        for (auto& inThread : threadsin) {
            inThread.join();
        }
        while (!DemoThreadData::shareQueue.IsEmpty()) {
            demoDatas[0].Get();
        }
    }
    BENCHMARK_LOGD("SafeBlockQueue testMutilthreadConcurrentGetAndPopInfullqueue001 end.");
}

/*
 * @tc.name: testPushNoWait001
 * @tc.desc: Single-threaded call PushNoWait and check the performance
 */
BENCHMARK_F(BenchmarkSafeBlockQueue, testPushNoWait001)(benchmark::State& state)
{
    BENCHMARK_LOGD("SafeBlockQueue testPushNoWait001 start.");
    while (state.KeepRunning()) {
        SafeBlockQueue<int> qi(QUEUE_CAPACITY);
        int i = 1;
        bool result = qi.PushNoWait(i);
        AssertTrue(result, "PushNoWait returned false, expected true.", state);
    }
    BENCHMARK_LOGD("SafeBlockQueue testPushNoWait001 end.");
}

/*
 * @tc.name: testPopNoWait001
 * @tc.desc: Single-threaded call PopNoWait and check the performance
 */
BENCHMARK_F(BenchmarkSafeBlockQueue, testPopNoWait001)(benchmark::State& state)
{
    BENCHMARK_LOGD("SafeBlockQueue testPopNoWait001 start.");
    const int pushCount = 10;
    while (state.KeepRunning()) {
        SafeBlockQueue<int> qi(QUEUE_CAPACITY);
        for (int i = 0; i < pushCount; i++) {
            qi.Push(i);  // Fill the queue
        }
        int out;
        bool result = qi.PopNotWait(out);
        AssertTrue(result, "PopNoWait returned false, expected true.", state);
    }
    BENCHMARK_LOGD("SafeBlockQueue testPopNoWait001 end.");
}
}  // namespace
}  // namespace OHOS
// Run the benchmark
BENCHMARK_MAIN();