/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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

#include "string_ex.h"
#include "unicode_ex.h"
#include "utils_log.h"
#include <iomanip>
#include <sstream>
using namespace std;

namespace OHOS {

static const int CHAR16_TO_CHAR8_PARAM_INVALID = -1;
static const int CHAR16_TO_CHAR8_EMPTY_STR = -2;
static const int CHAR16_TO_CHAR8_INSUFFICIENT_BUFFER = -3;

string UpperStr(const string& str)
{
    string upperString = str;
    transform(upperString.begin(), upperString.end(), upperString.begin(), ::toupper);
    return upperString;
}

string LowerStr(const string& str)
{
    string lowerString = str;
    transform(lowerString.begin(), lowerString.end(), lowerString.begin(), ::tolower);
    return lowerString;
}

string ReplaceStr(const string& str, const string& src, const string& dst)
{
    if (src.empty()) {
        return str;
    }

    string::size_type pos = 0;
    string strTmp = str;
    while ((pos = strTmp.find(src, pos)) != string::npos) {
        strTmp.replace(pos, src.length(), dst);
        pos += dst.length();
    }

    return strTmp;
}

string TrimStr(const string& str, const char cTrim /*= ' '*/)
{
    if (str.size() == 1 && str[0] == cTrim) {
        return string{};
    }

    string strTmp = str;
    std::string::size_type firstBound = strTmp.find_first_not_of(cTrim);
    if (firstBound != std::string::npos) {
        strTmp.erase(0, firstBound);
    }

    std::string::size_type lastBound = strTmp.find_last_not_of(cTrim);
    if (lastBound != std::string::npos && lastBound != strTmp.size() - 1) {
        strTmp.erase(lastBound + sizeof(char));
    }

    return strTmp;
}

string DexToHexString(int value, bool upper /*= true*/)
{
    stringstream ioss;
    string hexString;
    if (upper) {
        ioss << setiosflags(ios::uppercase) << hex << value;
    } else {
        ioss << hex << value;
    }

    ioss >> hexString;
    return hexString;
}

void SplitStr(const string& str, const string& sep, vector<string>& strs, bool canEmpty, bool needTrim)
{
    strs.clear();
    string strTmp = needTrim ? TrimStr(str) : str;
    string strPart;
    while (true) {
        string::size_type pos = strTmp.find(sep);
        if (string::npos == pos || sep.empty()) {
            strPart = needTrim ? TrimStr(strTmp) : strTmp;
            if (!strPart.empty() || canEmpty) {
                strs.push_back(strPart);
            }
            break;
        } else {
            strPart = needTrim ? TrimStr(strTmp.substr(0, pos)) : strTmp.substr(0, pos);
            if (!strPart.empty() || canEmpty) {
                strs.push_back(strPart);
            }
            strTmp = strTmp.substr(sep.size() + pos, strTmp.size() - sep.size() - pos);
        }
    }
}

bool StrToInt(const string& str, int& value)
{
    if (str.empty() || (!isdigit(str.front()) && (str.front() != '-'))) {
        return false;
    }

    char* end = nullptr;
    errno = 0;
    auto addr = str.c_str();
    auto result = strtol(addr, &end, 10); /* 10 means decimal */
    if ((end == addr) || (end[0] != '\0') || (errno == ERANGE) ||
            (result > INT_MAX) || (result < INT_MIN)) {
        return false;
    }
    value = static_cast<int>(result);
    return true;
}

bool IsNumericStr(const string& str)
{
    if (str.empty()) {
        return false;
    }

    for (const auto& c : str) {
        if (!isdigit(c)) {
            return false;
        }
    }

    return true;
}

bool IsAlphaStr(const string& str)
{
    if (str.empty()) {
        return false;
    }

    for (const auto& c : str) {
        if (!isalpha(c)) {
            return false;
        }
    }

    return true;
}

bool IsUpperStr(const string& str)
{
    if (str.empty()) {
        return false;
    }

    for (const auto& c : str) {
        if (!isupper(c)) {
            return false;
        }
    }

    return true;
}

bool IsLowerStr(const string& str)
{
    if (str.empty()) {
        return false;
    }

    for (const auto& c : str) {
        if (!islower(c)) {
            return false;
        }
    }

    return true;
}

bool IsSubStr(const string& str, const string& sub)
{
    if (sub.empty() || str.empty()) {
        return false;
    }

    return str.find(sub) != string::npos;
}

string::size_type GetFirstSubStrBetween(const string& str, const string& left,
    const string& right, string& sub)
{
    string::size_type leftPos = str.find(left);
    if (leftPos == string::npos) {
        return string::npos;
    }

    string::size_type rightPos = str.find(right, leftPos + left.length());
    if (rightPos == string::npos) {
        return string::npos;
    }

    sub = str.substr((leftPos + left.length()), (rightPos - (leftPos + left.length())));
    return rightPos;
}

void GetSubStrBetween(const string& str, const string& left, const string& right, vector<string>& sub)
{
    string subString;
    string strTmp = str;
    string::size_type pos = GetFirstSubStrBetween(strTmp, left, right, subString);
    while (pos != string::npos) {
        sub.push_back(subString);
        strTmp = strTmp.substr(pos);
        pos = GetFirstSubStrBetween(strTmp, left, right, subString);
    }
}

bool IsSameTextStr(const string& first, const string& second)
{
    return UpperStr(first) == UpperStr(second);
}

/*
 * utf8 rule
 * 0000 ~ 007F        : 0xxxxxxx
 * 0080 ~ 07FF        : 110xxxxx 10xxxxxx
 * 0800 ~ FFFF        : 1110xxxx 10xxxxxx 10xxxxxx
 * 10000 ~ 1FFFFF     : 11110xxx 10xxxxxx 10xxxxxx 10xxxxxx
 */
bool IsAsciiString(const string& str)
{
    size_t strLen = str.length();
    for (size_t i = 0; i < strLen; ++i) {
        if ((str[i] & 0x80) != 0) {
            return false;
        }
    }

    return true;
}

#ifndef IOS_PLATFORM
u16string Str8ToStr16(const string& str)
{
    u16string str16Value;
    if (!String8ToString16(str, str16Value)) {
        return u16string();
    }

    return str16Value;
}

string Str16ToStr8(const u16string& str16)
{
    string str8Value;
    if (!String16ToString8(str16, str8Value)) {
        return string();
    }

    return str8Value;
}

int GetUtf16ToUtf8Length(const u16string& str16)
{
    size_t str16Len = str16.length();
    if (str16Len == 0) {
        return -1;
    }
    const char16_t *utf16Str = str16.c_str();
    return Utf16ToUtf8Length(utf16Str, str16Len);
}

int Char16ToChar8(const u16string& str16, char *buffer, int bufferLen)
{
    if (buffer == nullptr || bufferLen <= 0) {
        return CHAR16_TO_CHAR8_PARAM_INVALID;
    }
    size_t str16Len = str16.length();
    if (str16Len == 0) {
        return CHAR16_TO_CHAR8_EMPTY_STR;
    }
    const char16_t *utf16Str = str16.c_str();
    int utf8Len = Utf16ToUtf8Length(utf16Str, str16Len);
    if (utf8Len < 0 || utf8Len >= INT_MAX || (utf8Len + 1) > bufferLen) {
        UTILS_LOGD("utf8buffer len:%{public}d, actual buffer len:%{public}d!", utf8Len + 1, bufferLen);
        return CHAR16_TO_CHAR8_INSUFFICIENT_BUFFER;
    }
    StrncpyStr16ToStr8(utf16Str, str16Len, buffer, utf8Len + 1);
    return utf8Len + 1;
}

#endif
} // namespace OHOS
