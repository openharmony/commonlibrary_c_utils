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

/** @file file_ex.h
*
*  @brief Provide global file operation functions implemented in c_utils.
*/

/**
* @defgroup FileReadWrite
* @{
* @brief To read from and write to files.
*
* Include reading from or writting to files and searching for specified strings.
*/

#ifndef UTILS_BASE_FILE_EX_H
#define UTILS_BASE_FILE_EX_H

#include <string>
#include <vector>

namespace OHOS {
/**
 * @ingroup FileReadWrite
 * @brief Read contents as a string from the specified file.
 *
 * @param filePath Path of the specified file.
 * @param content Target `std::string` object to store the result.
 * @return Return true on success, false if any error occurs.
 * @note Maximum size of the file is 32MB.
 */
bool LoadStringFromFile(const std::string& filePath, std::string& content);

/**
 * @ingroup FileReadWrite
 * @brief Write contents of a string to the specified file.
 *
 * @param filePath Path of the specified file.
 * @param content Target `std::string` object to be written to the file.
 * @param truncated Specify if truncated the original file.
 * @return Return true on success, false if any error occurs.
 */
bool SaveStringToFile(const std::string& filePath, const std::string& content, bool truncated = true);

/**
 * @ingroup FileReadWrite
 * @brief Read contents as a string from the file specified by its fd.
 *
 * @param fd File descriptor of the specified file.
 * @param content Target `std::string` object to store the result.
 * @return Return true on success, false if any error occurs.
 */
bool LoadStringFromFd(int fd, std::string& content);

/**
 * @ingroup FileReadWrite
 * @brief Write contents of a string to the file specified by its fd.
 *
 * @param fd File descriptor of the specified file.
 * @param content Target `std::string` object to be written to the file.
 * @return Return true on success, false if any error occurs.
 */
bool SaveStringToFd(int fd, const std::string& content);

/**
 * @ingroup FileReadWrite
 * @brief Read contents as a vector from the specified file.
 *
 * @param filePath Path of the specified file.
 * @param content Target `std::vector` object to store the result.
 * @return Return true on success, false if any error occurs.
 */
bool LoadBufferFromFile(const std::string& filePath, std::vector<char>& content);

/**
 * @ingroup FileReadWrite
 * @brief Write contents of a vector to the specified file.
 *
 * @param filePath Path of the specified file.
 * @param content Target `std::vector` object to be written to the file.
 * @return Return true on success, false if any error occurs.
 */
bool SaveBufferToFile(const std::string& filePath, const std::vector<char>& content, bool truncated = true);

/**
 * @ingroup FileReadWrite
 * @brief Check if the specified file exists
 *
 * @param filePath Path of the specified file.
 * @return Return true on success,
 * false if any error(e.g. Permission Denied) occurs.
 */
bool FileExists(const std::string& fileName);

/**
 * @ingroup FileReadWrite
 * @brief Check if the file contains specified contents in string.
 *
 * @param fileName Path of the specified file.
 * @param subStr Specified `std::string` object
 * @param caseSensitive Specify if case-sensitive
 * @return Return true if the file contains the specified string,
 * false if any error occurs.
 */
bool StringExistsInFile(const std::string& fileName, const std::string& subStr, bool caseSensitive = true);

/**
 * @ingroup FileReadWrite
 * @brief Get amount of the specified string in the file.
 *
 * @param fileName Path of the specified file.
 * @param subStr Specified `std::string` object
 * @param caseSensitive Specify if case-sensitive
 * @return Return the amount, return 0 if `subStr` is null.
 */
int  CountStrInFile(const std::string& fileName, const std::string& subStr, bool caseSensitive = true);
}

#endif

/**@}*/
