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

#ifndef FUZZ_LOG_H
#define FUZZ_LOG_H

#ifdef DEBUG_FUZZ
#include "hilog_base/log_base.h"
constexpr LogType FUZZ_LOG_TYPE = LOG_CORE;
constexpr unsigned int FUZZ_LOG_DOMAIN = 0xD003D00;
constexpr const char *FUZZ_LOG_TAG = "fuzz_test";
#define FUZZ_LOGF(...) (void)HiLogBasePrint(FUZZ_LOG_TYPE, LOG_FATAL, FUZZ_LOG_DOMAIN, FUZZ_LOG_TAG, __VA_ARGS__)
#define FUZZ_LOGE(...) (void)HiLogBasePrint(FUZZ_LOG_TYPE, LOG_ERROR, FUZZ_LOG_DOMAIN, FUZZ_LOG_TAG, __VA_ARGS__)
#define FUZZ_LOGW(...) (void)HiLogBasePrint(FUZZ_LOG_TYPE, LOG_WARN, FUZZ_LOG_DOMAIN, FUZZ_LOG_TAG, __VA_ARGS__)
#define FUZZ_LOGI(...) (void)HiLogBasePrint(FUZZ_LOG_TYPE, LOG_INFO, FUZZ_LOG_DOMAIN, FUZZ_LOG_TAG, __VA_ARGS__)
#define FUZZ_LOGD(...) (void)HiLogBasePrint(FUZZ_LOG_TYPE, LOG_DEBUG, FUZZ_LOG_DOMAIN, FUZZ_LOG_TAG, __VA_ARGS__)
#else
#define PARCEL_LOGF(...)
#define PARCEL_LOGE(...)
#define PARCEL_LOGW(...)
#define PARCEL_LOGI(...)
#define PARCEL_LOGD(...)
#endif  // (defined DEBUG_FUZZ)

#endif  // FUZZ_LOG_H