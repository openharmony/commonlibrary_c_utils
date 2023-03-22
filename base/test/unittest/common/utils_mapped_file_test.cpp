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

#include <gtest/gtest.h>
#include <sys/stat.h>
#include <unistd.h>
#include <iostream>
#include <fstream>
#include "errors.h"
#include "file_ex.h"
#include "directory_ex.h"
#include "common_mapped_file_errors.h"
#include "mapped_file.h"

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

/*
 * @tc.name: testDefaultMapping001
 * @tc.desc: Test file mapping with default params.
 */
HWTEST_F(UtilsMappedFileTest, testDefaultMapping001, TestSize.Level0)
{
    // 1. Create a new file
    std::string filename = "test_read_write_1.txt";
    std::string content = "Test for normal use.";
    filename.insert(0, SUITE_PATH).insert(0, BASE_PATH);
    RemoveTestFile(filename);

    ASSERT_TRUE(CreateTestFile(filename, content));

    // 2. map file
    MappedFile mf(filename);

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
    std::string readout;
    char* cur = mf.Begin();
    for (; cur <= mf.End(); cur++) {
        readout.push_back(*cur);
    }
    EXPECT_EQ(readout, content);

    // 4. write to mapped file
    std::string toWrite("Complete.");
    char* newCur = mf.Begin();
    for (std::string::size_type i = 0; i < toWrite.length(); i++) {
        (*newCur) = toWrite[i];
        newCur++;
    }
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
    char* cur = mf.Begin();
    for (; *cur != '\0'; cur++) {
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
    char* hint = reinterpret_cast<char*>(0x80000); // new mapping region will not guaranteed to be located at `hint`
    off_t size = 1024;
    off_t offset = 4 * 1024;

    // 1. map a non-existed file
    MappedFile mf(filename, MapMode::DEFAULT | MapMode::CREATE_IF_ABSENT, offset, size, hint);

    // check if file is created
    ASSERT_TRUE(FileExists(filename));

    // check status
    ASSERT_TRUE(mf.IsMapped());
    ASSERT_TRUE(mf.IsNormed());

    // check specified size
    struct stat stb = {0};
    if (stat(filename.c_str(), &stb) == 0) {
        EXPECT_EQ(stb.st_size, offset + size); // Exact file size should be offset + mapped size, contents will be
                                               // zero-filled.
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
    char* cur = mf.Begin();
    for (; cur <= mf.End() && *cur != '\0'; cur++) {
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
    filename.insert(0, SUITE_PATH).insert(0, BASE_PATH);
    RemoveTestFile(filename);

    ASSERT_TRUE(CreateTestFile(filename, content));

    // 2. map file
    MappedFile mf(filename, MapMode::DEFAULT | MapMode::PRIVATE);

    // 3. check status
    ASSERT_TRUE(mf.IsMapped());
    ASSERT_TRUE(mf.IsNormed());

    // 4. read from mapped file
    std::string readout;
    char* cur = mf.Begin();
    for (; cur <= mf.End(); cur++) {
        readout.push_back(*cur);
    }
    EXPECT_EQ(readout, content);

    // 5. write to mapped file
    std::string toWrite("Complete.");
    char* newCur = mf.Begin();
    for (std::string::size_type i = 0; i < toWrite.length(); i++) {
        (*newCur) = toWrite[i];
        newCur++;
    }
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
    filename.insert(0, SUITE_PATH).insert(0, BASE_PATH);
    RemoveTestFile(filename);

    ASSERT_TRUE(CreateTestFile(filename, content));

    // 2. map file
    MappedFile mf(filename, MapMode::DEFAULT | MapMode::READ_ONLY);

    // 3. check status
    ASSERT_TRUE(mf.IsMapped());
    ASSERT_TRUE(mf.IsNormed());

    // 4. read from mapped file
    std::string readout;
    char* cur = mf.Begin();
    for (; cur <= mf.End(); cur++) {
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
    filename.insert(0, SUITE_PATH).insert(0, BASE_PATH);
    RemoveTestFile(filename);

    ASSERT_TRUE(CreateTestFile(filename, content));

    // 2. map file
    MappedFile mf(filename);

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
    filename.insert(0, SUITE_PATH).insert(0, BASE_PATH);
    RemoveTestFile(filename);

    std::string filename1 = "test_remap_1.txt";
    std::string content1 = "Test for remapping use.";
    filename1.insert(0, SUITE_PATH).insert(0, BASE_PATH);
    RemoveTestFile(filename1);

    ASSERT_TRUE(CreateTestFile(filename, content));
    ASSERT_TRUE(CreateTestFile(filename1, content1));

    // 2. map file
    MappedFile mf(filename);

    // 3. check status after mapping
    ASSERT_TRUE(mf.IsMapped());
    ASSERT_TRUE(mf.IsNormed());

    // 4. check size
    struct stat stb = {0};
    stat(filename.c_str(), &stb);
    ASSERT_TRUE(stb.st_size == mf.Size() || mf.PageSize() == mf.Size());

    // 5. read from Mapped File
    std::string readout;
    char* cur = mf.Begin();
    for (; cur <= mf.End(); cur++) {
        readout.push_back(*cur);
    }
    EXPECT_EQ(readout, content);

    // 6. change params
    mf.ChangePath(filename1);
    mf.ChangeSize(MappedFile::DEFAULT_LENGTH);

    // 7. check status after changing
    EXPECT_FALSE(mf.IsMapped());
    EXPECT_FALSE(mf.IsNormed());

    // 8. remap file
    ASSERT_EQ(mf.Map(), MAPPED_FILE_ERR_OK);
    // 9. check status after remapping
    EXPECT_TRUE(mf.IsMapped());
    EXPECT_TRUE(mf.IsNormed());

    // 10. check size
    stat(filename1.c_str(), &stb);
    EXPECT_TRUE(stb.st_size == mf.Size());

    // 11. read from Mapped File
    std::string readout1;
    char* cur1 = mf.Begin();
    for (; cur1 <= mf.End(); cur1++) {
        readout1.push_back(*cur1);
    }
    EXPECT_EQ(readout1, content1);

    RemoveTestFile(filename);
    RemoveTestFile(filename1);
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
    filename.insert(0, SUITE_PATH).insert(0, BASE_PATH);
    RemoveTestFile(filename);

    std::string filename1 = "test_remap_1.txt";
    std::string content1 = "Test for remapping use.";
    filename1.insert(0, SUITE_PATH).insert(0, BASE_PATH);
    RemoveTestFile(filename1);

    ASSERT_TRUE(CreateTestFile(filename, content));
    ASSERT_TRUE(CreateTestFile(filename1, content1));

    // 2. map file
    MappedFile mf(filename);

    // 3. check status after mapping
    ASSERT_TRUE(mf.IsMapped());
    ASSERT_TRUE(mf.IsNormed());

    // 4. check size
    struct stat stb = {0};
    stat(filename.c_str(), &stb);
    ASSERT_TRUE(stb.st_size == mf.Size() || mf.PageSize() == mf.Size());

    // 5. read from Mapped File
    std::string readout;
    char* cur = mf.Begin();
    for (; cur <= mf.End(); cur++) {
        readout.push_back(*cur);
    }
    EXPECT_EQ(readout, content);

    // 6. change params
    mf.ChangePath(filename1);
    mf.ChangeSize(MappedFile::DEFAULT_LENGTH);

    // 7. check status after changing
    EXPECT_FALSE(mf.IsMapped());
    EXPECT_FALSE(mf.IsNormed());

    // 8. remap file
    ASSERT_EQ(mf.Resize(), MAPPED_FILE_ERR_OK);
    // 9. check status after remapping
    EXPECT_TRUE(mf.IsMapped());
    EXPECT_TRUE(mf.IsNormed());

    // 10. check size
    stat(filename1.c_str(), &stb);
    EXPECT_TRUE(stb.st_size == mf.Size());

    // 11. read from Mapped File
    std::string readout1;
    char* cur1 = mf.Begin();
    for (; cur1 <= mf.End(); cur1++) {
        readout1.push_back(*cur1);
    }
    EXPECT_EQ(readout1, content1);

    RemoveTestFile(filename);
    RemoveTestFile(filename1);
}

/*
 * @tc.name: testReMap004
 * @tc.desc: Test remapping only to extend mapped region via Resize(off_t, bool).
 */
HWTEST_F(UtilsMappedFileTest, testReMap004, TestSize.Level0)
{
    // 1. create a new file
    std::string filename = "test_remap.txt";
    std::string content = "Test for remapping use.";
    filename.insert(0, SUITE_PATH).insert(0, BASE_PATH);
    RemoveTestFile(filename);

    ASSERT_TRUE(CreateTestFile(filename, content));

    // 2. map file
    MappedFile mf(filename);

    // 3. check status after mapping
    ASSERT_TRUE(mf.IsMapped());
    ASSERT_TRUE(mf.IsNormed());

    // 4. check size
    struct stat stb = {0};
    stat(filename.c_str(), &stb);
    ASSERT_TRUE(stb.st_size == mf.Size() || mf.PageSize() == mf.Size());

    // 5. read from Mapped File
    std::string readout;
    char* cur = mf.Begin();
    for (; cur <= mf.End(); cur++) {
        readout.push_back(*cur);
    }
    EXPECT_EQ(readout, content);

    // 6. Remap to extend region
    ASSERT_EQ(mf.Resize(mf.Size() + 10), MAPPED_FILE_ERR_OK);
    // 7. check status after remapping
    EXPECT_TRUE(mf.IsMapped());
    EXPECT_TRUE(mf.IsNormed());

    // 8. check size after remapping
    stat(filename.c_str(), &stb);
    EXPECT_TRUE(stb.st_size < mf.Size());

    // 9. write to the extended region
    *(cur) = 'E';
    EXPECT_EQ((*cur), 'E');

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
    filename.insert(0, SUITE_PATH).insert(0, BASE_PATH);
    RemoveTestFile(filename);

    ASSERT_TRUE(CreateTestFile(filename, content));

    // 2. map file
    MappedFile mf(filename);

    // 3. check status after mapping
    ASSERT_TRUE(mf.IsMapped());
    ASSERT_TRUE(mf.IsNormed());

    // 4. check size
    struct stat stb = {0};
    stat(filename.c_str(), &stb);
    ASSERT_TRUE(stb.st_size == mf.Size() || mf.PageSize() == mf.Size());

    // 5. read from Mapped File
    std::string readout;
    char* cur = mf.Begin();
    for (; cur <= mf.End(); cur++) {
        readout.push_back(*cur);
    }
    EXPECT_EQ(readout, content);

    // 6. remap to extend region
    ASSERT_EQ(mf.Resize(mf.Size() + 10, true), MAPPED_FILE_ERR_OK);
    // check status after remapping
    EXPECT_TRUE(mf.IsMapped());
    EXPECT_TRUE(mf.IsNormed());

    // 7. check size after remapping
    stat(filename.c_str(), &stb);
    EXPECT_TRUE(stb.st_size == mf.Size()); // File size will sync to that of the mapped region.

    // 8. write to the extended region
    *(cur) = 'E';
    EXPECT_EQ((*cur), 'E');

    std::string res;
    LoadStringFromFile(filename, res);
    EXPECT_STREQ(res.c_str(), content.append("E").c_str()); // Changes will be sync in the original file.
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
    filename.insert(0, SUITE_PATH).insert(0, BASE_PATH);
    RemoveTestFile(filename);

    ASSERT_TRUE(CreateTestFile(filename, content));

    struct stat stb = {0};
    ASSERT_EQ(stat(filename.c_str(), &stb), 0);
    off_t orig = stb.st_size; // 25 bit

    // 2. extend its size
    int fd = open(filename.c_str(), O_RDWR | O_CLOEXEC);
    ASSERT_NE(fd, -1);
    ASSERT_EQ(ftruncate(fd, MappedFile::PageSize() + MappedFile::PageSize() / 100LL), 0);

    // 3. map file
    MappedFile mf(filename, MapMode::DEFAULT, 0, orig);

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

    off_t endOff;
    // 6. keep turnNext within a page
    for (unsigned int cnt = 2; cnt < (MappedFile::PageSize() / orig); cnt++) {
        endOff = mf.EndOffset();
        EXPECT_EQ(mf.TurnNext(), MAPPED_FILE_ERR_OK);
        EXPECT_EQ(mf.StartOffset(), endOff + 1);
        EXPECT_EQ(mf.Size(), orig);
    }
    std::cout << "==Last TurnNext() with The Same Size==" << std::endl;
    PrintStatus(mf);

    // 7. this turn will reach the bottom of a page
    endOff = mf.EndOffset();
    char* rEnd = mf.RegionEnd();
    char* end = mf.End();
    EXPECT_EQ(mf.TurnNext(), MAPPED_FILE_ERR_OK);
    EXPECT_EQ(mf.StartOffset(), endOff + 1);
    EXPECT_EQ(mf.Size(), static_cast<off_t>(rEnd - end));
    std::cout << "==Reached Bottom of A Page==" << std::endl;
    PrintStatus(mf);

    // 8. this turn will trigger a remapping
    endOff = mf.EndOffset();
    off_t curSize = mf.Size();
    EXPECT_EQ(mf.TurnNext(), MAPPED_FILE_ERR_OK);
    EXPECT_TRUE(mf.IsMapped());
    EXPECT_EQ(mf.StartOffset(), endOff + 1);
    EXPECT_EQ(mf.Size(), curSize);
    EXPECT_EQ(mf.RegionStart(), mf.Begin());
    EXPECT_EQ(static_cast<off_t>(mf.RegionEnd() - mf.RegionStart()) + 1LL, mf.PageSize());
    std::cout << "==Remap A New Page==" << std::endl;
    PrintStatus(mf);

    // 9. keep turnNext within a page
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
    filename.insert(0, SUITE_PATH).insert(0, BASE_PATH);
    RemoveTestFile(filename);

    ASSERT_TRUE(CreateTestFile(filename, content));

    // 2. map file
    MappedFile mf(filename);
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
    std::string filename = "test_remap.txt";
    std::string content = "Test for remapping use.";
    filename.insert(0, SUITE_PATH).insert(0, BASE_PATH);
    RemoveTestFile(filename);

    ASSERT_TRUE(CreateTestFile(filename, content));

    // 2. map file
    MappedFile mf(filename);

    // 3. check status after mapping
    ASSERT_TRUE(mf.IsMapped());
    ASSERT_TRUE(mf.IsNormed());

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
    filename.insert(0, SUITE_PATH).insert(0, BASE_PATH);
    RemoveTestFile(filename);

    ASSERT_TRUE(CreateTestFile(filename, content));

    // 2. map file
    MappedFile mf(filename);

    // 3. check status after mapping
    ASSERT_TRUE(mf.IsMapped());
    ASSERT_TRUE(mf.IsNormed());

    // 4. turn next mapped region with the same size as the file's initial size.
    EXPECT_EQ(mf.TurnNext(), ERR_INVALID_OPERATION);

    RemoveTestFile(filename);
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
    filename.insert(0, SUITE_PATH).insert(0, BASE_PATH);
    RemoveTestFile(filename);

    ASSERT_TRUE(CreateTestFile(filename, content));

    // 2. map file
    off_t offset = 100; // Specify offset that is not multiple of page-size.
    MappedFile mf(filename, MapMode::DEFAULT, offset);

    // 3. check status
    EXPECT_FALSE(mf.IsMapped());
    EXPECT_FALSE(mf.IsNormed()); // mapping will fail in normalize stage.

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
    filename.insert(0, SUITE_PATH).insert(0, BASE_PATH);
    RemoveTestFile(filename);

    ASSERT_TRUE(CreateTestFile(filename, content));

    // 2. map file
    off_t offset = 4 * 1024; // Specify offset excessing the substantial size of the file.
    MappedFile mf(filename, MapMode::DEFAULT, offset);

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

    // 3. check status
    EXPECT_FALSE(mf.IsMapped());
    EXPECT_FALSE(mf.IsNormed()); // mapping will fail in normalize stage.

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
    filename.insert(0, SUITE_PATH).insert(0, BASE_PATH);
    RemoveTestFile(filename);

    ASSERT_TRUE(CreateTestFile(filename, content));

    // 2. map file
    MapMode mode = static_cast<MapMode>(1) | static_cast<MapMode>(16) |
                   MapMode::PRIVATE | MapMode::READ_ONLY; // bits out of the scope will be ignored.
    MappedFile mf(filename, mode);

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
    filename.insert(0, SUITE_PATH).insert(0, BASE_PATH);
    RemoveTestFile(filename);

    ASSERT_TRUE(CreateTestFile(filename, content));

    // 2. map file
    off_t size = 5 * 1024; // Specified size excessing the last page of the file.
    MappedFile mf(filename, MapMode::DEFAULT, 0, size);

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
    filename.insert(0, SUITE_PATH).insert(0, BASE_PATH);
    RemoveTestFile(filename);

    ASSERT_TRUE(CreateTestFile(filename, content));

    // 2. Extend size manually
    int fd = open(filename.c_str(), O_RDWR | O_CLOEXEC);
    if (fd != -1) {
        std::cout << "open success." << std::endl;
        ftruncate(fd, 7 * 1024);

        // 3. map file
        off_t offset = 4 * 1024;
        off_t size = 5 * 1024; // Specified size excessing the last page of the file.
        MappedFile mf(filename, MapMode::DEFAULT, offset, size);

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
    filename.insert(0, SUITE_PATH).insert(0, BASE_PATH);
    RemoveTestFile(filename);

    ASSERT_TRUE(CreateTestFile(filename, content));

    // 2. map file
    MappedFile mf(filename);

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
    std::string readout;
    char* cur = mfNew.Begin();
    for (; cur <= mfNew.End(); cur++) {
        readout.push_back(*cur);
    }
    EXPECT_EQ(readout, content);

    // 6. write to mapped file
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

/*
 * @tc.name: testMoveMappedFile002
 * @tc.desc: Test move constructor with ummapped region.
 */
HWTEST_F(UtilsMappedFileTest, testMoveMappedFile002, TestSize.Level0)
{
    // 1. create a new file
    std::string filename = "test_move_2.txt";
    std::string content = "Test for move use.";
    filename.insert(0, SUITE_PATH).insert(0, BASE_PATH);
    RemoveTestFile(filename);

    ASSERT_TRUE(CreateTestFile(filename, content));

    // 2. map file
    MappedFile mf(filename);

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
    std::string readout;
    char* cur = mfNew.Begin();
    for (; cur <= mfNew.End(); cur++) {
        readout.push_back(*cur);
    }
    EXPECT_EQ(readout, content);

    // 7. write to mapped file
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

/*
 * @tc.name: testMoveMappedFile003
 * @tc.desc: Test move assignment operator overload.
 */
HWTEST_F(UtilsMappedFileTest, testMoveMappedFile003, TestSize.Level0)
{
    // 1. create a new file
    std::string filename = "test_move_3.txt";
    std::string content = "Test for move use.";
    filename.insert(0, SUITE_PATH).insert(0, BASE_PATH);
    RemoveTestFile(filename);

    std::string filename1 = "test_move_4.txt";
    std::string content1 = "Test for move use.";
    filename1.insert(0, SUITE_PATH).insert(0, BASE_PATH);
    RemoveTestFile(filename1);

    ASSERT_TRUE(CreateTestFile(filename, content));
    ASSERT_TRUE(CreateTestFile(filename1, content1));

    // 2. map file
    MappedFile mf(filename);
    MappedFile mf1(filename1);

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
    std::string readout;
    char* cur = mf.Begin();
    for (; cur <= mf.End(); cur++) {
        readout.push_back(*cur);
    }
    EXPECT_EQ(readout, content1);

    // 6. write to mapped file
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
 * @tc.name: testMoveMappedFile004
 * @tc.desc: Test move assignment operator overload with ummapped region.
 */
HWTEST_F(UtilsMappedFileTest, testMoveMappedFile004, TestSize.Level0)
{
    // 1. create a new file
    std::string filename = "test_move_4.txt";
    std::string content = "Test for move use.";
    filename.insert(0, SUITE_PATH).insert(0, BASE_PATH);
    RemoveTestFile(filename);

    std::string filename1 = "test_move_5.txt";
    std::string content1 = "Test for move use.";
    filename1.insert(0, SUITE_PATH).insert(0, BASE_PATH);
    RemoveTestFile(filename1);

    ASSERT_TRUE(CreateTestFile(filename, content));
    ASSERT_TRUE(CreateTestFile(filename1, content1));

    // 2. map file
    MappedFile mf(filename);
    MappedFile mf1(filename1);

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
    std::string readout;
    char* cur = mf.Begin();
    for (; cur <= mf.End(); cur++) {
        readout.push_back(*cur);
    }
    EXPECT_EQ(readout, content1);

    // 7. write to mapped file
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


}  // namespace
}  // namespace OHOS