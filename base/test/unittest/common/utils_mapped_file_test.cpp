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
#include "mapped_file.h"

#include <fstream>
#include <gtest/gtest.h>
#include <iostream>
#include <sys/stat.h>
#include <unistd.h>
#include "common_mapped_file_errors.h"
#include "directory_ex.h"
#include "errors.h"
#include "file_ex.h"

using namespace testing::ext;
using namespace OHOS::Utils;

namespace OHOS {
namespace {

class UtilsMappedFileTest : public testing::Test {
public:
    static constexpr char BASE_PATH[] = "/data/test/commonlibrary_c_utils/";
    static constexpr char SUITE_PATH[] = "mapped_file/";
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
};

void UtilsMappedFileTest::SetUpTestCase()
{
    std::string dir = std::string(BASE_PATH).append(SUITE_PATH);
    if (ForceCreateDirectory(dir)) {
        std::cout << "Create test dir:" << dir.c_str() << std::endl;
    } else {
        std::cout << "Create test dir Failed:" << dir.c_str() << std::endl;
    }

    std::cout << "Page size:" << MappedFile::PageSize() << std::endl;
}

void UtilsMappedFileTest::TearDownTestCase()
{
    if (ForceRemoveDirectory(std::string(BASE_PATH))) {
        std::cout << "Remove test dir:" << BASE_PATH << std::endl;
    }
}

void PrintStatus(MappedFile& mf)
{
    std::cout << "Mapped Region Start:" << reinterpret_cast<void*>(mf.RegionStart()) << std::endl <<
                 "Mapped Region End:" << reinterpret_cast<void*>(mf.RegionEnd()) << std::endl <<
                 "View start:" << reinterpret_cast<void*>(mf.Begin()) << std::endl <<
                 "View End:" << reinterpret_cast<void*>(mf.End()) << std::endl <<
                 "View Size:" << mf.Size() << std::endl <<
                 "File Offset Start:" << mf.StartOffset() << std::endl <<
                 "File Offset Start:" << mf.EndOffset() << std::endl;
}

bool CreateTestFile(const std::string& path, const std::string& content)
{
    std::ofstream out(path, std::ios_base::out | std::ios_base::trunc);
    if (out.is_open()) {
        out << content.c_str();
        return true;
    }

    std::cout << "open file failed!" << path.c_str() << std::endl;
    return false;
}

int RemoveTestFile(const std::string& path)
{
    return unlink(path.c_str());
}

bool SaveStringToFile(const std::string& filePath, const std::string& content, off_t offset, bool truncated /*= true*/)
{
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

void ReCreateFile(std::string& filename, const std::string& content)
{
    filename.insert(0, UtilsMappedFileTest::SUITE_PATH).insert(0, UtilsMappedFileTest::BASE_PATH);
    RemoveTestFile(filename);

    ASSERT_TRUE(CreateTestFile(filename, content));
}

void TestFileStatusAndRead(MappedFile& mf,
                           const std::string& filename,
                           const std::string& content,
                           struct stat* stb)
{
    ASSERT_NE(stb, nullptr);
    ASSERT_EQ(mf.Map(), MAPPED_FILE_ERR_OK);

    // check status after mapping
    ASSERT_TRUE(mf.IsMapped());
    ASSERT_TRUE(mf.IsNormed());

    // check size
    stat(filename.c_str(), stb);
    ASSERT_TRUE(stb->st_size == mf.Size() || mf.PageSize() == mf.Size());

    // read from Mapped File
    std::string readout;
    char* cur = mf.Begin();
    for (; cur <= mf.End(); cur++) {
        readout.push_back(*cur);
    }
    EXPECT_EQ(readout, content);

    // write to the extended region
    *(cur) = 'E';
    EXPECT_EQ((*cur), 'E');
}

void TestFileReadAndWrite(const MappedFile& mf, const std::string& content)
{
    // read from mapped file
    std::string readout;
    for (char* cur = mf.Begin(); cur <= mf.End(); cur++) {
        readout.push_back(*cur);
    }
    EXPECT_EQ(readout, content);

    // write to mapped file
    std::string toWrite("Complete.");
    char* newCur = mf.Begin();
    for (std::string::size_type i = 0; i < toWrite.length(); i++) {
        (*newCur) = toWrite[i];
        newCur++;
    }
}

void TestFileStat(MappedFile& mf,
                  const std::string& filename,
                  const std::string& content,
                  struct stat* stb)
{
    ASSERT_NE(stb, nullptr);
    ASSERT_EQ(mf.Map(), MAPPED_FILE_ERR_OK);

    // check status after mapping
    ASSERT_TRUE(mf.IsMapped());
    ASSERT_TRUE(mf.IsNormed());

    // check size
    stat(filename.c_str(), stb);
    ASSERT_TRUE(stb->st_size == mf.Size() || mf.PageSize() == mf.Size());

    // read from Mapped File
    std::string readout;
    for (char* cur = mf.Begin(); cur <= mf.End(); cur++) {
        readout.push_back(*cur);
    }
    EXPECT_EQ(readout, content);
}

void TestFileContentEqual(const MappedFile& mf,
                          const std::string& filename,
                          std::string& filename1,
                          std::string& content1,
                          struct stat* stb)
{
    // check pointer not null
    ASSERT_NE(stb, nullptr);
    // check status after remapping
    EXPECT_TRUE(mf.IsMapped());
    EXPECT_TRUE(mf.IsNormed());

    // check size
    stat(filename1.c_str(), stb);
    EXPECT_TRUE(stb->st_size == mf.Size());

    // read from Mapped File
    std::string readout1;
    for (char* cur1 = mf.Begin(); cur1 <= mf.End(); cur1++) {
        readout1.push_back(*cur1);
    }
    EXPECT_EQ(readout1, content1);

    RemoveTestFile(filename);
    RemoveTestFile(filename1);
}

void TestFileRegion(MappedFile& mf, const off_t orig)
{
    off_t endOff;
    // keep turnNext within a page
    ASSERT_NE(orig, 0);
    if (orig != 0) {
        for (unsigned int cnt = 2; cnt < (MappedFile::PageSize() / orig); cnt++) { // 2: start from 2 to take the first
                                                                                // TunrNext() calling in consideration.
            endOff = mf.EndOffset();
            EXPECT_EQ(mf.TurnNext(), MAPPED_FILE_ERR_OK);
            EXPECT_EQ(mf.StartOffset(), endOff + 1);
            EXPECT_EQ(mf.Size(), orig);
        }
    }
    std::cout << "==Last TurnNext() with The Same Size==" << std::endl;
    PrintStatus(mf);

    // this turn will reach the bottom of a page
    endOff = mf.EndOffset();
    char* rEnd = mf.RegionEnd();
    char* end = mf.End();
    EXPECT_EQ(mf.TurnNext(), MAPPED_FILE_ERR_OK);
    EXPECT_EQ(mf.StartOffset(), endOff + 1);
    EXPECT_EQ(mf.Size(), static_cast<off_t>(rEnd - end));
    std::cout << "==Reached Bottom of A Page==" << std::endl;
    PrintStatus(mf);
}

void TestFileWrite(const MappedFile& mfNew, const std::string& filename, const std::string& content)
{
    // read from mapped file
    std::string readout;
    for (char* cur = mfNew.Begin(); cur <= mfNew.End(); cur++) {
        readout.push_back(*cur);
    }
    EXPECT_EQ(readout, content);

    // write to mapped file
    std::string toWrite("Complete.");
    char* newCur = mfNew.Begin();
    for (std::string::size_type i = 0; i < toWrite.length(); i++) {
        (*newCur) = toWrite[i];
        newCur++;
    }
    std::string res;
    LoadStringFromFile(filename, res);
    EXPECT_EQ(res, "Complete.move use.");

    RemoveTestFile(filename);
}

void TestTwoFileWrite(const MappedFile& mf,
                      const std::string& filename,
                      const std::string& filename1,
                      const std::string& content1)
{
    std::string readout;
    for (char* cur = mf.Begin(); cur <= mf.End(); cur++) {
        readout.push_back(*cur);
    }
    EXPECT_EQ(readout, content1);

    //write to mapped file
    std::string toWrite("Complete.");
    char* newCur = mf.Begin();
    for (std::string::size_type i = 0; i < toWrite.length(); i++) {
        (*newCur) = toWrite[i];
        newCur++;
    }
    std::string res;
    LoadStringFromFile(filename1, res);
    EXPECT_EQ(res, "Complete.move use.");

    RemoveTestFile(filename);
    RemoveTestFile(filename1);
}

/*
 * @tc.name: testDefaultMapping001
 * @tc.desc: Test file mapping with default params.
 */
HWTEST_F(UtilsMappedFileTest, testDefaultMapping001, TestSize.Level0)
{
    // 1. Create a new file
    std::string filename = "test_read_write_1.txt";
    std::string content = "Test for normal use.";
    ReCreateFile(filename, content);

    // 2. map file
    MappedFile mf(filename);
    ASSERT_EQ(mf.Map(), MAPPED_FILE_ERR_OK);

    // check status
    ASSERT_TRUE(mf.IsMapped());
    ASSERT_TRUE(mf.IsNormed());

    // check size
    struct stat stb = {0};
    stat(filename.c_str(), &stb);
    ASSERT_TRUE(stb.st_size == mf.Size() || mf.PageSize() == mf.Size());

    // check map-mode
    ASSERT_EQ(MapMode::DEFAULT, mf.GetMode());

    // check offset
    ASSERT_EQ(mf.StartOffset(), 0u);

    // 3. read from mapped file
    // write to mapped file
    TestFileReadAndWrite(mf, content);

    std::string res;
    LoadStringFromFile(filename, res);
    EXPECT_EQ(res, "Complete.normal use.");

    // 5. test default mapping and write to addr which excess End() but not across this memory page.
    EXPECT_LE(mf.Size(), mf.PageSize());
    char* trueEnd = mf.RegionEnd();
    ASSERT_GT(trueEnd, mf.Begin());
    // write to mapped file
    (*trueEnd) = 'E'; // It is allowed to write to this address which excess the End()

    EXPECT_EQ((*trueEnd), 'E'); // and of course it is allowed to read from that same address.

    std::string res1;
    LoadStringFromFile(filename, res1);
    EXPECT_EQ(res1, "Complete.normal use."); // While no changes will be sync in the original file.

    RemoveTestFile(filename);
}

/*
 * @tc.name: testNewSharedMappingDefaultSize001
 * @tc.desc: Test mapping which will create a new file with default size.
 */
HWTEST_F(UtilsMappedFileTest, testNewSharedMappingDefaultSize001, TestSize.Level0)
{
    // 1. Create a new file
    std::string filename = "test_read_write_2.txt";
    filename.insert(0, SUITE_PATH).insert(0, BASE_PATH);
    RemoveTestFile(filename);

    // 2. map file
    MappedFile mf(filename, MapMode::DEFAULT | MapMode::CREATE_IF_ABSENT);
    ASSERT_EQ(mf.Map(), MAPPED_FILE_ERR_OK);

    // check if file is created
    ASSERT_TRUE(FileExists(filename));

    // check status
    ASSERT_TRUE(mf.IsMapped());
    ASSERT_TRUE(mf.IsNormed());

    // check map-mode
    ASSERT_EQ(MapMode::DEFAULT | MapMode::CREATE_IF_ABSENT, mf.GetMode());

    // check default size
    struct stat stb = {0};
    if (stat(filename.c_str(), &stb) == 0) {
        EXPECT_EQ(stb.st_size, mf.PageSize()); // contents will be zero-filled.
    }
    ASSERT_EQ(mf.Size(), mf.PageSize());

    // 3. write to mapped file
    std::string toWrite("Write to newly created file.");
    char* newCur = mf.Begin();
    for (std::string::size_type i = 0; i < toWrite.length(); i++) {
        (*newCur) = toWrite[i];
        newCur++;
    }
    std::string res;
    LoadStringFromFile(filename, res);
    EXPECT_STREQ(res.c_str(), toWrite.c_str()); // note that `res` contains filled '0',
                                                // use c_str() to compare conveniently.

    // 4. read from mapped file
    std::string toRead("Waiting to be read.");
    SaveStringToFile(filename, toRead, 0, true);
    std::string readout;
    for (char* cur = mf.Begin(); *cur != '\0'; cur++) {
        readout.push_back(*cur);
    }
    EXPECT_EQ(readout, toRead);

    RemoveTestFile(filename);
}

/*
 * @tc.name: testNewSharedMapping001
 * @tc.desc: Test mapping which will create a new file with specified params.
 */
HWTEST_F(UtilsMappedFileTest, testNewSharedMapping001, TestSize.Level0)
{
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
    ASSERT_EQ(mf.Map(), MAPPED_FILE_ERR_OK);

    // check if file is created
    ASSERT_TRUE(FileExists(filename));

    // check status
    ASSERT_TRUE(mf.IsMapped());
    ASSERT_TRUE(mf.IsNormed());

    // check specified size
    struct stat stb = {0};
    if (stat(filename.c_str(), &stb) == 0) {
        // Exact file size should be offset + mapped size, contents will be zero-filled.
        EXPECT_EQ(stb.st_size, offset + size);
    }
    ASSERT_EQ(mf.Size(), size);

    // check specified offset
    ASSERT_EQ(mf.StartOffset(), offset);

    // check hint
    ASSERT_TRUE(mf.GetHint() == nullptr || mf.GetHint() == hint);
    std::cout << "Exact addr:" <<
              reinterpret_cast<void*>(mf.Begin()) << std::endl <<
              "Input hint:" << reinterpret_cast<void*>(hint) << std::endl;

    // 2. write to mapped file
    std::string toWrite("Write to newly created file.");
    char* newCur = mf.Begin();
    for (std::string::size_type i = 0; i < toWrite.length(); i++) {
        (*newCur) = toWrite[i];
        newCur++;
    }
    std::cout << "Write finished" << std::endl;
    EXPECT_TRUE(StringExistsInFile(filename, toWrite));

    // 3. read from mapped file
    std::string toRead("Waiting to be read.");
    SaveStringToFile(filename, toRead, offset, true);
    std::string readout;
    for (char* cur = mf.Begin(); cur <= mf.End() && *cur != '\0'; cur++) {
        readout.push_back(*cur);
    }
    std::cout << "Read finished" << std::endl;
    EXPECT_EQ(readout, toRead);

    RemoveTestFile(filename);
}

/*
 * @tc.name: testPrivateMapping001
 * @tc.desc: Test mapping which will create a new file with specified params.
 */
HWTEST_F(UtilsMappedFileTest, testPrivateMapping001, TestSize.Level0)
{
    // 1. create a new file
    std::string filename = "test_read_write_4.txt";
    std::string content = "Test for private use.";
    ReCreateFile(filename, content);

    // 2. map file
    MappedFile mf(filename, MapMode::DEFAULT | MapMode::PRIVATE);
    ASSERT_EQ(mf.Map(), MAPPED_FILE_ERR_OK);

    // 3. check status
    ASSERT_TRUE(mf.IsMapped());
    ASSERT_TRUE(mf.IsNormed());

    // 4. read from mapped file
    // write to mapped file
    TestFileReadAndWrite(mf, content);

    std::string res;
    LoadStringFromFile(filename, res);
    EXPECT_EQ(res, content); // changes to private mapped file will not write back to the original file

    RemoveTestFile(filename);
}

/*
 * @tc.name: testSharedReadOnlyMapping001
 * @tc.desc: Test mapping which will create a new file with specified params.
 */
HWTEST_F(UtilsMappedFileTest, testSharedReadOnlyMapping001, TestSize.Level0)
{
    // 1. create a new file
    std::string filename = "test_read_write_5.txt";
    std::string content = "Test for readonly use.";

    ReCreateFile(filename, content);

    // 2. map file
    MappedFile mf(filename, MapMode::DEFAULT | MapMode::READ_ONLY);
    ASSERT_EQ(mf.Map(), MAPPED_FILE_ERR_OK);

    // 3. check status
    ASSERT_TRUE(mf.IsMapped());
    ASSERT_TRUE(mf.IsNormed());

    // 4. read from mapped file
    std::string readout;
    for (char* cur = mf.Begin(); cur <= mf.End(); cur++) {
        readout.push_back(*cur);
    }
    EXPECT_EQ(readout, content);
    // !Note: write operation is not permitted, which will raise a signal 11.

    RemoveTestFile(filename);
}

/*
 * @tc.name: testReMap001
 * @tc.desc: Test remapping using `Unmap()` and `Map()`
 */
HWTEST_F(UtilsMappedFileTest, testReMap001, TestSize.Level0)
{
    // 1. create a new file
    std::string filename = "test_remap_1.txt";
    std::string content = "Test for remapping use.";
    ReCreateFile(filename, content);

    // 2. map file
    MappedFile mf(filename);
    ASSERT_EQ(mf.Map(), MAPPED_FILE_ERR_OK);

    // 3. check status after mapping
    ASSERT_TRUE(mf.IsMapped());
    ASSERT_TRUE(mf.IsNormed());

    ASSERT_EQ(mf.Unmap(), MAPPED_FILE_ERR_OK);

    // 4. check status after unmapping
    EXPECT_FALSE(mf.IsMapped());
    EXPECT_TRUE(mf.IsNormed());
    EXPECT_EQ(mf.Begin(), nullptr);

    ASSERT_EQ(mf.Map(), MAPPED_FILE_ERR_OK);
    // 5. check status after remapping
    EXPECT_TRUE(mf.IsMapped());
    EXPECT_TRUE(mf.IsNormed());

    // 6. check default size
    struct stat stb = {0};
    stat(filename.c_str(), &stb);
    EXPECT_TRUE(stb.st_size == mf.Size() || mf.PageSize() == mf.Size());

    RemoveTestFile(filename);
}

/*
 * @tc.name: testReMap002
 * @tc.desc: Test remapping via changing params.
 */
HWTEST_F(UtilsMappedFileTest, testReMap002, TestSize.Level0)
{
    // 1. create a new file
    std::string filename = "test_remap.txt";
    std::string content = "Test for default use.";
    ReCreateFile(filename, content);

    std::string filename1 = "test_remap_1.txt";
    std::string content1 = "Test for remapping use.";
    ReCreateFile(filename1, content1);

    MappedFile mf(filename);

    // Change params when unmapped.
    ASSERT_TRUE(mf.ChangeSize(mf.Size() + 1024));
    ASSERT_TRUE(mf.ChangeSize(MappedFile::DEFAULT_LENGTH));
    ASSERT_TRUE(mf.ChangeOffset(mf.PageSize()));
    ASSERT_TRUE(mf.ChangeOffset(0));
    ASSERT_TRUE(mf.ChangePath(filename1));
    ASSERT_TRUE(mf.ChangePath(filename));
    ASSERT_TRUE(mf.ChangeHint(reinterpret_cast<char*>(0x89000))); // 0x89000: random address.
    ASSERT_TRUE(mf.ChangeMode(MapMode::READ_ONLY));

    struct stat stb = {0};

    // 3. check status after mapping
    // check size
    // read from Mapped File
    TestFileStat(mf, filename, content, &stb);

    // 4. change params
    ASSERT_TRUE(mf.ChangePath(filename1));
    ASSERT_TRUE(mf.ChangeSize(MappedFile::DEFAULT_LENGTH));
    ASSERT_TRUE(mf.ChangeHint(reinterpret_cast<char*>(0x80000))); // 0x80000: random address.
    ASSERT_TRUE(mf.ChangeMode(MapMode::DEFAULT | MapMode::CREATE_IF_ABSENT));

    // 5. check status after changing
    EXPECT_FALSE(mf.IsMapped());
    EXPECT_FALSE(mf.IsNormed());

    // 6. remap file
    ASSERT_EQ(mf.Map(), MAPPED_FILE_ERR_OK);

    TestFileContentEqual(mf, filename, filename1, content1, &stb);
}

/*
 * @tc.name: testReMap003
 * @tc.desc: Test remapping via Resize().
 */
HWTEST_F(UtilsMappedFileTest, testReMap003, TestSize.Level0)
{
    // 1. create a new file
    std::string filename = "test_remap.txt";
    std::string content = "Test for default use.";

    std::string filename1 = "test_remap_1.txt";
    std::string content1 = "Test for remapping use.";
    ReCreateFile(filename, content);
    ReCreateFile(filename1, content1);

    // 2. map file
    MappedFile mf(filename);

    struct stat stb = {0};

    // 3. check status after mapping
    // check size
    // read from Mapped File
    TestFileStat(mf, filename, content, &stb);

    // 4. change params
    mf.ChangePath(filename1);
    mf.ChangeSize(MappedFile::DEFAULT_LENGTH);

    // 5. check status after changing
    EXPECT_FALSE(mf.IsMapped());
    EXPECT_FALSE(mf.IsNormed());

    // 6. remap file
    ASSERT_EQ(mf.Resize(), MAPPED_FILE_ERR_OK);
    // 7. check status after remapping
    TestFileContentEqual(mf, filename, filename1, content1, &stb);
}

/*
 * @tc.name: testReMap004
 * @tc.desc: Test remapping only to extend mapped region via Resize(off_t, bool).
 */
HWTEST_F(UtilsMappedFileTest, testReMap004, TestSize.Level0)
{
    // 1. create a new file
    std::string content = "Test for remapping use.";
    std::string filename = "test_remap.txt";
    ReCreateFile(filename, content);

    // 2. map file
    MappedFile mf(filename);
    struct stat stb = {0};

    // 3. check status after mapping
    // check size
    // read from Mapped File
    // write to the extended region
    TestFileStatusAndRead(mf, filename, content, &stb);

    // 4. Remap to extend region
    ASSERT_EQ(mf.Resize(mf.Size() + 10), MAPPED_FILE_ERR_OK);
    // 5. check status after remapping
    EXPECT_TRUE(mf.IsMapped());
    EXPECT_TRUE(mf.IsNormed());

    // 6. check size after remapping
    stat(filename.c_str(), &stb);
    EXPECT_TRUE(stb.st_size < mf.Size());

    std::string res;
    LoadStringFromFile(filename, res);
    EXPECT_EQ(res, content); // No changes will be sync in the original file, since mapped region
                             // is larger than substantial size of the file

    RemoveTestFile(filename);
}

/*
 * @tc.name: testReMap005
 * @tc.desc: Test remapping to extend mapped region as well as substantial file size via Resize(off_t, bool).
 */
HWTEST_F(UtilsMappedFileTest, testReMap005, TestSize.Level0)
{
    // 1. create a new file
    std::string filename = "test_remap.txt";
    std::string content = "Test for remapping use.";
    ReCreateFile(filename, content);

    // 2. map file
    MappedFile mf(filename);
    struct stat stb = {0};

    // 3. check status after mapping
    // check size
    // read from Mapped File
    // write to the extended region
    TestFileStatusAndRead(mf, filename, content, &stb);

    // 4. remap to extend region
    ASSERT_EQ(mf.Resize(mf.Size() + 10, true), MAPPED_FILE_ERR_OK);
    // check status after remapping
    EXPECT_TRUE(mf.IsMapped());
    EXPECT_TRUE(mf.IsNormed());

    // 5. check size after remapping
    stat(filename.c_str(), &stb);
    EXPECT_TRUE(stb.st_size == mf.Size()); // File size will sync to that of the mapped region.

    std::string res;
    LoadStringFromFile(filename, res);
    EXPECT_STREQ(res.c_str(), content.append("E").c_str()); // Changes will be sync in the original file.
}

/*
 * @tc.name: testReMap006
 * @tc.desc: Test remapping to via Resize(off_t, bool).
 */
HWTEST_F(UtilsMappedFileTest, testReMap006, TestSize.Level0)
{
    // 1. create a new file
    std::string filename = "test_remap.txt";
    std::string content = "Test for remapping use.";

    ReCreateFile(filename, content);

    // 2. map file
    off_t size = 20;
    MappedFile mf(filename, MapMode::DEFAULT, 0, size);
    ASSERT_EQ(mf.Map(), MAPPED_FILE_ERR_OK);

    // 3. check status after mapping
    ASSERT_TRUE(mf.IsMapped());
    ASSERT_TRUE(mf.IsNormed());

    // 4. check size
    ASSERT_TRUE(size == mf.Size());

    // 5. remap to extend region
    ASSERT_EQ(mf.Resize(MappedFile::DEFAULT_LENGTH, true), MAPPED_FILE_ERR_OK);
    off_t lessSize = mf.Size() - 8;
    ASSERT_EQ(mf.Resize(lessSize, true), MAPPED_FILE_ERR_OK);
    // 6. check status after remapping
    EXPECT_TRUE(mf.IsMapped());
    EXPECT_TRUE(mf.IsNormed());

    // 7. check size after remapping
    struct stat stb = {0};
    stat(filename.c_str(), &stb);
    EXPECT_EQ(lessSize, mf.Size());
}

/*
 * @tc.name: testTurnNext001
 * @tc.desc: Test TurnNext() when `IsMapped()`.
 */
HWTEST_F(UtilsMappedFileTest, testTurnNext001, TestSize.Level0)
{
    // 1. create a new file
    std::string filename = "test_remap.txt";
    std::string content = "Test for remapping use.";

    ReCreateFile(filename, content);

    struct stat stb = {0};
    ASSERT_EQ(stat(filename.c_str(), &stb), 0);
    off_t orig = stb.st_size; // 23 bytes

    // 2. extend its size
    int fd = open(filename.c_str(), O_RDWR | O_CLOEXEC);
    ASSERT_NE(fd, -1);
    ASSERT_EQ(ftruncate(fd, MappedFile::PageSize() + MappedFile::PageSize() / 100LL), 0); // 100: ratio to a page.

    // 3. map file
    MappedFile mf(filename, MapMode::DEFAULT, 0, orig);
    ASSERT_EQ(mf.Map(), MAPPED_FILE_ERR_OK);

    // 4. check status after mapping
    ASSERT_TRUE(mf.IsMapped());
    ASSERT_TRUE(mf.IsNormed());

    // 5. turn next mapped region with the same size as the file's initial size.
    EXPECT_EQ(mf.TurnNext(), MAPPED_FILE_ERR_OK);
    char* cur = mf.Begin();
    *cur = 'N';

    std::string res;
    LoadStringFromFile(filename, res);
    EXPECT_STREQ(res.c_str(), content.append("N").c_str());

    // 6. keep turnNext within a page
    // this turn will reach the bottom of a page
    TestFileRegion(mf, orig);

    // 7. this turn will trigger a remapping
    off_t endOff = mf.EndOffset();
    off_t curSize = mf.Size();
    EXPECT_EQ(mf.TurnNext(), MAPPED_FILE_ERR_OK);
    EXPECT_TRUE(mf.IsMapped());
    EXPECT_EQ(mf.StartOffset(), endOff + 1);
    EXPECT_EQ(mf.Size(), curSize);
    EXPECT_EQ(mf.RegionStart(), mf.Begin());
    EXPECT_EQ(static_cast<off_t>(mf.RegionEnd() - mf.RegionStart()) + 1LL, mf.PageSize());
    std::cout << "==Remap A New Page==" << std::endl;
    PrintStatus(mf);

    // 8. keep turnNext within a page
    for (off_t cnt = 1; cnt < (MappedFile::PageSize() / 100LL / curSize); cnt++) {
        endOff = mf.EndOffset();
        EXPECT_EQ(mf.TurnNext(), MAPPED_FILE_ERR_OK);
        EXPECT_EQ(mf.StartOffset(), endOff + 1);
        EXPECT_EQ(mf.Size(), curSize);
    }

    // 10. this turn will fail since no place remained.
    EXPECT_NE(mf.TurnNext(), MAPPED_FILE_ERR_OK);

    RemoveTestFile(filename);
}

/*
 * @tc.name: testTurnNext002
 * @tc.desc: Test TurnNext() when `!IsMapped()`.
 */
HWTEST_F(UtilsMappedFileTest, testTurnNext002, TestSize.Level0)
{
    // 1. create a new file
    std::string filename = "test_remap.txt";
    std::string content = "Test for remapping use.";

    ReCreateFile(filename, content);

    // 2. map file
    MappedFile mf(filename);
    ASSERT_EQ(mf.Map(), MAPPED_FILE_ERR_OK);
    off_t curSize = mf.Size();
    off_t curOff = mf.StartOffset();

    // 3. check status after mapping
    ASSERT_TRUE(mf.IsMapped());
    ASSERT_TRUE(mf.IsNormed());
    // 4. recommand to unmap first before other operations on the file.
    ASSERT_EQ(mf.Unmap(), MAPPED_FILE_ERR_OK);
    // 5. enlarge file size to make it possible to `turnNext()`.
    ASSERT_EQ(ftruncate(mf.GetFd(), MappedFile::PageSize() + MappedFile::PageSize() / 100LL), 0);
    // 6. turn next page of `PageSize()` and keep the same `size_`
    EXPECT_EQ(mf.TurnNext(), MAPPED_FILE_ERR_OK);
    EXPECT_EQ(mf.Size(), curSize);
    EXPECT_EQ(static_cast<off_t>(mf.StartOffset()), curOff + mf.PageSize());

    RemoveTestFile(filename);
}

/*
 * @tc.name: testTurnNext003
 * @tc.desc: Test TurnNext() (using internal fd to `ftruncate()`).
 */
HWTEST_F(UtilsMappedFileTest, testTurnNext003, TestSize.Level0)
{
    // 1. create a new file
    std::string content = "Test for remapping use.";
    std::string filename = "test_remap.txt";

    ReCreateFile(filename, content);

    // 2. map file
    MappedFile mf(filename);
    ASSERT_EQ(mf.Map(), MAPPED_FILE_ERR_OK);

    // 3. check status after mapping
    ASSERT_TRUE(mf.IsNormed());
    ASSERT_TRUE(mf.IsMapped());

    // 4. recommand to unmap first before other operations on the file.
    ASSERT_EQ(mf.Unmap(), MAPPED_FILE_ERR_OK);
    // 5. enlarge file size to make it possible to `turnNext()`.
    ASSERT_EQ(ftruncate(mf.GetFd(), MappedFile::PageSize() + MappedFile::PageSize() / 100LL), 0);

    // 6. remap
    ASSERT_EQ(mf.Map(), MAPPED_FILE_ERR_OK);

    // 7. turn next mapped region with the same size as the file's initial size.
    ASSERT_EQ(mf.TurnNext(), MAPPED_FILE_ERR_OK);
    char* cur = mf.Begin();
    *cur = 'N';

    std::string res;
    LoadStringFromFile(filename, res);
    EXPECT_STREQ(res.c_str(), content.append("N").c_str());

    RemoveTestFile(filename);
}

/*
 * @tc.name: testTurnNext004
 * @tc.desc: Test TurnNext() failed.
 */
HWTEST_F(UtilsMappedFileTest, testTurnNext004, TestSize.Level0)
{
    // 1. create a new file
    std::string filename = "test_remap.txt";
    std::string content = "Test for remapping use.";

    ReCreateFile(filename, content);

    // 2. map file
    MappedFile mf(filename);
    ASSERT_EQ(mf.Map(), MAPPED_FILE_ERR_OK);

    // 3. check status after mapping
    ASSERT_TRUE(mf.IsMapped());
    ASSERT_TRUE(mf.IsNormed());

    // 4. turn next mapped region with the same size as the file's initial size.
    EXPECT_EQ(mf.TurnNext(), ERR_INVALID_OPERATION);

    RemoveTestFile(filename);
}

/*
 * @tc.name: testTurnNext005
 * @tc.desc: Test TurnNext() with file size less than one page.
 */
HWTEST_F(UtilsMappedFileTest, testTurnNext005, TestSize.Level0)
{
    // 1. create a new file
    std::string filename = "test_remap.txt";
    std::string content = "Test for remapping use.00";

    ReCreateFile(filename, content);

    struct stat stb = {0};
    ASSERT_EQ(stat(filename.c_str(), &stb), 0);
    off_t orig = stb.st_size; // 25 bytes

    // 2. extend its size
    int fd = open(filename.c_str(), O_RDWR | O_CLOEXEC);
    ASSERT_NE(fd, -1);
    ASSERT_EQ(ftruncate(fd, MappedFile::PageSize() + 10), 0); // 10: remain contents less than 25bits.

    // 3. map file
    MappedFile mf(filename, MapMode::DEFAULT, 0, orig);
    ASSERT_EQ(mf.Map(), MAPPED_FILE_ERR_OK);

    // 4. check status after mapping
    ASSERT_TRUE(mf.IsMapped());
    ASSERT_TRUE(mf.IsNormed());

    // 5. turn next mapped region with the same size as the file's initial size.
    EXPECT_EQ(mf.TurnNext(), MAPPED_FILE_ERR_OK);

    // 6. keep turnNext within a page
    // this turn will reach the bottom of a page
    TestFileRegion(mf, orig);

    // 7. this turn will trigger a remapping
    off_t endOff = mf.EndOffset();
    EXPECT_EQ(mf.TurnNext(), MAPPED_FILE_ERR_OK);
    EXPECT_TRUE(mf.IsMapped());
    EXPECT_EQ(mf.StartOffset(), endOff + 1);
    EXPECT_EQ(mf.Size(), 10);
    EXPECT_EQ(mf.RegionStart(), mf.Begin());
    EXPECT_EQ(static_cast<off_t>(mf.RegionEnd() - mf.RegionStart()) + 1LL, mf.PageSize());
    std::cout << "==Remap A New Page==" << std::endl;
    PrintStatus(mf);
}

/*
 * @tc.name: testInvalidMap001
 * @tc.desc: Test file mapping with invalid offset.
 */
HWTEST_F(UtilsMappedFileTest, testInvalidMap001, TestSize.Level0)
{
    // 1. create a new file
    std::string filename = "test_invalid_1.txt";
    std::string content = "Test for invalid use.";

    ReCreateFile(filename, content);

    // 2. map file
    off_t offset = 100; // Specify offset that is not multiple of page-size.
    MappedFile mf(filename, MapMode::DEFAULT, offset);
    ASSERT_NE(mf.Map(), MAPPED_FILE_ERR_OK);

    MappedFile mf1(filename, MapMode::DEFAULT, -1);
    ASSERT_NE(mf1.Map(), MAPPED_FILE_ERR_OK);

    // 3. check status
    EXPECT_FALSE(mf.IsMapped());
    EXPECT_FALSE(mf.IsNormed()); // mapping will fail in normalize stage.
    EXPECT_FALSE(mf1.IsMapped());
    EXPECT_FALSE(mf1.IsNormed()); // mapping will fail in normalize stage.

    RemoveTestFile(filename);
}

/*
 * @tc.name: testInvalidMap002
 * @tc.desc: Test file mapping with invalid offset excessing the substantial size of the file.
 */
HWTEST_F(UtilsMappedFileTest, testInvalidMap002, TestSize.Level0)
{
    // 1. create a new file
    std::string filename = "test_invalid_2.txt";
    std::string content = "Test for invalid use.";

    ReCreateFile(filename, content);

    // 2. map file
    off_t offset = 4 * 1024; // Specify offset excessing the substantial size of the file.
    MappedFile mf(filename, MapMode::DEFAULT, offset);
    ASSERT_NE(mf.Map(), MAPPED_FILE_ERR_OK);

    // 3. check status
    EXPECT_FALSE(mf.IsMapped());
    EXPECT_FALSE(mf.IsNormed()); // mapping will fail in normalize stage.

    RemoveTestFile(filename);
}

/*
 * @tc.name: testInvalidMap003
 * @tc.desc: Test mapping non-existed file without setting CREAT_IF_ABSENT.
 */
HWTEST_F(UtilsMappedFileTest, testInvalidMap003, TestSize.Level0)
{
    // 1. create a new file
    std::string filename = "test_invalid_3.txt";
    filename.insert(0, SUITE_PATH).insert(0, BASE_PATH);
    RemoveTestFile(filename);

    // 2. map file
    MappedFile mf(filename);
    ASSERT_NE(mf.Map(), MAPPED_FILE_ERR_OK);

    // 3. check status
    EXPECT_FALSE(mf.IsMapped());
    EXPECT_FALSE(mf.IsNormed()); // mapping will fail in normalize stage.

    RemoveTestFile(filename);
}

/*
 * @tc.name: testInvalidMap004
 * @tc.desc: Test mapping with invalid size.
 */
HWTEST_F(UtilsMappedFileTest, testInvalidMap004, TestSize.Level0)
{
    // 1. create a new file
    std::string filename = "test_invalid_4.txt";
    std::string content = "Test for invalid use.";

    ReCreateFile(filename, content);

    // 2. map file
    MappedFile mf(filename, MapMode::DEFAULT, 0, -2); // -2: less than DEFAULT_LENGTH(-1)
    ASSERT_EQ(mf.Map(), ERR_INVALID_VALUE);

    // 3. map again with another invalid param.
    MappedFile mf1(filename, MapMode::DEFAULT, 0, 0);
    ASSERT_EQ(mf1.Map(), ERR_INVALID_VALUE);

    // 4. check status
    EXPECT_FALSE(mf.IsMapped());
    EXPECT_FALSE(mf.IsNormed()); // mapping will fail in normalize stage.
    EXPECT_FALSE(mf1.IsMapped());
    EXPECT_FALSE(mf1.IsNormed()); // mapping will fail in normalize stage.

    RemoveTestFile(filename);
}

/*
 * @tc.name: testInvalidMap005
 * @tc.desc: Test mapping an already mapped file.
 */
HWTEST_F(UtilsMappedFileTest, testInvalidMap005, TestSize.Level0)
{
    // 1. create a new file
    std::string filename = "test_invalid_6.txt";
    std::string content = "Test for invalid use.";

    ReCreateFile(filename, content);

    // 2. map file
    MappedFile mf(filename);
    ASSERT_EQ(mf.Map(), MAPPED_FILE_ERR_OK);
    ASSERT_EQ(mf.Map(), ERR_INVALID_OPERATION); // Map again.

    // 3. check status
    EXPECT_TRUE(mf.IsMapped());
    EXPECT_TRUE(mf.IsNormed());

    RemoveTestFile(filename);
}

/*
 * @tc.name: testInvalidMap006
 * @tc.desc: Test resize with invalid params.
 */
HWTEST_F(UtilsMappedFileTest, testInvalidMap006, TestSize.Level0)
{
    // 1. create a new file
    std::string filename = "test_invalid_7.txt";
    std::string content = "Test for invalid use.";

    ReCreateFile(filename, content);

    // 2. map file
    MappedFile mf(filename);
    ASSERT_EQ(mf.Map(), MAPPED_FILE_ERR_OK);

    // 3. check status
    ASSERT_TRUE(mf.IsMapped());
    ASSERT_TRUE(mf.IsNormed());

    // 4. resize
    EXPECT_EQ(mf.Resize(0), ERR_INVALID_OPERATION);
    EXPECT_EQ(mf.Resize(-2), ERR_INVALID_OPERATION); // -2: less than DEFAULT_LENGTH(-1).
    EXPECT_EQ(mf.Resize(mf.Size()), ERR_INVALID_OPERATION);

    // 5. Unmap first then resize.
    ASSERT_EQ(mf.Unmap(), MAPPED_FILE_ERR_OK);
    EXPECT_EQ(mf.Resize(mf.Size() + 8), ERR_INVALID_OPERATION);

    RemoveTestFile(filename);
}

/*
 * @tc.name: testInvalidMap007
 * @tc.desc: Test resize with no param changed.
 */
HWTEST_F(UtilsMappedFileTest, testInvalidMap007, TestSize.Level0)
{
    // 1. create a new file
    std::string filename = "test_invalid_8.txt";
    std::string content = "Test for invalid use.";

    ReCreateFile(filename, content);

    // 2. map file
    MappedFile mf(filename);
    ASSERT_EQ(mf.Map(), MAPPED_FILE_ERR_OK);

    // 3. check status
    ASSERT_TRUE(mf.IsMapped());
    ASSERT_TRUE(mf.IsNormed());

    // 4. resize
    EXPECT_EQ(mf.Resize(), ERR_INVALID_OPERATION);

    RemoveTestFile(filename);
}

/*
 * @tc.name: testInvalidMap008
 * @tc.desc: Test TurnNext() with params changed.
 */
HWTEST_F(UtilsMappedFileTest, testInvalidMap008, TestSize.Level0)
{
    // 1. create a new file
    std::string filename = "test_invalid_9.txt";
    std::string content = "Test for invalid use.";

    ReCreateFile(filename, content);

    // 2. map file
    MappedFile mf(filename);
    ASSERT_EQ(mf.Map(), MAPPED_FILE_ERR_OK);

    // 3. check status
    ASSERT_TRUE(mf.IsMapped());
    ASSERT_TRUE(mf.IsNormed());

    // 4. Change params
    ASSERT_TRUE(mf.ChangeSize(mf.Size() + 1));

    // 5. check status
    ASSERT_FALSE(mf.IsMapped());
    ASSERT_FALSE(mf.IsNormed());

    // 6. turn next.
    EXPECT_EQ(mf.TurnNext(), ERR_INVALID_OPERATION);

    RemoveTestFile(filename);
}

/*
 * @tc.name: testInvalidMap009
 * @tc.desc: Test ChangeXX() with invalid params.
 */
HWTEST_F(UtilsMappedFileTest, testInvalidMap009, TestSize.Level0)
{
    // 1. create a new file
    std::string filename = "test_invalid_10.txt";
    std::string content = "Test for invalid use.";

    ReCreateFile(filename, content);

    // 2. create MappedFile
    MappedFile mf(filename);

    // 3. map file
    ASSERT_EQ(mf.Map(), MAPPED_FILE_ERR_OK);

    // 4. check status
    ASSERT_TRUE(mf.IsMapped());
    ASSERT_TRUE(mf.IsNormed());

    // 5. Change params
    ASSERT_FALSE(mf.ChangeOffset(mf.StartOffset()));
    ASSERT_FALSE(mf.ChangeSize(mf.Size()));
    ASSERT_FALSE(mf.ChangeHint(mf.GetHint()));
    ASSERT_FALSE(mf.ChangeMode(mf.GetMode()));
    ASSERT_FALSE(mf.ChangePath(mf.GetPath()));

    RemoveTestFile(filename);
}

/*
 * @tc.name: testAutoAdjustedMode001
 * @tc.desc: Test mapping file with invalid mapping mode, but can be auto adjusted.
 */
HWTEST_F(UtilsMappedFileTest, testAutoAdjustedMode001, TestSize.Level0)
{
    // 1. create a new file
    std::string filename = "test_adjmod_1.txt";
    std::string content = "Test for auto adj use.";

    ReCreateFile(filename, content);

    // 2. map file
    MapMode mode = static_cast<MapMode>(1) | static_cast<MapMode>(16) |
                   MapMode::PRIVATE | MapMode::READ_ONLY; // bits out of the scope will be ignored.
    MappedFile mf(filename, mode);
    ASSERT_EQ(mf.Map(), MAPPED_FILE_ERR_OK);

    // 3. check status
    EXPECT_TRUE(mf.IsMapped());
    EXPECT_TRUE(mf.IsNormed());

    // 4. check map-mode
    ASSERT_EQ(MapMode::PRIVATE | MapMode::READ_ONLY, mf.GetMode());

    RemoveTestFile(filename);
}

/*
 * @tc.name: testAutoAdjustedSize001
 * @tc.desc: Test file mapping with size excessing the last page of the file.
 */
HWTEST_F(UtilsMappedFileTest, testAutoAdjustedSize001, TestSize.Level0)
{
    // 1. create a new file
    std::string filename = "test_adjsize_1.txt";
    std::string content = "Test for auto adj use.";

    ReCreateFile(filename, content);

    // 2. map file
    off_t size = 5 * 1024; // Specified size excessing the last page of the file.
    MappedFile mf(filename, MapMode::DEFAULT, 0, size);
    ASSERT_EQ(mf.Map(), MAPPED_FILE_ERR_OK);

    // 3. check status
    EXPECT_TRUE(mf.IsMapped());
    EXPECT_TRUE(mf.IsNormed());

    // 4. check size
    struct stat stb = {0};
    stat(filename.c_str(), &stb);
    off_t max = (stb.st_size / mf.PageSize() + 1LL) * mf.PageSize() - 0LL;
    EXPECT_EQ(mf.Size(), max); // Size will be automatically adjusted, due to safe-concern.

    RemoveTestFile(filename);
}

/*
 * @tc.name: testAutoAdjustedSize002
 * @tc.desc: Test file mapping with size excessing the last page of the file.
 */
HWTEST_F(UtilsMappedFileTest, testAutoAdjustedSize002, TestSize.Level0)
{
    // 1. create a new file
    std::string filename = "test_adjsize_2.txt";
    std::string content = "Test for auto adj use.";

    ReCreateFile(filename, content);

    // 2. Extend size manually
    int fd = open(filename.c_str(), O_RDWR | O_CLOEXEC);
    if (fd != -1) {
        std::cout << "open success." << std::endl;
        ftruncate(fd, 7 * 1024);

        // 3. map file
        off_t offset = 4 * 1024;
        off_t size = 5 * 1024; // Specified size excessing the last page of the file.
        MappedFile mf(filename, MapMode::DEFAULT, offset, size);
        ASSERT_EQ(mf.Map(), MAPPED_FILE_ERR_OK);

        // 4. check status
        EXPECT_TRUE(mf.IsMapped());
        EXPECT_TRUE(mf.IsNormed());

        // 5. check size
        struct stat stb = {0};
        stat(filename.c_str(), &stb);
        off_t max = (stb.st_size / mf.PageSize() + 1LL) * mf.PageSize() - offset;
        EXPECT_EQ(mf.Size(), max); // Size will be automatically adjusted, due to safe-concern.

        close(fd);
    }

    RemoveTestFile(filename);
}

/*
 * @tc.name: testMoveMappedFile001
 * @tc.desc: Test move constructor.
 */
HWTEST_F(UtilsMappedFileTest, testMoveMappedFile001, TestSize.Level0)
{
    // 1. create a new file
    std::string filename = "test_move_1.txt";
    std::string content = "Test for move use.";

    ReCreateFile(filename, content);

    // 2. map file
    MappedFile mf(filename);
    ASSERT_EQ(mf.Map(), MAPPED_FILE_ERR_OK);

    off_t size = mf.Size();
    off_t offset = mf.StartOffset();
    char* data = mf.Begin();
    MapMode mode = mf.GetMode();
    const char* hint = mf.GetHint();

    // 3. move to a new object
    MappedFile mfNew(std::move(mf));

    // 4. check status and params after move
    EXPECT_FALSE(mf.IsMapped());
    EXPECT_FALSE(mf.IsNormed());
    EXPECT_EQ(mf.Begin(), nullptr);
    EXPECT_EQ(mf.Size(), MappedFile::DEFAULT_LENGTH);
    EXPECT_EQ(mf.StartOffset(), 0);
    EXPECT_EQ(mf.GetMode(), MapMode::DEFAULT);
    EXPECT_EQ(mf.GetHint(), nullptr);
    EXPECT_EQ(mf.GetPath(), "");

    EXPECT_TRUE(mfNew.IsMapped());
    EXPECT_TRUE(mfNew.IsNormed());
    EXPECT_EQ(mfNew.Begin(), data);
    EXPECT_EQ(mfNew.Size(), size);
    EXPECT_EQ(mfNew.StartOffset(), offset);
    EXPECT_EQ(mfNew.GetMode(), mode);
    EXPECT_EQ(mfNew.GetHint(), hint);
    EXPECT_EQ(mfNew.GetPath(), filename);

    // 5. read from mapped file
    // write to mapped file
    TestFileWrite(mfNew, filename, content);
}

/*
 * @tc.name: testMoveMappedFile002
 * @tc.desc: Test move constructor with ummapped region.
 */
HWTEST_F(UtilsMappedFileTest, testMoveMappedFile002, TestSize.Level0)
{
    // 1. create a new file
    std::string filename = "test_move_2.txt";
    std::string content = "Test for move use.";

    ReCreateFile(filename, content);

    // 2. map file
    MappedFile mf(filename);
    ASSERT_EQ(mf.Map(), MAPPED_FILE_ERR_OK);

    off_t size = mf.Size();
    off_t offset = mf.StartOffset();
    MapMode mode = mf.GetMode();
    const char* hint = mf.GetHint();

    ASSERT_EQ(mf.Unmap(), MAPPED_FILE_ERR_OK);
    // 3. move to a new object
    MappedFile mfNew(std::move(mf));

    // 4. check status and params after move
    EXPECT_FALSE(mf.IsMapped());
    EXPECT_FALSE(mf.IsNormed());
    EXPECT_EQ(mf.Begin(), nullptr);
    EXPECT_EQ(mf.Size(), MappedFile::DEFAULT_LENGTH);
    EXPECT_EQ(mf.StartOffset(), 0);
    EXPECT_EQ(mf.GetMode(), MapMode::DEFAULT);
    EXPECT_EQ(mf.GetHint(), nullptr);
    EXPECT_EQ(mf.GetPath(), "");

    EXPECT_FALSE(mfNew.IsMapped());
    EXPECT_TRUE(mfNew.IsNormed());
    EXPECT_EQ(mfNew.Begin(), nullptr);
    EXPECT_EQ(mfNew.Size(), size);
    EXPECT_EQ(mfNew.StartOffset(), offset);
    EXPECT_EQ(mfNew.GetMode(), mode);
    EXPECT_EQ(mfNew.GetHint(), hint);
    EXPECT_EQ(mfNew.GetPath(), filename);

    // 5. Map again
    ASSERT_EQ(mfNew.Map(), MAPPED_FILE_ERR_OK);
    // 6. read from mapped file
    // write to mapped file
    TestFileWrite(mfNew, filename, content);
}

/*
 * @tc.name: testMoveMappedFile003
 * @tc.desc: Test move assignment operator overload.
 */
HWTEST_F(UtilsMappedFileTest, testMoveMappedFile003, TestSize.Level0)
{
    // 1. create a new file
    std::string filename = "test_move_3.txt";
    std::string content = "Test for move use.";

    std::string filename1 = "test_move_4.txt";
    std::string content1 = "Test for move use.";

    ReCreateFile(filename, content);
    ReCreateFile(filename1, content1);

    // 2. map file
    MappedFile mf(filename);
    ASSERT_EQ(mf.Map(), MAPPED_FILE_ERR_OK);
    MappedFile mf1(filename1);
    ASSERT_EQ(mf1.Map(), MAPPED_FILE_ERR_OK);

    off_t size = mf1.Size();
    off_t offset = mf1.StartOffset();
    MapMode mode = mf1.GetMode();
    char* data = mf1.Begin();
    const char* hint = mf1.GetHint();

    // 3. move assignment
    mf = std::move(mf1);

    // 4. check status and params after move
    EXPECT_TRUE(mf.IsMapped());
    EXPECT_TRUE(mf.IsNormed());
    EXPECT_EQ(mf.Begin(), data);
    EXPECT_EQ(mf.Size(), size);
    EXPECT_EQ(mf.StartOffset(), offset);
    EXPECT_EQ(mf.GetMode(), mode);
    EXPECT_EQ(mf.GetHint(), hint);
    EXPECT_EQ(mf.GetPath(), filename1);

    // 5. read from mapped file
    // write to mapped file
    TestTwoFileWrite(mf, filename, filename1, content1);
}

/*
 * @tc.name: testMoveMappedFile004
 * @tc.desc: Test move assignment operator overload with ummapped region.
 */
HWTEST_F(UtilsMappedFileTest, testMoveMappedFile004, TestSize.Level0)
{
    // 1. create a new file
    std::string filename = "test_move_4.txt";
    std::string content = "Test for move use.";
    ReCreateFile(filename, content);

    std::string filename1 = "test_move_5.txt";
    std::string content1 = "Test for move use.";
    ReCreateFile(filename1, content1);

    // 2. map file
    MappedFile mf(filename);
    MappedFile mf1(filename1);
    ASSERT_EQ(mf.Map(), MAPPED_FILE_ERR_OK);
    ASSERT_EQ(mf1.Map(), MAPPED_FILE_ERR_OK);

    off_t size = mf1.Size();
    off_t offset = mf1.StartOffset();
    MapMode mode = mf1.GetMode();
    const char* hint = mf1.GetHint();

    // 3. ummap mf1
    ASSERT_EQ(mf1.Unmap(), MAPPED_FILE_ERR_OK);
    // 4. move assignment
    mf = std::move(mf1);
    // 5. check status and params after move
    EXPECT_FALSE(mf.IsMapped());
    EXPECT_TRUE(mf.IsNormed());
    EXPECT_EQ(mf.Begin(), nullptr); // since mf1 is unmapped, its `data_` are set to `nullptr`
    EXPECT_EQ(mf.Size(), size);
    EXPECT_EQ(mf.StartOffset(), offset);
    EXPECT_EQ(mf.GetMode(), mode);
    EXPECT_EQ(mf.GetHint(), hint);
    EXPECT_EQ(mf.GetPath(), filename1);

    ASSERT_EQ(mf.Map(), MAPPED_FILE_ERR_OK);
    // 6. read from mapped file
    // write to mapped file
    TestTwoFileWrite(mf, filename, filename1, content1);
}

}  // namespace
}  // namespace OHOS