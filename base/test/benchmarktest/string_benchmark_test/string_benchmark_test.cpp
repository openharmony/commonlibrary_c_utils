/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <benchmark/benchmark.h>
#include "string_ex.h"
#include <iostream>
#include "benchmark_log.h"
#include "benchmark_assert.h"
using namespace std;

namespace OHOS {
namespace {

static constexpr int32_t COMPARE_STRING_RESULT = 0;
static constexpr uint32_t STRSPLIT04_STRING_SIZE = 0;
static constexpr int STRTOINT01_INT_VALUE = 12345;
static constexpr int STRTOINT02_INT_VALUE = -12345;
static constexpr int GETSUBSTR01_POS_VALUE1 = 17;
static constexpr int GETSUBSTR01_POS_VALUE2 = 27;
static constexpr int GETSUBSTR04_STRING_SIZE = 0;

#define STRSPLIT01_CHAR_ARRAY_SIZE 3
#define STRSPLIT02_CHAR_ARRAY_SIZE 2
#define STRSPLIT03_CHAR_ARRAY_SIZE 3
#define GETSUBSTR03_CHAR_ARRAY_SIZE 2
#define GETSUBSTR04_CHAR_ARRAY_SIZE 2

class BenchmarkStringTest : public benchmark::Fixture {
public:
    void SetUp(const ::benchmark::State& state) override
    {
    }

    void TearDown(const ::benchmark::State& state) override
    {
    }

    BenchmarkStringTest()
    {
        Iterations(iterations);
        Repetitions(repetitions);
        ReportAggregatesOnly();
    }

    ~BenchmarkStringTest() override = default;

protected:
    const int32_t repetitions = 3;
    const int32_t iterations = 1000;
};

/*
* Feature: string_ex
* Function: UpperStr
* SubFunction: NA
* FunctionPoints:
* EnvConditions: NA
* CaseDescription: test for convert all letters of str  to uppercase
*/
BENCHMARK_F(BenchmarkStringTest, test_strupper_01)(benchmark::State& state)
{
    BENCHMARK_LOGD("StringTest test_strupper_01 start.");
    while (state.KeepRunning()) {
        string strBase = "strbase";
        string strTemp = "STRBASE";
        string result = UpperStr(strBase);
        AssertEqual(result, strTemp, "result did not equal strTemp as expected.", state);

        strBase = "StrBase";
        result = UpperStr(strBase);
        AssertEqual(result, strTemp, "result did not equal strTemp as expected.", state);
    }
    BENCHMARK_LOGD("StringTest test_strupper_01 end.");
}

BENCHMARK_F(BenchmarkStringTest, test_strupper_02)(benchmark::State& state)
{
    BENCHMARK_LOGD("StringTest test_strupper_02 start.");
    while (state.KeepRunning()) {
        string strBase = "";
        string strTemp = "";
        string result = UpperStr(strBase);
        AssertEqual(result, strTemp, "result did not equal strTemp as expected.", state);
    }
    BENCHMARK_LOGD("StringTest test_strupper_02 end.");
}

/*
* Feature: string_ex
* Function: LowerStr
* SubFunction: NA
* FunctionPoints:
* EnvConditions: NA
* CaseDescription: test for convert all letters of str  to lowercase
*/
BENCHMARK_F(BenchmarkStringTest, test_strlower_01)(benchmark::State& state)
{
    BENCHMARK_LOGD("StringTest test_strlower_01 start.");
    while (state.KeepRunning()) {
        string strBase = "STRbase";
        string strTemp = "strbase";
        string result = LowerStr(strBase);
        AssertEqual(result, strTemp, "result did not equal strTemp as expected.", state);

        strBase = "StrBase";
        result = LowerStr(strBase);
        AssertEqual(result, strTemp, "result did not equal strTemp as expected.", state);
    }
    BENCHMARK_LOGD("StringTest test_strlower_01 end.");
}

BENCHMARK_F(BenchmarkStringTest, test_strlower_02)(benchmark::State& state)
{
    BENCHMARK_LOGD("StringTest test_strlower_02 start.");
    while (state.KeepRunning()) {
        string strBase = "";
        string strTemp = "";
        string result = LowerStr(strBase);
        AssertEqual(result, strTemp, "result did not equal strTemp as expected.", state);
    }
    BENCHMARK_LOGD("StringTest test_strlower_02 end.");
}

/*
* Feature: string_ex
* Function: ReplaceStr
* SubFunction: NA
* FunctionPoints:
* EnvConditions: NA
* CaseDescription: test for replace src with dst int strBase
*/
BENCHMARK_F(BenchmarkStringTest, test_strreplace_01)(benchmark::State& state)
{
    BENCHMARK_LOGD("StringTest test_strreplace_01 start.");
    while (state.KeepRunning()) {
        string strBase = "test for replace";
        string src = "for";
        string dst = "with";
        string strTemp = "test with replace";
        string result = ReplaceStr(strBase, src, dst);
        AssertEqual(result, strTemp, "result did not equal strTemp as expected.", state);

        src = "test for replace";
        dst = "test";
        strTemp = "test";
        result = ReplaceStr(strBase, src, dst);
        AssertEqual(result, strTemp, "result did not equal strTemp as expected.", state);

        src = "";
        dst = "test";
        result = ReplaceStr(strBase, src, dst);
        AssertEqual(result, strBase, "result did not equal strBase as expected.", state);

        src = "for";
        dst = "";
        strTemp = "test  replace";
        result = ReplaceStr(strBase, src, dst);
        AssertEqual(result, strTemp, "result did not equal strTemp as expected.", state);
    }
    BENCHMARK_LOGD("StringTest test_strreplace_01 end.");
}

/*
* Feature: string_ex
* Function: TrimStr
* SubFunction: NA
* FunctionPoints:
* EnvConditions: NA
* CaseDescription: test for trim str front and end
*/
BENCHMARK_F(BenchmarkStringTest, test_strtrim_01)(benchmark::State& state)
{
    BENCHMARK_LOGD("StringTest test_strtrim_01 start.");
    while (state.KeepRunning()) {
        string strBase = "              test for trim ";
        string strTemp = "test for trim";
        string result = TrimStr(strBase);
        AssertEqual(result, strTemp, "result did not equal strTemp as expected.", state);
    }
    BENCHMARK_LOGD("StringTest test_strtrim_01 end.");
}

BENCHMARK_F(BenchmarkStringTest, test_strtrim_02)(benchmark::State& state)
{
    BENCHMARK_LOGD("StringTest test_strtrim_02 start.");
    while (state.KeepRunning()) {
        string strBase = "test";
        string strTemp = "es";
        string result = TrimStr(strBase, 't');
        AssertEqual(result, strTemp, "result did not equal strTemp as expected.", state);
    }
    BENCHMARK_LOGD("StringTest test_strtrim_02 end.");
}

/*
* Feature: string_ex
* Function: SplitStr
* SubFunction: NA
* FunctionPoints:
* EnvConditions: NA
* CaseDescription: test for split str by strSep
*/
BENCHMARK_F(BenchmarkStringTest, test_strsplit_01)(benchmark::State& state)
{
    BENCHMARK_LOGD("StringTest test_strsplit_01 start.");
    while (state.KeepRunning()) {
        string strBase = "test for split";
        string strSep = " ";
        string splitResult[STRSPLIT01_CHAR_ARRAY_SIZE] = { "test", "for", "split" };
        vector<string> strsRet;
        SplitStr(strBase, strSep, strsRet);

        for (int i = 0; i < STRSPLIT01_CHAR_ARRAY_SIZE; i++) {
            AssertEqual(splitResult[i], strsRet[i], "splitResult[i] did not equal strsRet[i] as expected.", state);
        }
    }
    BENCHMARK_LOGD("StringTest test_strsplit_01 end.");
}

BENCHMARK_F(BenchmarkStringTest, test_strsplit_02)(benchmark::State& state)
{
    BENCHMARK_LOGD("StringTest test_strsplit_02 start.");
    while (state.KeepRunning()) {
        string strBase = "test for split";
        string strSep = "for";
        string splitResult[STRSPLIT02_CHAR_ARRAY_SIZE] = { "test", "split" };
        vector<string> strsRet;
        SplitStr(strBase, strSep, strsRet);

        for (int i = 0; i < STRSPLIT02_CHAR_ARRAY_SIZE; i++) {
            AssertEqual(splitResult[i], strsRet[i], "splitResult[i] did not equal strsRet[i] as expected.", state);
        }

        splitResult[0] = "test ";
        splitResult[1] = " split";
        SplitStr(strBase, strSep, strsRet, false, false);
        for (int i = 0; i < STRSPLIT02_CHAR_ARRAY_SIZE; i++) {
            AssertEqual(splitResult[i], strsRet[i], "splitResult[i] did not equal strsRet[i] as expected.", state);
        }
    }
    BENCHMARK_LOGD("StringTest test_strsplit_02 end.");
}

BENCHMARK_F(BenchmarkStringTest, test_strsplit_03)(benchmark::State& state)
{
    BENCHMARK_LOGD("StringTest test_strsplit_03 start.");
    while (state.KeepRunning()) {
        string strBase = "test for for split";
        string strSep = "for";
        string splitResult[STRSPLIT03_CHAR_ARRAY_SIZE] = { "test", "", "split" };
        vector<string> strsRet;
        SplitStr(strBase, strSep, strsRet, true);
        for (int i = 0; i < (int)strsRet.size(); i++) {
            AssertEqual(splitResult[i], strsRet[i], "splitResult[i] did not equal strsRet[i] as expected.", state);
        }
    }
    BENCHMARK_LOGD("StringTest test_strsplit_03 end.");
}

/*
* Feature: string_ex
* Function: SplitStr
* SubFunction: NA
* FunctionPoints:
* EnvConditions: NA
* CaseDescription: test splitting a null string with a null seperator
*/
BENCHMARK_F(BenchmarkStringTest, test_strsplit_04)(benchmark::State& state)
{
    BENCHMARK_LOGD("StringTest test_strsplit_04 start.");
    while (state.KeepRunning()) {
        string strBase = "";
        string strSep = "";
        vector<string> strsRet1;
        SplitStr(strBase, strSep, strsRet1);
        AssertEqual(strsRet1.size(), STRSPLIT04_STRING_SIZE, "strsRet1.size() did not equal 0 as expected.", state);

        vector<string> strsRet2;
        SplitStr(strBase, strSep, strsRet2, true);
        AssertEqual(strsRet2[0], "", "strsRet2[0] did not equal \"\" as expected.", state);
    }
    BENCHMARK_LOGD("StringTest test_strsplit_04 end.");
}

/*
* Feature: string_ex
* Function: IsNumericStr
* SubFunction: NA
* FunctionPoints:
* EnvConditions: NA
* CaseDescription: test for judge all characters of the string are numbers
*/
BENCHMARK_F(BenchmarkStringTest, test_strisnumeric_01)(benchmark::State& state)
{
    BENCHMARK_LOGD("StringTest test_strisnumeric_01 start.");
    while (state.KeepRunning()) {
        string strBase = "1234556";
        bool result = IsNumericStr(strBase);
        AssertEqual(result, true, "result did not equal true as expected.", state);

        strBase = "1234,a";
        result = IsNumericStr(strBase);
        AssertEqual(result, false, "result did not equal false as expected.", state);

        strBase = "";
        result = IsNumericStr(strBase);
        AssertEqual(result, false, "result did not equal false as expected.", state);
    }
    BENCHMARK_LOGD("StringTest test_strisnumeric_01 end.");
}

/*
* Feature: string_ex
* Function: IsAlphaStr
* SubFunction: NA
* FunctionPoints:
* EnvConditions: NA
* CaseDescription: test for judge all characters of the string are alphabet
*/
BENCHMARK_F(BenchmarkStringTest, test_strisalpha_01)(benchmark::State& state)
{
    BENCHMARK_LOGD("StringTest test_strisalpha_01 start.");
    while (state.KeepRunning()) {
        string strBase = "1234556";
        bool result = IsAlphaStr(strBase);
        AssertEqual(result, false, "result did not equal false as expected.", state);

        strBase = "Acedafe";
        result = IsAlphaStr(strBase);
        AssertEqual(result, true, "result did not equal true as expected.", state);

        strBase = "Acedafe  ";
        result = IsAlphaStr(strBase);
        AssertEqual(result, false, "result did not equal false as expected.", state);

        strBase = "Acedafe3";
        result = IsAlphaStr(strBase);
        AssertEqual(result, false, "result did not equal false as expected.", state);

        strBase = "";
        result = IsAlphaStr(strBase);
        AssertEqual(result, false, "result did not equal false as expected.", state);
    }
    BENCHMARK_LOGD("StringTest test_strisalpha_01 end.");
}

/*
* Feature: string_ex
* Function: IsUpperStr
* SubFunction: NA
* FunctionPoints:
* EnvConditions: NA
* CaseDescription: test for judge all characters of the string are uppercase
*/
BENCHMARK_F(BenchmarkStringTest, test_IsUpperStr_01)(benchmark::State& state)
{
    BENCHMARK_LOGD("StringTest test_IsUpperStr_01 start.");
    while (state.KeepRunning()) {
        string strBase = "ABSEFAD";
        bool result = IsUpperStr(strBase);
        AssertEqual(result, true, "result did not equal true as expected.", state);

        strBase = "Afaefadf";
        result = IsUpperStr(strBase);
        AssertEqual(result, false, "result did not equal false as expected.", state);

        strBase = "12e13eaefd     ";
        result = IsUpperStr(strBase);
        AssertEqual(result, false, "result did not equal false as expected.", state);

        strBase = "";
        result = IsUpperStr(strBase);
        AssertEqual(result, false, "result did not equal false as expected.", state);
    }
    BENCHMARK_LOGD("StringTest test_IsUpperStr_01 end.");
}

/*
* Feature: string_ex
* Function: IsLowerStr
* SubFunction: NA
* FunctionPoints:
* EnvConditions: NA
* CaseDescription: test for judge all characters of the string are lowercase
*/
BENCHMARK_F(BenchmarkStringTest, test_IsLowerStr_01)(benchmark::State& state)
{
    BENCHMARK_LOGD("StringTest test_IsLowerStr_01 start.");
    while (state.KeepRunning()) {
        string strBase = "testlower";
        bool result = IsLowerStr(strBase);
        AssertEqual(result, true, "result did not equal true as expected.", state);

        strBase = "AAFDeadfkl";
        result = IsLowerStr(strBase);
        AssertEqual(result, false, "result did not equal false as expected.", state);

        strBase = "12e";
        result = IsLowerStr(strBase);
        AssertEqual(result, false, "result did not equal false as expected.", state);

        strBase = "";
        result = IsLowerStr(strBase);
        AssertEqual(result, false, "result did not equal false as expected.", state);
    }
    BENCHMARK_LOGD("StringTest test_IsLowerStr_01 end.");
}

/*
* Feature: string_ex
* Function: IsSubStr
* SubFunction: NA
* FunctionPoints:
* EnvConditions: NA
* CaseDescription: test for judge the sub_str in base_str
*/
BENCHMARK_F(BenchmarkStringTest, test_IsSubStr_01)(benchmark::State& state)
{
    BENCHMARK_LOGD("StringTest test_IsSubStr_01 start.");
    while (state.KeepRunning()) {
        string strBase = "test for issubstr";
        string strSub = "for";
        bool result = IsSubStr(strBase, strSub);
        AssertEqual(result, true, "result did not equal true as expected.", state);

        strBase = "";
        strSub = "";
        result = IsSubStr(strBase, strSub);
        AssertEqual(result, false, "result did not equal false as expected.", state);

        strSub = "fori";
        result = IsSubStr(strBase, strSub);
        AssertEqual(result, false, "result did not equal false as expected.", state);
    }
    BENCHMARK_LOGD("StringTest test_IsSubStr_01 end.");
}

/*
* Feature: string_ex
* Function: IsSameTextStr
* SubFunction: NA
* FunctionPoints:
* EnvConditions: NA
* CaseDescription: test for judge the strFirst's letter is same with strSecond
*/
BENCHMARK_F(BenchmarkStringTest, test_IsSameTextStr_01)(benchmark::State& state)
{
    BENCHMARK_LOGD("StringTest test_IsSameTextStr_01 start.");
    while (state.KeepRunning()) {
        string strFirst = "Test For StrSameText";
        string strSecond = "test for strsametext";
        bool result = IsSameTextStr(strFirst, strSecond);
        AssertEqual(result, true, "result did not equal true as expected.", state);

        strSecond = "test for strsametex";
        result = IsSameTextStr(strFirst, strSecond);
        AssertEqual(result, false, "result did not equal false as expected.", state);
    }
    BENCHMARK_LOGD("StringTest test_IsSameTextStr_01 end.");
}

/*
* Feature: string_ex
* Function: ToString
* SubFunction: NA
* FunctionPoints:
* EnvConditions: NA
* CaseDescription: test for convert int to str
*/
BENCHMARK_F(BenchmarkStringTest, test_ToString_01)(benchmark::State& state)
{
    BENCHMARK_LOGD("StringTest test_ToString_01 start.");
    while (state.KeepRunning()) {
        int ivalue = 12345;
        string strValue = "12345";
        string result = ToString(ivalue);
        AssertEqual(result, strValue, "result did not equal strValue as expected.", state);

        ivalue = -15;
        result = ToString(ivalue);
        AssertEqual(result, "-15", "result did not equal \"-15\" as expected.", state);
    }
    BENCHMARK_LOGD("StringTest test_ToString_01 end.");
}

/*
* Feature: string_ex
* Function: StrToInt
* SubFunction: NA
* FunctionPoints:
* EnvConditions: NA
* CaseDescription: test for convert str to int
*/
BENCHMARK_F(BenchmarkStringTest, test_StrToInt_01)(benchmark::State& state)
{
    BENCHMARK_LOGD("StringTest test_StrToInt_01 start.");
    while (state.KeepRunning()) {
        string strValue = "12345";
        int iValue = 0;
        bool result = StrToInt(strValue, iValue);
        AssertEqual(result, true, "result did not equal true as expected.", state);
        AssertEqual(iValue, STRTOINT01_INT_VALUE, "iValue did not equal 12345 as expected.", state);

        strValue = "123r54";
        result = StrToInt(strValue, iValue);
        AssertEqual(result, false, "result did not equal false as expected.", state);
    }
    BENCHMARK_LOGD("StringTest test_StrToInt_01 end.");
}

BENCHMARK_F(BenchmarkStringTest, test_StrToInt_02)(benchmark::State& state)
{
    BENCHMARK_LOGD("StringTest test_StrToInt_02 start.");
    while (state.KeepRunning()) {
        string strValue = "-12345";
        int iValue = 0;
        bool result = StrToInt(strValue, iValue);
        AssertEqual(result, true, "result did not equal true as expected.", state);
        AssertEqual(iValue, STRTOINT02_INT_VALUE, "iValue did not equal -12345 as expected.", state);

        strValue = "123=     54";
        result = StrToInt(strValue, iValue);
        AssertEqual(result, false, "result did not equal false as expected.", state);

        string strvalue2;
        result = StrToInt(strvalue2, iValue);
        AssertEqual(result, false, "result did not equal false as expected.", state);
    }
    BENCHMARK_LOGD("StringTest test_StrToInt_02 end.");
}

BENCHMARK_F(BenchmarkStringTest, test_StrToInt_03)(benchmark::State& state)
{
    BENCHMARK_LOGD("StringTest test_StrToInt_03 start.");
    while (state.KeepRunning()) {
        string strValue = "2147483648";
        int ivalue = 0;
        bool result = StrToInt(strValue, ivalue);
        AssertEqual(result, false, "result did not equal false as expected.", state);
    }
    BENCHMARK_LOGD("StringTest test_StrToInt_03 end.");
}

BENCHMARK_F(BenchmarkStringTest, test_StrToInt_04)(benchmark::State& state)
{
    BENCHMARK_LOGD("StringTest test_StrToInt_04 start.");
    while (state.KeepRunning()) {
        string strValue = "             ";
        int iValue = 0;
        bool result = StrToInt(strValue, iValue);
        AssertEqual(result, false, "result did not equal false as expected.", state);
    }
    BENCHMARK_LOGD("StringTest test_StrToInt_04 end.");
}

BENCHMARK_F(BenchmarkStringTest, test_strcovertfailed_01)(benchmark::State& state)
{
    BENCHMARK_LOGD("StringTest test_strcovertfailed_01 start.");
    while (state.KeepRunning()) {
        char test[] = {192, 157, 47, 106, 97, 18, 97, 47, 115, 1, 2};
        string strValue(test);

        bool ret = IsAsciiString(strValue);
        AssertEqual(ret, false, "ret did not equal false as expected.", state);

        strValue = "1234";
        ret = IsAsciiString(strValue);
        AssertEqual(ret, true, "ret did not equal true as expected.", state);

        strValue = "abcde";
        ret = IsAsciiString(strValue);
        AssertEqual(ret, true, "ret did not equal true as expected.", state);
    }
    BENCHMARK_LOGD("StringTest test_strcovertfailed_01 end.");
}


BENCHMARK_F(BenchmarkStringTest, test_strcovert_01)(benchmark::State& state)
{
    BENCHMARK_LOGD("StringTest test_strcovert_01 start.");
    while (state.KeepRunning()) {
        string strValue = "hello world!";
        u16string str16 = Str8ToStr16(strValue);
        AssertEqual(COMPARE_STRING_RESULT, strValue.compare(Str16ToStr8(str16)),
            "strValue.compare(Str16ToStr8(str16)) did not equal 0 as expected.", state);
    }
    BENCHMARK_LOGD("StringTest test_strcovert_01 end.");
}

BENCHMARK_F(BenchmarkStringTest, test_strcovert_02)(benchmark::State& state)
{
    BENCHMARK_LOGD("StringTest test_strcovert_02 start.");
    while (state.KeepRunning()) {
        string str8Value = "hello world!";
        u16string str16Result = u"hello world!";
        u16string str16Value = Str8ToStr16(str8Value);
        AssertEqual(COMPARE_STRING_RESULT, str16Result.compare(str16Value),
            "str16Result.compare(str16Value) did not equal 0 as expected.", state);

        str16Result = u"你好";
        string str8Result = Str16ToStr8(str16Result);
        str16Value = Str8ToStr16(str8Result);
        AssertEqual(COMPARE_STRING_RESULT, str16Result.compare(str16Value),
            "str16Result.compare(str16Value) did not equal 0 as expected.", state);


        str16Result = u"某某技术有限公司";
        str8Result = Str16ToStr8(str16Result);
        str16Value = Str8ToStr16(str8Result);
        AssertEqual(COMPARE_STRING_RESULT, str16Result.compare(str16Value),
            "str16Result.compare(str16Value) did not equal 0 as expected.", state);
    }
    BENCHMARK_LOGD("StringTest test_strcovert_02 end.");
}

BENCHMARK_F(BenchmarkStringTest, test_strcovert_03)(benchmark::State& state)
{
    BENCHMARK_LOGD("StringTest test_strcovert_03 start.");
    while (state.KeepRunning()) {
        string str8Value = "1234567890!@#$%^&*().";
        u16string str16Result = u"1234567890!@#$%^&*().";
        u16string str16Value = Str8ToStr16(str8Value);
        AssertEqual(COMPARE_STRING_RESULT, str16Result.compare(str16Value),
            "str16Result.compare(str16Value) did not equal 0 as expected.", state);

        string str8Result = Str16ToStr8(str16Value);
        AssertEqual(COMPARE_STRING_RESULT, str8Result.compare(str8Value),
            "str8Result.compare(str8Value) did not equal 0 as expected.", state);
    }
    BENCHMARK_LOGD("StringTest test_strcovert_03 end.");
}

BENCHMARK_F(BenchmarkStringTest, test_strcovert_04)(benchmark::State& state)
{
    BENCHMARK_LOGD("StringTest test_strcovert_04 start.");
    while (state.KeepRunning()) {
        string str8Value = "1234567890!@#$%^&*().qazxswedcvfr,./;'][";
        u16string str16Result = u"1234567890!@#$%^&*().qazxswedcvfr,./;'][";
        u16string str16Value = Str8ToStr16(str8Value);
        AssertEqual(COMPARE_STRING_RESULT, str16Result.compare(str16Value),
            "str16Result.compare(str16Value) did not equal 0 as expected.", state);

        string str8Result = Str16ToStr8(str16Value);
        AssertEqual(COMPARE_STRING_RESULT, str8Result.compare(str8Value),
            "str8Result.compare(str8Value) did not equal 0 as expected.", state);
    }
    BENCHMARK_LOGD("StringTest test_strcovert_04 end.");
}

BENCHMARK_F(BenchmarkStringTest, test_getsubstr_01)(benchmark::State& state)
{
    BENCHMARK_LOGD("StringTest test_getsubstr_01 start.");
    while (state.KeepRunning()) {
        string strBase = "test for {sub str} {sub str1}";
        string left = "{";
        string right = "}";
        string strResult = "sub str";
        string strValue;
        string::size_type pos = GetFirstSubStrBetween(strBase, left, right, strValue);
        AssertEqual(GETSUBSTR01_POS_VALUE1, (int)pos, "17 did not equal (int)pos as expected.", state);
        AssertEqual(strResult, strValue, "strResult did not equal strValue as expected.", state);

        strBase = "test for sub str} {sub str1}";
        strResult = "sub str1";
        pos = GetFirstSubStrBetween(strBase, left, right, strValue);
        AssertEqual(GETSUBSTR01_POS_VALUE2, (int)pos, "27 did not equal (int)pos as expected.", state);
        AssertEqual(strResult, strValue, "strResult did not equal strValue as expected.", state);
    }
    BENCHMARK_LOGD("StringTest test_getsubstr_01 end.");
}

BENCHMARK_F(BenchmarkStringTest, test_getsubstr_02)(benchmark::State& state)
{
    BENCHMARK_LOGD("StringTest test_getsubstr_02 start.");
    while (state.KeepRunning()) {
        string strBase = "test for} {sub str {sub str1";
        string left = "{";
        string right = "}";
        string strValue;
        string::size_type pos = GetFirstSubStrBetween(strBase, left, right, strValue);
        AssertEqual(pos, string::npos, "pos did not equal string::npos as expected.", state);
    }
    BENCHMARK_LOGD("StringTest test_getsubstr_02 end.");
}

BENCHMARK_F(BenchmarkStringTest, test_getsubstr_03)(benchmark::State& state)
{
    BENCHMARK_LOGD("StringTest test_getsubstr_03 start.");
    while (state.KeepRunning()) {
        string strBase = "test for {sub str} {sub str1}";
        string left = "{";
        string right = "}";
        string strResult[GETSUBSTR03_CHAR_ARRAY_SIZE] = { "sub str", "sub str1" };
        vector<string> strValue;
        GetSubStrBetween(strBase, left, right, strValue);
        for (int i = 0; i < GETSUBSTR03_CHAR_ARRAY_SIZE; i++) {
            AssertEqual(strResult[i], strValue[i], "strResult[i] did not equal strValue[i] as expected.", state);
        }
    }
    BENCHMARK_LOGD("StringTest test_getsubstr_03 end.");
}

BENCHMARK_F(BenchmarkStringTest, test_getsubstr_04)(benchmark::State& state)
{
    BENCHMARK_LOGD("StringTest test_getsubstr_04 start.");
    while (state.KeepRunning()) {
        string strBase = "test for } {sub str {sub str1";
        string left = "{";
        string right = "}";
        string strResult[GETSUBSTR04_CHAR_ARRAY_SIZE] = { "sub str", "sub str1" };
        vector<string> strValue;
        GetSubStrBetween(strBase, left, right, strValue);
        AssertEqual(GETSUBSTR04_STRING_SIZE, static_cast<int>(strValue.size()),
            "static_cast<int>(strValue.size()) did not equal 0 as expected.", state);
    }
    BENCHMARK_LOGD("StringTest test_getsubstr_04 end.");
}

BENCHMARK_F(BenchmarkStringTest, DexToHexString_01)(benchmark::State& state)
{
    BENCHMARK_LOGD("StringTest DexToHexString_01 start.");
    while (state.KeepRunning()) {
        int zeroValue = 0;
        string result = DexToHexString(zeroValue);
        AssertEqual(result, "0", "result did not equal \"0\" as expected.", state);

        int positiveValue = 14;
        result = DexToHexString(positiveValue);
        AssertEqual(result, "E", "result did not equal \"E\" as expected.", state);

        result = DexToHexString(positiveValue, false);
        AssertEqual(result, "e", "result did not equal \"e\" as expected.", state);

        int negativeValue = -14;
        result = DexToHexString(negativeValue, false);
        AssertEqual(result, "fffffff2", "result did not equal \"fffffff2\" as expected.", state);

        result = DexToHexString(negativeValue);
        AssertEqual(result, "FFFFFFF2", "result did not equal \"FFFFFFF2\" as expected.", state);

        int largeValue = 11259375;
        result = DexToHexString(largeValue);
        AssertEqual(result, "ABCDEF", "result did not equal \"ABCDEF\" as expected.", state);

        result = DexToHexString(largeValue, false);
        AssertEqual(result, "abcdef", "result did not equal \"abcdef\" as expected.", state);
    }
    BENCHMARK_LOGD("StringTest DexToHexString_01 end.");
}
}  // namespace
}  // namespace OHOS
// Run the benchmark
BENCHMARK_MAIN();