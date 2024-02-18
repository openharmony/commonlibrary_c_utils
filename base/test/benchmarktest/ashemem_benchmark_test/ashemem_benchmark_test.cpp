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
#include <algorithm>
#include <fstream>
#include <iostream>
#include <unistd.h>
#include <sys/mman.h>
#include "directory_ex.h"
#include "securec.h"
#include "parcel.h"
#include "refbase.h"
#include "ashmem.h"
#include "benchmark_log.h"
#include "benchmark_assert.h"
using namespace std;

namespace OHOS {
namespace {

const int MAX_PARCEL_SIZE = 1000;
char g_data[MAX_PARCEL_SIZE];
const int32_t MEMORY_SIZE = 1024;
const std::string MEMORY_CONTENT = "HelloWorld2020\0";
const std::string MEMORY_NAME = "Test SharedMemory\0";

class BenchmarkAshmemTest : public benchmark::Fixture {
public:
    BenchmarkAshmemTest()
    {
        Iterations(iterations);
        Repetitions(repetitions);
        ReportAggregatesOnly(true);
    }

    ~BenchmarkAshmemTest() override = default;
    void SetUp(const ::benchmark::State& state) override
    {
    };

    void TearDown(const ::benchmark::State& state) override
    {
        for (int i = 0; i < MAX_PARCEL_SIZE; i++) {
            g_data[i] = 0;
        }
    }

protected:
    const int32_t repetitions = 3;
    const int32_t iterations = 1000;
};

/**
 * @tc.name: test_ashmem_WriteAndRead_001
 * @tc.desc: create and map ashmem
 * @tc.type: FUNC
 */
BENCHMARK_F(BenchmarkAshmemTest, test_ashmem_WriteAndRead_001)(benchmark::State& state)
{
    BENCHMARK_LOGD("AshmemTest test_ashmem_WriteAndRead_001 start.");
    while (state.KeepRunning()) {
        sptr<Ashmem> ashmem = Ashmem::CreateAshmem(MEMORY_NAME.c_str(), MEMORY_SIZE);
        AssertUnequal(ashmem, nullptr, "ashmem != nullptr did not equal true as expected.", state);
        AssertEqual(ashmem->GetAshmemSize(), MEMORY_SIZE,
            "ashmem->GetAshmemSize() == MEMORY_SIZE did not equal true as expected.", state);

        bool ret = ashmem->MapAshmem(PROT_READ | PROT_WRITE);
        AssertTrue(ret, "ret did not equal true as expected.", state);

        ashmem->UnmapAshmem();
        ashmem->CloseAshmem();
    }
    BENCHMARK_LOGD("AshmemTest test_ashmem_WriteAndRead_001 end.");
}

/**
 * @tc.name: test_ashmem_WriteAndRead_002
 * @tc.desc: write to and read from ashmem
 * @tc.type: FUNC
 */
BENCHMARK_F(BenchmarkAshmemTest, test_ashmem_WriteAndRead_002)(benchmark::State& state)
{
    BENCHMARK_LOGD("AshmemTest test_ashmem_WriteAndRead_002 start.");
    while (state.KeepRunning()) {
        sptr<Ashmem> ashmem = Ashmem::CreateAshmem(MEMORY_NAME.c_str(), MEMORY_SIZE);
        AssertUnequal(ashmem, nullptr, "ashmem != nullptr did not equal true as expected.", state);

        bool ret = ashmem->MapReadAndWriteAshmem();
        AssertTrue(ret, "ret did not equal true as expected.", state);

        ret = ashmem->WriteToAshmem(MEMORY_CONTENT.c_str(), sizeof(MEMORY_CONTENT), 0);
        AssertTrue(ret, "ret did not equal true as expected.", state);

        ret = ashmem->WriteToAshmem(MEMORY_CONTENT.c_str(), sizeof(MEMORY_CONTENT), sizeof(MEMORY_CONTENT));
        AssertTrue(ret, "ret did not equal true as expected.", state);

        auto readData = ashmem->ReadFromAshmem(sizeof(MEMORY_CONTENT), 0);
        AssertUnequal(readData, nullptr, "readData != nullptr did not equal true as expected.", state);

        const char *readContent = reinterpret_cast<const char *>(readData);
        AssertEqual(memcmp(MEMORY_CONTENT.c_str(), readContent, sizeof(MEMORY_CONTENT)), 0,
            "memcmp(MEMORY_CONTENT.c_str(), readContent, sizeof(MEMORY_CONTENT)) did not equal 0 as expected.", state);

        readData = ashmem->ReadFromAshmem(sizeof(MEMORY_CONTENT), sizeof(MEMORY_CONTENT));
        AssertUnequal(readData, nullptr, "readData != nullptr did not equal true as expected.", state);

        readContent = reinterpret_cast<const char *>(readData);
        AssertEqual(memcmp(MEMORY_CONTENT.c_str(), readContent, sizeof(MEMORY_CONTENT)), 0,
            "memcmp(MEMORY_CONTENT.c_str(), readContent, sizeof(MEMORY_CONTENT)) did not equal 0 as expected.", state);

        ashmem->UnmapAshmem();
        ashmem->CloseAshmem();
    }
    BENCHMARK_LOGD("AshmemTest test_ashmem_WriteAndRead_002 end.");
}

/**
 * @tc.name: test_ashmem_WriteAndRead_003
 * @tc.desc: test read-only ashmem
 * @tc.type: FUNC
 */
BENCHMARK_F(BenchmarkAshmemTest, test_ashmem_WriteAndRead_003)(benchmark::State& state)
{
    BENCHMARK_LOGD("AshmemTest test_ashmem_WriteAndRead_003 begin");
    while (state.KeepRunning()) {
        sptr<Ashmem> ashmem = Ashmem::CreateAshmem(MEMORY_NAME.c_str(), MEMORY_SIZE);
        AssertUnequal(ashmem, nullptr, "ashmem != nullptr did not equal true as expected.", state);

        bool ret = ashmem->MapReadAndWriteAshmem();
        AssertTrue(ret, "ret did not equal true as expected.", state);

        ret = ashmem->WriteToAshmem(MEMORY_CONTENT.c_str(), sizeof(MEMORY_CONTENT), 0);
        AssertTrue(ret, "ret did not equal true as expected.", state);

        ashmem->UnmapAshmem();

        ret = ashmem->MapReadOnlyAshmem();
        AssertTrue(ret, "ret did not equal true as expected.", state);

        ret = ashmem->WriteToAshmem(MEMORY_CONTENT.c_str(), sizeof(MEMORY_CONTENT), sizeof(MEMORY_CONTENT));
        AssertFalse(ret, "ret did not equal false", state);

        auto readData = ashmem->ReadFromAshmem(sizeof(MEMORY_CONTENT), 0);
        AssertUnequal(readData, nullptr, "readData != nullptr did not equal true as expected.", state);

        const char *readContent = reinterpret_cast<const char *>(readData);
        AssertEqual(memcmp(MEMORY_CONTENT.c_str(), readContent, sizeof(MEMORY_CONTENT)), 0,
            "memcmp(MEMORY_CONTENT.c_str(), readContent, sizeof(MEMORY_CONTENT)) did not equal 0 as expected.", state);

        ashmem->UnmapAshmem();
        ashmem->CloseAshmem();
    }
    BENCHMARK_LOGD("AshmemTest test_ashmem_WriteAndRead_003 end.");
}

/**
 * @tc.name: test_ashmem_WriteAndRead_004
 * @tc.desc: set read-only protection and map again
 * @tc.type: FUNC
 */
BENCHMARK_F(BenchmarkAshmemTest, test_ashmem_WriteAndRead_004)(benchmark::State& state)
{
    BENCHMARK_LOGD("AshmemTest test_ashmem_WriteAndRead_004 start.");
    while (state.KeepRunning()) {
        sptr<Ashmem> ashmem = Ashmem::CreateAshmem(MEMORY_NAME.c_str(), MEMORY_SIZE);
        AssertUnequal(ashmem, nullptr, "ashmem != nullptr did not equal true as expected.", state);

        bool ret = ashmem->MapReadAndWriteAshmem();
        AssertTrue(ret, "ret did not equal true as expected.", state);

        ret = ashmem->WriteToAshmem(MEMORY_CONTENT.c_str(), sizeof(MEMORY_CONTENT), 0);
        AssertTrue(ret, "ret did not equal true as expected.", state);

        ashmem->UnmapAshmem();

        ret = ashmem->SetProtection(PROT_READ);
        AssertTrue(ret, "ret did not equal true as expected.", state);

        ret = ashmem->MapReadAndWriteAshmem();
        AssertFalse(ret, "ret did not equal false", state);

        ret = ashmem->MapReadOnlyAshmem();
        AssertTrue(ret, "ret did not equal true as expected.", state);

        auto readData = ashmem->ReadFromAshmem(sizeof(MEMORY_CONTENT), 0);
        AssertUnequal(readData, nullptr, "readData != nullptr did not equal true as expected.", state);

        const char *readContent = reinterpret_cast<const char *>(readData);
        AssertEqual(memcmp(MEMORY_CONTENT.c_str(), readContent, sizeof(MEMORY_CONTENT)), 0,
            "memcmp(MEMORY_CONTENT.c_str(), readContent, sizeof(MEMORY_CONTENT)) did not equal 0 as expected.", state);

        ashmem->UnmapAshmem();
        ashmem->CloseAshmem();
    }
    BENCHMARK_LOGD("AshmemTest test_ashmem_WriteAndRead_004 end.");
}

/**
 * @tc.name: test_ashmem_WriteAndRead_005
 * @tc.desc: set read-only protection without mapping again
 * @tc.type: FUNC
 */
BENCHMARK_F(BenchmarkAshmemTest, test_ashmem_WriteAndRead_005)(benchmark::State& state)
{
    BENCHMARK_LOGD("AshmemTest test_ashmem_WriteAndRead_005 start.");
    while (state.KeepRunning()) {
        sptr<Ashmem> ashmem = Ashmem::CreateAshmem(MEMORY_NAME.c_str(), MEMORY_SIZE);
        AssertUnequal(ashmem, nullptr, "ashmem != nullptr did not equal true as expected.", state);

        bool ret = ashmem->MapReadAndWriteAshmem();
        AssertTrue(ret, "ret did not equal true as expected.", state);

        ret = ashmem->WriteToAshmem(MEMORY_CONTENT.c_str(), sizeof(MEMORY_CONTENT), 0);
        AssertTrue(ret, "ret did not equal true as expected.", state);

        ret = ashmem->SetProtection(PROT_READ);
        AssertTrue(ret, "ret did not equal true as expected.", state);

        ret = ashmem->WriteToAshmem(MEMORY_CONTENT.c_str(), sizeof(MEMORY_CONTENT), 0);
        AssertFalse(ret, "ret did not equal false", state);

        auto readData = ashmem->ReadFromAshmem(sizeof(MEMORY_CONTENT), 0);
        AssertUnequal(readData, nullptr, "readData != nullptr did not equal true as expected.", state);

        const char *readContent = reinterpret_cast<const char *>(readData);
        AssertEqual(memcmp(MEMORY_CONTENT.c_str(), readContent, sizeof(MEMORY_CONTENT)), 0,
            "memcmp(MEMORY_CONTENT.c_str(), readContent, sizeof(MEMORY_CONTENT)) did not equal 0 as expected.", state);

        ashmem->UnmapAshmem();
        ashmem->CloseAshmem();
    }
    BENCHMARK_LOGD("AshmemTest test_ashmem_WriteAndRead_005 end.");
}


/**
 * @tc.name: test_ashmem_InvalidOperation_001
 * @tc.desc: create invalid-size ashmem or set invalid protection type
 * @tc.type: FUNC
 */
BENCHMARK_F(BenchmarkAshmemTest, test_ashmem_InvalidOperation_001)(benchmark::State& state)
{
    BENCHMARK_LOGD("AshmemTest test_ashmem_InvalidOperation_001 start.");
    while (state.KeepRunning()) {
        sptr<Ashmem> ashmem = Ashmem::CreateAshmem(MEMORY_NAME.c_str(), -1);
        AssertEqual(ashmem, nullptr, "ashmem == nullptr did not equal true as expected.", state);

        ashmem = Ashmem::CreateAshmem(MEMORY_NAME.c_str(), MEMORY_SIZE);
        AssertUnequal(ashmem, nullptr, "ashmem != nullptr did not equal true as expected.", state);

        bool ret = ashmem->SetProtection(-1);
        AssertFalse(ret, "ret did not equal false", state);

        ashmem->CloseAshmem();
    }
    BENCHMARK_LOGD("AshmemTest test_ashmem_InvalidOperation_001 end.");
}

/**
 * @tc.name: test_ashmem_InvalidOperation_002
 * @tc.desc: map after closing ashmem
 * @tc.type: FUNC
 */
BENCHMARK_F(BenchmarkAshmemTest, test_ashmem_InvalidOperation_002)(benchmark::State& state)
{
    BENCHMARK_LOGD("AshmemTest test_ashmem_InvalidOperation_002 start.");
    while (state.KeepRunning()) {
        sptr<Ashmem> ashmem = Ashmem::CreateAshmem(MEMORY_NAME.c_str(), MEMORY_SIZE);
        AssertUnequal(ashmem, nullptr, "ashmem != nullptr did not equal true as expected.", state);

        ashmem->CloseAshmem();

        bool ret = ashmem->MapReadAndWriteAshmem();
        AssertFalse(ret, "ret did not equal false", state);
    }
    BENCHMARK_LOGD("AshmemTest test_ashmem_InvalidOperation_002 end.");
}

/**
 * @tc.name: test_ashmem_InvalidOperation_003
 * @tc.desc: write or read after closing ashmem
 * @tc.type: FUNC
 */
BENCHMARK_F(BenchmarkAshmemTest, test_ashmem_InvalidOperation_003)(benchmark::State& state)
{
    BENCHMARK_LOGD("AshmemTest test_ashmem_InvalidOperation_003 start.");
    while (state.KeepRunning()) {
        sptr<Ashmem> ashmem = Ashmem::CreateAshmem(MEMORY_NAME.c_str(), MEMORY_SIZE);
        AssertUnequal(ashmem, nullptr, "ashmem != nullptr did not equal true as expected.", state);

        bool ret = ashmem->MapReadAndWriteAshmem();
        AssertTrue(ret, "ret did not equal true as expected.", state);

        ashmem->CloseAshmem();

        ret = ashmem->WriteToAshmem(MEMORY_CONTENT.c_str(), sizeof(MEMORY_CONTENT), 0);
        AssertFalse(ret, "ret did not equal false", state);

        auto readData = ashmem->ReadFromAshmem(sizeof(MEMORY_CONTENT), 0);
        AssertEqual(readData, nullptr, "readData == nullptr did not equal true as expected.", state);
    }
    BENCHMARK_LOGD("AshmemTest test_ashmem_InvalidOperation_003 end.");
}

/**
 * @tc.name: test_ashmem_InvalidOperation_004
 * @tc.desc: write or read after unmapping ashmem
 * @tc.type: FUNC
 */
BENCHMARK_F(BenchmarkAshmemTest, test_ashmem_InvalidOperation_004)(benchmark::State& state)
{
    BENCHMARK_LOGD("AshmemTest test_ashmem_InvalidOperation_004 start.");
    while (state.KeepRunning()) {
        sptr<Ashmem> ashmem = Ashmem::CreateAshmem(MEMORY_NAME.c_str(), MEMORY_SIZE);
        AssertUnequal(ashmem, nullptr, "ashmem != nullptr did not equal true as expected.", state);

        bool ret = ashmem->MapReadAndWriteAshmem();
        AssertTrue(ret, "ret did not equal true as expected.", state);

        ashmem->UnmapAshmem();

        ret = ashmem->WriteToAshmem(MEMORY_CONTENT.c_str(), sizeof(MEMORY_CONTENT), 0);
        AssertFalse(ret, "ret did not equal false", state);

        auto readData = ashmem->ReadFromAshmem(sizeof(MEMORY_CONTENT), 0);
        AssertEqual(readData, nullptr, "readData == nullptr did not equal true as expected.", state);

        ashmem->CloseAshmem();
    }
    BENCHMARK_LOGD("AshmemTest test_ashmem_InvalidOperation_004 end.");
}

/**
 * @tc.name: test_ashmem_InvalidOperation_005
 * @tc.desc: expand protection type
 * @tc.type: FUNC
 */
BENCHMARK_F(BenchmarkAshmemTest, test_ashmem_InvalidOperation_005)(benchmark::State& state)
{
    BENCHMARK_LOGD("AshmemTest test_ashmem_InvalidOperation_005 start.");
    while (state.KeepRunning()) {
        sptr<Ashmem> ashmem = Ashmem::CreateAshmem(MEMORY_NAME.c_str(), MEMORY_SIZE);
        AssertUnequal(ashmem, nullptr, "ashmem != nullptr did not equal true as expected.", state);

        bool ret = ashmem->SetProtection(PROT_WRITE);
        AssertTrue(ret, "ret did not equal true as expected.", state);

        ret = ashmem->SetProtection(PROT_READ);
        AssertFalse(ret, "ret did not equal false", state);

        ret = ashmem->SetProtection(PROT_READ | PROT_WRITE);
        AssertFalse(ret, "ret did not equal false", state);

        ret = ashmem->SetProtection(PROT_NONE);
        AssertTrue(ret, "ret did not equal true as expected.", state);

        ret = ashmem->SetProtection(PROT_READ);
        AssertFalse(ret, "ret did not equal false", state);

        ashmem->CloseAshmem();
    }
    BENCHMARK_LOGD("AshmemTest test_ashmem_InvalidOperation_005 end.");
}

/**
 * @tc.name: test_ashmem_InvalidOperation_006
 * @tc.desc: test invalid input or test invalid operation
 * @tc.type: FUNC
 */
BENCHMARK_F(BenchmarkAshmemTest, test_ashmem_InvalidOperation_006)(benchmark::State& state)
{
    BENCHMARK_LOGD("AshmemTest test_ashmem_InvalidOperation_006 start.");
    while (state.KeepRunning()) {
        sptr<Ashmem> ashmem = Ashmem::CreateAshmem(MEMORY_NAME.c_str(), MEMORY_SIZE);
        AssertUnequal(ashmem, nullptr, "ashmem != nullptr did not equal true as expected.", state);

        bool ret = ashmem->MapReadAndWriteAshmem();
        AssertTrue(ret, "ret did not equal true as expected.", state);

        ret = ashmem->WriteToAshmem(nullptr, sizeof(MEMORY_CONTENT), 0);
        AssertFalse(ret, "ret did not equal false", state);

        ret = ashmem->WriteToAshmem(MEMORY_CONTENT.c_str(), sizeof(MEMORY_CONTENT), MEMORY_SIZE+1);
        AssertFalse(ret, "ret did not equal false", state);

        ret = ashmem->WriteToAshmem(MEMORY_CONTENT.c_str(), sizeof(MEMORY_CONTENT), -1);
        AssertFalse(ret, "ret did not equal false", state);

        ret = ashmem->WriteToAshmem(MEMORY_CONTENT.c_str(), MEMORY_SIZE+1, 0);
        AssertFalse(ret, "ret did not equal false", state);

        ret = ashmem->WriteToAshmem(MEMORY_CONTENT.c_str(), -1, 0);
        AssertFalse(ret, "ret did not equal false", state);

        ret = ashmem->WriteToAshmem(MEMORY_CONTENT.c_str(), sizeof(MEMORY_CONTENT), MEMORY_SIZE);
        AssertFalse(ret, "ret did not equal false", state);

        ashmem->UnmapAshmem();
        ashmem->CloseAshmem();

        ashmem->GetAshmemSize();
        AssertFalse(ret, "ret did not equal false", state);

        ashmem->GetProtection();
        AssertFalse(ret, "ret did not equal false", state);

        ashmem->UnmapAshmem();
        AssertFalse(ret, "ret did not equal false", state);

        ashmem->CloseAshmem();
        AssertFalse(ret, "ret did not equal false", state);
    }
    BENCHMARK_LOGD("AshmemTest test_ashmem_InvalidOperation_006 end.");
}

/**
 * @tc.name: test_ashmem_constructor_001
 * @tc.desc: test Ashmem constructor
 * @tc.type: FUNC
 */
BENCHMARK_F(BenchmarkAshmemTest, test_ashmem_constructor_001)(benchmark::State& state)
{
    BENCHMARK_LOGD("AshmemTest test_ashmem_constructor_001 start.");
    while (state.KeepRunning()) {
        sptr<Ashmem> ashmem = Ashmem::CreateAshmem(MEMORY_NAME.c_str(), MEMORY_SIZE);
        AssertUnequal(ashmem, nullptr, "Failed to create Ashmem object.", state);

        int fd = ashmem->GetAshmemFd();
        int32_t size = ashmem->GetAshmemSize();
        AssertFalse((fd <= 0 || size <= 0), "Invalid file descriptor or size obtained from Ashmem.", state);

        sptr<Ashmem> newAshmem = new Ashmem(fd, size);
        AssertUnequal(newAshmem, nullptr, "Failed to create new Ashmem object with constructor.", state);

        int newFd = newAshmem->GetAshmemFd();
        int32_t newSize = newAshmem->GetAshmemSize();
        AssertFalse((newFd != fd || newSize != size),
            "Mismatch in file descriptor or size in new Ashmem object.", state);

        ashmem->UnmapAshmem();
        ashmem->CloseAshmem();
        newAshmem->UnmapAshmem();
        newAshmem->CloseAshmem();
    }
    BENCHMARK_LOGD("AshmemTest test_ashmem_constructor_001 end.");
}

/**
 * @tc.name: test_ashmem_ConstructorAndDestructor_001
 * @tc.desc: create and delete ashmem
 * @tc.type: FUNC
 */
BENCHMARK_F(BenchmarkAshmemTest, test_ashmem_ConstructorAndDestructor_001)(benchmark::State& state)
{
    BENCHMARK_LOGD("AshmemTest test_ashmem_ConstructorAndDestructor_001 start.");
    while (state.KeepRunning()) {
        sptr<Ashmem> ashmem = Ashmem::CreateAshmem(MEMORY_NAME.c_str(), MEMORY_SIZE);
        AssertUnequal(ashmem, nullptr, "ashmem != nullptr did not equal true as expected.", state);
        AssertEqual(ashmem->GetAshmemSize(), MEMORY_SIZE,
            "ashmem->GetAshmemSize() == MEMORY_SIZE did not equal true as expected.", state);

        ashmem->UnmapAshmem();
        ashmem->CloseAshmem();
    }
    BENCHMARK_LOGD("AshmemTest test_ashmem_ConstructorAndDestructor_001 end.");
}

/**
 * @tc.name: test_ashmem_GetAshmemFd_001
 * @tc.desc: test getting file descriptor of ashmem
 * @tc.type: FUNC
 */
BENCHMARK_F(BenchmarkAshmemTest, test_ashmem_GetAshmemFd_001)(benchmark::State& state)
{
    BENCHMARK_LOGD("AshmemTest test_ashmem_GetAshmemFd_001 start.");
    while (state.KeepRunning()) {
        sptr<Ashmem> ashmem = Ashmem::CreateAshmem(MEMORY_NAME.c_str(), MEMORY_SIZE);
        AssertUnequal(ashmem, nullptr, "Failed to create Ashmem object.", state);

        int fd = ashmem->GetAshmemFd();
        AssertGreaterThan(fd, 0, "Invalid file descriptor obtained from Ashmem.", state);

        ashmem->UnmapAshmem();
        ashmem->CloseAshmem();
    }
    BENCHMARK_LOGD("AshmemTest test_ashmem_GetAshmemFd_001 end.");
}
}  // namespace
}  // namespace OHOS
// Run the benchmark
BENCHMARK_MAIN();