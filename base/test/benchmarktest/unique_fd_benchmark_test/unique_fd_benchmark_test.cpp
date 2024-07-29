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
#include "unique_fd.h"
#include <fstream>
#include <iostream>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include "benchmark_log.h"
#include "benchmark_assert.h"
using namespace std;

namespace OHOS {
namespace {

static const char *TEST_FILE_NAME = "testfilename.test";

class BenchmarkUniqueFd : public benchmark::Fixture {
public:
    BenchmarkUniqueFd()
    {
        Iterations(iterations);
        Repetitions(repetitions);
        ReportAggregatesOnly();
    }

    ~BenchmarkUniqueFd() override = default;
    void SetUp(const ::benchmark::State& state) override
    {
        ofstream outfile;
        outfile.open(TEST_FILE_NAME, ios::out | ios::trunc);
        outfile << "testdata\n"
                << std::endl;
        outfile.close();
    }

    void TearDown(::benchmark::State& state) override
    {
        ifstream inputfile;
        inputfile.open(TEST_FILE_NAME, ios::in);
        std::string testStr;
        inputfile >> testStr;
        inputfile.close();
        AssertEqual(remove(TEST_FILE_NAME), 0, "remove(TEST_FILE_NAME) did not equal 0 as expected.", state);
    }

protected:
    const int32_t repetitions = 3;
    const int32_t iterations = 1000;
};

const int FILE_PERMISSION_READ_WRITE = 0666;
const int MIN_VALID_FD = 0;
const int MAX_VALID_FD = 1000000;

BENCHMARK_F(BenchmarkUniqueFd, testUtilsUniqueFd)(benchmark::State& state)
{
    BENCHMARK_LOGD("UniqueFd testUtilsUniqueFd start.");
    const int expectedValue = -1;
    while (state.KeepRunning()) {
        int fd = open("NOTHISFILE", O_RDWR, FILE_PERMISSION_READ_WRITE);

        UniqueFd ufd2(fd);
        AssertEqual(ufd2, expectedValue, "ufd2 did not equal expectedValue as expected.", state);
    }
    BENCHMARK_LOGD("UniqueFd testUtilsUniqueFd end.");
}

BENCHMARK_F(BenchmarkUniqueFd, testUtilsUniqueCtroFromInt)(benchmark::State& state)
{
    BENCHMARK_LOGD("UniqueFd testUtilsUniqueCtroFromInt start.");
    const int expectedValue = -1;
    while (state.KeepRunning()) {
        UniqueFd ufd2(open(TEST_FILE_NAME, O_RDWR, FILE_PERMISSION_READ_WRITE));
        AssertUnequal(ufd2, expectedValue, "ufd2 was not different from expectedValue as expected.", state);
    }
    BENCHMARK_LOGD("UniqueFd testUtilsUniqueCtroFromInt end.");
}

BENCHMARK_F(BenchmarkUniqueFd, testUtilsUniqueFdeqcompare)(benchmark::State& state)
{
    BENCHMARK_LOGD("UniqueFd testUtilsUniqueFdeqcompare start.");
    while (state.KeepRunning()) {
        int fd = open(TEST_FILE_NAME, O_RDWR, FILE_PERMISSION_READ_WRITE);
        UniqueFd ufd2(fd);
        AssertEqual(fd, ufd2, "fd did not equal ufd2 as expected.", state);
        AssertEqual(ufd2, fd, "ufd2 did not equal fd as expected.", state);
    }
    BENCHMARK_LOGD("UniqueFd testUtilsUniqueFdeqcompare end.");
}

BENCHMARK_F(BenchmarkUniqueFd, testUtilsUniqueFdeqcompareNl)(benchmark::State& state)
{
    BENCHMARK_LOGD("UniqueFd testUtilsUniqueFdeqcompareNl start.");
    while (state.KeepRunning()) {
        int fd = open(TEST_FILE_NAME, O_RDWR, FILE_PERMISSION_READ_WRITE);
        UniqueFd ufd2(fd);
        AssertGreaterThanOrEqual(ufd2, 0, "ufd2 >= 0 did not equal true as expected.", state);
        AssertLessThanOrEqual(0, ufd2, "0 <= ufd2 did not equal true as expected.", state);
    }
    BENCHMARK_LOGD("UniqueFd testUtilsUniqueFdeqcompareNl end.");
}

BENCHMARK_F(BenchmarkUniqueFd, testUtilsUniqueFdeqcompareBg)(benchmark::State& state)
{
    BENCHMARK_LOGD("UniqueFd testUtilsUniqueFdeqcompareBg start.");
    while (state.KeepRunning()) {
        int fd = open(TEST_FILE_NAME, O_RDWR, FILE_PERMISSION_READ_WRITE);
        UniqueFd ufd2(fd);
        AssertGreaterThan(ufd2, MIN_VALID_FD, "ufd2 > MIN_VALID_FD did not equal true as expected.", state);
        AssertLessThan(MIN_VALID_FD, ufd2, "MIN_VALID_FD < ufd2 did not equal true as expected.", state);
    }
    BENCHMARK_LOGD("UniqueFd testUtilsUniqueFdeqcompareBg end.");
}

BENCHMARK_F(BenchmarkUniqueFd, testUtilsUniqueFdeqcompareNb)(benchmark::State& state)
{
    BENCHMARK_LOGD("UniqueFd testUtilsUniqueFdeqcompareNb start.");
    while (state.KeepRunning()) {
        int fd = open(TEST_FILE_NAME, O_RDWR, FILE_PERMISSION_READ_WRITE);
        UniqueFd ufd2(fd);
        AssertLessThanOrEqual(ufd2, MAX_VALID_FD, "ufd2 <= MAX_VALID_FD did not equal true as expected.", state);
        AssertGreaterThanOrEqual(MAX_VALID_FD, ufd2, "MAX_VALID_FD >= ufd2 did not equal true as expected.", state);
    }
    BENCHMARK_LOGD("UniqueFd testUtilsUniqueFdeqcompareNb end.");
}

BENCHMARK_F(BenchmarkUniqueFd, testUtilsUniqueFdeqcompareLess)(benchmark::State& state)
{
    BENCHMARK_LOGD("UniqueFd testUtilsUniqueFdeqcompareLess start.");
    while (state.KeepRunning()) {
        int fd = open(TEST_FILE_NAME, O_RDWR, FILE_PERMISSION_READ_WRITE);
        UniqueFd ufd2(fd);
        AssertLessThan(ufd2, MAX_VALID_FD, "ufd2 < MAX_VALID_FD did not equal true as expected.", state);
        AssertGreaterThan(MAX_VALID_FD, ufd2, "MAX_VALID_FD > ufd2 did not equal true as expected.", state);
    }
    BENCHMARK_LOGD("UniqueFd testUtilsUniqueFdeqcompareLess end.");
}

BENCHMARK_F(BenchmarkUniqueFd, testUtilsUniqueFdeqcompareNeq)(benchmark::State& state)
{
    BENCHMARK_LOGD("UniqueFd testUtilsUniqueFdeqcompareNeq start.");
    while (state.KeepRunning()) {
        int fd = open(TEST_FILE_NAME, O_RDWR, FILE_PERMISSION_READ_WRITE);
        UniqueFd ufd2(fd);
        AssertUnequal(ufd2, MAX_VALID_FD, "ufd2 != MAX_VALID_FD did not equal true as expected.", state);
        AssertUnequal(MAX_VALID_FD, ufd2, "MAX_VALID_FD != ufd2 did not equal true as expected.", state);
    }
    BENCHMARK_LOGD("UniqueFd testUtilsUniqueFdeqcompareNeq end.");
}

class NewDeleter {
public:
    static int iflag;
    static void Close(int fd)
    {
        BENCHMARK_LOGD("UniqueFd static void Close is called.");

        iflag = 10; // give the expected fd an arbitrary value other than 0.
        close(fd);
    }
};

int NewDeleter::iflag = 0;

BENCHMARK_F(BenchmarkUniqueFd, testUtilsUniqueFdDefineDeletor)(benchmark::State& state)
{
    BENCHMARK_LOGD("UniqueFd testUtilsUniqueFdDefineDeletor start.");
    const int initialFlagValue = 0;
    const int expectedFlagValueAfterDelete = 10;
    const int invalidFileDescriptor = -1;
    while (state.KeepRunning()) {
        NewDeleter::iflag = 0;
        int fd = open(TEST_FILE_NAME, O_RDWR);

        {
            UniqueFdAddDeletor<NewDeleter> ufd2(fd);
            AssertEqual(NewDeleter::iflag, initialFlagValue,
                "NewDeleter::iflag did not equal 0 as expected.", state);
            BENCHMARK_LOGD("UniqueFd testUtilsUniqueFdDefineDeletor NewDeleter::iflag: %{public}d", NewDeleter::iflag);
            AssertUnequal(ufd2, invalidFileDescriptor, "open test.h error", state);
        }
        AssertEqual(NewDeleter::iflag, expectedFlagValueAfterDelete,
            "NewDeleter::iflag did not equal expectedFlagValueAfterDelete as expected.", state);
    }
    BENCHMARK_LOGD("UniqueFd testUtilsUniqueFdDefineDeletor end.");
}

BENCHMARK_F(BenchmarkUniqueFd, testUtilsUniqueFdDefineDeletorCloseStatus)(benchmark::State& state)
{
    BENCHMARK_LOGD("UniqueFd testUtilsUniqueFdDefineDeletorCloseStatus start.");
    const int errorReturn = -1;
    while (state.KeepRunning()) {
        int fd = open(TEST_FILE_NAME, O_RDWR);

        {
            UniqueFdAddDeletor<NewDeleter> ufd2(fd);
        }

        char buf[] = "test";
        int ret = write(fd, buf, sizeof(buf));
        AssertEqual(ret, errorReturn, "ret did not equal errorReturn as expected.", state);
    }
    BENCHMARK_LOGD("UniqueFd testUtilsUniqueFdDefineDeletorCloseStatus end.");
}

BENCHMARK_F(BenchmarkUniqueFd, testUtilsUniqueFdRelease)(benchmark::State& state)
{
    BENCHMARK_LOGD("UniqueFd testUtilsUniqueFdRelease start.");
    const int expectedValue = -1;
    while (state.KeepRunning()) {
        UniqueFd ufd(open(TEST_FILE_NAME, O_RDWR, FILE_PERMISSION_READ_WRITE));
        AssertUnequal(ufd, expectedValue, "ufd was not different from expectedValue as expected.", state);

        int fd = ufd.Release();
        AssertUnequal(fd, expectedValue, "fd was not different from expectedValue as expected.", state);
        DefaultDeleter::Close(fd);
    }
    BENCHMARK_LOGD("UniqueFd testUtilsUniqueFdRelease end.");
}

BENCHMARK_F(BenchmarkUniqueFd, testUtilsUniqueFdOperatorInt)(benchmark::State& state)
{
    BENCHMARK_LOGD("UniqueFd testUtilsUniqueFdOperatorInt start.");
    const int expectedValue = -1;
    while (state.KeepRunning()) {
        UniqueFd ufd(open(TEST_FILE_NAME, O_RDWR, FILE_PERMISSION_READ_WRITE));
        AssertUnequal(ufd, expectedValue, "ufd was not different from expectedValue as expected.", state);

        int fd = ufd;
        AssertEqual(fd, ufd, "fd did not equal ufd as expected.", state);
    }
    BENCHMARK_LOGD("UniqueFd testUtilsUniqueFdOperatorInt end.");
}

BENCHMARK_F(BenchmarkUniqueFd, testUtilsUniqueFdGet)(benchmark::State& state)
{
    BENCHMARK_LOGD("UniqueFd testUtilsUniqueFdGet start.");
    const int expectedValue = -1;
    while (state.KeepRunning()) {
        UniqueFd ufd(open(TEST_FILE_NAME, O_RDWR, FILE_PERMISSION_READ_WRITE));
        AssertUnequal(ufd, expectedValue, "ufd was not different from expectedValue as expected.", state);

        int fd = ufd.Get();
        AssertEqual(fd, ufd, "fd did not equal ufd as expected.", state);
    }
    BENCHMARK_LOGD("UniqueFd testUtilsUniqueFdGet end.");
}

BENCHMARK_F(BenchmarkUniqueFd, testUtilsUniqueFdMoveConstructor)(benchmark::State& state)
{
    BENCHMARK_LOGD("UniqueFd testUtilsUniqueFdMoveConstructor start.");
    const int expectedValue = -1;
    while (state.KeepRunning()) {
        UniqueFd ufd(open(TEST_FILE_NAME, O_RDWR, FILE_PERMISSION_READ_WRITE));
        AssertUnequal(ufd, expectedValue, "ufd was not different from expectedValue as expected.", state);

        UniqueFd ufd1(std::move(ufd));
        AssertEqual(ufd, expectedValue, "ufd did not equal expectedValue as expected.", state);
        AssertUnequal(ufd1, expectedValue, "ufd1 was not different from expectedValue as expected.", state);
    }
    BENCHMARK_LOGD("UniqueFd testUtilsUniqueFdMoveConstructor end.");
}

BENCHMARK_F(BenchmarkUniqueFd, testUtilsUniqueFdMoveAssignment)(benchmark::State& state)
{
    BENCHMARK_LOGD("UniqueFd testUtilsUniqueFdMoveAssignment start.");
    const int expectedValue = -1;
    while (state.KeepRunning()) {
        UniqueFd ufd(open(TEST_FILE_NAME, O_RDWR, FILE_PERMISSION_READ_WRITE));
        AssertUnequal(ufd, expectedValue, "ufd was not different from expectedValue as expected.", state);

        UniqueFd ufd1;
        ufd1 = std::move(ufd);
        AssertEqual(ufd, expectedValue, "ufd did not equal expectedValue as expected.", state);
        AssertUnequal(ufd1, expectedValue, "ufd1 was not different from expectedValue as expected.", state);
    }
    BENCHMARK_LOGD("UniqueFd testUtilsUniqueFdMoveAssignment end.");
}
}  // namespace
}  // namespace OHOS
// Run the benchmark
BENCHMARK_MAIN();