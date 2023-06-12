# 读写文件

## 概述
提供Rust侧对应c_utils中[读写文件](https://gitee.com/openharmony/commonlibrary_c_utils/blob/master/docs/zh-cn/c-utils-guide-file.md)的接口。其在Rust侧构成utils_rust包(Crate)中的file_ex模块，包括对文件内容的读写以及对指定字符串的查找功能。
## 涉及子模块

### utils_rust::file_ex::ffi
子模块`file_ex::ffi`借助CXX工具通过FFI实现与C++对应代码的互操作。其中各接口通过FFI调用C++侧对应接口，以实现读写文件相关功能。 

由于使用CXX对Rust侧以及C++侧的接口进行绑定，该模块中的接口命名风格与C++一致，其参数类型为兼容C++的Rust类型。

`using utils_rust::file_ex`

#### 全局函数

|                | 名称           |
| -------------- | -------------- |
| i32 | **RustCountStrInFile**(fileName: &String, subStr: &String, caseSensitive: bool)<br>查看指定文件中出现指定字符串的次数  |
| bool | **RustFileExists**(fileName: &String)<br>检查指定文件是否存在。  |
| bool | **RustLoadBufferFromFile**(filePath: &String, content:&mut Vec<c_char>)<br>从指定文件中读出数据，存入输入缓存区(`Vec<c_char>`)结构体中。  |
| bool | **RustLoadStringFromFd**(fd: i32, content: &mut String)<br>通过文件对应的文件描述符，从中读取全部字符串存入输入`String`对象中。  |
| bool | **RustLoadStringFromFile**(filePath: &String, content: &mut String)<br>从指定文件中读出全部字符串存入输入`String`对象中。  |
| bool | **RustSaveBufferToFile**(filePath: &String, content:&Vec<c_char>, truncated: bool)<br>向指定文件中写入缓存区(`Vec<c_char>`)对象中的数据。  |
| bool | **RustSaveStringToFd**(fd: i32, content: &String)<br>通过文件对应的文件描述符，向其写入字符串。  |
| bool | **RustSaveStringToFile**(filePath: &String, content: &String, truncated: bool)<br>将字符串写入指定文件中。  |
| bool | **RustStringExistsInFile**(fileName: &String, subStr: &String, caseSensitive: bool)<br>检查指定文件中是否包含指定字符串  |

## 使用示例

1. 测试用例编译运行方法

- 测试用例代码参见 base/test/unittest/rust/utils_rust_file_test.cpp

- 使用开发者自测试框架，使用方法参见：[开发自测试执行框架-测试用例执行](https://gitee.com/openharmony/testfwk_developer_test#%E6%B5%8B%E8%AF%95%E7%94%A8%E4%BE%8B%E6%89%A7%E8%A1%8C)

- 使用以下具体命令以运行`utils_rust::file_ex`对应测试用例

```bash
run -t UT -tp utils -ts utils_rust_file_test
```
