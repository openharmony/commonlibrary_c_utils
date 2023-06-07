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

 /**
  * @file common_mapped_file_errors.h
  *
  * @brief Provides the values of the <b>Code</b> segment in <b>ErrCode</b>
  * for the MappedFile module in the commonlibrary subsystem.
  */

#ifndef UTILS_COMMON_TIMER_ERRORS_H
#define UTILS_COMMON_TIMER_ERRORS_H

#include <cerrno>
#include "errors.h"
#include "common_errors.h"

namespace OHOS {
namespace Utils {

/**
 * ErrCode layout
 *
 * +-----+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
 * | Bit |31|30|29|28|27|26|25|24|23|22|21|20|19|18|17|16|15|14|13|12|11|10|09|08|07|06|05|04|03|02|01|00|
 * +-----+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
 * |Field|Reserved|        Subsystem      |  Module      |                  Code                         |
 * +-----+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
 *
 * In this file, the subsystem is "SUBSYS_COMMON", and the module is
 * "MODULE_MAPPED_FILE".
 */

using ErrCode = int;

// The error codes can be used only for the MappedFile module.
/**
 * @brief Provides the base error codes of the MappedFile module
 * in the commonlibrary subsystem.
 */
constexpr ErrCode COMMON_MAPPED_FILE_ERR_OFFSET = ErrCodeOffset(SUBSYS_COMMON, MODULE_MAPPED_FILE);

enum {
    MAPPED_FILE_ERR_OK           = COMMON_MAPPED_FILE_ERR_OFFSET + 0,
    MAPPED_FILE_ERR_FAILED       = COMMON_MAPPED_FILE_ERR_OFFSET + 1,
};

} // Utils
} // OHOS

#endif
