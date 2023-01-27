# 字符串处理

## 概述
使用c_utils提供的全局函数，实现对字符串的相关处理。

## 涉及功能
### include/string_ex.h

提供c_utils中实现的字符串操作功能

#### 函数

|                | 名称           |
| -------------- | -------------- |
| std::string | **DexToHexString**(int value, bool upper = true)<br>将十进制数字转换为十六进制表示的字符串。  |
| std::string::size_type | **GetFirstSubStrBetween**(const std::string& str, const std::string& left, const std::string& right, std::string& sub)<br>获取字符串中，第一次出现的两个输入字符子串中间的字符子串。  |
| void | **GetSubStrBetween**(const std::string& str, const std::string& left, const std::string& right, std::vector< std::string >& sub)<br>获取字符串中，所有出现的两个输入字符子串中间的字符子串。  |
| bool | **IsAlphaStr**(const std::string& str)<br>判断字符串中所有字符是否全部为英文字母。  |
| bool | **IsAsciiString**(const std::string& str)<br>判断字符串是否全部由ASCII码字符组成。  |
| bool | **IsLowerStr**(const std::string& str)<br>判断字符串中所有字符是否全部为英文小写字母。  |
| bool | **IsNumericStr**(const std::string& str)<br>判断字符串中所有字符是否全部为数字。  |
| bool | **IsSameTextStr**(const std::string& first, const std::string& second)<br>判断两个输入字符串内容是否相同(大小写不敏感)。  |
| bool | **IsSubStr**(const std::string& str, const std::string& sub)<br>判断字符串中是否存在指定子串。  |
| bool | **IsUpperStr**(const std::string& str)<br>判断字符串中所有字符是否全部为英文大写字母。  |
| std::string | **LowerStr**(const std::string& str)<br>将字符串的所有字母替换为小写。  |
| std::string | **ReplaceStr**(const std::string& str, const std::string& src, const std::string& dst)<br>将字符串中的指定子串替换为指定的目标字符串。  |
| void | **SplitStr**(const std::string& str, const std::string& sep, std::vector< std::string >& strs, bool canEmpty = false, bool needTrim = true)<br>将字符串按照指定分隔字符串进行切分。  |
| std::string | **Str16ToStr8**(const std::u16string& str16)<br>将UTF-16编码的`std::u16string`字符串对象转换为UTF-8编码的`std::string`对象。  |
| std::u16string | **Str8ToStr16**(const std::string& str)<br>将UTF-8编码的`std::string`字符串对象转换为UTF-16编码的`std::u16string`对象。  |
| bool | **StrToInt**(const std::string& str, int& value)<br>将字符串表示的数值转换为`int`类型。  |
| template <class T \> <br>std::string | **ToString**(T iValue)<br>将各种`int`类型以及`float`,`double`类型数据转换为字符串表示。  |
| std::string | **TrimStr**(const std::string& str, const char cTrim = ' ')<br>将字符串中位于头尾的指定字符去除。  |
| std::string | **UpperStr**(const std::string& str)<br>将字符串的所有字母替换为大写。  |


## 使用示例

1. 测试用例编译运行方法

- 测试用例代码参见 base/test/unittest/common/utils_string_test.cpp

- 使用开发者自测试框架，使用方法参见：[开发自测试执行框架-测试用例执行](https://gitee.com/openharmony/testfwk_developer_test#%E6%B5%8B%E8%AF%95%E7%94%A8%E4%BE%8B%E6%89%A7%E8%A1%8C)

- 使用以下具体命令以运行`string_ex.h`对应测试用例

```bash
run -t UT -tp utils -ts UtilsStringTest
```