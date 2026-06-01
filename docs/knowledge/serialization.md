# 序列化知识

本文只记录 Parcel 和 Parcelable 的边界约束。两者是 IPC/RPC 数据传递的基础设施。

## 核心模型

```
Parcelable    — 可序列化对象的抽象基类，继承自 RefBase
  │             子类必须实现 Marshalling(Parcel &) + static Unmarshalling(Parcel &)
  │
Parcel        — 数据容器，管理读写游标、对象偏移数组、内存分配
  ├─ Write*   — 写入原语/string/vector/Parcelable
  ├─ Read*    — 读取原语/string/vector/Parcelable
  └─ Allocator— 可替换的内存分配器（DefaultAllocator 默认）
```

## 边界/分类

| 概念 | 用途 | 常见误用 |
|---|---|---|
| `Parcelable::Marshalling` | 将对象写入 Parcel | 与 Unmarshalling 的读写顺序不一致 |
| `Parcelable::Unmarshalling(static)` | 从 Parcel 构造对象 | 不校验数据合法性（溢出、越界） |
| `WriteParcelable` vs `WriteRemoteObject` | 非远程对象调用 Marshalling；远程对象写入位置标记 | 把远程对象当普通对象写入 |
| `WriteInt32` + `ReadInt32` | 写入/读取 4 字节对齐的 int32 | WriteInt32 后 ReadInt16 导致后续数据全部错位 |
| `SetDataCapacity` / `SetMaxCapacity` | 控制数据区大小 | 在已写入数据后强行缩小 capacity 可能截断数据 |
| `InjectOffsets` | 注入对象偏移数组 | 只在接收 IPC 数据时使用，随意注入会破坏读操作 |
| `ParseFrom` | 从外部数据初始化 Parcel | 调用后只能读不能写 |

## 约束规则

- **必须**保证 Marshalling 和 Unmarshalling 的读写类型和顺序完全一致。
- **不要**修改已有 Parcelable 的序列化格式——否则破坏版本兼容。
- **不要**在 ReadParcelable 返回 nullptr 时不检查就直接使用。
- **不要**随意更换 Allocator——DefaultAllocator 是 new/delete 的标准实现。
- **必须**在 Read 前检查 CheckOffsets() 返回值。

## 修改前检查

- 新增字段是否修改了序列化格式？会影响 IPC 兼容性。
- Marshalling/Unmarshalling 的读写顺序是否匹配？
- 新增的 Read 操作是否校验了数据边界？

## 代码和测试

| 锚点 | 路径 |
|---|---|
| Parcel 头文件 | `base/include/parcel.h` |
| FlatObj 头文件 | `base/include/flat_obj.h` |
| 单测 | `UtilsParcelTest` |
| 模糊测试 | `base/test/fuzztest/parcel_fuzzer/` |
