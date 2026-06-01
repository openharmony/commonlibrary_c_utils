/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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

#include "directory_ex.h"
#include "directory_ex_inner.h"
#include <gtest/gtest.h>
#include <fcntl.h>
#include <algorithm>
#include <cstdlib>
#include <cstdio>
#include <iostream>
#include <fstream>
#include <sstream>
#include <sys/mount.h>
#include <unistd.h>
#include <vector>
#include <climits>

using namespace testing::ext;
using namespace std;

namespace OHOS {
namespace {
constexpr uint64_t STAT_BLOCK_SIZE = 512;

class UtilsDirectoryTest : public testing::Test {
public :
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void UtilsDirectoryTest::SetUpTestCase(void)
{
}

void UtilsDirectoryTest::TearDownTestCase(void)
{
}

void UtilsDirectoryTest::SetUp(void)
{
}

void UtilsDirectoryTest::TearDown(void)
{
}

uint64_t GetDiskUsage(const string& path)
{
    struct stat statbuf = {0};
    if (lstat(path.c_str(), &statbuf) != 0 || statbuf.st_blocks <= 0) {
        return 0;
    }
    return static_cast<uint64_t>(statbuf.st_blocks) * STAT_BLOCK_SIZE;
}

uint64_t GetDiskUsage(const vector<string>& paths)
{
    uint64_t totalSize = 0;
    for (const auto& path : paths) {
        totalSize += GetDiskUsage(path);
    }
    return totalSize;
}

uint64_t GetDuSummarizeSize(const string& path)
{
    string command = "du -s '" + path + "' 2>/dev/null";
    FILE *pipe = popen(command.c_str(), "r");
    if (pipe == nullptr) {
        return 0;
    }

    char buffer[256] = {0};
    string output;
    if (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
        output = buffer;
    }
    pclose(pipe);

    istringstream stream(output);
    uint64_t size = 0;
    stream >> size;
    return size;
}
/*
 * @tc.name: testGetCurrentProcFullFileName001
 * @tc.desc: get the directory of directorytest
 */
HWTEST_F(UtilsDirectoryTest, testGetCurrentProcFullFileName001, TestSize.Level0)
{
    string strBaseName = "/data/test/UtilsDirectoryTest";
    string strFilename = GetCurrentProcFullFileName();
    EXPECT_EQ(strFilename, strBaseName);
}

/*
 * @tc.name: testGetCurrentProcPath001
 * @tc.desc: get the path of directorytest
 */
HWTEST_F(UtilsDirectoryTest, testGetCurrentProcPath001, TestSize.Level0)
{
    string strPathName = "/data/test/";
    string strCurPathName = GetCurrentProcPath();
    EXPECT_EQ(strCurPathName, strPathName);
}

/*
 * @tc.name: testExtractFilePath001
 * @tc.desc: get the filename of the path
 */
HWTEST_F(UtilsDirectoryTest, testExtractFilePath001, TestSize.Level0)
{
    string strFilePath = "/data/test/";
    string strPath = ExtractFilePath(GetCurrentProcFullFileName());
    EXPECT_EQ(strFilePath, strPath);
}

/*
 * @tc.name: testExtractFileName001
 * @tc.desc: get the filename of the path
 */
HWTEST_F(UtilsDirectoryTest, testExtractFileName001, TestSize.Level0)
{
    string strBaseName = "UtilsDirectoryTest";
    string strName = ExtractFileName(GetCurrentProcFullFileName());
    EXPECT_EQ(strBaseName, strName);
}

/*
 * @tc.name: testExtractFileExt001
 * @tc.desc: get the filename of the path
 */
HWTEST_F(UtilsDirectoryTest, testExtractFileExt001, TestSize.Level0)
{
    string strBaseName = "test/test.txt";
    string strTypeName = ExtractFileExt(strBaseName);
    EXPECT_EQ(strTypeName, "txt");
}

/*
 * @tc.name: testExtractFileExt002
 * @tc.desc: get the filename of the path and test whether the filename contains "."
 */
HWTEST_F(UtilsDirectoryTest, testExtractFileExt002, TestSize.Level0)
{
    string strBaseName = "test/test_txt";
    string strTypeName = ExtractFileExt(strBaseName);
    EXPECT_EQ(strTypeName, "");
}

/*
 * @tc.name: testExcludeTrailingPathDelimiter001
 * @tc.desc: directory unit test
 */
HWTEST_F(UtilsDirectoryTest, testExcludeTrailingPathDelimiter001, TestSize.Level0)
{
    string strResult = "data/test/UtilsDirectoryTest";
    string strName = ExcludeTrailingPathDelimiter("data/test/UtilsDirectoryTest/");
    EXPECT_EQ(strResult, strName);
}

/*
 * @tc.name: testIncludeTrailingPathDelimiter001
 * @tc.desc: directory unit test
 */
HWTEST_F(UtilsDirectoryTest, testIncludeTrailingPathDelimiter001, TestSize.Level0)
{
    string strResult = "data/test/UtilsDirectoryTest/";
    string strName = IncludeTrailingPathDelimiter("data/test/UtilsDirectoryTest");
    EXPECT_EQ(strResult, strName);
}

/*
 * @tc.name: testIncludeTrailingPathDelimiter002
 * @tc.desc: directory unit test
 */
HWTEST_F(UtilsDirectoryTest, testIncludeTrailingPathDelimiter002, TestSize.Level0)
{
    string strResult = "/";
    string strName = IncludeTrailingPathDelimiter("");
    EXPECT_EQ(strResult, strName);
}

/*
 * @tc.name: testGetDirFiles001
 * @tc.desc: test GetDirFiles works on multi-level directory
 */
HWTEST_F(UtilsDirectoryTest, testGetDirFiles001, TestSize.Level0)
{
    string parentPath = "/data/test_dir";

    ForceCreateDirectory(parentPath);

    string dirs[6] = {
        "/data/test_dir/level1_1",
        "/data/test_dir/level1_2",
        "/data/test_dir/level1_2/level2_1",
        "/data/test_dir/level1_2/level2_2",
        "/data/test_dir/level1_2/level2_2/level3_1",
        "/data/test_dir/level1_3",
    };

    string resultfiles[9] = {
        "/data/test_dir/level1_1/test_file",
        "/data/test_dir/level1_2/level2_2/level3_1/test_file_1",
        "/data/test_dir/level1_2/level2_2/level3_1/test_file_2",
        "/data/test_dir/level1_2/level2_2/test_file_1",
        "/data/test_dir/level1_2/level2_2/test_file_2",
        "/data/test_dir/level1_2/level2_2/test_file_3",
        "/data/test_dir/level1_2/level2_2/test_file_4",
        "/data/test_dir/level1_2/test_file",
        "/data/test_dir/level1_3/test_file",
    };

    for (auto &path : dirs) {
        ForceCreateDirectory(path);
    }

    for (auto &filepath : resultfiles) {
        ofstream(filepath, fstream::out);
    }

    vector<string> files;

    GetDirFiles(parentPath, files);

    for (auto &filepath : resultfiles) {
        auto pos = find(files.begin(), files.end(), filepath);
        EXPECT_NE(pos, files.end());
    }

    ForceRemoveDirectory(parentPath);
}

/*
 * @tc.name: testGetDirFiles002
* @tc.desc: test GetDirFiles works on deeply nested directory and handles very long path
 */
HWTEST_F(UtilsDirectoryTest, testGetDirFiles002, TestSize.Level0)
{
    string parentPath = "/data/test_dir/";
    string veryLongPath = "/data/test_dir/";

    int length = 10000;

    for (int i = 0; i < length; i++) {
        veryLongPath += "0";
        veryLongPath += "/";
    }

    EXPECT_EQ(mkdir("/data/test_dir", S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH), 0);
    chdir(parentPath.c_str());

    for (int i = 0; i < length; i++) {
        EXPECT_EQ(mkdir("./0", S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH), 0);
        EXPECT_EQ(chdir("./0"), 0);
    }

    ofstream file("./test_file");
    file.close();
    EXPECT_EQ(chdir("/data/test"), 0);

    auto files = vector<string>();

    GetDirFiles(parentPath, files);

    EXPECT_EQ(files.size(), 1);
    EXPECT_EQ((veryLongPath + "test_file").length(), files[0].length());
    EXPECT_EQ(veryLongPath + "test_file", files[0]);

    ForceRemoveDirectory(parentPath);
}

/*
 * @tc.name: testGetDirFiles003
 * @tc.desc: test GetDirFiles works on symlink
 */
HWTEST_F(UtilsDirectoryTest, testGetDirFiles003, TestSize.Level0)
{
    // create a test dir
    string originalDataPath = "/data/original";
    EXPECT_EQ(ForceCreateDirectory(originalDataPath), true);

    string originalFilePath = "/data/original/original_file";
    string originalDirectoryPath = "/data/original/original_directory";

    ofstream(originalFilePath, fstream::out);

    ForceCreateDirectory(originalDirectoryPath);

    string testDataDir = "/data/test_dir";

    EXPECT_EQ(ForceCreateDirectory(testDataDir), true);

    // test symlink to directory outside the target directory
    string linktodir = IncludeTrailingPathDelimiter(testDataDir) + "symlink_dir";

    EXPECT_EQ(symlink(originalDirectoryPath.c_str(), linktodir.c_str()), 0);

    vector<string> dirResult;
    GetDirFiles(testDataDir, dirResult);

    EXPECT_EQ(dirResult.size(), 1);
    EXPECT_EQ(dirResult[0], linktodir);

    EXPECT_EQ(ForceRemoveDirectory(linktodir), true);

    // test symlink to file outside the target directory
    string linktofile = IncludeTrailingPathDelimiter(testDataDir) + "symlink_file";
    EXPECT_EQ(symlink(originalFilePath.c_str(), linktofile.c_str()), 0);

    vector<string> fileResult;
    GetDirFiles(testDataDir, fileResult);
    EXPECT_EQ(fileResult.size(), 1);
    EXPECT_EQ(fileResult[0], linktofile);

    EXPECT_EQ(RemoveFile(linktofile), true);

    // test symlink of files in the same directory
    string sourceFile = IncludeTrailingPathDelimiter(testDataDir) + "source";
    string symlinkFile = IncludeTrailingPathDelimiter(testDataDir) + "symlink_file";

    ofstream(sourceFile, fstream::out);
    EXPECT_EQ(symlink(sourceFile.c_str(), symlinkFile.c_str()), 0);

    vector<string> internalFiles;
    GetDirFiles(testDataDir, internalFiles);

    EXPECT_NE(find(internalFiles.begin(), internalFiles.end(), sourceFile), internalFiles.end());
    EXPECT_NE(find(internalFiles.begin(), internalFiles.end(), symlinkFile), internalFiles.end());

    EXPECT_EQ(RemoveFile(sourceFile), true);
    EXPECT_EQ(RemoveFile(symlinkFile), true);

    ForceRemoveDirectory(originalDataPath);
    ForceRemoveDirectory(testDataDir);
}

/*
 * @tc.name: testForceCreateDirectory001
 * @tc.desc: directory unit test
 */
HWTEST_F(UtilsDirectoryTest, testForceCreateDirectory001, TestSize.Level0)
{
    string dirpath = "/data/test_dir/test2/test3";
    bool ret = ForceCreateDirectory(dirpath);
    EXPECT_EQ(ret, true);
    ret = IsEmptyFolder(dirpath);
    EXPECT_EQ(ret, true);
}

/*
 * @tc.name: testForceRemoveDirectory001
 * @tc.desc: directory unit test
 */
HWTEST_F(UtilsDirectoryTest, testForceRemoveDirectory001, TestSize.Level0)
{
    string dirpath = "/data/test_dir";
    bool ret = ForceRemoveDirectory(dirpath);
    EXPECT_EQ(ret, true);
}

/*
 * @tc.name: testForceRemoveDirectory002
 * @tc.desc: test whether the folder exists
 */
HWTEST_F(UtilsDirectoryTest, testForceRemoveDirectory002, TestSize.Level0)
{
    string dirpath = "/data/test/utils_directory_tmp/";
    bool ret = ForceRemoveDirectory(dirpath);
    EXPECT_EQ(ret, false);
}


/*
 * @tc.name: testForceRemoveDirectory003
 * @tc.desc: test whether it works when the full path is over than 255.
 */
HWTEST_F(UtilsDirectoryTest, testForceRemoveDirectory003, TestSize.Level0)
{
    string dirpath = "/data/test/tmp/tmp/tmp/tmp/tmp/tmp/tmp/tmp/tmp/tmp/tmp/tmp/tmp/tmp/"
        "tmp/tmp/tmp/tmp/tmp/tmp/tmp/tmp/tmp/tmp/tmp/tmp/tmp/tmp/tmp/tmp/tmp/tmp/tmp/tmp/tmp/"
        "tmp/tmp/tmp/tmp/tmp/tmp/tmp/tmp/tmp/tmp/tmp/tmp/tmp/tmp/tmp/tmp/tmp/tmp/tmp/tmp/tmp/"
        "tmp/tmp/tmp/tmp/tmp/tmp/tmp/tmp/tmp/tmp/tmp/tmp/tmp/tmp/tmp/tmp/tmp/tmp/tmp/tmp/tmp";
    bool ret = ForceCreateDirectory(dirpath);
    EXPECT_EQ(ret, true);
    string rootpath = "/data/test/tmp";
    ret = ForceRemoveDirectory(rootpath);
    EXPECT_EQ(ret, true);
}

/*
 * @tc.name: testRemoveFile001
 * @tc.desc: directory unit test
 */
HWTEST_F(UtilsDirectoryTest, testRemoveFile001, TestSize.Level0)
{
    string dirpath = "/data/test_dir";
    bool ret = ForceCreateDirectory(dirpath);
    EXPECT_EQ(ret, true);
    string filename = dirpath + "/test.txt";
    FILE *fp = fopen(filename.c_str(), "w");
    if (NULL != fp) {
        fclose(fp);
        ret = RemoveFile(filename);
        EXPECT_EQ(ret, true);
    }
    ret = ForceRemoveDirectory(dirpath);
    EXPECT_EQ(ret, true);
}

/*
 * @tc.name: testRemoveFile002
 * @tc.desc: Remove soft link file.
 */
HWTEST_F(UtilsDirectoryTest, testRemoveFile002, TestSize.Level0)
{
    string dirpath = "/data/test_dir";
    bool ret = ForceCreateDirectory(dirpath);
    EXPECT_EQ(ret, true);

    string targetname = "/data/test_target.txt";
    FILE *fp = fopen(targetname.c_str(), "w");
    if (NULL != fp) {
        fclose(fp);
    }

    // symlink to a directory
    string linkpath = "/data/test_symlink_dir";
    int res = symlink(dirpath.c_str(), linkpath.c_str());
    EXPECT_EQ(res, 0);

    ret = ForceRemoveDirectory(linkpath);
    EXPECT_EQ(ret, true);

    // Target dir is not removed.
    ret = faccessat(AT_FDCWD, dirpath.c_str(), F_OK, AT_SYMLINK_NOFOLLOW);
    EXPECT_EQ(ret, 0);

    // symlink to a file
    string filename = dirpath + "/test.txt";
    res = symlink(targetname.c_str(), filename.c_str());
    EXPECT_EQ(res, 0);

    ret = ForceRemoveDirectory(dirpath);
    EXPECT_EQ(ret, true);

    // Target file is not removed.
    ret = faccessat(AT_FDCWD, targetname.c_str(), F_OK, AT_SYMLINK_NOFOLLOW);
    EXPECT_EQ(ret, 0);

    ret = RemoveFile(targetname);
    EXPECT_EQ(ret, true);
}

/*
 * @tc.name: testRemoveFile003
 * @tc.desc: Remove dangling soft link file.
 */
HWTEST_F(UtilsDirectoryTest, testRemoveFile003, TestSize.Level0)
{
    string dirpath = "/data/test_dir";
    bool ret = ForceCreateDirectory(dirpath);
    EXPECT_EQ(ret, true);

    // symlink to a file
    string targetname = "/data/nonexisted.txt";
    string filename = dirpath + "/test.txt";
    int res = symlink(targetname.c_str(), filename.c_str());
    EXPECT_EQ(res, 0);

    ret = ForceRemoveDirectory(dirpath);
    EXPECT_EQ(ret, true);

    ret = RemoveFile(targetname);
    EXPECT_EQ(ret, true);
}

/*
 * @tc.name: testGetFolderSize001
 * @tc.desc: directory unit test
 */
HWTEST_F(UtilsDirectoryTest, testGetFolderSize001, TestSize.Level0)
{
    string dirpath = "/data/test_folder/";
    bool ret = ForceCreateDirectory(dirpath);
    EXPECT_EQ(ret, true);
    ofstream out(dirpath + "test.txt");
    if (out.is_open()) {
        out << "This is a line.\n";
        out << "This is another line.\n";
        out.close();
    }
    uint64_t resultsize = GetFolderSize(dirpath);
    uint64_t resultcomp = 38;
    EXPECT_EQ(resultsize, resultcomp);

    mode_t mode = S_IRWXU | S_IRWXG | S_IRWXO;
    ret = ChangeModeFile(dirpath + "test.txt", mode);
    EXPECT_EQ(ret, true);

    mode = S_IRUSR  | S_IRGRP | S_IROTH;
    ret = ChangeModeDirectory(dirpath, mode);
    EXPECT_EQ(ret, true);

    ret = ForceRemoveDirectory(dirpath);
    EXPECT_EQ(ret, true);
}

/*
 * @tc.name: testGetFolderDiskUsage001
 * @tc.desc: test actual disk usage with files and directories
 */
HWTEST_F(UtilsDirectoryTest, testGetFolderDiskUsage001, TestSize.Level0)
{
    string dirpath = "/data/test_folder_disk_usage/";
    string subdir = dirpath + "subdir";
    string file = dirpath + "test.txt";
    string sparseFile = subdir + "/sparse.txt";
    EXPECT_EQ(ForceCreateDirectory(subdir), true);

    ofstream out(file);
    if (out.is_open()) {
        out << "This is a line.\n";
        out << "This is another line.\n";
        out.close();
    }
    FILE *fp = fopen(sparseFile.c_str(), "w");
    ASSERT_NE(fp, nullptr);
    EXPECT_EQ(ftruncate(fileno(fp), 1024 * 1024), 0);
    EXPECT_EQ(fclose(fp), 0);

    uint64_t resultSize = GetFolderDiskUsage(dirpath);
    vector<string> expectedFiles = { ExcludeTrailingPathDelimiter(dirpath), subdir, file, sparseFile };
    uint64_t expectedSize = GetDiskUsage(expectedFiles);
    EXPECT_EQ(resultSize, expectedSize);
    EXPECT_EQ(GetFolderSize(dirpath), static_cast<uint64_t>(38 + 1024 * 1024));

    EXPECT_EQ(ForceRemoveDirectory(dirpath), true);
}

/*
 * @tc.name: testGetFolderDiskUsage002
 * @tc.desc: test symbolic link disk usage without following target
 */
HWTEST_F(UtilsDirectoryTest, testGetFolderDiskUsage002, TestSize.Level0)
{
    string dirpath = "/data/test_folder_disk_usage_symlink/";
    string targetDir = "/data/test_folder_disk_usage_target/";
    string targetFile = targetDir + "target.txt";
    string linkFile = dirpath + "link.txt";
    string rootLink = "/data/test_folder_disk_usage_root_link";
    EXPECT_EQ(ForceCreateDirectory(dirpath), true);
    EXPECT_EQ(ForceCreateDirectory(targetDir), true);

    FILE *fp = fopen(targetFile.c_str(), "w");
    ASSERT_NE(fp, nullptr);
    EXPECT_EQ(ftruncate(fileno(fp), 1024 * 1024), 0);
    EXPECT_EQ(fclose(fp), 0);
    EXPECT_EQ(symlink(targetFile.c_str(), linkFile.c_str()), 0);

    uint64_t expectedSize = GetDiskUsage(vector<string> { ExcludeTrailingPathDelimiter(dirpath), linkFile });
    EXPECT_EQ(GetFolderDiskUsage(dirpath), expectedSize);

    EXPECT_EQ(symlink(targetDir.c_str(), rootLink.c_str()), 0);
    EXPECT_EQ(GetFolderDiskUsage(rootLink), GetDiskUsage(rootLink));

    EXPECT_EQ(RemoveFile(rootLink), true);
    EXPECT_EQ(ForceRemoveDirectory(dirpath), true);
    EXPECT_EQ(ForceRemoveDirectory(targetDir), true);
}

/*
 * @tc.name: testGetFolderDiskUsage003
 * @tc.desc: test hard links are counted once
 */
HWTEST_F(UtilsDirectoryTest, testGetFolderDiskUsage003, TestSize.Level0)
{
    string dirpath = "/data/test_folder_disk_usage_hardlink/";
    string file = dirpath + "test.txt";
    string hardLink = dirpath + "hardlink.txt";
    EXPECT_EQ(ForceCreateDirectory(dirpath), true);

    ofstream out(file);
    if (out.is_open()) {
        out << "This is a line.\n";
        out.close();
    }
    ASSERT_EQ(link(file.c_str(), hardLink.c_str()), 0);

    uint64_t expectedSize = GetDiskUsage(vector<string> { ExcludeTrailingPathDelimiter(dirpath), file });
    EXPECT_EQ(GetFolderDiskUsage(dirpath), expectedSize);

    EXPECT_EQ(ForceRemoveDirectory(dirpath), true);
}

/*
 * @tc.name: testGetFolderDiskUsage004
 * @tc.desc: test invalid paths return zero and file paths return self usage
 */
HWTEST_F(UtilsDirectoryTest, testGetFolderDiskUsage004, TestSize.Level0)
{
    string dirpath = "/data/test_folder_disk_usage_invalid/";
    string file = dirpath + "test.txt";
    EXPECT_EQ(ForceCreateDirectory(dirpath), true);
    ofstream out(file);
    out.close();

    EXPECT_EQ(GetFolderDiskUsage(""), static_cast<uint64_t>(0));
    EXPECT_EQ(GetFolderDiskUsage("/data/test_folder_disk_usage_not_exist/"), static_cast<uint64_t>(0));
    EXPECT_EQ(GetFolderDiskUsage(file), GetDiskUsage(file));

    EXPECT_EQ(ForceRemoveDirectory(dirpath), true);
}

/*
 * @tc.name: testGetFolderDiskUsage008
 * @tc.desc: test GetFolderDiskUsage matches du for a regular file path
 */
HWTEST_F(UtilsDirectoryTest, testGetFolderDiskUsage008, TestSize.Level0)
{
    string dirpath = "/data/test_folder_disk_usage_regular_file/";
    string file = dirpath + "regular.txt";
    EXPECT_EQ(ForceCreateDirectory(dirpath), true);

    ofstream out(file);
    ASSERT_EQ(out.is_open(), true);
    out << "test content for regular file path\n";
    out.close();

    uint64_t apiResult = GetFolderDiskUsage(file);
    uint64_t expectedSize = GetDiskUsage(file);
    uint64_t du512Blocks = 0;
    string duCmd = "du -s -K " + file + " 2>/dev/null";
    FILE *fp = popen(duCmd.c_str(), "r");
    ASSERT_NE(fp, nullptr);
    char buf[128] = {0};
    if (fgets(buf, sizeof(buf), fp) != nullptr) {
        du512Blocks = strtoull(buf, nullptr, 10);
    }
    pclose(fp);

    EXPECT_EQ(apiResult, expectedSize);
    EXPECT_EQ(apiResult, du512Blocks * STAT_BLOCK_SIZE);

    EXPECT_EQ(ForceRemoveDirectory(dirpath), true);
}

/*
 * @tc.name: testGetFolderDiskUsage009
 * @tc.desc: test GetFolderDiskUsage matches du for deep directory tree with many files
 */
HWTEST_F(UtilsDirectoryTest, testGetFolderDiskUsage009, TestSize.Level0)
{
    string dirpath = "/data/test_folder_disk_usage_deep/";
    string currentDir = ExcludeTrailingPathDelimiter(dirpath);
    constexpr uint32_t depth = 8;
    constexpr uint32_t filesPerLevel = 16;
    EXPECT_EQ(ForceCreateDirectory(dirpath), true);

    for (uint32_t level = 0; level < depth; ++level) {
        currentDir += "/level_" + to_string(level);
        EXPECT_EQ(ForceCreateDirectory(currentDir), true);

        for (uint32_t fileIndex = 0; fileIndex < filesPerLevel; ++fileIndex) {
            string filePath = currentDir + "/file_" + to_string(fileIndex) + ".txt";
            ofstream out(filePath);
            ASSERT_EQ(out.is_open(), true);
            out << "deep directory disk usage test level " << level
                << " file " << fileIndex << '\n';
            out.close();
        }
    }

    uint64_t apiResult = GetFolderDiskUsage(dirpath);
    uint64_t du512Blocks = 0;
    string duCmd = "du -s -K " + dirpath + " 2>/dev/null";
    FILE *fp = popen(duCmd.c_str(), "r");
    ASSERT_NE(fp, nullptr);
    char buf[128] = {0};
    if (fgets(buf, sizeof(buf), fp) != nullptr) {
        du512Blocks = strtoull(buf, nullptr, 10);
    }
    pclose(fp);

    EXPECT_NE(apiResult, static_cast<uint64_t>(0));
    EXPECT_EQ(apiResult, du512Blocks * STAT_BLOCK_SIZE);
    EXPECT_EQ(ForceRemoveDirectory(dirpath), true);
}

/*
 * @tc.name: testGetFolderDiskUsage005
 * @tc.desc: test GetFolderDiskUsage matches toybox du default block semantics
 */
HWTEST_F(UtilsDirectoryTest, testGetFolderDiskUsage005, TestSize.Level0)
{
    string dirpath = "/data/test_folder_disk_usage_du_default/";
    string subdir = dirpath + "subdir";
    string regularFile = dirpath + "regular.txt";
    string sparseFile = subdir + "/sparse.bin";
    string hardLink = dirpath + "regular_hardlink.txt";
    string symlinkFile = dirpath + "sparse_symlink.bin";
    EXPECT_EQ(ForceCreateDirectory(subdir), true);

    ofstream regularOut(regularFile);
    ASSERT_EQ(regularOut.is_open(), true);
    regularOut << "du default semantics test\n";
    regularOut.close();

    FILE *sparseFp = fopen(sparseFile.c_str(), "w");
    ASSERT_NE(sparseFp, nullptr);
    EXPECT_EQ(ftruncate(fileno(sparseFp), 1024 * 1024), 0);
    EXPECT_EQ(fclose(sparseFp), 0);

    ASSERT_EQ(link(regularFile.c_str(), hardLink.c_str()), 0);
    ASSERT_EQ(symlink(sparseFile.c_str(), symlinkFile.c_str()), 0);

    uint64_t resultSize = GetFolderDiskUsage(dirpath);
    uint64_t duSize = GetDuSummarizeSize(ExcludeTrailingPathDelimiter(dirpath));
    constexpr uint64_t duBlockSize = 1024;
    uint64_t expectedDuBlocks = (resultSize + duBlockSize - 1) / duBlockSize;
    EXPECT_EQ(expectedDuBlocks, duSize);

    EXPECT_EQ(ForceRemoveDirectory(dirpath), true);
}

/*
 * @tc.name: testGetFolderDiskUsage006
 * @tc.desc: test GetFolderDiskUsage matches du for bind-mounted directories
 */
HWTEST_F(UtilsDirectoryTest, testGetFolderDiskUsage006, TestSize.Level0)
{
    string testRoot = "/data/test_folder_disk_usage_bindmount/";
    string originalDir = testRoot + "original";
    string mirrorDir = testRoot + "mirror";
    string file1 = originalDir + "/file1";
    EXPECT_EQ(ForceCreateDirectory(originalDir), true);
    EXPECT_EQ(ForceCreateDirectory(mirrorDir), true);

    ofstream out(file1);
    ASSERT_EQ(out.is_open(), true);
    out << "test content for bind mount\n";
    out.close();

    int mountRet = mount(originalDir.c_str(), mirrorDir.c_str(), nullptr, MS_BIND, nullptr);
    if (mountRet != 0) {
        ForceRemoveDirectory(testRoot);
        GTEST_SKIP() << "Cannot set up bind mount (requires root): " << strerror(errno);
        return;
    }

    uint64_t apiResult = GetFolderDiskUsage(testRoot);
    uint64_t du512Blocks = 0;
    string duCmd = "du -s -K " + testRoot + " 2>/dev/null";
    FILE *fp = popen(duCmd.c_str(), "r");
    ASSERT_NE(fp, nullptr);
    char buf[128] = {0};
    if (fgets(buf, sizeof(buf), fp) != nullptr) {
        du512Blocks = strtoull(buf, nullptr, 10);
    }
    pclose(fp);

    uint64_t bindExpected = GetDiskUsage(vector<string> {
        ExcludeTrailingPathDelimiter(testRoot),
        ExcludeTrailingPathDelimiter(originalDir),
        ExcludeTrailingPathDelimiter(mirrorDir),
        file1,
        mirrorDir + "/file1",
    });

    EXPECT_EQ(apiResult, bindExpected);
    EXPECT_EQ(apiResult, du512Blocks * STAT_BLOCK_SIZE);

    EXPECT_EQ(umount(mirrorDir.c_str()), 0);
    if (!ForceRemoveDirectory(testRoot)) {
        string cleanupCmd = "rm -rf " + testRoot;
        system(cleanupCmd.c_str());
    }
    EXPECT_EQ(access(testRoot.c_str(), F_OK), -1);
}

/*
 * @tc.name: testGetFolderDiskUsage007
 * @tc.desc: test GetFolderDiskUsage does not follow a root directory symlink
 */
HWTEST_F(UtilsDirectoryTest, testGetFolderDiskUsage007, TestSize.Level0)
{
    string targetDir = "/data/test_folder_disk_usage_symlink_root_target/";
    string targetFile = targetDir + "file1";
    string rootLink = "/data/test_folder_disk_usage_symlink_root_link";
    EXPECT_EQ(ForceCreateDirectory(targetDir), true);

    ofstream out(targetFile);
    ASSERT_EQ(out.is_open(), true);
    out << "test content for symlink root\n";
    out.close();

    ASSERT_EQ(symlink(targetDir.c_str(), rootLink.c_str()), 0);

    uint64_t apiResult = GetFolderDiskUsage(rootLink);
    uint64_t expectedSize = GetDiskUsage(rootLink);
    uint64_t du512Blocks = 0;
    string duCmd = "du -s -K " + rootLink + " 2>/dev/null";
    FILE *fp = popen(duCmd.c_str(), "r");
    ASSERT_NE(fp, nullptr);
    char buf[128] = {0};
    if (fgets(buf, sizeof(buf), fp) != nullptr) {
        du512Blocks = strtoull(buf, nullptr, 10);
    }
    pclose(fp);

    EXPECT_EQ(apiResult, expectedSize);
    EXPECT_EQ(apiResult, du512Blocks * STAT_BLOCK_SIZE);

    EXPECT_EQ(RemoveFile(rootLink), true);
    EXPECT_EQ(ForceRemoveDirectory(targetDir), true);
}

/*
 * @tc.name: testChangeModeFile001
 * @tc.desc: test whether the folder exists
 */
HWTEST_F(UtilsDirectoryTest, testChangeModeFile001, TestSize.Level0)
{
    string dirpath = "/data/test/utils_directory_tmp/";
    mode_t mode = S_IRWXU | S_IRWXG | S_IRWXO;
    bool ret = ChangeModeFile(dirpath + "test.txt", mode);
    EXPECT_EQ(ret, false);
}

/*
 * @tc.name: testChangeModeDirectory001
 * @tc.desc: test whether the folder is empty and get the size of the folder
 */
HWTEST_F(UtilsDirectoryTest, testChangeModeDirectory001, TestSize.Level0)
{
    string dirpath = "";
    mode_t mode = S_IRUSR  | S_IRGRP | S_IROTH;
    bool ret = ChangeModeDirectory(dirpath, mode);
    EXPECT_EQ(ret, false);

    uint64_t resultsize = GetFolderSize(dirpath);
    uint64_t resultcomp = 0;
    EXPECT_EQ(resultsize, resultcomp);
}

/*
 * @tc.name: testPathToRealPath001
 * @tc.desc: directory unit test
 */
HWTEST_F(UtilsDirectoryTest, testPathToRealPath001, TestSize.Level0)
{
    string path = "/data/test";
    string realpath;
    bool ret = PathToRealPath(path, realpath);
    EXPECT_EQ(ret, true);
    EXPECT_EQ(path, realpath);
}

/*
 * @tc.name: testPathToRealPath002
 * @tc.desc: directory unit test
 */
HWTEST_F(UtilsDirectoryTest, testPathToRealPath002, TestSize.Level0)
{
    string path = "/data/../data/test";
    string realpath;
    bool ret = PathToRealPath(path, realpath);
    EXPECT_EQ(ret, true);
    EXPECT_EQ("/data/test", realpath);
}

/*
 * @tc.name: testPathToRealPath003
 * @tc.desc: directory unit test
 */
HWTEST_F(UtilsDirectoryTest, testPathToRealPath003, TestSize.Level0)
{
    string path = "./";
    string realpath;
    bool ret = PathToRealPath(path, realpath);
    EXPECT_EQ(ret, true);
    EXPECT_EQ("/data/test", realpath);
}

/*
 * @tc.name: testPathToRealPath004
 * @tc.desc: directory unit test
 */
HWTEST_F(UtilsDirectoryTest, testPathToRealPath004, TestSize.Level0)
{
    string path = "";
    string realpath;
    bool ret = PathToRealPath(path, realpath);
    EXPECT_EQ(ret, false);
}

/*
 * @tc.name: testPathToRealPath005
 * @tc.desc: directory unit test
 */
HWTEST_F(UtilsDirectoryTest, testPathToRealPath005, TestSize.Level0)
{
    string path = "/data/test/data/test/data/test/data/test/data/test/data/ \
        test/data/test/data/test/data/test/data/test/data/test/data/test/data/ \
        test/data/test/data/test/data/test/data/test/data/test/data/test/data/ \
        test/data/test/data/test/data/test/data/test/data/test/data/test/data/ \
        test/data/test/data/test/data/test";
    string realpath;
    bool ret = PathToRealPath(path, realpath);
    EXPECT_EQ(ret, false);
}

/*
 * @tc.name: testPathToRealPath006
 * @tc.desc: test whether the folder exists
 */
HWTEST_F(UtilsDirectoryTest, testPathToRealPath006, TestSize.Level0)
{
    string path(PATH_MAX, 'x');
    string realpath;
    bool ret = PathToRealPath(path, realpath);
    EXPECT_EQ(ret, false);
}

/*
 * @tc.name: testTransformFileName001
 * @tc.desc: test transform the file name
 */
#if defined(IOS_PLATFORM) || defined(_WIN32)
HWTEST_F(UtilsDirectoryTest, testTransformFileName001, TestSize.Level0)
{
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
    EXPECT_EQ(result, cmpName);
}
#endif
}  // namespace
}  // namespace OHOS
