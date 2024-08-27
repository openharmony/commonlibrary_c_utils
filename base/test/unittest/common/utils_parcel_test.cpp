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
#include <fstream>
#include <iostream>
#include "directory_ex.h"
#include "parcel.h"
#include "refbase.h"
#include "securec.h"
using namespace testing::ext;
using namespace std;

namespace OHOS {
namespace {
const int MAX_PARCEL_SIZE = 1000;
char g_data[MAX_PARCEL_SIZE];
class UtilsParcelTest : public testing::Test {
public:
    static constexpr size_t DEFAULT_CPACITY = 204800; // 200K
    static constexpr size_t CAPACITY_THRESHOLD = 4096; // 4k
    static void TearDownTestCase(void);
};

void UtilsParcelTest::TearDownTestCase(void)
{
    for (int i = 0; i < MAX_PARCEL_SIZE; i++) {
        g_data[i] = 0;
    }
}

class RemoteObject : public virtual Parcelable {
public:
    RemoteObject() { asRemote_ = true; };
    bool Marshalling(Parcel &parcel) const override;
    static sptr<RemoteObject> Unmarshalling(Parcel &parcel);
};

bool RemoteObject::Marshalling(Parcel &parcel) const
{
    parcel_flat_binder_object flat;
    flat.hdr.type = 0xff;
    flat.flags = 0x7f;
    flat.binder = 0;
    flat.handle = (uint32_t)(-1);
    flat.cookie = reinterpret_cast<uintptr_t>(this);
    bool status = parcel.WriteBuffer(&flat, sizeof(parcel_flat_binder_object));
    if (!status) {
        return false;
    }
    return true;
}

sptr<RemoteObject> RemoteObject::Unmarshalling(Parcel &parcel)
{
    const uint8_t *buffer = parcel.ReadBuffer(sizeof(parcel_flat_binder_object), false);
    if (buffer == nullptr) {
        return nullptr;
    }
    sptr<RemoteObject> obj = new RemoteObject();
    return obj;
}

/*-------------------------------base data------------------------------------*/

struct TestData {
    bool booltest;
    int8_t int8test;
    int16_t int16test;
    int32_t int32test;
    uint8_t uint8test;
    uint16_t uint16test;
    uint32_t uint32test;
};

void WriteTestData(Parcel &parcel, const struct TestData &data)
{
    bool result = false;

    result = parcel.WriteBool(data.booltest);
    EXPECT_EQ(result, true);

    result = parcel.WriteInt8(data.int8test);
    EXPECT_EQ(result, true);

    result = parcel.WriteInt16(data.int16test);
    EXPECT_EQ(result, true);

    result = parcel.WriteInt32(data.int32test);
    EXPECT_EQ(result, true);

    result = parcel.WriteUint8(data.uint8test);
    EXPECT_EQ(result, true);

    result = parcel.WriteUint16(data.uint16test);
    EXPECT_EQ(result, true);

    result = parcel.WriteUint32(data.uint32test);
    EXPECT_EQ(result, true);
}

void WriteUnalignedTestData(Parcel &parcel, const struct TestData &data)
{
    bool result = false;

    result = parcel.WriteBoolUnaligned(data.booltest);
    EXPECT_EQ(result, true);

    result = parcel.WriteInt8Unaligned(data.int8test);
    EXPECT_EQ(result, true);

    result = parcel.WriteInt16Unaligned(data.int16test);
    EXPECT_EQ(result, true);

    result = parcel.WriteUint8Unaligned(data.uint8test);
    EXPECT_EQ(result, true);

    result = parcel.WriteUint16Unaligned(data.uint16test);
    EXPECT_EQ(result, true);
}

void ReadTestData(Parcel &parcel, const struct TestData &data)
{
    bool readbool = parcel.ReadBool();
    EXPECT_EQ(readbool, data.booltest);

    int8_t readint8 = parcel.ReadInt8();
    EXPECT_EQ(readint8, data.int8test);

    int16_t readint16 = parcel.ReadInt16();
    EXPECT_EQ(readint16, data.int16test);

    int32_t readint32 = parcel.ReadInt32();
    EXPECT_EQ(readint32, data.int32test);

    uint8_t readuint8 = parcel.ReadUint8();
    EXPECT_EQ(readuint8, data.uint8test);

    uint16_t readuint16 = parcel.ReadUint16();
    EXPECT_EQ(readuint16, data.uint16test);

    uint32_t readuint32 = parcel.ReadUint32();
    EXPECT_EQ(readuint32, data.uint32test);
}

void ReadUnalignedTestData(Parcel &parcel, const struct TestData &data)
{
    bool readbool = parcel.ReadBoolUnaligned();
    EXPECT_EQ(readbool, data.booltest);

    int8_t readint8;
    EXPECT_TRUE(parcel.ReadInt8Unaligned(readint8));
    EXPECT_EQ(readint8, data.int8test);

    int16_t readint16;
    EXPECT_TRUE(parcel.ReadInt16Unaligned(readint16));
    EXPECT_EQ(readint16, data.int16test);

    uint8_t readuint8;
    EXPECT_TRUE(parcel.ReadUint8Unaligned(readuint8));
    EXPECT_EQ(readuint8, data.uint8test);

    uint16_t readuint16;
    EXPECT_TRUE(parcel.ReadUint16Unaligned(readuint16));
    EXPECT_EQ(readuint16, data.uint16test);
}

void ReadTestDataWithTarget(Parcel &parcel, const struct TestData &data)
{
    bool result = false;
    bool boolVal = true;
    result = parcel.ReadBool(boolVal);
    EXPECT_EQ(result, true);
    EXPECT_EQ(boolVal, data.booltest);

    int8_t int8Val;
    result = parcel.ReadInt8(int8Val);
    EXPECT_EQ(result, true);
    EXPECT_EQ(int8Val, data.int8test);

    int16_t int16Val;
    result = parcel.ReadInt16(int16Val);
    EXPECT_EQ(result, true);
    EXPECT_EQ(int16Val, data.int16test);

    int32_t int32Val;
    result = parcel.ReadInt32(int32Val);
    EXPECT_EQ(result, true);
    EXPECT_EQ(int32Val, data.int32test);

    uint8_t uint8Val;
    result = parcel.ReadUint8(uint8Val);
    EXPECT_EQ(result, true);
    EXPECT_EQ(uint8Val, data.uint8test);

    uint16_t uint16Val;
    result = parcel.ReadUint16(uint16Val);
    EXPECT_EQ(result, true);
    EXPECT_EQ(uint16Val, data.uint16test);

    uint32_t uint32Val;
    result = parcel.ReadUint32(uint32Val);
    EXPECT_EQ(result, true);
    EXPECT_EQ(uint32Val, data.uint32test);
}

/**
 * Here to simulate the scenario of ipc sending data, the buffer will be released when the Parcel object is destructed.
*/
bool SendData(void *&buffer, size_t size, const uint8_t *data)
{
    if (size <= 0) {
        return false;
    }
    buffer = malloc(size);
    if (buffer == nullptr) {
        return false;
    }
    if (memcpy_s(buffer, size, data, size) != EOK) {
        return false;
    }
    return true;
}

/**
 * @tc.name: test_parcel_001
 * @tc.desc: test parcel CheckOffsets, WriteRemoteObject, RewindRead and
 * RewindWrite failed.
 * @tc.type: FUNC
 */
HWTEST_F(UtilsParcelTest, test_parcel_001, TestSize.Level0)
{
    Parcel parcel;
    bool result = parcel.CheckOffsets();
    EXPECT_EQ(result, false);
    result = parcel.WriteRemoteObject(nullptr);
    EXPECT_EQ(result, false);
    result = parcel.RewindRead(parcel.GetDataSize() + 1);
    EXPECT_EQ(result, false);
    result = parcel.RewindWrite(parcel.GetDataSize() + 1);
    EXPECT_EQ(result, false);
}

/**
 * @tc.name: test_parcel_readvec_001
 * @tc.desc: test parcel read vector failed with invlalid input.
 * @tc.type: FUNC
 */
HWTEST_F(UtilsParcelTest, test_parcel_readvec_001, TestSize.Level0)
{
    Parcel parcel;

    bool result = parcel.ReadBoolVector(nullptr);
    EXPECT_EQ(result, false);

    result = parcel.ReadInt8Vector(nullptr);
    EXPECT_EQ(result, false);

    result = parcel.ReadInt16Vector(nullptr);
    EXPECT_EQ(result, false);

    result = parcel.ReadInt32Vector(nullptr);
    EXPECT_EQ(result, false);

    result = parcel.ReadInt64Vector(nullptr);
    EXPECT_EQ(result, false);

    result = parcel.ReadUInt8Vector(nullptr);
    EXPECT_EQ(result, false);

    result = parcel.ReadUInt16Vector(nullptr);
    EXPECT_EQ(result, false);

    result = parcel.ReadUInt32Vector(nullptr);
    EXPECT_EQ(result, false);

    result = parcel.ReadUInt64Vector(nullptr);
    EXPECT_EQ(result, false);

    result = parcel.ReadFloatVector(nullptr);
    EXPECT_EQ(result, false);

    result = parcel.ReadDoubleVector(nullptr);
    EXPECT_EQ(result, false);

    result = parcel.ReadStringVector(nullptr);
    EXPECT_EQ(result, false);

    result = parcel.ReadString16Vector(nullptr);
    EXPECT_EQ(result, false);
}

static void ReadvecTestTwoFunc01()
{
    Parcel parcel1;
    parcel1.WriteInt32(-1);
    std::vector<bool> val1;
    bool x1 = true;
    val1.push_back(x1);
    bool result = parcel1.ReadBoolVector(&val1);
    EXPECT_EQ(result, false);

    Parcel parcel2;
    parcel2.WriteInt32(-1);
    std::vector<int8_t> val2;
    int8_t x2 = 1;
    val2.push_back(x2);
    result = parcel2.ReadInt8Vector(&val2);
    EXPECT_EQ(result, false);

    Parcel parcel3;
    parcel3.WriteInt32(-1);
    std::vector<int16_t> val3;
    int16_t x3 = 1;
    val3.push_back(x3);
    result = parcel3.ReadInt16Vector(&val3);
    EXPECT_EQ(result, false);

    Parcel parcel4;
    parcel4.WriteInt32(-1);
    std::vector<int32_t> val4;
    int32_t x4 = 1;
    val4.push_back(x4);
    result = parcel4.ReadInt32Vector(&val4);
    EXPECT_EQ(result, false);
}

static void ReadvecTestTwoFunc02()
{
    Parcel parcel1;
    parcel1.WriteInt32(-1);
    std::vector<int64_t> val1;
    int64_t x1 = 1;
    val1.push_back(x1);
    bool result = parcel1.ReadInt64Vector(&val1);
    EXPECT_EQ(result, false);

    Parcel parcel2;
    parcel2.WriteInt32(-1);
    std::vector<uint8_t> val2;
    uint8_t x2 = 1;
    val2.push_back(x2);
    result = parcel2.ReadUInt8Vector(&val2);
    EXPECT_EQ(result, false);

    Parcel parcel3;
    parcel3.WriteInt32(-1);
    std::vector<uint16_t> val3;
    uint16_t x3 = 1;
    val3.push_back(x3);
    result = parcel3.ReadUInt16Vector(&val3);
    EXPECT_EQ(result, false);

    Parcel parcel4;
    parcel4.WriteInt32(-1);
    std::vector<uint32_t> val4;
    uint32_t x4 = 1;
    val4.push_back(x4);
    result = parcel4.ReadUInt32Vector(&val4);
    EXPECT_EQ(result, false);
}

static void ReadvecTestTwoFunc03()
{
    Parcel parcel1;
    parcel1.WriteInt32(-1);
    std::vector<uint64_t> val1;
    uint64_t x1 = 1;
    val1.push_back(x1);
    bool result = parcel1.ReadUInt64Vector(&val1);
    EXPECT_EQ(result, false);

    Parcel parcel2;
    parcel2.WriteInt32(-1);
    std::vector<float> val2;
    float x2 = 1;
    val2.push_back(x2);
    result = parcel2.ReadFloatVector(&val2);
    EXPECT_EQ(result, false);

    Parcel parcel3;
    parcel3.WriteInt32(-1);
    std::vector<double> val3;
    double x3 = 1;
    val3.push_back(x3);
    result = parcel3.ReadDoubleVector(&val3);
    EXPECT_EQ(result, false);

    Parcel parcel4;
    parcel4.WriteInt32(-1);
    std::vector<std::string> val4;
    std::string x4 = "test";
    val4.push_back(x4);
    result = parcel4.ReadStringVector(&val4);
    EXPECT_EQ(result, false);

    Parcel parcel5;
    parcel5.WriteInt32(-1);
    std::vector<std::u16string> val5;
    std::u16string x5 = u"test";
    val5.push_back(x5);
    result = parcel5.ReadString16Vector(&val5);
    EXPECT_EQ(result, false);
}

/**
 * @tc.name: test_parcel_readvec_002
 * @tc.desc: test parcel read vector failed with invlalid vector length -1.
 * @tc.type: FUNC
 */
HWTEST_F(UtilsParcelTest, test_parcel_readvec_002, TestSize.Level0)
{
    ReadvecTestTwoFunc01();
    ReadvecTestTwoFunc02();
    ReadvecTestTwoFunc03();
}

static void ReadvecTestThreeFunc01()
{
    Parcel parcel1;
    std::vector<bool> val1;
    bool x1 = true;
    val1.push_back(x1);
    parcel1.WriteInt32(val1.max_size());
    bool result = parcel1.ReadBoolVector(&val1);
    EXPECT_EQ(result, false);

    Parcel parcel2;
    std::vector<int8_t> val2;
    int8_t x2 = 1;
    val2.push_back(x2);
    parcel2.WriteInt32(val2.max_size());
    result = parcel2.ReadInt8Vector(&val2);
    EXPECT_EQ(result, false);

    Parcel parcel3;
    std::vector<int16_t> val3;
    int16_t x3 = 1;
    val3.push_back(x3);
    parcel3.WriteInt32(val3.max_size());
    result = parcel3.ReadInt16Vector(&val3);
    EXPECT_EQ(result, false);

    Parcel parcel4;
    std::vector<int32_t> val4;
    int32_t x4 = 1;
    val4.push_back(x4);
    parcel4.WriteInt32(val4.max_size());
    result = parcel4.ReadInt32Vector(&val4);
    EXPECT_EQ(result, false);
}

static void ReadvecTestThreeFunc02()
{
    Parcel parcel1;
    std::vector<int64_t> val1;
    int64_t x1 = 1;
    val1.push_back(x1);
    parcel1.WriteInt32(val1.max_size());
    bool result = parcel1.ReadInt64Vector(&val1);
    EXPECT_EQ(result, false);

    Parcel parcel2;
    std::vector<uint8_t> val2;
    uint8_t x2 = 1;
    val2.push_back(x2);
    parcel2.WriteInt32(val2.max_size());
    result = parcel2.ReadUInt8Vector(&val2);
    EXPECT_EQ(result, false);

    Parcel parcel3;
    std::vector<uint16_t> val3;
    uint16_t x3 = 1;
    val3.push_back(x3);
    parcel3.WriteInt32(val3.max_size());
    result = parcel3.ReadUInt16Vector(&val3);
    EXPECT_EQ(result, false);

    Parcel parcel4;
    std::vector<uint32_t> val4;
    uint32_t x4 = 1;
    val4.push_back(x4);
    parcel4.WriteInt32(val4.max_size());
    result = parcel4.ReadUInt32Vector(&val4);
    EXPECT_EQ(result, false);
}

static void ReadvecTestThreeFunc03()
{
    Parcel parcel1;
    std::vector<uint64_t> val1;
    uint64_t x1 = 1;
    val1.push_back(x1);
    parcel1.WriteInt32(val1.max_size());
    bool result = parcel1.ReadUInt64Vector(&val1);
    EXPECT_EQ(result, false);

    Parcel parcel2;
    std::vector<float> val2;
    float x2 = 1;
    val2.push_back(x2);
    parcel2.WriteInt32(val2.max_size());
    result = parcel2.ReadFloatVector(&val2);
    EXPECT_EQ(result, false);

    Parcel parcel3;
    std::vector<double> val3;
    double x3 = 1;
    val3.push_back(x3);
    parcel3.WriteInt32(val3.max_size());
    result = parcel3.ReadDoubleVector(&val3);
    EXPECT_EQ(result, false);

    Parcel parcel4;
    std::vector<std::string> val4;
    std::string x4 = "test";
    val4.push_back(x4);
    parcel4.WriteInt32(val4.max_size());
    result = parcel4.ReadStringVector(&val4);
    EXPECT_EQ(result, false);

    Parcel parcel5;
    std::vector<std::u16string> val5;
    std::u16string x5 = u"test";
    val5.push_back(x5);
    parcel5.WriteInt32(val5.max_size());
    result = parcel5.ReadString16Vector(&val5);
    EXPECT_EQ(result, false);
}

/**
 * @tc.name: test_parcel_readvec_003
 * @tc.desc: test parcel read vector failed with invlalid vector length
 * std::vector::max_size().
 * @tc.type: FUNC
 */
HWTEST_F(UtilsParcelTest, test_parcel_readvec_003, TestSize.Level0)
{
    ReadvecTestThreeFunc01();
    ReadvecTestThreeFunc02();
    ReadvecTestThreeFunc03();
}

static void WritevecTestOneFunc01(const size_t cap)
{
    Parcel parcel(nullptr);
    std::vector<bool> val1;
    bool x1 = true;
    for (int i = 0; i < cap / sizeof(bool); i++) {
        val1.push_back(x1);
    }
    bool result = parcel.WriteBoolVector(val1);
    EXPECT_EQ(result, false);

    parcel.FlushBuffer();
    std::vector<int8_t> val2;
    int8_t x2 = 1;
    for (int i = 0; i < cap / sizeof(int8_t); i++) {
        val2.push_back(x2);
    }
    result = parcel.WriteInt8Vector(val2);
    EXPECT_EQ(result, false);

    parcel.FlushBuffer();
    std::vector<int16_t> val3;
    int16_t x3 = 1;
    for (int i = 0; i < cap / sizeof(int16_t); i++) {
        val3.push_back(x3);
    }
    result = parcel.WriteInt16Vector(val3);
    EXPECT_EQ(result, false);

    parcel.FlushBuffer();
    std::vector<int32_t> val4;
    int32_t x4 = 1;
    for (int i = 0; i < cap / sizeof(int32_t); i++) {
        val4.push_back(x4);
    }
    result = parcel.WriteInt32Vector(val4);
    EXPECT_EQ(result, false);
}

static void WritevecTestOneFunc02(const size_t cap)
{
    Parcel parcel(nullptr);
    std::vector<int64_t> val1;
    int64_t x1 = 1;
    for (int i = 0; i < cap / sizeof(int64_t); i++) {
        val1.push_back(x1);
    }
    bool result = parcel.WriteInt64Vector(val1);
    EXPECT_EQ(result, false);

    parcel.FlushBuffer();
    std::vector<uint8_t> val2;
    uint8_t x2 = 1;
    for (int i = 0; i < cap / sizeof(uint8_t); i++) {
        val2.push_back(x2);
    }
    result = parcel.WriteUInt8Vector(val2);
    EXPECT_EQ(result, false);

    parcel.FlushBuffer();
    std::vector<uint16_t> val3;
    uint16_t x3 = 1;
    for (int i = 0; i < cap / sizeof(uint16_t); i++) {
        val3.push_back(x3);
    }
    result = parcel.WriteUInt16Vector(val3);
    EXPECT_EQ(result, false);

    parcel.FlushBuffer();
    std::vector<uint32_t> val4;
    uint32_t x4 = 1;
    for (int i = 0; i < cap / sizeof(uint32_t); i++) {
        val4.push_back(x4);
    }
    result = parcel.WriteUInt32Vector(val4);
    EXPECT_EQ(result, false);
}

static void WritevecTestOneFunc03(const size_t cap)
{
    Parcel parcel(nullptr);
    std::vector<uint64_t> val1;
    uint64_t x1 = 1;
    for (int i = 0; i < cap / sizeof(uint64_t); i++) {
        val1.push_back(x1);
    }
    bool result = parcel.WriteUInt64Vector(val1);
    EXPECT_EQ(result, false);

    parcel.FlushBuffer();
    std::vector<float> val2;
    float x2 = 1;
    for (int i = 0; i < cap / sizeof(float); i++) {
        val2.push_back(x2);
    }
    result = parcel.WriteFloatVector(val2);
    EXPECT_EQ(result, false);

    parcel.FlushBuffer();
    std::vector<double> val3;
    double x3 = 1;
    for (int i = 0; i < cap / sizeof(double); i++) {
        val3.push_back(x3);
    }
    result = parcel.WriteDoubleVector(val3);
    EXPECT_EQ(result, false);

    parcel.FlushBuffer();
    std::vector<std::string> val4;
    std::string x4((cap / sizeof(char)), 't');
    val4.push_back(x4);
    result = parcel.WriteStringVector(val4);
    EXPECT_EQ(result, false);

    parcel.FlushBuffer();
    std::vector<std::u16string> val5;
    std::u16string x5((cap / sizeof(char16_t)), u't');
    val5.push_back(x5);
    result = parcel.WriteString16Vector(val5);
    EXPECT_EQ(result, false);
}

/**
 * @tc.name: test_parcel_writevec_001
 * @tc.desc: test parcel write vector failed with writting data out of the
 * maximum capacity.
 * @tc.type: FUNC
 */
HWTEST_F(UtilsParcelTest, test_parcel_writevec_001, TestSize.Level0)
{
    size_t cap = DEFAULT_CPACITY;

    WritevecTestOneFunc01(cap);
    WritevecTestOneFunc02(cap);
    WritevecTestOneFunc03(cap);
}

/**
 * @tc.name: test_parcel_SetMaxCapacity_001
 * @tc.desc: test parcel primary type read write.
 * @tc.type: FUNC
 */
HWTEST_F(UtilsParcelTest, test_parcel_SetMaxCapacity_001, TestSize.Level0)
{
    size_t cap = DEFAULT_CPACITY;
    Parcel parcel(nullptr);
    EXPECT_TRUE(parcel.SetMaxCapacity(cap + 1));
    EXPECT_FALSE(parcel.SetMaxCapacity(cap - 1));
}

/**
 * @tc.name: test_parcel_SetAllocator_001
 * @tc.desc: test setting allocator to parcels with and without existed allocator.
 * @tc.type: FUNC
 */
HWTEST_F(UtilsParcelTest, test_parcel_SetAllocator_001, TestSize.Level0)
{
    Allocator* alloc = new DefaultAllocator();
    Parcel parcel(alloc);
    EXPECT_FALSE(parcel.SetAllocator(alloc));
    EXPECT_FALSE(parcel.SetAllocator(nullptr));

    struct TestData data = { true, -0x34, 0x5634, -0x12345678, 0x34, 0x5634, 0x12345678 };

    WriteTestData(parcel, data);
    parcel.SetAllocator(new DefaultAllocator());
    ReadTestData(parcel, data);
}

/**
 * @tc.name: test_parcel_write_001
 * @tc.desc: test parcel write failed.
 * @tc.type: FUNC
 */
HWTEST_F(UtilsParcelTest, test_parcel_write_001, TestSize.Level0)
{
    Parcel parcel1;
    parcel1.WriteBool(true);
    Parcel parcel2;
    void *buffer = nullptr;
    size_t size = parcel1.GetDataSize();
    if (!SendData(buffer, size, reinterpret_cast<const uint8_t *>(parcel1.GetData()))) {
        ASSERT_FALSE(false);
    }
    parcel2.ParseFrom(reinterpret_cast<uintptr_t>(buffer), parcel1.GetDataSize());

    string str8write;
    bool result = parcel2.WriteString(str8write);
    EXPECT_EQ(result, false);

    u16string str16Write;
    result = parcel2.WriteString16(str16Write);
    EXPECT_EQ(result, false);

    result = parcel2.WriteBool(false);
    EXPECT_EQ(result, false);

    result = parcel2.WriteBoolUnaligned(false);
    EXPECT_EQ(result, false);

    result = parcel2.WriteInt8(false);
    EXPECT_EQ(result, false);

    result = parcel2.WriteInt8Unaligned(false);
    EXPECT_EQ(result, false);

    result = parcel2.WriteInt32(false);
    EXPECT_EQ(result, false);

    result = parcel2.WriteInt64(false);
    EXPECT_EQ(result, false);

    result = parcel2.WriteUint8(false);
    EXPECT_EQ(result, false);

    result = parcel2.WriteUint16(false);
    EXPECT_EQ(result, false);

    result = parcel2.WriteUint8Unaligned(false);
    EXPECT_EQ(result, false);

    result = parcel2.WriteUint16Unaligned(false);
    EXPECT_EQ(result, false);

    result = parcel2.WriteUint32(false);
    EXPECT_EQ(result, false);

    result = parcel2.WriteUint64(false);
    EXPECT_EQ(result, false);

    result = parcel2.WriteFloat(false);
    EXPECT_EQ(result, false);

    result = parcel2.WriteDouble(false);
    EXPECT_EQ(result, false);

    result = parcel2.WritePointer(false);
    EXPECT_EQ(result, false);
}

/**
 * @tc.name: test_parcel_WriteAndRead_001
 * @tc.desc: test parcel primary type read write.
 * @tc.type: FUNC
 */
HWTEST_F(UtilsParcelTest, test_parcel_WriteAndRead_001, TestSize.Level0)
{
    Parcel parcel(nullptr);
    struct TestData data = { true, -0x34, 0x5634, -0x12345678, 0x34, 0x5634, 0x12345678 };
    WriteTestData(parcel, data);
    ReadTestData(parcel, data);

    WriteUnalignedTestData(parcel, data);
    ReadUnalignedTestData(parcel, data);
}

/**
 * @tc.name: test_parcel_WriteAndRead_002
 * @tc.desc: test parcel primary type read write.
 * @tc.type: FUNC
 */
HWTEST_F(UtilsParcelTest, test_parcel_WriteAndRead_002, TestSize.Level0)
{
    Parcel parcel1(nullptr);
    Parcel parcel2(nullptr);
    struct TestData data = { true, -0x34, 0x5634, -0x12345678, 0x34, 0x5634, 0x12345678 };
    WriteTestData(parcel1, data);
    WriteUnalignedTestData(parcel1, data);

    void *buffer = nullptr;
    size_t size = parcel1.GetDataSize();
    if (!SendData(buffer, size, reinterpret_cast<const uint8_t *>(parcel1.GetData()))) {
        ASSERT_FALSE(false);
    }

    bool result = parcel2.ParseFrom(reinterpret_cast<uintptr_t>(buffer), parcel1.GetDataSize());
    EXPECT_EQ(result, true);

    ReadTestData(parcel2, data);
    ReadUnalignedTestData(parcel2, data);
}

/**
 * @tc.name: test_parcel_WriteAndRead_003
 * @tc.desc: test parcel primary type read write.
 * @tc.type: FUNC
 */
HWTEST_F(UtilsParcelTest, test_parcel_WriteAndRead_003, TestSize.Level0)
{
    Parcel parcel1(nullptr);
    Parcel parcel2(nullptr);
    struct TestData data = { true, -0x34, 0x5634, -0x12345678, 0x34, 0x5634, 0x12345678 };
    WriteTestData(parcel1, data);

    void *buffer = nullptr;
    size_t size = parcel1.GetDataSize();
    if (!SendData(buffer, size, reinterpret_cast<const uint8_t *>(parcel1.GetData()))) {
        ASSERT_FALSE(false);
    }

    bool result = parcel2.ParseFrom(reinterpret_cast<uintptr_t>(buffer), parcel1.GetDataSize());
    EXPECT_EQ(result, true);

    ReadTestDataWithTarget(parcel2, data);
}

/**
 * @tc.name: test_parcel_WriteAndRead_004
 * @tc.desc: test parcel primary type read write.
 * @tc.type: FUNC
 */
HWTEST_F(UtilsParcelTest, test_parcel_WriteAndRead_004, TestSize.Level0)
{
    Parcel parcel1(nullptr);

    int64_t int64test = -0x1234567887654321;
    bool result = parcel1.WriteInt64(int64test);
    EXPECT_EQ(result, true);

    uint64_t uint64test = 0x1234567887654321;
    result = parcel1.WriteUint64(uint64test);
    EXPECT_EQ(result, true);

    int64_t readint64 = parcel1.ReadInt64();
    EXPECT_EQ(readint64, int64test);

    uint64_t readuint64 = parcel1.ReadUint64();
    EXPECT_EQ(readuint64, uint64test);

    Parcel parcel2(nullptr);

    void *buffer = nullptr;
    size_t size = parcel1.GetDataSize();
    if (!SendData(buffer, size, reinterpret_cast<const uint8_t *>(parcel1.GetData()))) {
        ASSERT_FALSE(false);
    }

    result = parcel2.ParseFrom(reinterpret_cast<uintptr_t>(buffer), parcel1.GetDataSize());

    readint64 = parcel2.ReadInt64();
    EXPECT_EQ(readint64, int64test);

    readuint64 = parcel2.ReadUint64();
    EXPECT_EQ(readuint64, uint64test);
}

/**
 * @tc.name: test_parcel_WriteAndRead_String_001
 * @tc.desc: test parcel string read write.
 * @tc.type: FUNC
 */
HWTEST_F(UtilsParcelTest, test_parcel_WriteAndRead_String_001, TestSize.Level0)
{
    Parcel parcel1(nullptr);

    string strWrite = "test";
    bool result = parcel1.WriteString(strWrite);
    EXPECT_EQ(result, true);

    string strWrite1 =
        "test for write string padded**********************************************************##################";
    result = parcel1.WriteString(strWrite1);
    EXPECT_EQ(result, true);

    string strWrite2 =
        "test for write string padded**********************************************************##################";
    result = parcel1.WriteString(strWrite2);
    EXPECT_EQ(result, true);

    string strRead = parcel1.ReadString();
    string strRead1 = parcel1.ReadString();
    string strRead2 = parcel1.ReadString();
    EXPECT_EQ(0, strcmp(strRead.c_str(), strWrite.c_str()));
    EXPECT_EQ(0, strcmp(strRead1.c_str(), strWrite1.c_str()));
    EXPECT_EQ(0, strcmp(strRead2.c_str(), strWrite2.c_str()));

    Parcel parcel2(nullptr);

    void *buffer = nullptr;
    size_t size = parcel1.GetDataSize();
    if (!SendData(buffer, size, reinterpret_cast<const uint8_t *>(parcel1.GetData()))) {
        ASSERT_FALSE(false);
    }

    result = parcel2.ParseFrom(reinterpret_cast<uintptr_t>(buffer), parcel1.GetDataSize());

    strRead = parcel2.ReadString();
    strRead1 = parcel2.ReadString();
    strRead2 = parcel2.ReadString();
    EXPECT_EQ(0, strcmp(strRead.c_str(), strWrite.c_str()));
    EXPECT_EQ(0, strcmp(strRead1.c_str(), strWrite1.c_str()));
    EXPECT_EQ(0, strcmp(strRead2.c_str(), strWrite2.c_str()));
}

/**
 * @tc.name: test_parcel_WriteAndRead_String_002
 * @tc.desc: test parcel string read write.
 * @tc.type: FUNC
 */
HWTEST_F(UtilsParcelTest, test_parcel_WriteAndRead_String_002, TestSize.Level0)
{
    Parcel parcel1(nullptr);

    u16string str16Write = u"12345";
    bool result = parcel1.WriteString16(str16Write);
    EXPECT_EQ(result, true);

    u16string str16Write2 = u"12345 test for write16string padded*********";
    result = parcel1.WriteString16(str16Write2);
    EXPECT_EQ(result, true);

    u16string str16Read = parcel1.ReadString16();
    u16string str16Read2 = parcel1.ReadString16();
    EXPECT_EQ(0, str16Read.compare(str16Write));
    EXPECT_EQ(0, str16Read2.compare(str16Write2));

    Parcel parcel2(nullptr);

    void *buffer = nullptr;
    size_t size = parcel1.GetDataSize();
    if (!SendData(buffer, size, reinterpret_cast<const uint8_t *>(parcel1.GetData()))) {
        ASSERT_FALSE(false);
    }

    result = parcel2.ParseFrom(reinterpret_cast<uintptr_t>(buffer), parcel1.GetDataSize());

    str16Read = parcel2.ReadString16();
    str16Read2 = parcel2.ReadString16();
    EXPECT_EQ(0, str16Read.compare(str16Write));
    EXPECT_EQ(0, str16Read2.compare(str16Write2));
}

/**
 * @tc.name: test_parcel_WriteAndRead_String_003
 * @tc.desc: test parcel CString read write.
 * @tc.type: FUNC
 */
HWTEST_F(UtilsParcelTest, test_parcel_WriteAndRead_String_003, TestSize.Level0)
{
    Parcel parcel(nullptr);
    string test1 = "12345";
    string test2 = "23456";
    string test3 = "34567";
    string test4 = "45678";
    bool result = parcel.WriteCString(nullptr);
    EXPECT_FALSE(result);
    result = parcel.WriteCString(test1.c_str());
    EXPECT_TRUE(result);
    result = parcel.WriteCString(test2.c_str());
    EXPECT_TRUE(result);
    result = parcel.WriteCString(test3.c_str());
    EXPECT_TRUE(result);
    result = parcel.WriteCString(test4.c_str());
    EXPECT_TRUE(result);

    EXPECT_EQ(0, strcmp(test1.c_str(), parcel.ReadCString()));
    EXPECT_EQ(0, strcmp(test2.c_str(), parcel.ReadCString()));
    EXPECT_EQ(0, strcmp(test3.c_str(), parcel.ReadCString()));
    EXPECT_EQ(0, strcmp(test4.c_str(), parcel.ReadCString()));
}

/**
 * @tc.name: test_parcel_WriteAndRead_String004
 * @tc.desc: test parcel CString read write.
 * @tc.type: FUNC
 */
HWTEST_F(UtilsParcelTest, test_parcel_WriteAndRead_String004, TestSize.Level0)
{
    Parcel parcel1(nullptr);
    bool result = false;

    // write from Java, read from C++
    result = parcel1.WriteString16WithLength(nullptr, 0);
    EXPECT_EQ(result, true);

    u16string str16write = u"12345";
    char16_t *value1 = str16write.data();
    result = parcel1.WriteString16WithLength(value1, str16write.length());
    EXPECT_EQ(result, true);

    u16string str16write2 = u"12345 test for write16string padded*********";
    char16_t *value2 = str16write2.data();
    result = parcel1.WriteString16WithLength(value2, str16write2.length());
    EXPECT_EQ(result, true);

    u16string str16readNull = parcel1.ReadString16();
    u16string str16read1 = parcel1.ReadString16();
    u16string str16read2 = parcel1.ReadString16();
    EXPECT_EQ(0, str16readNull.compare(std::u16string()));
    EXPECT_EQ(0, str16read1.compare(str16write));
    EXPECT_EQ(0, str16read2.compare(str16write2));

    // write from C++, read from Java
    result = parcel1.WriteString16(str16write);
    EXPECT_EQ(result, true);

    result = parcel1.WriteString16(str16write2);
    EXPECT_EQ(result, true);

    int32_t readLength1 = 0;
    u16string str16read3 = parcel1.ReadString16WithLength(readLength1);
    EXPECT_EQ(readLength1, static_cast<int32_t>(str16write.length()));

    int32_t readLength2 = 0;
    u16string str16read4 = parcel1.ReadString16WithLength(readLength2);
    EXPECT_EQ(readLength2, static_cast<int32_t>(str16write2.length()));

    EXPECT_EQ(0, str16read3.compare(str16write));
    EXPECT_EQ(0, str16read4.compare(str16write2));
}

/**
 * @tc.name: test_parcel_WriteAndRead_String005
 * @tc.desc: test parcel CString read write.
 * @tc.type: FUNC
 */
HWTEST_F(UtilsParcelTest, test_parcel_WriteAndRead_String005, TestSize.Level0)
{
    Parcel parcel1(nullptr);
    bool result = false;

    // write from Java, read from C++
    result = parcel1.WriteString8WithLength(nullptr, 0);
    EXPECT_EQ(result, true);

    string str8write = "12345";
    char *value1 = str8write.data();
    result = parcel1.WriteString8WithLength(value1, str8write.length());
    EXPECT_EQ(result, true);

    string str8write2 = "12345 test for write16string padded*********";
    char *value2 = str8write2.data();
    result = parcel1.WriteString8WithLength(value2, str8write2.length());
    EXPECT_EQ(result, true);

    string str8readNull = parcel1.ReadString();
    string str8read1 = parcel1.ReadString();
    string str8read2 = parcel1.ReadString();
    EXPECT_EQ(0, str8readNull.compare(std::string()));
    EXPECT_EQ(0, str8read1.compare(str8write));
    EXPECT_EQ(0, str8read2.compare(str8write2));

    // write from C++, read from Java
    result = parcel1.WriteString(str8write);
    EXPECT_EQ(result, true);

    result = parcel1.WriteString(str8write2);
    EXPECT_EQ(result, true);

    int32_t readLength1 = 0;
    string str8read3 = parcel1.ReadString8WithLength(readLength1);
    EXPECT_EQ(readLength1, static_cast<int32_t>(str8write.length()));

    int32_t readLength2 = 0;
    string str8read4 = parcel1.ReadString8WithLength(readLength2);
    EXPECT_EQ(readLength2, static_cast<int32_t>(str8write2.length()));

    EXPECT_EQ(0, str8read3.compare(str8write));
    EXPECT_EQ(0, str8read4.compare(str8write2));
}

/**
 * @tc.name: test_parcel_WriteAndRead_Float_001
 * @tc.desc: test parcel float types read write.
 * @tc.type: FUNC
 */
HWTEST_F(UtilsParcelTest, test_parcel_WriteAndRead_Float_001, TestSize.Level0)
{
    Parcel parcel1(nullptr);

    float floatwrite = 12.345678f;
    bool result = parcel1.WriteFloat(floatwrite);
    EXPECT_EQ(result, true);

    double doublewrite = 1345.7653;
    result = parcel1.WriteDouble(doublewrite);
    EXPECT_EQ(result, true);

    float floatread;
    result = parcel1.ReadFloat(floatread);
    EXPECT_EQ(result, true);
    EXPECT_EQ(floatwrite, floatread);

    double doubleread;
    doubleread = parcel1.ReadDouble();
    EXPECT_EQ(doublewrite, doubleread);

    Parcel parcel2(nullptr);

    void *buffer = nullptr;
    size_t size = parcel1.GetDataSize();
    if (!SendData(buffer, size, reinterpret_cast<const uint8_t *>(parcel1.GetData()))) {
        ASSERT_FALSE(false);
    }

    result = parcel2.ParseFrom(reinterpret_cast<uintptr_t>(buffer), parcel1.GetDataSize());
    result = parcel2.ReadFloat(floatread);
    EXPECT_EQ(result, true);
    EXPECT_EQ(floatwrite, floatread);

    doubleread = parcel2.ReadDouble();
    EXPECT_EQ(doublewrite, doubleread);
}

/**
 * @tc.name: test_parcel_WriteAndRead_String_005
 * @tc.desc: test parcel String type read write.
 * @tc.type: FUNC
 */
HWTEST_F(UtilsParcelTest, test_parcel_WriteAndRead_String_005, TestSize.Level0)
{
    Parcel parcel1(nullptr);

    string strwrite = "test";
    bool result = parcel1.WriteString(strwrite);
    EXPECT_EQ(result, true);

    string strwrite1 =
        "test for write string padded**********************************************************##################";
    result = parcel1.WriteString(strwrite1);
    EXPECT_EQ(result, true);

    string strwrite2 =
        "test for write string padded**********************************************************##################";
    result = parcel1.WriteString(strwrite2);
    EXPECT_EQ(result, true);

    string strread;
    string strread1;
    string strread2;
    result = parcel1.ReadString(strread);
    EXPECT_EQ(result, true);
    result = parcel1.ReadString(strread1);
    EXPECT_EQ(result, true);
    result = parcel1.ReadString(strread2);
    EXPECT_EQ(result, true);
    EXPECT_EQ(0, strcmp(strread.c_str(), strwrite.c_str()));
    EXPECT_EQ(0, strcmp(strread1.c_str(), strwrite1.c_str()));
    EXPECT_EQ(0, strcmp(strread2.c_str(), strwrite2.c_str()));

    Parcel parcel2(nullptr);

    void *buffer = nullptr;
    size_t size = parcel1.GetDataSize();
    if (!SendData(buffer, size, reinterpret_cast<const uint8_t *>(parcel1.GetData()))) {
        ASSERT_FALSE(false);
    }

    result = parcel2.ParseFrom(reinterpret_cast<uintptr_t>(buffer), parcel1.GetDataSize());
    EXPECT_EQ(result, true);

    result = parcel2.ReadString(strread);
    EXPECT_EQ(result, true);
    result = parcel2.ReadString(strread1);
    EXPECT_EQ(result, true);
    result = parcel2.ReadString(strread2);
    EXPECT_EQ(result, true);
    EXPECT_EQ(0, strcmp(strread.c_str(), strwrite.c_str()));
    EXPECT_EQ(0, strcmp(strread1.c_str(), strwrite1.c_str()));
    EXPECT_EQ(0, strcmp(strread2.c_str(), strwrite2.c_str()));
}

struct Padded {
    char title;
    int32_t handle;
    uint64_t cookie;
};

struct Unpadded {
    char tip;
};

void ValidatePadded(const struct Padded &left, const struct Padded &right)
{
    EXPECT_EQ(left.title, right.title);
    EXPECT_EQ(left.handle, right.handle);
    EXPECT_EQ(left.cookie, right.cookie);
}

void ValidateUnpadded(const struct Unpadded &left, const struct Unpadded &right)
{
    EXPECT_EQ(left.tip, right.tip);
}

/**
 * @tc.name: test_CalcNewCapacity_001
 * @tc.desc: test kinds of input to CalcNewCapacity.
 * @tc.type: FUNC
 */
HWTEST_F(UtilsParcelTest, test_CalcNewCapacity_001, TestSize.Level0)
{
    Parcel parcel;

    size_t newMaxCapacity;
    size_t minNewCapacity = CAPACITY_THRESHOLD;
    const string strLenThreshd = string(minNewCapacity, 't');
    bool ret = parcel.WriteUnpadBuffer(static_cast<const void *>(strLenThreshd.data()), minNewCapacity);
    EXPECT_EQ(true, ret); // calculated capacity = CAPACITY_THRESHOLD

    newMaxCapacity = CAPACITY_THRESHOLD - 1;
    minNewCapacity = newMaxCapacity;
    const string strLessThreshd = string(minNewCapacity, 'l');
    parcel.SetMaxCapacity(newMaxCapacity);
    ret = parcel.WriteUnpadBuffer(static_cast<const void *>(strLessThreshd.data()), minNewCapacity);
    EXPECT_EQ(true, ret); // calculated capacity = newMaxCapacity

    newMaxCapacity = -1; // minNewCapacity = CAPACITY_THRESHOLD - 1
    const string strNoMaxCap = string(minNewCapacity, 'n');
    parcel.SetMaxCapacity(newMaxCapacity);
    ret = parcel.WriteUnpadBuffer(static_cast<const void *>(strNoMaxCap.data()), minNewCapacity);
    EXPECT_EQ(ret, true); // calculated capacity = CAPACITY_THRESHOLD

    minNewCapacity = CAPACITY_THRESHOLD + 1; // newMaxCapacity = -1
    const string strExtThreshd = string(minNewCapacity, 'e');
    parcel.SetMaxCapacity(newMaxCapacity);
    ret = parcel.WriteUnpadBuffer(static_cast<const void *>(strExtThreshd.data()), minNewCapacity);
    EXPECT_EQ(ret, true); // calculated capacity = 2 * CAPACITY_THRESHOLD

    newMaxCapacity = CAPACITY_THRESHOLD; // minNewCapacity = CAPACITY_THRESHOLD + 1
    const string strCapThreshd = string(minNewCapacity, 'e');
    parcel.SetMaxCapacity(newMaxCapacity);
    ret = parcel.WriteUnpadBuffer(static_cast<const void *>(strCapThreshd.data()), minNewCapacity);
    EXPECT_EQ(ret, true); // calculated capacity = CAPACITY_THRESHOLD
}

/**
 * @tc.name: test_SetDataCapacity_001
 * @tc.desc: test kinds of input to SetDataCapacity.
 * @tc.type: FUNC
 */
HWTEST_F(UtilsParcelTest, test_SetDataCapacity_001, TestSize.Level0)
{
    Parcel parcel;
    struct TestData data = { true, -0x34, 0x5634, -0x12345678, 0x34, 0x5634, 0x12345678 };

    WriteTestData(parcel, data);
    bool result = parcel.SetDataCapacity(0);
    EXPECT_FALSE(result);
}

/**
 * @tc.name: test_SetDataSize_001
 * @tc.desc: test kinds of input to SetDataSize.
 * @tc.type: FUNC
 */
HWTEST_F(UtilsParcelTest, test_SetDataSize_001, TestSize.Level0)
{
    Parcel parcel;

    bool result = parcel.SetDataCapacity(sizeof(bool));
    EXPECT_TRUE(result);
    result = parcel.WriteBool(true);
    EXPECT_TRUE(result);
    result = parcel.SetDataSize(DEFAULT_CPACITY + 1);
    EXPECT_FALSE(result);
}

/**
 * @tc.name: test_parcel_Data_Structure_001
 * @tc.desc: test parcel struct data related function.
 * @tc.type: FUNC
 */
HWTEST_F(UtilsParcelTest, test_parcel_Data_Structure_001, TestSize.Level0)
{
    Parcel parcel(nullptr);

    const struct Padded pad = { 'p', 0x34567890, -0x2345678998765432 };
    const struct Unpadded unpad = { 'u' };

    bool result = parcel.WriteBuffer(static_cast<const void *>(&pad), sizeof(struct Padded));
    EXPECT_EQ(true, result);
    const struct Padded *padRead = reinterpret_cast<const struct Padded *>(parcel.ReadBuffer(sizeof(struct Padded)));
    ValidatePadded(*padRead, pad);
    EXPECT_EQ(parcel.GetWritePosition(), parcel.GetReadPosition());

    result = parcel.WriteBuffer(static_cast<const void *>(&unpad), sizeof(struct Unpadded));
    const struct Unpadded *unpadRead =
        reinterpret_cast<const struct Unpadded *>(parcel.ReadBuffer(sizeof(struct Unpadded)));
    ValidateUnpadded(*unpadRead, unpad);
    EXPECT_NE(parcel.GetWritePosition(), parcel.GetReadPosition());

    parcel.RewindRead(0);
    parcel.RewindWrite(0);
    EXPECT_EQ(parcel.GetWritePosition(), parcel.GetReadPosition());

    result = parcel.WriteUnpadBuffer(static_cast<const void *>(&pad), sizeof(struct Padded));
    EXPECT_EQ(true, result);
    const struct Padded *padReadNew =
        reinterpret_cast<const struct Padded *>(parcel.ReadUnpadBuffer(sizeof(struct Padded)));
    ValidatePadded(*padReadNew, pad);
    EXPECT_EQ(parcel.GetWritePosition(), parcel.GetReadPosition());

    result = parcel.WriteUnpadBuffer(static_cast<const void *>(&unpad), sizeof(struct Unpadded));
    EXPECT_EQ(true, result);
    const struct Unpadded *unpadReadNew =
        reinterpret_cast<const struct Unpadded *>(parcel.ReadUnpadBuffer(sizeof(struct Unpadded)));
    ValidateUnpadded(*unpadReadNew, unpad);
    EXPECT_EQ(parcel.GetWritePosition(), parcel.GetReadPosition());
}

/**
 * @tc.name: test_parcel_Data_Structure_002
 * @tc.desc: test invalid input to WriteBuffer and WriteBufferAddTerminator.
 * @tc.type: FUNC
 */
HWTEST_F(UtilsParcelTest, test_parcel_Data_Structure_002, TestSize.Level0)
{
    Parcel parcel(nullptr);

    const string str = "test invalid input";
    const string strOverflow = "test write with SIZE_MAX bytes";
    const string strWriteFail = string((DEFAULT_CPACITY + 1) / sizeof(char), 'f');
    const string strWriteTermFail = string((DEFAULT_CPACITY - 2) / sizeof(char), 't');

    bool result = parcel.WriteBuffer(nullptr, sizeof(string));
    EXPECT_EQ(false, result);
    result = parcel.WriteBufferAddTerminator(nullptr, sizeof(string), sizeof(char));
    EXPECT_EQ(false, result);

    result = parcel.WriteBuffer(static_cast<const void *>(str.data()), 0);
    EXPECT_EQ(false, result);
    result = parcel.WriteBufferAddTerminator(static_cast<const void *>(str.data()), 0, sizeof(char));
    EXPECT_EQ(false, result);

    result = parcel.WriteBuffer(static_cast<const void *>(strWriteFail.data()), strWriteFail.length());
    EXPECT_EQ(false, result);
    result = parcel.WriteBufferAddTerminator(static_cast<const void *>(strWriteFail.data()),
                                             strWriteFail.length(), sizeof(char));
    EXPECT_EQ(false, result);

    result = parcel.WriteBufferAddTerminator(static_cast<const void *>(str.data()), str.length(), sizeof(char));
    EXPECT_EQ(true, result);

    Parcel recvParcel(nullptr);
    void *buffer = nullptr;
    size_t size = parcel.GetDataSize();
    if (!SendData(buffer, size, reinterpret_cast<const uint8_t *>(parcel.GetData()))) {
        ASSERT_FALSE(false);
    }
    result = recvParcel.ParseFrom(reinterpret_cast<uintptr_t>(buffer), parcel.GetDataSize());
    EXPECT_EQ(result, true);
    result = recvParcel.WriteBufferAddTerminator(static_cast<const void *>(&str), str.length() + 1, sizeof(char));
    EXPECT_EQ(result, false);
}

struct VectorTestData {
    vector<bool> booltest = { false, false, true, false, true };
    vector<int8_t> int8test = { 0x01, 0x10, -0x20, 0x30, 0x40 };
    vector<int16_t> int16test = { 0x1234, -0x2345, 0x3456, -0x4567, 0x5678 };
    vector<int32_t> int32test = { 0x12345678, -0x23456789, 0x34567890, -0x45678901 };
    vector<int64_t> int64test = { 0x1234567887654321, -0x2345678998765432 };
    vector<uint8_t> uint8test = { 0x01, 0x10, 0x20, 0x30, 0x40 };
    vector<uint16_t> uint16test = { 0x1234, 0x2345, 0x3456, 0x4567, 0x5678 };
    vector<uint32_t> uint32test = { 0x12345678, 0x23456789, 0x34567890, 0x45678901 };
    vector<uint64_t> uint64test = { 0x1234567887654321, 0x2345678998765432 };
};

void WriteVectorTestData(Parcel &parcel, const VectorTestData &data)
{
    bool result = parcel.WriteBoolVector(data.booltest);
    EXPECT_EQ(result, true);
    result = parcel.WriteInt8Vector(data.int8test);
    EXPECT_EQ(result, true);
    result = parcel.WriteInt16Vector(data.int16test);
    EXPECT_EQ(result, true);
    result = parcel.WriteInt32Vector(data.int32test);
    EXPECT_EQ(result, true);
    result = parcel.WriteInt64Vector(data.int64test);
    EXPECT_EQ(result, true);
    result = parcel.WriteUInt8Vector(data.uint8test);
    EXPECT_EQ(result, true);
    result = parcel.WriteUInt16Vector(data.uint16test);
    EXPECT_EQ(result, true);
    result = parcel.WriteUInt32Vector(data.uint32test);
    EXPECT_EQ(result, true);
    result = parcel.WriteUInt64Vector(data.uint64test);
    EXPECT_EQ(result, true);
}

void ReadVectorTestDataFunc01(Parcel &parcel, const VectorTestData &data)
{
    vector<bool> boolread;
    vector<int8_t> int8read;
    vector<int16_t> int16read;
    vector<int32_t> int32read;
    vector<int64_t> int64read;

    bool result = parcel.ReadBoolVector(&boolread);
    EXPECT_EQ(result, true);
    for (size_t i = 0; i < data.booltest.size(); i++) {
        EXPECT_EQ(data.booltest[i], boolread[i]);
    }

    result = parcel.ReadInt8Vector(&int8read);
    EXPECT_EQ(result, true);
    for (size_t i = 0; i < data.int8test.size(); i++) {
        EXPECT_EQ(data.int8test[i], int8read[i]);
    }

    result = parcel.ReadInt16Vector(&int16read);
    EXPECT_EQ(result, true);
    for (size_t i = 0; i < data.int16test.size(); i++) {
        EXPECT_EQ(data.int16test[i], int16read[i]);
    }

    result = parcel.ReadInt32Vector(&int32read);
    EXPECT_EQ(result, true);
    for (size_t i = 0; i < data.int32test.size(); i++) {
        EXPECT_EQ(data.int32test[i], int32read[i]);
    }

    result = parcel.ReadInt64Vector(&int64read);
    EXPECT_EQ(result, true);
    for (size_t i = 0; i < data.int64test.size(); i++) {
        EXPECT_EQ(data.int64test[i], int64read[i]);
    }
}

void ReadVectorTestDataFunc02(Parcel &parcel, const VectorTestData &data)
{
    vector<uint8_t> uint8read;
    vector<uint16_t> uint16read;
    vector<uint32_t> uint32read;
    vector<uint64_t> uint64read;

    bool result = parcel.ReadUInt8Vector(&uint8read);
    EXPECT_EQ(result, true);
    for (size_t i = 0; i < data.uint8test.size(); i++) {
        EXPECT_EQ(data.uint8test[i], uint8read[i]);
    }

    result = parcel.ReadUInt16Vector(&uint16read);
    EXPECT_EQ(result, true);
    for (size_t i = 0; i < data.uint16test.size(); i++) {
        EXPECT_EQ(data.uint16test[i], uint16read[i]);
    }

    result = parcel.ReadUInt32Vector(&uint32read);
    EXPECT_EQ(result, true);
    for (size_t i = 0; i < data.uint32test.size(); i++) {
        EXPECT_EQ(data.uint32test[i], uint32read[i]);
    }

    result = parcel.ReadUInt64Vector(&uint64read);
    EXPECT_EQ(result, true);
    for (size_t i = 0; i < data.uint64test.size(); i++) {
        EXPECT_EQ(data.uint64test[i], uint64read[i]);
    }
}

void ReadVectorTestData(Parcel &parcel, const VectorTestData &data)
{
    ReadVectorTestDataFunc01(parcel, data);
    ReadVectorTestDataFunc02(parcel, data);
}

/**
 * @tc.name: test_parcel_WriteAndReadVector_001
 * @tc.desc: test vector parcel read and write.
 * @tc.type: FUNC
 */
HWTEST_F(UtilsParcelTest, test_parcel_WriteAndReadVector_001, TestSize.Level0)
{
    Parcel parcel(nullptr);
    struct VectorTestData data;

    WriteVectorTestData(parcel, data);
    ReadVectorTestData(parcel, data);
}

/**
 * @tc.name: test_parcel_WriteAndReadVector_002
 * @tc.desc: test vector parcel read and write.
 * @tc.type: FUNC
 */
HWTEST_F(UtilsParcelTest, test_parcel_WriteAndReadVector_002, TestSize.Level0)
{
    Parcel parcel1(nullptr);
    struct VectorTestData data;
    WriteVectorTestData(parcel1, data);

    Parcel parcel2(nullptr);

    void *buffer = nullptr;
    size_t size = parcel1.GetDataSize();
    if (!SendData(buffer, size, reinterpret_cast<const uint8_t *>(parcel1.GetData()))) {
        ASSERT_FALSE(false);
    }

    bool result = parcel2.ParseFrom(reinterpret_cast<uintptr_t>(buffer), parcel1.GetDataSize());
    EXPECT_EQ(result, true);
    ReadVectorTestData(parcel2, data);
}

/**
 * @tc.name: test_parcel_WriteAndReadVector_003
 * @tc.desc: test vector parcel read and write.
 * @tc.type: FUNC
 */
HWTEST_F(UtilsParcelTest, test_parcel_WriteAndReadVector_003, TestSize.Level0)
{
    Parcel parcel1(nullptr);
    Parcel parcel2(nullptr);

    vector<string> stringtest{ "test", "test for", "test for write", "test for write vector" };
    vector<u16string> string16test{ u"test", u"test for", u"test for write", u"test for write vector" };

    bool result = parcel1.WriteStringVector(stringtest);
    EXPECT_EQ(result, true);
    result = parcel1.WriteString16Vector(string16test);
    EXPECT_EQ(result, true);

    vector<string> stringread;
    result = parcel1.ReadStringVector(&stringread);
    EXPECT_EQ(result, true);
    for (size_t i = 0; i < stringtest.size(); i++) {
        EXPECT_EQ(stringtest[i], stringread[i]);
    }

    vector<u16string> u16stringread;
    result = parcel1.ReadString16Vector(&u16stringread);
    EXPECT_EQ(result, true);
    for (size_t i = 0; i < string16test.size(); i++) {
        EXPECT_EQ(0, string16test[i].compare(u16stringread[i]));
    }

    void *buffer = nullptr;
    size_t size = parcel1.GetDataSize();
    if (!SendData(buffer, size, reinterpret_cast<const uint8_t *>(parcel1.GetData()))) {
        ASSERT_FALSE(false);
    }

    result = parcel2.ParseFrom(reinterpret_cast<uintptr_t>(buffer), parcel1.GetDataSize());
    result = parcel2.ReadStringVector(&stringread);
    EXPECT_EQ(result, true);
    for (size_t i = 0; i < stringtest.size(); i++) {
        EXPECT_EQ(stringtest[i], stringread[i]);
    }

    result = parcel2.ReadString16Vector(&u16stringread);
    EXPECT_EQ(result, true);
    for (size_t i = 0; i < string16test.size(); i++) {
        EXPECT_EQ(0, string16test[i].compare(u16stringread[i]));
    }
}

/**
 * @tc.name: test_parcel_WriteAndReadVector_004
 * @tc.desc: test vector parcel read and write.
 * @tc.type: FUNC
 */
HWTEST_F(UtilsParcelTest, test_parcel_WriteAndReadVector_004, TestSize.Level0)
{
    Parcel parcel1(nullptr);
    Parcel parcel2(nullptr);

    vector<float> floattest{ 11221.132313, 11221.45678 };
    vector<double> doubletest{ 1122.132313, 1122.45678 };

    bool result = parcel1.WriteFloatVector(floattest);
    EXPECT_EQ(result, true);

    result = parcel1.WriteDoubleVector(doubletest);
    EXPECT_EQ(result, true);

    vector<float> floatread;
    vector<double> doubleread;

    result = parcel1.ReadFloatVector(&floatread);
    EXPECT_EQ(result, true);
    for (size_t i = 0; i < floattest.size(); i++) {
        EXPECT_EQ(floattest[i], floatread[i]);
    }

    result = parcel1.ReadDoubleVector(&doubleread);
    EXPECT_EQ(result, true);
    for (size_t i = 0; i < doubletest.size(); i++) {
        EXPECT_EQ(doubletest[i], doubleread[i]);
    }
}

bool CallWriteVector(Parcel &parcel, const std::vector<bool> &vectorTest)
{
    return parcel.WriteBoolVector(vectorTest);
}

bool CallWriteVector(Parcel &parcel, const std::vector<int8_t> &vectorTest)
{
    return parcel.WriteInt8Vector(vectorTest);
}

bool CallWriteVector(Parcel &parcel, const std::vector<int16_t> &vectorTest)
{
    return parcel.WriteInt16Vector(vectorTest);
}

bool CallWriteVector(Parcel &parcel, const std::vector<int32_t> &vectorTest)
{
    return parcel.WriteInt32Vector(vectorTest);
}

bool CallWriteVector(Parcel &parcel, const std::vector<int64_t> &vectorTest)
{
    return parcel.WriteInt64Vector(vectorTest);
}

bool CallWriteVector(Parcel &parcel, const std::vector<uint8_t> &vectorTest)
{
    return parcel.WriteUInt8Vector(vectorTest);
}

bool CallWriteVector(Parcel &parcel, const std::vector<uint16_t> &vectorTest)
{
    return parcel.WriteUInt16Vector(vectorTest);
}

bool CallWriteVector(Parcel &parcel, const std::vector<uint32_t> &vectorTest)
{
    return parcel.WriteUInt32Vector(vectorTest);
}

bool CallWriteVector(Parcel &parcel, const std::vector<uint64_t> &vectorTest)
{
    return parcel.WriteUInt64Vector(vectorTest);
}

bool CallWriteVector(Parcel &parcel, const std::vector<float> &vectorTest)
{
    return parcel.WriteFloatVector(vectorTest);
}

bool CallWriteVector(Parcel &parcel, const std::vector<double> &vectorTest)
{
    return parcel.WriteDoubleVector(vectorTest);
}

bool CallWriteVector(Parcel &parcel, const std::vector<std::string> &vectorTest)
{
    return parcel.WriteStringVector(vectorTest);
}

bool CallWriteVector(Parcel &parcel, const std::vector<std::u16string> &vectorTest)
{
    return parcel.WriteString16Vector(vectorTest);
}

template <typename T>
void ParcelWriteVector(const std::vector<T> &vectorTest)
{
    Parcel parcel1(nullptr);
    Parcel parcel2(nullptr);
    bool result = CallWriteVector(parcel1, vectorTest);
    EXPECT_EQ(result, true);

    void *buffer = nullptr;
    size_t size = parcel1.GetDataSize();
    if (!SendData(buffer, size, reinterpret_cast<const uint8_t *>(parcel1.GetData()))) {
        ASSERT_FALSE(false);
    }
    result = parcel2.ParseFrom(reinterpret_cast<uintptr_t>(buffer), parcel1.GetDataSize());
    EXPECT_EQ(result, true);

    result = CallWriteVector(parcel2, vectorTest);
    EXPECT_EQ(result, false);
}

/**
 * @tc.name: test_parcel_WriteAndReadVector_005
 * @tc.desc: test vector parcel write failed.
 * @tc.type: FUNC
 */
HWTEST_F(UtilsParcelTest, test_parcel_WriteAndReadVector_005, TestSize.Level0)
{
    vector<bool> boolVectorTest { true, false };
    vector<int8_t> int8VectorTest { 1, 0 };
    vector<int16_t> int16VectorTest { 1, 0 };
    vector<int32_t> int32VectorTest { 1, 0 };
    vector<int64_t> int64VectorTest { 1, 0 };
    vector<uint8_t> uint8VectorTest { 1, 0 };
    vector<uint16_t> uint16VectorTest { 1, 0 };
    vector<uint32_t> uint32VectorTest { 1, 0 };
    vector<uint64_t> uint64VectorTest { 1, 0 };
    vector<float> floatVectorTest { 1.1, 0 };
    vector<double> doubleVectorTest { 1.1, 0 };
    vector<std::string> stringVectorTest { "true", "false" };
    vector<std::u16string> string16VectorTest { u"true", u"false" };

    ParcelWriteVector(boolVectorTest);
    ParcelWriteVector(int8VectorTest);
    ParcelWriteVector(int16VectorTest);
    ParcelWriteVector(int32VectorTest);
    ParcelWriteVector(int64VectorTest);
    ParcelWriteVector(uint8VectorTest);
    ParcelWriteVector(uint16VectorTest);
    ParcelWriteVector(uint32VectorTest);
    ParcelWriteVector(uint64VectorTest);
    ParcelWriteVector(floatVectorTest);
    ParcelWriteVector(doubleVectorTest);
    ParcelWriteVector(stringVectorTest);
    ParcelWriteVector(string16VectorTest);
}

class TestParcelable : public virtual Parcelable {
public:
    TestParcelable() = default;
    ~TestParcelable() = default;

    bool Marshalling(Parcel &parcel) const override;
    static TestParcelable *Unmarshalling(Parcel &parcel);

public:
    int32_t int32Write_ = -0x12345678;
    int32_t int32Read_;
};

bool TestParcelable::Marshalling(Parcel &parcel) const
{
    bool result = parcel.WriteInt32(this->int32Write_);
    return result;
}

TestParcelable *TestParcelable::Unmarshalling(Parcel &parcel)
{
    auto *read = new TestParcelable();
    read->int32Read_ = parcel.ReadInt32();
    return read;
}

/**
 * @tc.name: test_parcel_parcelable_001
 * @tc.desc: test parcel read and write parcelable obj.
 * @tc.type: FUNC
 */
HWTEST_F(UtilsParcelTest, test_parcel_parcelable_001, TestSize.Level0)
{
    Parcel parcel(nullptr);
    sptr<TestParcelable> parcelableWrite = new TestParcelable();
    bool result = false;

    result = parcel.WriteParcelable(parcelableWrite);
    EXPECT_EQ(true, result);
    EXPECT_EQ(parcel.GetWritePosition(), parcel.GetDataSize());

    sptr<TestParcelable> parcelableRead = parcel.ReadParcelable<TestParcelable>();
    EXPECT_EQ(parcelableWrite->int32Write_, parcelableRead->int32Read_);
    EXPECT_EQ(parcel.GetReadPosition(), parcel.GetDataSize());
}

/**
 * @tc.name: test_parcel_parcelable_002
 * @tc.desc: test parcel read and write parcelable obj.
 * @tc.type: FUNC
 */
HWTEST_F(UtilsParcelTest, test_parcel_parcelable_002, TestSize.Level0)
{
    Parcel parcel(nullptr);

    bool result = parcel.WriteParcelable(nullptr);
    EXPECT_EQ(true, result);

    sptr<TestParcelable> parcelableRead = parcel.ReadParcelable<TestParcelable>();
    EXPECT_EQ(nullptr, parcelableRead);
}

/**
 * @tc.name: test_parcel_parcelable_003
 * @tc.desc: test parcel read and write parcelable obj.
 * @tc.type: FUNC
 */
HWTEST_F(UtilsParcelTest, test_parcel_parcelable_003, TestSize.Level0)
{
    Parcel parcel(nullptr);
    sptr<TestParcelable> parcelableWriteNull;
    bool result = parcel.WriteStrongParcelable(parcelableWriteNull);
    EXPECT_EQ(true, result);

    sptr<TestParcelable> parcelableWrite = new TestParcelable();

    bool test = parcelableWrite->TestBehavior(Parcelable::BehaviorFlag::HOLD_OBJECT);
    EXPECT_EQ(false, test);
    test = parcelableWrite->TestBehavior(Parcelable::BehaviorFlag::IPC);
    EXPECT_EQ(false, test);
    test = parcelableWrite->TestBehavior(Parcelable::BehaviorFlag::RPC);
    EXPECT_EQ(false, test);

    result = parcel.WriteStrongParcelable(parcelableWrite);
    EXPECT_EQ(true, result);

    sptr<TestParcelable> parcelableReadNull = parcel.ReadParcelable<TestParcelable>();
    EXPECT_EQ(nullptr, parcelableReadNull);

    sptr<TestParcelable> parcelableRead = parcel.ReadParcelable<TestParcelable>();
    EXPECT_EQ(parcelableWrite->int32Write_, parcelableRead->int32Read_);

    test = parcelableWrite->TestBehavior(Parcelable::BehaviorFlag::HOLD_OBJECT);
    EXPECT_EQ(true, test);
    test = parcelableWrite->TestBehavior(Parcelable::BehaviorFlag::IPC);
    EXPECT_EQ(false, test);
    test = parcelableWrite->TestBehavior(Parcelable::BehaviorFlag::RPC);
    EXPECT_EQ(false, test);

    parcelableWrite->ClearBehavior(Parcelable::BehaviorFlag::HOLD_OBJECT);
    test = parcelableWrite->TestBehavior(Parcelable::BehaviorFlag::HOLD_OBJECT);
    EXPECT_EQ(false, test);
}

/**
 * @tc.name: test_SetMaxCapacity_001
 * @tc.desc: test parcel capacity function.
 * @tc.type: FUNC
 */
HWTEST_F(UtilsParcelTest, test_SetMaxCapacity_001, TestSize.Level0)
{
    Parcel parcel(nullptr);
    char test[48] = {0};
    bool ret = parcel.WriteBuffer(test, 48);
    EXPECT_EQ(true, ret);
    // because default maxCap is 200 * 1024, so reset it more
    parcel.SetMaxCapacity(201 * 1024);
    // test write data over max capacity: 205780 + 48 > 201 * 1024
    char test2[205780] = {0};
    ret = parcel.WriteBuffer(test2, 205780);
    EXPECT_EQ(false, ret);
}

/**
 * @tc.name: test_SetMaxCapacity_002
 * @tc.desc: test parcel capacity function.
 * @tc.type: FUNC
 */
HWTEST_F(UtilsParcelTest, test_SetMaxCapacity_002, TestSize.Level0)
{
    Parcel parcel(nullptr);
    char test[48] = {0};
    bool ret = parcel.WriteInt32(5767168);
    EXPECT_EQ(true, ret);
    ret = parcel.WriteBuffer(test, 48);
    EXPECT_EQ(true, ret);
    vector<std::u16string> val;
    ret = parcel.ReadString16Vector(&val);
    EXPECT_EQ(false, ret);
}

HWTEST_F(UtilsParcelTest, test_ValidateReadData_001, TestSize.Level0)
{
    Parcel parcel(nullptr);
    parcel.WriteBool(true);
    string strWrite = "test";
    bool result = parcel.WriteString(strWrite);
    EXPECT_EQ(result, true);

    RemoteObject obj1;
    result = parcel.WriteRemoteObject(&obj1);
    EXPECT_EQ(result, true);
    parcel.WriteInt32(5);
    RemoteObject obj2;
    result = parcel.WriteRemoteObject(&obj2);
    EXPECT_EQ(result, true);
    u16string str16Write = u"12345";
    result = parcel.WriteString16(str16Write);
    EXPECT_EQ(result, true);

    bool readBool = parcel.ReadBool();
    EXPECT_EQ(readBool, true);

    string strRead = parcel.ReadString();
    EXPECT_EQ(0, strcmp(strRead.c_str(), strWrite.c_str()));

    sptr<RemoteObject> readObj1 = parcel.ReadObject<RemoteObject>();
    EXPECT_EQ(true, readObj1.GetRefPtr() != nullptr);

    int32_t readInt32 = parcel.ReadInt32();
    EXPECT_EQ(readInt32, 5);

    sptr<RemoteObject> readObj2 = parcel.ReadObject<RemoteObject>();
    EXPECT_EQ(true, readObj2.GetRefPtr() != nullptr);

    u16string str16Read = parcel.ReadString16();
    EXPECT_EQ(0, str16Read.compare(str16Write));
}

HWTEST_F(UtilsParcelTest, test_ValidateReadData_002, TestSize.Level0)
{
    Parcel parcel(nullptr);
    parcel.WriteBool(true);
    string strWrite = "test";
    bool result = parcel.WriteString(strWrite);
    EXPECT_EQ(result, true);

    RemoteObject obj1;
    result = parcel.WriteRemoteObject(&obj1);
    EXPECT_EQ(result, true);
    parcel.WriteInt32(5);
    RemoteObject obj2;
    result = parcel.WriteRemoteObject(&obj2);
    EXPECT_EQ(result, true);
    u16string str16Write = u"12345";
    result = parcel.WriteString16(str16Write);
    EXPECT_EQ(result, true);

    bool readBool = parcel.ReadBool();
    EXPECT_EQ(readBool, true);

    string strRead = parcel.ReadString();
    EXPECT_EQ(0, strcmp(strRead.c_str(), strWrite.c_str()));

    int32_t readInt32 = parcel.ReadInt32();
    EXPECT_EQ(readInt32, 0);

    u16string str16Read = parcel.ReadString16();
    EXPECT_EQ(0, str16Read.compare(std::u16string()));

    sptr<RemoteObject> readObj1 = parcel.ReadObject<RemoteObject>();
    EXPECT_EQ(true, readObj1.GetRefPtr() == nullptr);
}

HWTEST_F(UtilsParcelTest, test_RewindWrite_001, TestSize.Level0)
{
    Parcel parcel(nullptr);
    parcel.WriteInt32(5);
    string strWrite = "test";
    parcel.WriteString(strWrite);
    RemoteObject obj1;
    parcel.WriteRemoteObject(&obj1);
    size_t pos = parcel.GetWritePosition();
    parcel.WriteInt32(5);
    RemoteObject obj2;
    parcel.WriteRemoteObject(&obj2);
    u16string str16Write = u"12345";
    parcel.WriteString16(str16Write);

    bool result = parcel.RewindWrite(pos);
    EXPECT_EQ(result, true);
    parcel.WriteInt32(5);
    parcel.WriteInt32(5);

    int32_t readint32 = parcel.ReadInt32();
    EXPECT_EQ(readint32, 5);
    string strRead = parcel.ReadString();
    EXPECT_EQ(0, strcmp(strRead.c_str(), strWrite.c_str()));
    sptr<RemoteObject> readObj1 = parcel.ReadObject<RemoteObject>();
    EXPECT_EQ(true, readObj1.GetRefPtr() != nullptr);
    readint32 = parcel.ReadInt32();
    EXPECT_EQ(readint32, 5);
    sptr<RemoteObject> readObj2 = parcel.ReadObject<RemoteObject>();
    EXPECT_EQ(true, readObj2.GetRefPtr() == nullptr);
    readint32 = parcel.ReadInt32();
    EXPECT_EQ(readint32, 5);
}

HWTEST_F(UtilsParcelTest, test_RewindWrite_002, TestSize.Level0)
{
    Parcel parcel(nullptr);
    parcel.WriteInt32(5);
    string strWrite = "test";
    parcel.WriteString(strWrite);
    RemoteObject obj1;
    parcel.WriteRemoteObject(&obj1);
    parcel.WriteInt32(5);
    RemoteObject obj2;
    parcel.WriteRemoteObject(&obj2);
    size_t pos = parcel.GetWritePosition();
    u16string str16Write = u"12345";
    parcel.WriteString16(str16Write);

    bool result = parcel.RewindWrite(pos);
    EXPECT_EQ(result, true);

    int32_t readint32 = parcel.ReadInt32();
    EXPECT_EQ(readint32, 5);
    string strRead = parcel.ReadString();
    EXPECT_EQ(0, strcmp(strRead.c_str(), strWrite.c_str()));
    uint32_t readUint32 = parcel.ReadUint32();
    EXPECT_EQ(readUint32, 0);
    string strRead2 = parcel.ReadString();
    EXPECT_EQ(0, strRead2.compare(std::string()));
    sptr<RemoteObject> readObj1 = parcel.ReadObject<RemoteObject>();
    EXPECT_EQ(true, readObj1.GetRefPtr() == nullptr);
    double readDouble = parcel.ReadDouble();
    EXPECT_EQ(readDouble, 0);
}

HWTEST_F(UtilsParcelTest, test_RewindWrite_003, TestSize.Level0)
{
    Parcel parcel(nullptr);
    std::vector<int32_t> val{1, 2, 3, 4, 5};
    EXPECT_EQ(val.size(), 5);
    bool result = parcel.WriteInt32Vector(val);
    EXPECT_EQ(result, true);
    size_t pos = parcel.GetWritePosition() - sizeof(int32_t);
    result = parcel.RewindWrite(pos);
    EXPECT_EQ(result, true);
    RemoteObject obj;
    parcel.WriteRemoteObject(&obj);

    std::vector<int32_t> int32Read;
    result = parcel.ReadInt32Vector(&int32Read);
    EXPECT_EQ(result, false);
    EXPECT_EQ(int32Read.size(), 5);
    EXPECT_EQ(int32Read[0], 1);
    EXPECT_EQ(int32Read[1], 2);
    EXPECT_EQ(int32Read[2], 3);
    EXPECT_EQ(int32Read[3], 4);
    EXPECT_EQ(int32Read[4], 0);
}

HWTEST_F(UtilsParcelTest, test_VectorDataPadding_001, TestSize.Level0)
{
    Parcel parcel1(nullptr);
    std::vector<bool> val1(121, true);
    bool result = parcel1.WriteBoolVector(val1);
    EXPECT_EQ(result, true);

    int32_t targetVal = 123;
    parcel1.WriteInt32(targetVal);

    std::vector<bool> val2;
    result = parcel1.ReadBoolVector(&val2);
    int32_t target = parcel1.ReadInt32();
    EXPECT_EQ(target, targetVal);
}

HWTEST_F(UtilsParcelTest, test_VectorDataPadding_002, TestSize.Level0)
{
    Parcel parcel1(nullptr);
    std::vector<bool> val1(15, true);
    bool result = parcel1.WriteBoolVector(val1);
    EXPECT_EQ(result, true);

    std::vector<bool> val2(16, true);
    result = parcel1.WriteBoolVector(val2);
    EXPECT_EQ(result, true);

    std::vector<bool> val3;
    result = parcel1.ReadBoolVector(&val3);
    for (int i = 0; i < val1.size(); i++) {
        EXPECT_EQ(val1[i], val3[i]);
    }

    std::vector<bool> val4;
    result = parcel1.ReadBoolVector(&val4);
    for (int i = 0; i < val2.size(); i++) {
        EXPECT_EQ(val2[i], val4[i]);
    }
    parcel1.FlushBuffer();

    result = parcel1.WriteBoolVector(val2);
    EXPECT_EQ(result, true);
    result = parcel1.WriteBoolVector(val1);
    EXPECT_EQ(result, true);

    std::vector<bool> val5;
    result = parcel1.ReadBoolVector(&val5);
    for (int i = 0; i < val2.size(); i++) {
        EXPECT_EQ(val2[i], val5[i]);
    }

    std::vector<bool> val6;
    result = parcel1.ReadBoolVector(&val6);
    for (int i = 0; i < val1.size(); i++) {
        EXPECT_EQ(val1[i], val6[i]);
    }
}

HWTEST_F(UtilsParcelTest, test_VectorDataPadding_003, TestSize.Level0)
{
    Parcel parcel1(nullptr);
    std::vector<bool> val1(17, true);
    bool result = parcel1.WriteBoolVector(val1);
    EXPECT_EQ(result, true);

    std::vector<int16_t> val2(18, 1);
    result = parcel1.WriteInt16Vector(val2);
    EXPECT_EQ(result, true);

    std::vector<bool> val3;
    result = parcel1.ReadBoolVector(&val3);
    for (int i = 0; i < val1.size(); i++) {
        EXPECT_EQ(val1[i], val3[i]);
    }

    std::vector<int16_t> val4;
    result = parcel1.ReadInt16Vector(&val4);
    for (int i = 0; i < val2.size(); i++) {
        EXPECT_EQ(val2[i], val4[i]);
    }
    parcel1.FlushBuffer();

    result = parcel1.WriteInt16Vector(val2);
    EXPECT_EQ(result, true);
    result = parcel1.WriteBoolVector(val1);
    EXPECT_EQ(result, true);

    std::vector<int16_t> val5;
    result = parcel1.ReadInt16Vector(&val5);
    for (int i = 0; i < val2.size(); i++) {
        EXPECT_EQ(val2[i], val5[i]);
    }

    std::vector<bool> val6;
    result = parcel1.ReadBoolVector(&val6);
    for (int i = 0; i < val1.size(); i++) {
        EXPECT_EQ(val1[i], val6[i]);
    }
}

HWTEST_F(UtilsParcelTest, test_VectorDataPadding_004, TestSize.Level0)
{
    Parcel parcel1(nullptr);
    std::vector<int16_t> val1(19, 1);
    bool result = parcel1.WriteInt16Vector(val1);
    EXPECT_EQ(result, true);

    std::vector<int16_t> val2(20, 1);
    result = parcel1.WriteInt16Vector(val2);
    EXPECT_EQ(result, true);

    std::vector<int16_t> val3;
    result = parcel1.ReadInt16Vector(&val3);
    for (int i = 0; i < val1.size(); i++) {
        EXPECT_EQ(val1[i], val3[i]);
    }

    std::vector<int16_t> val4;
    result = parcel1.ReadInt16Vector(&val4);
    for (int i = 0; i < val2.size(); i++) {
        EXPECT_EQ(val2[i], val4[i]);
    }
    parcel1.FlushBuffer();

    result = parcel1.WriteInt16Vector(val2);
    EXPECT_EQ(result, true);
    result = parcel1.WriteInt16Vector(val1);
    EXPECT_EQ(result, true);

    std::vector<int16_t> val5;
    result = parcel1.ReadInt16Vector(&val5);
    for (int i = 0; i < val2.size(); i++) {
        EXPECT_EQ(val2[i], val5[i]);
    }

    std::vector<int16_t> val6;
    result = parcel1.ReadInt16Vector(&val6);
    for (int i = 0; i < val1.size(); i++) {
        EXPECT_EQ(val1[i], val6[i]);
    }
}

#ifdef __aarch64__
HWTEST_F(UtilsParcelTest, test_WriteStringDataLength_001, TestSize.Level0)
{
    Parcel parcel1(nullptr);

    std::string veryLongString(static_cast<size_t>(INT32_MAX) + 1, '#');
    bool result = parcel1.WriteCString(veryLongString.c_str());
    EXPECT_EQ(result, false);
    parcel1.FlushBuffer();

    result = parcel1.WriteString(veryLongString);
    EXPECT_EQ(result, false);
    parcel1.FlushBuffer();

    std::u16string veryLongStringU16(static_cast<size_t>(INT32_MAX) / 2, '#');
    result = parcel1.WriteString16(veryLongStringU16);
    EXPECT_EQ(result, false);
    parcel1.FlushBuffer();

    result = parcel1.WriteString16WithLength(veryLongStringU16.c_str(), static_cast<size_t>(INT32_MAX) / 2);
    EXPECT_EQ(result, false);
    parcel1.FlushBuffer();

    result = parcel1.WriteString8WithLength(veryLongString.c_str(), static_cast<size_t>(INT32_MAX) + 1);
    EXPECT_EQ(result, false);
    parcel1.FlushBuffer();

    result = parcel1.WriteCString(veryLongString.substr(0, DEFAULT_CPACITY - 1).c_str());
    EXPECT_EQ(result, true);
    parcel1.FlushBuffer();

    result = parcel1.WriteString(veryLongString.substr(0, DEFAULT_CPACITY - 5));
    EXPECT_EQ(result, true);
    parcel1.FlushBuffer();

    result = parcel1.WriteString16(veryLongStringU16.substr(0, (DEFAULT_CPACITY - 4) / 2 - 1));
    EXPECT_EQ(result, true);
    parcel1.FlushBuffer();

    result = parcel1.WriteString16WithLength(veryLongStringU16.c_str(), (DEFAULT_CPACITY - 4) / 2 - 1);
    EXPECT_EQ(result, true);
    parcel1.FlushBuffer();

    result = parcel1.WriteString8WithLength(veryLongString.c_str(), DEFAULT_CPACITY - 5);
    EXPECT_EQ(result, true);
    parcel1.FlushBuffer();
}
#endif
}  // namespace
}  // namespace OHOS
