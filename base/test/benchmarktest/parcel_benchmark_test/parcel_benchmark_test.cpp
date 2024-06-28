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
#include "directory_ex.h"
#include "parcel.h"
#include "refbase.h"
#include "securec.h"
#include "benchmark_log.h"
#include "benchmark_assert.h"
using namespace std;

namespace OHOS {
namespace {

static constexpr size_t DEFAULT_CPACITY = 204800; // 200K
static constexpr size_t CAPACITY_THRESHOLD = 4096; // 4k
static constexpr int32_t COMPARE_STRING_RESULT = 0;
static constexpr size_t DATA_CAPACITY_INIT_SIZE = 0;
static constexpr size_t DATA_OFFSETS_INIT_SIZE = 0;
static constexpr size_t DATA_READBYTES_INIT_SIZE = 0;
static constexpr size_t REWIND_INIT_VALUE = 0;
static constexpr size_t REWINDWRITE003_VECTOR_LENGTH = 5;
static constexpr int32_t WRITE_AND_CMP_INT32_VALUE = 5;

#define PARCEL_TEST_CHAR_ARRAY_SIZE 48
#define PARCEL_TEST1_CHAR_ARRAY_SIZE 205780
#define PARCEL_WRITEPOINTER_CHAR_ARRAY_SIZE 128
#define PARCEL_INJECTOFFSETS_CHAR_ARRAY_SIZE 256

class BenchmarkParcelTest : public benchmark::Fixture {
public:
    void SetUp(const ::benchmark::State& state) override
    {
    }

    void TearDown(const ::benchmark::State& state) override
    {
    }

    BenchmarkParcelTest()
    {
        Iterations(iterations);
        Repetitions(repetitions);
        ReportAggregatesOnly();
    }

    ~BenchmarkParcelTest() override = default;

protected:
    const int32_t repetitions = 3;
    const int32_t iterations = 1000;
};


class RemoteObject : public virtual Parcelable {
public:
    RemoteObject()
    {
        asRemote_ = true;
    }
    bool Marshalling(Parcel &parcel) const override;
    static sptr<RemoteObject> Unmarshalling(Parcel &parcel);
    static bool Marshalling(Parcel &parcel, const sptr<RemoteObject> &object);
};

bool RemoteObject::Marshalling(Parcel &parcel, const sptr<RemoteObject> &object)
{
    return false;
}

bool RemoteObject::Marshalling(Parcel &parcel) const
{
    BENCHMARK_LOGD("ParcelTest (bool RemoteObject::Marshalling(Parcel &parcel) const) is called.");
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
    BENCHMARK_LOGD("ParcelTest sptr<RemoteObject> RemoteObject::Unmarshalling(Parcel &parcel) is called.");
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

void WriteTestData(Parcel &parcel, const struct TestData &data, benchmark::State& state)
{
    BENCHMARK_LOGD("ParcelTest void WriteTestData is called.");
    bool result = false;
    result = parcel.WriteBool(data.booltest);
    AssertEqual(result, true, "result did not equal true as expected.", state);

    result = parcel.WriteInt8(data.int8test);
    AssertEqual(result, true, "result did not equal true as expected.", state);

    result = parcel.WriteInt16(data.int16test);
    AssertEqual(result, true, "result did not equal true as expected.", state);

    result = parcel.WriteInt32(data.int32test);
    AssertEqual(result, true, "result did not equal true as expected.", state);

    result = parcel.WriteUint8(data.uint8test);
    AssertEqual(result, true, "result did not equal true as expected.", state);

    result = parcel.WriteUint16(data.uint16test);
    AssertEqual(result, true, "result did not equal true as expected.", state);

    result = parcel.WriteUint32(data.uint32test);
    AssertEqual(result, true, "result did not equal true as expected.", state);
}

void WriteUnalignedTestData(Parcel &parcel, const struct TestData &data, benchmark::State& state)
{
    BENCHMARK_LOGD("ParcelTest void WriteUnalignedTestData is called.");
    bool result = false;
    result = parcel.WriteBoolUnaligned(data.booltest);
    AssertEqual(result, true, "result did not equal true as expected.", state);

    result = parcel.WriteInt8Unaligned(data.int8test);
    AssertEqual(result, true, "result did not equal true as expected.", state);

    result = parcel.WriteInt16Unaligned(data.int16test);
    AssertEqual(result, true, "result did not equal true as expected.", state);

    result = parcel.WriteUint8Unaligned(data.uint8test);
    AssertEqual(result, true, "result did not equal true as expected.", state);

    result = parcel.WriteUint16Unaligned(data.uint16test);
    AssertEqual(result, true, "result did not equal true as expected.", state);
}

void ReadTestData(Parcel &parcel, const struct TestData &data, benchmark::State& state)
{
    BENCHMARK_LOGD("ParcelTest void ReadTestData is called.");
    bool readbool = parcel.ReadBool();
    AssertEqual(readbool, data.booltest, "readbool did not equal data.booltest as expected.", state);

    int8_t readint8 = parcel.ReadInt8();
    AssertEqual(readint8, data.int8test, "readint8 did not equal data.int8test as expected.", state);

    int16_t readint16 = parcel.ReadInt16();
    AssertEqual(readint16, data.int16test, "readint16 did not equal data.int16test as expected.", state);

    int32_t readint32 = parcel.ReadInt32();
    AssertEqual(readint32, data.int32test, "readint32 did not equal data.int32test as expected.", state);

    uint8_t readuint8 = parcel.ReadUint8();
    AssertEqual(readuint8, data.uint8test, "readuint8 did not equal data.uint8test as expected.", state);

    uint16_t readuint16 = parcel.ReadUint16();
    AssertEqual(readuint16, data.uint16test, "readuint16 did not equal data.uint16test as expected.", state);

    uint32_t readuint32 = parcel.ReadUint32();
    AssertEqual(readuint32, data.uint32test, "readuint32 did not equal data.uint32test as expected.", state);
}

void ReadUnalignedTestData(Parcel &parcel, const struct TestData &data, benchmark::State& state)
{
    BENCHMARK_LOGD("ParcelTest void ReadUnalignedTestData is called.");
    bool readbool = parcel.ReadBoolUnaligned();
    AssertEqual(readbool, data.booltest, "readbool did not equal data.booltest as expected.", state);

    int8_t readint8;
    AssertTrue(parcel.ReadInt8Unaligned(readint8),
        "parcel.ReadInt8Unaligned(readint8) did not equal true as expected.", state);
    AssertEqual(readint8, data.int8test, "readint8 did not equal data.int8test as expected.", state);

    int16_t readint16;
    AssertTrue(parcel.ReadInt16Unaligned(readint16),
        "parcel.ReadInt16Unaligned(readint16) did not equal true as expected.", state);
    AssertEqual(readint16, data.int16test, "readint16 did not equal data.int16test as expected.", state);

    uint8_t readuint8;
    AssertTrue(parcel.ReadUint8Unaligned(readuint8),
        "parcel.ReadUint8Unaligned(readuint8) did not equal true as expected.", state);
    AssertEqual(readuint8, data.uint8test, "readuint8 did not equal data.uint8test as expected.", state);

    uint16_t readuint16;
    AssertTrue(parcel.ReadUint16Unaligned(readuint16),
        "parcel.ReadUint16Unaligned(readuint16) did not equal true as expected.", state);
    AssertEqual(readuint16, data.uint16test, "readuint16 did not equal data.uint16test as expected.", state);
}

void ReadTestDataWithTarget(Parcel &parcel, const struct TestData &data, benchmark::State& state)
{
    BENCHMARK_LOGD("ParcelTest void ReadTestDataWithTarget is called.");
    bool result = false;
    bool boolVal = true;
    result = parcel.ReadBool(boolVal);
    AssertEqual(result, true, "result did not equal true as expected.", state);
    AssertEqual(boolVal, data.booltest, "boolVal did not equal data.booltest as expected.", state);

    int8_t int8Val;
    result = parcel.ReadInt8(int8Val);
    AssertEqual(result, true, "result did not equal true as expected.", state);
    AssertEqual(int8Val, data.int8test, "int8Val did not equal data.int8test as expected.", state);

    int16_t int16Val;
    result = parcel.ReadInt16(int16Val);
    AssertEqual(result, true, "result did not equal true as expected.", state);
    AssertEqual(int16Val, data.int16test, "int16Val did not equal data.int16test as expected.", state);

    int32_t int32Val;
    result = parcel.ReadInt32(int32Val);
    AssertEqual(result, true, "result did not equal true as expected.", state);
    AssertEqual(int32Val, data.int32test, "int32Val did not equal data.int32test as expected.", state);

    uint8_t uint8Val;
    result = parcel.ReadUint8(uint8Val);
    AssertEqual(result, true, "result did not equal true as expected.", state);
    AssertEqual(uint8Val, data.uint8test, "uint8Val did not equal data.uint8test as expected.", state);

    uint16_t uint16Val;
    result = parcel.ReadUint16(uint16Val);
    AssertEqual(result, true, "result did not equal true as expected.", state);
    AssertEqual(uint16Val, data.uint16test, "uint16Val did not equal data.uint16test as expected.", state);

    uint32_t uint32Val;
    result = parcel.ReadUint32(uint32Val);
    AssertEqual(result, true, "result did not equal true as expected.", state);
    AssertEqual(uint32Val, data.uint32test, "uint32Val did not equal data.uint32test as expected.", state);
}

/**
 * Here to simulate the scenario of ipc sending data,
 * the buffer will be released when the Parcel object is destructed.
 */
bool SendData(void *&buffer, size_t size, const uint8_t *data)
{
    BENCHMARK_LOGD("ParcelTest bool SendData is called.");
    const size_t parcelMinSizeValue = 1;
    if (size < parcelMinSizeValue) {
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

bool CopyOldParcelToNewParcel(Parcel &oldParcel, Parcel &newParcel, benchmark::State& state)
{
    void *buffer = nullptr;
    size_t size = oldParcel.GetDataSize();

    AssertTrue(SendData(buffer, size, reinterpret_cast<const uint8_t *>(oldParcel.GetData())),
        "SendData(buffer, size, reinterpret_cast<const uint8_t *>(oldParcel.GetData())) \
        did not equal true as expected.", state);

    bool result = newParcel.ParseFrom(reinterpret_cast<uintptr_t>(buffer), size);
    AssertEqual(result, true, "result did not equal true as expected.", state);

    return true;
}

/**
 * @tc.name: test_parcel_001
 * @tc.desc: test parcel CheckOffsets, WriteRemoteObject, RewindRead and
 * RewindWrite failed.
 * @tc.type: FUNC
 */
BENCHMARK_F(BenchmarkParcelTest, test_parcel_001)(benchmark::State& state)
{
    BENCHMARK_LOGD("ParcelTest test_parcel_001 start.");
    while (state.KeepRunning()) {
        Parcel parcel;
        bool result = parcel.CheckOffsets();
        AssertEqual(result, false, "result did not equal false as expected.", state);

        result = parcel.WriteRemoteObject(nullptr);
        AssertEqual(result, false, "result did not equal false as expected.", state);

        size_t rewindReadPos = parcel.GetDataSize() + 1;
        result = parcel.RewindRead(rewindReadPos);
        AssertEqual(result, false, "result did not equal false as expected.", state);

        result = parcel.RewindWrite(rewindReadPos);
        AssertEqual(result, false, "result did not equal false as expected.", state);
    }
    BENCHMARK_LOGD("ParcelTest test_parcel_001 end.");
}

/**
 * @tc.name: test_parcel_readvec_001
 * @tc.desc: test parcel read vector failed with invlalid input.
 * @tc.type: FUNC
 */
BENCHMARK_F(BenchmarkParcelTest, test_parcel_readvec_001)(benchmark::State& state)
{
    BENCHMARK_LOGD("ParcelTest test_parcel_readvec_001 start.");
    while (state.KeepRunning()) {
        Parcel parcel;

        bool result = parcel.ReadBoolVector(nullptr);
        AssertEqual(result, false, "result did not equal false as expected.", state);

        result = parcel.ReadInt8Vector(nullptr);
        AssertEqual(result, false, "result did not equal false as expected.", state);

        result = parcel.ReadInt16Vector(nullptr);
        AssertEqual(result, false, "result did not equal false as expected.", state);

        result = parcel.ReadInt32Vector(nullptr);
        AssertEqual(result, false, "result did not equal false as expected.", state);

        result = parcel.ReadInt64Vector(nullptr);
        AssertEqual(result, false, "result did not equal false as expected.", state);

        result = parcel.ReadUInt8Vector(nullptr);
        AssertEqual(result, false, "result did not equal false as expected.", state);

        result = parcel.ReadUInt16Vector(nullptr);
        AssertEqual(result, false, "result did not equal false as expected.", state);

        result = parcel.ReadUInt32Vector(nullptr);
        AssertEqual(result, false, "result did not equal false as expected.", state);

        result = parcel.ReadUInt64Vector(nullptr);
        AssertEqual(result, false, "result did not equal false as expected.", state);

        result = parcel.ReadFloatVector(nullptr);
        AssertEqual(result, false, "result did not equal false as expected.", state);

        result = parcel.ReadDoubleVector(nullptr);
        AssertEqual(result, false, "result did not equal false as expected.", state);

        result = parcel.ReadStringVector(nullptr);
        AssertEqual(result, false, "result did not equal false as expected.", state);

        result = parcel.ReadString16Vector(nullptr);
        AssertEqual(result, false, "result did not equal false as expected.", state);
    }
    BENCHMARK_LOGD("ParcelTest test_parcel_readvec_001 end.");
}

/**
 * @tc.name: test_parcel_readvec_002
 * @tc.desc: test parcel read vector failed with invlalid vector length -1.
 * @tc.type: FUNC
 */
template <typename T1, typename F>
void WriteSignalDataToVector002(T1 &value, benchmark::State& state, const F executeFunc)
{
    int32_t writeInt32Value = -1;
    Parcel parcel;
    std::vector<T1> vec;
    vec.push_back(value);
    parcel.WriteInt32(writeInt32Value);

    bool result = executeFunc(parcel, vec);
    AssertEqual(result, false, "result did not equal false as expected.", state);
}

BENCHMARK_F(BenchmarkParcelTest, test_parcel_readvec_002)(benchmark::State& state)
{
    BENCHMARK_LOGD("ParcelTest test_parcel_readvec_002 start.");
    while (state.KeepRunning()) {
        bool writeBoolValue = true;
        WriteSignalDataToVector002(writeBoolValue, state,
            [](auto &parcel, auto &vec) {return parcel.ReadBoolVector(&vec);});

        int8_t writeInt8Value = 1;
        WriteSignalDataToVector002(writeInt8Value, state,
            [](auto &parcel, auto &vec) {return parcel.ReadInt8Vector(&vec);});

        int16_t writeInt16Value = 1;
        WriteSignalDataToVector002(writeInt16Value, state,
            [](auto &parcel, auto &vec) {return parcel.ReadInt16Vector(&vec);});

        int32_t writeInt32Value = 1;
        WriteSignalDataToVector002(writeInt32Value, state,
            [](auto &parcel, auto &vec) {return parcel.ReadInt32Vector(&vec);});

        int64_t writeInt64Value = 1;
        WriteSignalDataToVector002(writeInt64Value, state,
            [](auto &parcel, auto &vec) {return parcel.ReadInt64Vector(&vec);});

        uint8_t writeUint8Value = 1;
        WriteSignalDataToVector002(writeUint8Value, state,
            [](auto &parcel, auto &vec) {return parcel.ReadUInt8Vector(&vec);});

        uint16_t writeUint16Value = 1;
        WriteSignalDataToVector002(writeUint16Value, state,
            [](auto &parcel, auto &vec) {return parcel.ReadUInt16Vector(&vec);});

        uint32_t writeUint32Value = 1;
        WriteSignalDataToVector002(writeUint32Value, state,
            [](auto &parcel, auto &vec) {return parcel.ReadUInt32Vector(&vec);});

        uint64_t writeUint64Value = 1;
        WriteSignalDataToVector002(writeUint64Value, state,
            [](auto &parcel, auto &vec) {return parcel.ReadUInt64Vector(&vec);});

        float writeFloatValue = 1;
        WriteSignalDataToVector002(writeFloatValue, state,
            [](auto &parcel, auto &vec) {return parcel.ReadFloatVector(&vec);});

        double writeDoubleValue = 1;
        WriteSignalDataToVector002(writeDoubleValue, state,
            [](auto &parcel, auto &vec) {return parcel.ReadDoubleVector(&vec);});

        std::string writeStringValue = "test";
        WriteSignalDataToVector002(writeStringValue, state,
            [](auto &parcel, auto &vec) {return parcel.ReadStringVector(&vec);});

        std::u16string writeU16StringValue = u"test";
        WriteSignalDataToVector002(writeU16StringValue, state,
            [](auto &parcel, auto &vec) {return parcel.ReadString16Vector(&vec);});
    }
    BENCHMARK_LOGD("ParcelTest test_parcel_readvec_002 end.");
}

/**
 * @tc.name: test_parcel_readvec_003
 * @tc.desc: test parcel read vector failed with invlalid vector length
 * std::vector::max_size().
 * @tc.type: FUNC
 */
template <typename T1, typename F>
void WriteSignalDataToVector003(T1 &value, benchmark::State& state, const F executeFunc)
{
    Parcel parcel;
    std::vector<T1> vec;
    vec.push_back(value);
    parcel.WriteInt32(vec.max_size());

    bool result = executeFunc(parcel, vec);
    AssertEqual(result, false, "result did not equal false as expected.", state);
}

BENCHMARK_F(BenchmarkParcelTest, test_parcel_readvec_003)(benchmark::State& state)
{
    BENCHMARK_LOGD("ParcelTest test_parcel_readvec_003 start.");
    while (state.KeepRunning()) {
        bool writeBoolValue = true;
        WriteSignalDataToVector003(writeBoolValue, state,
            [](auto &parcel, auto &vec) {return parcel.ReadBoolVector(&vec);});

        int8_t writeInt8Value = 1;
        WriteSignalDataToVector003(writeInt8Value, state,
            [](auto &parcel, auto &vec) {return parcel.ReadInt8Vector(&vec);});

        int16_t writeInt16Value = 1;
        WriteSignalDataToVector003(writeInt16Value, state,
            [](auto &parcel, auto &vec) {return parcel.ReadInt16Vector(&vec);});

        int32_t writeInt32Value = 1;
        WriteSignalDataToVector003(writeInt32Value, state,
            [](auto &parcel, auto &vec) {return parcel.ReadInt32Vector(&vec);});

        int64_t writeInt64Value = 1;
        WriteSignalDataToVector003(writeInt64Value, state,
            [](auto &parcel, auto &vec) {return parcel.ReadInt64Vector(&vec);});

        uint8_t writeUint8Value = 1;
        WriteSignalDataToVector003(writeUint8Value, state,
            [](auto &parcel, auto &vec) {return parcel.ReadUInt8Vector(&vec);});

        uint16_t writeUint16Value = 1;
        WriteSignalDataToVector003(writeUint16Value, state,
            [](auto &parcel, auto &vec) {return parcel.ReadUInt16Vector(&vec);});

        uint32_t writeUint32Value = 1;
        WriteSignalDataToVector003(writeUint32Value, state,
            [](auto &parcel, auto &vec) {return parcel.ReadUInt32Vector(&vec);});

        uint64_t writeUint64Value = 1;
        WriteSignalDataToVector003(writeUint64Value, state,
            [](auto &parcel, auto &vec) {return parcel.ReadUInt64Vector(&vec);});

        float writeFloatValue = 1;
        WriteSignalDataToVector003(writeFloatValue, state,
            [](auto &parcel, auto &vec) {return parcel.ReadFloatVector(&vec);});

        double writeDoubleValue = 1;
        WriteSignalDataToVector003(writeDoubleValue, state,
            [](auto &parcel, auto &vec) {return parcel.ReadDoubleVector(&vec);});

        std::string writeStringValue = "test";
        WriteSignalDataToVector003(writeStringValue, state,
            [](auto &parcel, auto &vec) {return parcel.ReadStringVector(&vec);});

        std::u16string writeU16StringValue = u"test";
        WriteSignalDataToVector003(writeU16StringValue, state,
            [](auto &parcel, auto &vec) {return parcel.ReadString16Vector(&vec);});
    }
    BENCHMARK_LOGD("ParcelTest test_parcel_readvec_003 end.");
}

/**
 * @tc.name: test_parcel_writevec_001
 * @tc.desc: test parcel write vector failed with writting data out of the
 * maximum capacity.
 * @tc.type: FUNC
 */
template <typename T1, typename F>
void WriteDataToVector(T1 &value, benchmark::State& state, const F executeFunc)
{
    std::vector<T1> vec;
    for (unsigned int idx = 0; idx < DEFAULT_CPACITY / sizeof(T1); idx++) {
        vec.push_back(value);
    }
    bool result = executeFunc(vec);
    AssertEqual(result, false, "result did not equal false as expected.", state);
}

template <typename F>
void WriteDataToVector(std::string &value, benchmark::State& state, const F executeFunc)
{
    std::vector<std::string> vec;
    vec.push_back(value);

    bool result = executeFunc(vec);
    AssertEqual(result, false, "result did not equal false as expected.", state);
}

template <typename F>
void WriteDataToVector(std::u16string &value, benchmark::State& state, const F executeFunc)
{
    std::vector<std::u16string> vec;
    vec.push_back(value);

    bool result = executeFunc(vec);
    AssertEqual(result, false, "result did not equal false as expected.", state);
}

BENCHMARK_F(BenchmarkParcelTest, test_parcel_writevec_001)(benchmark::State& state)
{
    BENCHMARK_LOGD("ParcelTest test_parcel_writevec_001 start.");
    while (state.KeepRunning()) {
        Parcel parcel(nullptr);
        bool writeBoolValue = true;
        WriteDataToVector(writeBoolValue, state, [&](auto &vec) {return parcel.WriteBoolVector(vec);});

        int8_t writeInt8Value = 1;
        WriteDataToVector(writeInt8Value, state,
            [&](auto &vec) {parcel.FlushBuffer(); return parcel.WriteInt8Vector(vec);});

        int16_t writeInt16Value = 1;
        WriteDataToVector(writeInt16Value, state,
            [&](auto &vec) {parcel.FlushBuffer(); return parcel.WriteInt16Vector(vec);});

        int32_t writeInt32Value = 1;
        WriteDataToVector(writeInt32Value, state,
            [&](auto &vec) {parcel.FlushBuffer(); return parcel.WriteInt32Vector(vec);});

        int64_t writeInt64Value = 1;
        WriteDataToVector(writeInt64Value, state,
            [&](auto &vec) {parcel.FlushBuffer(); return parcel.WriteInt64Vector(vec);});

        uint8_t writeUint8Value = 1;
        WriteDataToVector(writeUint8Value, state,
            [&](auto &vec) {parcel.FlushBuffer(); return parcel.WriteUInt8Vector(vec);});

        uint16_t writeUint16Value = 1;
        WriteDataToVector(writeUint16Value, state,
            [&](auto &vec) {parcel.FlushBuffer(); return parcel.WriteUInt16Vector(vec);});

        uint32_t writeUint32Value = 1;
        WriteDataToVector(writeUint32Value, state,
            [&](auto &vec) {parcel.FlushBuffer(); return parcel.WriteUInt32Vector(vec);});

        uint64_t writeUint64Value = 1;
        WriteDataToVector(writeUint64Value, state,
            [&](auto &vec) {parcel.FlushBuffer(); return parcel.WriteUInt64Vector(vec);});

        float writeFloatValue = 1;
        WriteDataToVector(writeFloatValue, state,
            [&](auto &vec) {parcel.FlushBuffer(); return parcel.WriteFloatVector(vec);});

        double writeDoubleValue = 1;
        WriteDataToVector(writeDoubleValue, state,
            [&](auto &vec) {parcel.FlushBuffer(); return parcel.WriteDoubleVector(vec);});

        char insertCharValue = 't';
        std::string x12((DEFAULT_CPACITY / sizeof(char)), insertCharValue);
        WriteDataToVector(x12, state,
            [&](auto &vec) {parcel.FlushBuffer(); return parcel.WriteStringVector(vec);});

        char16_t insertCharTValue = u't';
        std::u16string x13((DEFAULT_CPACITY / sizeof(char16_t)), insertCharTValue);
        WriteDataToVector(x13, state,
            [&] (auto &vec) {parcel.FlushBuffer(); return parcel.WriteString16Vector(vec);});
    }
    BENCHMARK_LOGD("ParcelTest test_parcel_writevec_001 end.");
}

/**
 * @tc.name: test_parcel_SetMaxCapacity_001
 * @tc.desc: test parcel primary type read write.
 * @tc.type: FUNC
 */
BENCHMARK_F(BenchmarkParcelTest, test_parcel_SetMaxCapacity_001)(benchmark::State& state)
{
    BENCHMARK_LOGD("ParcelTest test_parcel_SetMaxCapacity_001 start.");
    const size_t setMaxCapacityValue = 1;
    while (state.KeepRunning()) {
        size_t cap = DEFAULT_CPACITY;
        Parcel parcel(nullptr);
        AssertTrue(parcel.SetMaxCapacity(cap + setMaxCapacityValue),
            "parcel.SetMaxCapacity(cap + 1) did not equal true as expected.", state);
        AssertFalse(parcel.SetMaxCapacity(cap - setMaxCapacityValue),
            "parcel.SetMaxCapacity(cap - 1) did not equal false as expected.", state);
    }
    BENCHMARK_LOGD("ParcelTest test_parcel_SetMaxCapacity_001 end.");
}

/**
 * @tc.name: test_parcel_SetAllocator_001
 * @tc.desc: test setting allocator to parcels with and without existed allocator.
 * @tc.type: FUNC
 */
BENCHMARK_F(BenchmarkParcelTest, test_parcel_SetAllocator_001)(benchmark::State& state)
{
    BENCHMARK_LOGD("ParcelTest test_parcel_SetAllocator_001 start.");
    while (state.KeepRunning()) {
        Allocator* alloc = new DefaultAllocator();
        Parcel parcel(alloc);
        AssertFalse(parcel.SetAllocator(alloc),
            "parcel.SetAllocator(alloc) did not equal false as expected.", state);
        AssertFalse(parcel.SetAllocator(nullptr),
            "parcel.SetAllocator(nullptr) did not equal false as expected.", state);

        struct TestData data = { true, -0x34, 0x5634, -0x12345678, 0x34, 0x5634, 0x12345678 };

        WriteTestData(parcel, data, state);
        parcel.SetAllocator(new DefaultAllocator());
        ReadTestData(parcel, data, state);
    }
    BENCHMARK_LOGD("ParcelTest test_parcel_SetAllocator_001 end.");
}

/**
 * @tc.name: test_parcel_write_001
 * @tc.desc: test parcel write failed.
 * @tc.type: FUNC
 */
template <typename T1, typename F>
void TestParcelWrite001(T1 value, benchmark::State& state, const F executeFunc)
{
    bool result = executeFunc(value);
    AssertEqual(result, false, "result did not equal false as expected.", state);
}

BENCHMARK_F(BenchmarkParcelTest, test_parcel_write_001)(benchmark::State& state)
{
    BENCHMARK_LOGD("ParcelTest test_parcel_write_001 start.");
    while (state.KeepRunning()) {
        Parcel parcel1;
        parcel1.WriteBool(true);
        Parcel parcel2;
        AssertTrue(CopyOldParcelToNewParcel(parcel1, parcel2, state),
            "CopyOldParcelToNewParcel(parcel1, parcel2, state) did not equal true as expected.", state);

        string str8write;
        TestParcelWrite001(str8write, state, [&](auto &value) {return parcel2.WriteString(value);});
        u16string str16Write;
        TestParcelWrite001(str16Write, state, [&](auto &value) {return parcel2.WriteString16(value);});

        TestParcelWrite001(false, state, [&](auto &value) {return parcel2.WriteBool(value);});
        TestParcelWrite001(false, state, [&](auto &value) {return parcel2.WriteBoolUnaligned(value);});

        TestParcelWrite001(false, state, [&](auto &value) {return parcel2.WriteInt8(value);});
        TestParcelWrite001(false, state, [&](auto &value) {return parcel2.WriteInt8Unaligned(value);});

        TestParcelWrite001(false, state, [&](auto &value) {return parcel2.WriteInt32(value);});

        TestParcelWrite001(false, state, [&](auto &value) {return parcel2.WriteInt64(value);});

        TestParcelWrite001(false, state, [&](auto &value) {return parcel2.WriteUint8(value);});

        TestParcelWrite001(false, state, [&](auto &value) {return parcel2.WriteUint16(value);});

        TestParcelWrite001(false, state, [&](auto &value) {return parcel2.WriteUint8Unaligned(value);});

        TestParcelWrite001(false, state, [&](auto &value) {return parcel2.WriteUint16Unaligned(value);});

        TestParcelWrite001(false, state, [&](auto &value) {return parcel2.WriteUint32(value);});

        TestParcelWrite001(false, state, [&](auto &value) {return parcel2.WriteUint64(value);});

        TestParcelWrite001(false, state, [&](auto &value) {return parcel2.WriteFloat(value);});

        TestParcelWrite001(false, state, [&](auto &value) {return parcel2.WriteDouble(value);});

        TestParcelWrite001(false, state, [&](auto &value) {return parcel2.WritePointer(value);});
    }
    BENCHMARK_LOGD("ParcelTest test_parcel_write_001 end.");
}

/**
 * @tc.name: test_parcel_WriteAndRead_001
 * @tc.desc: test parcel primary type read write.
 * @tc.type: FUNC
 */
BENCHMARK_F(BenchmarkParcelTest, test_parcel_WriteAndRead_001)(benchmark::State& state)
{
    BENCHMARK_LOGD("ParcelTest test_parcel_WriteAndRead_001 start.");
    while (state.KeepRunning()) {
        Parcel parcel(nullptr);
        struct TestData data = { true, -0x34, 0x5634, -0x12345678, 0x34, 0x5634, 0x12345678 };
        WriteTestData(parcel, data, state);
        ReadTestData(parcel, data, state);

        WriteUnalignedTestData(parcel, data, state);
        ReadUnalignedTestData(parcel, data, state);
    }
    BENCHMARK_LOGD("ParcelTest test_parcel_WriteAndRead_001 end.");
}

/**
 * @tc.name: test_parcel_WriteAndRead_002
 * @tc.desc: test parcel primary type read write.
 * @tc.type: FUNC
 */
BENCHMARK_F(BenchmarkParcelTest, test_parcel_WriteAndRead_002)(benchmark::State& state)
{
    BENCHMARK_LOGD("ParcelTest test_parcel_WriteAndRead_002 start.");
    while (state.KeepRunning()) {
        Parcel parcel1(nullptr);
        Parcel parcel2(nullptr);
        struct TestData data = { true, -0x34, 0x5634, -0x12345678, 0x34, 0x5634, 0x12345678 };
        WriteTestData(parcel1, data, state);
        WriteUnalignedTestData(parcel1, data, state);

        AssertTrue(CopyOldParcelToNewParcel(parcel1, parcel2, state),
            "CopyOldParcelToNewParcel(parcel1, parcel2, state) did not equal true as expected.", state);
        ReadTestData(parcel2, data, state);
        ReadUnalignedTestData(parcel2, data, state);
    }
    BENCHMARK_LOGD("ParcelTest test_parcel_WriteAndRead_002 end.");
}

/**
 * @tc.name: test_parcel_WriteAndRead_003
 * @tc.desc: test parcel primary type read write.
 * @tc.type: FUNC
 */
BENCHMARK_F(BenchmarkParcelTest, test_parcel_WriteAndRead_003)(benchmark::State& state)
{
    BENCHMARK_LOGD("ParcelTest test_parcel_WriteAndRead_003 start.");
    while (state.KeepRunning()) {
        Parcel parcel1(nullptr);
        Parcel parcel2(nullptr);
        struct TestData data = { true, -0x34, 0x5634, -0x12345678, 0x34, 0x5634, 0x12345678 };
        WriteTestData(parcel1, data, state);

        AssertTrue(CopyOldParcelToNewParcel(parcel1, parcel2, state),
            "CopyOldParcelToNewParcel(parcel1, parcel2, state) did not equal true as expected.", state);
        ReadTestDataWithTarget(parcel2, data, state);
    }
    BENCHMARK_LOGD("ParcelTest test_parcel_WriteAndRead_003 end.");
}

/**
 * @tc.name: test_parcel_WriteAndRead_004
 * @tc.desc: test parcel primary type read write.
 * @tc.type: FUNC
 */
BENCHMARK_F(BenchmarkParcelTest, test_parcel_WriteAndRead_004)(benchmark::State& state)
{
    BENCHMARK_LOGD("ParcelTest test_parcel_WriteAndRead_004 start.");
    while (state.KeepRunning()) {
        Parcel parcel1(nullptr);
        int64_t int64test = -0x1234567887654321;
        bool result = parcel1.WriteInt64(int64test);
        AssertEqual(result, true, "result did not equal true as expected.", state);

        uint64_t uint64test = 0x1234567887654321;
        result = parcel1.WriteUint64(uint64test);
        AssertEqual(result, true, "result did not equal true as expected.", state);

        int64_t readint64 = parcel1.ReadInt64();
        AssertEqual(readint64, int64test, "readint64 did not equal int64test as expected.", state);

        uint64_t readuint64 = parcel1.ReadUint64();
        AssertEqual(readuint64, uint64test, "readuint64 did not equal uint64test as expected.", state);

        Parcel parcel2(nullptr);
        AssertTrue(CopyOldParcelToNewParcel(parcel1, parcel2, state),
            "CopyOldParcelToNewParcel(parcel1, parcel2, state) did not equal true as expected.", state);

        readint64 = parcel2.ReadInt64();
        AssertEqual(readint64, int64test, "readint64 did not equal int64test as expected.", state);

        readuint64 = parcel2.ReadUint64();
        AssertEqual(readuint64, uint64test, "readuint64 did not equal uint64test as expected.", state);
    }
    BENCHMARK_LOGD("ParcelTest test_parcel_WriteAndRead_004 end.");
}

/**
 * @tc.name: test_parcel_WriteAndRead_String_001
 * @tc.desc: test parcel string read write.
 * @tc.type: FUNC
 */
BENCHMARK_F(BenchmarkParcelTest, test_parcel_WriteAndRead_String_001)(benchmark::State& state)
{
    BENCHMARK_LOGD("ParcelTest test_parcel_WriteAndRead_String_001 start.");
    while (state.KeepRunning()) {
        Parcel parcel1(nullptr);
        string strWrite = "test";
        bool result = parcel1.WriteString(strWrite);
        AssertEqual(result, true, "result did not equal true as expected.", state);
        string strWrite1 =
            "test for write string padded**********************************************************##################";
        result = parcel1.WriteString(strWrite1);
        AssertEqual(result, true, "result did not equal true as expected.", state);

        string strWrite2 =
            "test for write string padded**********************************************************##################";
        result = parcel1.WriteString(strWrite2);
        AssertEqual(result, true, "result did not equal true as expected.", state);
        string strRead = parcel1.ReadString();
        string strRead1 = parcel1.ReadString();
        string strRead2 = parcel1.ReadString();
        AssertEqual(COMPARE_STRING_RESULT, strcmp(strRead.c_str(), strWrite.c_str()),
            "strcmp(strRead.c_str(), strWrite.c_str()) did not equal 0 as expected.", state);
        AssertEqual(COMPARE_STRING_RESULT, strcmp(strRead1.c_str(), strWrite1.c_str()),
            "strcmp(strRead1.c_str(), strWrite1.c_str()) did not equal 0 as expected.", state);
        AssertEqual(COMPARE_STRING_RESULT, strcmp(strRead2.c_str(), strWrite2.c_str()),
            "strcmp(strRead2.c_str(), strWrite2.c_str()) did not equal 0 as expected.", state);

        Parcel parcel2(nullptr);
        AssertTrue(CopyOldParcelToNewParcel(parcel1, parcel2, state),
            "CopyOldParcelToNewParcel(parcel1, parcel2, state) did not equal true as expected.", state);

        strRead = parcel2.ReadString();
        strRead1 = parcel2.ReadString();
        strRead2 = parcel2.ReadString();
        AssertEqual(COMPARE_STRING_RESULT, strcmp(strRead.c_str(), strWrite.c_str()),
            "strcmp(strRead.c_str(), strWrite.c_str()) did not equal 0 as expected.", state);
        AssertEqual(COMPARE_STRING_RESULT, strcmp(strRead1.c_str(), strWrite1.c_str()),
            "strcmp(strRead1.c_str(), strWrite1.c_str()) did not equal 0 as expected.", state);
        AssertEqual(COMPARE_STRING_RESULT, strcmp(strRead2.c_str(), strWrite2.c_str()),
            "strcmp(strRead2.c_str(), strWrite2.c_str()) did not equal 0 as expected.", state);
    }
    BENCHMARK_LOGD("ParcelTest test_parcel_WriteAndRead_String_001 end.");
}

/**
 * @tc.name: test_parcel_WriteAndRead_String_002
 * @tc.desc: test parcel string read write.
 * @tc.type: FUNC
 */
BENCHMARK_F(BenchmarkParcelTest, test_parcel_WriteAndRead_String_002)(benchmark::State& state)
{
    BENCHMARK_LOGD("ParcelTest test_parcel_WriteAndRead_String_002 start.");
    while (state.KeepRunning()) {
        Parcel parcel1(nullptr);
        u16string str16Write = u"12345";
        bool result = parcel1.WriteString16(str16Write);
        AssertEqual(result, true, "result did not equal true as expected.", state);

        u16string str16Write2 = u"12345 test for write16string padded*********";
        result = parcel1.WriteString16(str16Write2);
        AssertEqual(result, true, "result did not equal true as expected.", state);

        u16string str16Read = parcel1.ReadString16();
        u16string str16Read2 = parcel1.ReadString16();
        AssertEqual(COMPARE_STRING_RESULT, str16Read.compare(str16Write),
            "str16Read.compare(str16Write) did not equal 0 as expected.", state);
        AssertEqual(COMPARE_STRING_RESULT, str16Read2.compare(str16Write2),
            "str16Read2.compare(str16Write2) did not equal 0 as expected.", state);

        Parcel parcel2(nullptr);
        AssertTrue(CopyOldParcelToNewParcel(parcel1, parcel2, state),
            "CopyOldParcelToNewParcel(parcel1, parcel2, state) did not equal true as expected.", state);

        str16Read = parcel2.ReadString16();
        str16Read2 = parcel2.ReadString16();
        AssertEqual(COMPARE_STRING_RESULT, str16Read.compare(str16Write),
            "str16Read.compare(str16Write) did not equal 0 as expected.", state);
        AssertEqual(COMPARE_STRING_RESULT, str16Read2.compare(str16Write2),
            "str16Read2.compare(str16Write2) did not equal 0 as expected.", state);
    }
    BENCHMARK_LOGD("ParcelTest test_parcel_WriteAndRead_String_002 end.");
}

/**
 * @tc.name: test_parcel_WriteAndRead_String_003
 * @tc.desc: test parcel CString read write.
 * @tc.type: FUNC
 */
BENCHMARK_F(BenchmarkParcelTest, test_parcel_WriteAndRead_String_003)(benchmark::State& state)
{
    BENCHMARK_LOGD("ParcelTest test_parcel_WriteAndRead_String_003 start.");
    while (state.KeepRunning()) {
        Parcel parcel(nullptr);
        string test1 = "12345";
        string test2 = "23456";
        string test3 = "34567";
        string test4 = "45678";
        bool result = parcel.WriteCString(nullptr);
        AssertEqual(result, false, "result did not equal false as expected.", state);

        result = parcel.WriteCString(test1.c_str());
        AssertEqual(result, true, "result did not equal true as expected.", state);

        result = parcel.WriteCString(test2.c_str());
        AssertEqual(result, true, "result did not equal true as expected.", state);

        result = parcel.WriteCString(test3.c_str());
        AssertEqual(result, true, "result did not equal true as expected.", state);

        result = parcel.WriteCString(test4.c_str());
        AssertEqual(result, true, "result did not equal true as expected.", state);

        AssertEqual(COMPARE_STRING_RESULT, strcmp(test1.c_str(), parcel.ReadCString()),
            "strcmp(test1.c_str(), parcel.ReadCString()) did not equal 0 as expected.", state);
        AssertEqual(COMPARE_STRING_RESULT, strcmp(test2.c_str(), parcel.ReadCString()),
            "strcmp(test2.c_str(), parcel.ReadCString()) did not equal 0 as expected.", state);
        AssertEqual(COMPARE_STRING_RESULT, strcmp(test3.c_str(), parcel.ReadCString()),
            "strcmp(test3.c_str(), parcel.ReadCString()) did not equal 0 as expected.", state);
        AssertEqual(COMPARE_STRING_RESULT, strcmp(test4.c_str(), parcel.ReadCString()),
            "strcmp(test4.c_str(), parcel.ReadCString()) did not equal 0 as expected.", state);
    }
    BENCHMARK_LOGD("ParcelTest test_parcel_WriteAndRead_String_003 end.");
}

/**
 * @tc.name: test_parcel_WriteAndRead_String004
 * @tc.desc: test parcel CString read write.
 * @tc.type: FUNC
 */
BENCHMARK_F(BenchmarkParcelTest, test_parcel_WriteAndRead_String004)(benchmark::State& state)
{
    BENCHMARK_LOGD("ParcelTest test_parcel_WriteAndRead_String004 start.");
    while (state.KeepRunning()) {
        Parcel parcel1(nullptr);
        bool result = false;
        size_t writeString16Length = 0;
        // write from Java, read from C++
        result = parcel1.WriteString16WithLength(nullptr, writeString16Length);
        AssertEqual(result, true, "result did not equal true as expected.", state);

        u16string str16write = u"12345";
        char16_t *value1 = str16write.data();
        result = parcel1.WriteString16WithLength(value1, str16write.length());
        AssertEqual(result, true, "result did not equal true as expected.", state);

        u16string str16write2 = u"12345 test for write16string padded*********";
        char16_t *value2 = str16write2.data();
        result = parcel1.WriteString16WithLength(value2, str16write2.length());
        AssertEqual(result, true, "result did not equal true as expected.", state);

        u16string str16readNull = parcel1.ReadString16();
        u16string str16read1 = parcel1.ReadString16();
        u16string str16read2 = parcel1.ReadString16();
        AssertEqual(COMPARE_STRING_RESULT, str16readNull.compare(std::u16string()),
            "str16readNull.compare(std::u16string()) did not equal 0 as expected.", state);
        AssertEqual(COMPARE_STRING_RESULT, str16read1.compare(str16write),
            "str16read1.compare(str16write) did not equal 0 as expected.", state);
        AssertEqual(COMPARE_STRING_RESULT, str16read2.compare(str16write2),
            "str16read2.compare(str16write2) did not equal 0 as expected.", state);

        // write from C++, read from Java
        result = parcel1.WriteString16(str16write);
        AssertEqual(result, true, "result did not equal true as expected.", state);

        result = parcel1.WriteString16(str16write2);
        AssertEqual(result, true, "result did not equal true as expected.", state);

        int32_t readLength1 = 0;
        u16string str16read3 = parcel1.ReadString16WithLength(readLength1);
        AssertEqual(readLength1, static_cast<int32_t>(str16write.length()),
            "readLength1 did not equal static_cast<int32_t>(str16write.length()) as expected.", state);

        int32_t readLength2 = 0;
        u16string str16read4 = parcel1.ReadString16WithLength(readLength2);
        AssertEqual(readLength2, static_cast<int32_t>(str16write2.length()),
            "readLength2 did not equal static_cast<int32_t>(str16write2.length()) as expected.", state);

        AssertEqual(COMPARE_STRING_RESULT, str16read3.compare(str16write),
            "str16read3.compare(str16write) did not equal 0 as expected.", state);

        AssertEqual(COMPARE_STRING_RESULT, str16read4.compare(str16write2),
            "str16read4.compare(str16write2) did not equal 0 as expected.", state);
    }
    BENCHMARK_LOGD("ParcelTest test_parcel_WriteAndRead_String004 end.");
}

/**
 * @tc.name: test_parcel_WriteAndRead_String005
 * @tc.desc: test parcel CString read write.
 * @tc.type: FUNC
 */
BENCHMARK_F(BenchmarkParcelTest, test_parcel_WriteAndRead_String005)(benchmark::State& state)
{
    BENCHMARK_LOGD("ParcelTest test_parcel_WriteAndRead_String005 start.");
    while (state.KeepRunning()) {
        Parcel parcel1(nullptr);
        bool result = false;
        size_t writeString8Length = 0;
        // write from Java, read from C++
        result = parcel1.WriteString8WithLength(nullptr, writeString8Length);
        AssertEqual(result, true, "result did not equal true as expected.", state);

        string str8write = "12345";
        char *value1 = str8write.data();
        result = parcel1.WriteString8WithLength(value1, str8write.length());
        AssertEqual(result, true, "result did not equal true as expected.", state);

        string str8write2 = "12345 test for write16string padded*********";
        char *value2 = str8write2.data();
        result = parcel1.WriteString8WithLength(value2, str8write2.length());
        AssertEqual(result, true, "result did not equal true as expected.", state);

        string str8readNull = parcel1.ReadString();
        string str8read1 = parcel1.ReadString();
        string str8read2 = parcel1.ReadString();
        AssertEqual(COMPARE_STRING_RESULT, str8readNull.compare(std::string()),
            "str8readNull.compare(std::string()) did not equal 0 as expected.", state);
        AssertEqual(COMPARE_STRING_RESULT, str8read1.compare(str8write),
            "str8read1.compare(str8write) did not equal 0 as expected.", state);
        AssertEqual(COMPARE_STRING_RESULT, str8read2.compare(str8write2),
            "str8read2.compare(str8write2) did not equal 0 as expected.", state);

        // write from C++, read from Java
        result = parcel1.WriteString(str8write);
        AssertEqual(result, true, "result did not equal true as expected.", state);

        result = parcel1.WriteString(str8write2);
        AssertEqual(result, true, "result did not equal true as expected.", state);

        int32_t readLength1 = 0;
        string str8read3 = parcel1.ReadString8WithLength(readLength1);
        AssertEqual(readLength1, static_cast<int32_t>(str8write.length()),
            "readLength1 did not equal static_cast<int32_t>(str8write.length()) as expected.", state);

        int32_t readLength2 = 0;
        string str8read4 = parcel1.ReadString8WithLength(readLength2);
        AssertEqual(readLength2, static_cast<int32_t>(str8write2.length()),
            "readLength2 did not equal static_cast<int32_t>(str8write2.length()) as expected.", state);
        AssertEqual(COMPARE_STRING_RESULT, str8read3.compare(str8write),
            "str8read3.compare(str8write) did not equal 0 as expected.", state);
        AssertEqual(COMPARE_STRING_RESULT, str8read4.compare(str8write2),
            "str8read4.compare(str8write2) did not equal 0 as expected.", state);
    }
    BENCHMARK_LOGD("ParcelTest test_parcel_WriteAndRead_String005 end.");
}

/**
 * @tc.name: test_parcel_WriteAndRead_Float_001
 * @tc.desc: test parcel float types read write.
 * @tc.type: FUNC
 */
BENCHMARK_F(BenchmarkParcelTest, test_parcel_WriteAndRead_Float_001)(benchmark::State& state)
{
    BENCHMARK_LOGD("ParcelTest test_parcel_WriteAndRead_Float_001 start.");
    while (state.KeepRunning()) {
        Parcel parcel1(nullptr);
        float floatwrite = 12.345678f;
        bool result = parcel1.WriteFloat(floatwrite);
        AssertEqual(result, true, "result did not equal true as expected.", state);

        double doublewrite = 1345.7653;
        result = parcel1.WriteDouble(doublewrite);
        AssertEqual(result, true, "result did not equal true as expected.", state);

        float floatread;
        result = parcel1.ReadFloat(floatread);
        AssertEqual(result, true, "result did not equal true as expected.", state);
        AssertEqual(floatwrite, floatread, "floatwrite did not equal floatread as expected.", state);

        double doubleread;
        doubleread = parcel1.ReadDouble();
        AssertEqual(doublewrite, doubleread, "doublewrite did not equal doubleread as expected.", state);

        Parcel parcel2(nullptr);
        AssertTrue(CopyOldParcelToNewParcel(parcel1, parcel2, state),
            "CopyOldParcelToNewParcel(parcel1, parcel2, state) did not equal true as expected.", state);

        result = parcel2.ReadFloat(floatread);
        AssertEqual(result, true, "result did not equal true as expected.", state);
        AssertEqual(floatwrite, floatread, "floatwrite did not equal floatread as expected.", state);

        doubleread = parcel2.ReadDouble();
        AssertEqual(doublewrite, doubleread, "doublewrite did not equal doubleread as expected.", state);
    }
    BENCHMARK_LOGD("ParcelTest test_parcel_WriteAndRead_Float_001 end.");
}

/**
 * @tc.name: test_parcel_WriteAndRead_String_005
 * @tc.desc: test parcel String type read write.
 * @tc.type: FUNC
 */
BENCHMARK_F(BenchmarkParcelTest, test_parcel_WriteAndRead_String_005)(benchmark::State& state)
{
    BENCHMARK_LOGD("ParcelTest test_parcel_WriteAndRead_String_005 start.");
    while (state.KeepRunning()) {
        Parcel parcel1(nullptr);
        string strwrite = "test";
        bool result = parcel1.WriteString(strwrite);
        AssertEqual(result, true, "result did not equal true as expected.", state);
        string strwrite1 =
            "test for write string padded**********************************************************##################";
        result = parcel1.WriteString(strwrite1);
        AssertEqual(result, true, "result did not equal true as expected.", state);
        string strwrite2 =
            "test for write string padded**********************************************************##################";
        result = parcel1.WriteString(strwrite2);
        AssertEqual(result, true, "result did not equal true as expected.", state);

        string strread;
        string strread1;
        string strread2;
        result = parcel1.ReadString(strread);
        AssertEqual(result, true, "result did not equal true as expected.", state);
        result = parcel1.ReadString(strread1);
        AssertEqual(result, true, "result did not equal true as expected.", state);
        result = parcel1.ReadString(strread2);
        AssertEqual(result, true, "result did not equal true as expected.", state);

        AssertEqual(COMPARE_STRING_RESULT, strcmp(strread.c_str(), strwrite.c_str()),
            "strcmp(strread.c_str(), strwrite.c_str()) did not equal 0 as expected.", state);
        AssertEqual(COMPARE_STRING_RESULT, strcmp(strread1.c_str(), strwrite1.c_str()),
            "strcmp(strread1.c_str(), strwrite1.c_str()) did not equal 0 as expected.", state);
        AssertEqual(COMPARE_STRING_RESULT, strcmp(strread2.c_str(), strwrite2.c_str()),
            "strcmp(strread2.c_str(), strwrite2.c_str()) did not equal 0 as expected.", state);

        Parcel parcel2(nullptr);
        AssertTrue(CopyOldParcelToNewParcel(parcel1, parcel2, state),
            "CopyOldParcelToNewParcel(parcel1, parcel2, state) did not equal true as expected.", state);

        result = parcel2.ReadString(strread);
        AssertEqual(result, true, "result did not equal true as expected.", state);
        result = parcel2.ReadString(strread1);
        AssertEqual(result, true, "result did not equal true as expected.", state);
        result = parcel2.ReadString(strread2);
        AssertEqual(result, true, "result did not equal true as expected.", state);

        AssertEqual(COMPARE_STRING_RESULT, strcmp(strread.c_str(), strwrite.c_str()),
            "strcmp(strread.c_str(), strwrite.c_str()) did not equal 0 as expected.", state);
        AssertEqual(COMPARE_STRING_RESULT, strcmp(strread1.c_str(), strwrite1.c_str()),
            "strcmp(strread1.c_str(), strwrite1.c_str()) did not equal 0 as expected.", state);
        AssertEqual(COMPARE_STRING_RESULT, strcmp(strread2.c_str(), strwrite2.c_str()),
            "strcmp(strread2.c_str(), strwrite2.c_str()) did not equal 0 as expected.", state);
    }
    BENCHMARK_LOGD("ParcelTest test_parcel_WriteAndRead_String_005 end.");
}

struct Padded {
    char title;
    int32_t handle;
    uint64_t cookie;
};

struct Unpadded {
    char tip;
};

void ValidatePadded(const struct Padded &left, const struct Padded &right, benchmark::State& state)
{
    BENCHMARK_LOGD("ParcelTest void ValidatePadded is called.");
    AssertEqual(left.title, right.title, "left.title did not equal right.title as expected.", state);

    AssertEqual(left.handle, right.handle, "left.handle did not equal right.handle as expected.", state);

    AssertEqual(left.cookie, right.cookie, "left.cookie did not equal right.cookie as expected.", state);
}

void ValidateUnpadded(const struct Unpadded &left, const struct Unpadded &right, benchmark::State& state)
{
    BENCHMARK_LOGD("ParcelTest void ValidateUnpadded is called.");
    AssertEqual(left.tip, right.tip, "left.tip did not equal right.tip as expected.", state);
}

/**
 * @tc.name: test_CalcNewCapacity_001
 * @tc.desc: test kinds of input to CalcNewCapacity.
 * @tc.type: FUNC
 */
BENCHMARK_F(BenchmarkParcelTest, test_CalcNewCapacity_001)(benchmark::State& state)
{
    BENCHMARK_LOGD("ParcelTest test_CalcNewCapacity_001 start.");
    while (state.KeepRunning()) {
        Parcel parcel;
        size_t newMaxCapacity;
        size_t minNewCapacity = CAPACITY_THRESHOLD;
        const string strLenThreshd = string(minNewCapacity, 't');
        bool ret = parcel.WriteUnpadBuffer(static_cast<const void *>(strLenThreshd.data()), minNewCapacity);
        AssertEqual(ret, true, "ret did not equal true as expected.", state);

        newMaxCapacity = CAPACITY_THRESHOLD - 1;
        minNewCapacity = newMaxCapacity;
        const string strLessThreshd = string(minNewCapacity, 'l');
        parcel.SetMaxCapacity(newMaxCapacity);
        ret = parcel.WriteUnpadBuffer(static_cast<const void *>(strLessThreshd.data()), minNewCapacity);
        AssertEqual(ret, true, "ret did not equal true as expected.", state);

        newMaxCapacity = -1; // minNewCapacity = CAPACITY_THRESHOLD - 1
        const string strNoMaxCap = string(minNewCapacity, 'n');
        parcel.SetMaxCapacity(newMaxCapacity);
        ret = parcel.WriteUnpadBuffer(static_cast<const void *>(strNoMaxCap.data()), minNewCapacity);
        AssertEqual(ret, true, "ret did not equal true as expected.", state);

        minNewCapacity = CAPACITY_THRESHOLD + 1; // newMaxCapacity = -1
        const string strExtThreshd = string(minNewCapacity, 'e');
        parcel.SetMaxCapacity(newMaxCapacity);
        ret = parcel.WriteUnpadBuffer(static_cast<const void *>(strExtThreshd.data()), minNewCapacity);
        AssertEqual(ret, true, "ret did not equal true as expected.", state);

        newMaxCapacity = CAPACITY_THRESHOLD; // minNewCapacity = CAPACITY_THRESHOLD + 1
        const string strCapThreshd = string(minNewCapacity, 'e');
        parcel.SetMaxCapacity(newMaxCapacity);
        ret = parcel.WriteUnpadBuffer(static_cast<const void *>(strCapThreshd.data()), minNewCapacity);
        AssertEqual(ret, true, "ret did not equal true as expected.", state);
    }
    BENCHMARK_LOGD("ParcelTest test_CalcNewCapacity_001 end.");
}

/**
 * @tc.name: test_SetDataCapacity_001
 * @tc.desc: test kinds of input to SetDataCapacity.
 * @tc.type: FUNC
 */
BENCHMARK_F(BenchmarkParcelTest, test_SetDataCapacity_001)(benchmark::State& state)
{
    BENCHMARK_LOGD("ParcelTest test_SetDataCapacity_001 start.");
    while (state.KeepRunning()) {
        Parcel parcel;
        struct TestData data = { true, -0x34, 0x5634, -0x12345678, 0x34, 0x5634, 0x12345678 };
        WriteTestData(parcel, data, state);

        bool result = parcel.SetDataCapacity(0);
        AssertFalse(result, "result did not equal false as expected.", state);
    }
    BENCHMARK_LOGD("ParcelTest test_SetDataCapacity_001 end.");
}

/**
 * @tc.name: test_SetDataSize_001
 * @tc.desc: test kinds of input to SetDataSize.
 * @tc.type: FUNC
 */
BENCHMARK_F(BenchmarkParcelTest, test_SetDataSize_001)(benchmark::State& state)
{
    BENCHMARK_LOGD("ParcelTest test_SetDataSize_001 start.");
    while (state.KeepRunning()) {
        Parcel parcel;
        bool result = parcel.SetDataCapacity(sizeof(bool));
        AssertEqual(result, true, "result did not equal true as expected.", state);

        result = parcel.WriteBool(true);
        AssertEqual(result, true, "result did not equal true as expected.", state);

        result = parcel.SetDataSize(DEFAULT_CPACITY + 1);
        AssertEqual(result, false, "result did not equal false as expected.", state);
    }
    BENCHMARK_LOGD("ParcelTest test_SetDataSize_001 end.");
}

/**
 * @tc.name: test_parcel_Data_Structure_001
 * @tc.desc: test parcel struct data related function.
 * @tc.type: FUNC
 */
BENCHMARK_F(BenchmarkParcelTest, test_parcel_Data_Structure_001)(benchmark::State& state)
{
    BENCHMARK_LOGD("ParcelTest test_parcel_Data_Structure_001 start.");
    while (state.KeepRunning()) {
        Parcel parcel(nullptr);
        const struct Padded pad = { 'p', 0x34567890, -0x2345678998765432 };
        const struct Unpadded unpad = { 'u' };

        bool result = parcel.WriteBuffer(static_cast<const void *>(&pad), sizeof(struct Padded));
        AssertEqual(result, true, "result did not equal true as expected.", state);

        const struct Padded *padRead =
            reinterpret_cast<const struct Padded *>(parcel.ReadBuffer(sizeof(struct Padded)));
        ValidatePadded(*padRead, pad, state);
        AssertEqual(parcel.GetWritePosition(), parcel.GetReadPosition(),
            "parcel.GetWritePosition() did not equal parcel.GetReadPosition() as expected.", state);

        result = parcel.WriteBuffer(static_cast<const void *>(&unpad), sizeof(struct Unpadded));
        const struct Unpadded *unpadRead =
            reinterpret_cast<const struct Unpadded *>(parcel.ReadBuffer(sizeof(struct Unpadded)));
        ValidateUnpadded(*unpadRead, unpad, state);
        AssertUnequal(parcel.GetWritePosition(), parcel.GetReadPosition(),
            "parcel.GetWritePosition() was not different from parcel.GetReadPosition() as expected.", state);

        parcel.RewindRead(REWIND_INIT_VALUE);
        parcel.RewindWrite(REWIND_INIT_VALUE);
        AssertEqual(parcel.GetWritePosition(), parcel.GetReadPosition(),
            "parcel.GetWritePosition() did not equal parcel.GetReadPosition() as expected.", state);

        result = parcel.WriteUnpadBuffer(static_cast<const void *>(&pad), sizeof(struct Padded));
        AssertEqual(result, true, "result did not equal true as expected.", state);

        const struct Padded *padReadNew =
            reinterpret_cast<const struct Padded *>(parcel.ReadUnpadBuffer(sizeof(struct Padded)));
        ValidatePadded(*padReadNew, pad, state);
        AssertEqual(parcel.GetWritePosition(), parcel.GetReadPosition(),
            "parcel.GetWritePosition() did not equal parcel.GetReadPosition() as expected.", state);

        result = parcel.WriteUnpadBuffer(static_cast<const void *>(&unpad), sizeof(struct Unpadded));
        AssertEqual(result, true, "result did not equal true as expected.", state);

        const struct Unpadded *unpadReadNew =
            reinterpret_cast<const struct Unpadded *>(parcel.ReadUnpadBuffer(sizeof(struct Unpadded)));
        ValidateUnpadded(*unpadReadNew, unpad, state);
        AssertEqual(parcel.GetWritePosition(), parcel.GetReadPosition(),
            "parcel.GetWritePosition() did not equal parcel.GetReadPosition() as expected.", state);
    }
    BENCHMARK_LOGD("ParcelTest test_parcel_Data_Structure_001 end.");
}

/**
 * @tc.name: test_parcel_Data_Structure_002
 * @tc.desc: test invalid input to WriteBuffer and WriteBufferAddTerminator.
 * @tc.type: FUNC
 */
BENCHMARK_F(BenchmarkParcelTest, test_parcel_Data_Structure_002)(benchmark::State& state)
{
    BENCHMARK_LOGD("ParcelTest test_parcel_Data_Structure_002 start.");
    while (state.KeepRunning()) {
        Parcel parcel(nullptr);
        const string str = "test invalid input";
        const string strOverflow = "test write with SIZE_MAX bytes";
        const string strWriteFail = string((DEFAULT_CPACITY + 1) / sizeof(char), 'f');
        const string strWriteTermFail = string((DEFAULT_CPACITY - 2) / sizeof(char), 't');
        bool result = parcel.WriteBuffer(nullptr, sizeof(string));
        AssertEqual(result, false, "result did not equal false as expected.", state);

        result = parcel.WriteBufferAddTerminator(nullptr, sizeof(string), sizeof(char));
        AssertEqual(result, false, "result did not equal false as expected.", state);

        result = parcel.WriteBuffer(static_cast<const void *>(str.data()), 0);
        AssertEqual(result, false, "result did not equal false as expected.", state);

        size_t writeBufferSize = 0;
        result = parcel.WriteBufferAddTerminator(static_cast<const void *>(str.data()), writeBufferSize, sizeof(char));
        AssertEqual(result, false, "result did not equal false as expected.", state);

        result = parcel.WriteBuffer(static_cast<const void *>(strWriteFail.data()), strWriteFail.length());
        AssertEqual(result, false, "result did not equal false as expected.", state);

        result = parcel.WriteBufferAddTerminator(static_cast<const void *>(strWriteFail.data()),
                                                strWriteFail.length(), sizeof(char));
        AssertEqual(result, false, "result did not equal false as expected.", state);

        result = parcel.WriteBufferAddTerminator(static_cast<const void *>(str.data()), str.length(), sizeof(char));
        AssertEqual(result, true, "result did not equal true as expected.", state);

        Parcel recvParcel(nullptr);
        AssertTrue(CopyOldParcelToNewParcel(parcel, recvParcel, state),
            "CopyOldParcelToNewParcel(parcel, recvParcel, state) did not equal true as expected.", state);

        result = recvParcel.WriteBufferAddTerminator(static_cast<const void *>(&str), str.length() + 1, sizeof(char));
        AssertEqual(result, false, "result did not equal false as expected.", state);
    }
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

void WriteVectorTestData(Parcel &parcel, const VectorTestData &data, benchmark::State& state)
{
    BENCHMARK_LOGD("ParcelTest void WriteVectorTestData is called.");
    bool result = parcel.WriteBoolVector(data.booltest);
    AssertEqual(result, true, "result did not equal true as expected.", state);

    result = parcel.WriteInt8Vector(data.int8test);
    AssertEqual(result, true, "result did not equal true as expected.", state);

    result = parcel.WriteInt16Vector(data.int16test);
    AssertEqual(result, true, "result did not equal true as expected.", state);

    result = parcel.WriteInt32Vector(data.int32test);
    AssertEqual(result, true, "result did not equal true as expected.", state);

    result = parcel.WriteInt64Vector(data.int64test);
    AssertEqual(result, true, "result did not equal true as expected.", state);

    result = parcel.WriteUInt8Vector(data.uint8test);
    AssertEqual(result, true, "result did not equal true as expected.", state);

    result = parcel.WriteUInt16Vector(data.uint16test);
    AssertEqual(result, true, "result did not equal true as expected.", state);

    result = parcel.WriteUInt32Vector(data.uint32test);
    AssertEqual(result, true, "result did not equal true as expected.", state);

    result = parcel.WriteUInt64Vector(data.uint64test);
    AssertEqual(result, true, "result did not equal true as expected.", state);
}

template <typename T1>
void LoopReadVectorCmpData(const vector<T1> &vec1, const vector<T1> &vec2,
    const char* printInfo, benchmark::State& state)
{
    for (size_t idx = 0; idx < vec1.size(); idx++) {
        AssertEqual(vec1[idx], vec2[idx], printInfo, state);
    }
}

template <> void LoopReadVectorCmpData<u16string>(const vector<u16string> &vec1, const vector<u16string> &vec2,
    const char* printInfo, benchmark::State& state)
{
    for (size_t idx = 0; idx < vec1.size(); idx++) {
        AssertEqual(COMPARE_STRING_RESULT, vec1[idx].compare(vec2[idx]), printInfo, state);
    }
}

void ReadVectorTestData(Parcel &parcel, const VectorTestData &data, benchmark::State& state)
{
    BENCHMARK_LOGD("ParcelTest void ReadVectorTestData is called.");
    vector<bool> boolread;
    vector<int8_t> int8read;
    vector<int16_t> int16read;
    vector<int32_t> int32read;
    vector<int64_t> int64read;
    vector<uint8_t> uint8read;
    vector<uint16_t> uint16read;
    vector<uint32_t> uint32read;
    vector<uint64_t> uint64read;
    bool result = parcel.ReadBoolVector(&boolread);
    AssertEqual(result, true, "result did not equal true as expected.", state);
    LoopReadVectorCmpData(data.booltest, boolread,
        "data.booltest[i] did not equal boolread[i] as expected.", state);

    result = parcel.ReadInt8Vector(&int8read);
    AssertEqual(result, true, "result did not equal true as expected.", state);
    LoopReadVectorCmpData(data.int8test, int8read,
        "data.int8test[i] did not equal int8read[i] as expected.", state);

    result = parcel.ReadInt16Vector(&int16read);
    AssertEqual(result, true, "result did not equal true as expected.", state);
    LoopReadVectorCmpData(data.int16test, int16read,
        "data.int16test[i] did not equal int16read[i] as expected.", state);

    result = parcel.ReadInt32Vector(&int32read);
    AssertEqual(result, true, "result did not equal true as expected.", state);
    LoopReadVectorCmpData(data.int32test, int32read,
        "data.int32test[i] did not equal int32read[i] as expected.", state);

    result = parcel.ReadInt64Vector(&int64read);
    AssertEqual(result, true, "result did not equal true as expected.", state);
    LoopReadVectorCmpData(data.int64test, int64read,
        "data.int64test[i] did not equal int64read[i] as expected.", state);

    result = parcel.ReadUInt8Vector(&uint8read);
    AssertEqual(result, true, "result did not equal true as expected.", state);
    LoopReadVectorCmpData(data.uint8test, uint8read,
        "data.uint8test[i] did not equal uint8read[i] as expected.", state);

    result = parcel.ReadUInt16Vector(&uint16read);
    AssertEqual(result, true, "result did not equal true as expected.", state);
    LoopReadVectorCmpData(data.uint16test, uint16read,
        "data.uint16test[i] did not equal uint16read[i] as expected.", state);

    result = parcel.ReadUInt32Vector(&uint32read);
    AssertEqual(result, true, "result did not equal true as expected.", state);
    LoopReadVectorCmpData(data.uint32test, uint32read,
        "data.uint32test[i] did not equal uint32read[i] as expected.", state);

    result = parcel.ReadUInt64Vector(&uint64read);
    AssertEqual(result, true, "result did not equal true as expected.", state);
    LoopReadVectorCmpData(data.uint64test, uint64read,
        "data.uint64test[i] did not equal uint64read[i] as expected.", state);
}

/**
 * @tc.name: test_parcel_WriteAndReadVector_001
 * @tc.desc: test vector parcel read and write.
 * @tc.type: FUNC
 */
BENCHMARK_F(BenchmarkParcelTest, test_parcel_WriteAndReadVector_001)(benchmark::State& state)
{
    BENCHMARK_LOGD("ParcelTest test_parcel_WriteAndReadVector_001 start.");
    while (state.KeepRunning()) {
        Parcel parcel(nullptr);
        struct VectorTestData data;

        WriteVectorTestData(parcel, data, state);
        ReadVectorTestData(parcel, data, state);
    }
    BENCHMARK_LOGD("ParcelTest test_parcel_WriteAndReadVector_001 end.");
}

/**
 * @tc.name: test_parcel_WriteAndReadVector_002
 * @tc.desc: test vector parcel read and write.
 * @tc.type: FUNC
 */
BENCHMARK_F(BenchmarkParcelTest, test_parcel_WriteAndReadVector_002)(benchmark::State& state)
{
    BENCHMARK_LOGD("ParcelTest test_parcel_WriteAndReadVector_002 start.");
    while (state.KeepRunning()) {
        Parcel parcel1(nullptr);
        struct VectorTestData data;
        WriteVectorTestData(parcel1, data, state);

        Parcel parcel2(nullptr);
        AssertTrue(CopyOldParcelToNewParcel(parcel1, parcel2, state),
            "CopyOldParcelToNewParcel(parcel1, parcel2, state) did not equal true as expected.", state);
        ReadVectorTestData(parcel2, data, state);
    }
    BENCHMARK_LOGD("ParcelTest test_parcel_WriteAndReadVector_002 end.");
}

/**
 * @tc.name: test_parcel_WriteAndReadVector_003
 * @tc.desc: test vector parcel read and write.
 * @tc.type: FUNC
 */
BENCHMARK_F(BenchmarkParcelTest, test_parcel_WriteAndReadVector_003)(benchmark::State& state)
{
    BENCHMARK_LOGD("ParcelTest test_parcel_WriteAndReadVector_003 start.");
    while (state.KeepRunning()) {
        Parcel parcel1(nullptr);
        vector<string> stringtest{ "test", "test for", "test for write", "test for write vector" };
        vector<u16string> string16test{ u"test", u"test for", u"test for write", u"test for write vector" };
        bool result = parcel1.WriteStringVector(stringtest);
        AssertEqual(result, true, "result did not equal true as expected.", state);

        result = parcel1.WriteString16Vector(string16test);
        AssertEqual(result, true, "result did not equal true as expected.", state);

        vector<string> stringread;
        result = parcel1.ReadStringVector(&stringread);
        AssertEqual(result, true, "result did not equal true as expected.", state);
        LoopReadVectorCmpData(stringtest, stringread,
            "stringtest[i] did not equal stringread[i] as expected.", state);

        vector<u16string> u16stringread;
        result = parcel1.ReadString16Vector(&u16stringread);
        AssertEqual(result, true, "result did not equal true as expected.", state);
        LoopReadVectorCmpData(string16test, u16stringread,
            "string16test[i].compare(u16stringread[i]) did not equal 0 as expected.", state);

        Parcel parcel2(nullptr);
        AssertTrue(CopyOldParcelToNewParcel(parcel1, parcel2, state),
            "CopyOldParcelToNewParcel(parcel1, parcel2, state) did not equal true as expected.", state);

        result = parcel2.ReadStringVector(&stringread);
        AssertEqual(result, true, "result did not equal true as expected.", state);
        LoopReadVectorCmpData(stringtest, stringread,
            "stringtest[i] did not equal stringread[i] as expected.", state);

        result = parcel2.ReadString16Vector(&u16stringread);
        AssertEqual(result, true, "result did not equal true as expected.", state);
        LoopReadVectorCmpData(string16test, u16stringread,
            "string16test[i].compare(u16stringread[i]) did not equal 0 as expected.", state);
    }
    BENCHMARK_LOGD("ParcelTest test_parcel_WriteAndReadVector_003 end.");
}

/**
 * @tc.name: test_parcel_WriteAndReadVector_004
 * @tc.desc: test vector parcel read and write.
 * @tc.type: FUNC
 */
BENCHMARK_F(BenchmarkParcelTest, test_parcel_WriteAndReadVector_004)(benchmark::State& state)
{
    BENCHMARK_LOGD("ParcelTest test_parcel_WriteAndReadVector_004 start.");
    while (state.KeepRunning()) {
        Parcel parcel1(nullptr);
        Parcel parcel2(nullptr);
        vector<float> floattest{ 11221.132313, 11221.45678 };
        vector<double> doubletest{ 1122.132313, 1122.45678 };

        bool result = parcel1.WriteFloatVector(floattest);
        AssertEqual(result, true, "result did not equal true as expected.", state);

        result = parcel1.WriteDoubleVector(doubletest);
        AssertEqual(result, true, "result did not equal true as expected.", state);

        vector<float> floatread;
        vector<double> doubleread;

        result = parcel1.ReadFloatVector(&floatread);
        AssertEqual(result, true, "result did not equal true as expected.", state);
        LoopReadVectorCmpData(floattest, floatread,
            "floattest[i] did not equal floatread[i] as expected.", state);

        result = parcel1.ReadDoubleVector(&doubleread);
        AssertEqual(result, true, "result did not equal true as expected.", state);
        LoopReadVectorCmpData(doubletest, doubleread,
            "doubletest[i] did not equal doubleread[i] as expected.", state);
    }
}

bool CallWriteVector(Parcel &parcel, const std::vector<bool> &vectorTest)
{
    BENCHMARK_LOGD("ParcelTest bool CallWriteVector is called.");
    return parcel.WriteBoolVector(vectorTest);
}

bool CallWriteVector(Parcel &parcel, const std::vector<int8_t> &vectorTest)
{
    BENCHMARK_LOGD("ParcelTest bool CallWriteVector is called.");
    return parcel.WriteInt8Vector(vectorTest);
}

bool CallWriteVector(Parcel &parcel, const std::vector<int16_t> &vectorTest)
{
    BENCHMARK_LOGD("ParcelTest bool CallWriteVector is called.");
    return parcel.WriteInt16Vector(vectorTest);
}

bool CallWriteVector(Parcel &parcel, const std::vector<int32_t> &vectorTest)
{
    BENCHMARK_LOGD("ParcelTest bool CallWriteVector is called.");
    return parcel.WriteInt32Vector(vectorTest);
}

bool CallWriteVector(Parcel &parcel, const std::vector<int64_t> &vectorTest)
{
    BENCHMARK_LOGD("ParcelTest bool CallWriteVector is called.");
    return parcel.WriteInt64Vector(vectorTest);
}

bool CallWriteVector(Parcel &parcel, const std::vector<uint8_t> &vectorTest)
{
    BENCHMARK_LOGD("ParcelTest bool CallWriteVector is called.");
    return parcel.WriteUInt8Vector(vectorTest);
}

bool CallWriteVector(Parcel &parcel, const std::vector<uint16_t> &vectorTest)
{
    BENCHMARK_LOGD("ParcelTest bool CallWriteVector is called.");
    return parcel.WriteUInt16Vector(vectorTest);
}

bool CallWriteVector(Parcel &parcel, const std::vector<uint32_t> &vectorTest)
{
    BENCHMARK_LOGD("ParcelTest bool CallWriteVector is called.");
    return parcel.WriteUInt32Vector(vectorTest);
}

bool CallWriteVector(Parcel &parcel, const std::vector<uint64_t> &vectorTest)
{
    BENCHMARK_LOGD("ParcelTest bool CallWriteVector is called.");
    return parcel.WriteUInt64Vector(vectorTest);
}

bool CallWriteVector(Parcel &parcel, const std::vector<float> &vectorTest)
{
    BENCHMARK_LOGD("ParcelTest bool CallWriteVector is called.");
    return parcel.WriteFloatVector(vectorTest);
}

bool CallWriteVector(Parcel &parcel, const std::vector<double> &vectorTest)
{
    BENCHMARK_LOGD("ParcelTest bool CallWriteVector is called.");
    return parcel.WriteDoubleVector(vectorTest);
}

bool CallWriteVector(Parcel &parcel, const std::vector<std::string> &vectorTest)
{
    BENCHMARK_LOGD("ParcelTest bool CallWriteVector is called.");
    return parcel.WriteStringVector(vectorTest);
}

bool CallWriteVector(Parcel &parcel, const std::vector<std::u16string> &vectorTest)
{
    BENCHMARK_LOGD("ParcelTest bool CallWriteVector is called.");
    return parcel.WriteString16Vector(vectorTest);
}

template <typename T>
void ParcelWriteVector(const std::vector<T> &vectorTest, benchmark::State& state)
{
    BENCHMARK_LOGD("ParcelTest void ParcelWriteVector is called.");
    Parcel parcel1(nullptr);
    Parcel parcel2(nullptr);
    bool result = CallWriteVector(parcel1, vectorTest);
    AssertEqual(result, true, "result did not equal true as expected.", state);
    AssertTrue(CopyOldParcelToNewParcel(parcel1, parcel2, state),
        "CopyOldParcelToNewParcel(parcel1, parcel2, state) did not equal true as expected.", state);

    result = CallWriteVector(parcel2, vectorTest);
    AssertEqual(result, false, "result did not equal false as expected.", state);
}

/**
 * @tc.name: test_parcel_WriteAndReadVector_005
 * @tc.desc: test vector parcel write failed.
 * @tc.type: FUNC
 */
BENCHMARK_F(BenchmarkParcelTest, test_parcel_WriteAndReadVector_005)(benchmark::State& state)
{
    BENCHMARK_LOGD("ParcelTest test_parcel_WriteAndReadVector_005 start.");
    while (state.KeepRunning()) {
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

        ParcelWriteVector(boolVectorTest, state);
        ParcelWriteVector(int8VectorTest, state);
        ParcelWriteVector(int16VectorTest, state);
        ParcelWriteVector(int32VectorTest, state);
        ParcelWriteVector(int64VectorTest, state);
        ParcelWriteVector(uint8VectorTest, state);
        ParcelWriteVector(uint16VectorTest, state);
        ParcelWriteVector(uint32VectorTest, state);
        ParcelWriteVector(uint64VectorTest, state);
        ParcelWriteVector(floatVectorTest, state);
        ParcelWriteVector(doubleVectorTest, state);
        ParcelWriteVector(stringVectorTest, state);
        ParcelWriteVector(string16VectorTest, state);
    }
    BENCHMARK_LOGD("ParcelTest test_parcel_WriteAndReadVector_005 end.");
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
    BENCHMARK_LOGD("ParcelTest bool TestParcelable::Marshalling is called.");
    bool result = parcel.WriteInt32(this->int32Write_);
    return result;
}

TestParcelable *TestParcelable::Unmarshalling(Parcel &parcel)
{
    BENCHMARK_LOGD("ParcelTest TestParcelable *TestParcelable::Unmarshalling is called.");
    auto *read = new TestParcelable();
    read->int32Read_ = parcel.ReadInt32();
    return read;
}

/**
 * @tc.name: test_parcel_parcelable_001
 * @tc.desc: test parcel read and write parcelable obj.
 * @tc.type: FUNC
 */
BENCHMARK_F(BenchmarkParcelTest, test_parcel_parcelable_001)(benchmark::State& state)
{
    BENCHMARK_LOGD("ParcelTest test_parcel_parcelable_001 start.");
    while (state.KeepRunning()) {
        Parcel parcel(nullptr);
        sptr<TestParcelable> parcelableWrite = new TestParcelable();
        bool result = false;

        result = parcel.WriteParcelable(parcelableWrite);
        AssertEqual(result, true, "result did not equal true as expected.", state);
        AssertEqual(parcel.GetWritePosition(), parcel.GetDataSize(),
            "parcel.GetWritePosition() did not equal parcel.GetDataSize() as expected.", state);

        sptr<TestParcelable> parcelableRead = parcel.ReadParcelable<TestParcelable>();
        AssertEqual(parcelableWrite->int32Write_, parcelableRead->int32Read_,
            "parcelableWrite->int32Write_ did not equal parcelableRead->int32Read_ as expected.", state);
        AssertEqual(parcel.GetReadPosition(), parcel.GetDataSize(),
            "parcel.GetReadPosition() did not equal parcel.GetDataSize() as expected.", state);
    }
    BENCHMARK_LOGD("ParcelTest test_parcel_parcelable_001 end.");
}

/**
 * @tc.name: test_parcel_parcelable_002
 * @tc.desc: test parcel read and write parcelable obj.
 * @tc.type: FUNC
 */
BENCHMARK_F(BenchmarkParcelTest, test_parcel_parcelable_002)(benchmark::State& state)
{
    BENCHMARK_LOGD("ParcelTest test_parcel_parcelable_002 start.");
    while (state.KeepRunning()) {
        Parcel parcel(nullptr);
        bool result = parcel.WriteParcelable(nullptr);
        AssertEqual(result, true, "result did not equal true as expected.", state);

        sptr<TestParcelable> parcelableRead = parcel.ReadParcelable<TestParcelable>();
        AssertEqual(nullptr, parcelableRead, "nullptr did not equal parcelableRead as expected.", state);
    }
    BENCHMARK_LOGD("ParcelTest test_parcel_parcelable_002 end.");
}

/**
 * @tc.name: test_parcel_parcelable_003
 * @tc.desc: test parcel read and write parcelable obj.
 * @tc.type: FUNC
 */
BENCHMARK_F(BenchmarkParcelTest, test_parcel_parcelable_003)(benchmark::State& state)
{
    BENCHMARK_LOGD("ParcelTest test_parcel_parcelable_003 start.");
    while (state.KeepRunning()) {
        Parcel parcel(nullptr);
        sptr<TestParcelable> parcelableWriteNull;
        bool result = parcel.WriteStrongParcelable(parcelableWriteNull);
        AssertEqual(result, true, "result did not equal true as expected.", state);

        sptr<TestParcelable> parcelableWrite = new TestParcelable();

        bool test = parcelableWrite->TestBehavior(Parcelable::BehaviorFlag::HOLD_OBJECT);
        AssertEqual(test, false, "test did not equal false as expected.", state);

        test = parcelableWrite->TestBehavior(Parcelable::BehaviorFlag::IPC);
        AssertEqual(test, false, "test did not equal false as expected.", state);

        test = parcelableWrite->TestBehavior(Parcelable::BehaviorFlag::RPC);
        AssertEqual(test, false, "test did not equal false as expected.", state);

        result = parcel.WriteStrongParcelable(parcelableWrite);
        AssertEqual(result, true, "result did not equal true as expected.", state);

        sptr<TestParcelable> parcelableReadNull = parcel.ReadParcelable<TestParcelable>();
        AssertEqual(nullptr, parcelableReadNull,
            "nullptr did not equal parcelableReadNull as expected.", state);

        sptr<TestParcelable> parcelableRead = parcel.ReadParcelable<TestParcelable>();
        AssertEqual(parcelableWrite->int32Write_, parcelableRead->int32Read_,
            "parcelableWrite->int32Write_ did not equal parcelableRead->int32Read_ as expected.", state);

        test = parcelableWrite->TestBehavior(Parcelable::BehaviorFlag::HOLD_OBJECT);
        AssertEqual(test, true, "test did not equal true as expected.", state);

        test = parcelableWrite->TestBehavior(Parcelable::BehaviorFlag::IPC);
        AssertEqual(test, false, "test did not equal false as expected.", state);

        test = parcelableWrite->TestBehavior(Parcelable::BehaviorFlag::RPC);
        AssertEqual(test, false, "test did not equal false as expected.", state);

        parcelableWrite->ClearBehavior(Parcelable::BehaviorFlag::HOLD_OBJECT);
        test = parcelableWrite->TestBehavior(Parcelable::BehaviorFlag::HOLD_OBJECT);
        AssertEqual(test, false, "test did not equal false as expected.", state);
    }
    BENCHMARK_LOGD("ParcelTest test_parcel_parcelable_003 end.");
}

/**
 * @tc.name: test_SetMaxCapacity_001
 * @tc.desc: test parcel capacity function.
 * @tc.type: FUNC
 */
BENCHMARK_F(BenchmarkParcelTest, test_SetMaxCapacity_001)(benchmark::State& state)
{
    BENCHMARK_LOGD("ParcelTest test_SetMaxCapacity_001 start.");
    while (state.KeepRunning()) {
        Parcel parcel(nullptr);
        char test[PARCEL_TEST_CHAR_ARRAY_SIZE] = {0};
        bool ret = parcel.WriteBuffer(test, PARCEL_TEST_CHAR_ARRAY_SIZE);
        AssertEqual(ret, true, "ret did not equal true as expected.", state);

        // because default maxCap is 200 * 1024, so reset it more
        const size_t maxCapacitySize = 201 * 1024;
        parcel.SetMaxCapacity(maxCapacitySize);
        // test write data over max capacity: 205780 + 48 > 201 * 1024
        char test2[PARCEL_TEST1_CHAR_ARRAY_SIZE] = {0};
        ret = parcel.WriteBuffer(test2, PARCEL_TEST1_CHAR_ARRAY_SIZE);
        AssertEqual(ret, false, "ret did not equal false as expected.", state);
    }
    BENCHMARK_LOGD("ParcelTest test_SetMaxCapacity_001 end.");
}

/**
 * @tc.name: test_SetMaxCapacity_002
 * @tc.desc: test parcel capacity function.
 * @tc.type: FUNC
 */
BENCHMARK_F(BenchmarkParcelTest, test_SetMaxCapacity_002)(benchmark::State& state)
{
    BENCHMARK_LOGD("ParcelTest test_SetMaxCapacity_002 start.");
    while (state.KeepRunning()) {
        Parcel parcel(nullptr);
        char test[PARCEL_TEST_CHAR_ARRAY_SIZE] = {0};
        bool ret = parcel.WriteInt32(5767168);
        AssertEqual(ret, true, "ret did not equal true as expected.", state);

        ret = parcel.WriteBuffer(test, PARCEL_TEST_CHAR_ARRAY_SIZE);
        AssertEqual(ret, true, "ret did not equal true as expected.", state);

        vector<std::u16string> val;
        ret = parcel.ReadString16Vector(&val);
        AssertEqual(ret, false, "ret did not equal false as expected.", state);
    }
    BENCHMARK_LOGD("ParcelTest test_SetMaxCapacity_002 end.");
}

static void ParcelWriteData(Parcel& parcel, u16string& str16Write, string& strWrite, benchmark::State& state)
{
    parcel.WriteBool(true);
    bool result = parcel.WriteString(strWrite);
    AssertEqual(result, true, "result did not equal true as expected.", state);

    RemoteObject obj1;
    result = parcel.WriteRemoteObject(&obj1);
    AssertEqual(result, true, "result did not equal true as expected.", state);

    parcel.WriteInt32(WRITE_AND_CMP_INT32_VALUE);
    RemoteObject obj2;
    result = parcel.WriteRemoteObject(&obj2);
    AssertEqual(result, true, "result did not equal true as expected.", state);

    result = parcel.WriteString16(str16Write);
    AssertEqual(result, true, "result did not equal true as expected.", state);
}

BENCHMARK_F(BenchmarkParcelTest, test_ValidateReadData_001)(benchmark::State& state)
{
    BENCHMARK_LOGD("ParcelTest test_ValidateReadData_001 start.");
    while (state.KeepRunning()) {
        Parcel parcel(nullptr);
        u16string str16Write = u"12345";
        string strWrite = "test";
        ParcelWriteData(parcel, str16Write, strWrite, state);

        bool readBool = parcel.ReadBool();
        AssertEqual(readBool, true, "readBool did not equal true as expected.", state);

        string strRead = parcel.ReadString();
        AssertEqual(COMPARE_STRING_RESULT, strcmp(strRead.c_str(), strWrite.c_str()),
            "strcmp(strRead.c_str(), strWrite.c_str()) did not equal 0 as expected.", state);

        sptr<RemoteObject> readObj1 = parcel.ReadObject<RemoteObject>();
        AssertEqual(true, (readObj1.GetRefPtr() != nullptr),
            "(readObj1.GetRefPtr() != nullptr) did not equal true as expected.", state);

        int32_t readInt32 = parcel.ReadInt32();
        AssertEqual(readInt32, WRITE_AND_CMP_INT32_VALUE, "readInt32 did not equal 5 as expected.", state);

        sptr<RemoteObject> readObj2 = parcel.ReadObject<RemoteObject>();
        AssertEqual(true, (readObj2.GetRefPtr() != nullptr),
            "(readObj2.GetRefPtr() != nullptr) did not equal true as expected.", state);

        u16string str16Read = parcel.ReadString16();
        AssertEqual(COMPARE_STRING_RESULT, str16Read.compare(str16Write),
            "str16Read.compare(str16Write) did not equal 0 as expected.", state);
    }
    BENCHMARK_LOGD("ParcelTest test_ValidateReadData_001 end.");
}

BENCHMARK_F(BenchmarkParcelTest, test_ValidateReadData_002)(benchmark::State& state)
{
    BENCHMARK_LOGD("ParcelTest test_ValidateReadData_002 start.");
    while (state.KeepRunning()) {
        Parcel parcel(nullptr);
        string strWrite = "test";
        u16string str16Write = u"12345";
        ParcelWriteData(parcel, str16Write, strWrite, state);

        bool readBool = parcel.ReadBool();
        AssertEqual(readBool, true, "readBool did not equal true as expected.", state);

        string strRead = parcel.ReadString();
        AssertEqual(COMPARE_STRING_RESULT, strcmp(strRead.c_str(), strWrite.c_str()),
            "strcmp(strRead.c_str(), strWrite.c_str()) did not equal 0 as expected.", state);

        int32_t readInt32 = parcel.ReadInt32();
        int32_t compareInt32Value = 0;
        AssertEqual(readInt32, compareInt32Value, "readInt32 did not equal 0 as expected.", state);

        u16string str16Read = parcel.ReadString16();
        AssertEqual(COMPARE_STRING_RESULT, str16Read.compare(std::u16string()),
            "str16Read.compare(std::u16string()) did not equal 0 as expected.", state);

        sptr<RemoteObject> readObj1 = parcel.ReadObject<RemoteObject>();
        AssertEqual(true, (readObj1.GetRefPtr() == nullptr),
            "(readObj1.GetRefPtr() == nullptr) did not equal true as expected.", state);
    }
    BENCHMARK_LOGD("ParcelTest test_ValidateReadData_002 end.");
}

BENCHMARK_F(BenchmarkParcelTest, test_RewindWrite_001)(benchmark::State& state)
{
    BENCHMARK_LOGD("ParcelTest test_RewindWrite_001 start.");
    while (state.KeepRunning()) {
        Parcel parcel(nullptr);
        parcel.WriteInt32(WRITE_AND_CMP_INT32_VALUE);
        string strWrite = "test";
        parcel.WriteString(strWrite);
        RemoteObject obj1;
        parcel.WriteRemoteObject(&obj1);
        size_t pos = parcel.GetWritePosition();
        parcel.WriteInt32(WRITE_AND_CMP_INT32_VALUE);
        RemoteObject obj2;
        parcel.WriteRemoteObject(&obj2);
        u16string str16Write = u"12345";
        parcel.WriteString16(str16Write);

        bool result = parcel.RewindWrite(pos);
        AssertEqual(result, true, "result did not equal true as expected.", state);
        parcel.WriteInt32(WRITE_AND_CMP_INT32_VALUE);
        parcel.WriteInt32(WRITE_AND_CMP_INT32_VALUE);

        int32_t readint32 = parcel.ReadInt32();
        AssertEqual(readint32, WRITE_AND_CMP_INT32_VALUE, "readint32 did not equal 5 as expected.", state);

        string strRead = parcel.ReadString();
        AssertEqual(COMPARE_STRING_RESULT, strcmp(strRead.c_str(), strWrite.c_str()),
            "strcmp(strRead.c_str(), strWrite.c_str()) did not equal 0 as expected.", state);

        sptr<RemoteObject> readObj1 = parcel.ReadObject<RemoteObject>();
        AssertEqual(true, (readObj1.GetRefPtr() != nullptr),
            "(readObj1.GetRefPtr() != nullptr) did not equal true as expected.", state);

        readint32 = parcel.ReadInt32();
        AssertEqual(readint32, WRITE_AND_CMP_INT32_VALUE, "readint32 did not equal 5 as expected.", state);

        sptr<RemoteObject> readObj2 = parcel.ReadObject<RemoteObject>();
        AssertEqual(true, (readObj2.GetRefPtr() == nullptr),
            "(readObj2.GetRefPtr() == nullptr) did not equal true as expected.", state);
        readint32 = parcel.ReadInt32();
        AssertEqual(readint32, WRITE_AND_CMP_INT32_VALUE, "readint32 did not equal 5 as expected.", state);
    }
    BENCHMARK_LOGD("ParcelTest test_RewindWrite_001 end.");
}

BENCHMARK_F(BenchmarkParcelTest, test_RewindWrite_002)(benchmark::State& state)
{
    BENCHMARK_LOGD("ParcelTest test_RewindWrite_002 start.");
    while (state.KeepRunning()) {
        Parcel parcel(nullptr);
        parcel.WriteInt32(WRITE_AND_CMP_INT32_VALUE);
        string strWrite = "test";
        parcel.WriteString(strWrite);
        RemoteObject obj1;
        parcel.WriteRemoteObject(&obj1);
        parcel.WriteInt32(WRITE_AND_CMP_INT32_VALUE);
        RemoteObject obj2;
        parcel.WriteRemoteObject(&obj2);
        size_t pos = parcel.GetWritePosition();
        u16string str16Write = u"12345";
        parcel.WriteString16(str16Write);
        bool result = parcel.RewindWrite(pos);
        AssertEqual(result, true, "result did not equal true as expected.", state);

        int32_t readint32 = parcel.ReadInt32();
        AssertEqual(readint32, WRITE_AND_CMP_INT32_VALUE, "readint32 did not equal 5 as expected.", state);

        string strRead = parcel.ReadString();
        AssertEqual(COMPARE_STRING_RESULT, strcmp(strRead.c_str(), strWrite.c_str()),
            "strcmp(strRead.c_str(), strWrite.c_str()) did not equal 0 as expected.", state);

        uint32_t readUint32 = parcel.ReadUint32();
        uint32_t compareUint32Value = 0;
        AssertEqual(readUint32, compareUint32Value, "readUint32 did not equal 0 as expected.", state);

        string strRead2 = parcel.ReadString();
        AssertEqual(COMPARE_STRING_RESULT, strRead2.compare(std::string()),
            "strRead2.compare(std::string()) did not equal 0 as expected.", state);

        sptr<RemoteObject> readObj1 = parcel.ReadObject<RemoteObject>();
        AssertEqual(true, (readObj1.GetRefPtr() == nullptr),
            "(readObj1.GetRefPtr() == nullptr) did not equal true as expected.", state);

        double compareDoubleValue = 0;
        double readDouble = parcel.ReadDouble();
        AssertEqual(readDouble, compareDoubleValue, "readDouble did not equal 0 as expected.", state);
    }
    BENCHMARK_LOGD("ParcelTest test_RewindWrite_002 end.");
}

enum ParcelVectorValue {
    VALUE_ZERO = 0,
    VALUE_ONE = 1,
    VALUE_TWO = 2,
    VALUE_THREE = 3,
    VALUE_FORE = 4,
    VALUE_FIVE = 5
};
enum ParcelVectorIndex {
    INDEX_ZERO = 0,
    INDEX_ONE = 1,
    INDEX_TWO = 2,
    INDEX_THREE = 3,
    INDEX_FORE = 4,
    INDEX_FIVE = 5
};

BENCHMARK_F(BenchmarkParcelTest, test_RewindWrite_003)(benchmark::State& state)
{
    BENCHMARK_LOGD("ParcelTest test_RewindWrite_003 start.");
    while (state.KeepRunning()) {
        Parcel parcel(nullptr);
        std::vector<int32_t> val{VALUE_ONE, VALUE_TWO, VALUE_THREE, VALUE_FORE, VALUE_FIVE};
        AssertEqual(val.size(), REWINDWRITE003_VECTOR_LENGTH, "val.size() did not equal 5 as expected.", state);
        bool result = parcel.WriteInt32Vector(val);
        AssertEqual(result, true, "result did not equal true as expected.", state);
        size_t pos = parcel.GetWritePosition() - sizeof(int32_t);
        result = parcel.RewindWrite(pos);
        AssertEqual(result, true, "result did not equal true as expected.", state);

        RemoteObject obj;
        parcel.WriteRemoteObject(&obj);
        std::vector<int32_t> int32Read;
        result = parcel.ReadInt32Vector(&int32Read);
        AssertEqual(result, false, "result did not equal false as expected.", state);
        AssertEqual(int32Read.size(), REWINDWRITE003_VECTOR_LENGTH,
            "int32Read.size() did not equal 5 as expected.", state);
        AssertEqual(int32Read[INDEX_ZERO], VALUE_ONE, "int32Read[0] did not equal 1 as expected.", state);
        AssertEqual(int32Read[INDEX_ONE], VALUE_TWO, "int32Read[1] did not equal 2 as expected.", state);
        AssertEqual(int32Read[INDEX_TWO], VALUE_THREE, "int32Read[2] did not equal 3 as expected.", state);
        AssertEqual(int32Read[INDEX_THREE], VALUE_FORE, "int32Read[3] did not equal 4 as expected.", state);
        AssertEqual(int32Read[INDEX_FORE], VALUE_ZERO, "int32Read[4] did not equal 0 as expected.", state);
    }
    BENCHMARK_LOGD("ParcelTest test_RewindWrite_003 end.");
}

/**
 * @tc.name: test_GetDataCapacity_001
 * @tc.desc: test kinds of input to SetDataCapacity.
 * @tc.type: FUNC
 */
BENCHMARK_F(BenchmarkParcelTest, test_GetDataCapacity_001)(benchmark::State& state)
{
    BENCHMARK_LOGD("ParcelTest test_GetDataCapacity_001 start.");
    while (state.KeepRunning()) {
        Parcel parcel;
        size_t size = parcel.GetDataCapacity();
        AssertEqual(size, DATA_CAPACITY_INIT_SIZE,
            "test_GetDataCapacity_001 size did not equal zero as expected.", state);

        struct TestData data = { true, -0x34, 0x5634, -0x12345678, 0x34, 0x5634, 0x12345678 };
        WriteTestData(parcel, data, state);
        size = parcel.GetDataCapacity();
        AssertUnequal(size, DATA_CAPACITY_INIT_SIZE,
            "test_GetDataCapacity_001 size equal zero as expected.", state);
    }
    BENCHMARK_LOGD("ParcelTest test_GetDataCapacity_001 end.");
}

/**
 * @tc.name: test_GetMaxCapacity_001
 * @tc.desc: test parcel capacity function.
 * @tc.type: FUNC
 */
BENCHMARK_F(BenchmarkParcelTest, test_GetMaxCapacity_001)(benchmark::State& state)
{
    BENCHMARK_LOGD("ParcelTest test_GetMaxCapacity_001 start.");
    while (state.KeepRunning()) {
        Parcel parcel(nullptr);
        size_t size = parcel.GetMaxCapacity();
        AssertEqual(size, DEFAULT_CPACITY,
            "test_GetMaxCapacity_001 size did not equal DEFAULT_CPACITY as expected.", state);

        size_t maxCapacity = DEFAULT_CPACITY * 2;
        bool result = parcel.SetMaxCapacity(maxCapacity);
        AssertEqual(result, true, "test_GetMaxCapacity_001 result did not equal true as expected.", state);

        size = parcel.GetMaxCapacity();
        AssertEqual(size, maxCapacity,
            "test_GetMaxCapacity_001 size did not equal to maxCapacity as expected.", state);
    }
    BENCHMARK_LOGD("ParcelTest test_GetMaxCapacity_001 end.");
}

/**
 * @tc.name: test_GetObjectOffsets_001
 * @tc.desc: test parcel Object offsets function.
 * @tc.type: FUNC
 */
BENCHMARK_F(BenchmarkParcelTest, test_GetObjectOffsets_001)(benchmark::State& state)
{
    BENCHMARK_LOGD("ParcelTest test_GetObjectOffsets_001 start.");
    while (state.KeepRunning()) {
        Parcel parcel(nullptr);
        binder_size_t* pObjectOffsets = reinterpret_cast<binder_size_t*>(parcel.GetObjectOffsets());
        AssertEqual(pObjectOffsets, nullptr,
            "test_GetObjectOffsets_001 pObjectOffsets did not equal to nullptr as expected.", state);

        RemoteObject obj;
        bool result = parcel.WriteRemoteObject(&obj);
        AssertEqual(result, true,
            "test_GetObjectOffsets_001 result did not equal true as expected.", state);

        pObjectOffsets = reinterpret_cast<binder_size_t*>(parcel.GetObjectOffsets());
        AssertUnequal(pObjectOffsets, nullptr,
            "test_GetObjectOffsets_001 pObjectOffsets equal nullptr as expected.", state);
    }
    BENCHMARK_LOGD("ParcelTest test_GetObjectOffsets_001 end.");
}

/**
 * @tc.name: test_GetOffsetsSize_001
 * @tc.desc: test parcel Object offsets Size function.
 * @tc.type: FUNC
 */
BENCHMARK_F(BenchmarkParcelTest, test_GetOffsetsSize_001)(benchmark::State& state)
{
    BENCHMARK_LOGD("ParcelTest test_GetOffsetsSize_001 start.");
    while (state.KeepRunning()) {
        Parcel parcel(nullptr);
        size_t size = parcel.GetOffsetsSize();
        AssertEqual(size, DATA_OFFSETS_INIT_SIZE,
            "test_GetOffsetsSize_001 size did not equal to 0 as expected.", state);

        RemoteObject obj;
        bool result = parcel.WriteRemoteObject(&obj);
        AssertEqual(result, true, "test_GetOffsetsSize_001 result did not equal true as expected.", state);

        size = parcel.GetOffsetsSize();
        AssertUnequal(size, DATA_OFFSETS_INIT_SIZE, "test_GetOffsetsSize_001 size equal 0 as expected.", state);
    }
    BENCHMARK_LOGD("ParcelTest test_GetOffsetsSize_001 end.");
}

/**
 * @tc.name: test_GetReadableBytes_001
 * @tc.desc: test parcel readable bytes function.
 * @tc.type: FUNC
 */
BENCHMARK_F(BenchmarkParcelTest, test_GetReadableBytes_001)(benchmark::State& state)
{
    BENCHMARK_LOGD("ParcelTest test_GetReadableBytes_001 start.");
    while (state.KeepRunning()) {
        Parcel parcel(nullptr);
        int8_t writeInt8 = 0x34;
        bool result = parcel.WriteInt8(writeInt8);
        AssertEqual(result, true, "test_GetReadableBytes_001 result did not equal true as expected.", state);

        size_t size = parcel.GetReadableBytes();
        AssertUnequal(size, DATA_READBYTES_INIT_SIZE, "test_GetReadableBytes_001 size equal 0 as expected.", state);

        int8_t readint8 = parcel.ReadInt8();
        AssertEqual(readint8, writeInt8,
            "test_GetReadableBytes_001 readint8 not equal writeInt8 as expected.", state);

        size = parcel.GetReadableBytes();
        AssertEqual(size, DATA_READBYTES_INIT_SIZE,
            "test_GetReadableBytes_001 size did not equal 0 as expected.", state);
    }
    BENCHMARK_LOGD("ParcelTest test_GetReadableBytes_001 end.");
}

/**
 * @tc.name: test_GetWritableBytes_001
 * @tc.desc: test parcel writable bytes function.
 * @tc.type: FUNC
 */
BENCHMARK_F(BenchmarkParcelTest, test_GetWritableBytes_001)(benchmark::State& state)
{
    BENCHMARK_LOGD("ParcelTest test_GetWritableBytes_001 start.");
    while (state.KeepRunning()) {
        Parcel parcel(nullptr);
        int8_t writeInt8 = 0x34;
        bool result = parcel.WriteInt8(writeInt8);
        AssertEqual(result, true, "test_GetWritableBytes_001 result did not equal true as expected.", state);

        size_t size = parcel.GetDataCapacity() - sizeof(int32_t);
        size_t writablesize = parcel.GetWritableBytes();
        AssertEqual(writablesize, size,
            "test_GetWritableBytes_001 writablesize did not equal size as expected.", state);

        int16_t writeInt16 = 0x1234;
        result = parcel.WriteInt16(writeInt16);
        AssertEqual(result, true, "test_GetWritableBytes_001 result did not equal true as expected.", state);

        size = parcel.GetDataCapacity() - (sizeof(int32_t)*2);
        writablesize = parcel.GetWritableBytes();
        AssertEqual(writablesize, size,
            "test_GetWritableBytes_001 writablesize did not equal size as expected.", state);
    }
    BENCHMARK_LOGD("ParcelTest test_GetWritableBytes_001 end.");
}

/**
 * @tc.name: test_InjectOffsets_001
 * @tc.desc: test parcel inject offsets function.
 * @tc.type: FUNC
 */
BENCHMARK_F(BenchmarkParcelTest, test_InjectOffsets_001)(benchmark::State& state)
{
    BENCHMARK_LOGD("ParcelTest test_InjectOffsets_001 start.");
    while (state.KeepRunning()) {
        Parcel parcel(nullptr);
        int32_t size = 256;
        bool result = parcel.WriteInt32(size);
        AssertEqual(result, true, "test_InjectOffsets_001 result did not equal true as expected.", state);

        binder_size_t objectOffsets[PARCEL_INJECTOFFSETS_CHAR_ARRAY_SIZE] = {0};
        result = parcel.WriteBuffer(static_cast<const void *>(objectOffsets), sizeof(binder_size_t) * size);
        AssertEqual(result, true, "test_InjectOffsets_001 result did not equal true as expected.", state);

        int32_t offsetSize = parcel.ReadInt32();
        AssertEqual(offsetSize, size, "test_InjectOffsets_001 offsetSize did not equal size as expected.", state);

        const uint8_t* offsets = parcel.ReadBuffer(sizeof(binder_size_t) * offsetSize);
        AssertUnequal(offsets, nullptr, "test_InjectOffsets_001 offsets equal nullptr as expected.", state);

        Parcel newParcel(nullptr);
        newParcel.InjectOffsets(reinterpret_cast<binder_size_t>(offsets), offsetSize);

        binder_size_t* pObjectOffsets = reinterpret_cast<binder_size_t*>(newParcel.GetObjectOffsets());
        AssertUnequal(pObjectOffsets, nullptr,
            "test_InjectOffsets_001 pObjectOffsets equal nullptr as expected.", state);
    }
    BENCHMARK_LOGD("ParcelTest test_InjectOffsets_001 end.");
}

/**
 * @tc.name: test_parcel_WriteAndRead_Float_002
 * @tc.desc: test parcel write and read write.
 * @tc.type: FUNC
 */
BENCHMARK_F(BenchmarkParcelTest, test_parcel_WriteAndRead_Float_002)(benchmark::State& state)
{
    BENCHMARK_LOGD("ParcelTest test_parcel_WriteAndRead_Float_002 start.");
    while (state.KeepRunning()) {
        Parcel parcel1(nullptr);
        float floatwrite = 12.345678f;
        bool result = parcel1.WriteFloat(floatwrite);
        AssertEqual(result, true, "test_parcel_WriteAndRead_Float_002 result did not equal true as expected.", state);

        double doublewrite = 1345.7653;
        result = parcel1.WriteDouble(doublewrite);
        AssertEqual(result, true, "test_parcel_WriteAndRead_Float_002 result did not equal true as expected.", state);

        float floatread = parcel1.ReadFloat();
        AssertEqual(floatread, floatwrite,
            "test_parcel_WriteAndRead_Float_002 floatread did not equal floatwrite as expected.", state);

        double doubleread;
        result = parcel1.ReadDouble(doubleread);
        AssertEqual(result, true, "test_parcel_WriteAndRead_Float_002 result did not equal true as expected.", state);
        AssertEqual(doubleread, doublewrite,
            "test_parcel_WriteAndRead_Float_002 doublewrite did not equal doubleread as expected.", state);
    }
    BENCHMARK_LOGD("ParcelTest test_parcel_WriteAndRead_Float_002 end.");
}
/**
 * @tc.name: test_parcel_WriteAndRead_005
 * @tc.desc: test parcel write and read write.
 * @tc.type: FUNC
 */
BENCHMARK_F(BenchmarkParcelTest, test_parcel_WriteAndRead_005)(benchmark::State& state)
{
    BENCHMARK_LOGD("ParcelTest test_parcel_WriteAndRead_005 start.");
    while (state.KeepRunning()) {
        Parcel parcel1(nullptr);
        int64_t int64test = -0x1234567887654321;
        bool result = parcel1.WriteInt64(int64test);
        AssertEqual(result, true, "test_parcel_WriteAndRead_005 result did not equal true as expected.", state);

        uint64_t uint64test = 0x1234567887654321;
        result = parcel1.WriteUint64(uint64test);
        AssertEqual(result, true, "test_parcel_WriteAndRead_005 result did not equal true as expected.", state);

        int64_t readint64;
        result = parcel1.ReadInt64(readint64);
        AssertEqual(result, true, "test_parcel_WriteAndRead_005 result did not equal true as expected.", state);
        AssertEqual(readint64, int64test,
            "test_parcel_WriteAndRead_005 readint64 did not equal int64test as expected.", state);

        uint64_t readuint64;
        result = parcel1.ReadUint64(readuint64);
        AssertEqual(result, true, "test_parcel_WriteAndRead_005 result did not equal true as expected.", state);
        AssertEqual(readuint64, uint64test,
            "test_parcel_WriteAndRead_005 readuint64 did not equal uint64test as expected.", state);
    }
    BENCHMARK_LOGD("ParcelTest test_parcel_WriteAndRead_005 end.");
}

/**
 * @tc.name: test_ReadPointer_001
 * @tc.desc: test parcel read pointer function.
 * @tc.type: FUNC
 */
BENCHMARK_F(BenchmarkParcelTest, test_ReadPointer_001)(benchmark::State& state)
{
    BENCHMARK_LOGD("ParcelTest test_ReadPointer_001 start.");
    while (state.KeepRunning()) {
        Parcel parcel(nullptr);
        char writePointer[PARCEL_WRITEPOINTER_CHAR_ARRAY_SIZE] = {0};
        bool result = parcel.WritePointer((uintptr_t)writePointer);
        AssertEqual(result, true, "test_ReadPointer_001 result did not equal true as expected.", state);

        char* readPointer = reinterpret_cast<char*>(parcel.ReadPointer());
        AssertEqual(readPointer, writePointer,
            "test_ReadPointer_001 readPointer did not equal writePointer as expected.", state);
    }
    BENCHMARK_LOGD("ParcelTest test_ReadPointer_001 end.");
}

/**
 * @tc.name: test_parcel_WriteAndRead_String006
 * @tc.desc: test parcel CString read write.
 * @tc.type: FUNC
 */
BENCHMARK_F(BenchmarkParcelTest, test_parcel_WriteAndRead_String006)(benchmark::State& state)
{
    BENCHMARK_LOGD("ParcelTest test_parcel_WriteAndRead_String006 start.");
    while (state.KeepRunning()) {
        Parcel parcel1(nullptr);
        u16string str16Write = u"12345";
        bool result = parcel1.WriteString16(str16Write);
        AssertEqual(result, true, "test_parcel_WriteAndRead_String006 result did not equal true as expected.", state);

        u16string str16Read;
        result = parcel1.ReadString16(str16Read);
        AssertEqual(result, true, "test_parcel_WriteAndRead_String006 result did not equal true as expected.", state);
        AssertEqual(COMPARE_STRING_RESULT, str16Read.compare(str16Write),
            "test_parcel_WriteAndRead_String006 str16Read.compare(str16Write) did not equal 0 as expected.", state);

        Parcel parcel2(nullptr);
        AssertTrue(CopyOldParcelToNewParcel(parcel1, parcel2, state),
            "CopyOldParcelToNewParcel(parcel1, parcel2, state) did not equal true as expected.", state);

        result = parcel2.ReadString16(str16Read);
        AssertEqual(result, true, "test_parcel_WriteAndRead_String006 result did not equal true as expected.", state);
        AssertEqual(COMPARE_STRING_RESULT, str16Read.compare(str16Write),
            "test_parcel_WriteAndRead_String006 str16Read.compare(str16Write) did not equal 0 as expected.", state);
    }
    BENCHMARK_LOGD("ParcelTest test_parcel_WriteAndRead_String006 end.");
}

/**
 * @tc.name: test_parcel_parcelable_004
 * @tc.desc: test parcel ReadStrongParcelable obj.
 * @tc.type: FUNC
 */
BENCHMARK_F(BenchmarkParcelTest, test_parcel_parcelable_004)(benchmark::State& state)
{
    BENCHMARK_LOGD("ParcelTest test_parcel_parcelable_004 start.");
    while (state.KeepRunning()) {
        Parcel parcel(nullptr);
        sptr<TestParcelable> parcelableWrite = new TestParcelable();
        bool result = parcel.WriteStrongParcelable(parcelableWrite);
        AssertEqual(result, true, "test_parcel_parcelable_004 result did not equal true as expected.", state);

        sptr<TestParcelable> parcelableRead = parcel.ReadStrongParcelable<TestParcelable>();
        AssertUnequal(nullptr, parcelableRead,
            "test_parcel_parcelable_004 nullptr equal to parcelableRead as expected.", state);
    }
    BENCHMARK_LOGD("ParcelTest test_parcel_parcelable_004 end.");
}

/**
 * @tc.name: test_SkipBytes_001
 * @tc.desc: test parcel Skip Bytes function.
 * @tc.type: FUNC
 */
BENCHMARK_F(BenchmarkParcelTest, test_SkipBytes_001)(benchmark::State& state)
{
    BENCHMARK_LOGD("ParcelTest test_SkipBytes_001 start.");
    while (state.KeepRunning()) {
        Parcel parcel(nullptr);
        char buffer[CAPACITY_THRESHOLD] = {0};
        bool result = parcel.WriteBuffer(static_cast<const void *>(buffer), CAPACITY_THRESHOLD);
        AssertEqual(result, true, "test_SkipBytes_001 result did not equal true as expected.", state);

        size_t skipSize = 1024;
        parcel.SkipBytes(skipSize);
        size_t readableSize = parcel.GetReadableBytes();
        AssertEqual((skipSize+readableSize), CAPACITY_THRESHOLD,
            "test_SkipBytes_001 (skipSize+readableSize) did not equal CAPACITY_THRESHOLD as expected.", state);

        skipSize = CAPACITY_THRESHOLD;
        parcel.SkipBytes(skipSize);
        readableSize = parcel.GetReadableBytes();
        AssertEqual(readableSize, DATA_READBYTES_INIT_SIZE,
            "test_SkipBytes_001 readableSize did not equal 0 as expected.", state);
    }
    BENCHMARK_LOGD("ParcelTest test_SkipBytes_001 end.");
}

/**
 * @tc.name: test_WriteObject_001
 * @tc.desc: test parcel Write Object function.
 * @tc.type: FUNC
 */
BENCHMARK_F(BenchmarkParcelTest, test_WriteObject_001)(benchmark::State& state)
{
    BENCHMARK_LOGD("ParcelTest test_WriteObject_001 start.");
    while (state.KeepRunning()) {
        Parcel parcel(nullptr);
        sptr<RemoteObject> remoteWriteNUll;
        bool result = parcel.WriteObject<RemoteObject>(remoteWriteNUll);
        AssertEqual(result, false, "test_WriteObject_001 result did not equal false as expected.", state);

        sptr<RemoteObject> parcelableWrite = new RemoteObject();
        result = parcel.WriteObject<RemoteObject>(parcelableWrite);
        AssertEqual(result, true, "test_WriteObject_001 result did not equal true as expected.", state);

        sptr<RemoteObject> parcelableRead = parcel.ReadObject<RemoteObject>();
        AssertEqual((parcelableRead.GetRefPtr() != nullptr), true,
            "test_WriteObject_001 (parcelableRead.GetRefPtr() != nullptr) did not equal true as expected.", state);
    }
    BENCHMARK_LOGD("ParcelTest test_WriteObject_001 end.");
}
}  // namespace
}  // namespace OHOS
// Run the benchmark
BENCHMARK_MAIN();