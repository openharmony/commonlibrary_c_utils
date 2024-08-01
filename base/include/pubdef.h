/*
 * Copyright (c) 2021-2023 Huawei Device Co., Ltd.
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
 * @file pubdef.h
 *
 * @brief The file contains macro definitions for some basic functions
 * in c_utils, for example, unit conversion and space freeup.
 */

#ifndef UTILS_BASE_PUBDEF_H
#define UTILS_BASE_PUBDEF_H

namespace OHOS {

// Used for unit conversion from byte to megabyte
#define B_TO_M(b) ((b) / 1024 / 1024)
// Used for unit conversion from megabyte to byte
#define M_TO_B(m) ((m) * 1024 * 1024)
// Used for unit conversion from byte to kilobyte
#define B_TO_K(b) ((b) / 1024)
// Used for unit conversion from kilobyte to byte
#define K_TO_B(m) ((m) * 1024)

// Used for unit conversion from hour to second
#define HOUR_TO_SECOND(h) ((h) * 3600)
// Used for unit conversion from second to hour
#define SECOND_TO_HOUR(s) ((s) / 3600)

// Used to return the array size
#define ARRAY_SIZE(a) (sizeof((a)) / sizeof((a[0])))

// Used to free the space pointed by pointer p
#define FREE_AND_NIL(p) do {if (p) { delete (p); (p) = nullptr;}} while (0)
// Used to free the array space pointed by pointer p
#define FREE_AND_NIL_ARRAY(p) do {if (p) {delete[] (p); (p) = nullptr;}} while (0)

// Used to return the maximum of two numbers
#define MAX(x, y)  (((x) > (y)) ? (x) : (y))
// Used to return the minimum of two numbers
#define MIN(x, y)  (((x) < (y)) ? (x) : (y))

// Used to determine whether float number 'x' is 0
#define EPS (1e-8)
#define EQUAL_TO_ZERO(x) (fabs(x) <= (EPS))

// Used to attempt a retry when the syscall error code is EINTR
#ifndef TEMP_FAILURE_RETRY
#define TEMP_FAILURE_RETRY(exp)            \
    ({                                     \
    decltype(exp) _rc;                     \
    do {                                   \
        _rc = (exp);                       \
    } while ((_rc == -1) && (errno == EINTR)); \
    _rc;                                   \
    })
#endif
}

#endif

