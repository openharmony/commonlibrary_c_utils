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

#include <gtest/gtest.h>
#include <algorithm>
#include <iostream>
#include <fstream>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "file_ex.h"

using namespace testing::ext;
using namespace std;

namespace OHOS {
namespace {
class UtilsFileTest : public testing::Test {
public:
    static constexpr char CONTENT_STR[] = "TTtt@#$%^&*()_+~`";
    static constexpr char FILE_PATH[] = "./tmp.txt";
    static constexpr char NULL_STR[] = "";
    static constexpr int MAX_FILE_LENGTH = 32 * 1024 * 1024;
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void UtilsFileTest::SetUpTestCase(void)
{
}

void UtilsFileTest::TearDownTestCase(void)
{
}

void UtilsFileTest::SetUp(void)
{
}

void UtilsFileTest::TearDown(void)
{
}

bool CreateTestFile(const std::string& path, const std::string& content)
{
    ofstream out(path, ios_base::out | ios_base::trunc);
    if (out.is_open()) {
        out << content;
        return true;
    }

    std::cout << "open file failed!" << path << std::endl;
    return false;
}

int RemoveTestFile(const std::string& path)
{
    return unlink(path.c_str());
}

/*
 * @tc.name: testLoadStringFromFile001
 * @tc.desc: Test loading an existed file 'meminfo'
 */
HWTEST_F(UtilsFileTest, testLoadStringFromFile001, TestSize.Level0)
{
    string str;
    string filename = "/proc/meminfo";
    EXPECT_TRUE(LoadStringFromFile(filename, str));

    string str2;
    int fd = open(filename.c_str(), O_RDONLY);
    EXPECT_TRUE(LoadStringFromFd(fd, str2));
    close(fd);
    EXPECT_EQ(str.size(), str2.size());

    vector<char> buff;
    bool ret = LoadBufferFromFile(filename, buff);
    EXPECT_TRUE(ret);
    EXPECT_EQ(str2.size(), buff.size());
}

/*
 * @tc.name: testLoadStringFromFile002
 * @tc.desc: Test loading a non-existed file
 */
HWTEST_F(UtilsFileTest, testLoadStringFromFile002, TestSize.Level0)
{
    string str;
    string filename = NULL_STR;
    EXPECT_FALSE(LoadStringFromFile(filename, str));
    EXPECT_TRUE(str.empty());
}

/*
 * @tc.name: testLoadStringFromFile003
 * @tc.desc: Test loading a newly created file with null contents
 */
HWTEST_F(UtilsFileTest, testLoadStringFromFile003, TestSize.Level0)
{
    string str;
    string filename = FILE_PATH;
    string content = NULL_STR;
    CreateTestFile(filename, content);
    EXPECT_TRUE(LoadStringFromFile(filename, str));
    RemoveTestFile(filename);
    EXPECT_TRUE(str == content);
}

/*
 * @tc.name: testLoadStringFromFile004
 * @tc.desc: Test loading a newly created file with contents
 */
HWTEST_F(UtilsFileTest, testLoadStringFromFile004, TestSize.Level0)
{
    string str;
    string filename = FILE_PATH;
    string content = CONTENT_STR;
    CreateTestFile(filename, content);
    EXPECT_TRUE(LoadStringFromFile(filename, str));
    RemoveTestFile(filename);
    EXPECT_TRUE(str == content);
}

/*
 * @tc.name: testLoadStringFromFile005
 * @tc.desc: Test loading a newly created file, whose contents are of maximum length
 */
HWTEST_F(UtilsFileTest, testLoadStringFromFile005, TestSize.Level1)
{
    string str;
    string filename = FILE_PATH;
    string content(MAX_FILE_LENGTH, 't');
    CreateTestFile(filename, content);
    EXPECT_TRUE(LoadStringFromFile(filename, str));
    RemoveTestFile(filename);
    EXPECT_TRUE(str == content);
}

/*
 * @tc.name: testLoadStringFromFile006
 * @tc.desc: Test loading a newly created file, whose contents exceeds maximum length
 */
HWTEST_F(UtilsFileTest, testLoadStringFromFile006, TestSize.Level1)
{
    string str;
    string filename = FILE_PATH;
    string content(MAX_FILE_LENGTH + 1, 't');
    CreateTestFile(filename, content);
    EXPECT_FALSE(LoadStringFromFile(filename, str));
    RemoveTestFile(filename);
    EXPECT_TRUE(str.empty());
}

/*
 * @tc.name: testLoadStringFromFd001
 * @tc.desc: Test loading a file by a invalid fd -1
 */
HWTEST_F(UtilsFileTest, testLoadStringFromFd001, TestSize.Level0)
{
    string result;
    EXPECT_FALSE(LoadStringFromFd(-1, result));
    EXPECT_EQ(result, "");
}

/*
 * @tc.name: testLoadStringFromFd002
 * @tc.desc: Test loading a newly created file without contents by its fd
 */
HWTEST_F(UtilsFileTest, testLoadStringFromFd002, TestSize.Level0)
{
    string result;
    string filename = FILE_PATH;
    string content = NULL_STR;
    CreateTestFile(filename, content);
    int fd = open(filename.c_str(), O_RDONLY);
    EXPECT_TRUE(LoadStringFromFd(fd, result));
    close(fd);
    RemoveTestFile(filename);
    EXPECT_TRUE(result == content);
}

/*
 * @tc.name: testLoadStringFromFd003
 * @tc.desc: Test loading a newly created file with contents by its fd
 */
HWTEST_F(UtilsFileTest, testLoadStringFromFd003, TestSize.Level0)
{
    string result;
    string filename = FILE_PATH;
    string content = CONTENT_STR;
    CreateTestFile(filename, content);
    int fd = open(filename.c_str(), O_RDONLY);
    EXPECT_TRUE(LoadStringFromFd(fd, result));
    close(fd);
    RemoveTestFile(filename);
    EXPECT_TRUE(result == content);
}

/*
 * @tc.name: testLoadStringFromFd004
 * @tc.desc: Test loading a newly created file by fd, whose contents are of maximum length
 */
HWTEST_F(UtilsFileTest, testLoadStringFromFd004, TestSize.Level1)
{
    string result;
    string filename = FILE_PATH;
    string content(MAX_FILE_LENGTH, 't');
    CreateTestFile(filename, content);
    int fd = open(filename.c_str(), O_RDONLY);
    EXPECT_TRUE(LoadStringFromFd(fd, result));
    close(fd);
    RemoveTestFile(filename);
    EXPECT_TRUE(result == content);
}

/*
 * @tc.name: testLoadStringFromFd005
 * @tc.desc: Test loading a newly created file by fd, whose contents exceeds maximum length
 */
HWTEST_F(UtilsFileTest, testLoadStringFromFd005, TestSize.Level1)
{
    string result;
    string filename = FILE_PATH;
    string content(MAX_FILE_LENGTH + 1, 't');
    CreateTestFile(filename, content);
    int fd = open(filename.c_str(), O_RDONLY);
    EXPECT_FALSE(LoadStringFromFd(fd, result));
    close(fd);
    RemoveTestFile(filename);
    EXPECT_NE(result, content);
}

/*
 * @tc.name: testLoadStringFromFd006
 * @tc.desc: Test loading a newly created file by fd, which is closed ahead of loading.
 */
HWTEST_F(UtilsFileTest, testLoadStringFromFd006, TestSize.Level0)
{
    string result;
    string filename = FILE_PATH;
    string content = CONTENT_STR;
    CreateTestFile(filename, content);
    int fd = open(filename.c_str(), O_RDONLY);
    close(fd);
    EXPECT_FALSE(LoadStringFromFd(fd, result));
    RemoveTestFile(filename);
    EXPECT_EQ(result, "");
}

/*
 * @tc.name: testSaveStringToFile001
 * @tc.desc: singleton template
 */
HWTEST_F(UtilsFileTest, testSaveStringToFile001, TestSize.Level0)
{
    string path = FILE_PATH;
    string content = CONTENT_STR;
    string newContent;
    CreateTestFile(path, content);
    bool ret = SaveStringToFile(path, newContent);
    EXPECT_EQ(ret, true);

    string loadResult;
    EXPECT_TRUE(LoadStringFromFile(path, loadResult));
    RemoveTestFile(path);
    EXPECT_EQ(loadResult, content);
}

/*
 * @tc.name: testSaveStringToFile002
 * @tc.desc: singleton template
 */
HWTEST_F(UtilsFileTest, testSaveStringToFile002, TestSize.Level0)
{
    string path = FILE_PATH;
    string content = "Before truncated！";
    CreateTestFile(path, content);

    string newContent = CONTENT_STR;
    EXPECT_TRUE(SaveStringToFile(path, newContent));

    string loadResult;
    EXPECT_TRUE(LoadStringFromFile(path, loadResult));
    RemoveTestFile(path);
    EXPECT_EQ(loadResult, newContent);
}

/*
 * @tc.name: testSaveStringToFile003
 * @tc.desc: Test writting an empty string to a file in truncate mode
 */
HWTEST_F(UtilsFileTest, testSaveStringToFile003, TestSize.Level0)
{
    string path = FILE_PATH;
    string content = "Before truncated！";
    CreateTestFile(path, content);

    string newContent;
    bool ret = SaveStringToFile(path, newContent, true);
    EXPECT_EQ(ret, true);

    string loadResult;
    ret = LoadStringFromFile(path, loadResult);
    RemoveTestFile(path);
    EXPECT_EQ(ret, true);
    EXPECT_STREQ(loadResult.c_str(), content.c_str());
}

/*
 * @tc.name: testSaveStringToFile004
 * @tc.desc: Test writting an empty string to a file in append mode
 */
HWTEST_F(UtilsFileTest, testSaveStringToFile004, TestSize.Level0)
{
    string newContent;
    string path = FILE_PATH;
    string content = "Before appended！";
    CreateTestFile(path, content);
    bool ret = SaveStringToFile(path, newContent, false);
    EXPECT_EQ(ret, true);

    string loadResult;
    ret = LoadStringFromFile(path, loadResult);
    RemoveTestFile(path);
    EXPECT_EQ(ret, true);
    EXPECT_STREQ(loadResult.c_str(), content.c_str());
}

/*
 * @tc.name: testSaveStringToFile005
 * @tc.desc: Test writting a string to a file in append mode
 */
HWTEST_F(UtilsFileTest, testSaveStringToFile005, TestSize.Level0)
{
    string path = FILE_PATH;
    string content = "Before appended！";
    CreateTestFile(path, content);

    string newContent = CONTENT_STR;
    bool ret = SaveStringToFile(path, newContent, false);
    EXPECT_EQ(ret, true);

    string loadResult;
    ret = LoadStringFromFile(path, loadResult);
    RemoveTestFile(path);
    EXPECT_EQ(ret, true);
    EXPECT_EQ(loadResult, content + newContent);
}

/*
 * @tc.name: testSaveStringToFd001
 * @tc.desc: Test writting an empty string to files with invalid fds
 */
HWTEST_F(UtilsFileTest, testSaveStringToFd001, TestSize.Level0)
{
    string content;
    bool ret = SaveStringToFd(0, content);
    EXPECT_EQ(ret, false);
    ret = SaveStringToFd(-1, content);
    EXPECT_EQ(ret, false);

    content = CONTENT_STR;
    ret = SaveStringToFd(0, content);
    EXPECT_EQ(ret, false);
    ret = SaveStringToFd(-1, content);
    EXPECT_EQ(ret, false);
}

/*
 * @tc.name: testSaveStringToFd002
 * @tc.desc: Test writting an empty string to a file specified by its fd
 */
HWTEST_F(UtilsFileTest, testSaveStringToFd002, TestSize.Level0)
{
    string content;
    string filename = FILE_PATH;
    int fd = open(filename.c_str(), O_RDWR | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
    bool ret = SaveStringToFd(fd, content);
    close(fd);
    EXPECT_EQ(ret, true);

    string loadResult;
    fd = open(filename.c_str(), O_RDONLY);
    ret = LoadStringFromFd(fd, loadResult);
    close(fd);
    RemoveTestFile(filename);
    EXPECT_EQ(ret, true);
    EXPECT_EQ(loadResult, "");
}

/*
 * @tc.name: testSaveStringToFd003
 * @tc.desc: Test loading a non-empty string to a file specified by its fd
 */
HWTEST_F(UtilsFileTest, testSaveStringToFd003, TestSize.Level0)
{
    string content = CONTENT_STR;
    string filename = FILE_PATH;
    int fd = open(filename.c_str(), O_RDWR | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
    bool ret = SaveStringToFd(fd, content);
    close(fd);
    EXPECT_EQ(ret, true);

    string loadResult;
    fd = open(filename.c_str(), O_RDONLY);
    ret = LoadStringFromFd(fd, loadResult);
    close(fd);
    RemoveTestFile(filename);
    EXPECT_EQ(ret, true);
    EXPECT_EQ(loadResult, content);
}

/*
 * @tc.name: testSaveStringToFd004
 * @tc.desc: Test loading a non-empty string to a file without write-authority specified by its fd
 */
HWTEST_F(UtilsFileTest, testSaveStringToFd004, TestSize.Level0)
{
    string content = CONTENT_STR;
    string filename = FILE_PATH;
    int fd = open(filename.c_str(), O_RDONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
    bool ret = SaveStringToFd(fd, content);
    close(fd);
    EXPECT_EQ(ret, false);

    string loadResult;
    fd = open(filename.c_str(), O_RDONLY);
    ret = LoadStringFromFd(fd, loadResult);
    close(fd);
    RemoveTestFile(filename);
    EXPECT_EQ(ret, true);
    EXPECT_EQ(loadResult, "");
}

/*
 * @tc.name: testLoadBufferFromFile001
 * @tc.desc: singleton template
 */
HWTEST_F(UtilsFileTest, testLoadBufferFromFile001, TestSize.Level0)
{
    vector<char> buff;
    string filename = "";
    bool ret = LoadBufferFromFile(filename, buff);
    EXPECT_FALSE(ret);
    EXPECT_EQ(0, static_cast<int>(buff.size()));
}

/*
 * @tc.name: testLoadBufferFromFile002
 * @tc.desc: singleton template
 */
HWTEST_F(UtilsFileTest, testLoadBufferFromFile002, TestSize.Level0)
{
    vector<char> buff;
    string filename = FILE_PATH;
    string content;
    CreateTestFile(filename, content);
    bool ret = LoadBufferFromFile(filename, buff);
    RemoveTestFile(filename);
    EXPECT_TRUE(ret);
    EXPECT_EQ(0, static_cast<int>(buff.size()));
}

/*
 * @tc.name: testLoadBufferFromFile003
 * @tc.desc: singleton template
 */
HWTEST_F(UtilsFileTest, testLoadBufferFromFile003, TestSize.Level0)
{
    vector<char> buff;
    string filename = FILE_PATH;
    string content = "TXB";
    CreateTestFile(filename, content);
    bool ret = LoadBufferFromFile(filename, buff);
    RemoveTestFile(filename);
    EXPECT_TRUE(ret);
    EXPECT_EQ(3, (int)buff.size());
    EXPECT_EQ('T', buff[0]);
    EXPECT_EQ('X', buff[1]);
    EXPECT_EQ('B', buff[2]);
}

/*
 * @tc.name: testLoadBufferFromFile004
 * @tc.desc: singleton template
 */
HWTEST_F(UtilsFileTest, testLoadBufferFromFile004, TestSize.Level1)
{
    vector<char> buff;
    string filename = FILE_PATH;
    string content(32 * 1024 * 1024 + 1, 't');
    CreateTestFile(filename, content);
    bool ret = LoadBufferFromFile(filename, buff);
    RemoveTestFile(filename);
    EXPECT_EQ(ret, false);
    EXPECT_EQ(0, static_cast<int>(buff.size()));
}

/*
 * @tc.name: testSaveBufferToFile001
 * @tc.desc: singleton template
 */
HWTEST_F(UtilsFileTest, testSaveBufferToFile001, TestSize.Level0)
{
    vector<char> buff;
    string path = FILE_PATH;
    string content = "ttxx";
    CreateTestFile(path, content);
    bool ret = SaveBufferToFile(path, buff, false);
    EXPECT_EQ(ret, true);

    string loadResult;
    ret = LoadStringFromFile(path, loadResult);
    RemoveTestFile(path);
    EXPECT_EQ(ret, true);
    EXPECT_EQ(loadResult, content);
}

/*
 * @tc.name: testSaveBufferToFile002
 * @tc.desc: singleton template
 */
HWTEST_F(UtilsFileTest, testSaveBufferToFile002, TestSize.Level0)
{
    string path = FILE_PATH;
    string content = "ttxx";
    CreateTestFile(path, content);

    vector<char> newContent = {'x', 'x', 't', 't'};
    bool ret = SaveBufferToFile(path, newContent);
    EXPECT_EQ(ret, true);

    string loadResult;
    ret = LoadStringFromFile(path, loadResult);
    RemoveTestFile(path);
    EXPECT_EQ(ret, true);
    EXPECT_EQ(loadResult, std::string(newContent.begin(), newContent.end()));
}

/*
 * @tc.name: testSaveBufferToFile003
 * @tc.desc: singleton template
 */
HWTEST_F(UtilsFileTest, testSaveBufferToFile003, TestSize.Level0)
{
    string path = FILE_PATH;
    string content = "ttxx";
    CreateTestFile(path, content);

    vector<char> newContent = {'x', 'x', 't', 't'};
    bool ret = SaveBufferToFile(path, newContent, false);
    EXPECT_EQ(ret, true);

    string loadResult;
    ret = LoadStringFromFile(path, loadResult);
    RemoveTestFile(path);
    EXPECT_EQ(ret, true);
    EXPECT_EQ(loadResult, content + std::string(newContent.begin(), newContent.end()));
}

/*
 * @tc.name: testStringExistsInFile001
 * @tc.desc: singleton template
 */
HWTEST_F(UtilsFileTest, testStringExistsInFile001, TestSize.Level0)
{
    string str = "abc";
    string filename = "";
    EXPECT_FALSE(StringExistsInFile(filename, str, true));
    EXPECT_FALSE(str.empty());
}

/*
 * @tc.name: testStringExistsInFile002
 * @tc.desc: singleton template
 */
HWTEST_F(UtilsFileTest, testStringExistsInFile002, TestSize.Level0)
{
    string str = NULL_STR;
    string filename = FILE_PATH;
    string content = "hello world!";
    CreateTestFile(filename, content);
    EXPECT_FALSE(StringExistsInFile(filename, str, true));
    RemoveTestFile(filename);
}

/*
 * @tc.name: testStringExistsInFile003
 * @tc.desc: singleton template
 */
HWTEST_F(UtilsFileTest, testStringExistsInFile003, TestSize.Level0)
{
    string str = "world";
    string filename = FILE_PATH;
    string content = "hello world!";
    CreateTestFile(filename, content);
    EXPECT_TRUE(StringExistsInFile(filename, str, true));
    RemoveTestFile(filename);
}

/*
 * @tc.name: testStringExistsInFile004
 * @tc.desc: singleton template
 */
HWTEST_F(UtilsFileTest, testStringExistsInFile004, TestSize.Level1)
{
    string str1(32 * 1024 * 1024 + 1, 't');
    string str2(32 * 1024 * 1024, 't');
    string filename = FILE_PATH;
    string content(32 * 1024 * 1024, 't');
    CreateTestFile(filename, content);
    EXPECT_FALSE(StringExistsInFile(filename, str1, true));
    EXPECT_TRUE(StringExistsInFile(filename, str2, true));
    RemoveTestFile(filename);
}

/*
 * @tc.name: testStringExistsInFile005
 * @tc.desc: singleton template
 */
HWTEST_F(UtilsFileTest, testStringExistsInFile005, TestSize.Level0)
{
    string str = "woRld";
    string filename = FILE_PATH;
    string content = "hello world!";
    CreateTestFile(filename, content);
    EXPECT_TRUE(StringExistsInFile(filename, str, false));
    EXPECT_FALSE(StringExistsInFile(filename, str, true));
    RemoveTestFile(filename);
}

/*
 * @tc.name: testStringExistsInFile006
 * @tc.desc: singleton template
 */
HWTEST_F(UtilsFileTest, testStringExistsInFile006, TestSize.Level0)
{
    string str1 = "woRld!";
    string str2 = "123";
    string str3 = "llo ";
    string str4 = "123 w";
    string str5 = "hi";
    string filename = FILE_PATH;
    string content = "Test, hello 123 World!";
    CreateTestFile(filename, content);
    EXPECT_TRUE(StringExistsInFile(filename, str1, false));
    EXPECT_FALSE(StringExistsInFile(filename, str1, true));

    EXPECT_TRUE(StringExistsInFile(filename, str2, false));
    EXPECT_TRUE(StringExistsInFile(filename, str2, true));

    EXPECT_TRUE(StringExistsInFile(filename, str3, false));
    EXPECT_TRUE(StringExistsInFile(filename, str3, true));

    EXPECT_TRUE(StringExistsInFile(filename, str4, false));
    EXPECT_FALSE(StringExistsInFile(filename, str4, true));

    EXPECT_FALSE(StringExistsInFile(filename, str5, false));
    EXPECT_FALSE(StringExistsInFile(filename, str5, true));
    RemoveTestFile(filename);
}

/*
 * @tc.name: testStringExistsInFile007
 * @tc.desc: singleton template
 */
HWTEST_F(UtilsFileTest, testStringExistsInFile007, TestSize.Level0)
{
    string str1 = "is";
    string str2 = "\n\ris";
    string filename = FILE_PATH;
    string content = "Test, special string\n\ris ok";
    CreateTestFile(filename, content);
    EXPECT_TRUE(StringExistsInFile(filename, str1, false));
    EXPECT_TRUE(StringExistsInFile(filename, str1, true));

    EXPECT_TRUE(StringExistsInFile(filename, str2, false));
    EXPECT_TRUE(StringExistsInFile(filename, str2, true));
    RemoveTestFile(filename);
}

/*
 * @tc.name: testFileExist001
 * @tc.desc: singleton template
 */
HWTEST_F(UtilsFileTest, testFileExist001, TestSize.Level0)
{
    string filepath = "/proc/meminfo";
    string filepath1 = "/proc/meminfo1";

    EXPECT_TRUE(FileExists(filepath));
    EXPECT_FALSE(FileExists(filepath1));
}

/*
 * @tc.name: testCountStrInFile001
 * @tc.desc: singleton template
 */
HWTEST_F(UtilsFileTest, testCountStrInFile001, TestSize.Level0)
{
    string str = "abc";
    string filename = "";
    EXPECT_EQ(CountStrInFile(filename, str, true), -1);
    EXPECT_FALSE(str.empty());
}

/*
 * @tc.name: testCountStrInFile002
 * @tc.desc: singleton template
 */
HWTEST_F(UtilsFileTest, testCountStrInFile002, TestSize.Level0)
{
    string str = NULL_STR;
    string filename = FILE_PATH;
    string content = "hello world!";
    CreateTestFile(filename, content);
    EXPECT_EQ(CountStrInFile(filename, str, true), -1);
    RemoveTestFile(filename);
}

/*
 * @tc.name: testCountStrInFile003
 * @tc.desc: singleton template
 */
HWTEST_F(UtilsFileTest, testCountStrInFile003, TestSize.Level1)
{
    string str1(32 * 1024 * 1024 + 1, 't');
    string str2(32 * 1024 * 1024, 't');
    string filename = FILE_PATH;
    string content(32 * 1024 * 1024, 't');
    CreateTestFile(filename, content);
    EXPECT_EQ(CountStrInFile(filename, str1, true), 0);
    EXPECT_EQ(CountStrInFile(filename, str2, true), 1);
    RemoveTestFile(filename);
}

/*
 * @tc.name: testCountStrInFile004
 * @tc.desc: singleton template
 */
HWTEST_F(UtilsFileTest, testCountStrInFile004, TestSize.Level0)
{
    string str1 = "very";
    string str2 = "VERY";
    string str3 = "abc";
    string filename = FILE_PATH;
    string content = "This is very very long string for test.\n Very Good,\r VERY HAPPY.";
    CreateTestFile(filename, content);
    EXPECT_EQ(CountStrInFile(filename, str1, true), 2);
    EXPECT_EQ(CountStrInFile(filename, str1, false), 4);

    EXPECT_EQ(CountStrInFile(filename, str2, true), 1);
    EXPECT_EQ(CountStrInFile(filename, str2, false), 4);

    EXPECT_EQ(CountStrInFile(filename, str3, true), 0);
    RemoveTestFile(filename);
}

/*
 * @tc.name: testCountStrInFile005
 * @tc.desc: singleton template
 */
HWTEST_F(UtilsFileTest, testCountStrInFile005, TestSize.Level0)
{
    string str1 = "aba";
    string filename = FILE_PATH;
    string content = "This is abababaBABa.";
    CreateTestFile(filename, content);
    EXPECT_EQ(CountStrInFile(filename, str1, true), 2);
    EXPECT_EQ(CountStrInFile(filename, str1, false), 3);
    RemoveTestFile(filename);
}
}  // namespace
}  // namespace OHOS