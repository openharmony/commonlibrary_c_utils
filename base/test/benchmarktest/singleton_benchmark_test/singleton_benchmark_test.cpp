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
#include "singleton.h"
#include <algorithm>
#include <iostream>
#include <fstream>
#include "benchmark_log.h"
#include "benchmark_assert.h"
using namespace std;

namespace OHOS {
namespace {

static constexpr long DELAYEDSINGLETON_SP1_USE_COUNT = 2;
static constexpr long DELAYEDSINGLETON_SP2_USE_COUNT = 3;

class BenchmarkSingletonTest : public benchmark::Fixture {
public:
    void SetUp(const ::benchmark::State& state) override
    {
    }

    void TearDown(const ::benchmark::State& state) override
    {
    }

    BenchmarkSingletonTest()
    {
        Iterations(iterations);
        Repetitions(repetitions);
        ReportAggregatesOnly();
    }

    ~BenchmarkSingletonTest() override = default;

protected:
    const int32_t repetitions = 3;
    const int32_t iterations = 1000;
};

class DelayedSingletonDeclearTest {
    DECLARE_DELAYED_SINGLETON(DelayedSingletonDeclearTest);
public:
    void* GetObjAddr()
    {
        BENCHMARK_LOGD("SingletonTest DelayedSingletonDeclearTest void* GetObjAddr is called.");
        return static_cast<void*>(this);
    }
};

DelayedSingletonDeclearTest::~DelayedSingletonDeclearTest() {};
DelayedSingletonDeclearTest::DelayedSingletonDeclearTest() {};

class SingletonDeclearTest {
    DECLARE_SINGLETON(SingletonDeclearTest);
public:
    void* GetObjAddr()
    {
        BENCHMARK_LOGD("SingletonTest SingletonDeclearTest void* GetObjAddr is called.");
        return static_cast<void*>(this);
    }
};

SingletonDeclearTest::~SingletonDeclearTest() {};
SingletonDeclearTest::SingletonDeclearTest() {};

class SingletonTest : public Singleton<SingletonTest> {
public:
    void* GetObjAddr()
    {
        BENCHMARK_LOGD("SingletonTest SingletonTest void* GetObjAddr is called.");
        return static_cast<void*>(this);
    }
};

class DelayedSingletonTest : public DelayedSingleton<DelayedSingletonTest> {
public:
    void* GetObjAddr()
    {
        BENCHMARK_LOGD("SingletonTest DelayedSingletonTest void* GetObjAddr is called.");
        return static_cast<void*>(this);
    }
};


class DelayedRefSingletonDeclearTest {
    DECLARE_DELAYED_REF_SINGLETON(DelayedRefSingletonDeclearTest);
public:
    void* GetObjAddr()
    {
        BENCHMARK_LOGD("SingletonTest DelayedRefSingletonDeclearTest void* GetObjAddr is called.");
        return static_cast<void*>(this);
    }
};

DelayedRefSingletonDeclearTest::DelayedRefSingletonDeclearTest() {};
DelayedRefSingletonDeclearTest::~DelayedRefSingletonDeclearTest() {};

class DelayedRefSingletonTest : public DelayedRefSingleton<DelayedRefSingletonTest> {
public:
    void* GetObjAddr()
    {
        BENCHMARK_LOGD("SingletonTest DelayedRefSingletonTest void* GetObjAddr is called.");
        return static_cast<void*>(this);
    }
};

BENCHMARK_F(BenchmarkSingletonTest, test_DelayedSingletonDeclearTest)(benchmark::State& state)
{
    BENCHMARK_LOGD("SingletonTest test_DelayedSingletonDeclearTest start.");
    while (state.KeepRunning()) {
        shared_ptr<DelayedSingletonDeclearTest> sp1 = DelayedSingleton<DelayedSingletonDeclearTest>::GetInstance();
        AssertEqual(sp1.use_count(), DELAYEDSINGLETON_SP1_USE_COUNT,
            "sp1.use_count() did not equal DELAYEDSINGLETON_SP1_USE_COUNT as expected.", state);

        shared_ptr<DelayedSingletonDeclearTest> sp2 = DelayedSingleton<DelayedSingletonDeclearTest>::GetInstance();
        AssertEqual(sp1->GetObjAddr(), sp2->GetObjAddr(),
            "sp1->GetObjAddr() did not equal sp2->GetObjAddr() as expected.", state);
        AssertEqual(sp1.get(), sp2.get(), "sp1.get() did not equal sp2.get() as expected.", state);
        AssertEqual(sp2.use_count(), DELAYEDSINGLETON_SP2_USE_COUNT,
            "sp2.use_count() did not equal DELAYEDSINGLETON_SP2_USE_COUNT as expected.", state);
    }
    BENCHMARK_LOGD("SingletonTest test_DelayedSingletonDeclearTest end.");
}

BENCHMARK_F(BenchmarkSingletonTest, test_SingletonDeclearTest)(benchmark::State& state)
{
    BENCHMARK_LOGD("SingletonTest test_SingletonDeclearTest start.");
    while (state.KeepRunning()) {
        SingletonDeclearTest &st1 = Singleton<SingletonDeclearTest>::GetInstance();
        SingletonDeclearTest &st2 = Singleton<SingletonDeclearTest>::GetInstance();
        AssertEqual(st1.GetObjAddr(), st2.GetObjAddr(),
            "st1.GetObjAddr() did not equal st2.GetObjAddr() as expected.", state);
    }
    BENCHMARK_LOGD("SingletonTest test_SingletonDeclearTest end.");
}

BENCHMARK_F(BenchmarkSingletonTest, test_SingletonTest)(benchmark::State& state)
{
    BENCHMARK_LOGD("SingletonTest test_SingletonTest start.");
    while (state.KeepRunning()) {
        SingletonTest &st1 = SingletonTest::GetInstance();
        SingletonTest &st2 = SingletonTest::GetInstance();
        AssertEqual(st1.GetObjAddr(), st2.GetObjAddr(),
            "st1.GetObjAddr() did not equal st2.GetObjAddr() as expected.", state);
    }
    BENCHMARK_LOGD("SingletonTest test_SingletonTest end.");
}

BENCHMARK_F(BenchmarkSingletonTest, test_DelayedSingletonTest)(benchmark::State& state)
{
    BENCHMARK_LOGD("SingletonTest test_DelayedSingletonTest start.");
    while (state.KeepRunning()) {
        shared_ptr<DelayedSingletonTest> sp1 = DelayedSingletonTest::GetInstance();
        AssertEqual(sp1.use_count(), DELAYEDSINGLETON_SP1_USE_COUNT,
            "sp1.use_count() did not equal DELAYEDSINGLETON_SP1_USE_COUNT as expected.", state);

        shared_ptr<DelayedSingletonTest> sp2 = DelayedSingletonTest::GetInstance();
        AssertEqual(sp1->GetObjAddr(), sp2->GetObjAddr(),
            "sp1->GetObjAddr() did not equal sp2->GetObjAddr() as expected.", state);
        AssertEqual(sp1.get(), sp2.get(), "sp1.get() did not equal sp2.get() as expected.", state);
        AssertEqual(sp2.use_count(), DELAYEDSINGLETON_SP2_USE_COUNT,
            "sp2.use_count() did not equal DELAYEDSINGLETON_SP2_USE_COUNT as expected.", state);
    }
    BENCHMARK_LOGD("SingletonTest test_DelayedSingletonTest end.");
}

BENCHMARK_F(BenchmarkSingletonTest, test_DelayedRefSingletonTest)(benchmark::State& state)
{
    BENCHMARK_LOGD("SingletonTest test_DelayedRefSingletonTest start.");
    while (state.KeepRunning()) {
        DelayedRefSingletonTest& p1 = DelayedRefSingletonTest::GetInstance();
        DelayedRefSingletonTest& p2 = DelayedRefSingletonTest::GetInstance();
        AssertEqual(p1.GetObjAddr(), p2.GetObjAddr(),
            "p1.GetObjAddr() did not equal p2.GetObjAddr() as expected.", state);
    }
    BENCHMARK_LOGD("SingletonTest test_DelayedRefSingletonTest end.");
}

BENCHMARK_F(BenchmarkSingletonTest, test_DelayedRefSingletonDeclearTest)(benchmark::State& state)
{
    BENCHMARK_LOGD("SingletonTest test_DelayedRefSingletonDeclearTest start.");
    while (state.KeepRunning()) {
        DelayedRefSingletonDeclearTest& p1 = DelayedRefSingleton<DelayedRefSingletonDeclearTest>::GetInstance();
        DelayedRefSingletonDeclearTest& p2 = DelayedRefSingleton<DelayedRefSingletonDeclearTest>::GetInstance();
        AssertEqual(p1.GetObjAddr(), p2.GetObjAddr(),
            "p1.GetObjAddr() did not equal p2.GetObjAddr() as expected.", state);
    }
    BENCHMARK_LOGD("SingletonTest test_DelayedRefSingletonDeclearTest end.");
}

/**
 * @tc.name: test_DelayedSingletonDestroyTest
 * @tc.desc: test Singleton Destroy Instance
 * @tc.type: FUNC
 */
BENCHMARK_F(BenchmarkSingletonTest, test_DelayedSingletonDestroyTest)(benchmark::State& state)
{
    BENCHMARK_LOGD("SingletonTest test_DelayedSingletonDestroyTest start.");
    while (state.KeepRunning()) {
        shared_ptr<DelayedSingletonTest> sp1 = DelayedSingleton<DelayedSingletonTest>::GetInstance();
        AssertUnequal(sp1, nullptr, "sp1 equal nullptr as expected.", state);

        sp1.reset();
        DelayedSingleton<DelayedSingletonTest>::DestroyInstance();
        AssertEqual(sp1, nullptr, "sp1 not equal nullptr as expected.", state);
    }
    BENCHMARK_LOGD("SingletonTest test_DelayedSingletonDestroyTest end.");
}
}  // namespace
}  // namespace OHOS
// Run the benchmark
BENCHMARK_MAIN();