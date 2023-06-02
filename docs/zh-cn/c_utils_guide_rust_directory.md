# Rust-文件与目录

## 概述

提供Rust侧对应c_utils中[文件与目录](https://gitee.com/openharmony/commonlibrary_c_utils/blob/master/docs/zh-cn/c-utils-guide-directory.md)的接口。其在Rust侧构成utils_rust包(Crate)中的directory_ex模块，包括删除文件，获取文件后缀名和更改文件权限等函数。

## 涉及子模块
### utils_rust::directory_ex::ffi

子模块`directory_ex::ffi`借助CXX工具通过FFI实现与C++对应代码的互操作。其中各接口通过FFI调用C++侧对应接口，以实现文件与目录相关功能。

由于使用CXX对Rust侧以及C++侧的接口进行绑定，该模块中的接口命名风格与C++一致，其参数类型为兼容C++的Rust类型。

`using utils_rust::directory_ex`

#### 全局函数

|返回类型           |名称           |
| -------------- | -------------- |
|String  |**RustGetCurrentProcFullFileName**()<br>获取当前程序的完整的绝对路径。   |
|String  |**RustGetCurrentProcPath**()<br>获取当前程序的绝对路径。   |
|String  |**RustExtractFilePath**(fileFullName: &String)<br> 通过输入的文件完整路径去获取对应文件所处目录路径。   |
|String  |**RustExtractFileName**(fileFullName: &String)<br> 通过输入的文件完整路径去获取对应文件的名称。   |
|String  |**RustExtractFileExt**(fileName: &String)<br> 通过输入的文件名去获取对应文件的后缀名。   |
|String  |**RustExcludeTrailingPathDelimiter**(path: &String)<br>返回以去掉'/'结尾的对应路径。   |
|String  |**RustIncludeTrailingPathDelimiter**(path: &String)<br> 返回以'/'为结尾的对应路径。   |
|()      |**RustGetDirFiles**(path: &String, files: &mut Vec<String>)<br>获取当前路径及路径子目录下的所有文档。 |
|bool    |**RustPathToRealPath**(path: &String, realPath: &mut String)<br> 从路径获取真实路径。   |
|bool    |**IsEmptyFolder**(path: &CxxString)<br>判断路径是否为空。   |
|bool    |**ForceCreateDirectory**(path: &CxxString)<br> 强制创建带有子目录的目录。   |
|bool    |**ForceRemoveDirectory**(path: &CxxString)<br>强制删除包含子目录和文档的目录。   |
|bool    |**RemoveFile**(fileName: &CxxString)<br>删除文件。   |
|u64     |**GetFolderSize**(path: &CxxString)<br> 获取文件夹大小（字节）。   |
|bool    |**ChangeModeFile**(fileName: &CxxString, mode: &u32)<br> 更改输入文档的权限。   |
|bool    |**ChangeModeDirectory**(path: &CxxString, mode: &u32)<br>更改输入目录的权限，包括子目录。   |


## 使用示例
1. 测试用例编译运行方法

- 测试用例代码参见 base/test/unittest/rust/utils_rust_directory_test.rs

- 使用开发者自测试框架，使用方法参见：[开发自测试执行框架-测试用例执行](https://gitee.com/openharmony/testfwk_developer_test#%E6%B5%8B%E8%AF%95%E7%94%A8%E4%BE%8B%E6%89%A7%E8%A1%8C)

- 使用以下具体命令以运行`utils_rust::directory_ex`对应测试用例

```bash
run -t UT -tp utils -ts utils_rust_directory_test
```