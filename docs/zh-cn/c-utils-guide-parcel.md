# 使用Parcel作为数据容器

## 概述

Parcel对象是一个数据容器，其提供一个内存空间以供数据写入。Parcel对象不仅支持诸如`int`,`float`,`double`等基本类型的写入，同时也支持扁平化地写入一个继承了`Parcelable`类的子类对象。可用于IPC中以实现数据通信功能。

## 涉及功能

### OHOS::Parcel
数据/消息的容器类。

#### 具体描述

```cpp
class OHOS::Parcel;
```

包含用于写入以及读出多种类型的数据，包括基本类型、Parcelable对象等。 


`#include <parcel.h>`

#### Public Functions

| 返回类型       | 名称           |
| -------------- | -------------- |
| | **Parcel**() |
| | **Parcel**(Allocator* allocator)<br>构造Parcel对象，并指定内存分配器Allcator。  |
| virtual | **~Parcel**() |
| bool | **CheckOffsets**()<br>检查从当前光标读取对象的操作是否可行。  |
| void | **FlushBuffer**()<br>释放parcel中的数据区域，并重置该parcel状态。  |
| uintptr_t | **GetData**() const<br>获取指向当前parcel中数据起始位置的指针。  |
| size_t | **GetDataCapacity**() const<br>获取当前parcel的总体容量(字节)，即parcel中数据区域的当前总大小。  |
| size_t | **GetDataSize**() const<br>获取当前parcel中已存在数据的总大小。  |
| size_t | **GetMaxCapacity**() const |
| binder_size_t | **GetObjectOffsets**() const<br>获取写入至当前parcel的每一个oject的具体位置。  |
| size_t | **GetOffsetsSize**() const<br>获取当前存储的所有object的位置的总大小。  |
| size_t | **GetReadableBytes**() const<br>获取剩余可从当前parcel读出的总字节数。  |
| size_t | **GetReadPosition**()<br>获取当前读光标位置。  |
| size_t | **GetWritableBytes**() const<br>获取剩余可向当前parcel写入的总字节数。  |
| size_t | **GetWritePosition**()<br>获取当前写光标位置。  |
| void | **InjectOffsets**(binder_size_t offsets, size_t offsetSize)<br>记录一个数组，该数组内包含多个对象的位置偏移量。  |
| bool | **ParseFrom**(uintptr_t data, size_t size)<br>使用当前parcel读入输入数据。  |
| bool | **ReadBool**() |
| bool | **ReadBool**(bool& value) |
| bool | **ReadBoolUnaligned**() |
| bool | **ReadBoolVector**(std::vector< bool >* val) |
| const uint8_t* | **ReadBuffer**(size_t length)<br>从当前parcel中读出一块数据(一块缓存区的数据)。  |
| const char* | **ReadCString**()<br>从当前parcel中读出C-风格的字符串。  |
| double | **ReadDouble**() |
| bool | **ReadDouble**(double& value) |
| bool | **ReadDoubleVector**(std::vector< double >* val) |
| float | **ReadFloat**() |
| bool | **ReadFloat**(float& value) |
| bool | **ReadFloatVector**(std::vector< float >* val) |
| int16_t | **ReadInt16**() |
| bool | **ReadInt16**(int16_t& value) |
| bool | **ReadInt16Unaligned**(int16_t& value) |
| bool | **ReadInt16Vector**(std::vector< int16_t >* val) |
| int32_t | **ReadInt32**() |
| bool | **ReadInt32**(int32_t& value) |
| bool | **ReadInt32Vector**(std::vector< int32_t >* val) |
| int64_t | **ReadInt64**() |
| bool | **ReadInt64**(int64_t& value) |
| bool | **ReadInt64Vector**(std::vector< int64_t >* val) |
| int8_t | **ReadInt8**() |
| bool | **ReadInt8**(int8_t& value) |
| bool | **ReadInt8Unaligned**(int8_t& value) |
| bool | **ReadInt8Vector**(std::vector< int8_t >* val) |
| template <typename T \> <br>sptr< T > | **ReadObject**()<br>从当前parcel读出某一具体对象。  |
| template <typename T \> <br>T* | **ReadParcelable**()<br>从当前parcel读出一个Parcelable(及其子类)对象。  |
| uintptr_t | **ReadPointer**() |
| const std::string | **ReadString**()<br>从当前parcel中读出一个C++字符串(`std::string`)对象。  |
| bool | **ReadString**(std::string& value)<br>从当前parcel读出C++`std::string`字符串对象，并存入输入对象中。  |
| const std::u16string | **ReadString16**()<br>从当前parcel中读出一个UTF-16编码的C++字符串(`std::u16string`)对象。  |
| bool | **ReadString16**(std::u16string& value)<br>从当前parcel读出一个UTF-16编码的C++`std::u16string`字符串对象，并存入输入对象中。  |
| bool | **ReadString16Vector**(std::vector< std::u16string >* val) |
| const std::u16string | **ReadString16WithLength**(int32_t& len)<br>从当前parcel中读出UTF-16编码的C++字符串(`std::u16string`)对象以及其对应长度。  |
| const std::string | **ReadString8WithLength**(int32_t& len)<br>从当前parcel中读出C++字符串(`std::string`)对象以及其对应长度。  |
| bool | **ReadStringVector**(std::vector< std::string >* val) |
| template <typename T \> <br>sptr< T > | **ReadStrongParcelable**()<br>从当前parcel读出一个Parcelable对象，并使用智能指针管理该对象。  |
| uint16_t | **ReadUint16**() |
| bool | **ReadUint16**(uint16_t& value) |
| bool | **ReadUint16Unaligned**(uint16_t& value) |
| bool | **ReadUInt16Vector**(std::vector< uint16_t >* val) |
| uint32_t | **ReadUint32**() |
| bool | **ReadUint32**(uint32_t& value) |
| bool | **ReadUInt32Vector**(std::vector< uint32_t >* val) |
| uint64_t | **ReadUint64**() |
| bool | **ReadUint64**(uint64_t& value) |
| bool | **ReadUInt64Vector**(std::vector< uint64_t >* val) |
| uint8_t | **ReadUint8**() |
| bool | **ReadUint8**(uint8_t& value) |
| bool | **ReadUint8Unaligned**(uint8_t& value) |
| bool | **ReadUInt8Vector**(std::vector< uint8_t >* val) |
| const uint8_t* | **ReadUnpadBuffer**(size_t length)<br>从当前parcel中读出一块数据(一块缓存区的数据)。  |
| template <typename T \> <br>bool | **ReadVector**(std::vector< T >* val, bool(Parcel::*)(T&) Read)<br>从当前parcel读出一个`std::vector`对象。  |
| bool | **RewindRead**(size_t newPosition)<br>将读光标置于指定位置。  |
| bool | **RewindWrite**(size_t offsets)<br>将写光标置于指定位置。  |
| bool | **SetAllocator**(Allocator* allocator)<br>设置当前parcel的内存分配器Allocator对象。  |
| bool | **SetDataCapacity**(size_t newCapacity)<br>设置当前parcel的以字节数为单位的容量大小，即parcel内数据区域的大小。  |
| bool | **SetDataSize**(size_t dataSize)<br>设置当前parcel的已存在数据大小(字节)。  |
| bool | **SetMaxCapacity**(size_t maxCapacity)<br>设置当前parcel的以字节为单位的最大容量。  |
| void | **SkipBytes**(size_t bytes)<br>在读操作中，跳过接下来由`bytes`指定的几个字节。  |
| bool | **WriteBool**(bool value) |
| bool | **WriteBoolUnaligned**(bool value) |
| bool | **WriteBoolVector**(const std::vector< bool >& val) |
| bool | **WriteBuffer**(const void* data, size_t size) |
| bool | **WriteBufferAddTerminator**(const void* data, size_t size, size_t typeSize) |
| bool | **WriteCString**(const char* value)<br>向当前parcel写入一个C风格的字符串。  |
| bool | **WriteDouble**(double value) |
| bool | **WriteDoubleVector**(const std::vector< double >& val) |
| bool | **WriteFloat**(float value) |
| bool | **WriteFloatVector**(const std::vector< float >& val) |
| bool | **WriteInt16**(int16_t value) |
| bool | **WriteInt16Unaligned**(int16_t value) |
| bool | **WriteInt16Vector**(const std::vector< int16_t >& val) |
| bool | **WriteInt32**(int32_t value) |
| bool | **WriteInt32Vector**(const std::vector< int32_t >& val) |
| bool | **WriteInt64**(int64_t value) |
| bool | **WriteInt64Vector**(const std::vector< int64_t >& val) |
| bool | **WriteInt8**(int8_t value) |
| bool | **WriteInt8Unaligned**(int8_t value) |
| bool | **WriteInt8Vector**(const std::vector< int8_t >& val) |
| template <typename T \> <br>bool | **WriteObject**(const sptr< T >& object)<br>向当前parcel写入某一具体对象。  |
| bool | **WriteParcelable**(const Parcelable* object)<br>向当前parcel写入Parcelable对象。  |
| bool | **WritePointer**(uintptr_t value) |
| bool | **WriteRemoteObject**(const Parcelable* object)<br>向当前parcel中写入remote对象。  |
| bool | **WriteString**(const std::string& value)<br>向当前parcel写入一个C++的std::string字符串。  |
| bool | **WriteString16**(const std::u16string& value)<br>向当前parcel写入一个C++的std::string字符串。  |
| bool | **WriteString16Vector**(const std::vector< std::u16string >& val) |
| bool | **WriteString16WithLength**(const char16_t* value, size_t len)<br>向当前parcel写入一个UTF-16编码的字符串。  |
| bool | **WriteString8WithLength**(const char* value, size_t len)<br>向当前parcel写入一个UTF-8编码的字符串。  |
| bool | **WriteStringVector**(const std::vector< std::string >& val) |
| bool | **WriteStrongParcelable**(const sptr< Parcelable >& object)<br>将Parcelable对象的`HOLD_OBJECT`行为开启后写入当前parcel。  |
| bool | **WriteUint16**(uint16_t value) |
| bool | **WriteUint16Unaligned**(uint16_t value) |
| bool | **WriteUInt16Vector**(const std::vector< uint16_t >& val) |
| bool | **WriteUint32**(uint32_t value) |
| bool | **WriteUInt32Vector**(const std::vector< uint32_t >& val) |
| bool | **WriteUint64**(uint64_t value) |
| bool | **WriteUInt64Vector**(const std::vector< uint64_t >& val) |
| bool | **WriteUint8**(uint8_t value) |
| bool | **WriteUint8Unaligned**(uint8_t value) |
| bool | **WriteUInt8Vector**(const std::vector< uint8_t >& val) |
| bool | **WriteUnpadBuffer**(const void* data, size_t size)<br>基于数据区指针及数据长度写入一段数据，功能与WriteBuffer完全相同，`注:`该接口内部会自动计算并写入对齐长度|
| template <typename T1 ,typename T2 \> <br>bool | **WriteVector**(const std::vector< T1 >& val, bool(Parcel::*)(T2) Write)<br>向当前parcel写入一个`std::vector`对象。  |

#### Protected Functions

| 返回类型       | 名称           |
| -------------- | -------------- |
| bool | **EnsureObjectsCapacity**()<br>确保当前写入对象数量小于对象容量。  |
| bool | **WriteObjectOffset**(binder_size_t offset)<br>记录待写入对象的具体位置，该位置表示为相对于数据区域起始处的偏移量。  |

### OHOS::Parcelable

定义了实例可被写入至某一Parcel的类接口。 

#### 具体描述

```cpp
class OHOS::Parcelable;
```

**提示**: 如果当前对象为remote，其地址将被用于在内核中进行数据传输。 

`#include <parcel.h>`

继承自 OHOS::RefBase

#### Public Types

|                | 名称           |
| -------------- | -------------- |
| enum| **BehaviorFlag** { IPC = 0x01, RPC = 0x02, HOLD_OBJECT = 0x10}<br>用于描述Parcelable对象具体行为的枚举类。  |

#### Public Functions

| 返回类型       | 名称           |
| -------------- | -------------- |
| | **Parcelable**() |
| | **Parcelable**(bool asRemote)<br>构造一个Parcelable对象。  |
| virtual | **~Parcelable**() = default |
| void | **ClearBehavior**(BehaviorFlag b) const<br>清除指定行为标志位。  |
| virtual bool | **Marshalling**(Parcel& parcel) const =0<br>向指定Parcel对象中写入当前Parcelable对象。  |
| void | **SetBehavior**(BehaviorFlag b) const<br>设置指定行为标志位。  |
| bool | **TestBehavior**(BehaviorFlag b) const<br>检查指定行为是否已开启。  |

#### Public Attributes

|                | 名称           |
| -------------- | -------------- |
| bool | **asRemote_** <br>指明对象是否为remote的标志位。  |
| uint8_t | **behavior_** <br>指定已开启行为的具体值。  |


## 使用示例

1. 使用方法(伪代码)

```c++
// 写入端以某种顺序写入数据
struct TestData {
    bool booltest;
    int8_t int8test;
    int16_t int16test;
    int32_t int32test;
    uint8_t uint8test;
    uint16_t uint16test;
    uint32_t uint32test;
};

...

Parcel parcel(nullptr);
struct TestData data = { true, -0x34, 0x5634, -0x12345678, 0x34, 0x5634, 0x12345678 };
bool result = false;

result = parcel.WriteBool(data.booltest);
if (!result) {
    // 写失败处理
}

result = parcel.WriteInt8(data.int8test);
if (!result) {
    // 写失败处理
}

result = parcel.WriteInt16(data.int16test);
if (!result) {
    // 写失败处理
}

result = parcel.WriteInt32(data.int32test);
if (!result) {
    // 写失败处理
}

result = parcel.WriteUint8(data.uint8test);
if (!result) {
    // 写失败处理
}

result = parcel.WriteUint16(data.uint16test);
if (!result) {
    // 写失败处理
}

result = parcel.WriteUint32(data.uint32test);
if (!result) {
    // 写失败处理
}
```

```c++
// 接收端根据写入端写入顺序读取数据
bool readbool = parcel.ReadBool();

int8_t readint8 = parcel.ReadInt8();

int16_t readint16 = parcel.ReadInt16();

int32_t readint32 = parcel.ReadInt32();

uint8_t readuint8 = parcel.ReadUint8();

uint16_t readuint16 = parcel.ReadUint16();

uint32_t readuint32 = parcel.ReadUint32();
```

2. 常见接口限制及使用误区

- ReadBuffer/ReadUnpadBuffer/WriteBuffer/WriteUnpadBuffer

    不推荐ReadBuffer与WriteBuffer/WriteUnpadBuffer对应配合使用，可能因为对齐问题导致ReadBuffer后的Read操作从错误的偏移位置进行读取，进而导致读取异常；
    ReadUnpadBuffer与WriteBuffer/WriteUnpadBuffer配合使用为正确的使用方式。

```cpp
// ReadBuffer: 读取buffer，且内部数据区按参数设置长度偏移，不考虑数据对齐
// ReadUnpadBuffer: 读取buffer，内部数据区基于读取长度自动计算对齐并偏移，将数据对齐考虑在内
// WriteBuffer: 写入数据，内部数据区会基于写入长度计算对齐长度并自动偏移
// WriteUnpadBuffer: 与WriteBuffer完全相同

struct Padded {
    char title;
    int32_t handle;
    uint64_t cookie;
};

struct Unpadded {
    char tip;
};

Parcel parcel(nullptr);
const struct Padded pad = { 'p', 0x34567890, -0x2345678998765432 };
const struct Unpadded unpad = { 'u' };
// CASE 1:写入对齐数据
// 后续代码为单case下不同情况的使用代码，并非真实连续调用
parcel.WriteBuffer(static_cast<const void *>(&pad), sizeof(struct Padded));
parcel.WriteInt32(1);

// 错误使用但结果正常：
parcel.ReadBuffer(sizeof(struct Padded)); // 可以正常读取buffer内容
parcel.ReadInt32(); // 后续读取内容正常

// 正确使用：
parcel.ReadUnpadBuffer(sizeof(struct Padded)); // 可以正常读取buffer内容
parcel.ReadInt32(); // 后续读取内容正常

// CASE 2:写入非对齐数据
// 后续代码为单case下不同情况的使用代码，并非真实连续调用
parcel.WriteBuffer(static_cast<const void *>(&unpad), sizeof(struct Unpadded));
parcel.WriteInt32(1);

// 错误使用，结果异常：
parcel.ReadBuffer(sizeof(struct Unpadded)); // 可以正常读取buffer内容
parcel.ReadInt32(); // 后续读取内容异常

// 正确使用：
parcel.ReadUnpadBuffer(sizeof(struct Unpadded)); // 可以正常读取buffer内容
parcel.ReadInt32(); // 后续读取内容正常
```

- 基础类型的Read接口，如ReadInt32，ReadFloat等读取失败
    
    当前在基础Read接口内加入了安全校验机制，当发现基础类型的读操作在读取Object对象数据内容时，该行为会被拦截

```cpp
// 伪代码：

Parcel parcel(nullptr);
Parcelable object;
parcel.WriteRemoteObject(object);
parcel.ReadInt32(); // False

```

- 使用WriteBuffer写入字符串，忽略结束符导致读取字符串长度异常

    WriteBuffer接口并非专门处理字符串写入的接口，因此错误传递写入长度，可能会导致字符串的结束符丢失

```cpp
// 伪代码：

string str = "abcdefg";
Parcel parcel(nullptr);
char *strPtr = str.c_str();
auto len = str.length();
parcel.WriteBuffer(strPtr, len);

parcel.ReadBuffer(len); // 读取字符串长度异常

```

3. 测试用例编译运行方法

- 测试用例代码参见 base/test/unittest/common/utils_parcel_test.cpp

- 使用开发者自测试框架，使用方法参见：[开发自测试执行框架-测试用例执行](https://gitee.com/openharmony/testfwk_developer_test#%E6%B5%8B%E8%AF%95%E7%94%A8%E4%BE%8B%E6%89%A7%E8%A1%8C)

- 使用以下具体命令以运行`parcel.h`对应测试用例

```bash
run -t UT -tp utils -ts UtilsParcelTest
```