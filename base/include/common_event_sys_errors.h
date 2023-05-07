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
  * @file common_event_sys_errors.h
  *
  * @brief Provide value of 'Code' segment of ErrCode for 'EventSystem' module in
  * commonlibrary subsystem.
  */

#ifndef UTILS_COMMON_EVENT_SYS_ERRORS_H
#define UTILS_COMMON_EVENT_SYS_ERRORS_H

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
 * In this file, subsystem is "SUBSYS_COMMON" and module is "MODULE_EVENT_SYSTEM".
 */

using ErrCode = int;

// offset of event system module error, only be used in this file.
/**
 * @brief Base ErrCode of module 'EventSystem' in commonlibrary subsystem.
 */
constexpr ErrCode COMMON_EVENT_SYS_ERR_OFFSET = ErrCodeOffset(SUBSYS_COMMON, MODULE_EVENT_SYS);

enum {
    EVENT_SYS_ERR_OK              = COMMON_EVENT_SYS_ERR_OFFSET + 0,
    EVENT_SYS_ERR_FAILED          = COMMON_EVENT_SYS_ERR_OFFSET + 1,
    EVENT_SYS_ERR_ALREADY_STARTED = COMMON_EVENT_SYS_ERR_OFFSET + 2,
    EVENT_SYS_ERR_NOT_FOUND       = COMMON_EVENT_SYS_ERR_OFFSET + 3,
    EVENT_SYS_ERR_BADF            = COMMON_EVENT_SYS_ERR_OFFSET + 4,
    EVENT_SYS_ERR_BADEVENT        = COMMON_EVENT_SYS_ERR_OFFSET + 5,
    EVENT_SYS_ERR_NOEVENT         = COMMON_EVENT_SYS_ERR_OFFSET + 6,
};

} // Utils
} // OHOS

#endif
