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
#include "directory_ex.h"
#include <fcntl.h>
#include <algorithm>
#include <iostream>
#include <fstream>
#include <unistd.h>
#include "benchmark_log.h"
#include "benchmark_assert.h"
using namespace std;

namespace OHOS {
namespace {

class BenchmarkDirectoryTest : public benchmark::Fixture {
public:
    void SetUp(const ::benchmark::State& state) override
    {
    }

    void TearDown(const ::benchmark::State& state) override
    {
    }

    BenchmarkDirectoryTest()
    {
        Iterations(iterations);
        Repetitions(repetitions);
        ReportAggregatesOnly();
    }

    ~BenchmarkDirectoryTest() override = default;

protected:
    const int32_t repetitions = 3;
    const int32_t iterations = 1000;
};

/*
 * @tc.name: testGetCurrentProcFullFileName001
 * @tc.desc: get the directory of directorytest
 */
BENCHMARK_F(BenchmarkDirectoryTest, testGetCurrentProcFullFileName001)(benchmark::State& state)
{
    BENCHMARK_LOGD("DirectoryTest testGetCurrentProcFullFileName001 start.");
    while (state.KeepRunning()) {
        string strBaseName = "/data/test/DirectoryTest";
        string strFilename = GetCurrentProcFullFileName();
        AssertEqual(strFilename, strBaseName, "strFilename did not equal strBaseName as expected.", state);
    }
    BENCHMARK_LOGD("DirectoryTest testGetCurrentProcFullFileName001 end.");
}

/*
 * @tc.name: testGetCurrentProcPath001
 * @tc.desc: get the path of directorytest
 */
BENCHMARK_F(BenchmarkDirectoryTest, testGetCurrentProcPath001)(benchmark::State& state)
{
    BENCHMARK_LOGD("DirectoryTest testGetCurrentProcPath001 start.");
    while (state.KeepRunning()) {
        string strPathName = "/data/test/";
        string strCurPathName = GetCurrentProcPath();
        AssertEqual(strCurPathName, strPathName, "strCurPathName did not equal strPathName as expected.", state);
    }
    BENCHMARK_LOGD("DirectoryTest testGetCurrentProcPath001 end.");
}

/*
 * @tc.name: testExtractFilePath001
 * @tc.desc: get the filename of the path
 */
BENCHMARK_F(BenchmarkDirectoryTest, testExtractFilePath001)(benchmark::State& state)
{
    BENCHMARK_LOGD("DirectoryTest testExtractFilePath001 start.");
    while (state.KeepRunning()) {
        string strFilePath = "/data/test/";
        string strPath = ExtractFilePath(GetCurrentProcFullFileName());
        AssertEqual(strFilePath, strPath, "strFilePath did not equal strPath as expected.", state);
    }
    BENCHMARK_LOGD("DirectoryTest testExtractFilePath001 end.");
}

/*
 * @tc.name: testExtractFileName001
 * @tc.desc: get the filename of the path
 */
BENCHMARK_F(BenchmarkDirectoryTest, testExtractFileName001)(benchmark::State& state)
{
    BENCHMARK_LOGD("DirectoryTest testExtractFileName001 start.");
    while (state.KeepRunning()) {
        string strBaseName = "DirectoryTest";
        string strName = ExtractFileName(GetCurrentProcFullFileName());
        AssertEqual(strBaseName, strName, "strBaseName did not equal strName as expected.", state);
    }
    BENCHMARK_LOGD("DirectoryTest testExtractFileName001 end.");
}

/*
 * @tc.name: testExtractFileExt001
 * @tc.desc: get the filename of the path
 */
BENCHMARK_F(BenchmarkDirectoryTest, testExtractFileExt001)(benchmark::State& state)
{
    BENCHMARK_LOGD("DirectoryTest testExtractFileExt001 start.");
    while (state.KeepRunning()) {
        string strBaseName = "test/test.txt";
        string strTypeName = ExtractFileExt(strBaseName);
        AssertEqual(strTypeName, "txt", "strTypeName did not equal \"txt\" as expected.", state);
    }
    BENCHMARK_LOGD("DirectoryTest testExtractFileExt001 end.");
}

/*
 * @tc.name: testExtractFileExt002
 * @tc.desc: get the filename of the path and test whether the filename contains "."
 */
BENCHMARK_F(BenchmarkDirectoryTest, testExtractFileExt002)(benchmark::State& state)
{
    BENCHMARK_LOGD("DirectoryTest testExtractFileExt002 start.");
    while (state.KeepRunning()) {
        string strBaseName = "test/test_txt";
        string strTypeName = ExtractFileExt(strBaseName);
        AssertEqual(strTypeName, "", "strTypeName did not equal \"\" as expected.", state);
    }
    BENCHMARK_LOGD("DirectoryTest testExtractFileExt002 end.");
}

/*
 * @tc.name: testExcludeTrailingPathDelimiter001
 * @tc.desc: directory unit test
 */
BENCHMARK_F(BenchmarkDirectoryTest, testExcludeTrailingPathDelimiter001)(benchmark::State& state)
{
    BENCHMARK_LOGD("DirectoryTest testExcludeTrailingPathDelimiter001 start.");
    while (state.KeepRunning()) {
        string strResult = "data/test/DirectoryTest";
        string strName = ExcludeTrailingPathDelimiter("data/test/DirectoryTest/");
        AssertEqual(strResult, strName, "strResult did not equal strName as expected.", state);
    }
    BENCHMARK_LOGD("DirectoryTest testExcludeTrailingPathDelimiter001 end.");
}

/*
 * @tc.name: testIncludeTrailingPathDelimiter001
 * @tc.desc: directory unit test
 */
BENCHMARK_F(BenchmarkDirectoryTest, testIncludeTrailingPathDelimiter001)(benchmark::State& state)
{
    BENCHMARK_LOGD("DirectoryTest testIncludeTrailingPathDelimiter001 start.");
    while (state.KeepRunning()) {
        string strResult = "data/test/DirectoryTest/";
        string strName = IncludeTrailingPathDelimiter("data/test/DirectoryTest");
        AssertEqual(strResult, strName, "strResult did not equal strName as expected.", state);
    }
    BENCHMARK_LOGD("DirectoryTest testIncludeTrailingPathDelimiter001 end.");
}

/*
 * @tc.name: testGetDirFiles001
 * @tc.desc: directory unit test
 */
BENCHMARK_F(BenchmarkDirectoryTest, testGetDirFiles001)(benchmark::State& state)
{
    BENCHMARK_LOGD("DirectoryTest testGetDirFiles001 start.");
    while (state.KeepRunning()) {
        string resultfile[2] = { "/data/test/TestFile.txt", "/data/test/DirectoryTest" };
        // prepare test data
        ofstream file(resultfile[0], fstream::out);

        string dirpath = "/data/";
        vector<string> filenames;
        GetDirFiles(dirpath, filenames);
        auto pos = find(filenames.begin(), filenames.end(), resultfile[0]);
        AssertUnequal(pos, filenames.end(), "pos was not different from filenames.end() as expected.", state);

        pos = find(filenames.begin(), filenames.end(), resultfile[1]);
        AssertUnequal(pos, filenames.end(), "pos was not different from filenames.end() as expected.", state);

        // delete test data
        RemoveFile(resultfile[0]);
    }
    BENCHMARK_LOGD("DirectoryTest testGetDirFiles001 end.");
}

/*
 * @tc.name: testForceCreateDirectory001
 * @tc.desc: directory unit test
 */
BENCHMARK_F(BenchmarkDirectoryTest, testForceCreateDirectory001)(benchmark::State& state)
{
    BENCHMARK_LOGD("DirectoryTest testForceCreateDirectory001 start.");
    while (state.KeepRunning()) {
        string dirpath = "/data/test_dir/test2/test3";
        bool ret = ForceCreateDirectory(dirpath);
        AssertEqual(ret, true, "ret did not equal true as expected.", state);
        ret = IsEmptyFolder(dirpath);
        AssertEqual(ret, true, "ret did not equal true as expected.", state);
    }
    BENCHMARK_LOGD("DirectoryTest testForceCreateDirectory001 end.");
}

/*
 * @tc.name: testForceRemoveDirectory001
 * @tc.desc: directory unit test
 */
BENCHMARK_F(BenchmarkDirectoryTest, testForceRemoveDirectory001)(benchmark::State& state)
{
    BENCHMARK_LOGD("DirectoryTest testForceRemoveDirectory001 start.");
    while (state.KeepRunning()) {
        string dirpath = "/data/test_dir";
        bool ret = ForceCreateDirectory(dirpath);
        AssertEqual(ret, true, "ret did not equal true as expected.", state);
        ret = ForceRemoveDirectory(dirpath);
        AssertEqual(ret, true, "ret did not equal true as expected.", state);
    }
    BENCHMARK_LOGD("DirectoryTest testForceRemoveDirectory001 end.");
}

/*
 * @tc.name: testForceRemoveDirectory002
 * @tc.desc: test whether the folder exists
 */
BENCHMARK_F(BenchmarkDirectoryTest, testForceRemoveDirectory002)(benchmark::State& state)
{
    BENCHMARK_LOGD("DirectoryTest testForceRemoveDirectory002 start.");
    while (state.KeepRunning()) {
        string dirpath = "/data/test/utils_directory_tmp/";
        bool ret = ForceRemoveDirectory(dirpath);
        AssertEqual(ret, false, "ret did not equal false as expected.", state);
    }
    BENCHMARK_LOGD("DirectoryTest testForceRemoveDirectory002 end.");
}

/*
 * @tc.name: testRemoveFile001
 * @tc.desc: directory unit test
 */
BENCHMARK_F(BenchmarkDirectoryTest, testRemoveFile001)(benchmark::State& state)
{
    BENCHMARK_LOGD("DirectoryTest testRemoveFile001 start.");
    while (state.KeepRunning()) {
        string dirpath = "/data/test_dir";
        bool ret = ForceCreateDirectory(dirpath);
        AssertEqual(ret, true, "ret did not equal true as expected.", state);
        string filename = dirpath + "/test.txt";
        FILE *fp = fopen(filename.c_str(), "w");
        if (fp != nullptr) {
            fclose(fp);
            ret = RemoveFile(filename);
            AssertEqual(ret, true, "ret did not equal true as expected.", state);
        }
        ret = ForceRemoveDirectory(dirpath);
        AssertEqual(ret, true, "ret did not equal true as expected.", state);
    }
    BENCHMARK_LOGD("DirectoryTest testRemoveFile001 end.");
}

/*
 * @tc.name: testRemoveFile002
 * @tc.desc: Remove soft link file.
 */
BENCHMARK_F(BenchmarkDirectoryTest, testRemoveFile002)(benchmark::State& state)
{
    BENCHMARK_LOGD("DirectoryTest testRemoveFile002 start.");
    while (state.KeepRunning()) {
        string dirpath = "/data/test_dir";
        bool ret = ForceCreateDirectory(dirpath);
        AssertEqual(ret, true, "ret did not equal true as expected.", state);

        string targetname = "/data/test_target.txt";
        FILE *fp = fopen(targetname.c_str(), "w");
        if (fp != nullptr) {
            fclose(fp);
        }

        // symlink to a directory
        string linkpath = "/data/test_symlink_dir";
        int res = symlink(dirpath.c_str(), linkpath.c_str());
        AssertEqual(res, 0, "res did not equal 0 as expected.", state);

        ret = ForceRemoveDirectory(linkpath);
        AssertEqual(ret, true, "ret did not equal true as expected.", state);

        // Target dir is not removed.
        ret = faccessat(AT_FDCWD, dirpath.c_str(), F_OK, AT_SYMLINK_NOFOLLOW);
        AssertEqual(ret, 0, "ret did not equal 0 as expected.", state);

        // symlink to a file
        string filename = dirpath + "/test.txt";
        res = symlink(targetname.c_str(), filename.c_str());
        AssertEqual(res, 0, "res did not equal 0 as expected.", state);

        ret = ForceRemoveDirectory(dirpath);
        AssertEqual(ret, true, "ret did not equal true as expected.", state);

        // Target file is not removed.
        ret = faccessat(AT_FDCWD, targetname.c_str(), F_OK, AT_SYMLINK_NOFOLLOW);
        AssertEqual(ret, 0, "ret did not equal 0 as expected.", state);

        ret = RemoveFile(targetname);
        AssertEqual(ret, true, "ret did not equal true as expected.", state);
    }
    BENCHMARK_LOGD("DirectoryTest testRemoveFile002 end.");
}

/*
 * @tc.name: testRemoveFile003
 * @tc.desc: Remove dangling soft link file.
 */
BENCHMARK_F(BenchmarkDirectoryTest, testRemoveFile003)(benchmark::State& state)
{
    BENCHMARK_LOGD("DirectoryTest testRemoveFile003 start.");
    while (state.KeepRunning()) {
        string dirpath = "/data/test_dir";
        bool ret = ForceCreateDirectory(dirpath);
        AssertEqual(ret, true, "ret did not equal true as expected.", state);

        // symlink to a file
        string targetname = "/data/nonexisted.txt";
        string filename = dirpath + "/test.txt";
        int res = symlink(targetname.c_str(), filename.c_str());
        AssertEqual(res, 0, "res did not equal 0 as expected.", state);

        ret = ForceRemoveDirectory(dirpath);
        AssertEqual(ret, true, "ret did not equal true as expected.", state);

        ret = RemoveFile(targetname);
        AssertEqual(ret, true, "ret did not equal true as expected.", state);
    }
    BENCHMARK_LOGD("DirectoryTest testRemoveFile003 end.");
}

/*
 * @tc.name: testGetFolderSize001
 * @tc.desc: directory unit test
 */
BENCHMARK_F(BenchmarkDirectoryTest, testGetFolderSize001)(benchmark::State& state)
{
    BENCHMARK_LOGD("DirectoryTest testGetFolderSize001 start.");
    while (state.KeepRunning()) {
        string dirpath = "/data/test_folder/";
        bool ret = ForceCreateDirectory(dirpath);
        AssertEqual(ret, true, "ret did not equal true as expected.", state);
        ofstream out(dirpath + "test.txt");
        if (out.is_open()) {
            out << "This is a line.\n";
            out << "This is another line.\n";
            out.close();
        }
        uint64_t resultsize = GetFolderSize(dirpath);
        uint64_t resultcomp = 38;
        AssertEqual(resultsize, resultcomp, "resultsize did not equal resultcomp as expected.", state);

        mode_t mode = S_IRWXU | S_IRWXG | S_IRWXO;
        ret = ChangeModeFile(dirpath + "test.txt", mode);
        AssertEqual(ret, true, "ret did not equal true as expected.", state);

        mode = S_IRUSR  | S_IRGRP | S_IROTH;
        ret = ChangeModeDirectory(dirpath, mode);
        AssertEqual(ret, true, "ret did not equal true as expected.", state);

        ret = ForceRemoveDirectory(dirpath);
        AssertEqual(ret, true, "ret did not equal true as expected.", state);
    }
    BENCHMARK_LOGD("DirectoryTest testGetFolderSize001 end.");
}

/*
 * @tc.name: testChangeModeFile001
 * @tc.desc: test whether the folder exists
 */
BENCHMARK_F(BenchmarkDirectoryTest, testChangeModeFile001)(benchmark::State& state)
{
    BENCHMARK_LOGD("DirectoryTest testChangeModeFile001 start.");
    while (state.KeepRunning()) {
        string dirpath = "/data/test/utils_directory_tmp/";
        mode_t mode = S_IRWXU | S_IRWXG | S_IRWXO;
        bool ret = ChangeModeFile(dirpath + "test.txt", mode);
        AssertEqual(ret, false, "ret did not equal false as expected.", state);
    }
    BENCHMARK_LOGD("DirectoryTest testChangeModeFile001 end.");
}

/*
 * @tc.name: testChangeModeDirectory001
 * @tc.desc: test whether the folder is empty and get the size of the folder
 */
BENCHMARK_F(BenchmarkDirectoryTest, testChangeModeDirectory001)(benchmark::State& state)
{
    BENCHMARK_LOGD("DirectoryTest testChangeModeDirectory001 start.");
    while (state.KeepRunning()) {
        string dirpath = "";
        mode_t mode = S_IRUSR  | S_IRGRP | S_IROTH;
        bool ret = ChangeModeDirectory(dirpath, mode);
        AssertEqual(ret, false, "ret did not equal false as expected.", state);

        uint64_t resultsize = GetFolderSize(dirpath);
        uint64_t resultcomp = 0;
        AssertEqual(resultsize, resultcomp, "resultsize did not equal resultcomp as expected.", state);
    }
    BENCHMARK_LOGD("DirectoryTest testChangeModeDirectory001 end.");
}

/*
 * @tc.name: testPathToRealPath001
 * @tc.desc: directory unit test
 */
BENCHMARK_F(BenchmarkDirectoryTest, testPathToRealPath001)(benchmark::State& state)
{
    BENCHMARK_LOGD("DirectoryTest testPathToRealPath001 start.");
    while (state.KeepRunning()) {
        string path = "/data/test";
        string realpath;
        bool ret = PathToRealPath(path, realpath);
        AssertEqual(ret, true, "ret did not equal true as expected.", state);
        AssertEqual(path, realpath, "path did not equal realpath as expected.", state);
    }
    BENCHMARK_LOGD("DirectoryTest testPathToRealPath001 end.");
}

/*
 * @tc.name: testPathToRealPath002
 * @tc.desc: directory unit test
 */
BENCHMARK_F(BenchmarkDirectoryTest, testPathToRealPath002)(benchmark::State& state)
{
    BENCHMARK_LOGD("DirectoryTest testPathToRealPath002 start.");
    while (state.KeepRunning()) {
        string path = "/data/../data/test";
        string realpath;
        bool ret = PathToRealPath(path, realpath);
        AssertEqual(ret, true, "ret did not equal true as expected.", state);
        AssertEqual("/data/test", realpath, "\"/data/test\" did not equal realpath as expected.", state);
    }
    BENCHMARK_LOGD("DirectoryTest testPathToRealPath002 end.");
}

/*
 * @tc.name: testPathToRealPath003
 * @tc.desc: directory unit test
 */
BENCHMARK_F(BenchmarkDirectoryTest, testPathToRealPath003)(benchmark::State& state)
{
    BENCHMARK_LOGD("DirectoryTest testPathToRealPath003 start.");
    while (state.KeepRunning()) {
        string path = "./";
        string realpath;
        bool ret = PathToRealPath(path, realpath);
        AssertEqual(ret, true, "ret did not equal true as expected.", state);
        AssertEqual("/data/test", realpath, "\"/data/test\" did not equal realpath as expected.", state);
    }
    BENCHMARK_LOGD("DirectoryTest testPathToRealPath003 end.");
}

/*
 * @tc.name: testPathToRealPath004
 * @tc.desc: directory unit test
 */
BENCHMARK_F(BenchmarkDirectoryTest, testPathToRealPath004)(benchmark::State& state)
{
    BENCHMARK_LOGD("DirectoryTest testPathToRealPath004 start.");
    while (state.KeepRunning()) {
        string path = "";
        string realpath;
        bool ret = PathToRealPath(path, realpath);
        AssertEqual(ret, false, "ret did not equal false as expected.", state);
    }
    BENCHMARK_LOGD("DirectoryTest testPathToRealPath004 end.");
}

/*
 * @tc.name: testPathToRealPath005
 * @tc.desc: directory unit test
 */
BENCHMARK_F(BenchmarkDirectoryTest, testPathToRealPath005)(benchmark::State& state)
{
    BENCHMARK_LOGD("DirectoryTest testPathToRealPath005 start.");
    while (state.KeepRunning()) {
        string path = "/data/test/data/test/data/test/data/test/data/test/data/ \
            test/data/test/data/test/data/test/data/test/data/test/data/test/data/ \
            test/data/test/data/test/data/test/data/test/data/test/data/test/data/ \
            test/data/test/data/test/data/test/data/test/data/test/data/test/data/ \
            test/data/test/data/test/data/test";
        string realpath;
        bool ret = PathToRealPath(path, realpath);
        AssertEqual(ret, false, "ret did not equal false as expected.", state);
    }
    BENCHMARK_LOGD("DirectoryTest testPathToRealPath005 end.");
}

/*
 * @tc.name: testPathToRealPath006
 * @tc.desc: test whether the folder exists
 */
BENCHMARK_F(BenchmarkDirectoryTest, testPathToRealPath006)(benchmark::State& state)
{
    BENCHMARK_LOGD("DirectoryTest testPathToRealPath006 start.");
    while (state.KeepRunning()) {
        string path(PATH_MAX, 'x');
        string realpath;
        bool ret = PathToRealPath(path, realpath);
        AssertEqual(ret, false, "ret did not equal false as expected.", state);
    }
    BENCHMARK_LOGD("DirectoryTest testPathToRealPath006 end.");
}

/*
 * @tc.name: testTransformFileName001
 * @tc.desc: test transform the file name
 */
#if defined(IOS_PLATFORM) || defined(_WIN32)
BENCHMARK_F(BenchmarkDirectoryTest, testTransformFileName001)(benchmark::State& state)
{
    BENCHMARK_LOGD("DirectoryTest testTransformFileName001 start.");
    while (state.KeepRunning()) {
        string srcName = "test";
        string result = TransformFileName(srcName);
        string cmpName = srcName;
    #ifdef _WIN32
        cmpName = cmpName.append(".dll");
    #elif defined IOS_PLATFORM
        cmpName = cmpName.append(".dylib");
    #endif
        AssertEqual(result, cmpName, "result did not equal cmpName as expected.", state);

        srcName = srcName.append(".app");
        result = TransformFileName(srcName);
    #ifdef _WIN32
        cmpName = cmpName.append(".dll");
    #elif defined IOS_PLATFORM
        cmpName = cmpName.append(".dylib");
    #endif
        AssertEqual(result, cmpName, "result did not equal cmpName as expected.", state);
    }
    BENCHMARK_LOGD("DirectoryTest testTransformFileName001 end.");
}
#endif
}  // namespace
}  // namespace OHOS
// Run the benchmark
BENCHMARK_MAIN();