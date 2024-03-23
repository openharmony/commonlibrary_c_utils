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
#include <thread>
#include <string>
#include "rwlock.h"
#include "benchmark_log.h"
#include "benchmark_assert.h"
using namespace std;

namespace OHOS {
namespace {

class BenchmarkRWLockTest : public benchmark::Fixture {
public:
    void SetUp(const ::benchmark::State& state) override
    {
    }

    void TearDown(const ::benchmark::State& state) override
    {
    }

    BenchmarkRWLockTest()
    {
        Iterations(iterations);
        Repetitions(repetitions);
        ReportAggregatesOnly();
    }

    ~BenchmarkRWLockTest() override = default;

protected:
    const int32_t repetitions = 3;
    const int32_t iterations = 1000;
};

const int SLEEP_DURATION_MS = 4;

// This class is designed for test RWLock. "buf_" is protected by "rwLock_".
class TestRWLock {
public:
    TestRWLock():rwLock_(), buf_() {}

    explicit TestRWLock(bool writeFirst):rwLock_(writeFirst), buf_() {}

    void WriteStr(const string& str)
    {
        BENCHMARK_LOGD("RWLockTest void WriteStr is called.");
        rwLock_.LockWrite();
        for (auto it = str.begin(); it != str.end(); it++) {
            buf_.push_back(*it);
            this_thread::sleep_for(std::chrono::milliseconds(10)); // 10: Extend time of holding the lock
        }
        rwLock_.UnLockWrite();
        return;
    }

    void ReadStr(string& str)
    {
        BENCHMARK_LOGD("RWLockTest void ReadStr is called.");
        rwLock_.LockRead();
        for (auto it = buf_.begin(); it != buf_.end(); it++) {
            str.push_back(*it);
            this_thread::sleep_for(std::chrono::milliseconds(10)); // 10: Extend time of holding the lock
        }
        rwLock_.UnLockRead();
        return;
    }
private:
    Utils::RWLock rwLock_;
    string buf_;
};

const string WRITE_IN_1("write1");
const string WRITE_IN_2("write2");

/*
 * @tc.name: testRWLock001
 * @tc.desc: RWLock here is under write-first mode. If there are some writing operation waiting,
 * reading will never happen. Reading operations are likely to run at the same time, when all writing operations
 * have finished.
 */
BENCHMARK_F(BenchmarkRWLockTest, testRWLock001)(benchmark::State& state)
{
    BENCHMARK_LOGD("RWLockTest testRWLock001 start.");
    while (state.KeepRunning()) {
        TestRWLock test;

        thread first(bind(&TestRWLock::WriteStr, ref(test), ref(WRITE_IN_1)));
        // Try our best to make `first` get the lock
        this_thread::sleep_for(std::chrono::milliseconds(SLEEP_DURATION_MS));

        string readOut1("");
        thread second(bind(&TestRWLock::ReadStr, ref(test), ref(readOut1)));
        thread third(bind(&TestRWLock::WriteStr, ref(test), ref(WRITE_IN_2)));
        string readOut2("");
        thread fourth(bind(&TestRWLock::ReadStr, ref(test), ref(readOut2)));


        first.join();
        second.join();
        third.join();
        fourth.join();

        AssertEqual(readOut1, WRITE_IN_1 + WRITE_IN_2,
            "readOut1 did not equal WRITE_IN_1 + WRITE_IN_2 as expected.", state);
        AssertEqual(readOut2, WRITE_IN_1 + WRITE_IN_2,
            "readOut2 did not equal WRITE_IN_1 + WRITE_IN_2 as expected.", state);
    }
    BENCHMARK_LOGD("RWLockTest testRWLock001 end.");
}

/*
 * @tc.name: testRWLock002
 * @tc.desc: RWLock here is not under write-first mode. So if there are writing and reading operations in queue
 * with a writing mission running, they will compete when the writing mission completing, but reading operations are
 * likely to run at the same time.
 */
BENCHMARK_F(BenchmarkRWLockTest, testRWLock002)(benchmark::State& state)
{
    BENCHMARK_LOGD("RWLockTest testRWLock002 start.");
    while (state.KeepRunning()) {
        TestRWLock test(false);

        thread first(bind(&TestRWLock::WriteStr, ref(test), ref(WRITE_IN_1)));
        this_thread::sleep_for(chrono::milliseconds(SLEEP_DURATION_MS));

        string readOut1("");
        thread second(bind(&TestRWLock::ReadStr, ref(test), ref(readOut1)));
        thread third(bind(&TestRWLock::WriteStr, ref(test), ref(WRITE_IN_2)));
        string readOut2("");
        thread fourth(bind(&TestRWLock::ReadStr, ref(test), ref(readOut2)));

        first.join();
        second.join();
        third.join();
        fourth.join();

        AssertEqual(readOut1, readOut2, "readOut1 did not equal readOut2 as expected.", state);
    }
    BENCHMARK_LOGD("RWLockTest testRWLock002 end.");
}

/*
 * @tc.name: testRWLockDefaultConstructor001
 * @tc.desc: This test case validates the default constructor of RWLock. By default, the RWLock is in write-first mode.
 * In this mode, if there are pending write operations, read operations will not occur. This test case creates
 * a default RWLock and attempts to perform read operations while write operations are pending.
 * The expected behavior is that the read operations should not occur until the write operations are complete.
 */
BENCHMARK_F(BenchmarkRWLockTest, testRWLockDefaultConstructor001)(benchmark::State& state)
{
    BENCHMARK_LOGD("RWLockTest testRWLockDefaultConstructor001 start.");
    while (state.KeepRunning()) {
        TestRWLock test;

        thread first(bind(&TestRWLock::WriteStr, ref(test), ref(WRITE_IN_1)));
        this_thread::sleep_for(std::chrono::milliseconds(SLEEP_DURATION_MS));

        string readOut1("");
        thread second(bind(&TestRWLock::ReadStr, ref(test), ref(readOut1)));

        first.join();
        second.join();

        AssertEqual(readOut1, WRITE_IN_1, "readOut1 did not equal WRITE_IN_1 as expected.", state);
    }
    BENCHMARK_LOGD("RWLockTest testRWLockDefaultConstructor001 end.");
}

/*
 * @tc.name: testUniqueWriteGuardScope001
 * @tc.desc: This benchmark test is designed to test the functionality of the UniqueWriteGuard class.
 * In this test, a write lock is acquired on an instance of the RWLock class using an instance of
 * the UniqueWriteGuard class. The WriteStr method of the TestRWLock class is then called to write a string to
 * the buffer of the TestRWLock instance. After the write operation, the write lock is automatically released
 * because the UniqueWriteGuard instance goes out of scope. The ReadStr method of the TestRWLock class is then called
 * to read the string from the buffer of the TestRWLock instance. If the read string does not match the written
 * string, the test fails and an error message is logged. This test case is repeated multiple times to measure
 * the performance of the write lock operation.
 */
BENCHMARK_F(BenchmarkRWLockTest, testUniqueWriteGuardScope001)(benchmark::State& state)
{
    BENCHMARK_LOGD("RWLockTest testUniqueWriteGuardScope001 start.");
    while (state.KeepRunning()) {
        OHOS::Utils::RWLock rwLock_;
        TestRWLock test;
        string readOut1("");
        OHOS::Utils::UniqueWriteGuard<OHOS::Utils::RWLock> guard(rwLock_);
        test.WriteStr(WRITE_IN_1);
        test.ReadStr(readOut1);
        AssertEqual(readOut1, WRITE_IN_1, "readOut1 did not equal WRITE_IN_1 as expected.", state);
    }
    BENCHMARK_LOGD("RWLockTest testUniqueWriteGuardScope001 end.");
}

/*
 * @tc.name: testUniqueReadGuardScope001
 * @tc.desc: This benchmark test is designed to test the functionality of the UniqueReadGuard class.
 * In this test, a read lock is acquired on an instance of the RWLock class using an instance of
 * the UniqueReadGuard class. The ReadStr method of the TestRWLock class is then called to read the string from
 * the buffer of the TestRWLock instance. After the read operation, the read lock is automatically released because
 * the UniqueReadGuard instance goes out of scope. This test case is repeated multiple times to measure
 * the performance of the read lock operation.
 */
BENCHMARK_F(BenchmarkRWLockTest, testUniqueReadGuardScope001)(benchmark::State& state)
{
    BENCHMARK_LOGD("RWLockTest testUniqueReadGuardScope001 start.");
    while (state.KeepRunning()) {
        OHOS::Utils::RWLock rwLock_;
        TestRWLock test;
        string readOut1("");
        test.WriteStr(WRITE_IN_1);  // Write a string to the buffer before acquiring the read lock.
        OHOS::Utils::UniqueReadGuard<OHOS::Utils::RWLock> guard(rwLock_);
        test.ReadStr(readOut1);
        AssertEqual(readOut1, WRITE_IN_1, "readOut1 did not equal WRITE_IN_1 as expected.", state);
    }
    BENCHMARK_LOGD("RWLockTest testUniqueReadGuardScope001 end.");
}
}  // namespace
}  // namespace OHOS
// Run the benchmark
BENCHMARK_MAIN();
