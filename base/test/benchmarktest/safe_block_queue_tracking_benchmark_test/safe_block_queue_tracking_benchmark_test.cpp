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
#include <thread>
#include <iostream>
#include <chrono>
#include "benchmark_log.h"
#include "benchmark_assert.h"
using namespace std;

namespace OHOS {
namespace {

class BenchmarkSafeBlockQueueTracking : public benchmark::Fixture {
public:
    void SetUp(const ::benchmark::State& state) override
    {
    }

    void TearDown(const ::benchmark::State& state) override
    {
    }

    BenchmarkSafeBlockQueueTracking()
    {
        Iterations(iterations);
        Repetitions(repetitions);
        ReportAggregatesOnly();
    }

    ~BenchmarkSafeBlockQueueTracking() override = default;

protected:
    const int32_t repetitions = 3;
    const int32_t iterations = 50;
};

const unsigned int QUEUE_SLOTS = 5;
const unsigned int THREAD_NUM = QUEUE_SLOTS + 1;
const int QUEUE_CAPACITY = 10;
const int SLEEP_FOR_HUNDRED_MILLISECOND = 50;

class DemoThreadData {
public:
    DemoThreadData()
    {
        putStatus = false;
        getStatus = false;
        joinStatus = false;
    }

    static SafeBlockQueueTracking<int> shareQueue;
    static bool joinStatus;
    bool putStatus;
    bool getStatus;

    void Put(int i)
    {
        shareQueue.Push(i);
        putStatus = true;
    }

    void Get()
    {
        shareQueue.Pop();
        getStatus = true;
    }

    void GetAndOneTaskDone()
    {
        shareQueue.Pop();
        getStatus = true;
        shareQueue.OneTaskDone();
    }

    void Join()
    {
        shareQueue.Join();
        joinStatus = true;
    }
};

SafeBlockQueueTracking<int> DemoThreadData::shareQueue(QUEUE_SLOTS);
bool DemoThreadData::joinStatus = false;

void PutHandleThreadData(DemoThreadData& q, int i)
{
    q.Put(i);
}

void GetThreadDateGetedStatus(std::array<DemoThreadData, THREAD_NUM>& demoDatas, unsigned int& getedOut,
    unsigned int& ungetedOut)
{
    getedOut = 0;
    ungetedOut = 0;
    for (auto& t : demoDatas) {
        if (t.getStatus) {
            getedOut += 1;
        } else {
            ungetedOut += 1;
        }
    }
}

void GetThreadDatePushedStatus(std::array<DemoThreadData, THREAD_NUM>& demoDatas, unsigned int& pushedIn,
    unsigned int& unpushedIn)
{
    pushedIn = 0;
    unpushedIn = 0;
    for (auto& t : demoDatas) {
        if (t.putStatus) {
            pushedIn += 1;
        } else {
            unpushedIn += 1;
        }
    }
}

void PutHandleThreadDataTime(DemoThreadData& q, int i,
    std::chrono::time_point<std::chrono::high_resolution_clock> absTime)
{
    std::this_thread::sleep_until(absTime);
    q.Put(i);
}

void GetAndOneTaskDoneHandleThreadDataTime(DemoThreadData& q, int i,
    std::chrono::time_point<std::chrono::high_resolution_clock> absTime)
{
    std::this_thread::sleep_until(absTime);
    q.GetAndOneTaskDone();
}

void StartThreads(std::thread (&threads)[THREAD_NUM],
                  void (*function)(DemoThreadData&, int, std::chrono::time_point<std::chrono::high_resolution_clock>),
                  std::array<DemoThreadData, THREAD_NUM>& demoDatas,
                  std::chrono::time_point<std::chrono::high_resolution_clock> timeT)
{
    for (unsigned int i = 0; i < THREAD_NUM; i++) {
        threads[i] = std::thread(function, std::ref(demoDatas[i]), i, timeT);
    }
}

template <size_t N>
void joinAllThreads(std::thread (&threads)[N])
{
    for (auto& t : threads) {
        if (t.joinable()) {
            t.join();
        }
    }
}

void processSharedQueueTasks(DemoThreadData& data)
{
    while (!DemoThreadData::shareQueue.IsEmpty()) {
        data.GetAndOneTaskDone();
    }
}

static auto GetTimeOfSleepHundredMillisecond()
{
    using std::chrono::system_clock;
    auto timeT = std::chrono::high_resolution_clock::now();
    timeT += std::chrono::milliseconds(SLEEP_FOR_HUNDRED_MILLISECOND);
    return timeT;
}

/*
 * @tc.name: testPut001
 * @tc.desc: Single-threaded call put and get to determine that the normal scenario is working properly
 */
BENCHMARK_F(BenchmarkSafeBlockQueueTracking, testPut001)(benchmark::State& state)
{
    BENCHMARK_LOGD("SafeBlockQueueTracking testPut001 start.");
    while (state.KeepRunning()) {
        SafeBlockQueueTracking<int> qi(QUEUE_CAPACITY);
        int i = 1;
        qi.Push(i);
        AssertEqual(static_cast<unsigned>(1), qi.Size(),
            "static_cast<unsigned>(1) did not equal qi.Size() as expected.", state);
        AssertGreaterThan(qi.GetUnfinishTaskNum(), 0,
            "qi.GetUnfinishTaskNum() was not greater than 0 as expected.", state);
    }
    BENCHMARK_LOGD("SafeBlockQueueTracking testPut001 end.");
}

/*
 * @tc.name: testGet001
 * @tc.desc: Single-threaded call put and get to determine that the normal scenario is working properly
 */
BENCHMARK_F(BenchmarkSafeBlockQueueTracking, testGet001)(benchmark::State& state)
{
    BENCHMARK_LOGD("SafeBlockQueueTracking testGet001 start.");
    const int numberOfPushes = 3;
    const int expectedFirstElement = 0;
    const int minValueForComparison = 0;
    while (state.KeepRunning()) {
        SafeBlockQueueTracking<int> qi(QUEUE_CAPACITY);
        for (int i = 0; i < numberOfPushes; i++) {
            qi.Push(i);
        }
        AssertEqual(static_cast<unsigned>(numberOfPushes), qi.Size(),
            "static_cast<unsigned>(numberOfPushes) did not equal qi.Size() as expected.", state);
        int t = qi.Pop();
        AssertEqual(t, expectedFirstElement, "t did not equal expectedFirstElement as expected.", state);
        AssertGreaterThan(qi.GetUnfinishTaskNum(), minValueForComparison,
            "qi.GetUnfinishTaskNum() was not greater than minValueForComparison as expected.", state);
    }
    BENCHMARK_LOGD("SafeBlockQueueTracking testGet001 end.");
}

/*
 * @tc.name: testMutilthreadPutAndBlock001
 * @tc.desc: Multiple threads put until blocking runs, one thread gets, all threads finish running normally
 */
BENCHMARK_F(BenchmarkSafeBlockQueueTracking, testMutilthreadPutAndBlock001)(benchmark::State& state)
{
    BENCHMARK_LOGD("SafeBlockQueueTracking testMutilthreadPutAndBlock001 start.");
    const int putValue = 1;
    std::thread threads[THREAD_NUM];
    std::array<DemoThreadData, THREAD_NUM> demoDatas;
    while (state.KeepRunning()) {
        demoDatas.fill(DemoThreadData());
        demoDatas[0].Put(putValue);
        AssertFalse((demoDatas[0].joinStatus), "step 1: demoDatas[0].joinStatus did not equal false.", state);

        // start thread to join
        DemoThreadData tmp = DemoThreadData();
        std::thread joinThread = std::thread(&DemoThreadData::Join, tmp);
        AssertFalse((demoDatas[0].joinStatus), "step 2: demoDatas[0].joinStatus did not equal false.", state);

        for (unsigned int i = 0; i < THREAD_NUM; i++) {
            threads[i] = std::thread(PutHandleThreadData, std::ref(demoDatas[i]), i);
        }
        AssertFalse((demoDatas[0].joinStatus), "step 3: demoDatas[0].joinStatus did not equal false.", state);
        // 1. queue is full and some threads is blocked
        std::this_thread::sleep_for(std::chrono::milliseconds(SLEEP_FOR_HUNDRED_MILLISECOND));
        AssertTrue((DemoThreadData::shareQueue.IsFull()),
            "DemoThreadData::shareQueue.IsFull() did not equal true as expected.", state);

        AssertFalse((demoDatas[0].joinStatus), "step 4: demoDatas[0].joinStatus did not equal false.", state);
        // 2.  get one out  and wait some put in
        for (unsigned int i = 0; i < THREAD_NUM; i++) {
            demoDatas[0].GetAndOneTaskDone();
            AssertFalse((demoDatas[0].joinStatus), "step 5: demoDatas[0].joinStatus did not equal false.", state);
        }
        AssertFalse((demoDatas[0].joinStatus), "step 6: demoDatas[0].joinStatus did not equal false.", state);
        demoDatas[0].GetAndOneTaskDone();
        std::this_thread::sleep_for(std::chrono::milliseconds(SLEEP_FOR_HUNDRED_MILLISECOND));

        AssertTrue((demoDatas[0].joinStatus), "step 7: demoDatas[0].joinStatus did not equal true.", state);

        // recover state
        joinAllThreads(threads);
        joinThread.join();

        processSharedQueueTasks(demoDatas[0]);
        demoDatas[0].joinStatus = false;
    }
    BENCHMARK_LOGD("SafeBlockQueueTracking testMutilthreadPutAndBlock001 end.");
}

/*
 * @tc.name: PutAndBlockInblankqueue001
 * @tc.desc: Multi-threaded put() on the empty queue. When n threads are waiting to reach a certain
 * time-point, everyone puts concurrent to see the status of the queue and the state of the thread.
 */
BENCHMARK_F(BenchmarkSafeBlockQueueTracking, PutAndBlockInblankqueue001)(benchmark::State& state)
{
    BENCHMARK_LOGD("SafeBlockQueueTracking PutAndBlockInblankqueue001 start.");
    std::thread threads[THREAD_NUM];
    std::array<DemoThreadData, THREAD_NUM> demoDatas;
    while (state.KeepRunning()) {
        demoDatas.fill(DemoThreadData());
        auto timeT = GetTimeOfSleepHundredMillisecond();
        AssertTrue((DemoThreadData::shareQueue.IsEmpty()),
            "DemoThreadData::shareQueue.IsEmpty() did not equal true as expected.", state);
        StartThreads(threads, PutHandleThreadDataTime, demoDatas, timeT);
        std::this_thread::sleep_for(std::chrono::milliseconds(SLEEP_FOR_HUNDRED_MILLISECOND));
        DemoThreadData tmp = DemoThreadData();
        std::thread joinThread = std::thread(&DemoThreadData::Join, tmp);
        AssertFalse((demoDatas[0].joinStatus), "step 1: demoDatas[0].joinStatus did not equal false.", state);
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
        for (unsigned int i = 0; i < THREAD_NUM - QUEUE_SLOTS; i++) {
            demoDatas[0].GetAndOneTaskDone();
            AssertFalse((demoDatas[0].joinStatus), "step 2: demoDatas[0].joinStatus did not equal false.", state);
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(SLEEP_FOR_HUNDRED_MILLISECOND));
        AssertTrue((DemoThreadData::shareQueue.IsFull()),
            "DemoThreadData::shareQueue.IsFull() did not equal true as expected.", state);
        GetThreadDatePushedStatus(demoDatas, pushedIn, unpushedIn);
        GetThreadDateGetedStatus(demoDatas, getedOut, ungetedOut);
        AssertEqual(pushedIn, THREAD_NUM, "pushedIn did not equal THREAD_NUM as expected.", state);
        AssertEqual(getedOut, THREAD_NUM - QUEUE_SLOTS,
            "getedOut did not equal THREAD_NUM - QUEUE_SLOTS as expected.", state);
        joinAllThreads(threads);
        processSharedQueueTasks(demoDatas[0]);
        std::this_thread::sleep_for(std::chrono::milliseconds(SLEEP_FOR_HUNDRED_MILLISECOND));
        AssertTrue((demoDatas[0].joinStatus), "step 3: demoDatas[0].joinStatus did not equal true.", state);
        demoDatas[0].joinStatus = false;
        joinThread.join();
    }
    BENCHMARK_LOGD("SafeBlockQueueTracking PutAndBlockInblankqueue001 end.");
}

/*
 * @tc.name: testPutAndBlockInFullQueue001
 * @tc.desc: Multi-threaded put() on the full queue. When n threads are waiting to reach a certain
 * time-point, everyone puts concurrent to see the status of the queue and the state of the thread.
 */
BENCHMARK_F(BenchmarkSafeBlockQueueTracking, testPutAndBlockInFullQueue001)(benchmark::State& state)
{
    BENCHMARK_LOGD("SafeBlockQueueTracking testPutAndBlockInFullQueue001 start.");
    std::thread threads[THREAD_NUM];
    std::array<DemoThreadData, THREAD_NUM> demoDatas;
    while (state.KeepRunning()) {
        demoDatas.fill(DemoThreadData());
        auto timeT = GetTimeOfSleepHundredMillisecond();
        AssertTrue((DemoThreadData::shareQueue.IsEmpty()),
            "DemoThreadData::shareQueue.IsEmpty() did not equal true as expected.", state);
        for (unsigned int i = 0; i < QUEUE_SLOTS; i++) {
            int t = i;
            DemoThreadData::shareQueue.Push(t);
        }
        AssertTrue((DemoThreadData::shareQueue.IsFull()), "shareQueue.IsFull() did not equal true.", state);
        StartThreads(threads, PutHandleThreadDataTime, demoDatas, timeT);
        std::this_thread::sleep_for(std::chrono::milliseconds(SLEEP_FOR_HUNDRED_MILLISECOND));
        DemoThreadData tmp = DemoThreadData();
        std::thread joinThread = std::thread(&DemoThreadData::Join, tmp);
        AssertFalse((demoDatas[0].joinStatus), "demoDatas[0].joinStatus did not equal false as expected.", state);
        unsigned int pushedIn = 0;
        unsigned int unpushedIn = 0;
        GetThreadDatePushedStatus(demoDatas, pushedIn, unpushedIn);
        AssertEqual(pushedIn, static_cast<unsigned int>(0),
            "pushedIn did not equal static_cast<unsigned int>(0) as expected.", state);
        AssertEqual(unpushedIn, THREAD_NUM, "unpushedIn did not equal THREAD_NUM as expected.", state);
        AssertTrue((DemoThreadData::shareQueue.IsFull()),
            "DemoThreadData::shareQueue.IsFull() did not equal true as expected.", state);
        for (unsigned int i = 0; i < THREAD_NUM; i++) {
            DemoThreadData::shareQueue.Pop();
            DemoThreadData::shareQueue.OneTaskDone();
            std::this_thread::sleep_for(std::chrono::milliseconds(SLEEP_FOR_HUNDRED_MILLISECOND));
            AssertTrue((DemoThreadData::shareQueue.IsFull()), "shareQueue.IsFull() did not equal true.", state);
                GetThreadDatePushedStatus(demoDatas, pushedIn, unpushedIn);
            AssertEqual(pushedIn, i + 1, "pushedIn did not equal i + 1 as expected.", state);
            AssertEqual(unpushedIn, THREAD_NUM - (i + 1),
                "unpushedIn did not equal THREAD_NUM - (i + 1) as expected.", state);
        }
        joinAllThreads(threads);
        processSharedQueueTasks(demoDatas[0]);
        demoDatas[0].joinStatus = false;
        joinThread.join();
    }
    BENCHMARK_LOGD("SafeBlockQueueTracking testPutAndBlockInFullQueue001 end.");
}

/*
 * @tc.name: GetAndBlockInblankqueue001
 * @tc.desc: Multi-threaded get() on the empty queue. When n threads are waiting to reach a certain
 * time-point, everyone gets concurrent to see the status of the queue and the state of the thread.
 */
BENCHMARK_F(BenchmarkSafeBlockQueueTracking, GetAndBlockInblankqueue001)(benchmark::State& state)
{
    BENCHMARK_LOGD("SafeBlockQueueTracking GetAndBlockInblankqueue001 start.");
    std::thread threads[THREAD_NUM];
    std::array<DemoThreadData, THREAD_NUM> demoDatas;
    while (state.KeepRunning()) {
        demoDatas.fill(DemoThreadData());
        auto timeT = GetTimeOfSleepHundredMillisecond();
        AssertTrue((DemoThreadData::shareQueue.IsEmpty()),
            "DemoThreadData::shareQueue.IsEmpty() did not equal true as expected.", state);
        StartThreads(threads, GetAndOneTaskDoneHandleThreadDataTime, demoDatas, timeT);
        std::this_thread::sleep_for(std::chrono::milliseconds(SLEEP_FOR_HUNDRED_MILLISECOND));
        unsigned int getedOut = 0;
        unsigned int ungetedOut = 0;
        GetThreadDateGetedStatus(demoDatas, getedOut, ungetedOut);
        AssertEqual(getedOut, static_cast<unsigned int>(0),
            "getedOut did not equal static_cast<unsigned int>(0) as expected.", state);
        AssertEqual(ungetedOut, THREAD_NUM, "ungetedOut did not equal THREAD_NUM as expected.", state);
        AssertTrue((DemoThreadData::shareQueue.IsEmpty()),
            "DemoThreadData::shareQueue.IsEmpty() did not equal true as expected.", state);
        DemoThreadData tmp = DemoThreadData();
        std::thread joinThread = std::thread(&DemoThreadData::Join, tmp);
        AssertFalse((demoDatas[0].joinStatus), "demoDatas[0].joinStatus did not equal false as expected.", state);
        int value = 1;
        for (unsigned int i = 0; i < THREAD_NUM; i++) {
            DemoThreadData::shareQueue.Push(value);
            std::this_thread::sleep_for(std::chrono::milliseconds(SLEEP_FOR_HUNDRED_MILLISECOND));
            AssertTrue((DemoThreadData::shareQueue.IsEmpty()),
                "DemoThreadData::shareQueue.IsEmpty() did not equal true as expected.", state);
                GetThreadDateGetedStatus(demoDatas, getedOut, ungetedOut);
            AssertEqual(getedOut, i + 1, "getedOut did not equal i + 1 as expected.", state);
            AssertEqual(ungetedOut, THREAD_NUM - (i + 1),
                "ungetedOut did not equal THREAD_NUM - (i + 1) as expected.", state);
        }
        joinAllThreads(threads);
        processSharedQueueTasks(demoDatas[0]);
        AssertTrue((demoDatas[0].joinStatus), "demoDatas[0].joinStatus did not equal true as expected.", state);
        demoDatas[0].joinStatus = false;
        joinThread.join();
    }
    BENCHMARK_LOGD("SafeBlockQueueTracking GetAndBlockInblankqueue001 end.");
}

static void QueuePushFullEquivalent(const int Equivalent, benchmark::State& state)
{
    for (unsigned int i = 0; i < QUEUE_SLOTS; i++) {
        DemoThreadData::shareQueue.Push(Equivalent);
    }
    AssertTrue((DemoThreadData::shareQueue.IsFull()), "shareQueue.IsFull() did not equal true.", state);
}

static void QueuePushFullNotEquivalent(const unsigned int remain)
{
    for (unsigned int i = 0; i < QUEUE_SLOTS - remain; i++) {
        int t = i;
        DemoThreadData::shareQueue.Push(t);
    }
}

/*
 * @tc.name: GetAndBlockInfullqueue001
 * @tc.desc: Multi-threaded get() on the full queue. When n threads are waiting to reach a certain
 * time-point, everyone gets concurrent to see the status of the queue and the state of the thread.
 */
BENCHMARK_F(BenchmarkSafeBlockQueueTracking, GetAndBlockInfullqueue001)(benchmark::State& state)
{
    BENCHMARK_LOGD("SafeBlockQueueTracking GetAndBlockInfullqueue001 start.");
    std::thread threads[THREAD_NUM];
    std::array<DemoThreadData, THREAD_NUM> demoDatas;
    while (state.KeepRunning()) {
        demoDatas.fill(DemoThreadData());
        auto timeT = GetTimeOfSleepHundredMillisecond();
        AssertTrue((DemoThreadData::shareQueue.IsEmpty()), "shareQueue.IsEmpty() did not equal true.", state);
        int t = 1;
        QueuePushFullEquivalent(t, state);
        demoDatas[0].joinStatus = false;
        DemoThreadData tmp = DemoThreadData();
        std::thread joinThread = std::thread(&DemoThreadData::Join, tmp);
        AssertFalse((demoDatas[0].joinStatus), "step 1: demoDatas[0].joinStatus did not equal false.", state);
        StartThreads(threads, GetAndOneTaskDoneHandleThreadDataTime, demoDatas, timeT);
        std::this_thread::sleep_for(std::chrono::milliseconds(SLEEP_FOR_HUNDRED_MILLISECOND));
        AssertTrue((DemoThreadData::shareQueue.IsEmpty()), "shareQueue.IsEmpty() did not equal true.", state);
        AssertTrue((demoDatas[0].joinStatus), "step 2: demoDatas[0].joinStatus did not equal true.", state);
        unsigned int getedOut = 0;
        unsigned int ungetedOut = 0;
        GetThreadDateGetedStatus(demoDatas, getedOut, ungetedOut);
        AssertEqual(getedOut, QUEUE_SLOTS, "getedOut did not equal QUEUE_SLOTS as expected.", state);
        AssertEqual(ungetedOut, THREAD_NUM - QUEUE_SLOTS, "ungetedOut did not equal THREAD_NUM - QUEUE_SLOTS", state);
        for (unsigned int i = 0; i < THREAD_NUM - QUEUE_SLOTS; i++) {
            demoDatas[0].Put(t);
        }
        demoDatas[0].joinStatus = false;
        DemoThreadData tmp2 = DemoThreadData();
        std::thread joinThread2 = std::thread(&DemoThreadData::Join, tmp2);
        std::this_thread::sleep_for(std::chrono::milliseconds(SLEEP_FOR_HUNDRED_MILLISECOND));
        AssertTrue((DemoThreadData::shareQueue.IsEmpty()), "shareQueue.IsEmpty() did not equal true.", state);
        AssertTrue((demoDatas[0].joinStatus), "step 3: demoDatas[0].joinStatus did not equal true.", state);
        GetThreadDateGetedStatus(demoDatas, getedOut, ungetedOut);
        AssertEqual(getedOut, THREAD_NUM, "getedOut did not equal THREAD_NUM as expected.", state);
        AssertEqual(ungetedOut, static_cast<unsigned int>(0),
            "ungetedOut did not equal static_cast<unsigned int>(0) as expected.", state);
        joinAllThreads(threads);
        processSharedQueueTasks(demoDatas[0]);
        demoDatas[0].joinStatus = false;
        joinThread.join();
        joinThread2.join();
    }
    BENCHMARK_LOGD("SafeBlockQueueTracking GetAndBlockInfullqueue001 end.");
}

/*
 * @tc.name: GetAndBlockInnotfullqueue001
 * @tc.desc: Multi-threaded get() on the notfull queue. When n threads are waiting to reach a certain
 * time-point, everyone get concurrent to see the status of the queue and the state of the thread.
 */
BENCHMARK_F(BenchmarkSafeBlockQueueTracking, GetAndBlockInnotfullqueue001)(benchmark::State& state)
{
    BENCHMARK_LOGD("SafeBlockQueueTracking GetAndBlockInnotfullqueue001 start.");
    std::thread threads[THREAD_NUM];
    std::array<DemoThreadData, THREAD_NUM> demoDatas;
    while (state.KeepRunning()) {
        demoDatas.fill(DemoThreadData());
        auto timeT = GetTimeOfSleepHundredMillisecond();
        AssertTrue((DemoThreadData::shareQueue.IsEmpty()),
            "DemoThreadData::shareQueue.IsEmpty() did not equal true as expected.", state);
        const unsigned int REMAIN_SLOTS = 3;
        QueuePushFullNotEquivalent(REMAIN_SLOTS);
        demoDatas[0].joinStatus = false;
        DemoThreadData tmp = DemoThreadData();
        std::thread joinThread = std::thread(&DemoThreadData::Join, tmp);
        AssertFalse((demoDatas[0].joinStatus), "step 1: demoDatas[0].joinStatus did not equal false.", state);
        StartThreads(threads, GetAndOneTaskDoneHandleThreadDataTime, demoDatas, timeT);
        std::this_thread::sleep_for(std::chrono::milliseconds(SLEEP_FOR_HUNDRED_MILLISECOND));
        unsigned int getedOut = 0;
        unsigned int ungetedOut = 0;
        GetThreadDateGetedStatus(demoDatas, getedOut, ungetedOut);
        AssertEqual(getedOut, QUEUE_SLOTS - REMAIN_SLOTS, "getedOut did not equal QUEUE_SLOTS - REMAIN_SLOTS.", state);
        AssertEqual(ungetedOut, THREAD_NUM - (QUEUE_SLOTS - REMAIN_SLOTS),
            "ungetedOut did not equal THREAD_NUM - (QUEUE_SLOTS - REMAIN_SLOTS) as expected.", state);
        AssertTrue((demoDatas[0].joinStatus), "step 2: demoDatas[0].joinStatus did not equal true.", state);
        for (unsigned int i = 0; i < ungetedOut; i++) {
            int t = i;
            DemoThreadData::shareQueue.Push(t);
        }
        demoDatas[0].joinStatus = false;
        DemoThreadData tmp2 = DemoThreadData();
        std::thread joinThread2 = std::thread(&DemoThreadData::Join, tmp2);
        std::this_thread::sleep_for(std::chrono::milliseconds(SLEEP_FOR_HUNDRED_MILLISECOND));
        GetThreadDateGetedStatus(demoDatas, getedOut, ungetedOut);
        AssertEqual(getedOut, THREAD_NUM, "getedOut did not equal THREAD_NUM as expected.", state);
        AssertEqual(ungetedOut, static_cast<unsigned int>(0),
            "ungetedOut did not equal static_cast<unsigned int>(0) as expected.", state);
        joinAllThreads(threads);
        AssertTrue((demoDatas[0].joinStatus), "step 3: demoDatas[0].joinStatus did not equal true.", state);
        demoDatas[0].joinStatus = false;
        joinThread.join();
        joinThread2.join();
    }
    BENCHMARK_LOGD("SafeBlockQueueTracking GetAndBlockInnotfullqueue001 end.");
}

/*
 * @tc.name: PutAndBlockInnotfullqueue001
 * @tc.desc: Multi-threaded put() on the not full queue. When n threads are waiting to reach a certain
 * time-point, everyone puts concurrent to see the status of the queue and the state of the thread.
 */
BENCHMARK_F(BenchmarkSafeBlockQueueTracking, PutAndBlockInnotfullqueue001)(benchmark::State& state)
{
    BENCHMARK_LOGD("SafeBlockQueueTracking PutAndBlockInnotfullqueue001 start.");
    std::thread threads[THREAD_NUM];
    std::array<DemoThreadData, THREAD_NUM> demoDatas;
    while (state.KeepRunning()) {
        demoDatas.fill(DemoThreadData());
        auto timeT = GetTimeOfSleepHundredMillisecond();
        const unsigned int REMAIN_SLOTS = 3;
        AssertTrue((DemoThreadData::shareQueue.IsEmpty()),
            "DemoThreadData::shareQueue.IsEmpty() did not equal true as expected.", state);
        QueuePushFullNotEquivalent(REMAIN_SLOTS);
        demoDatas[0].joinStatus = false;
        DemoThreadData tmp = DemoThreadData();
        std::thread joinThread = std::thread(&DemoThreadData::Join, tmp);
        AssertFalse((demoDatas[0].joinStatus), "step 1: demoDatas[0].joinStatus did not equal false.", state);
        StartThreads(threads, PutHandleThreadDataTime, demoDatas, timeT);
        AssertFalse((demoDatas[0].joinStatus), "step 2: demoDatas[0].joinStatus did not equal false.", state);
        std::this_thread::sleep_for(std::chrono::milliseconds(SLEEP_FOR_HUNDRED_MILLISECOND));
        AssertFalse((demoDatas[0].joinStatus), "step 3: demoDatas[0].joinStatus did not equal false.", state);
        unsigned int putedin = 0;
        unsigned int unputedin = 0;
        GetThreadDatePushedStatus(demoDatas, putedin, unputedin);
        AssertEqual(putedin, REMAIN_SLOTS, "putedin did not equal REMAIN_SLOTS as expected.", state);
        AssertEqual(unputedin, THREAD_NUM - REMAIN_SLOTS,
            "unputedin did not equal THREAD_NUM - REMAIN_SLOTS as expected.", state);
        for (unsigned int i = 0; i < unputedin; i++) {
            DemoThreadData::shareQueue.Pop();
            DemoThreadData::shareQueue.OneTaskDone();
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(SLEEP_FOR_HUNDRED_MILLISECOND));
        GetThreadDatePushedStatus(demoDatas, putedin, unputedin);
        AssertEqual(putedin, THREAD_NUM, "putedin did not equal THREAD_NUM as expected.", state);
        AssertEqual(unputedin, static_cast<unsigned int>(0),
            "unputedin did not equal static_cast<unsigned int>(0) as expected.", state);
        AssertFalse((demoDatas[0].joinStatus), "step 4: demoDatas[0].joinStatus did not equal false.", state);
        joinAllThreads(threads);
        processSharedQueueTasks(demoDatas[0]);
        std::this_thread::sleep_for(std::chrono::milliseconds(SLEEP_FOR_HUNDRED_MILLISECOND));
        AssertTrue((demoDatas[0].joinStatus), "step 5: demoDatas[0].joinStatus did not equal true.", state);
        joinThread.join();
        demoDatas[0].joinStatus = false;
    }
    BENCHMARK_LOGD("SafeBlockQueueTracking PutAndBlockInnotfullqueue001 end.");
}

/*
 * @tc.name: testMutilthreadConcurrentGetAndPopInfullqueue001
 * @tc.desc: Multi-threaded put() and Multi-threaded get() on the full queue. When all threads are waiting to reach a
 * certain time-point, everyone run concurrently to see the status of the queue and the state of the thread.
 */
BENCHMARK_F(BenchmarkSafeBlockQueueTracking, testMutilthreadConcurrentGetAndPopInfullqueue001)(benchmark::State& state)
{
    BENCHMARK_LOGD("SafeBlockQueueTracking testMutilthreadConcurrentGetAndPopInfullqueue001 start.");
    std::thread threadsout[THREAD_NUM];
    std::array<DemoThreadData, THREAD_NUM> demoDatas;
    while (state.KeepRunning()) {
        demoDatas.fill(DemoThreadData());
        std::thread threadsin[THREAD_NUM];
        auto timeT = GetTimeOfSleepHundredMillisecond();
        AssertTrue((DemoThreadData::shareQueue.IsEmpty()), "shareQueue.IsEmpty() did not equal true.", state);
        int t = 1;
        QueuePushFullEquivalent(t, state);
        demoDatas[0].joinStatus = false;
        DemoThreadData tmp = DemoThreadData();
        std::thread joinThread = std::thread(&DemoThreadData::Join, tmp);
        AssertFalse((demoDatas[0].joinStatus), "step 1: demoDatas[0].joinStatus did not equal false.", state);
        StartThreads(threadsin, PutHandleThreadDataTime, demoDatas, timeT);
        StartThreads(threadsout, GetAndOneTaskDoneHandleThreadDataTime, demoDatas, timeT);
        std::this_thread::sleep_for(std::chrono::milliseconds(SLEEP_FOR_HUNDRED_MILLISECOND));
        AssertTrue((DemoThreadData::shareQueue.IsFull()), "shareQueue.IsFull() did not equal true.", state);
        unsigned int getedOut = 0;
        unsigned int ungetedOut = 0;
        unsigned int pushedIn = 0;
        unsigned int unpushedIn = 0;
        GetThreadDateGetedStatus(demoDatas, getedOut, ungetedOut);
        GetThreadDatePushedStatus(demoDatas, pushedIn, unpushedIn);
        AssertEqual(pushedIn, THREAD_NUM, "pushedIn did not equal THREAD_NUM as expected.", state);
        AssertEqual(getedOut, THREAD_NUM, "getedOut did not equal THREAD_NUM as expected.", state);
        AssertFalse((demoDatas[0].joinStatus), "step 2: demoDatas[0].joinStatus did not equal false.", state);
        demoDatas[0].joinStatus = false;
        for (auto& outThread : threadsout) {
            outThread.join();
        }
        for (auto& inThread : threadsin) {
            inThread.join();
        }
        processSharedQueueTasks(demoDatas[0]);
        joinThread.join();
        AssertTrue((demoDatas[0].joinStatus), "step 3: demoDatas[0].joinStatus did not equal true.", state);
        demoDatas[0].joinStatus = false;
    }
    BENCHMARK_LOGD("SafeBlockQueueTracking testMutilthreadConcurrentGetAndPopInfullqueue001 end.");
}

/*
 * @tc.name: testPushNoWait001
 * @tc.desc: Single-threaded call PushNoWait of SafeBlockQueueTracking and check the performance
 */
BENCHMARK_F(BenchmarkSafeBlockQueueTracking, testPushNoWait001)(benchmark::State& state)
{
    BENCHMARK_LOGD("SafeBlockQueueTracking testPushNoWait001 start.");
    const int pushAttempts = 10;
    while (state.KeepRunning()) {
        SafeBlockQueueTracking<int> qi(QUEUE_CAPACITY);
        for (int i = 0; i < pushAttempts; i++) {
            bool result = qi.PushNoWait(i);
            AssertTrue(result, "PushNoWait returned false, expected true.", state);
        }
    }
    BENCHMARK_LOGD("SafeBlockQueueTracking testPushNoWait001 end.");
}
}  // namespace
}  // namespace OHOS
// Run the benchmark
BENCHMARK_MAIN();