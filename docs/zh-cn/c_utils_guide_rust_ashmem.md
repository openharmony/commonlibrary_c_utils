# Rust侧-匿名共享内存(Ashmem, Anonymous Shared Memory)

## 概述
提供Rust侧对应c_utils中[使用匿名共享内存(Ashmem, Anonymous Shared Memory)](https://gitee.com/openharmony/commonlibrary_c_utils/blob/master/docs/zh-cn/c-utils-guide-ashmem.md)的相关接口。
其在Rust侧构成utils_rust包(Crate)中的ashmem模块，包括Rust侧对匿名共享内存进行操作的各接口方法。

## 涉及子模块
### utils_rust::ashmem

模块`ashmem`中提供Rust风格的接口以对匿名共享内存进行操作。当前各Rust风格接口底层实际调用子模块`ashmem::ffi`中的方法。

`using utils_rust::ashmem`

`struct Ashmem`

#### Public Functions

| 返回类型       | 名称           |
| -------------- | -------------- |
| Option< Ashmem > | **create_ashmem_instance**(name: &str, size: i32)<br> 创建Rust侧Ashmem结构体对象。  |
| void | **close_ashmem**(self: &Ashmem)<br>通过内部维护的文件描述符关闭当前ashmem。  |
| int | **get_ashmem_fd**(self: &Ashmem)<br>获取内核中对应ashmem的文件描述符。  |
| int32_t | **get_ashmem_size**(self: &Ashmem)<br>获取内核中ashmem区域的大小。  |
| int | **get_protection**(self: &Ashmem)<br>获取内核中的ashmem区域的保护权限值。  |
| bool | **SetProtection**(self: &Ashmem, protType: i32)<br>设置内核中的ashmem区域的保护权限。  |
| bool | **map_ashmem**(self: &Ashmem, mapType: i32)<br>将内核中的ashmem内存区域映射至用户空间。  |
| void | **unmap_ashmem**(self: &Ashmem)<br>解除ashmem映射。  |
| bool | **map_read_write_ashmem**(self: &Ashmem)<br>以读/写模式映射ashmem内存区域。  |
| bool | **map_read_only_ashmem**(self: &Ashmem)<br>以只读模式映射ashmem内存区域。  |
| *const c_char | **read_from_ashmem**(&self, size: i32, offset: i32)<br>从ashmem内存区域`offset`处读出数据。  |
| bool | **write_to_ashmem**(&self, data: *const c_char, size: i32, offset: i32)<br>在ashmem内存区域`offset`处写入数据。  |

### utils_rust::ashmem::ffi

子模块`ashmem::ffi`借助CXX工具通过FFI实现与C++对应代码的互操作。其中各接口通过FFI调用C++侧对应接口，以实现操作匿名共享内存的功能。

由于使用CXX对Rust侧以及C++侧的接口进行绑定，该模块中的接口命名风格与C++一致，其参数类型为兼容C++的Rust类型。

`using utils_rust::ashmem::ffi`

#### Public Functions

| 返回类型       | 名称           |
| -------------- | -------------- |
| SharedPtr< Ashmem > | **CreateAshmemStd**(name: *const c_char, size: i32)<br>使用指定名称及大小创建Ashmem对象。  |
| void | **CloseAshmem**(self: &Ashmem)<br>通过内部维护的文件描述符关闭当前ashmem。  |
| int | **GetAshmemFd**(self: &Ashmem)<br>获取内核中对应ashmem的文件描述符。  |
| int32_t | **GetAshmemSize**(self: &Ashmem)<br>获取内核中ashmem区域的大小。  |
| int | **GetProtection**(self: &Ashmem)<br>获取内核中的ashmem区域的保护权限值。  |
| bool | **MapAshmem**(self: &Ashmem, mapType: i32)<br>将内核中的ashmem内存区域映射至用户空间。  |
| bool | **MapReadAndWriteAshmem**(self: &Ashmem)<br>以读/写模式映射ashmem内存区域。  |
| bool | **MapReadOnlyAshmem**(self: &Ashmem)<br>以只读模式映射ashmem内存区域。  |
| *const c_void | **ReadFromAshmem**(self: &Ashmem, size: i32, offset: i32)<br>从ashmem内存区域`offset`处读出数据。  |
| bool | **SetProtection**(self: &Ashmem, protType: i32)<br>设置内核中的ashmem区域的保护权限。  |
| void | **UnmapAshmem**(self: &Ashmem)<br>解除ashmem映射。  |
| bool | **WriteToAshmem**(self: &Ashmem, data: *const c_void, size: i32, offset: i32)<br>在ashmem内存区域`offset`处写入数据。  |

## 使用示例

1. 使用方法(伪代码)

```rust
    // ffi接口
    let c_name = CString::new(MEMORY_NAME).expect("CString::new Failed!");
    let ashmem = unsafe { ashmem::ffi::CreateAshmemStd(c_name.as_ptr(), MEMORY_SIZE) };
    assert!(!ashmem.is_null());
    assert_eq!(ashmem.GetAshmemSize(), MEMORY_SIZE);

    assert!(ashmem.MapAshmem(ashmem::PROT_READ | ashmem::PROT_WRITE));

    // 当使用结束时不要忘记解映射和关闭ashmem
    ashmem.UnmapAshmem();
    ashmem.CloseAshmem();

    // rust风格接口
    let ashmem = unsafe { ashmem::create_ashmem_instance(MEMORY_NAME, MEMORY_SIZE) };
    assert!(ashmem.is_some());

    let ashmem = ashmem.unwrap();
    assert_eq!(ashmem.get_ashmem_size(), MEMORY_SIZE);

    assert!(ashmem.map_ashmem(ashmem::PROT_READ | ashmem::PROT_WRITE));

    // 当使用结束时不要忘记解映射和关闭ashmem
    ashmem.unmap_ashmem();
    ashmem.close_ashmem();
```

2. 测试用例编译运行方法

- 测试用例代码参见 base/test/unittest/rust/utils_rust_ashmem_test.rs

- 使用开发者自测试框架，使用方法参见：[开发自测试执行框架-测试用例执行](https://gitee.com/openharmony/testfwk_developer_test#%E6%B5%8B%E8%AF%95%E7%94%A8%E4%BE%8B%E6%89%A7%E8%A1%8C)

- 使用以下具体命令以运行`utils_rust::ashmem`对应测试用例

```bash
run -t UT -tp utils -ts utils_rust_ashmem_test
```

## 常见问题

请参考[使用匿名共享内存(Ashmem, Anonymous Shared Memory)](https://gitee.com/openharmony/commonlibrary_c_utils/blob/master/docs/zh-cn/c-utils-guide-ashmem.md)
