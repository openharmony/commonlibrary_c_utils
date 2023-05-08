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

#include <cstdint>
#include <functional>

#ifndef UTILS_IO_EVENT_COMMON_H
#define UTILS_IO_EVENT_COMMON_H

namespace OHOS {
namespace Utils {

using EventId = uint32_t;

using EPEventId = uint32_t;
using REventId = EventId;

using EventCallback = std::function<void()>;

static constexpr int IO_EVENT_INVALID_FD = -1;


namespace Events {
    static constexpr EventId EVENT_NONE  = 0u;
    static constexpr EventId EVENT_READ  = 1u;
    static constexpr EventId EVENT_WRITE = 1u << 1;
    static constexpr EventId EVENT_CLOSE = 1u << 2;
    static constexpr EventId EVENT_ERROR = 1u << 3;
    static constexpr EventId EVENT_INVALID = static_cast<uint32_t>(-1);
}

} // namespace Utils
} // namespace OHOS
#endif /* UTILS_IO_EVENT_COMMON_H */