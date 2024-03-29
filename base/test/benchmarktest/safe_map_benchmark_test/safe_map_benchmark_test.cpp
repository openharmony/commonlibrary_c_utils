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
#include "safe_map.h"
#include <array>
#include <future>
#include <iostream>
#include <thread>
#include <chrono>
#include "benchmark_log.h"
#include "benchmark_assert.h"
using namespace std;
using std::chrono::system_clock;

namespace OHOS {
namespace {

class BenchmarkSafeMap : public benchmark::Fixture {
public:
    void SetUp(const ::benchmark::State& state) override
    {
    }

    void TearDown(const ::benchmark::State& state) override
    {
    }

    BenchmarkSafeMap()
    {
        Iterations(iterations);
        Repetitions(repetitions);
        ReportAggregatesOnly();
    }

    ~BenchmarkSafeMap() override = default;

protected:
    const int32_t repetitions = 3;
    const int32_t iterations = 50;
};

const int INSERT_ONE = 1;
const int INSERT_TWO = 2;
const int INSERT_THREE = 3;
const int INSERT_FOUR = 4;
const int INSERT_FIVE = 5;
const int INSERT_SIX = 6;

/*
 * @tc.name: testUtilsCopyAndAssign001
 * @tc.desc: single thread test the normal feature insert and erase and EnsureInsert
 */
BENCHMARK_F(BenchmarkSafeMap, testUtilsCopyAndAssign001)(benchmark::State& state)
{
    BENCHMARK_LOGD("SafeMap testUtilsCopyAndAssign001 start.");
    while (state.KeepRunning()) {
        SafeMap<string, int> demoData;
        // insert new
        demoData.Insert("A", INSERT_ONE);
        AssertFalse(demoData.IsEmpty(), "demoData.IsEmpty() did not equal false as expected.", state);
        AssertEqual(demoData.Size(), INSERT_ONE, "demoData.Size() did not equal INSERT_ONE as expected.", state);

        SafeMap<string, int> newdemo = demoData;
        int tar = -1;
        AssertTrue(newdemo.Find("A", tar), "newdemo.Find(\"A\", tar) did not equal true as expected.", state);
        AssertEqual(INSERT_ONE, tar, "INSERT_ONE did not equal tar as expected.", state);

        tar = -1;
        SafeMap<string, int> newdemo2;
        newdemo2 = demoData;
        AssertTrue(newdemo2.Find("A", tar), "newdemo2.Find(\"A\", tar) did not equal true as expected.", state);
        AssertEqual(INSERT_ONE, tar, "INSERT_ONE did not equal tar as expected.", state);
    }
    BENCHMARK_LOGD("SafeMap testUtilsCopyAndAssign001 end.");
}

/*
 * @tc.name: testUtilsoperator001
 * @tc.desc: SafeMap
 */
BENCHMARK_F(BenchmarkSafeMap, testUtilsoperator001)(benchmark::State& state)
{
    BENCHMARK_LOGD("SafeMap testUtilsoperator001 start.");
    while (state.KeepRunning()) {
        SafeMap<string, int> demoData;
        // insert new
        demoData.Insert("A", INSERT_ONE);
        AssertFalse(demoData.IsEmpty(), "demoData.IsEmpty() did not equal false as expected.", state);
        AssertEqual(demoData.Size(), INSERT_ONE, "demoData.Size() did not equal INSERT_ONE as expected.", state);

        int valueA;
        bool foundA = demoData.Find("A", valueA);
        AssertTrue(foundA, "demoData.Find(\"A\", valueA) did not return true as expected.", state);
        AssertEqual(valueA, INSERT_ONE, "Value retrieved did not equal INSERT_ONE as expected.", state);

        SafeMap<string, int> newdemo = demoData;
        int valueNewDemoA;
        bool foundNewDemoA = newdemo.Find("A", valueNewDemoA);
        AssertTrue(foundNewDemoA, "newdemo.Find(\"A\", valueNewDemoA) did not return true as expected.", state);
        AssertEqual(valueNewDemoA, INSERT_ONE, "Value retrieved did not equal INSERT_ONE as expected.", state);

        int tar = -1;
        newdemo.Insert("B", INSERT_SIX);
        bool foundB = newdemo.Find("B", tar);
        AssertTrue(foundB, "newdemo.Find(\"B\", tar) did not return true as expected.", state);
        AssertEqual(INSERT_SIX, tar, "INSERT_SIX did not equal tar as expected.", state);

        SafeMap<string, int> newdemo2;
        newdemo2 = newdemo;
        int valueNewDemo2A;
        bool foundNewDemo2A = newdemo2.Find("A", valueNewDemo2A);
        AssertTrue(foundNewDemo2A, "newdemo2.Find(\"A\", valueNewDemo2A) did not return true as expected.", state);
        AssertEqual(valueNewDemo2A, INSERT_ONE, "Value retrieved did not equal INSERT_ONE as expected.", state);
    }
    BENCHMARK_LOGD("SafeMap testUtilsoperator001 end.");
}

/*
 * @tc.name: testUtilsNormalFeatureInsert001
 * @tc.desc: SafeMap
 */
BENCHMARK_F(BenchmarkSafeMap, testUtilsNormalFeatureInsert001)(benchmark::State& state)
{
    BENCHMARK_LOGD("SafeMap testUtilsNormalFeatureInsert001 start.");
    while (state.KeepRunning()) {
        SafeMap<string, int> demoData;
        AssertTrue(demoData.IsEmpty(), "demoData.IsEmpty() did not equal true as expected.", state);

        // insert new
        demoData.Insert("A", INSERT_ONE);
        AssertFalse(demoData.IsEmpty(), "demoData.IsEmpty() did not equal false as expected.", state);
        AssertEqual(demoData.Size(), INSERT_ONE, "demoData.Size() did not equal INSERT_ONE as expected.", state);

        // insert copy one should fail
        AssertFalse(demoData.Insert("A", INSERT_TWO),
            "demoData.Insert(\"A\", INSERT_TWO) did not equal false as expected.", state);
        AssertEqual(demoData.Size(), INSERT_ONE, "demoData.Size() did not equal INSERT_ONE as expected.", state);
    }
    BENCHMARK_LOGD("SafeMap testUtilsNormalFeatureInsert001 end.");
}

/*
 * @tc.name: testUtilsNormalFeatureEnsureInsert001
 * @tc.desc: SafeMap
 */
BENCHMARK_F(BenchmarkSafeMap, testUtilsNormalFeatureEnsureInsert001)(benchmark::State& state)
{
    BENCHMARK_LOGD("SafeMap testUtilsNormalFeatureEnsureInsert001 start.");
    while (state.KeepRunning()) {
        SafeMap<string, int> demoData;
        AssertTrue(demoData.IsEmpty(), "demoData.IsEmpty() did not equal true as expected.", state);

        demoData.Insert("A", INSERT_ONE);
        demoData.EnsureInsert("B", INSERT_TWO);

        AssertFalse(demoData.IsEmpty(), "demoData.IsEmpty() did not equal false as expected.", state);
        AssertEqual(demoData.Size(), INSERT_TWO, "demoData.Size() did not equal INSERT_TWO as expected.", state);

        // insert copy one and new one
        demoData.EnsureInsert("B", INSERT_FIVE);
        demoData.EnsureInsert("C", INSERT_SIX);
        AssertEqual(demoData.Size(), INSERT_THREE, "demoData.Size() did not equal INSERT_THREE as expected.", state);
    }
    BENCHMARK_LOGD("SafeMap testUtilsNormalFeatureEnsureInsert001 end.");
}

/*
 * @tc.name: testUtilsNormalFeatureFind001
 * @tc.desc: SafeMap
 */
BENCHMARK_F(BenchmarkSafeMap, testUtilsNormalFeatureFind001)(benchmark::State& state)
{
    BENCHMARK_LOGD("SafeMap testUtilsNormalFeatureFind001 start.");
    while (state.KeepRunning()) {
        SafeMap<string, int> demoData;
        AssertTrue(demoData.IsEmpty(), "demoData.IsEmpty() did not equal true as expected.", state);

        demoData.Insert("A", INSERT_ONE);
        demoData.Insert("B", 10000);
        demoData.EnsureInsert("B", INSERT_TWO);
        demoData.EnsureInsert("C", INSERT_SIX);

        AssertFalse(demoData.IsEmpty(), "demoData.IsEmpty() did not equal false as expected.", state);
        AssertEqual(demoData.Size(), INSERT_THREE, "demoData.Size() did not equal INSERT_THREE as expected.", state);

        int i = 0;
        AssertTrue(demoData.Find("A", i), "demoData.Find(\"A\", i) did not equal true as expected.", state);
        AssertEqual(i, INSERT_ONE, "i did not equal INSERT_ONE as expected.", state);
        AssertTrue(demoData.Find("B", i), "demoData.Find(\"B\", i) did not equal true as expected.", state);
        AssertEqual(i, INSERT_TWO, "i did not equal INSERT_TWO as expected.", state);

        AssertTrue(demoData.Find("C", i), "demoData.Find(\"C\", i) did not equal true as expected.", state);
        AssertEqual(i, INSERT_SIX, "i did not equal INSERT_SIX as expected.", state);
    }
    BENCHMARK_LOGD("SafeMap testUtilsNormalFeatureFind001 end.");
}

/*
 * @tc.name: testUtilsNormalFeatureFindAndSet001
 * @tc.desc: SafeMap
 */
BENCHMARK_F(BenchmarkSafeMap, testUtilsNormalFeatureFindAndSet001)(benchmark::State& state)
{
    BENCHMARK_LOGD("SafeMap testUtilsNormalFeatureFindAndSet001 start.");
    while (state.KeepRunning()) {
        SafeMap<string, int> demoData;
        AssertTrue(demoData.IsEmpty(), "demoData.IsEmpty() did not equal true as expected.", state);

        demoData.Insert("A", INSERT_ONE);
        demoData.EnsureInsert("B", INSERT_TWO);

        int oldvalue = 0;
        int newvalue = 3;
        AssertTrue(demoData.FindOldAndSetNew("A", oldvalue, newvalue),
            "demoData.FindOldAndSetNew(\"A\", oldvalue, newvalue) did not equal true as expected.", state);

        // old value
        AssertEqual(oldvalue, INSERT_ONE, "oldvalue did not equal INSERT_ONE as expected.", state);

        newvalue = 4;
        AssertTrue(demoData.FindOldAndSetNew("B", oldvalue, newvalue),
            "demoData.FindOldAndSetNew(\"B\", oldvalue, newvalue) did not equal true as expected.", state);

        // old value
        AssertEqual(oldvalue, INSERT_TWO, "oldvalue did not equal INSERT_TWO as expected.", state);

        int i = -1;
        AssertTrue(demoData.Find("A", i), "demoData.Find(\"A\", i) did not equal true as expected.", state);

        // new value
        AssertEqual(i, INSERT_THREE, "i did not equal INSERT_THREE as expected.", state);
        AssertTrue(demoData.Find("B", i), "demoData.Find(\"B\", i) did not equal true as expected.", state);

        // new value
        AssertEqual(i, INSERT_FOUR, "i did not equal INSERT_FOUR as expected.", state);
    }
    BENCHMARK_LOGD("SafeMap testUtilsNormalFeatureFindAndSet001 end.");
}

/*
 * @tc.name: testUtilsNormalFeatureEraseAndClear001
 * @tc.desc: SafeMap
 */
BENCHMARK_F(BenchmarkSafeMap, testUtilsNormalFeatureEraseAndClear001)(benchmark::State& state)
{
    BENCHMARK_LOGD("SafeMap testUtilsNormalFeatureEraseAndClear001 start.");
    while (state.KeepRunning()) {
        SafeMap<string, int> demoData;
        AssertTrue(demoData.IsEmpty(), "demoData.IsEmpty() did not equal true as expected.", state);

        demoData.Insert("A", INSERT_ONE);
        demoData.EnsureInsert("B", INSERT_TWO);

        AssertEqual(demoData.Size(), INSERT_TWO, "demoData.Size() did not equal INSERT_TWO as expected.", state);
        demoData.Erase("A");
        AssertEqual(demoData.Size(), INSERT_ONE, "demoData.Size() did not equal INSERT_ONE as expected.", state);

        demoData.Clear();
        AssertEqual(demoData.Size(), 0, "demoData.Size() did not equal 0 as expected.", state);
    }
    BENCHMARK_LOGD("SafeMap testUtilsNormalFeatureEraseAndClear001 end.");
}

/*
 * @tc.name: testUtilsNormalFeatureIterate001
 * @tc.desc: Using Iterate to change the second parameter of SafeMap
 */
void callback(const std::string str, int& value)
{
    value++;
}

BENCHMARK_F(BenchmarkSafeMap, testUtilsNormalFeatureIterate001)(benchmark::State& state)
{
    BENCHMARK_LOGD("SafeMap testUtilsNormalFeatureIterate001 start.");
    while (state.KeepRunning()) {
        SafeMap<string, int> demoData;
        AssertTrue(demoData.IsEmpty(), "demoData.IsEmpty() did not equal true as expected.", state);

        demoData.Insert("A", INSERT_ONE);
        demoData.Insert("B", INSERT_TWO);
        demoData.Insert("C", INSERT_THREE);
        demoData.Insert("D", INSERT_FOUR);
        demoData.Iterate(callback);

        AssertEqual(demoData.Size(), INSERT_FOUR, "demoData.Size() did not equal INSERT_FOUR as expected.", state);

        int valueA;
        bool foundA = demoData.Find("A", valueA);
        AssertTrue(foundA, "Key \"A\" was not found as expected.", state);
        AssertEqual(valueA, INSERT_TWO, "Value for key \"A\" did not equal INSERT_TWO as expected.", state);

        int valueB;
        bool foundB = demoData.Find("B", valueB);
        AssertTrue(foundB, "Key \"B\" was not found as expected.", state);
        AssertEqual(valueB, INSERT_THREE, "Value for key \"B\" did not equal INSERT_THREE as expected.", state);

        int valueC;
        bool foundC = demoData.Find("C", valueC);
        AssertTrue(foundC, "Key \"C\" was not found as expected.", state);
        AssertEqual(valueC, INSERT_FOUR, "Value for key \"C\" did not equal INSERT_FOUR as expected.", state);

        int valueD;
        bool foundD = demoData.Find("D", valueD);
        AssertTrue(foundD, "Key \"D\" was not found as expected.", state);
        AssertEqual(valueD, INSERT_FIVE, "Value for key \"D\" did not equal INSERT_FIVE as expected.", state);
    }
    BENCHMARK_LOGD("SafeMap testUtilsNormalFeatureIterate001 end.");
}

/*
 * @tc.name: testSafeMapConstructor001
 * @tc.desc: SafeMapConstructor
 */
BENCHMARK_F(BenchmarkSafeMap, testSafeMapConstructor001)(benchmark::State& state)
{
    BENCHMARK_LOGD("SafeMap testSafeMapConstructor001 start.");
    while (state.KeepRunning()) {
        SafeMap<string, int> demoData;

        bool result = demoData.Insert("ONE", INSERT_ONE);
        AssertEqual(result, true, "result did not equal true as expected.", state);
        result = demoData.Insert("TWO", INSERT_TWO);
        AssertEqual(result, true, "result did not equal true as expected.", state);

        SafeMap<string, int> demoData1(demoData);
        int valueOne;
        int valueTwo;
        bool foundOne = demoData1.Find("ONE", valueOne);
        bool foundTwo = demoData1.Find("TWO", valueTwo);
        AssertTrue(foundOne && foundTwo, "Not all keys were found as expected.", state);
        AssertEqual(valueOne, INSERT_ONE, "Value for key \"ONE\" did not match as expected.", state);
        AssertEqual(valueTwo, INSERT_TWO, "Value for key \"TWO\" did not match as expected.", state);
    }
    BENCHMARK_LOGD("SafeMap testSafeMapConstructor001 end.");
}

/*
 * @tc.name: testSafeMapOperator001
 * @tc.desc: SafeMapConstructor
 */
BENCHMARK_F(BenchmarkSafeMap, testSafeMapOperator001)(benchmark::State& state)
{
    BENCHMARK_LOGD("SafeMap testSafeMapOperator001 start.");
    while (state.KeepRunning()) {
        SafeMap<string, int> demoData;

        bool result = demoData.Insert("ONE", INSERT_ONE);
        AssertEqual(result, true, "result did not equal true as expected.", state);
        result = demoData.Insert("TWO", INSERT_TWO);
        AssertEqual(result, true, "result did not equal true as expected.", state);

        int valueOne;
        int valueTwo;
        bool foundOne = demoData.Find("ONE", valueOne);
        bool foundTwo = demoData.Find("TWO", valueTwo);
        AssertTrue(foundOne, "Key \"ONE\" was not found as expected.", state);
        AssertTrue(foundTwo, "Key \"TWO\" was not found as expected.", state);
        AssertEqual(valueOne, INSERT_ONE, "Value for key \"ONE\" did not equal INSERT_ONE as expected.", state);
        AssertEqual(valueTwo, INSERT_TWO, "Value for key \"TWO\" did not equal INSERT_TWO as expected.", state);
    }
    BENCHMARK_LOGD("SafeMap testSafeMapOperator001 end.");
}

/*
 * @tc.name: testSafeMapOperator002
 * @tc.desc: SafeMapConstructor
 */
BENCHMARK_F(BenchmarkSafeMap, testSafeMapOperator002)(benchmark::State& state)
{
    BENCHMARK_LOGD("SafeMap testSafeMapOperator002 start.");
    while (state.KeepRunning()) {
        SafeMap<string, int> demoData;

        bool result = demoData.Insert("ONE", INSERT_ONE);
        AssertEqual(result, true, "result did not equal true as expected.", state);

        SafeMap<string, int> demoData1;
        result = demoData1.Insert("TWO", INSERT_ONE);
        AssertEqual(result, true, "result did not equal true as expected.", state);

        int valueDemoData;
        int valueDemoData1;
        bool foundDemoData = demoData.Find("ONE", valueDemoData);
        bool foundDemoData1 = demoData1.Find("TWO", valueDemoData1);
        BENCHMARK_LOGD("SafeMap data1:%{public}d data2:%{public}d", foundDemoData, foundDemoData1);
        AssertEqual(valueDemoData, valueDemoData1,
            "Values for keys \"ONE\" in demoData and demoData1 did not match as expected.", state);
    }
    BENCHMARK_LOGD("SafeMap testSafeMapOperator002 end.");
}

/*
 * @tc.name: testUtilsConcurrentIterate001
 * @tc.desc: 10 threads test in iterate operation to rewrite a SafeMap.
 */
const int SLEEP_FOR_FIFTY_MILLISECOND = 50;
const int THREAD_NUM = 10;
const int DATA_NUM = 5;
BENCHMARK_F(BenchmarkSafeMap, testUtilsConcurrentIterate001)(benchmark::State& state)
{
    BENCHMARK_LOGD("SafeMap testUtilsConcurrentIterate001 start.");
    while (state.KeepRunning()) {
        SafeMap<string, int> demoData;
        for (int i = 0; i < DATA_NUM; i++) {
            demoData.Insert("A" + std::to_string(i), 0);
        }
        std::thread threads[THREAD_NUM];

        auto lamfuncIterate = [](SafeMap<string, int>& data, const int& cnt,
            std::chrono::time_point<std::chrono::high_resolution_clock> absTime) {
            auto callback_it = [cnt](const string data, int& value) {
                value = cnt;
            };
            std::this_thread::sleep_until(absTime);
            data.Iterate(callback_it);
        };

        auto timeT = std::chrono::high_resolution_clock::now();
        timeT += std::chrono::milliseconds(SLEEP_FOR_FIFTY_MILLISECOND);

        for (int i = 0; i < THREAD_NUM; ++i) {
            threads[i] = std::thread(lamfuncIterate, std::ref(demoData), i, timeT);
        }

        for (auto& t : threads) {
            t.join();
        }

        for (int i = 0; i < DATA_NUM - 1; i++) {
            int valueCurrent;
            int valueNext;
            bool foundCurrent = demoData.Find("A" + std::to_string(i), valueCurrent);
            bool foundNext = demoData.Find("A" + std::to_string(i + 1), valueNext);

            AssertTrue(foundCurrent && foundNext, "Not all keys were found as expected.", state);
            AssertEqual(valueCurrent, valueNext,
                ("Values for keys \"A" + std::to_string(i) + "\" and \"A" + std::to_string(i + 1) +
                "\" did not match as expected.").c_str(), state);
        }
    }
    BENCHMARK_LOGD("SafeMap testUtilsConcurrentIterate001 end.");
}

/*
 * @tc.name: testUtilsConcurrentWriteAndRead001
 * @tc.desc: 100 threads test in writein to the same key of the map, while read at same time  and no throw
 */
BENCHMARK_F(BenchmarkSafeMap, testUtilsConcurrentWriteAndRead001)(benchmark::State& state)
{
    BENCHMARK_LOGD("SafeMap testUtilsConcurrentWriteAndRead001 start.");
    SafeMap<string, int> demoData;
    std::thread threads[THREAD_NUM];
    std::thread checkThread[THREAD_NUM];
    while (state.KeepRunning()) {
        auto lamfuncInsert = [](SafeMap<string, int>& data, const string& key,
            const int& value, std::chrono::time_point<std::chrono::high_resolution_clock> absTime) {
            std::this_thread::sleep_until(absTime);
            data.EnsureInsert(key, value);
        };

        auto lamfuncCheck = [](SafeMap<string, int>& data, const string& key,
            std::chrono::time_point<std::chrono::high_resolution_clock> absTime) {
            std::this_thread::sleep_until(absTime);
            thread_local int i = -1;
            data.Find(key, i);
        };

        auto timeT = std::chrono::high_resolution_clock::now();
        timeT += std::chrono::milliseconds(SLEEP_FOR_FIFTY_MILLISECOND);
        string key("A");

        for (int i = 0; i < THREAD_NUM; ++i) {
            threads[i] = std::thread(lamfuncInsert, std::ref(demoData), key, i, timeT);
            checkThread[i] = std::thread(lamfuncCheck, std::ref(demoData), key, timeT);
        }

        for (auto& t : threads) {
            t.join();
        }

        for (auto& t : checkThread) {
            t.join();
        }
    }
    BENCHMARK_LOGD("SafeMap testUtilsConcurrentWriteAndRead001 end.");
}

void ClearAllContainer(SafeMap<string, int>& demoData, std::vector<std::future<int>>& vcfi, vector<int>& result)
{
    demoData.Clear();
    result.clear();
    vcfi.clear();
}

/*
 * @tc.name: testUtilsConcurrentWriteAndFind001
 * @tc.desc: 100 threads test in writein to the corresponding key of the map,
 * while read at same time  and check the results
 */
BENCHMARK_F(BenchmarkSafeMap, testUtilsConcurrentWriteAndFind001)(benchmark::State& state)
{
    BENCHMARK_LOGD("SafeMap testUtilsConcurrentWriteAndFind001 start.");
    SafeMap<string, int> demoData;
    std::thread threads[THREAD_NUM];
    std::vector<std::future<int>> vcfi;
    while (state.KeepRunning()) {
        auto lamfuncInsert = [](SafeMap<string, int>& data, const string& key, const int& value,
            std::chrono::time_point<std::chrono::high_resolution_clock> absTime) {
            std::this_thread::sleep_until(absTime);
            data.EnsureInsert(key, value);
        };
        auto lamfuncCheckLoop = [](SafeMap<string, int>& data, const string& key,
            std::chrono::time_point<std::chrono::high_resolution_clock> absTime) {
            std::this_thread::sleep_until(absTime);
            thread_local int i = -1;
            while (!data.Find(key, i)) {
                std::this_thread::sleep_for(std::chrono::microseconds(10));
            }
            return i;
        };
        auto timeT = std::chrono::high_resolution_clock::now();
        timeT += std::chrono::milliseconds(SLEEP_FOR_FIFTY_MILLISECOND);
        string key("A");
        for (int i = 0; i < THREAD_NUM; ++i) {
            threads[i] = std::thread(lamfuncInsert, std::ref(demoData), key + std::to_string(i), i, timeT);
            vcfi.push_back(std::async(std::launch::async, lamfuncCheckLoop,
                std::ref(demoData), key + std::to_string(i), timeT));
        }
        for (auto& t : threads) {
            t.join();
        }
        vector<int> result;
        for (auto& t : vcfi) {
            result.push_back(t.get());
        }
        std::sort(result.begin(), result.end());
        for (int i = 0; i < THREAD_NUM; ++i) {
            AssertEqual(i, result[i], "tmp did not equal result[i+10] as expected.", state);
        }
        ClearAllContainer(demoData, vcfi, result);
    }
    BENCHMARK_LOGD("SafeMap testUtilsConcurrentWriteAndFind001 end.");
}

/*
 * @tc.name: testUtilsConcurrentWriteAndFindAndSet001
 * @tc.desc: 100 threads test in writein to the corresponding key of the map,
 * while findandfix at same time  and check the results
 */
BENCHMARK_F(BenchmarkSafeMap, testUtilsConcurrentWriteAndFindAndSet001)(benchmark::State& state)
{
    SafeMap<string, int> demoData;
    std::thread threads[THREAD_NUM];
    std::vector<std::future<int>> vcfi;
    while (state.KeepRunning()) {
        auto lamfuncInsert = [](SafeMap<string, int>& data, const string& key,
            const int& value, std::chrono::time_point<std::chrono::high_resolution_clock> absTime) {
            std::this_thread::sleep_until(absTime);
            data.EnsureInsert(key, value);
        };
        auto lamfuncCheckLoop = [](SafeMap<string, int>& data, const string& key,
            const int& newvalue, std::chrono::time_point<std::chrono::high_resolution_clock> absTime) {
            std::this_thread::sleep_until(absTime);
            thread_local int i = -1;
            while (!data.FindOldAndSetNew(key, i, newvalue)) {
                std::this_thread::sleep_for(std::chrono::microseconds(10));
            }
            return i;
        };
        auto timeT = std::chrono::high_resolution_clock::now();
        timeT += std::chrono::milliseconds(SLEEP_FOR_FIFTY_MILLISECOND);
        string key("A");
        for (int i = 0; i < THREAD_NUM; ++i) {
            threads[i] = std::thread(lamfuncInsert, std::ref(demoData),
                key + std::to_string(i), i, timeT);
            vcfi.push_back(std::async(std::launch::async, lamfuncCheckLoop,
                std::ref(demoData), key + std::to_string(i), i + 1, timeT));
        }
        for (auto& t : threads)
            t.join();
        vector<int> result;
        for (auto& t : vcfi)
            result.push_back(t.get());
        std::sort(result.begin(), result.end());
        for (int i = 0; i < THREAD_NUM; ++i) {
            AssertEqual(i, result[i], "i did not equal result[i] as expected.", state);
        }
        result.clear();
        for (int i = 0; i < THREAD_NUM; ++i) {
            int t = -1;
            AssertTrue((demoData.Find("A" + std::to_string(i), t)), "demoData.Find did not equal true.", state);
            result.push_back(t);
        }
        std::sort(result.begin(), result.end());
        for (int i = 0; i < THREAD_NUM; ++i) {
            AssertEqual(i + 1, result[i], "i + 1 did not equal result[i] as expected.", state);
        }
        ClearAllContainer(demoData, vcfi, result);
    }
}
}  // namespace
}  // namespace OHOS
// Run the benchmark
BENCHMARK_MAIN();
