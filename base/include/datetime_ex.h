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

#ifndef DATETIME_EX_H
#define DATETIME_EX_H

#include <ctime>
#include <cstdint>
namespace OHOS {

/**
 * Here is the definition of strct tm:
 * struct tm
 * {
 *     int tm_sec;                   // Seconds.     [0-60] (1 leap second)
 *     int tm_min;                   // Minutes.     [0-59]
 *     int tm_hour;                  // Hours.       [0-23]
 *     int tm_mday;                  // Day.         [1-31]
 *     int tm_mon;                   // Month.       [0-11]
 *     int tm_year;                  // Year - 1900.
 *     int tm_wday;                  // Day of week. [0-6]
 *     int tm_yday;                  // Days in year.[0-365]
 *     int tm_isdst;                 // DST.         [-1/0/1]
 *     #ifdef  __USE_BSD
 *     long int tm_gmtoff;           //Seconds east of UTC.
 *     __const char *tm_zone;        //Timezone abbreviation.
 *     #else
 *     long int __tm_gmtoff;         //Seconds east of UTC.
 *     __const char *__tm_zone;      //Timezone abbreviation.
 *     #endif
 * };
 */

constexpr int64_t SEC_TO_NANOSEC = 1000000000;
constexpr int64_t SEC_TO_MICROSEC = 1000000;
constexpr int64_t SEC_TO_MILLISEC = 1000;
constexpr int64_t MILLISEC_TO_NANOSEC = 1000000;
constexpr int64_t MICROSEC_TO_NANOSEC = 1000;

constexpr int SECONDS_PER_HOUR = 3600; // 60 * 60
constexpr int SECONDS_PER_DAY = 86400; // 60 * 60 * 24

/**
 * @brief Convert seconds to nanoseconds.
 */
constexpr inline int64_t SecToNanosec(int64_t sec)
{
    return sec * SEC_TO_NANOSEC;
}

/**
 * @brief Convert milliseconds to nanoseconds.
 */
constexpr inline int64_t MillisecToNanosec(int64_t millise)
{
    return millise * MILLISEC_TO_NANOSEC;
}

/**
 * @brief Convert microseconds to nanoseconds.
 */
constexpr inline int64_t MicrosecToNanosec(int64_t microsec)
{
    return microsec * MICROSEC_TO_NANOSEC;
}

/**
 * @brief Convert nanoseconds to seconds.
 */
constexpr inline int64_t NanosecToSec(int64_t nanosec)
{
    return nanosec / SEC_TO_NANOSEC;
}

/**
 * @brief Convert nanoseconds to milliseconds.
 */
constexpr inline int64_t NanosecToMillisec(int64_t nanosec)
{
    return nanosec / MILLISEC_TO_NANOSEC;
}

/**
 * @brief Convert nanoseconds to microseconds.
 */
constexpr inline int64_t NanosecToMicrosec(int64_t nanosec)
{
    return nanosec / MICROSEC_TO_NANOSEC;
}

/**
 * @brief Get seconds from 1970 to now.
 */
int64_t GetSecondsSince1970ToNow();

/**
 * @brief Get seconds from 1970 to inputtime.
 */
int64_t GetSecondsSince1970ToPointTime(struct tm inputTm);

/**
 * @brief Get seconds between inputTm1 and inputTm2.
 */
int64_t GetSecondsBetween(struct tm inputTm1, struct tm inputTm2);

/**
 * @brief Get days from 1970 to now.
 */
int64_t GetDaysSince1970ToNow();

/**
 * @brief Get current timezone.
 *
 * @param timezone Today the world is divided into 24 timezones, they are 
 * medium timezone (zero timezone), east 1-12 timezone, west 1-12 timezone.The 
 * East time zone is +1 to +12 and the West time zone is -1 to -12.
 * @return return true if get success, else false.
 */
bool GetLocalTimeZone(int& timezone);

/**
 * @brief Get current time.
 * @return return true if get success, else false.
 */
bool GetSystemCurrentTime(struct tm* curTime);

/**
 * @brief Get current milliseconds since the system was started.
 */
int64_t GetTickCount();

/**
 * @brief Get current microseconds since the system was started.
 */
int64_t GetMicroTickCount();
}

#endif
