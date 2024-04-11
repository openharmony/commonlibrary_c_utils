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
#include "mapped_file.h"
#include <fstream>
#include <iostream>
#include <sys/stat.h>
#include <unistd.h>
#include "common_mapped_file_errors.h"
#include "directory_ex.h"
#include "errors.h"
#include "file_ex.h"
#include "benchmark_log.h"
#include "benchmark_assert.h"
using namespace OHOS::Utils;

namespace OHOS {
namespace {

static constexpr char BASE_PATH[] = "/data/test/commonlibrary_c_utils/";
static constexpr char SUITE_PATH[] = "mapped_file/";
const int CONSTANT_ZERO = 0;

class BenchmarkMappedFileTest : public benchmark::Fixture {
public:
    BenchmarkMappedFileTest()
    {
        Iterations(iterations);
        Repetitions(repetitions);
        ReportAggregatesOnly();
    }

    ~BenchmarkMappedFileTest() override = default;
    void SetUp(const ::benchmark::State& state) override
    {
        std::string dir = std::string(BASE_PATH).append(SUITE_PATH);
        if (ForceCreateDirectory(dir)) {
            BENCHMARK_LOGD("Create test dir: %{public}s", dir.c_str());
        } else {
            BENCHMARK_LOGD("Create test dir Failed: %{public}s", dir.c_str());
        }
    }

    void TearDown(const ::benchmark::State& state) override
    {
        if (ForceRemoveDirectory(std::string(BASE_PATH))) {
            BENCHMARK_LOGD("Remove test dir: %{public}s", BASE_PATH);
        }
    }

protected:
    const int32_t repetitions = 3;
    const int32_t iterations = 1000;
};

void PrintStatus(MappedFile& mf)
{
    BENCHMARK_LOGD("MappedFileTest void PrintStatus is called.");
    BENCHMARK_LOGD("Mapped Region Start: %{public}p\n"
                   "Mapped Region End: %{public}p\n"
                   "View start: %{public}p\n"
                   "View End: %{public}p\n",
                   reinterpret_cast<void*>(mf.RegionStart()),
                   reinterpret_cast<void*>(mf.RegionEnd()),
                   reinterpret_cast<void*>(mf.Begin()),
                   reinterpret_cast<void*>(mf.End()));
}

bool CreateTestFile(const std::string& path, const std::string& content)
{
    BENCHMARK_LOGD("MappedFileTest bool CreateTestFile is called.");
    std::ofstream out(path, std::ios_base::out | std::ios_base::trunc);
    if (out.is_open()) {
        out << content.c_str();
        return true;
    }

    return false;
}

int RemoveTestFile(const std::string& path)
{
    BENCHMARK_LOGD("MappedFileTest int RemoveTestFile is called.");
    return unlink(path.c_str());
}

bool SaveStringToFile(const std::string& filePath, const std::string& content, off_t offset, bool truncated /*= true*/)
{
    BENCHMARK_LOGD("MappedFileTest bool SaveStringToFile is called.");
    if (content.empty()) {
        return true;
    }

    std::ofstream file;
    if (truncated) {
        file.open(filePath.c_str(), std::ios::out | std::ios::trunc);
    } else {
        file.open(filePath.c_str(), std::ios::out | std::ios::app);
    }

    if (!file.is_open()) {
        return false;
    }

    file.seekp(offset, std::ios::beg);

    file.write(content.c_str(), content.length());
    if (file.fail()) {
        return false;
    }
    return true;
}

void CreateFile(std::string& filename, std::string& content, benchmark::State& state)
{
    filename.insert(0, SUITE_PATH).insert(0, BASE_PATH);
    RemoveTestFile(filename);

    AssertTrue((CreateTestFile(filename, content)),
        "CreateTestFile(filename, content) did not equal true as expected.", state);
}

void ReadFromMappedFile(std::string& content, MappedFile& mf, benchmark::State& state)
{
    std::string readout;
    for (char* cur = mf.Begin(); cur <= mf.End(); cur++) {
        readout.push_back(*cur);
    }
    AssertEqual(readout, content, "readout did not equal content as expected.", state);
}

void WriteToMappedFile(std::string& toWrite, MappedFile& mf)
{
    char* newCur = mf.Begin();
    for (std::string::size_type i = 0; i < toWrite.length(); i++) {
        (*newCur) = toWrite[i];
        newCur++;
    }
}

/*
 * @tc.name: testDefaultMapping001
 * @tc.desc: Test file mapping with default params.
 */
BENCHMARK_F(BenchmarkMappedFileTest, testDefaultMapping001)(benchmark::State& state)
{
    BENCHMARK_LOGD("MappedFileTest testDefaultMapping001 start.");
    while (state.KeepRunning()) {
        // 1. Create a new file
        std::string filename = "test_read_write_1.txt";
        std::string content = "Test for normal use.";
        CreateFile(filename, content, state);

        // 2. map file
        MappedFile mf(filename);
        AssertEqual(mf.Map(), MAPPED_FILE_ERR_OK, "mf.Map() did not equal MAPPED_FILE_ERR_OK as expected.", state);

        // check status
        AssertTrue((mf.IsMapped()), "mf.IsMapped() did not equal true as expected.", state);
        AssertTrue((mf.IsNormed()), "mf.IsNormed() did not equal true as expected.", state);

        // check size
        struct stat stb = {0};
        stat(filename.c_str(), &stb);
        AssertTrue((stb.st_size == mf.Size() || mf.PageSize() == mf.Size()),
            "stb.st_size == mf.Size() || mf.PageSize() == mf.Size() did not equal true as expected.", state);

        // check map-mode
        AssertEqual(MapMode::DEFAULT, mf.GetMode(), "MapMode::DEFAULT did not equal mf.GetMode() as expected.", state);

        // check offset
        AssertEqual(mf.StartOffset(), 0u, "mf.StartOffset() did not equal 0u as expected.", state);

        // 3. read from mapped file
        ReadFromMappedFile(content, mf, state);

        // 4. write to mapped file
        std::string toWrite("Complete.");
        WriteToMappedFile(toWrite, mf);
        std::string res;
        LoadStringFromFile(filename, res);
        AssertEqual(res, "Complete.normal use.", "res did not equal \"Complete.normal use.\" as expected.", state);

        // 5. test default mapping and write to addr which excess End() but not across this memory page.
        AssertLessThanOrEqual(mf.Size(), mf.PageSize(),
            "mf.Size() was not less than or equal to mf.PageSize() as expected.", state);
        char* trueEnd = mf.RegionEnd();
        AssertGreaterThan(trueEnd, mf.Begin(), "trueEnd was not greater than mf.Begin() as expected.", state);
        // write to mapped file
        (*trueEnd) = 'E'; // It is allowed to write to this address which excess the End()

        AssertEqual((*trueEnd), 'E', "(*trueEnd) did not equal 'E' as expected.", state);

        std::string res1;
        LoadStringFromFile(filename, res1);
        AssertEqual(res1, "Complete.normal use.", "res1 did not equal \"Complete.normal use.\" as expected.", state);

        RemoveTestFile(filename);
    }
    BENCHMARK_LOGD("MappedFileTest testDefaultMapping001 end.");
}

/*
 * @tc.name: testNewSharedMappingDefaultSize001
 * @tc.desc: Test mapping which will create a new file with default size.
 */
BENCHMARK_F(BenchmarkMappedFileTest, testNewSharedMappingDefaultSize001)(benchmark::State& state)
{
    BENCHMARK_LOGD("MappedFileTest testNewSharedMappingDefaultSize001 start.");
    while (state.KeepRunning()) {
        // 1. Create a new file
        std::string filename = "test_read_write_2.txt";
        filename.insert(0, SUITE_PATH).insert(0, BASE_PATH);
        RemoveTestFile(filename);

        // 2. map file
        MappedFile mf(filename, MapMode::DEFAULT | MapMode::CREATE_IF_ABSENT);
        AssertEqual(mf.Map(), MAPPED_FILE_ERR_OK, "mf.Map() did not equal MAPPED_FILE_ERR_OK as expected.", state);

        // check if file is created
        AssertTrue((FileExists(filename)), "FileExists(filename) did not equal true as expected.", state);

        // check status
        AssertTrue((mf.IsMapped()), "mf.IsMapped() did not equal true as expected.", state);
        AssertTrue((mf.IsNormed()), "mf.IsNormed() did not equal true as expected.", state);

        // check map-mode
        AssertEqual((MapMode::DEFAULT | MapMode::CREATE_IF_ABSENT), mf.GetMode(),
            "(MapMode::DEFAULT | MapMode::CREATE_IF_ABSENT) did not equal mf.GetMode() as expected.", state);

        // check default size
        struct stat stb = {0};
        if (stat(filename.c_str(), &stb) == 0) {
            AssertEqual(stb.st_size, mf.PageSize(), "stb.st_size did not equal mf.PageSize() as expected.", state);
        }
        AssertEqual(mf.Size(), mf.PageSize(), "mf.Size() did not equal mf.PageSize() as expected.", state);

        // 3. write to mapped file
        std::string toWrite("Write to newly created file.");
        WriteToMappedFile(toWrite, mf);
        std::string res;
        LoadStringFromFile(filename, res);
        AssertEqual(strcmp(res.c_str(), toWrite.c_str()), 0,
            "The two strings, res.c_str() and toWrite.c_str(), did not have the same content as expected.", state);
                                                    // use c_str() to compare conveniently.

        // 4. read from mapped file
        std::string toRead("Waiting to be read.");
        SaveStringToFile(filename, toRead, 0, true);
        std::string readout;
        for (char* cur = mf.Begin(); *cur != '\0'; cur++) {
            readout.push_back(*cur);
        }
        AssertEqual(readout, toRead, "readout did not equal toRead as expected.", state);

        RemoveTestFile(filename);
    }
    BENCHMARK_LOGD("MappedFileTest testNewSharedMappingDefaultSize001 end.");
}

/*
 * @tc.name: testNewSharedMapping001
 * @tc.desc: Test mapping which will create a new file with specified params.
 */
BENCHMARK_F(BenchmarkMappedFileTest, testNewSharedMapping001)(benchmark::State& state)
{
    BENCHMARK_LOGD("MappedFileTest testNewSharedMapping001 start.");
    while (state.KeepRunning()) {
        std::string filename = "test_read_write_3.txt";
        filename.insert(0, SUITE_PATH).insert(0, BASE_PATH);
        RemoveTestFile(filename);

        // set params
        // new mapping region will not guaranteed to be located at `hint`
        char* hint = reinterpret_cast<char*>(0x80000); // 0x80000: hint(expected address).
        off_t size = 1024;
        off_t offset = 4 * 1024;

        // 1. map a non-existed file
        MappedFile mf(filename, MapMode::DEFAULT | MapMode::CREATE_IF_ABSENT, offset, size, hint);
        AssertEqual(mf.Map(), MAPPED_FILE_ERR_OK, "mf.Map() did not equal MAPPED_FILE_ERR_OK as expected.", state);

        // check if file is created
        AssertTrue((FileExists(filename)), "FileExists(filename) did not equal true as expected.", state);

        // check status
        AssertTrue((mf.IsMapped()), "mf.IsMapped() did not equal true as expected.", state);
        AssertTrue((mf.IsNormed()), "mf.IsNormed() did not equal true as expected.", state);

        // check specified size
        struct stat stb = {0};
        if (stat(filename.c_str(), &stb) == 0) {
            // Exact file size should be offset + mapped size, contents will be zero-filled.
            AssertEqual(stb.st_size, offset + size, "stb.st_size did not equal offset + size as expected.", state);
        }
        AssertEqual(mf.Size(), size, "mf.Size() did not equal size as expected.", state);

        // check specified offset
        AssertEqual(mf.StartOffset(), offset, "mf.StartOffset() did not equal offset as expected.", state);

        // check hint
        AssertTrue((mf.GetHint() == nullptr || mf.GetHint() == hint),
            "(mf.GetHint() == nullptr || mf.GetHint() == hint) did not equal true as expected.", state);

        // 2. write to mapped file
        std::string toWrite("Write to newly created file.");
        WriteToMappedFile(toWrite, mf);
        AssertTrue((StringExistsInFile(filename, toWrite)),
            "StringExistsInFile(filename, toWrite) did not equal true as expected.", state);

        // 3. read from mapped file
        std::string toRead("Waiting to be read.");
        SaveStringToFile(filename, toRead, offset, true);
        std::string readout;
        for (char* cur = mf.Begin(); cur <= mf.End() && *cur != '\0'; cur++) {
            readout.push_back(*cur);
        }
        AssertEqual(readout, toRead, "readout did not equal toRead as expected.", state);

        RemoveTestFile(filename);
    }
    BENCHMARK_LOGD("MappedFileTest testNewSharedMapping001 end.");
}

/*
 * @tc.name: testPrivateMapping001
 * @tc.desc: Test mapping which will create a new file with specified params.
 */
BENCHMARK_F(BenchmarkMappedFileTest, testPrivateMapping001)(benchmark::State& state)
{
    BENCHMARK_LOGD("MappedFileTest testPrivateMapping001 start.");
    while (state.KeepRunning()) {
        // 1. create a new file
        std::string filename = "test_read_write_4.txt";
        std::string content = "Test for private use.";
        CreateFile(filename, content, state);

        // 2. map file
        MappedFile mf(filename, MapMode::DEFAULT | MapMode::PRIVATE);
        AssertEqual(mf.Map(), MAPPED_FILE_ERR_OK, "mf.Map() did not equal MAPPED_FILE_ERR_OK as expected.", state);

        // 3. check status
        AssertTrue((mf.IsMapped()), "mf.IsMapped() did not equal true as expected.", state);
        AssertTrue((mf.IsNormed()), "mf.IsNormed() did not equal true as expected.", state);

        // 4. read from mapped file
        ReadFromMappedFile(content, mf, state);

        // 5. write to mapped file
        std::string toWrite("Complete.");
        WriteToMappedFile(toWrite, mf);
        std::string res;
        LoadStringFromFile(filename, res);
        AssertEqual(res, content, "res did not equal content as expected.", state);

        RemoveTestFile(filename);
    }
    BENCHMARK_LOGD("MappedFileTest testPrivateMapping001 end.");
}

/*
 * @tc.name: testSharedReadOnlyMapping001
 * @tc.desc: Test mapping which will create a new file with specified params.
 */
BENCHMARK_F(BenchmarkMappedFileTest, testSharedReadOnlyMapping001)(benchmark::State& state)
{
    BENCHMARK_LOGD("MappedFileTest testSharedReadOnlyMapping001 start.");
    while (state.KeepRunning()) {
        // 1. create a new file
        std::string filename = "test_read_write_5.txt";
        std::string content = "Test for readonly use.";
        CreateFile(filename, content, state);

        // 2. map file
        MappedFile mf(filename, MapMode::DEFAULT | MapMode::READ_ONLY);
        AssertEqual(mf.Map(), MAPPED_FILE_ERR_OK, "mf.Map() did not equal MAPPED_FILE_ERR_OK as expected.", state);

        // 3. check status
        AssertTrue((mf.IsMapped()), "mf.IsMapped() did not equal true as expected.", state);
        AssertTrue((mf.IsNormed()), "mf.IsNormed() did not equal true as expected.", state);

        // 4. read from mapped file
        ReadFromMappedFile(content, mf, state);
        // !Note: write operation is not permitted, which will raise a signal 11.

        RemoveTestFile(filename);
    }
    BENCHMARK_LOGD("MappedFileTest testSharedReadOnlyMapping001 end.");
}

/*
 * @tc.name: testReMap001
 * @tc.desc: Test remapping using `Unmap()` and `Map()`
 */
BENCHMARK_F(BenchmarkMappedFileTest, testReMap001)(benchmark::State& state)
{
    BENCHMARK_LOGD("MappedFileTest testReMap001 start.");
    while (state.KeepRunning()) {
        // 1. create a new file
        std::string filename = "test_remap_1.txt";
        std::string content = "Test for remapping use.";
        CreateFile(filename, content, state);

        // 2. map file
        MappedFile mf(filename);
        AssertEqual(mf.Map(), MAPPED_FILE_ERR_OK, "mf.Map() did not equal MAPPED_FILE_ERR_OK as expected.", state);

        // 3. check status after mapping
        AssertTrue((mf.IsMapped()), "mf.IsMapped() did not equal true as expected.", state);
        AssertTrue((mf.IsNormed()), "mf.IsNormed() did not equal true as expected.", state);

        AssertEqual(mf.Unmap(), MAPPED_FILE_ERR_OK,
            "mf.Unmap() did not equal MAPPED_FILE_ERR_OK as expected.", state);

        // 4. check status after unmapping
        AssertFalse((mf.IsMapped()), "mf.IsMapped() did not equal false as expected.", state);
        AssertTrue((mf.IsNormed()), "mf.IsNormed() did not equal true as expected.", state);
        AssertEqual(mf.Begin(), nullptr, "mf.Begin() did not equal nullptr as expected.", state);

        AssertEqual(mf.Map(), MAPPED_FILE_ERR_OK, "mf.Map() did not equal MAPPED_FILE_ERR_OK as expected.", state);
        // 5. check status after remapping
        AssertTrue((mf.IsMapped()), "mf.IsMapped() did not equal true as expected.", state);
        AssertTrue((mf.IsNormed()), "mf.IsNormed() did not equal true as expected.", state);

        // 6. check default size
        struct stat stb = {0};
        stat(filename.c_str(), &stb);
        AssertTrue((stb.st_size == mf.Size() || mf.PageSize() == mf.Size()),
            "stb.st_size == mf.Size() || mf.PageSize() == mf.Size() did not equal true as expected.", state);

        RemoveTestFile(filename);
    }
    BENCHMARK_LOGD("MappedFileTest testReMap001 end.");
}

static void ChangeParamsWhenUnmapped(MappedFile& mf, const int sizeIncrement, std::string& filename,
                                     std::string& filename1, benchmark::State& state)
{
    AssertTrue((mf.ChangeSize(mf.Size() + sizeIncrement)),
        "mf.ChangeSize(mf.Size() + sizeIncrement) did not equal true as expected.", state);
    AssertTrue((mf.ChangeSize(MappedFile::DEFAULT_LENGTH)),
        "mf.ChangeSize(MappedFile::DEFAULT_LENGTH) did not equal true as expected.", state);
    AssertTrue((mf.ChangeOffset(mf.PageSize())),
        "mf.ChangeOffset(mf.PageSize()) did not equal true as expected.", state);
    AssertTrue((mf.ChangeOffset(0)), "mf.ChangeOffset(0) did not equal true as expected.", state);
    AssertTrue((mf.ChangePath(filename1)), "mf.ChangePath(filename1) did not equal true as expected.", state);
    AssertTrue((mf.ChangePath(filename)), "mf.ChangePath(filename) did not equal true as expected.", state);
    AssertTrue((mf.ChangeHint(reinterpret_cast<char*>(0x89000))),
        "mf.ChangeHint(reinterpret_cast<char*>(0x89000)) did not equal true as expected.", state);
    AssertTrue((mf.ChangeMode(MapMode::READ_ONLY)),
        "mf.ChangeMode(MapMode::READ_ONLY) did not equal true as expected.", state);
}

/*
 * @tc.name: testReMap002
 * @tc.desc: Test remapping via changing params.
 */
BENCHMARK_F(BenchmarkMappedFileTest, testReMap002)(benchmark::State& state)
{
    BENCHMARK_LOGD("MappedFileTest testReMap002 start.");
    const int sizeIncrement = 1024;
    while (state.KeepRunning()) {
        // 1. create a new file
        std::string filename = "test_remap.txt";
        std::string content = "Test for default use.";
        CreateFile(filename, content, state);

        std::string filename1 = "test_remap_1.txt";
        std::string content1 = "Test for remapping use.";
        CreateFile(filename1, content1, state);

        MappedFile mf(filename);

        // Change params when unmapped.
        ChangeParamsWhenUnmapped(mf, sizeIncrement, filename, filename1, state);

        // 2. map file
        AssertEqual(mf.Map(), MAPPED_FILE_ERR_OK, "mf.Map() did not equal MAPPED_FILE_ERR_OK as expected.", state);

        // 3. check status after mapping
        AssertTrue((mf.IsMapped()), "mf.IsMapped() did not equal true as expected.", state);
        AssertTrue((mf.IsNormed()), "mf.IsNormed() did not equal true as expected.", state);

        // 4. check size
        struct stat stb = {0};
        stat(filename.c_str(), &stb);
        AssertTrue((stb.st_size == mf.Size() || mf.PageSize() == mf.Size()),
            "stb.st_size == mf.Size() || mf.PageSize() == mf.Size() did not equal true as expected.", state);

        // 5. read from Mapped File
        ReadFromMappedFile(content, mf, state);

        // 6. change params
        AssertTrue((mf.ChangePath(filename1)), "mf.ChangePath(filename1) did not equal true as expected.", state);
        AssertTrue((mf.ChangeSize(MappedFile::DEFAULT_LENGTH)),
            "mf.ChangeSize(MappedFile::DEFAULT_LENGTH) did not equal true as expected.", state);
        AssertTrue((mf.ChangeHint(reinterpret_cast<char*>(0x80000))),
            "mf.ChangeHint(reinterpret_cast<char*>(0x80000)) did not equal true as expected.", state);
        AssertTrue((mf.ChangeMode(MapMode::DEFAULT | MapMode::CREATE_IF_ABSENT)),
            "mf.ChangeMode(MapMode::DEFAULT | MapMode::CREATE_IF_ABSENT) did not equal true as expected.", state);

        // 7. check status after changing
        AssertFalse((mf.IsMapped()), "mf.IsMapped() did not equal false as expected.", state);
        AssertFalse((mf.IsNormed()), "mf.IsNormed() did not equal false as expected.", state);

        // 8. remap file
        AssertEqual(mf.Map(), MAPPED_FILE_ERR_OK, "mf.Map() did not equal MAPPED_FILE_ERR_OK as expected.", state);
        // 9. check status after remapping
        AssertTrue((mf.IsMapped()), "mf.IsMapped() did not equal true as expected.", state);
        AssertTrue((mf.IsNormed()), "mf.IsNormed() did not equal true as expected.", state);

        // 10. check size
        stat(filename1.c_str(), &stb);
        AssertEqual(stb.st_size, mf.Size(), "stb.st_size == mf.Size() did not equal true as expected.", state);

        // 11. read from Mapped File
        ReadFromMappedFile(content1, mf, state);

        RemoveTestFile(filename);
        RemoveTestFile(filename1);
    }
    BENCHMARK_LOGD("MappedFileTest testReMap002 end.");
}

/*
 * @tc.name: testReMap003
 * @tc.desc: Test remapping via Resize().
 */
BENCHMARK_F(BenchmarkMappedFileTest, testReMap003)(benchmark::State& state)
{
    BENCHMARK_LOGD("MappedFileTest testReMap003 start.");
    while (state.KeepRunning()) {
        // 1. create a new file
        std::string filename = "test_remap.txt";
        std::string content = "Test for default use.";
        CreateFile(filename, content, state);

        std::string filename1 = "test_remap_1.txt";
        std::string content1 = "Test for remapping use.";
        CreateFile(filename1, content1, state);

        // 2. map file
        MappedFile mf(filename);
        AssertEqual(mf.Map(), MAPPED_FILE_ERR_OK, "mf.Map() did not equal MAPPED_FILE_ERR_OK as expected.", state);

        // 3. check status after mapping
        AssertTrue((mf.IsMapped()), "mf.IsMapped() did not equal true as expected.", state);
        AssertTrue((mf.IsNormed()), "mf.IsNormed() did not equal true as expected.", state);

        // 4. check size
        struct stat stb = {0};
        stat(filename.c_str(), &stb);
        AssertTrue((stb.st_size == mf.Size() || mf.PageSize() == mf.Size()),
            "stb.st_size == mf.Size() || mf.PageSize() == mf.Size() did not equal true as expected.", state);

        // 5. read from Mapped File
        ReadFromMappedFile(content, mf, state);

        // 6. change params
        mf.ChangePath(filename1);
        mf.ChangeSize(MappedFile::DEFAULT_LENGTH);

        // 7. check status after changing
        AssertFalse((mf.IsMapped()), "mf.IsMapped() did not equal false as expected.", state);
        AssertFalse((mf.IsNormed()), "mf.IsNormed() did not equal false as expected.", state);

        // 8. remap file
        AssertEqual(mf.Resize(), MAPPED_FILE_ERR_OK,
            "mf.Resize() did not equal MAPPED_FILE_ERR_OK as expected.", state);
        // 9. check status after remapping
        AssertTrue((mf.IsMapped()), "mf.IsMapped() did not equal true as expected.", state);
        AssertTrue((mf.IsNormed()), "mf.IsNormed() did not equal true as expected.", state);

        // 10. check size
        stat(filename1.c_str(), &stb);
        AssertEqual(stb.st_size, mf.Size(), "stb.st_size == mf.Size() did not equal true as expected.", state);

        // 11. read from Mapped File
        ReadFromMappedFile(content1, mf, state);

        RemoveTestFile(filename);
        RemoveTestFile(filename1);
    }
    BENCHMARK_LOGD("MappedFileTest testReMap003 end.");
}

/*
 * @tc.name: testReMap004
 * @tc.desc: Test remapping only to extend mapped region via Resize(off_t, bool).
 */
BENCHMARK_F(BenchmarkMappedFileTest, testReMap004)(benchmark::State& state)
{
    BENCHMARK_LOGD("MappedFileTest testReMap004 start.");
    while (state.KeepRunning()) {
        // 1. create a new file
        std::string filename = "test_remap.txt";
        std::string content = "Test for remapping use.";
        CreateFile(filename, content, state);

        // 2. map file
        MappedFile mf(filename);
        AssertEqual(mf.Map(), MAPPED_FILE_ERR_OK, "mf.Map() did not equal MAPPED_FILE_ERR_OK as expected.", state);

        // 3. check status after mapping
        AssertTrue((mf.IsMapped()), "mf.IsMapped() did not equal true as expected.", state);
        AssertTrue((mf.IsNormed()), "mf.IsNormed() did not equal true as expected.", state);

        // 4. check size
        struct stat stb = {0};
        stat(filename.c_str(), &stb);
        AssertTrue((stb.st_size == mf.Size() || mf.PageSize() == mf.Size()),
            "stb.st_size == mf.Size() || mf.PageSize() == mf.Size() did not equal true as expected.", state);

        // 5. read from Mapped File
        char* cur = mf.Begin();
        std::string readout;
        for (; cur <= mf.End(); cur++) {
            readout.push_back(*cur);
        }
        AssertEqual(readout, content, "readout did not equal content as expected.", state);

        // 6. Remap to extend region
        const int newSize = 10;
        AssertEqual(mf.Resize(mf.Size() + newSize), MAPPED_FILE_ERR_OK,
            "mf.Resize(mf.Size() + newSize) did not equal MAPPED_FILE_ERR_OK as expected.", state);
        // 7. check status after remapping
        AssertTrue((mf.IsMapped()), "mf.IsMapped() did not equal true as expected.", state);
        AssertTrue((mf.IsNormed()), "mf.IsNormed() did not equal true as expected.", state);

        // 8. check size after remapping
        stat(filename.c_str(), &stb);
        AssertLessThan(stb.st_size, mf.Size(), "stb.st_size < mf.Size() did not equal true as expected.", state);

        // 9. write to the extended region
        *(cur) = 'E';
        AssertEqual((*cur), 'E', "(*cur) did not equal 'E' as expected.", state);

        std::string res;
        LoadStringFromFile(filename, res);
        AssertEqual(res, content, "res did not equal content as expected.", state);
                                // is larger than substantial size of the file

        RemoveTestFile(filename);
    }
    BENCHMARK_LOGD("MappedFileTest testReMap004 end.");
}

/*
 * @tc.name: testReMap005
 * @tc.desc: Test remapping to extend mapped region as well as substantial file size via Resize(off_t, bool).
 */
BENCHMARK_F(BenchmarkMappedFileTest, testReMap005)(benchmark::State& state)
{
    BENCHMARK_LOGD("MappedFileTest testReMap005 start.");
    while (state.KeepRunning()) {
        // 1. create a new file
        std::string content = "Test for remapping use.";
        std::string filename = "test_remap.txt";
        CreateFile(filename, content, state);

        // 2. map file
        MappedFile mf(filename);
        AssertEqual(mf.Map(), MAPPED_FILE_ERR_OK, "mf.Map() did not equal MAPPED_FILE_ERR_OK as expected.", state);

        // 3. check status after mapping
        AssertTrue((mf.IsNormed()), "mf.IsNormed() did not equal true as expected.", state);
        AssertTrue((mf.IsMapped()), "mf.IsMapped() did not equal true as expected.", state);

        // 4. check size
        struct stat stb = {0};
        stat(filename.c_str(), &stb);
        AssertTrue((stb.st_size == mf.Size() || mf.PageSize() == mf.Size()),
            "stb.st_size == mf.Size() || mf.PageSize() == mf.Size() did not equal true as expected.", state);

        // 5. read from Mapped File
        std::string readout;
        char* cur = mf.Begin();
        for (; cur <= mf.End(); cur++) {
            readout.push_back(*cur);
        }
        AssertEqual(readout, content, "readout did not equal content as expected.", state);

        // 6. remap to extend region
        const int newSize = 10;
        AssertEqual(mf.Resize(mf.Size() + newSize, true), MAPPED_FILE_ERR_OK,
            "mf.Resize(mf.Size() + newSize, true) did not equal MAPPED_FILE_ERR_OK as expected.", state);
        // check status after remapping
        AssertTrue((mf.IsMapped()), "mf.IsMapped() did not equal true as expected.", state);
        AssertTrue((mf.IsNormed()), "mf.IsNormed() did not equal true as expected.", state);

        // 7. check size after remapping
        stat(filename.c_str(), &stb);
        AssertEqual(stb.st_size, mf.Size(), "stb.st_size == mf.Size() did not equal true as expected.", state);

        // 8. write to the extended region
        *(cur) = 'E';
        AssertEqual((*cur), 'E', "(*cur) did not equal 'E' as expected.", state);

        std::string res;
        LoadStringFromFile(filename, res);
        AssertEqual(strcmp(res.c_str(), content.append("E").c_str()), 0,
            "The two strings, res.c_str() and content.append(\"E\").c_str(), \
            did not have the same content as expected.", state);
    }
    BENCHMARK_LOGD("MappedFileTest testReMap005 end.");
}

/*
 * @tc.name: testReMap006
 * @tc.desc: Test remapping to via Resize(off_t, bool).
 */
BENCHMARK_F(BenchmarkMappedFileTest, testReMap006)(benchmark::State& state)
{
    BENCHMARK_LOGD("MappedFileTest testReMap006 start.");
    while (state.KeepRunning()) {
        // 1. create a new file
        std::string filename = "test_remap.txt";
        std::string content = "Test for remapping use.";
        CreateFile(filename, content, state);

        // 2. map file
        off_t size = 20;
        MappedFile mf(filename, MapMode::DEFAULT, 0, size);
        AssertEqual(mf.Map(), MAPPED_FILE_ERR_OK, "mf.Map() did not equal MAPPED_FILE_ERR_OK as expected.", state);

        // 3. check status after mapping
        AssertTrue((mf.IsMapped()), "mf.IsMapped() did not equal true as expected.", state);
        AssertTrue((mf.IsNormed()), "mf.IsNormed() did not equal true as expected.", state);

        // 4. check size
        AssertEqual(size, mf.Size(), "size == mf.Size() did not equal true as expected.", state);

        // 5. remap to extend region
        AssertEqual(mf.Resize(MappedFile::DEFAULT_LENGTH, true), MAPPED_FILE_ERR_OK,
            "mf.Resize(MappedFile::DEFAULT_LENGTH, true) did not equal MAPPED_FILE_ERR_OK as expected.", state);
        off_t sizeDifference = 8;
        off_t lessSize = mf.Size() - sizeDifference;
        AssertEqual(mf.Resize(lessSize, true), MAPPED_FILE_ERR_OK,
            "mf.Resize(lessSize, true) did not equal MAPPED_FILE_ERR_OK as expected.", state);
        // check status after remapping
        AssertTrue((mf.IsMapped()), "mf.IsMapped() did not equal true as expected.", state);
        AssertTrue((mf.IsNormed()), "mf.IsNormed() did not equal true as expected.", state);

        // 6. check size after remapping
        struct stat stb = {0};
        stat(filename.c_str(), &stb);
        AssertEqual(lessSize, mf.Size(), "lessSize did not equal mf.Size() as expected.", state);
    }
    BENCHMARK_LOGD("MappedFileTest testReMap006 end.");
}

void KeepAPageAndReachBottom(off_t& endOff, const off_t orig, MappedFile& mf, benchmark::State& state)
{
    AssertUnequal(orig, CONSTANT_ZERO, "The divisor cannot be 0", state);
    if (orig != CONSTANT_ZERO) {
        // keep turnNext within a page
        for (unsigned int cnt = 2; cnt < (MappedFile::PageSize() / orig); cnt++) {
            // 2: start from 2 to take the first, TunrNext() calling in consideration.
            endOff = mf.EndOffset();
            AssertEqual(mf.TurnNext(), MAPPED_FILE_ERR_OK,
                "mf.TurnNext() did not equal MAPPED_FILE_ERR_OK as expected.", state);
            AssertEqual(mf.StartOffset(), endOff + 1, "mf.StartOffset() did not equal endOff + 1 as expected.", state);
            AssertEqual(mf.Size(), orig, "mf.Size() did not equal orig as expected.", state);
        }
        PrintStatus(mf);
    }

    // this turn will reach the bottom of a page
    endOff = mf.EndOffset();
    char* rEnd = mf.RegionEnd();
    char* end = mf.End();
    AssertEqual(mf.TurnNext(), MAPPED_FILE_ERR_OK,
        "mf.TurnNext() did not equal MAPPED_FILE_ERR_OK as expected.", state);
    AssertEqual(mf.StartOffset(), endOff + 1, "mf.StartOffset() did not equal endOff + 1 as expected.", state);
    AssertEqual(mf.Size(), static_cast<off_t>(rEnd - end),
        "mf.Size() did not equal static_cast<off_t>(rEnd - end) as expected.", state);
    PrintStatus(mf);
}

static void Remapping(MappedFile& mf, off_t& endOff, off_t& curSize, benchmark::State& state)
{
    // 7. this turn will trigger a remapping
    endOff = mf.EndOffset();
    AssertEqual(mf.TurnNext(), MAPPED_FILE_ERR_OK,
        "mf.TurnNext() did not equal MAPPED_FILE_ERR_OK as expected.", state);
    AssertTrue((mf.IsMapped()), "mf.IsMapped() did not equal true as expected.", state);
    AssertEqual(mf.StartOffset(), endOff + 1, "mf.StartOffset() did not equal endOff + 1 as expected.", state);
    AssertEqual(mf.Size(), curSize, "mf.Size() did not equal curSize as expected.", state);
    AssertEqual(mf.RegionStart(), mf.Begin(), "mf.RegionStart() did not equal mf.Begin() as expected.", state);
    AssertEqual(static_cast<off_t>(mf.RegionEnd() - mf.RegionStart()) + 1LL, mf.PageSize(),
        "static_cast<off_t>(mf.RegionEnd() - mf.RegionStart()) + 1LL did not equal mf.PageSize().", state);
    PrintStatus(mf);
}

/*
 * @tc.name: testTurnNext001
 * @tc.desc: Test TurnNext() when `IsMapped()`.
 */
BENCHMARK_F(BenchmarkMappedFileTest, testTurnNext001)(benchmark::State& state)
{
    BENCHMARK_LOGD("MappedFileTest testTurnNext001 start.");
    while (state.KeepRunning()) {
        // 1. create a new file
        std::string filename = "test_remap.txt";
        std::string content = "Test for remapping use.";
        CreateFile(filename, content, state);

        struct stat stb = {0};
        AssertEqual(stat(filename.c_str(), &stb), 0,
            "stat(filename.c_str(), &stb) did not equal 0 as expected.", state);
        off_t orig = stb.st_size; // 23 bytes

        // 2. extend its size
        int fd = open(filename.c_str(), O_RDWR | O_CLOEXEC);
        AssertUnequal(fd, -1, "fd was not different from -1 as expected.", state);
        AssertEqual(ftruncate(fd, MappedFile::PageSize() + MappedFile::PageSize() / 100LL), 0,
            "ftruncate(fd, MappedFile::PageSize() + MappedFile::PageSize() / 100LL) did not equal 0.", state);

        // 3. map file
        MappedFile mf(filename, MapMode::DEFAULT, 0, orig);
        AssertEqual(mf.Map(), MAPPED_FILE_ERR_OK, "mf.Map() did not equal MAPPED_FILE_ERR_OK as expected.", state);

        // 4. check status after mapping
        AssertTrue((mf.IsMapped()), "mf.IsMapped() did not equal true as expected.", state);
        AssertTrue((mf.IsNormed()), "mf.IsNormed() did not equal true as expected.", state);

        // 5. turn next mapped region with the same size as the file's initial size.
        AssertEqual(mf.TurnNext(), MAPPED_FILE_ERR_OK,
            "mf.TurnNext() did not equal MAPPED_FILE_ERR_OK as expected.", state);
        char* cur = mf.Begin();
        *cur = 'N';

        std::string res;
        LoadStringFromFile(filename, res);
        AssertEqual(strcmp(res.c_str(), content.append("N").c_str()), 0,
            "The two strings, res.c_str() and content.append(\"N\").c_str(), did not have the same content.", state);

        off_t endOff;
        // 6. keep turnNext within a page,and reach the bottom of a page
        KeepAPageAndReachBottom(endOff, orig, mf, state);

        // 7. this turn will trigger a remapping
        off_t curSize = mf.Size();
        Remapping(mf, endOff, curSize, state);

        // 8. keep turnNext within a page
        for (off_t cnt = 1; cnt < (MappedFile::PageSize() / 100LL / curSize); cnt++) {
            endOff = mf.EndOffset();
            AssertEqual(mf.TurnNext(), MAPPED_FILE_ERR_OK,
            "mf.TurnNext() did not equal MAPPED_FILE_ERR_OK as expected.", state);
            AssertEqual(mf.StartOffset(), endOff + 1, "mf.StartOffset() did not equal endOff + 1 as expected.", state);
            AssertEqual(mf.Size(), curSize, "mf.Size() did not equal curSize as expected.", state);
        }

        // 9. this turn will fail since no place remained.
        AssertUnequal(mf.TurnNext(), MAPPED_FILE_ERR_OK,
            "mf.TurnNext() was not different from MAPPED_FILE_ERR_OK as expected.", state);

        RemoveTestFile(filename);
    }
    BENCHMARK_LOGD("MappedFileTest testTurnNext001 end.");
}

/*
 * @tc.name: testTurnNext002
 * @tc.desc: Test TurnNext() when `!IsMapped()`.
 */
BENCHMARK_F(BenchmarkMappedFileTest, testTurnNext002)(benchmark::State& state)
{
    BENCHMARK_LOGD("MappedFileTest testTurnNext002 start.");
    while (state.KeepRunning()) {
        // 1. create a new file
        std::string filename = "test_remap.txt";
        std::string content = "Test for remapping use.";
        CreateFile(filename, content, state);

        // 2. map file
        MappedFile mf(filename);
        AssertEqual(mf.Map(), MAPPED_FILE_ERR_OK, "mf.Map() did not equal MAPPED_FILE_ERR_OK as expected.", state);
        off_t curSize = mf.Size();
        off_t curOff = mf.StartOffset();

        // 3. check status after mapping
        AssertTrue((mf.IsMapped()), "mf.IsMapped() did not equal true as expected.", state);
        AssertTrue((mf.IsNormed()), "mf.IsNormed() did not equal true as expected.", state);
        // 4. recommand to unmap first before other operations on the file.
        AssertEqual(mf.Unmap(), MAPPED_FILE_ERR_OK,
            "mf.Unmap() did not equal MAPPED_FILE_ERR_OK as expected.", state);
        // 5. enlarge file size to make it possible to `turnNext()`.
        AssertEqual(ftruncate(mf.GetFd(), MappedFile::PageSize() + MappedFile::PageSize() / 100LL), 0,
            "ftruncate(mf.GetFd(), MappedFile::PageSize() + MappedFile::PageSize() / 100LL) did not equal 0.", state);
        // 6. turn next page of `PageSize()` and keep the same `size_`
        AssertEqual(mf.TurnNext(), MAPPED_FILE_ERR_OK,
            "mf.TurnNext() did not equal MAPPED_FILE_ERR_OK as expected.", state);
        AssertEqual(mf.Size(), curSize, "mf.Size() did not equal curSize as expected.", state);
        AssertEqual(static_cast<off_t>(mf.StartOffset()), curOff + mf.PageSize(),
            "static_cast<off_t>(mf.StartOffset()) did not equal curOff + mf.PageSize() as expected.", state);

        RemoveTestFile(filename);
    }
    BENCHMARK_LOGD("MappedFileTest testTurnNext002 end.");
}

/*
 * @tc.name: testTurnNext003
 * @tc.desc: Test TurnNext() (using internal fd to `ftruncate()`).
 */
BENCHMARK_F(BenchmarkMappedFileTest, testTurnNext003)(benchmark::State& state)
{
    BENCHMARK_LOGD("MappedFileTest testTurnNext003 start.");
    while (state.KeepRunning()) {
        // 1. create a new file
        std::string filename = "test_remap.txt";
        std::string content = "Test for remapping use.";
        CreateFile(filename, content, state);

        // 2. map file
        MappedFile mf(filename);
        AssertEqual(mf.Map(), MAPPED_FILE_ERR_OK, "mf.Map() did not equal MAPPED_FILE_ERR_OK as expected.", state);

        // 3. check status after mapping
        AssertTrue((mf.IsMapped()), "mf.IsMapped() did not equal true as expected.", state);
        AssertTrue((mf.IsNormed()), "mf.IsNormed() did not equal true as expected.", state);

        // 4. recommand to unmap first before other operations on the file.
        AssertEqual(mf.Unmap(), MAPPED_FILE_ERR_OK,
            "mf.Unmap() did not equal MAPPED_FILE_ERR_OK as expected.", state);
        // 5. enlarge file size to make it possible to `turnNext()`.
        AssertEqual(ftruncate(mf.GetFd(), MappedFile::PageSize() + MappedFile::PageSize() / 100LL), 0,
            "ftruncate(mf.GetFd(), MappedFile::PageSize() + MappedFile::PageSize() / 100LL) did not equal 0.", state);

        // 6. remap
        AssertEqual(mf.Map(), MAPPED_FILE_ERR_OK, "mf.Map() did not equal MAPPED_FILE_ERR_OK as expected.", state);

        // 7. turn next mapped region with the same size as the file's initial size.
        AssertEqual(mf.TurnNext(), MAPPED_FILE_ERR_OK,
            "mf.TurnNext() did not equal MAPPED_FILE_ERR_OK as expected.", state);
        char* cur = mf.Begin();
        *cur = 'N';

        std::string res;
        LoadStringFromFile(filename, res);
        AssertEqual(strcmp(res.c_str(), content.append("N").c_str()), 0,
            "The two strings, res.c_str() and content.append(\"N\").c_str(), did not have the same content.", state);

        RemoveTestFile(filename);
    }
    BENCHMARK_LOGD("MappedFileTest testTurnNext003 end.");
}

/*
 * @tc.name: testTurnNext004
 * @tc.desc: Test TurnNext() failed.
 */
BENCHMARK_F(BenchmarkMappedFileTest, testTurnNext004)(benchmark::State& state)
{
    BENCHMARK_LOGD("MappedFileTest testTurnNext004 start.");
    while (state.KeepRunning()) {
        // 1. create a new file
        std::string filename = "test_remap.txt";
        std::string content = "Test for remapping use.";
        CreateFile(filename, content, state);

        // 2. map file
        MappedFile mf(filename);
        AssertEqual(mf.Map(), MAPPED_FILE_ERR_OK, "mf.Map() did not equal MAPPED_FILE_ERR_OK as expected.", state);

        // 3. check status after mapping
        AssertTrue((mf.IsMapped()), "mf.IsMapped() did not equal true as expected.", state);
        AssertTrue((mf.IsNormed()), "mf.IsNormed() did not equal true as expected.", state);

        // 4. turn next mapped region with the same size as the file's initial size.
        AssertEqual(mf.TurnNext(), ERR_INVALID_OPERATION,
            "mf.TurnNext() did not equal ERR_INVALID_OPERATION as expected.", state);

        RemoveTestFile(filename);
    }
    BENCHMARK_LOGD("MappedFileTest testTurnNext004 end.");
}

/*
 * @tc.name: testTurnNext005
 * @tc.desc: Test TurnNext() with file size less than one page.
 */
BENCHMARK_F(BenchmarkMappedFileTest, testTurnNext005)(benchmark::State& state)
{
    BENCHMARK_LOGD("MappedFileTest testTurnNext005 start.");
    while (state.KeepRunning()) {
        // 1. create a new file
        std::string filename = "test_remap.txt";
        std::string content = "Test for remapping use.00";
        CreateFile(filename, content, state);

        struct stat stb = {0};
        AssertEqual(stat(filename.c_str(), &stb), 0,
            "stat(filename.c_str(), &stb) did not equal 0 as expected.", state);
        off_t orig = stb.st_size; // 25 bytes

        // 2. extend its size
        int fd = open(filename.c_str(), O_RDWR | O_CLOEXEC);
        AssertUnequal(fd, -1, "fd was not different from -1 as expected.", state);
        const int newSize = 10;
        AssertEqual(ftruncate(fd, MappedFile::PageSize() + newSize), 0,
            "ftruncate(fd, MappedFile::PageSize() + newSize) did not equal 0 as expected.", state);

        // 3. map file
        MappedFile mf(filename, MapMode::DEFAULT, 0, orig);
        AssertEqual(mf.Map(), MAPPED_FILE_ERR_OK, "mf.Map() did not equal MAPPED_FILE_ERR_OK as expected.", state);

        // 4. check status after mapping
        AssertTrue((mf.IsMapped()), "mf.IsMapped() did not equal true as expected.", state);
        AssertTrue((mf.IsNormed()), "mf.IsNormed() did not equal true as expected.", state);

        // 5. turn next mapped region with the same size as the file's initial size.
        AssertEqual(mf.TurnNext(), MAPPED_FILE_ERR_OK,
            "mf.TurnNext() did not equal MAPPED_FILE_ERR_OK as expected.", state);

        off_t endOff;
        // 6. keep turnNext within a page,and reach the bottom of a page
        KeepAPageAndReachBottom(endOff, orig, mf, state);

        // 7. this turn will trigger a remapping
        endOff = mf.EndOffset();
        AssertEqual(mf.TurnNext(), MAPPED_FILE_ERR_OK,
            "mf.TurnNext() did not equal MAPPED_FILE_ERR_OK as expected.", state);
        AssertTrue((mf.IsMapped()), "mf.IsMapped() did not equal true as expected.", state);
        AssertEqual(mf.StartOffset(), endOff + 1, "mf.StartOffset() did not equal endOff + 1 as expected.", state);
        const int expectedSize = 10;
        AssertEqual(mf.Size(), expectedSize, "mf.Size() did not equal expectedSize as expected.", state);
        AssertEqual(mf.RegionStart(), mf.Begin(), "mf.RegionStart() did not equal mf.Begin() as expected.", state);
        AssertEqual(static_cast<off_t>(mf.RegionEnd() - mf.RegionStart()) + 1LL, mf.PageSize(),
            "static_cast<off_t>(mf.RegionEnd() - mf.RegionStart()) + 1LL did not equal mf.PageSize().", state);
        PrintStatus(mf);
    }
    BENCHMARK_LOGD("MappedFileTest testTurnNext005 end.");
}

/*
 * @tc.name: testInvalidMap001
 * @tc.desc: Test file mapping with invalid offset.
 */
BENCHMARK_F(BenchmarkMappedFileTest, testInvalidMap001)(benchmark::State& state)
{
    BENCHMARK_LOGD("MappedFileTest testInvalidMap001 start.");
    while (state.KeepRunning()) {
        // 1. create a new file
        std::string filename = "test_invalid_1.txt";
        std::string content = "Test for invalid use.";
        CreateFile(filename, content, state);

        // 2. map file
        off_t offset = 100; // Specify offset that is not multiple of page-size.
        MappedFile mf(filename, MapMode::DEFAULT, offset);
        AssertUnequal(mf.Map(), MAPPED_FILE_ERR_OK,
            "mf.Map() was not different from MAPPED_FILE_ERR_OK as expected.", state);

        MappedFile mf1(filename, MapMode::DEFAULT, -1);
        AssertUnequal(mf1.Map(), MAPPED_FILE_ERR_OK,
            "mf1.Map() was not different from MAPPED_FILE_ERR_OK as expected.", state);

        // 3. check status
        AssertFalse((mf.IsMapped()), "mf.IsMapped() did not equal false as expected.", state);
        AssertFalse((mf.IsNormed()), "mf.IsNormed() did not equal false as expected.", state);
        AssertFalse((mf1.IsMapped()), "mf1.IsMapped() did not equal false as expected.", state);
        AssertFalse((mf1.IsNormed()), "mf1.IsNormed() did not equal false as expected.", state);

        RemoveTestFile(filename);
    }
    BENCHMARK_LOGD("MappedFileTest testInvalidMap001 end.");
}

/*
 * @tc.name: testInvalidMap002
 * @tc.desc: Test file mapping with invalid offset excessing the substantial size of the file.
 */
BENCHMARK_F(BenchmarkMappedFileTest, testInvalidMap002)(benchmark::State& state)
{
    BENCHMARK_LOGD("MappedFileTest testInvalidMap002 start.");
    while (state.KeepRunning()) {
        // 1. create a new file
        std::string filename = "test_invalid_2.txt";
        std::string content = "Test for invalid use.";
        CreateFile(filename, content, state);

        // 2. map file
        off_t offset = 4 * 1024; // Specify offset excessing the substantial size of the file.
        MappedFile mf(filename, MapMode::DEFAULT, offset);
        AssertUnequal(mf.Map(), MAPPED_FILE_ERR_OK,
            "mf.Map() was not different from MAPPED_FILE_ERR_OK as expected.", state);

        // 3. check status
        AssertFalse((mf.IsMapped()), "mf.IsMapped() did not equal false as expected.", state);
        AssertFalse((mf.IsNormed()), "mf.IsNormed() did not equal false as expected.", state);

        RemoveTestFile(filename);
    }
    BENCHMARK_LOGD("MappedFileTest testInvalidMap002 end.");
}

/*
 * @tc.name: testInvalidMap003
 * @tc.desc: Test mapping non-existed file without setting CREAT_IF_ABSENT.
 */
BENCHMARK_F(BenchmarkMappedFileTest, testInvalidMap003)(benchmark::State& state)
{
    BENCHMARK_LOGD("MappedFileTest testInvalidMap003 start.");
    while (state.KeepRunning()) {
        // 1. create a new file
        std::string filename = "test_invalid_3.txt";
        filename.insert(0, SUITE_PATH).insert(0, BASE_PATH);
        RemoveTestFile(filename);

        // 2. map file
        MappedFile mf(filename);
        AssertUnequal(mf.Map(), MAPPED_FILE_ERR_OK,
            "mf.Map() was not different from MAPPED_FILE_ERR_OK as expected.", state);

        // 3. check status
        AssertFalse((mf.IsMapped()), "mf.IsMapped() did not equal false as expected.", state);
        AssertFalse((mf.IsNormed()), "mf.IsNormed() did not equal false as expected.", state);

        RemoveTestFile(filename);
    }
    BENCHMARK_LOGD("MappedFileTest testInvalidMap003 end.");
}

/*
 * @tc.name: testInvalidMap004
 * @tc.desc: Test mapping with invalid size.
 */
BENCHMARK_F(BenchmarkMappedFileTest, testInvalidMap004)(benchmark::State& state)
{
    BENCHMARK_LOGD("MappedFileTest testInvalidMap004 start.");
    while (state.KeepRunning()) {
        // 1. create a new file
        std::string filename = "test_invalid_4.txt";
        std::string content = "Test for invalid use.";
        CreateFile(filename, content, state);

        // 2. map file
        MappedFile mf(filename, MapMode::DEFAULT, 0, -2); // -2: less than DEFAULT_LENGTH(-1)
        AssertEqual(mf.Map(), ERR_INVALID_VALUE, "mf.Map() did not equal ERR_INVALID_VALUE as expected.", state);

        // 3. map again with another invalid param.
        MappedFile mf1(filename, MapMode::DEFAULT, 0, 0);
        AssertEqual(mf1.Map(), ERR_INVALID_VALUE, "mf1.Map() did not equal ERR_INVALID_VALUE as expected.", state);

        // 3. check status
        AssertFalse((mf.IsMapped()), "mf.IsMapped() did not equal false as expected.", state);
        AssertFalse((mf.IsNormed()), "mf.IsNormed() did not equal false as expected.", state);
        AssertFalse((mf1.IsMapped()), "mf1.IsMapped() did not equal false as expected.", state);
        AssertFalse((mf1.IsNormed()), "mf1.IsNormed() did not equal false as expected.", state);

        RemoveTestFile(filename);
    }
    BENCHMARK_LOGD("MappedFileTest testInvalidMap004 end.");
}

/*
 * @tc.name: testInvalidMap005
 * @tc.desc: Test mapping an already mapped file.
 */
BENCHMARK_F(BenchmarkMappedFileTest, testInvalidMap005)(benchmark::State& state)
{
    BENCHMARK_LOGD("MappedFileTest testInvalidMap005 start.");
    while (state.KeepRunning()) {
        // 1. create a new file
        std::string filename = "test_invalid_6.txt";
        std::string content = "Test for invalid use.";
        CreateFile(filename, content, state);

        // 2. map file
        MappedFile mf(filename);
        AssertEqual(mf.Map(), MAPPED_FILE_ERR_OK, "mf.Map() did not equal MAPPED_FILE_ERR_OK as expected.", state);
        AssertEqual(mf.Map(), ERR_INVALID_OPERATION,
            "mf.Map() did not equal ERR_INVALID_OPERATION as expected.", state);

        // 3. check status
        AssertTrue((mf.IsMapped()), "mf.IsMapped() did not equal true as expected.", state);
        AssertTrue((mf.IsNormed()), "mf.IsNormed() did not equal true as expected.", state);

        RemoveTestFile(filename);
    }
    BENCHMARK_LOGD("MappedFileTest testInvalidMap005 end.");
}

/*
 * @tc.name: testInvalidMap006
 * @tc.desc: Test resize with invalid params.
 */
BENCHMARK_F(BenchmarkMappedFileTest, testInvalidMap006)(benchmark::State& state)
{
    BENCHMARK_LOGD("MappedFileTest testInvalidMap006 start.");
    while (state.KeepRunning()) {
        // 1. create a new file
        std::string filename = "test_invalid_7.txt";
        std::string content = "Test for invalid use.";
        CreateFile(filename, content, state);


        // 2. map file
        MappedFile mf(filename);
        AssertEqual(mf.Map(), MAPPED_FILE_ERR_OK, "mf.Map() did not equal MAPPED_FILE_ERR_OK as expected.", state);

        // 3. check status
        AssertTrue((mf.IsMapped()), "mf.IsMapped() did not equal true as expected.", state);
        AssertTrue((mf.IsNormed()), "mf.IsNormed() did not equal true as expected.", state);

        // 4. resize
        const int newSize1 = 0;
        AssertEqual(mf.Resize(newSize1), ERR_INVALID_OPERATION,
            "mf.Resize(newSize1) did not equal ERR_INVALID_OPERATION as expected.", state);
        const int newSize2 = -2;
        AssertEqual(mf.Resize(newSize2), ERR_INVALID_OPERATION,
            "mf.Resize(newSize2) did not equal ERR_INVALID_OPERATION as expected.", state);
        AssertEqual(mf.Resize(mf.Size()), ERR_INVALID_OPERATION,
            "mf.Resize(mf.Size()) did not equal ERR_INVALID_OPERATION as expected.", state);

        // 5. Unmap first then resize.
        AssertEqual(mf.Unmap(), MAPPED_FILE_ERR_OK,
            "mf.Unmap() did not equal MAPPED_FILE_ERR_OK as expected.", state);
        const int newSize3 = 8;
        AssertEqual(mf.Resize(mf.Size() + newSize3), ERR_INVALID_OPERATION,
            "mf.Resize(mf.Size() + newSize3) did not equal ERR_INVALID_OPERATION as expected.", state);

        RemoveTestFile(filename);
    }
    BENCHMARK_LOGD("MappedFileTest testInvalidMap006 end.");
}

/*
 * @tc.name: testInvalidMap007
 * @tc.desc: Test resize with no param changed.
 */
BENCHMARK_F(BenchmarkMappedFileTest, testInvalidMap007)(benchmark::State& state)
{
    BENCHMARK_LOGD("MappedFileTest testInvalidMap007 start.");
    while (state.KeepRunning()) {
        // 1. create a new file
        std::string filename = "test_invalid_8.txt";
        std::string content = "Test for invalid use.";
        CreateFile(filename, content, state);


        // 2. map file
        MappedFile mf(filename);
        AssertEqual(mf.Map(), MAPPED_FILE_ERR_OK, "mf.Map() did not equal MAPPED_FILE_ERR_OK as expected.", state);

        // 3. check status
        AssertTrue((mf.IsMapped()), "mf.IsMapped() did not equal true as expected.", state);
        AssertTrue((mf.IsNormed()), "mf.IsNormed() did not equal true as expected.", state);

        // 4. resize
        AssertEqual(mf.Resize(), ERR_INVALID_OPERATION,
            "mf.Resize() did not equal ERR_INVALID_OPERATION as expected.", state);

        RemoveTestFile(filename);
    }
    BENCHMARK_LOGD("MappedFileTest testInvalidMap007 end.");
}

/*
 * @tc.name: testInvalidMap008
 * @tc.desc: Test TurnNext() with params changed.
 */
BENCHMARK_F(BenchmarkMappedFileTest, testInvalidMap008)(benchmark::State& state)
{
    BENCHMARK_LOGD("MappedFileTest testInvalidMap008 start.");
    while (state.KeepRunning()) {
        // 1. create a new file
        std::string filename = "test_invalid_9.txt";
        std::string content = "Test for invalid use.";
        CreateFile(filename, content, state);


        // 2. map file
        MappedFile mf(filename);
        AssertEqual(mf.Map(), MAPPED_FILE_ERR_OK, "mf.Map() did not equal MAPPED_FILE_ERR_OK as expected.", state);

        // 3. check status
        AssertTrue((mf.IsMapped()), "mf.IsMapped() did not equal true as expected.", state);
        AssertTrue((mf.IsNormed()), "mf.IsNormed() did not equal true as expected.", state);

        // 5. Change params
        AssertTrue((mf.ChangeSize(mf.Size() + 1)),
            "mf.ChangeSize(mf.Size() + 1) did not equal true as expected.", state);

        // 6. check status
        AssertFalse((mf.IsMapped()), "mf.IsMapped() did not equal false as expected.", state);
        AssertFalse((mf.IsNormed()), "mf.IsNormed() did not equal false as expected.", state);

        // 4. turn next.
        AssertEqual(mf.TurnNext(), ERR_INVALID_OPERATION,
            "mf.TurnNext() did not equal ERR_INVALID_OPERATION as expected.", state);

        RemoveTestFile(filename);
    }
    BENCHMARK_LOGD("MappedFileTest testInvalidMap008 end.");
}

/*
 * @tc.name: testInvalidMap009
 * @tc.desc: Test ChangeXX() with invalid params.
 */
BENCHMARK_F(BenchmarkMappedFileTest, testInvalidMap009)(benchmark::State& state)
{
    BENCHMARK_LOGD("MappedFileTest testInvalidMap009 start.");
    while (state.KeepRunning()) {
        // 1. create a new file
        std::string filename = "test_invalid_10.txt";
        std::string content = "Test for invalid use.";
        CreateFile(filename, content, state);


        // 2. create MappedFile
        MappedFile mf(filename);

        // 3. map file
        AssertEqual(mf.Map(), MAPPED_FILE_ERR_OK, "mf.Map() did not equal MAPPED_FILE_ERR_OK as expected.", state);

        // 4. check status
        AssertTrue((mf.IsMapped()), "mf.IsMapped() did not equal true as expected.", state);
        AssertTrue((mf.IsNormed()), "mf.IsNormed() did not equal true as expected.", state);

        // 5. Change params
        AssertFalse((mf.ChangeOffset(mf.StartOffset())),
            "mf.ChangeOffset(mf.StartOffset()) did not equal false as expected.", state);
        AssertFalse((mf.ChangeSize(mf.Size())), "mf.ChangeSize(mf.Size()) did not equal false as expected.", state);
        AssertFalse((mf.ChangeHint(mf.GetHint())),
            "mf.ChangeHint(mf.GetHint()) did not equal false as expected.", state);
        AssertFalse((mf.ChangeMode(mf.GetMode())),
            "mf.ChangeMode(mf.GetMode()) did not equal false as expected.", state);
        AssertFalse((mf.ChangePath(mf.GetPath())),
            "mf.ChangePath(mf.GetPath()) did not equal false as expected.", state);

        RemoveTestFile(filename);
    }
    BENCHMARK_LOGD("MappedFileTest testInvalidMap009 end.");
}

/*
 * @tc.name: testAutoAdjustedMode001
 * @tc.desc: Test mapping file with invalid mapping mode, but can be auto adjusted.
 */
BENCHMARK_F(BenchmarkMappedFileTest, testAutoAdjustedMode001)(benchmark::State& state)
{
    BENCHMARK_LOGD("MappedFileTest testAutoAdjustedMode001 start.");
    while (state.KeepRunning()) {
        // 1. create a new file
        std::string filename = "test_adjmod_1.txt";
        std::string content = "Test for auto adj use.";
        CreateFile(filename, content, state);


        // 2. map file
        const int mapMode1 = 1;
        const int mapMode2 = 16;
        MapMode mode = static_cast<MapMode>(mapMode1) | static_cast<MapMode>(mapMode2) |
                    MapMode::PRIVATE | MapMode::READ_ONLY; // bits out of the scope will be ignored.
        MappedFile mf(filename, mode);
        AssertEqual(mf.Map(), MAPPED_FILE_ERR_OK, "mf.Map() did not equal MAPPED_FILE_ERR_OK as expected.", state);

        // 3. check status
        AssertTrue((mf.IsMapped()), "mf.IsMapped() did not equal true as expected.", state);
        AssertTrue((mf.IsNormed()), "mf.IsNormed() did not equal true as expected.", state);

        // 4. check map-mode
        AssertEqual((MapMode::PRIVATE | MapMode::READ_ONLY), mf.GetMode(),
            "MapMode::PRIVATE | MapMode::READ_ONLY did not equal mf.GetMode() as expected.", state);

        RemoveTestFile(filename);
    }
    BENCHMARK_LOGD("MappedFileTest testAutoAdjustedMode001 end.");
}

/*
 * @tc.name: testAutoAdjustedSize001
 * @tc.desc: Test file mapping with size excessing the last page of the file.
 */
BENCHMARK_F(BenchmarkMappedFileTest, testAutoAdjustedSize001)(benchmark::State& state)
{
    BENCHMARK_LOGD("MappedFileTest testAutoAdjustedSize001 start.");
    while (state.KeepRunning()) {
        // 1. create a new file
        std::string filename = "test_adjsize_1.txt";
        std::string content = "Test for auto adj use.";
        CreateFile(filename, content, state);


        // 2. map file
        off_t size = 5 * 1024; // Specified size excessing the last page of the file.
        MappedFile mf(filename, MapMode::DEFAULT, 0, size);
        AssertEqual(mf.Map(), MAPPED_FILE_ERR_OK, "mf.Map() did not equal MAPPED_FILE_ERR_OK as expected.", state);

        // 3. check status
        AssertTrue((mf.IsMapped()), "mf.IsMapped() did not equal true as expected.", state);
        AssertTrue((mf.IsNormed()), "mf.IsNormed() did not equal true as expected.", state);

        // 4. check size
        struct stat stb = {0};
        stat(filename.c_str(), &stb);
        off_t max = (stb.st_size / mf.PageSize() + 1LL) * mf.PageSize() - 0LL;
        AssertEqual(mf.Size(), max, "mf.Size() did not equal max as expected.", state);

        RemoveTestFile(filename);
    }
    BENCHMARK_LOGD("MappedFileTest testAutoAdjustedSize001 end.");
}

/*
 * @tc.name: testAutoAdjustedSize002
 * @tc.desc: Test file mapping with size excessing the last page of the file.
 */
BENCHMARK_F(BenchmarkMappedFileTest, testAutoAdjustedSize002)(benchmark::State& state)
{
    BENCHMARK_LOGD("MappedFileTest testAutoAdjustedSize002 start.");
    while (state.KeepRunning()) {
        // 1. create a new file
        std::string filename = "test_adjsize_2.txt";
        std::string content = "Test for auto adj use.";
        CreateFile(filename, content, state);


        // 2. Extend size manually
        int fd = open(filename.c_str(), O_RDWR | O_CLOEXEC);
        if (fd != -1) {
            const int newSize = 7 * 1024;
            ftruncate(fd, newSize);

            // 3. map file
            off_t offset = 4 * 1024;
            off_t size = 5 * 1024; // Specified size excessing the last page of the file.
            MappedFile mf(filename, MapMode::DEFAULT, offset, size);
            AssertEqual(mf.Map(), MAPPED_FILE_ERR_OK, "mf.Map() did not equal MAPPED_FILE_ERR_OK as expected.", state);

            // 4. check status
            AssertTrue((mf.IsMapped()), "mf.IsMapped() did not equal true as expected.", state);
            AssertTrue((mf.IsNormed()), "mf.IsNormed() did not equal true as expected.", state);

            // 5. check size
            struct stat stb = {0};
            stat(filename.c_str(), &stb);
            off_t max = (stb.st_size / mf.PageSize() + 1LL) * mf.PageSize() - offset;
            AssertEqual(mf.Size(), max, "mf.Size() did not equal max as expected.", state);

            close(fd);
        }

        RemoveTestFile(filename);
    }
    BENCHMARK_LOGD("MappedFileTest testAutoAdjustedSize002 end.");
}

/*
 * @tc.name: testMoveMappedFile001
 * @tc.desc: Test move constructor.
 */
BENCHMARK_F(BenchmarkMappedFileTest, testMoveMappedFile001)(benchmark::State& state)
{
    BENCHMARK_LOGD("MappedFileTest testMoveMappedFile001 start.");
    while (state.KeepRunning()) {
        // 1. create a new file
        std::string filename = "test_move_1.txt";
        std::string content = "Test for move use.";
        CreateFile(filename, content, state);

        // 2. map file
        MappedFile mf(filename);
        AssertEqual(mf.Map(), MAPPED_FILE_ERR_OK, "mf.Map() did not equal MAPPED_FILE_ERR_OK as expected.", state);

        off_t size = mf.Size();
        off_t offset = mf.StartOffset();
        char* data = mf.Begin();
        MapMode mode = mf.GetMode();
        const char* hint = mf.GetHint();

        // 3. move to a new object
        MappedFile mfNew(std::move(mf));

        // 4. check status and params after move
        AssertFalse((mf.IsNormed()), "mf.IsNormed() did not equal false as expected.", state);
        AssertFalse((mf.IsMapped()), "mf.IsMapped() did not equal false as expected.", state);

        AssertEqual(mf.Begin(), nullptr, "mf.Begin() did not equal nullptr as expected.", state);
        AssertEqual(mf.Size(), MappedFile::DEFAULT_LENGTH,
            "mf.Size() did not equal MappedFile::DEFAULT_LENGTH as expected.", state);
        AssertEqual(mf.StartOffset(), 0, "mf.StartOffset() did not equal 0 as expected.", state);
        AssertEqual(mf.GetMode(), MapMode::DEFAULT,
            "mf.GetMode() did not equal MapMode::DEFAULT as expected.", state);
        AssertEqual(mf.GetHint(), nullptr, "mf.GetHint() did not equal nullptr as expected.", state);
        AssertEqual(mf.GetPath(), "", "mf.GetPath() did not equal "" as expected.", state);

        AssertTrue((mfNew.IsNormed()), "mfNew.IsNormed() did not equal true as expected.", state);
        AssertTrue((mfNew.IsMapped()), "mfNew.IsMapped() did not equal true as expected.", state);
        AssertEqual(mfNew.Begin(), data, "mfNew.Begin() did not equal data as expected.", state);
        AssertEqual(mfNew.Size(), size, "mfNew.Size() did not equal size as expected.", state);
        AssertEqual(mfNew.StartOffset(), offset, "mfNew.StartOffset() did not equal offset as expected.", state);
        AssertEqual(mfNew.GetMode(), mode, "mfNew.GetMode() did not equal mode as expected.", state);
        AssertEqual(mfNew.GetHint(), hint, "mfNew.GetHint() did not equal hint as expected.", state);
        AssertEqual(mfNew.GetPath(), filename, "mfNew.GetPath() did not equal filename as expected.", state);

        // 5. read from mapped file
        ReadFromMappedFile(content, mfNew, state);

        // 6. write to mapped file
        std::string toWrite("Complete.");
        WriteToMappedFile(toWrite, mfNew);
        std::string res;
        LoadStringFromFile(filename, res);
        AssertEqual(res, "Complete.move use.", "res did not equal \"Complete.move use.\" as expected.", state);

        RemoveTestFile(filename);
    }
    BENCHMARK_LOGD("MappedFileTest testMoveMappedFile001 end.");
}

/*
 * @tc.name: testMoveMappedFile002
 * @tc.desc: Test move constructor with ummapped region.
 */
BENCHMARK_F(BenchmarkMappedFileTest, testMoveMappedFile002)(benchmark::State& state)
{
    BENCHMARK_LOGD("MappedFileTest testMoveMappedFile002 start.");
    while (state.KeepRunning()) {
        // 1. create a new file
        std::string filename = "test_move_2.txt";
        std::string content = "Test for move use.";
        CreateFile(filename, content, state);

        // 2. map file
        MappedFile mf(filename);
        AssertEqual(mf.Map(), MAPPED_FILE_ERR_OK, "mf.Map() did not equal MAPPED_FILE_ERR_OK as expected.", state);

        off_t size = mf.Size();
        off_t offset = mf.StartOffset();
        MapMode mode = mf.GetMode();
        const char* hint = mf.GetHint();

        AssertEqual(mf.Unmap(), MAPPED_FILE_ERR_OK,
            "mf.Unmap() did not equal MAPPED_FILE_ERR_OK as expected.", state);
        // 3. move to a new object
        MappedFile mfNew(std::move(mf));

        // 4. check status and params after move
        AssertFalse((mf.IsMapped()), "mf.IsMapped() did not equal false as expected.", state);
        AssertFalse((mf.IsNormed()), "mf.IsNormed() did not equal false as expected.", state);

        AssertEqual(mf.Begin(), nullptr, "mf.Begin() did not equal nullptr as expected.", state);
        AssertEqual(mf.Size(), MappedFile::DEFAULT_LENGTH,
            "mf.Size() did not equal MappedFile::DEFAULT_LENGTH as expected.", state);
        AssertEqual(mf.StartOffset(), 0, "mf.StartOffset() did not equal 0 as expected.", state);
        AssertEqual(mf.GetMode(), MapMode::DEFAULT,
            "mf.GetMode() did not equal MapMode::DEFAULT as expected.", state);
        AssertEqual(mf.GetHint(), nullptr, "mf.GetHint() did not equal nullptr as expected.", state);
        AssertEqual(mf.GetPath(), "", "mf.GetPath() did not equal "" as expected.", state);

        AssertFalse((mfNew.IsMapped()), "mfNew.IsMapped() did not equal false as expected.", state);
        AssertTrue((mfNew.IsNormed()), "mfNew.IsNormed() did not equal true as expected.", state);
        AssertEqual(mfNew.Begin(), nullptr, "mfNew.Begin() did not equal nullptr as expected.", state);
        AssertEqual(mfNew.Size(), size, "mfNew.Size() did not equal size as expected.", state);
        AssertEqual(mfNew.StartOffset(), offset, "mfNew.StartOffset() did not equal offset as expected.", state);
        AssertEqual(mfNew.GetMode(), mode, "mfNew.GetMode() did not equal mode as expected.", state);
        AssertEqual(mfNew.GetHint(), hint, "mfNew.GetHint() did not equal hint as expected.", state);
        AssertEqual(mfNew.GetPath(), filename, "mfNew.GetPath() did not equal filename as expected.", state);

        // 5. Map again
        AssertEqual(mfNew.Map(), MAPPED_FILE_ERR_OK,
            "mfNew.Map() did not equal MAPPED_FILE_ERR_OK as expected.", state);
        // 6. read from mapped file
        ReadFromMappedFile(content, mfNew, state);

        // 7. write to mapped file
        std::string toWrite("Complete.");
        WriteToMappedFile(toWrite, mfNew);
        std::string res;
        LoadStringFromFile(filename, res);
        AssertEqual(res, "Complete.move use.", "res did not equal \"Complete.move use.\" as expected.", state);

        RemoveTestFile(filename);
    }
    BENCHMARK_LOGD("MappedFileTest testMoveMappedFile002 end.");
}

/*
 * @tc.name: testMoveMappedFile003
 * @tc.desc: Test move assignment operator overload.
 */
BENCHMARK_F(BenchmarkMappedFileTest, testMoveMappedFile003)(benchmark::State& state)
{
    BENCHMARK_LOGD("MappedFileTest testMoveMappedFile003 start.");
    while (state.KeepRunning()) {
        // 1. create a new file
        std::string filename = "test_move_3.txt";
        std::string content = "Test for move use.";
        CreateFile(filename, content, state);

        std::string filename1 = "test_move_4.txt";
        std::string content1 = "Test for move use.";
        CreateFile(filename1, content1, state);

        // 2. map file
        MappedFile mf(filename);
        AssertEqual(mf.Map(), MAPPED_FILE_ERR_OK, "mf.Map() did not equal MAPPED_FILE_ERR_OK as expected.", state);
        MappedFile mf1(filename1);
        AssertEqual(mf1.Map(), MAPPED_FILE_ERR_OK, "mf1.Map() did not equal MAPPED_FILE_ERR_OK as expected.", state);

        off_t size = mf1.Size();
        off_t offset = mf1.StartOffset();
        MapMode mode = mf1.GetMode();
        char* data = mf1.Begin();
        const char* hint = mf1.GetHint();

        // 3. move assignment
        mf = std::move(mf1);

        // 4. check status and params after move
        AssertTrue((mf.IsMapped()), "mf.IsMapped() did not equal true as expected.", state);
        AssertTrue((mf.IsNormed()), "mf.IsNormed() did not equal true as expected.", state);
        AssertEqual(mf.Begin(), data, "mf.Begin() did not equal data as expected.", state);
        AssertEqual(mf.Size(), size, "mf.Size() did not equal size as expected.", state);
        AssertEqual(mf.StartOffset(), offset, "mf.StartOffset() did not equal offset as expected.", state);
        AssertEqual(mf.GetMode(), mode, "mf.GetMode() did not equal mode as expected.", state);
        AssertEqual(mf.GetHint(), hint, "mf.GetHint() did not equal hint as expected.", state);
        AssertEqual(mf.GetPath(), filename1, "mf.GetPath() did not equal filename1 as expected.", state);

        // 5. read from mapped file
        ReadFromMappedFile(content1, mf, state);

        // 6. write to mapped file
        std::string toWrite("Complete.");
        WriteToMappedFile(toWrite, mf);
        std::string res;
        LoadStringFromFile(filename1, res);
        AssertEqual(res, "Complete.move use.", "res did not equal \"Complete.move use.\" as expected.", state);

        RemoveTestFile(filename);
        RemoveTestFile(filename1);
    }
    BENCHMARK_LOGD("MappedFileTest testMoveMappedFile003 end.");
}

/*
 * @tc.name: testMoveMappedFile004
 * @tc.desc: Test move assignment operator overload with ummapped region.
 */
BENCHMARK_F(BenchmarkMappedFileTest, testMoveMappedFile004)(benchmark::State& state)
{
    BENCHMARK_LOGD("MappedFileTest testMoveMappedFile004 start.");
    while (state.KeepRunning()) {
        // 1. create a new file
        std::string filename = "test_move_4.txt";
        std::string content = "Test for move use.";
        CreateFile(filename, content, state);

        std::string filename1 = "test_move_5.txt";
        std::string content1 = "Test for move use.";
        CreateFile(filename1, content1, state);

        // 2. map file
        MappedFile mf(filename);
        AssertEqual(mf.Map(), MAPPED_FILE_ERR_OK, "mf.Map() did not equal MAPPED_FILE_ERR_OK as expected.", state);
        MappedFile mf1(filename1);
        AssertEqual(mf1.Map(), MAPPED_FILE_ERR_OK, "mf1.Map() did not equal MAPPED_FILE_ERR_OK as expected.", state);

        off_t size = mf1.Size();
        off_t offset = mf1.StartOffset();
        MapMode mode = mf1.GetMode();
        const char* hint = mf1.GetHint();

        // 3. ummap mf1
        AssertEqual(mf1.Unmap(), MAPPED_FILE_ERR_OK,
            "mf1.Unmap() did not equal MAPPED_FILE_ERR_OK as expected.", state);
        // 4. move assignment
        mf = std::move(mf1);
        // 5. check status and params after move
        AssertFalse((mf.IsMapped()), "mf.IsMapped() did not equal false as expected.", state);
        AssertTrue((mf.IsNormed()), "mf.IsNormed() did not equal true as expected.", state);
        AssertEqual(mf.Begin(), nullptr, "mf.Begin() did not equal nullptr as expected.", state);
        AssertEqual(mf.Size(), size, "mf.Size() did not equal size as expected.", state);
        AssertEqual(mf.StartOffset(), offset, "mf.StartOffset() did not equal offset as expected.", state);
        AssertEqual(mf.GetMode(), mode, "mf.GetMode() did not equal mode as expected.", state);
        AssertEqual(mf.GetHint(), hint, "mf.GetHint() did not equal hint as expected.", state);
        AssertEqual(mf.GetPath(), filename1, "mf.GetPath() did not equal filename1 as expected.", state);

        AssertEqual(mf.Map(), MAPPED_FILE_ERR_OK, "mf.Map() did not equal MAPPED_FILE_ERR_OK as expected.", state);
        // 6. read from mapped file
        ReadFromMappedFile(content1, mf, state);

        // 7. write to mapped file
        std::string toWrite("Complete.");
        WriteToMappedFile(toWrite, mf);
        std::string res;
        LoadStringFromFile(filename1, res);
        AssertEqual(res, "Complete.move use.", "res did not equal \"Complete.move use.\" as expected.", state);

        RemoveTestFile(filename);
        RemoveTestFile(filename1);
    }
    BENCHMARK_LOGD("MappedFileTest testMoveMappedFile004 end.");
}

/*
 * @tc.name: testNormalize001
 * @tc.desc: Test Normalize via changing offset and size.
 */
BENCHMARK_F(BenchmarkMappedFileTest, testNormalize001)(benchmark::State& state)
{
    BENCHMARK_LOGD("MappedFileTest testNormalize001 start.");
    while (state.KeepRunning()) {
        // create a new file
        std::string filename = "test_normalize.txt";
        std::string content = "Test for normalize use.";
        filename.insert(0, SUITE_PATH).insert(0, BASE_PATH);
        RemoveTestFile(filename);

        const int mapMode1 = -1;
        const int mapMode2 = -2;
        MappedFile mf(filename, MapMode::DEFAULT, mapMode1, mapMode2);
        AssertEqual(mf.Normalize(), ERR_INVALID_VALUE,
            "mf.Normalize did not equal ERR_INVALID_VALUE as expected.", state);

        RemoveTestFile(filename);

        filename = "test_normalize1.txt";
        content = "Test for normalize1 use.";
        CreateFile(filename, content, state);


        off_t size = 20;
        MappedFile mf1(filename, MapMode::DEFAULT, 0, size);
        ErrCode res = mf1.Normalize();
        AssertEqual(res, MAPPED_FILE_ERR_OK, "res did not equal MAPPED_FILE_ERR_OK as expected.", state);

        RemoveTestFile(filename);
    }
    BENCHMARK_LOGD("MappedFileTest testNormalize001 end.");
}

/*
 * @tc.name: testClear001
 * @tc.desc: Test Clear.
 */
BENCHMARK_F(BenchmarkMappedFileTest, testClear001)(benchmark::State& state)
{
    BENCHMARK_LOGD("MappedFileTest testClear001 start.");
    while (state.KeepRunning()) {
        // 1. create a new file
        std::string filename = "test_clear.txt";
        std::string content = "Test for clear use.";
        CreateFile(filename, content, state);


        MappedFile mf1(filename);
        // 2. map file
        AssertEqual(mf1.Map(), MAPPED_FILE_ERR_OK, "mf1.Map() did not equal MAPPED_FILE_ERR_OK as expected.", state);

        // 3. clear mapping
        AssertEqual(mf1.Clear(true), MAPPED_FILE_ERR_OK,
            "mf1.Clear(true) did not equal MAPPED_FILE_ERR_OK as expected.", state);
        RemoveTestFile(filename);
    }
    BENCHMARK_LOGD("MappedFileTest testClear001 end.");
}

/*
 * @tc.name: testMoveConstructor001
 * @tc.desc: Test Move Constructor.
 */
BENCHMARK_F(BenchmarkMappedFileTest, testMoveConstructor001)(benchmark::State& state)
{
    BENCHMARK_LOGD("MappedFileTest testMoveConstructor001 start.");
    while (state.KeepRunning()) {
        std::string filename = "test_move_assignment.txt";
        std::string content = "Test for move assignment use.";
        filename.insert(0, SUITE_PATH).insert(0, BASE_PATH);

        AssertTrue((CreateTestFile(filename, content)),
            "CreateTestFile(filename, content) did not equal true as expected.", state);

        MappedFile mf(filename);

        AssertEqual(mf.Map(), MAPPED_FILE_ERR_OK, "mf.Map() did not equal MAPPED_FILE_ERR_OK as expected.", state);
        AssertTrue((mf.IsNormed()), "mf.IsNormed() did not equal true as expected.", state);

        MappedFile mf1(std::move(mf));
        AssertTrue((mf1.IsNormed()), "mf.IsNormed() did not equal true as expected.", state);
        AssertFalse(mf.IsNormed(), "mf.IsNormed() did not equal false as expected.", state);

        RemoveTestFile(filename);
    }
    BENCHMARK_LOGD("MappedFileTest testMoveConstructor001 end.");
}
}  // namespace
}  // namespace OHOS
// Run the benchmark
BENCHMARK_MAIN();