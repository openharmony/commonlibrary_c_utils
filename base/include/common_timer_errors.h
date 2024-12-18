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
  * @file common_timer_errors.h
  *
  * @brief Provides the values of the <b>Code</b> segment in <b>ErrCode</b>
  * for the Timer module in the commonlibrary subsystem.
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
 * "MODULE_TIMER".
 */

using ErrCode = int;

// The error codes can be used only for the Timer module.
/**
 * @brief Provides the base error codes of the Timer module
 * in the commonlibrary subsystem.
 */
constexpr ErrCode COMMON_TIMER_ERR_OFFSET = ErrCodeOffset(SUBSYS_COMMON, MODULE_TIMER);

/**
 * @brief Enumerates the values of the <b>Code</b> segment in <b>ErrCode</b>
 * for the Timer module.
 *
 * @var TIMER_ERR_OK Indicates an operation success.
 * @var TIMER_ERR_DEAL_FAILED Indicates an operation failure.
 * @var TIMER_ERR_BADF Indicates a bad file descriptor.
 * @var TIMER_ERR_INVALID_VALUE Indicates an invalid value.
 */
enum {
    TIMER_ERR_OK                = 0,
    TIMER_ERR_DEAL_FAILED       = COMMON_TIMER_ERR_OFFSET + EAGAIN,
    TIMER_ERR_BADF              = COMMON_TIMER_ERR_OFFSET + EBADF,
    TIMER_ERR_INVALID_VALUE     = COMMON_TIMER_ERR_OFFSET + EINVAL
};

} // Utils
} // OHOS

#endif
