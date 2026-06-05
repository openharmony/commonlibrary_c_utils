# 安全容器知识

本文记录 SafeMap、SafeQueue、SafeStack、SafeBlockQueue、SortedVector、FlatObj、Singleton 的边界约束。

## 核心模型

```
SafeMap<K,V>         — std::map + std::mutex，每操作加锁
SafeQueue<T>         — std::deque + std::mutex，FIFO
SafeStack<T>         — std::deque + std::mutex，LIFO
SafeBlockQueue<T>    — 阻塞队列（生产者-消费者模型）
SortedVector<T>      — 排序 vector，插入时保持有序
Singleton<T>         — 饿汉单例（静态初始化，无锁）
DelayedSingleton<T>  — 懒汉单例（shared_ptr + double-check lock）
DelayedRefSingleton<T> — 懒汉单例（裸指针 + double-check lock）
```

## 边界/分类

| 类型 | 线程安全边界 | 常见误用 |
|---|---|---|
| SafeMap | 每个方法加锁，但连续两次调用之间不保证一致性 | Size() 后立即假设 map 没变 |
| SafeMap::ReadVal | 读取值——key 不存在时会插入默认值 | 以为 key 不存在时会返回空或报错 |
| SafeMap::EnsureInsert | 强制插入；如果 key 已存在，先删再插 | 非原子操作，但加锁保证安全 |
| SafeMap::Find | 返回 true/false + 输出 value | 忽略返回值直接使用 value |
| SafeQueue/SafeStack | 每个操作加锁，Push/Pop 成对 | Empty() 返回 false 后立即 Pop 仍可能失败 |
| SafeBlockQueue | 阻塞 Pop，支持超时 | 生产者停止后消费者无限等 |
| Singleton | 线程安全由静态初始化保证（C++11） | 析构顺序问题：两个 Singleton 互相依赖导致 use-after-free |
| DelayedSingleton | GetInstance() 用 double-check lock，线程安全 | DestroyInstance() 后旧的 shared_ptr 仍可能被外部持有 |
| DelayedRefSingleton | GetInstance() double-check，返回引用 | new 出来的对象永远不 delete，外部可能意外 delete |

## 约束规则

- **不要**在 SafeMap 两次独立调用之间假设状态不变。
- **不要**在多线程中使用裸 std::map/std::queue 替代安全容器。
- **不要**对空 SafeQueue 直接 Pop——先检查 Empty()，且注意检查后状态可能改变。
- **不要**在 SafeBlockQueue 中依赖消费者一定能等到数据——设置超时。
- **必须**注意 Singleton 之间的析构顺序依赖。
- **只适合**用 DelayedSingleton 当实例可能被销毁后重建；否则用 Singleton。

## 修改前检查

- 是否引入了新的不安全的裸容器操作？
- 是否在锁外依赖了锁内获取的状态？
- 新增容器类是否需要线程安全？不需要的话别加锁。

## 代码和测试

| 锚点 | 路径 |
|---|---|
| SafeMap 头文件 | `base/include/safe_map.h` |
| SafeQueue/SafeStack 头文件 | `base/include/safe_queue.h` |
| SafeBlockQueue 头文件 | `base/include/safe_block_queue.h` |
| SortedVector 头文件 | `base/include/sorted_vector.h` |
| FlatObj 头文件 | `base/include/flat_obj.h` |
| Singleton 头文件 | `base/include/singleton.h` |
| 单测 | `UtilsSafeMapTest` `UtilsSafeBlockQueueTest` `UtilsSafeBlockQueueTrackingTest` `UtilsSafeQueueTest` `UtilsSingletonTest` `UtilsSortedVectorTest` |
