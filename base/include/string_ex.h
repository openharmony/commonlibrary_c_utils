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

/**
* @file string_ex.h
*
* @brief Provide global string operation function implemented in c_utils.
*/

/**
* @defgroup StringOperation
* @{
* @brief To operate strings.
*
* Include converting between uppercase and lowercase,
* string replacement, trim and split etc.
*/
#ifndef STRING_EX_H
#define STRING_EX_H

#include <string>
#include <vector>

namespace OHOS {

/**
 * @ingroup StringOperation
 * @brief Convert all letters of string to uppercase.
 *
 * @param str Base string.
 * @return Return a new converted `std::string` object.
 */
std::string UpperStr(const std::string& str);

/**
 * @ingroup StringOperation
 * @brief Convert all letters of string to lowercase.
 *
 * @param str Base string.
 * @return Return a new converted `std::string` object.
 */
std::string LowerStr(const std::string& str);

/**
 * @ingroup StringOperation
 * @brief Replace `src` with `dst` in base string `str`.
 *
 * @param str Target sub-string to be replaced.
 * @param src Base string.
 * @param dst Expected sub-string for replacement.
 * @return Return a new replaced `std::string` object.
 */
std::string ReplaceStr(const std::string& str, const std::string& src, const std::string& dst);

/**
 * @ingroup StringOperation
 * @brief Trim string by `cTrim` at the front and end of `str`.
 *
 * @param str Base string.
 * @param cTrim Target string used in trim，which is '' by default.
 * @return Return a new trimmed `std::string` object.
 */
std::string TrimStr(const std::string& str, const char cTrim = ' ');

/**
 * @ingroup StringOperation
 * @brief Convert decimal to hexadecimal string.
 *
 * @param value Target decimal value.
 * @param upper Specify if output as uppercase,
 * whose value is `true` by default。
 * @return Return a new converted `std::string` object.
 */
std::string DexToHexString(int value, bool upper = true);

/**
 * @ingroup StringOperation
 * @brief Split string by `sep`.
 *
 * @param str Base string.
 * @param sep Sub-string as separator.
 * @param strs `std::vector` object to store the results.
 * @param canEmpty Specify if output empty string as results,
 * whose value is false by default.
 * @param needTrim Specify if need to trim by '',
 * whose value is true by default.
 */
void SplitStr(const std::string& str, const std::string& sep, std::vector<std::string>& strs,
              bool canEmpty = false, bool needTrim = true);

/**
 * @ingroup StringOperation
 * @brief Convert int and double and so on to string.
 *
 * @tparam T Specific type of input data.
 * @param  iValue Input data.
 * @return Return a new converted `std::string` object.
 */
template<class T>
inline std::string ToString(T iValue)
{
    return std::to_string(iValue);
}

/**
 * @ingroup StringOperation
 * @brief Convert string to int.
 *
 * @param str String to be converted.
 * @param value Target `int` variable to store the result.
 * @return Return true if converting success, false if failed.
 */
bool StrToInt(const std::string& str, int& value);

/**
 * @ingroup StringOperation
 * @brief Judge if all characters of the string are numbers.
 *
 * @param str Base string.
 * @return Return true if all are numbers, otherwise false.
 */
bool IsNumericStr(const std::string& str);

/**
 * @ingroup StringOperation
 * @brief Judge if all characters of the string are alphabet.
 *
 * @param str Base string.
 * @return Return true if all are alphabet, otherwise false.
 */
bool IsAlphaStr(const std::string& str);

/**
 * @ingroup StringOperation
 * @brief Judge if all characters of the string are uppercase.
 *
 * @param str Base string.
 * @return Return true if all are uppercase, otherwise false.
 */
bool IsUpperStr(const std::string& str);

/**
 * @ingroup StringOperation
 * @brief Judge if all characters of the string are lowercase.
 *
 * @param str Base string.
 * @return Return true if all are lowercase, otherwise false.
 */
bool IsLowerStr(const std::string& str);

/**
 * @ingroup StringOperation
 * @brief Judge if `str` contains the `sub`.
 *
 * @param str Base string.
 * @param sub Target sub-string.
 * @return Return true if contains, otherwise false.
 */
bool IsSubStr(const std::string& str, const std::string& sub);

/**
 * @ingroup StringOperation
 * @brief Get the first sub_str between `left` and `right`.
 *
 * @param str Base string.
 * @param left Specified left string.
 * @param right Specified right string.
 * @param sub Target `std::string` object to store the result string.
 * @return Return the right string pos, if failed return string::npos.
 */
std::string::size_type GetFirstSubStrBetween(const std::string& str, const std::string& left,
                                             const std::string& right, std::string& sub);

/**
 * @ingroup StringOperation
 * @brief Get all of the sub strings between `left` and `right`.
 *
 * @param str Base string.
 * @param left Specified left string.
 * @param right Specified right string.
 * @param sub Target `std::vector` object to store all of the result strings.
 */
void GetSubStrBetween(const std::string& str, const std::string& left,
                      const std::string& right, std::vector<std::string>& sub);

/**
 * @ingroup StringOperation
 * @brief Judge if the `first`'s letter is same with `second`.
 *
 * @param first First input string.
 * @param second Second input string.
 * @note Case-insensitive.
 */
bool IsSameTextStr(const std::string& first, const std::string& second);

/**
 * @ingroup StringOperation
 * @brief Judge if all of the characters in `str` are ASCII encoded.
 *
 * @param str Base string.
 * @return Return true if all are ASCII encoded, otherwise false.
 */
bool IsAsciiString(const std::string& str);

#ifndef IOS_PLATFORM
/**
 * @ingroup StringOperation
 * @brief Convert a string from UTF-16 to UTF-8 encoded.
 *
 * @param str16 Input `std::u16string` object.
 * @return If converting failed, return an empty `std::string` object.
 */
std::string Str16ToStr8(const std::u16string& str16);

/**
 * @ingroup StringOperation
 * @brief Convert a string from UTF-8 to UTF-16 encoded.
 *
 * @param str Input `std::string` object.
 * @return If converting failed, return an empty `std::u16string` object.
 */
std::u16string Str8ToStr16(const std::string& str);
#endif
} // namespace OHOS

#endif // STRING_EX_H

/**@}*/
