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
#include <iostream>
#include <fstream>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include "file_ex.h"
#include "benchmark_log.h"
#include "benchmark_assert.h"
using namespace std;

namespace OHOS {
namespace {

static constexpr char CONTENT_STR[] = "TTtt@#$%^&*()_+~`";
static constexpr char FILE_PATH[] = "./tmp1.txt";
static constexpr char NULL_STR[] = "";
static constexpr int MAX_FILE_LENGTH = 1 * 1024 * 1024;
static constexpr int EXCEEDS_MAXIMUM_LENGTH = 32 * 1024 * 1024 + 1;

class BenchmarkFileTest : public benchmark::Fixture {
public:
    void SetUp(const ::benchmark::State& state) override
    {
    }

    void TearDown(const ::benchmark::State& state) override
    {
    }

    BenchmarkFileTest()
    {
        Iterations(iterations);
        Repetitions(repetitions);
        ReportAggregatesOnly();
    }

    ~BenchmarkFileTest() override = default;

protected:
    const int32_t repetitions = 3;
    const int32_t iterations = 1000;
};

bool CreateTestFile(const std::string& path, const std::string& content)
{
    ofstream out(path, ios_base::out | ios_base::trunc);
    if (out.is_open()) {
        out << content;
        return true;
    }

    BENCHMARK_LOGD("open file failed! %{public}s", path.c_str());
    return false;
}

int RemoveTestFile(const std::string& path)
{
    return unlink(path.c_str());
}

void LoadString(string& filename, string& content, benchmark::State& state)
{
    while (state.KeepRunning()) {
        string result;
        CreateTestFile(filename, content);
        int fd = open(filename.c_str(), O_RDONLY);
        AssertTrue((LoadStringFromFd(fd, result)),
            "LoadStringFromFd(fd, result) did not equal true as expected.", state);
        close(fd);
        RemoveTestFile(filename);
        AssertEqual(result, content, "result == content did not equal true as expected.", state);
    }
}

void SaveString(string& filename, string& content, benchmark::State& state)
{
    while (state.KeepRunning()) {
        int fd = open(filename.c_str(), O_RDWR | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
        bool ret = SaveStringToFd(fd, content);
        close(fd);
        AssertEqual(ret, true, "ret did not equal true as expected.", state);

        string loadResult;
        fd = open(filename.c_str(), O_RDONLY);
        ret = LoadStringFromFd(fd, loadResult);
        close(fd);
        RemoveTestFile(filename);
        AssertEqual(ret, true, "ret did not equal true as expected.", state);
        AssertEqual(loadResult, content, "loadResult did not equal content as expected.", state);
    }
}

/*
 * @tc.name: testLoadStringFromFile001
 * @tc.desc: Test loading an existed file 'meminfo'
 */
BENCHMARK_F(BenchmarkFileTest, testLoadStringFromFile001)(benchmark::State& state)
{
    BENCHMARK_LOGD("FileTest testLoadStringFromFile001 start.");
    string filename = "/proc/meminfo";
    while (state.KeepRunning()) {
        string str;
        AssertTrue((LoadStringFromFile(filename, str)),
            "LoadStringFromFile(filename, str) did not equal true as expected.", state);

        string str2;
        int fd = open(filename.c_str(), O_RDONLY);
        AssertTrue((LoadStringFromFd(fd, str2)), "LoadStringFromFd(fd, str2) did not equal true as expected.", state);
        close(fd);
        AssertEqual(str.size(), str2.size(), "str.size() did not equal str2.size() as expected.", state);

        vector<char> buff;
        bool ret = LoadBufferFromFile(filename, buff);
        AssertTrue((ret), "ret did not equal true as expected.", state);
        AssertEqual(str2.size(), buff.size(), "str2.size() did not equal buff.size() as expected.", state);
    }
    BENCHMARK_LOGD("FileTest testLoadStringFromFile001 end.");
}

/*
 * @tc.name: testLoadStringFromFile002
 * @tc.desc: Test loading a non-existed file
 */
BENCHMARK_F(BenchmarkFileTest, testLoadStringFromFile002)(benchmark::State& state)
{
    BENCHMARK_LOGD("FileTest testLoadStringFromFile002 start.");
    string filename = NULL_STR;
    while (state.KeepRunning()) {
        string str;
        AssertFalse((LoadStringFromFile(filename, str)),
            "LoadStringFromFile(filename, str) did not equal false as expected.", state);
        AssertTrue((str.empty()), "str.empty() did not equal true as expected.", state);
    }
    BENCHMARK_LOGD("FileTest testLoadStringFromFile002 end.");
}

/*
 * @tc.name: testLoadStringFromFile003
 * @tc.desc: Test loading a newly created file with null contents
 */
BENCHMARK_F(BenchmarkFileTest, testLoadStringFromFile003)(benchmark::State& state)
{
    BENCHMARK_LOGD("FileTest testLoadStringFromFile003 start.");
    string filename = FILE_PATH;
    string content = NULL_STR;
    while (state.KeepRunning()) {
        string str;
        CreateTestFile(filename, content);
        AssertTrue((LoadStringFromFile(filename, str)),
            "LoadStringFromFile(filename, str) did not equal true as expected.", state);
        RemoveTestFile(filename);
        AssertEqual(str, content, "str == content did not equal true as expected.", state);
    }
    BENCHMARK_LOGD("FileTest testLoadStringFromFile003 end.");
}

/*
 * @tc.name: testLoadStringFromFile004
 * @tc.desc: Test loading a newly created file with contents
 */
BENCHMARK_F(BenchmarkFileTest, testLoadStringFromFile004)(benchmark::State& state)
{
    BENCHMARK_LOGD("FileTest testLoadStringFromFile004 start.");
    string filename = FILE_PATH;
    string content = CONTENT_STR;
    while (state.KeepRunning()) {
        string str;
        CreateTestFile(filename, content);
        AssertTrue((LoadStringFromFile(filename, str)),
            "LoadStringFromFile(filename, str) did not equal true as expected.", state);
        RemoveTestFile(filename);
        AssertEqual(str, content, "str == content did not equal true as expected.", state);
    }
    BENCHMARK_LOGD("FileTest testLoadStringFromFile004 end.");
}

/*
 * @tc.name: testLoadStringFromFile005
 * @tc.desc: Test loading a newly created file, whose contents are of maximum length
 */
BENCHMARK_F(BenchmarkFileTest, testLoadStringFromFile005)(benchmark::State& state)
{
    BENCHMARK_LOGD("FileTest testLoadStringFromFile005 start.");
    string content(MAX_FILE_LENGTH, 't');
    string filename = FILE_PATH;
    while (state.KeepRunning()) {
        string str;
        CreateTestFile(filename, content);
        AssertTrue((LoadStringFromFile(filename, str)),
            "LoadStringFromFile(filename, str) did not equal true as expected.", state);
        RemoveTestFile(filename);
        AssertEqual(str, content, "str == content did not equal true as expected.", state);
    }
    BENCHMARK_LOGD("FileTest testLoadStringFromFile005 end.");
}

/*
 * @tc.name: testLoadStringFromFile006
 * @tc.desc: Test loading a newly created file, whose contents exceeds maximum length
 */
BENCHMARK_F(BenchmarkFileTest, testLoadStringFromFile006)(benchmark::State& state)
{
    BENCHMARK_LOGD("FileTest testLoadStringFromFile006 start.");
    string content(EXCEEDS_MAXIMUM_LENGTH, 't');
    string filename = FILE_PATH;
    while (state.KeepRunning()) {
        string str;
        CreateTestFile(filename, content);
        AssertFalse((LoadStringFromFile(filename, str)),
            "LoadStringFromFile(filename, str) did not equal false as expected.", state);
        RemoveTestFile(filename);
        AssertTrue((str.empty()), "str.empty() did not equal true as expected.", state);
    }
    BENCHMARK_LOGD("FileTest testLoadStringFromFile006 end.");
}

/*
 * @tc.name: testLoadStringFromFd001
 * @tc.desc: Test loading a file by a invalid fd -1
 */
BENCHMARK_F(BenchmarkFileTest, testLoadStringFromFd001)(benchmark::State& state)
{
    BENCHMARK_LOGD("FileTest testLoadStringFromFd001 start.");
    while (state.KeepRunning()) {
        string result;
        AssertFalse((LoadStringFromFd(-1, result)),
            "LoadStringFromFd(-1, result) did not equal false as expected.", state);
        AssertEqual(result, "", "result did not equal "" as expected.", state);
    }
    BENCHMARK_LOGD("FileTest testLoadStringFromFd001 end.");
}

/*
 * @tc.name: testLoadStringFromFd002
 * @tc.desc: Test loading a newly created file without contents by its fd
 */
BENCHMARK_F(BenchmarkFileTest, testLoadStringFromFd002)(benchmark::State& state)
{
    BENCHMARK_LOGD("FileTest testLoadStringFromFd002 start.");
    string filename = FILE_PATH;
    string content = NULL_STR;
    LoadString(filename, content, state);
    BENCHMARK_LOGD("FileTest testLoadStringFromFd002 end.");
}

/*
 * @tc.name: testLoadStringFromFd003
 * @tc.desc: Test loading a newly created file with contents by its fd
 */
BENCHMARK_F(BenchmarkFileTest, testLoadStringFromFd003)(benchmark::State& state)
{
    BENCHMARK_LOGD("FileTest testLoadStringFromFd003 start.");
    string filename = FILE_PATH;
    string content = CONTENT_STR;
    LoadString(filename, content, state);
    BENCHMARK_LOGD("FileTest testLoadStringFromFd003 end.");
}

/*
 * @tc.name: testLoadStringFromFd004
 * @tc.desc: Test loading a newly created file by fd, whose contents are of maximum length
 */
BENCHMARK_F(BenchmarkFileTest, testLoadStringFromFd004)(benchmark::State& state)
{
    BENCHMARK_LOGD("FileTest testLoadStringFromFd004 start.");
    string content(MAX_FILE_LENGTH, 't');
    string filename = FILE_PATH;
    LoadString(filename, content, state);
    BENCHMARK_LOGD("FileTest testLoadStringFromFd004 end.");
}

/*
 * @tc.name: testLoadStringFromFd005
 * @tc.desc: Test loading a newly created file by fd, whose contents exceeds maximum length
 */
BENCHMARK_F(BenchmarkFileTest, testLoadStringFromFd005)(benchmark::State& state)
{
    BENCHMARK_LOGD("FileTest testLoadStringFromFd005 start.");
    string content(EXCEEDS_MAXIMUM_LENGTH, 't');
    string filename = FILE_PATH;
    while (state.KeepRunning()) {
        string result;
        CreateTestFile(filename, content);
        int fd = open(filename.c_str(), O_RDONLY);
        AssertFalse((LoadStringFromFd(fd, result)),
            "LoadStringFromFd(fd, result) did not equal false as expected.", state);
        close(fd);
        RemoveTestFile(filename);
        AssertUnequal(result, content, "result was not different from content as expected.", state);
    }
    BENCHMARK_LOGD("FileTest testLoadStringFromFd005 end.");
}

/*
 * @tc.name: testLoadStringFromFd006
 * @tc.desc: Test loading a newly created file by fd, which is closed ahead of loading.
 */
BENCHMARK_F(BenchmarkFileTest, testLoadStringFromFd006)(benchmark::State& state)
{
    BENCHMARK_LOGD("FileTest testLoadStringFromFd006 start.");
    string filename = FILE_PATH;
    string content = CONTENT_STR;
    while (state.KeepRunning()) {
        string result;
        CreateTestFile(filename, content);
        int fd = open(filename.c_str(), O_RDONLY);
        close(fd);
        AssertFalse((LoadStringFromFd(fd, result)),
            "LoadStringFromFd(fd, result) did not equal false as expected.", state);
        RemoveTestFile(filename);
        AssertEqual(result, "", "result did not equal "" as expected.", state);
    }
    BENCHMARK_LOGD("FileTest testLoadStringFromFd006 end.");
}

/*
 * @tc.name: testSaveStringToFile001
 * @tc.desc: singleton template
 */
BENCHMARK_F(BenchmarkFileTest, testSaveStringToFile001)(benchmark::State& state)
{
    BENCHMARK_LOGD("FileTest testSaveStringToFile001 start.");
    string path = FILE_PATH;
    string content = CONTENT_STR;
    while (state.KeepRunning()) {
        string newContent;
        CreateTestFile(path, content);
        bool ret = SaveStringToFile(path, newContent);
        AssertEqual(ret, true, "ret did not equal true as expected.", state);

        string loadResult;
        AssertTrue((LoadStringFromFile(path, loadResult)),
            "LoadStringFromFile(path, loadResult) did not equal true as expected.", state);
        RemoveTestFile(path);
        AssertEqual(loadResult, content, "loadResult did not equal content as expected.", state);
    }
    BENCHMARK_LOGD("FileTest testSaveStringToFile001 end.");
}

/*
 * @tc.name: testSaveStringToFile002
 * @tc.desc: singleton template
 */
BENCHMARK_F(BenchmarkFileTest, testSaveStringToFile002)(benchmark::State& state)
{
    BENCHMARK_LOGD("FileTest testSaveStringToFile002 start.");
    string path = FILE_PATH;
    string content = "Before truncated！";
    while (state.KeepRunning()) {
        CreateTestFile(path, content);

        string newContent = CONTENT_STR;
        AssertTrue((SaveStringToFile(path, newContent)),
            "SaveStringToFile(path, newContent) did not equal true as expected.", state);

        string loadResult;
        AssertTrue((LoadStringFromFile(path, loadResult)),
            "LoadStringFromFile(path, loadResult) did not equal true as expected.", state);
        RemoveTestFile(path);
        AssertEqual(loadResult, newContent, "loadResult did not equal newContent as expected.", state);
    }
    BENCHMARK_LOGD("FileTest testSaveStringToFile002 end.");
}

/*
 * @tc.name: testSaveStringToFile003
 * @tc.desc: Test writting an empty string to a file in truncate mode
 */
BENCHMARK_F(BenchmarkFileTest, testSaveStringToFile003)(benchmark::State& state)
{
    BENCHMARK_LOGD("FileTest testSaveStringToFile003 start.");
    string path = FILE_PATH;
    string content = "Before truncated！";
    while (state.KeepRunning()) {
        CreateTestFile(path, content);

        string newContent;
        bool ret = SaveStringToFile(path, newContent, true);
        AssertEqual(ret, true, "ret did not equal true as expected.", state);

        string loadResult;
        ret = LoadStringFromFile(path, loadResult);
        RemoveTestFile(path);
        AssertEqual(ret, true, "ret did not equal true as expected.", state);
        AssertEqual(strcmp(loadResult.c_str(), content.c_str()), 0,
            "The two strings, loadResult.c_str() and content.c_str(), did not have the same content as expected.",
            state);
    }
    BENCHMARK_LOGD("FileTest testSaveStringToFile003 end.");
}

/*
 * @tc.name: testSaveStringToFile004
 * @tc.desc: Test writting an empty string to a file in append mode
 */
BENCHMARK_F(BenchmarkFileTest, testSaveStringToFile004)(benchmark::State& state)
{
    BENCHMARK_LOGD("FileTest testSaveStringToFile004 start.");
    string path = FILE_PATH;
    string content = "Before appended！";
    while (state.KeepRunning()) {
        string newContent;
        CreateTestFile(path, content);
        bool ret = SaveStringToFile(path, newContent, false);
        AssertEqual(ret, true, "ret did not equal true as expected.", state);

        string loadResult;
        ret = LoadStringFromFile(path, loadResult);
        RemoveTestFile(path);
        AssertEqual(ret, true, "ret did not equal true as expected.", state);
        AssertEqual(strcmp(loadResult.c_str(), content.c_str()), 0,
            "The two strings, loadResult.c_str() and content.c_str(), did not have the same content as expected.",
            state);
    }
    BENCHMARK_LOGD("FileTest testSaveStringToFile004 end.");
}

/*
 * @tc.name: testSaveStringToFile005
 * @tc.desc: Test writting a string to a file in append mode
 */
BENCHMARK_F(BenchmarkFileTest, testSaveStringToFile005)(benchmark::State& state)
{
    BENCHMARK_LOGD("FileTest testSaveStringToFile005 start.");
    string path = FILE_PATH;
    string content = "Before appended！";
    while (state.KeepRunning()) {
        CreateTestFile(path, content);

        string newContent = CONTENT_STR;
        bool ret = SaveStringToFile(path, newContent, false);
        AssertEqual(ret, true, "ret did not equal true as expected.", state);

        string loadResult;
        ret = LoadStringFromFile(path, loadResult);
        RemoveTestFile(path);
        AssertEqual(ret, true, "ret did not equal true as expected.", state);
        AssertEqual(loadResult, content + newContent,
            "loadResult did not equal content + newContent as expected.", state);
    }
    BENCHMARK_LOGD("FileTest testSaveStringToFile005 end.");
}

/*
 * @tc.name: testSaveStringToFd001
 * @tc.desc: Test writting an empty string to files with invalid fds
 */
BENCHMARK_F(BenchmarkFileTest, testSaveStringToFd001)(benchmark::State& state)
{
    BENCHMARK_LOGD("FileTest testSaveStringToFd001 start.");
    while (state.KeepRunning()) {
        string content;
        bool ret = SaveStringToFd(0, content);
        AssertEqual(ret, false, "ret did not equal false as expected.", state);
        ret = SaveStringToFd(-1, content);
        AssertEqual(ret, false, "ret did not equal false as expected.", state);

        content = CONTENT_STR;
        ret = SaveStringToFd(0, content);
        AssertEqual(ret, false, "ret did not equal false as expected.", state);
        ret = SaveStringToFd(-1, content);
        AssertEqual(ret, false, "ret did not equal false as expected.", state);
    }
    BENCHMARK_LOGD("FileTest testSaveStringToFd001 end.");
}

/*
 * @tc.name: testSaveStringToFd002
 * @tc.desc: Test writting an empty string to a file specified by its fd
 */
BENCHMARK_F(BenchmarkFileTest, testSaveStringToFd002)(benchmark::State& state)
{
    BENCHMARK_LOGD("FileTest testSaveStringToFd002 start.");
    string filename = FILE_PATH;
    string content;
    SaveString(filename, content, state);
    BENCHMARK_LOGD("FileTest testSaveStringToFd002 end.");
}

/*
 * @tc.name: testSaveStringToFd003
 * @tc.desc: Test loading a non-empty string to a file specified by its fd
 */
BENCHMARK_F(BenchmarkFileTest, testSaveStringToFd003)(benchmark::State& state)
{
    BENCHMARK_LOGD("FileTest testSaveStringToFd003 start.");
    string content = CONTENT_STR;
    string filename = FILE_PATH;
    SaveString(filename, content, state);
    BENCHMARK_LOGD("FileTest testSaveStringToFd003 end.");
}

/*
 * @tc.name: testSaveStringToFd004
 * @tc.desc: Test loading a non-empty string to a file without write-authority specified by its fd
 */
BENCHMARK_F(BenchmarkFileTest, testSaveStringToFd004)(benchmark::State& state)
{
    BENCHMARK_LOGD("FileTest testSaveStringToFd004 start.");
    string content = CONTENT_STR;
    string filename = FILE_PATH;
    while (state.KeepRunning()) {
        int fd = open(filename.c_str(), O_RDONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
        bool ret = SaveStringToFd(fd, content);
        close(fd);
        AssertEqual(ret, false, "ret did not equal false as expected.", state);

        string loadResult;
        fd = open(filename.c_str(), O_RDONLY);
        ret = LoadStringFromFd(fd, loadResult);
        close(fd);
        RemoveTestFile(filename);
        AssertEqual(ret, true, "ret did not equal true as expected.", state);
        AssertEqual(loadResult, "", "loadResult did not equal "" as expected.", state);
    }
    BENCHMARK_LOGD("FileTest testSaveStringToFd004 end.");
}

/*
 * @tc.name: testLoadBufferFromFile001
 * @tc.desc: singleton template
 */
BENCHMARK_F(BenchmarkFileTest, testLoadBufferFromFile001)(benchmark::State& state)
{
    BENCHMARK_LOGD("FileTest testLoadBufferFromFile001 start.");
    string filename = "";
    while (state.KeepRunning()) {
        vector<char> buff;
        bool ret = LoadBufferFromFile(filename, buff);
        AssertFalse((ret), "ret did not equal false as expected.", state);
        AssertEqual(0, static_cast<int>(buff.size()),
            "static_cast<int>(buff.size()) did not equal 0 as expected.", state);
    }
    BENCHMARK_LOGD("FileTest testLoadBufferFromFile001 end.");
}

/*
 * @tc.name: testLoadBufferFromFile002
 * @tc.desc: singleton template
 */
BENCHMARK_F(BenchmarkFileTest, testLoadBufferFromFile002)(benchmark::State& state)
{
    BENCHMARK_LOGD("FileTest testLoadBufferFromFile002 start.");
    string filename = FILE_PATH;
    while (state.KeepRunning()) {
        vector<char> buff;
        string content;
        CreateTestFile(filename, content);
        bool ret = LoadBufferFromFile(filename, buff);
        RemoveTestFile(filename);
        AssertTrue((ret), "ret did not equal true as expected.", state);
        AssertEqual(0, static_cast<int>(buff.size()),
            "static_cast<int>(buff.size()) did not equal 0 as expected.", state);
    }
    BENCHMARK_LOGD("FileTest testLoadBufferFromFile002 end.");
}

/*
 * @tc.name: testLoadBufferFromFile003
 * @tc.desc: singleton template
 */
BENCHMARK_F(BenchmarkFileTest, testLoadBufferFromFile003)(benchmark::State& state)
{
    BENCHMARK_LOGD("FileTest testLoadBufferFromFile003 start.");
    string filename = FILE_PATH;
    string content = "TXB";
    while (state.KeepRunning()) {
        vector<char> buff;
        CreateTestFile(filename, content);
        bool ret = LoadBufferFromFile(filename, buff);
        RemoveTestFile(filename);
        AssertTrue((ret), "ret did not equal true as expected.", state);
        AssertEqual(3, (int)buff.size(), "3 did not equal (int)buff.size() as expected.", state);
        AssertEqual('T', buff[0], "'T' did not equal buff[0] as expected.", state);
        AssertEqual('X', buff[1], "'X' did not equal buff[1] as expected.", state);
        AssertEqual('B', buff[2], "'B' did not equal buff[2] as expected.", state);
    }
    BENCHMARK_LOGD("FileTest testLoadBufferFromFile003 end.");
}

/*
 * @tc.name: testLoadBufferFromFile004
 * @tc.desc: singleton template
 */
BENCHMARK_F(BenchmarkFileTest, testLoadBufferFromFile004)(benchmark::State& state)
{
    BENCHMARK_LOGD("FileTest testLoadBufferFromFile004 start.");
    string content(EXCEEDS_MAXIMUM_LENGTH, 't');
    string filename = FILE_PATH;
    while (state.KeepRunning()) {
        vector<char> buff;
        CreateTestFile(filename, content);
        bool ret = LoadBufferFromFile(filename, buff);
        RemoveTestFile(filename);
        AssertEqual(ret, false, "ret did not equal false as expected.", state);
        AssertEqual(0, static_cast<int>(buff.size()),
            "static_cast<int>(buff.size()) did not equal 0 as expected.", state);
    }
    BENCHMARK_LOGD("FileTest testLoadBufferFromFile004 end.");
}

/*
 * @tc.name: testSaveBufferToFile001
 * @tc.desc: singleton template
 */
BENCHMARK_F(BenchmarkFileTest, testSaveBufferToFile001)(benchmark::State& state)
{
    BENCHMARK_LOGD("FileTest testSaveBufferToFile001 start.");
    string path = FILE_PATH;
    string content = "ttxx";
    while (state.KeepRunning()) {
        vector<char> buff;
        CreateTestFile(path, content);
        bool ret = SaveBufferToFile(path, buff, false);
        AssertEqual(ret, true, "ret did not equal true as expected.", state);

        string loadResult;
        ret = LoadStringFromFile(path, loadResult);
        RemoveTestFile(path);
        AssertEqual(ret, true, "ret did not equal true as expected.", state);
        AssertEqual(loadResult, content, "loadResult did not equal content as expected.", state);
    }
    BENCHMARK_LOGD("FileTest testSaveBufferToFile001 end.");
}

/*
 * @tc.name: testSaveBufferToFile002
 * @tc.desc: singleton template
 */
BENCHMARK_F(BenchmarkFileTest, testSaveBufferToFile002)(benchmark::State& state)
{
    BENCHMARK_LOGD("FileTest testSaveBufferToFile002 start.");
    string path = FILE_PATH;
    string content = "ttxx";
    while (state.KeepRunning()) {
        CreateTestFile(path, content);

        vector<char> newContent = {'x', 'x', 't', 't'};
        bool ret = SaveBufferToFile(path, newContent);
        AssertEqual(ret, true, "ret did not equal true as expected.", state);

        string loadResult;
        ret = LoadStringFromFile(path, loadResult);
        RemoveTestFile(path);
        AssertEqual(ret, true, "ret did not equal true as expected.", state);
        AssertEqual(loadResult, std::string(newContent.begin(), newContent.end()),
            "loadResult did not equal std::string(newContent.begin(), newContent.end()) as expected.", state);
    }
    BENCHMARK_LOGD("FileTest testSaveBufferToFile002 end.");
}

/*
 * @tc.name: testSaveBufferToFile003
 * @tc.desc: singleton template
 */
BENCHMARK_F(BenchmarkFileTest, testSaveBufferToFile003)(benchmark::State& state)
{
    BENCHMARK_LOGD("FileTest testSaveBufferToFile003 start.");
    string path = FILE_PATH;
    string content = "ttxx";
    while (state.KeepRunning()) {
        CreateTestFile(path, content);

        vector<char> newContent = {'x', 'x', 't', 't'};
        bool ret = SaveBufferToFile(path, newContent, false);
        AssertEqual(ret, true, "ret did not equal true as expected.", state);

        string loadResult;
        ret = LoadStringFromFile(path, loadResult);
        RemoveTestFile(path);
        AssertEqual(ret, true, "ret did not equal true as expected.", state);
        AssertEqual(loadResult, content + std::string(newContent.begin(), newContent.end()),
            "loadResult did not equal content + std::string(newContent.begin(), newContent.end()).", state);
    }
    BENCHMARK_LOGD("FileTest testSaveBufferToFile003 end.");
}

/*
 * @tc.name: testStringExistsInFile001
 * @tc.desc: singleton template
 */
BENCHMARK_F(BenchmarkFileTest, testStringExistsInFile001)(benchmark::State& state)
{
    BENCHMARK_LOGD("FileTest testStringExistsInFile001 start.");
    string str = "abc";
    string filename = "";
    while (state.KeepRunning()) {
        AssertFalse((StringExistsInFile(filename, str, true)),
            "StringExistsInFile(filename, str, true) did not equal false as expected.", state);
        AssertFalse((str.empty()), "str.empty() did not equal false as expected.", state);
    }
    BENCHMARK_LOGD("FileTest testStringExistsInFile001 end.");
}

/*
 * @tc.name: testStringExistsInFile002
 * @tc.desc: singleton template
 */
BENCHMARK_F(BenchmarkFileTest, testStringExistsInFile002)(benchmark::State& state)
{
    BENCHMARK_LOGD("FileTest testStringExistsInFile002 start.");
    string str = NULL_STR;
    string filename = FILE_PATH;
    string content = "hello world!";
    while (state.KeepRunning()) {
        CreateTestFile(filename, content);
        AssertFalse((StringExistsInFile(filename, str, true)),
            "StringExistsInFile(filename, str, true) did not equal false as expected.", state);
        RemoveTestFile(filename);
    }
    BENCHMARK_LOGD("FileTest testStringExistsInFile002 end.");
}

/*
 * @tc.name: testStringExistsInFile003
 * @tc.desc: singleton template
 */
BENCHMARK_F(BenchmarkFileTest, testStringExistsInFile003)(benchmark::State& state)
{
    BENCHMARK_LOGD("FileTest testStringExistsInFile003 start.");
    string str = "world";
    string filename = FILE_PATH;
    string content = "hello world!";
    while (state.KeepRunning()) {
        CreateTestFile(filename, content);
        AssertTrue((StringExistsInFile(filename, str, true)),
            "StringExistsInFile(filename, str, true) did not equal true as expected.", state);
        RemoveTestFile(filename);
    }
    BENCHMARK_LOGD("FileTest testStringExistsInFile003 end.");
}

/*
 * @tc.name: testStringExistsInFile004
 * @tc.desc: singleton template
 */
BENCHMARK_F(BenchmarkFileTest, testStringExistsInFile004)(benchmark::State& state)
{
    BENCHMARK_LOGD("FileTest testStringExistsInFile004 start.");
    string str1(MAX_FILE_LENGTH + 1, 't');
    string str2(MAX_FILE_LENGTH, 't');
    string content(MAX_FILE_LENGTH, 't');
    string filename = FILE_PATH;
    while (state.KeepRunning()) {
        CreateTestFile(filename, content);
        AssertFalse((StringExistsInFile(filename, str1, true)),
            "StringExistsInFile(filename, str1, true) did not equal false as expected.", state);
        AssertTrue((StringExistsInFile(filename, str2, true)),
            "StringExistsInFile(filename, str2, true) did not equal true as expected.", state);
        RemoveTestFile(filename);
    }
    BENCHMARK_LOGD("FileTest testStringExistsInFile004 end.");
}

/*
 * @tc.name: testStringExistsInFile005
 * @tc.desc: singleton template
 */
BENCHMARK_F(BenchmarkFileTest, testStringExistsInFile005)(benchmark::State& state)
{
    BENCHMARK_LOGD("FileTest testStringExistsInFile005 start.");
    string str = "woRld";
    string filename = FILE_PATH;
    string content = "hello world!";
    while (state.KeepRunning()) {
        CreateTestFile(filename, content);
        AssertTrue((StringExistsInFile(filename, str, false)),
            "StringExistsInFile(filename, str, false) did not equal true as expected.", state);
        AssertFalse((StringExistsInFile(filename, str, true)),
            "StringExistsInFile(filename, str, true) did not equal false as expected.", state);
        RemoveTestFile(filename);
    }
    BENCHMARK_LOGD("FileTest testStringExistsInFile005 end.");
}

/*
 * @tc.name: testStringExistsInFile006
 * @tc.desc: singleton template
 */
BENCHMARK_F(BenchmarkFileTest, testStringExistsInFile006)(benchmark::State& state)
{
    BENCHMARK_LOGD("FileTest testStringExistsInFile006 start.");
    string str1 = "woRld!";
    string str2 = "123";
    string str3 = "llo ";
    string str4 = "123 w";
    string str5 = "hi";
    string filename = FILE_PATH;
    string content = "Test, hello 123 World!";
    while (state.KeepRunning()) {
        CreateTestFile(filename, content);
        AssertTrue((StringExistsInFile(filename, str1, false)),
            "StringExistsInFile(filename, str1, false) did not equal true as expected.", state);
        AssertFalse((StringExistsInFile(filename, str1, true)),
            "StringExistsInFile(filename, str1, true) did not equal false as expected.", state);

        AssertTrue((StringExistsInFile(filename, str2, false)),
            "StringExistsInFile(filename, str2, false) did not equal true as expected.", state);
        AssertTrue((StringExistsInFile(filename, str2, true)),
            "StringExistsInFile(filename, str2, true) did not equal true as expected.", state);

        AssertTrue((StringExistsInFile(filename, str3, false)),
            "StringExistsInFile(filename, str3, false) did not equal true as expected.", state);
        AssertTrue((StringExistsInFile(filename, str3, true)),
            "StringExistsInFile(filename, str3, true) did not equal true as expected.", state);

        AssertTrue((StringExistsInFile(filename, str4, false)),
            "StringExistsInFile(filename, str4, false) did not equal true as expected.", state);
        AssertFalse((StringExistsInFile(filename, str4, true)),
            "StringExistsInFile(filename, str4, true) did not equal false as expected.", state);

        AssertFalse((StringExistsInFile(filename, str5, false)),
            "StringExistsInFile(filename, str5, false) did not equal false as expected.", state);
        AssertFalse((StringExistsInFile(filename, str5, true)),
            "StringExistsInFile(filename, str5, true) did not equal false as expected.", state);
        RemoveTestFile(filename);
    }
    BENCHMARK_LOGD("FileTest testStringExistsInFile006 end.");
}

/*
 * @tc.name: testStringExistsInFile007
 * @tc.desc: singleton template
 */
BENCHMARK_F(BenchmarkFileTest, testStringExistsInFile007)(benchmark::State& state)
{
    BENCHMARK_LOGD("FileTest testStringExistsInFile007 start.");
    string str1 = "is";
    string str2 = "\n\ris";
    string filename = FILE_PATH;
    string content = "Test, special string\n\ris ok";
    while (state.KeepRunning()) {
        CreateTestFile(filename, content);
        AssertTrue((StringExistsInFile(filename, str1, false)),
            "StringExistsInFile(filename, str1, false) did not equal true as expected.", state);
        AssertTrue((StringExistsInFile(filename, str1, true)),
            "StringExistsInFile(filename, str1, true) did not equal true as expected.", state);

        AssertTrue((StringExistsInFile(filename, str2, false)),
            "StringExistsInFile(filename, str2, false) did not equal true as expected.", state);
        AssertTrue((StringExistsInFile(filename, str2, true)),
            "StringExistsInFile(filename, str2, true) did not equal true as expected.", state);
        RemoveTestFile(filename);
    }
    BENCHMARK_LOGD("FileTest testStringExistsInFile007 end.");
}

/*
 * @tc.name: testFileExist001
 * @tc.desc: singleton template
 */
BENCHMARK_F(BenchmarkFileTest, testFileExist001)(benchmark::State& state)
{
    BENCHMARK_LOGD("FileTest testFileExist001 start.");
    string filepath = "/proc/meminfo";
    string filepath1 = "/proc/meminfo1";
    while (state.KeepRunning()) {
        AssertTrue((FileExists(filepath)), "FileExists(filepath) did not equal true as expected.", state);
        AssertFalse((FileExists(filepath1)), "FileExists(filepath1) did not equal false as expected.", state);
    }
    BENCHMARK_LOGD("FileTest testFileExist001 end.");
}

/*
 * @tc.name: testCountStrInFile001
 * @tc.desc: singleton template
 */
BENCHMARK_F(BenchmarkFileTest, testCountStrInFile001)(benchmark::State& state)
{
    BENCHMARK_LOGD("FileTest testCountStrInFile001 start.");
    string str = "abc";
    string filename = "";
    while (state.KeepRunning()) {
        AssertEqual(CountStrInFile(filename, str, true), -1,
            "CountStrInFile(filename, str, true) did not equal -1 as expected.", state);
        AssertFalse((str.empty()), "str.empty() did not equal false as expected.", state);
    }
    BENCHMARK_LOGD("FileTest testCountStrInFile001 end.");
}

/*
 * @tc.name: testCountStrInFile002
 * @tc.desc: singleton template
 */
BENCHMARK_F(BenchmarkFileTest, testCountStrInFile002)(benchmark::State& state)
{
    BENCHMARK_LOGD("FileTest testCountStrInFile002 start.");
    string str = NULL_STR;
    string filename = FILE_PATH;
    string content = "hello world!";
    while (state.KeepRunning()) {
        CreateTestFile(filename, content);
        AssertEqual(CountStrInFile(filename, str, true), -1,
            "CountStrInFile(filename, str, true) did not equal -1 as expected.", state);
        RemoveTestFile(filename);
    }
    BENCHMARK_LOGD("FileTest testCountStrInFile002 end.");
}

/*
 * @tc.name: testCountStrInFile003
 * @tc.desc: singleton template
 */
BENCHMARK_F(BenchmarkFileTest, testCountStrInFile003)(benchmark::State& state)
{
    BENCHMARK_LOGD("FileTest testCountStrInFile003 start.");
    string str1(MAX_FILE_LENGTH + 1, 't');
    string str2(MAX_FILE_LENGTH, 't');
    string content(MAX_FILE_LENGTH, 't');
    string filename = FILE_PATH;
    while (state.KeepRunning()) {
        CreateTestFile(filename, content);
        AssertEqual(CountStrInFile(filename, str1, true), 0,
            "CountStrInFile(filename, str1, true) did not equal 0 as expected.", state);
        AssertEqual(CountStrInFile(filename, str2, true), 1,
            "CountStrInFile(filename, str2, true) did not equal 1 as expected.", state);
        RemoveTestFile(filename);
    }
    BENCHMARK_LOGD("FileTest testCountStrInFile003 end.");
}

/*
 * @tc.name: testCountStrInFile004
 * @tc.desc: singleton template
 */
BENCHMARK_F(BenchmarkFileTest, testCountStrInFile004)(benchmark::State& state)
{
    BENCHMARK_LOGD("FileTest testCountStrInFile004 start.");
    string str1 = "very";
    string str2 = "VERY";
    string str3 = "abc";
    string filename = FILE_PATH;
    string content = "This is very very long string for test.\n Very Good,\r VERY HAPPY.";
    while (state.KeepRunning()) {
        CreateTestFile(filename, content);
        AssertEqual(CountStrInFile(filename, str1, true), 2,
            "CountStrInFile(filename, str1, true) did not equal 2 as expected.", state);
        AssertEqual(CountStrInFile(filename, str1, false), 4,
            "CountStrInFile(filename, str1, false) did not equal 4 as expected.", state);

        AssertEqual(CountStrInFile(filename, str2, true), 1,
            "CountStrInFile(filename, str2, true) did not equal 1 as expected.", state);
        AssertEqual(CountStrInFile(filename, str2, false), 4,
            "CountStrInFile(filename, str2, false) did not equal 4 as expected.", state);

        AssertEqual(CountStrInFile(filename, str3, true), 0,
            "CountStrInFile(filename, str3, true) did not equal 0 as expected.", state);
        RemoveTestFile(filename);
    }
    BENCHMARK_LOGD("FileTest testCountStrInFile004 end.");
}

/*
 * @tc.name: testCountStrInFile005
 * @tc.desc: singleton template
 */
BENCHMARK_F(BenchmarkFileTest, testCountStrInFile005)(benchmark::State& state)
{
    BENCHMARK_LOGD("FileTest testCountStrInFile005 start.");
    string str1 = "aba";
    string filename = FILE_PATH;
    string content = "This is abababaBABa.";
    while (state.KeepRunning()) {
        CreateTestFile(filename, content);
        AssertEqual(CountStrInFile(filename, str1, true), 2,
            "CountStrInFile(filename, str1, true) did not equal 2 as expected.", state);
        AssertEqual(CountStrInFile(filename, str1, false), 3,
            "CountStrInFile(filename, str1, false) did not equal 3 as expected.", state);
        RemoveTestFile(filename);
    }
    BENCHMARK_LOGD("FileTest testCountStrInFile005 end.");
}
}  // namespace
}  // namespace OHOS
// Run the benchmark
BENCHMARK_MAIN();