# 文件与目录

## 概述

### 简介
提供一些文件及路径的相关操作的能力增强接口。包括删除文件，获取文件后缀名和更改文件权限等函数。

## 接口说明

|返回类型           |名称           |
| -------------- | -------------- |
|string  |**GetCurrentProcFullFileName**()<br>获取当前程序的完整的绝对路径。   |
|string  |**GetCurrentProcPath**()<br>获取当前程序的绝对路径。   |
|string  |**ExtractFilePath**(const std::string& fileFullName)<br> 通过输入的文件完整路径去获取对应文件所处目录路径。   |
|string  |**ExtractFileName**(const std::string& fileFullName)<br> 通过输入的文件完整路径去获取对应文件的名称。   |
|string  |**ExtractFileExt**(const std::string& fileName)<br> 通过输入的文件名去获取对应文件的后缀名。   |
|string  |**ExcludeTrailingPathDelimiter**(const std::string& path)<br>返回以去掉'/'结尾的对应路径。   |
|string  |**IncludeTrailingPathDelimiter**(const std::string& path)<br> 返回以'/'为结尾的对应路径。   |
|void    |**GetDirFiles**(const std::string& path, std::vector<std::string>& files)<br>获取当前路径及路径子目录下的所有文档。   |
|bool    |**IsEmptyFolder**(const std::string& path)<br>判断路径是否为空。   |
|bool    |**ForceCreateDirectory**(const std::string& path)<br> 强制创建带有子目录的目录。   |
|bool    |**ForceRemoveDirectory**(const std::string& path)<br>强制删除包含子目录和文档的目录。   |
|bool    |**RemoveFile**(const std::string& fileName)<br>删除文件。   |
|uint64_t    |**GetFolderSize**(const std::string& path)<br> 获取文件夹大小（字节）。   |
|bool   |**ChangeModeFile**(const std::string& fileName, const mode_t& mode)<br> 更改输入文档的权限。   |
|bool   |**ChangeModeDirectory**(const std::string& path, const mode_t& mode)<br>更改输入目录的权限，包括子目录。   |
|bool   |**PathToRealPath**(const std::string& path, std::string& realPath)<br> 从路径获取真实路径。   |

## 使用示例
1. 测试用例编译运行方法

- 测试用例代码参见base/test/unittest/common/utils_directory_test.cpp

- 使用开发者自测试框架，使用方法参见：[开发自测试执行框架-测试用例执行](https://gitee.com/openharmony/testfwk_developer_test#%E6%B5%8B%E8%AF%95%E7%94%A8%E4%BE%8B%E6%89%A7%E8%A1%8C)

- 使用以下具体命令以运行`directory_ex.h`对应测试用例

```bash
run -t UT -tp utils -ts UtilsDirectoryTest
```