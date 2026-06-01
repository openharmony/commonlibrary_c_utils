# 文件与 I/O 知识

本文记录 file_ex、directory_ex、mapped_file、unique_fd、ashmem 五类的边界约束。字符串操作、日期时间等辅助函数不在此列。

## 核心模型

```
file_ex       — 文件读写、存在性检查、字符串搜索（C++ 实现）
directory_ex  — 目录遍历、创建/删除、路径处理（C++ 实现）
unique_fd     — RAII 管理 fd，默认 Deleter 调用 close()
mapped_file   — 内存映射文件
ashmem        — Android 匿名共享内存，通过 Rust CXX 桥接
```

Rust 模块（ashmem, directory, file）通过 `rust_cxx` 桥接到 C++，接口声明在对应 .h 文件的 `#ifdef UTILS_CXX_RUST` 块中。

## 边界/分类

| 概念 | 用途 | 常见误用 |
|---|---|---|
| `LoadStringFromFile` | 读文件内容到 string | 最大 32MB，超限行为未定义 |
| `SaveStringToFile` | 写 string 到文件 | truncated 参数决定是覆盖还是追加 |
| `LoadStringFromFd` | 从已有 fd 读 | fd 所有权不转移，调用者负责关闭 |
| `UniqueFd` | 自动管理 fd 生命周期 | 析构时自动 close；Release() 后调用者负责关闭 |
| `UniqueFd::Release()` | 放弃 fd 管理权 | 释放后必须由另一个 UniqueFd 接管或手动 close |
| `ForceCreateDirectory` | 递归创建目录 | 权限不足时静默失败 |
| `ForceRemoveDirectory` | 递归删除目录 | 不可逆操作；权限不足时静默失败 |
| `GetDirFiles` | 递归获取目录下所有文件 | 大目录可能 OOM |
| `PathToRealPath` | 相对路径转绝对路径 | 符号链接会被解析 |
| `ChangeModeFile` / `ChangeModeDirectory` | 修改文件/目录权限 | mode 直接传给 chmod()，不做校验 |
| ashmem 接口 | 共享内存创建/映射 | 通过 Rust CXX；改 Rust 侧接口需同步 C++ 侧 |

## 约束规则

- **不要**直接使用 raw fd 做资源管理，优先用 `UniqueFd`。
- **不要**在 `Release()` 后继续使用该 UniqueFd 对象。
- **不要**假设文件操作一定成功——多数接口返回 bool 但调用者常忽略。
- **必须**注意 `ForceRemoveDirectory` 的不可逆性。
- **只适合**小文件（<32MB）直接 LoadStringFromFile 全量读入。

## 修改前检查

- 是否新增了 fd 操作但没有配对 close？
- 是否修改了 Rust 侧接口但没有同步 C++ 声明？
- 是否影响了路径编码或跨平台兼容性？
- 是否引入了文件操作的权限问题？

## 代码和测试

| 锚点 | 路径 |
|---|---|
| file_ex 头文件 | `base/include/file_ex.h` |
| directory_ex 头文件 | `base/include/directory_ex.h` |
| unique_fd 头文件 | `base/include/unique_fd.h` |
| mapped_file 头文件 | `base/include/mapped_file.h` |
| ashmem 头文件 | `base/include/ashmem.h` |
| Rust 源码 | `base/src/rust/` |
| C++ 单测 | `UtilsFileTest` `UtilsDirectoryTest` `UtilsMappedFileTest` `UtilsUniqueFdTest` `UtilsAshmemTest` |
