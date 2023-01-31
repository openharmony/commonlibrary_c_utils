# 日期与时间

## 概述

### 简介
提供一些时间的相关操作的能力增强接口。包括时间转换，获取天数和获取时区等函数。

## 接口说明
|返回类型           |名称           |
| :------------ | :------------ |
|int64_t   |**SecToNanosec**(int64_t sec) <br>将秒数转换为纳秒数。   |
|int64_t   |**MillisecToNanosec**(int64_t millise)<br>将毫秒数转换为纳秒数。   |
|int64_t   |**MicrosecToNanosec**(int64_t microsec)<br>将微秒数转换为纳秒数。   |
|int64_t   |**NanosecToSec**(int64_t nanosec)<br>将纳秒数转换为秒数。   |
|int64_t   |**NanosecToMillisec**(int64_t nanosec)<br>将纳秒数转换为毫秒数。   |
|int64_t   |**NanosecToMicrosec**(int64_t nanosec)<br>将纳秒数转换为微秒数。   |
|int64_t   |**GetSecondsSince1970ToNow**()<br>获取从 1970 年到现在的秒数。   |
|int64_t   |**GetSecondsSince1970ToPointTime**(struct tm inputTm)<br>获取从 1970 到输入时间的秒数。   |
|int64_t   |**GetSecondsBetween**(struct tm inputTm1, struct tm inputTm2)<br>获取 inputTm1 和 inputTm2 之间的秒数。   |
|int64_t   |**GetDaysSince1970ToNow**()<br>获取从 1970 年到现在的天数。   |
|bool      |**GetLocalTimeZone**(int& timezone)<br>获取当前时区。   |
|bool      |**GetSystemCurrentTime**(struct tm* curTime) <br>获取当前时间。   |
|int64_t  |**GetTickCount**()<br>获取自系统启动以来的毫秒数。   |
|int64_t  |**GetMicroTickCount**()<br>获取自系统启动以来的微秒数。   |

## 使用示例
1. 测试用例编译运行方法

- 测试用例代码参见base/test/unittest/common/utils_datetime_test.cpp

- 使用开发者自测试框架，使用方法参见：[开发自测试执行框架-测试用例执行](https://gitee.com/openharmony/testfwk_developer_test#%E6%B5%8B%E8%AF%95%E7%94%A8%E4%BE%8B%E6%89%A7%E8%A1%8C)

- 使用以下具体命令以运行`datetime_ex.h`对应测试用例

```bash
run -t UT -tp utils -ts UtilsDateTimeTest
```