# 读写文件

## 概述
使用c_utils提供的全局函数，实现对文件的相关操作。

## 涉及功能
### include/file_ex.h

提供c_utils中实现的文件读写功能。 

#### 描述

具体包括对文件内容的读写以及对指定字符串的查找功能。 

#### 函数

|                | 名称           |
| -------------- | -------------- |
| int | **CountStrInFile**(const std::string& fileName, const std::string& subStr, bool caseSensitive = true)<br>查看指定文件中出现指定字符串的次数  |
| bool | **FileExists**(const std::string& fileName)<br>检查指定文件是否存在。  |
| bool | **LoadBufferFromFile**(const std::string& filePath, std::vector< char >& content)<br>从指定文件中读出数据，存入输入缓存区(`std::vector`)对象中。  |
| bool | **LoadStringFromFd**(int fd, std::string& content)<br>通过文件对应的文件描述符，从中读取全部字符串存入输入`std::string`对象中。  |
| bool | **LoadStringFromFile**(const std::string& filePath, std::string& content)<br>从指定文件中读出全部字符串存入输入`std::string`对象中。  |
| bool | **SaveBufferToFile**(const std::string& filePath, const std::vector< char >& content, bool truncated = true)<br>向指定文件中写入缓存区(`std::vector`)对象中的数据。  |
| bool | **SaveStringToFd**(int fd, const std::string& content)<br>通过文件对应的文件描述符，向其写入字符串。  |
| bool | **SaveStringToFile**(const std::string& filePath, const std::string& content, bool truncated = true)<br>将字符串写入指定文件中。  |
| bool | **StringExistsInFile**(const std::string& fileName, const std::string& subStr, bool caseSensitive = true)<br>检查指定文件中是否包含指定字符串  |


## 使用示例

1. 测试用例编译运行方法

- 测试用例代码参见 base/test/unittest/common/utils_file_test.cpp

- 使用开发者自测试框架，使用方法参见：[开发自测试执行框架-测试用例执行](https://gitee.com/openharmony/testfwk_developer_test#%E6%B5%8B%E8%AF%95%E7%94%A8%E4%BE%8B%E6%89%A7%E8%A1%8C)

- 使用以下具体命令以运行`file_ex.h`对应测试用例

```bash
run -t UT -tp utils -ts UtilsFileTest
```
