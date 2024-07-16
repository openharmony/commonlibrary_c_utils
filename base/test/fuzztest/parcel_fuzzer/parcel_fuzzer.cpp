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

#include "parcel_fuzzer.h"

#include "fuzz_log.h"
#include "fuzzer/FuzzedDataProvider.h"
#include "parcel.h"
#include "securec.h"
#include "string_ex.h"

using namespace std;

namespace OHOS {
const uint8_t MAX_BUFFER_SIZE = 255;
const uint8_t MAX_STRING_LENGTH = 255;
const uint8_t MAX_VECTOR_SIZE = 10;
const int MAX_OPERATIONS_GENERAL = 500;
const int MAX_OPERATIONS_UNALIGNED = 100;
const int MAX_OPERATIONS_OTHER = 15;

enum Group {
    GENERAL_GROUP = 0,
    UNALIGNED_GROUP = 1,
    OTHER_GROUP = 2,
};

#define PARCEL_NO_INPUT_WITH_RETURN(T, FUN) \
    [](FuzzedDataProvider*, Parcel& p) {    \
        FUZZ_LOGI("%{public}s", #FUN);      \
        T t = p.FUN();                      \
        (void)t;                            \
    }

#define PARCEL_INT_INPUT_WITH_BOOL_RETURN(T, FUN) \
    [](FuzzedDataProvider* d, Parcel& p) {        \
        FUZZ_LOGI("%{public}s", #FUN);            \
        T data = d->ConsumeIntegral<T>();         \
        bool t = p.FUN(data);                     \
        (void)t;                                  \
    }

#define PARCEL_REF_INPUT_WITH_BOOL_RETURN(T, FUN) \
    [](FuzzedDataProvider*, Parcel& p) {          \
        FUZZ_LOGI("%{public}s", #FUN);            \
        T out {};                                 \
        bool t = p.FUN(out);                      \
        (void)t;                                  \
    }

#define PARCEL_WRITE_VECTOR_WITH_BOOL_RETURN(T, FUN)                      \
    [](FuzzedDataProvider* d, Parcel& p) {                                \
        FUZZ_LOGI("%{public}s", #FUN);                                    \
        size_t n = d->ConsumeIntegralInRange<size_t>(1, MAX_VECTOR_SIZE); \
        void* buf = malloc(n * sizeof(T));                                \
        size_t wb = d->ConsumeData(buf, n * sizeof(T));                   \
        auto* tmp = static_cast<const T*>(buf);                           \
        std::vector<T> data(tmp, tmp + wb / sizeof(T));                   \
        bool t = p.FUN(data);                                             \
        (void)t;                                                          \
        free(buf);                                                        \
    }

#define PARCEL_POINT_INPUT_WITH_BOOL_RETURN(T, FUN) \
    [](FuzzedDataProvider*, Parcel& p) {            \
        FUZZ_LOGI("%{public}s", #FUN);              \
        vector<T> out;                              \
        bool t = p.FUN(&out);                       \
        (void)t;                                    \
    }

class TestParcelable : public virtual Parcelable {
public:
    TestParcelable() = default;
    explicit TestParcelable(bool asRemote)
    {
        asRemote_ = asRemote;
    };
    ~TestParcelable() = default;

    bool Marshalling(Parcel& parcel) const override;
    static TestParcelable* Unmarshalling(Parcel& parcel);
    static bool Marshalling(Parcel& parcel, const sptr<TestParcelable>& object);

public:
    int32_t int32Write_ = -0x12345678;
    int32_t int32Read_;
};

bool TestParcelable::Marshalling(Parcel& parcel) const
{
    bool result = parcel.WriteInt32(this->int32Write_);
    return result;
}

TestParcelable* TestParcelable::Unmarshalling(Parcel& parcel)
{
    auto* read = new TestParcelable();
    read->int32Read_ = parcel.ReadInt32();
    return read;
}

bool TestParcelable::Marshalling(Parcel& parcel, const sptr<TestParcelable>& object)
{
    bool result = parcel.WriteInt32(object->int32Write_);
    return result;
}

class RemoteObject : public virtual Parcelable {
public:
    RemoteObject()
    {
        asRemote_ = true;
    };
    bool Marshalling(Parcel& parcel) const override;
    static sptr<RemoteObject> Unmarshalling(Parcel& parcel);
};

bool RemoteObject::Marshalling(Parcel& parcel) const
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

sptr<RemoteObject> RemoteObject::Unmarshalling(Parcel& parcel)
{
    const uint8_t* buffer = parcel.ReadBuffer(sizeof(parcel_flat_binder_object), false);
    if (buffer == nullptr) {
        return nullptr;
    }
    sptr<RemoteObject> obj = new RemoteObject();
    return obj;
}

const std::vector<std::function<void(FuzzedDataProvider*, Parcel&)>> operations = {
    PARCEL_NO_INPUT_WITH_RETURN(size_t, GetDataSize),
    PARCEL_NO_INPUT_WITH_RETURN(uintptr_t, GetData),
    PARCEL_NO_INPUT_WITH_RETURN(size_t, GetOffsetsSize),
    PARCEL_NO_INPUT_WITH_RETURN(size_t, GetWritableBytes),
    PARCEL_NO_INPUT_WITH_RETURN(size_t, GetReadableBytes),
    PARCEL_NO_INPUT_WITH_RETURN(size_t, GetDataCapacity),
    PARCEL_NO_INPUT_WITH_RETURN(size_t, GetMaxCapacity),

    [](FuzzedDataProvider* dataProvider, Parcel& parcel) {
        FUZZ_LOGI("WriteBool");
        bool booltest = dataProvider->ConsumeBool();
        parcel.WriteBool(booltest);
    },

    PARCEL_INT_INPUT_WITH_BOOL_RETURN(int8_t, WriteInt8),
    PARCEL_INT_INPUT_WITH_BOOL_RETURN(int16_t, WriteInt16),
    PARCEL_INT_INPUT_WITH_BOOL_RETURN(int32_t, WriteInt32),
    PARCEL_INT_INPUT_WITH_BOOL_RETURN(int64_t, WriteInt64),
    PARCEL_INT_INPUT_WITH_BOOL_RETURN(uint8_t, WriteUint8),
    PARCEL_INT_INPUT_WITH_BOOL_RETURN(uint16_t, WriteUint16),
    PARCEL_INT_INPUT_WITH_BOOL_RETURN(uint32_t, WriteUint32),
    PARCEL_INT_INPUT_WITH_BOOL_RETURN(uint64_t, WriteUint64),
    PARCEL_INT_INPUT_WITH_BOOL_RETURN(uintptr_t, WritePointer),

    [](FuzzedDataProvider* dataProvider, Parcel& parcel) {
        FUZZ_LOGI("WriteFloat");
        float floattest = dataProvider->ConsumeFloatingPoint<float>();
        parcel.WriteFloat(floattest);
    },

    [](FuzzedDataProvider* dataProvider, Parcel& parcel) {
        FUZZ_LOGI("WriteDouble");
        double doubletest = dataProvider->ConsumeFloatingPoint<double>();
        parcel.WriteDouble(doubletest);
    },

    [](FuzzedDataProvider* dataProvider, Parcel& parcel) {
        FUZZ_LOGI("WriteBuffer");
        size_t bufferSize = dataProvider->ConsumeIntegralInRange<size_t>(1, MAX_BUFFER_SIZE);
        void* buffer = malloc(bufferSize);
        size_t writtenBytes = dataProvider->ConsumeData(buffer, bufferSize);
        parcel.WriteBuffer(buffer, writtenBytes);
        free(buffer);
    },

    [](FuzzedDataProvider* dataProvider, Parcel& parcel) {
        FUZZ_LOGI("WriteBufferAddTerminator");
        size_t bufferSize = dataProvider->ConsumeIntegralInRange<size_t>(1, MAX_BUFFER_SIZE);
        void* buffer = malloc(bufferSize);
        size_t writtenBytes = dataProvider->ConsumeData(buffer, bufferSize);
        const size_t maxTypeSize = 4; // max type size is 4 bytes.
        if (writtenBytes == 0) {
            free(buffer);
            return;
        }
        size_t typeSize = dataProvider->ConsumeIntegralInRange<size_t>(0, maxTypeSize);
        parcel.WriteBufferAddTerminator(buffer, writtenBytes, typeSize);
        free(buffer);
    },

    [](FuzzedDataProvider* dataProvider, Parcel& parcel) {
        FUZZ_LOGI("WriteUnpadBuffer");
        size_t bufferSize = dataProvider->ConsumeIntegralInRange<size_t>(1, MAX_BUFFER_SIZE);
        void* buffer = malloc(bufferSize);
        size_t writtenBytes = dataProvider->ConsumeData(buffer, bufferSize);
        parcel.WriteUnpadBuffer(buffer, writtenBytes);
        free(buffer);
    },

    [](FuzzedDataProvider* dataProvider, Parcel& parcel) {
        FUZZ_LOGI("WriteCString");
        string teststr = dataProvider->ConsumeRandomLengthString(MAX_STRING_LENGTH);
        parcel.WriteCString(teststr.c_str());
    },

    [](FuzzedDataProvider* dataProvider, Parcel& parcel) {
        FUZZ_LOGI("WriteString");
        string teststr = dataProvider->ConsumeRandomLengthString(MAX_STRING_LENGTH);
        parcel.WriteString(teststr);
    },

    [](FuzzedDataProvider* dataProvider, Parcel& parcel) {
        FUZZ_LOGI("WriteString16");
        string utf8 = dataProvider->ConsumeRandomLengthString(MAX_STRING_LENGTH);
        u16string utf16 = Str8ToStr16(utf8);
        parcel.WriteString16(utf16);
    },

    [](FuzzedDataProvider* dataProvider, Parcel& parcel) {
        FUZZ_LOGI("WriteString16WithLength");
        string utf8 = dataProvider->ConsumeRandomLengthString(MAX_STRING_LENGTH);
        u16string utf16 = Str8ToStr16(utf8);
        char16_t* value = utf16.data();
        parcel.WriteString16WithLength(value, utf16.length());
    },

    [](FuzzedDataProvider* dataProvider, Parcel& parcel) {
        FUZZ_LOGI("WriteString8WithLength");
        string utf8 = dataProvider->ConsumeRandomLengthString(MAX_STRING_LENGTH);
        char* value = utf8.data();
        parcel.WriteString8WithLength(value, utf8.length());
    },

    [](FuzzedDataProvider* dataProvider, Parcel& parcel) {
        FUZZ_LOGI("WriteParcelable");
        bool remoteFlag = dataProvider->ConsumeBool();
        TestParcelable parcelableWrite(remoteFlag);
        parcel.WriteParcelable(&parcelableWrite);
    },

    [](FuzzedDataProvider* dataProvider, Parcel& parcel) {
        FUZZ_LOGI("WriteStrongParcelable");
        bool remoteFlag = dataProvider->ConsumeBool();
        sptr<TestParcelable> parcelableWrite = new TestParcelable(remoteFlag);
        parcel.WriteStrongParcelable(parcelableWrite);
    },

    [](FuzzedDataProvider* dataProvider, Parcel& parcel) {
        FUZZ_LOGI("WriteRemoteObject");
        RemoteObject obj;
        parcel.WriteRemoteObject(&obj);
    },

    [](FuzzedDataProvider* dataProvider, Parcel& parcel) {
        FUZZ_LOGI("WriteObject");
        bool remoteFlag = dataProvider->ConsumeBool();
        sptr<TestParcelable> parcelableWrite = new TestParcelable(remoteFlag);
        parcel.WriteObject(parcelableWrite);
    },

    [](FuzzedDataProvider* dataProvider, Parcel& parcel) {
        FUZZ_LOGI("ParseFrom");
        void* buffer = nullptr;
        size_t testdataSize = parcel.GetDataSize();
        if (testdataSize > 0) {
            buffer = malloc(testdataSize);
            memcpy_s(buffer, testdataSize, reinterpret_cast<const uint8_t*>(parcel.GetData()), testdataSize);
        }
        Parcel parcel2(nullptr);
        parcel2.ParseFrom(reinterpret_cast<uintptr_t>(buffer), testdataSize);
    },

    PARCEL_NO_INPUT_WITH_RETURN(bool, ReadBool),
    PARCEL_NO_INPUT_WITH_RETURN(int8_t, ReadInt8),
    PARCEL_NO_INPUT_WITH_RETURN(int16_t, ReadInt16),
    PARCEL_NO_INPUT_WITH_RETURN(int32_t, ReadInt32),
    PARCEL_NO_INPUT_WITH_RETURN(int64_t, ReadInt64),
    PARCEL_NO_INPUT_WITH_RETURN(uint8_t, ReadUint8),
    PARCEL_NO_INPUT_WITH_RETURN(uint16_t, ReadUint16),
    PARCEL_NO_INPUT_WITH_RETURN(uint32_t, ReadUint32),
    PARCEL_NO_INPUT_WITH_RETURN(uint64_t, ReadUint64),
    PARCEL_NO_INPUT_WITH_RETURN(float, ReadFloat),
    PARCEL_NO_INPUT_WITH_RETURN(double, ReadDouble),
    PARCEL_NO_INPUT_WITH_RETURN(uintptr_t, ReadPointer),

    PARCEL_REF_INPUT_WITH_BOOL_RETURN(bool, ReadBool),
    PARCEL_REF_INPUT_WITH_BOOL_RETURN(int8_t, ReadInt8),
    PARCEL_REF_INPUT_WITH_BOOL_RETURN(int16_t, ReadInt16),
    PARCEL_REF_INPUT_WITH_BOOL_RETURN(int32_t, ReadInt32),
    PARCEL_REF_INPUT_WITH_BOOL_RETURN(int64_t, ReadInt64),
    PARCEL_REF_INPUT_WITH_BOOL_RETURN(uint8_t, ReadUint8),
    PARCEL_REF_INPUT_WITH_BOOL_RETURN(uint16_t, ReadUint16),
    PARCEL_REF_INPUT_WITH_BOOL_RETURN(uint32_t, ReadUint32),
    PARCEL_REF_INPUT_WITH_BOOL_RETURN(uint64_t, ReadUint64),
    PARCEL_REF_INPUT_WITH_BOOL_RETURN(float, ReadFloat),
    PARCEL_REF_INPUT_WITH_BOOL_RETURN(double, ReadDouble),

    [](FuzzedDataProvider* dataProvider, Parcel& parcel) {
        FUZZ_LOGI("ReadUnpadBuffer");
        size_t bufferSize = dataProvider->ConsumeIntegralInRange<size_t>(1, MAX_BUFFER_SIZE);
        parcel.ReadUnpadBuffer(bufferSize);
    },

    PARCEL_NO_INPUT_WITH_RETURN(const char*, ReadCString),
    PARCEL_NO_INPUT_WITH_RETURN(string, ReadString),
    PARCEL_REF_INPUT_WITH_BOOL_RETURN(string, ReadString),
    PARCEL_NO_INPUT_WITH_RETURN(u16string, ReadString16),
    PARCEL_REF_INPUT_WITH_BOOL_RETURN(u16string, ReadString16),

    [](FuzzedDataProvider*, Parcel& parcel) {
        FUZZ_LOGI("ReadString16WithLength");
        int32_t strlen = 0;
        parcel.ReadString16WithLength(strlen);
    },

    [](FuzzedDataProvider*, Parcel& parcel) {
        FUZZ_LOGI("ReadString8WithLength");
        int32_t strlen = 0;
        parcel.ReadString8WithLength(strlen);
    },

    PARCEL_NO_INPUT_WITH_RETURN(size_t, GetReadPosition),
    PARCEL_NO_INPUT_WITH_RETURN(size_t, GetWritePosition),

    PARCEL_NO_INPUT_WITH_RETURN(sptr<TestParcelable>, ReadParcelable<TestParcelable>),
    PARCEL_NO_INPUT_WITH_RETURN(sptr<TestParcelable>, ReadStrongParcelable<TestParcelable>),
    PARCEL_NO_INPUT_WITH_RETURN(bool, CheckOffsets),
    PARCEL_NO_INPUT_WITH_RETURN(sptr<TestParcelable>, ReadObject<TestParcelable>),

    [](FuzzedDataProvider*, Parcel& parcel) {
        FUZZ_LOGI("SetAllocator");
        parcel.SetAllocator(new DefaultAllocator());
    },

    [](FuzzedDataProvider*, Parcel& parcel) {
        FUZZ_LOGI("InjectOffsets");
        Parcel parcel2(nullptr);
        parcel2.InjectOffsets(parcel.GetObjectOffsets(), parcel.GetOffsetsSize());
    },

    [](FuzzedDataProvider*, Parcel& parcel) {
        FUZZ_LOGI("FlushBuffer");
        parcel.FlushBuffer();
    },

    PARCEL_WRITE_VECTOR_WITH_BOOL_RETURN(int8_t, WriteInt8Vector),
    PARCEL_WRITE_VECTOR_WITH_BOOL_RETURN(int32_t, WriteInt32Vector),
    PARCEL_WRITE_VECTOR_WITH_BOOL_RETURN(int64_t, WriteInt64Vector),
    PARCEL_WRITE_VECTOR_WITH_BOOL_RETURN(uint8_t, WriteUInt8Vector),
    PARCEL_WRITE_VECTOR_WITH_BOOL_RETURN(uint16_t, WriteUInt16Vector),
    PARCEL_WRITE_VECTOR_WITH_BOOL_RETURN(uint32_t, WriteUInt32Vector),
    PARCEL_WRITE_VECTOR_WITH_BOOL_RETURN(uint64_t, WriteUInt64Vector),
    PARCEL_WRITE_VECTOR_WITH_BOOL_RETURN(float, WriteFloatVector),
    PARCEL_WRITE_VECTOR_WITH_BOOL_RETURN(double, WriteDoubleVector),

    [](FuzzedDataProvider* dataProvider, Parcel& parcel) {
        FUZZ_LOGI("WriteStringVector");
        size_t vectorSize = dataProvider->ConsumeIntegralInRange<size_t>(1, MAX_VECTOR_SIZE);
        std::vector<string> testdata(vectorSize);
        for (size_t i = 0; i < vectorSize; i++) {
            size_t strlen = dataProvider->ConsumeIntegralInRange<size_t>(1, MAX_STRING_LENGTH);
            std::vector<char> vec = dataProvider->ConsumeBytesWithTerminator<char>(strlen);
            testdata[i] = vec.data();
        }
        parcel.WriteStringVector(testdata);
    },

    [](FuzzedDataProvider* dataProvider, Parcel& parcel) {
        FUZZ_LOGI("WriteString16Vector");
        size_t vectorSize = dataProvider->ConsumeIntegralInRange<size_t>(1, MAX_VECTOR_SIZE);
        std::vector<u16string> testdata(vectorSize);
        for (size_t i = 0; i < vectorSize; i++) {
            size_t strlen = dataProvider->ConsumeIntegralInRange<size_t>(1, MAX_STRING_LENGTH);
            std::vector<char> vec = dataProvider->ConsumeBytesWithTerminator<char>(strlen);
            string str = vec.data();
            testdata[i] = Str8ToStr16(str);
        }
        parcel.WriteString16Vector(testdata);
    },

    PARCEL_POINT_INPUT_WITH_BOOL_RETURN(int32_t, ReadInt32Vector),
    PARCEL_POINT_INPUT_WITH_BOOL_RETURN(int64_t, ReadInt64Vector),
    PARCEL_POINT_INPUT_WITH_BOOL_RETURN(uint32_t, ReadUInt32Vector),
    PARCEL_POINT_INPUT_WITH_BOOL_RETURN(uint64_t, ReadUInt64Vector),
    PARCEL_POINT_INPUT_WITH_BOOL_RETURN(float, ReadFloatVector),
    PARCEL_POINT_INPUT_WITH_BOOL_RETURN(double, ReadDoubleVector),
    PARCEL_POINT_INPUT_WITH_BOOL_RETURN(string, ReadStringVector),
    PARCEL_POINT_INPUT_WITH_BOOL_RETURN(u16string, ReadString16Vector),
};

const std::vector<std::function<void(FuzzedDataProvider*, Parcel&)>> unaligned_operations = {
    // error call ReadDouble after following methods, will crash with "signal SIGBUS: illegal alignment".
    [](FuzzedDataProvider* dataProvider, Parcel& parcel) {
        FUZZ_LOGI("ReadBuffer");
        size_t bufferSize = dataProvider->ConsumeIntegralInRange<size_t>(1, MAX_BUFFER_SIZE);
        parcel.ReadBuffer(bufferSize);
    },

    [](FuzzedDataProvider* dataProvider, Parcel& parcel) {
        FUZZ_LOGI("SkipBytes");
        size_t bufferSize = dataProvider->ConsumeIntegralInRange<size_t>(1, MAX_BUFFER_SIZE);
        parcel.SkipBytes(bufferSize);
    },

    [](FuzzedDataProvider* dataProvider, Parcel& parcel) {
        FUZZ_LOGI("RewindRead");
        size_t bufferSize = dataProvider->ConsumeIntegralInRange<size_t>(1, MAX_BUFFER_SIZE);
        parcel.RewindRead(bufferSize);
    },

    PARCEL_POINT_INPUT_WITH_BOOL_RETURN(bool, ReadBoolVector),
    PARCEL_POINT_INPUT_WITH_BOOL_RETURN(int8_t, ReadInt8Vector),
    PARCEL_POINT_INPUT_WITH_BOOL_RETURN(int16_t, ReadInt16Vector),
    PARCEL_POINT_INPUT_WITH_BOOL_RETURN(uint8_t, ReadUInt8Vector),
    PARCEL_POINT_INPUT_WITH_BOOL_RETURN(uint16_t, ReadUInt16Vector),

    PARCEL_NO_INPUT_WITH_RETURN(bool, ReadBoolUnaligned),
    PARCEL_REF_INPUT_WITH_BOOL_RETURN(int8_t, ReadInt8Unaligned),
    PARCEL_REF_INPUT_WITH_BOOL_RETURN(int16_t, ReadInt16Unaligned),
    PARCEL_REF_INPUT_WITH_BOOL_RETURN(uint8_t, ReadUint8Unaligned),
    PARCEL_REF_INPUT_WITH_BOOL_RETURN(uint16_t, ReadUint16Unaligned),

    // error call WriteDouble/WriteFloat after RewindWrite, will crash with "signal SIGBUS: illegal alignment".
    [](FuzzedDataProvider* dataProvider, Parcel& parcel) {
        FUZZ_LOGI("RewindWrite");
        size_t bufferSize = dataProvider->ConsumeIntegralInRange<size_t>(1, MAX_BUFFER_SIZE);
        parcel.RewindWrite(bufferSize);
    },
    [](FuzzedDataProvider* dataProvider, Parcel& parcel) {
        FUZZ_LOGI("WriteBoolVector");
        size_t vectorSize = dataProvider->ConsumeIntegralInRange<size_t>(1, MAX_VECTOR_SIZE);
        std::vector<uint8_t> data = dataProvider->ConsumeBytes<uint8_t>(vectorSize);
        if (data.size() > 0) {
            std::vector<bool> testdata(data.size());
            for (size_t i = 0; i < testdata.size(); i++) {
                testdata[i] = 1 & data[i];
            }
            parcel.WriteBoolVector(testdata);
        }
    },

    PARCEL_WRITE_VECTOR_WITH_BOOL_RETURN(int16_t, WriteInt16Vector),

    [](FuzzedDataProvider* dataProvider, Parcel& parcel) {
        FUZZ_LOGI("WriteBoolUnaligned");
        bool booltest = dataProvider->ConsumeBool();
        parcel.WriteBoolUnaligned(booltest);
    },

    PARCEL_INT_INPUT_WITH_BOOL_RETURN(int8_t, WriteInt8Unaligned),
    PARCEL_INT_INPUT_WITH_BOOL_RETURN(int16_t, WriteInt16Unaligned),
    PARCEL_INT_INPUT_WITH_BOOL_RETURN(uint8_t, WriteUint8Unaligned),
    PARCEL_INT_INPUT_WITH_BOOL_RETURN(uint16_t, WriteUint16Unaligned),
};

const std::vector<std::function<void(FuzzedDataProvider*, Parcel&)>> other_operations = {
    PARCEL_NO_INPUT_WITH_RETURN(size_t, GetDataSize),
    PARCEL_NO_INPUT_WITH_RETURN(size_t, GetDataCapacity),
    PARCEL_NO_INPUT_WITH_RETURN(size_t, GetMaxCapacity),

    // cannot call randomly with other operations.
    [](FuzzedDataProvider* dataProvider, Parcel& parcel) {
        FUZZ_LOGI("SetDataCapacity");
        size_t capacity = dataProvider->ConsumeIntegralInRange<size_t>(1, MAX_BUFFER_SIZE);
        parcel.SetDataCapacity(capacity);
    },

    [](FuzzedDataProvider* dataProvider, Parcel& parcel) {
        FUZZ_LOGI("SetDataSize");
        size_t dataSize = dataProvider->ConsumeIntegralInRange<size_t>(1, MAX_BUFFER_SIZE);
        parcel.SetDataSize(dataSize);
    },

    [](FuzzedDataProvider* dataProvider, Parcel& parcel) {
        FUZZ_LOGI("SetMaxCapacity");
        size_t maxCapacity = dataProvider->ConsumeIntegralInRange<size_t>(1, MAX_BUFFER_SIZE);
        parcel.SetMaxCapacity(maxCapacity);
    },
};

void ParcelTestFunc(const uint8_t* data, size_t size, FuzzedDataProvider* dataProvider)
{
    FUZZ_LOGI("ParcelTestFunc start");
    uint8_t opSet = dataProvider->ConsumeIntegralInRange<uint8_t>(GENERAL_GROUP, OTHER_GROUP);
    uint8_t maxGeneral = operations.size() - 1;
    uint8_t maxUnaligned = unaligned_operations.size() - 1;
    uint8_t maxOther = other_operations.size() - 1;
    Parcel parcel1(nullptr);
    int opCnt = 0;

    switch (opSet) {
        case GENERAL_GROUP:
            while (dataProvider->remaining_bytes() > 0 && opCnt++ < MAX_OPERATIONS_GENERAL) {
                uint8_t op = dataProvider->ConsumeIntegralInRange<uint8_t>(0, maxGeneral);
                operations[op](dataProvider, parcel1);
            }
            break;
        case UNALIGNED_GROUP:
            while (dataProvider->remaining_bytes() > 0 && opCnt++ < MAX_OPERATIONS_UNALIGNED) {
                uint8_t op = dataProvider->ConsumeIntegralInRange<uint8_t>(0, maxUnaligned);
                unaligned_operations[op](dataProvider, parcel1);
            }
            break;
        case OTHER_GROUP:
            while (dataProvider->remaining_bytes() > 0 && opCnt++ < MAX_OPERATIONS_OTHER) {
                uint8_t op = dataProvider->ConsumeIntegralInRange<uint8_t>(0, maxOther);
                other_operations[op](dataProvider, parcel1);
            }
            break;
        default:
            break;
    }
    FUZZ_LOGI("ParcelTestFunc end");
}

} // namespace OHOS

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    FuzzedDataProvider dataProvider(data, size);
    OHOS::ParcelTestFunc(data, size, &dataProvider);
    return 0;
}
