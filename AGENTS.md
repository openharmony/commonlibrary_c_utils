# AGENTS.md — c_utils AI Agent 行为指引

## 1. 代码地图

本仓库实现 OpenHarmony 标准系统 C++ 工具库 `c_utils`。核心职责：为上层提供文件、路径、字符串、并发、安全容器、序列化、事件处理等基础能力。

最关键的边界：**`base/include/` 中所有 .h 文件是对外 API，不得随意修改签名和语义。**

| 目录 | 用途 | 风险 |
|---|---|---|
| `base/include/` | 所有对外 API 头文件 | 高 — 修改签名即破坏兼容性 |
| `base/src/` | C++ 实现代码 | 中 — 修改可能影响多模块 |
| `base/src/rust/` | Rust 实现（ashmem/directory/file） | 中 — 通过 CXX 桥接，改接口需同步 |
| `base/test/unittest/common/` | C++ 单元测试 | 低 |
| `base/test/unittest/rust/` | Rust 单元测试 | 低 |
| `base/test/benchmarktest/` | 性能基准测试 | 低 |
| `base/test/fuzztest/` | 模糊测试（Parcel/RefBase/Timer 等） | 低 |
| `docs/zh-cn/` | 中文 API 使用指南（面向开发者） | 低 |
| `docs/en/` | 英文 API 使用指南 | 低 |
| `bundle.json` | 组件依赖和元数据声明 | 中 |
| `base/BUILD.gn` | 构建入口 | 中 |

## 2. 知识路由

改动前，按任务中出现的关键词查下表，先读对应知识文件再动手。

| 关键词 | 风险提示 | 先读 |
|---|---|---|
| thread / 线程 / thread_ex / thread_pool / 线程池 / rwlock / 读写锁 / semaphore / 信号量 | 并发 bug 难以复现；禁止 sleep_for 同步；线程池生命周期陷阱 | `docs/knowledge/threading.md` |
| file_ex / file / 文件 / directory_ex / 目录 / directory / mapped_file / 文件映射 / unique_fd / fd / ashmem / 共享内存 | 路径编码；fd 所有权转移；跨平台差异；ashmem 通过 Rust CXX 桥接 | `docs/knowledge/file_io.md` |
| safe_map / safe_queue / safe_block_queue / 安全队列 / sorted_vector / flat_obj / singleton / 单例 | 线程安全边界不等于保证；容器选型错误；Singleton 析构顺序 | `docs/knowledge/data_containers.md` |
| parcel / 序列化 / 反序列化 / Marshalling / Unmarshalling | 读写顺序必须一致；格式变更破坏 IPC 兼容性 | `docs/knowledge/serialization.md` |
| refbase / RefPtr / sptr / wptr / weak_ptr / 引用计数 / 循环引用 / MakeSptr | 裸指针构造 sptr 导致计数分裂；循环引用泄漏；promote() 不检查 nullptr | `docs/knowledge/memory.md` |
| event_handler / event / 事件 / io_event / io_event_reactor / epoll / observer / 观察者 / timer / 定时器 | epoll 回调重入；NotifyObservers 中改 obs 集合；Timer Shutdown(false) 风险 | `docs/knowledge/event_handling.md` |


## 3. 约束边界

### 架构不变量

- `base/include/` 中的公共 API 是稳定接口，不是内部实现的投影。
- `utils`（动态库）依赖 `utilsbase`（静态库），不要反向依赖。
- Rust 模块通过 `rust_cxx` 桥接，改 Rust 侧接口需同步 C++ 侧。
- 线程安全注解宏（`thread_safety_analysis_macros.h`）是编译期检查，不要绕过。

### 禁止

- **不要**删除、重命名、修改 `base/include/` 中已有 API 的签名，除非任务明确提出。
- **不要**新增未在 `bundle.json` 中声明且无合理理由的第三方依赖。
- **不要**进行大规模重构、批量删除或重排。
- **不要**在并发测试中使用固定 `sleep_for` 做同步，用 `std::latch` / `std::barrier` / condition variable。
- **不要**新增 `utils_*_test.cpp` 后忘记在同目录 `BUILD.gn` 中注册 `ohos_unittest`。
- **不要**删除 Apache 2.0 许可证头。
- **不要**为通过测试而删除日志、事件、错误码或诊断信息。

### 先问

- 新增依赖。
- 修改公共 API 语义。
- 修改 `bundle.json` 中的组件声明。
- 改动影响 `bundle.json` 中已声明的 25 个对外头文件。
- 需要批量修改 5 个以上文件时。

## 4. 验证闭环

> 以下命令从 OpenHarmony 源码根目录执行，`rk3568` 替换为实际产品名。

### 构建

```bash
# 全量构建 c_utils 组件
./build.sh --product-name rk3568 --build-target c_utils

# 构建动态库
./build.sh --product-name rk3568 --build-target commonlibrary/c_utils/base:utils

# 构建静态库
./build.sh --product-name rk3568 --build-target commonlibrary/c_utils/base:utilsbase

# 构建 Rust 动态库
./build.sh --product-name rk3568 --build-target commonlibrary/c_utils/base:utils_rust
```

### 测试

```bash
# 构建全部单元测试
./build.sh --product-name rk3568 --build-target commonlibrary/c_utils/base/test:unittest

# 构建单个单元测试（示例）
./build.sh --product-name rk3568 --build-target commonlibrary/c_utils/base/test/unittest/common:UtilsThreadTest
```

### 任务分级验证

| 任务类型 | 最少验证 |
|---|---|
| 修改 `base/src/` 下单个 .cpp | 构建对应模块 + 运行对应单测 |
| 修改 `base/include/` 下头文件 | 构建整个组件 + 运行所有单测 |
| 修改 Rust 代码 (`base/src/rust/`) | 构建 `utils_rust` + 运行对应 Rust 测试 |
| 新增测试 | 构建测试目标 + 运行该测试 + 运行相邻测试 |
| 修改 BUILD.gn 或 bundle.json | 全量构建 + 确认编译通过 |

