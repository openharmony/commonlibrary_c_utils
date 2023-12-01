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
 
#ifndef LOG_H
#define LOG_H

#include "hilog_base/log_base.h"

constexpr LogType BENCHMARK_LOG_TYPE = LOG_CORE;
constexpr unsigned int BENCHMARK_LOG_DOMAIN = 0xD003D00;
constexpr const char *BENCHMARK_LOG_TAG = "benchmark_test";

#define BENCHMARK_LOGF(...) \
    (void)HiLogBasePrint(BENCHMARK_LOG_TYPE, LOG_FATAL, BENCHMARK_LOG_DOMAIN, BENCHMARK_LOG_TAG, __VA_ARGS__)
#define BENCHMARK_LOGE(...) \
    (void)HiLogBasePrint(BENCHMARK_LOG_TYPE, LOG_ERROR, BENCHMARK_LOG_DOMAIN, BENCHMARK_LOG_TAG, __VA_ARGS__)
#define BENCHMARK_LOGW(...) \
    (void)HiLogBasePrint(BENCHMARK_LOG_TYPE, LOG_WARN, BENCHMARK_LOG_DOMAIN, BENCHMARK_LOG_TAG, __VA_ARGS__)
#define BENCHMARK_LOGI(...) \
    (void)HiLogBasePrint(BENCHMARK_LOG_TYPE, LOG_INFO, BENCHMARK_LOG_DOMAIN, BENCHMARK_LOG_TAG, __VA_ARGS__)
#define BENCHMARK_LOGD(...) \
    (void)HiLogBasePrint(BENCHMARK_LOG_TYPE, LOG_DEBUG, BENCHMARK_LOG_DOMAIN, BENCHMARK_LOG_TAG, __VA_ARGS__)

#endif // LOG_H
