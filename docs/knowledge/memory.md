# 内存管理知识

本文只记录 RefBase / sptr / wptr 的边界约束。这是 c_utils 的引用计数智能指针系统。

## 核心模型

```
RefBase               — 基类，任何想被智能指针管理的类必须继承它
  └─ RefCounter        — 引用计数存储（atomicStrong_ + atomicWeak_ + atomicRefCount_）
       ├─ sptr<T>      — 强引用智能指针，持有 RefBase*，影响对象生命周期
       └─ wptr<T>      — 弱引用智能指针，持有 WeakRefCounter*，不影响对象生命周期
            └─ promote() → sptr<T>  — 弱→强提升（可能失败返回 nullptr）
```

关键生命周期钩子（均为空实现，子类可选覆盖）：
- `OnFirstStrongRef()` — 第一个 sptr 建立时
- `OnLastStrongRef()` — 最后一个 sptr 释放时
- `OnLastWeakRef()` — 最后一个 wptr 释放时

## 边界/分类

| 概念 | 用途 | 常见误用 |
|---|---|---|
| `sptr<T>::MakeSptr(args...)` | 创建对象并用 sptr 管理（推荐） | 用 new + 裸指针构造 sptr，容易造成引用计数错乱 |
| `sptr<T>(T *other)` | 用已有裸指针创建 sptr | 多个 sptr 各自用裸指针构造 → 引用计数分裂 → double-free |
| `wptr<T>::promote()` | 弱引用提升为强引用 | 不检查返回值是否为 nullptr |
| `ExtendObjectLifetime()` | 延长生命周期：最后一个强引用释放后，只要还有弱引用，对象就不析构 | 忘记同时还有弱引用，内存长期不释放 |
| `ForceSetRefPtr(T *)` | 强制设置内部指针（不增减计数） | 只应在 MakeSptr 等内部使用，外部调用导致计数错乱 |
| `RefBase` 的复制构造 | 分配到新的 RefCounter，计数归零 | 复制后两个对象各有独立的引用计数 |
| `RefBase` 的移动构造 | 接管原对象的 RefCounter | 原对象不再有效，访问会出问题 |

## 约束规则

- **必须**用 `sptr<T>::MakeSptr(args...)` 创建新的智能指针管理对象。
- **不要**用裸指针构造 sptr，除非你完全确定该指针从未被其他 sptr 管理。
- **不要**在外部调用 `ForceSetRefPtr`。
- **不要**创建 RefBase 对象和 sptr 之间的循环引用（sptr→对象A→sptr→对象B→sptr→对象A），用 wptr 打破。
- **必须**在 promote() 后检查返回值。
- **不要**在 OnLastStrongRef 中创建新的 sptr 指向同一个对象。

## 修改前检查

- 是否引入了循环引用？
- 是否有裸指针绕过 sptr 直接管理 RefBase 对象？
- 是否遗漏了 promote() 的空指针检查？
- 是否修改了引用计数的原子操作逻辑？

## 代码和测试

| 锚点 | 路径 |
|---|---|
| RefBase 头文件 | `base/include/refbase.h` |
| 实现 | `base/src/refbase.cpp` |
| 单测 | `UtilsRefbaseTest` |
| 模糊测试 | `base/test/fuzztest/refbase_fuzzer/` |
