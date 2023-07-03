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
 *     int tm_sec;                   // Seconds. Value range: [0-60] (1 leap second)
 *     int tm_min;                   // Minutes. Value range: [0-59]
 *     int tm_hour;                  // Hours. Value range: [0-23]
 *     int tm_mday;                  // Day. Value range: [1-31]
 *     int tm_mon;                   // Month. Value range: [0-11]
 *     int tm_year;                  // Year - 1900.
 *     int tm_wday;                  // Day of week. Value range: [0-6]
 *     int tm_yday;                  // Days in year. Value range: [0-365]
 *     int tm_isdst;                 // DST. Value range: [-1/0/1]
 *     #ifdef  __USE_BSD
 *     long int tm_gmtoff;           // Seconds east of UTC.
 *     __const char *tm_zone;        // Time zone abbreviation.
 *     #else
 *     long int __tm_gmtoff;         // Seconds east of UTC.
 *     __const char *__tm_zone;      // Time zone abbreviation.
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
 * @brief Converts seconds to nanoseconds.
 */
constexpr inline int64_t SecToNanosec(int64_t sec)
{
    return sec * SEC_TO_NANOSEC;
}

/**
 * @brief Converts milliseconds to nanoseconds.
 */
constexpr inline int64_t MillisecToNanosec(int64_t millise)
{
    return millise * MILLISEC_TO_NANOSEC;
}

/**
 * @brief Converts microseconds to nanoseconds.
 */
constexpr inline int64_t MicrosecToNanosec(int64_t microsec)
{
    return microsec * MICROSEC_TO_NANOSEC;
}

/**
 * @brief Converts nanoseconds to seconds.
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
 * @brief Converts nanoseconds to microseconds.
 */
constexpr inline int64_t NanosecToMicrosec(int64_t nanosec)
{
    return nanosec / MICROSEC_TO_NANOSEC;
}

/**
 * @brief Obtains the number of seconds from 00:00:00 on January 1, 1970
 * to the current time.
 */
int64_t GetSecondsSince1970ToNow();

/**
 * @brief Obtains the number of seconds from 00:00:00 on January 1, 1970
 * to the specified point of time.
 */
int64_t GetSecondsSince1970ToPointTime(struct tm inputTm);

/**
 * @brief Obtains the number of seconds between inputTm1 and inputTm2.
 */
int64_t GetSecondsBetween(struct tm inputTm1, struct tm inputTm2);

/**
 * @brief Obtains the number of days from January 1, 1970 to the current date.
 */
int64_t GetDaysSince1970ToNow();

/**
 * @brief Obtains the local time zone.
 *
 * @param timezone Indicates the time zone. A total of 24 time zones are
 * supported, with the eastern time zones represented by +1 to +12, and
 * the western time zones -1 to -12.
 * @return Returns <b>true</b> if the operation is successful;
 * returns <b>false</b> otherwise.
 */
bool GetLocalTimeZone(int& timezone);

/**
 * @brief Obtains the current time.
 * @return Returns <b>true</b> if the operation is successful;
 * returns <b>false</b> otherwise.
 */
bool GetSystemCurrentTime(struct tm* curTime);

/**
 * @brief Obtains the number of milliseconds since the system was started.
 */
int64_t GetTickCount();

/**
 * @brief Obtains the number of microseconds since the system was started.
 */
int64_t GetMicroTickCount();
}

#endif
