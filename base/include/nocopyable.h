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

#ifndef UTILS_BASE_NOCOPYABLE_H
#define UTILS_BASE_NOCOPYABLE_H

namespace OHOS {

/**
 * @brief Disables the copy and move construct/assignment method.
 */
#define DISALLOW_COPY_AND_MOVE(className)\
    DISALLOW_COPY(className);\
    DISALLOW_MOVE(className)

/**
 * @brief Disables the copy construct/assignment method.
 */
#define DISALLOW_COPY(className)\
    className(const className&) = delete;\
    className& operator= (const className&) = delete

/**
 * @brief Disables the move construct/assignment method.
 */
#define DISALLOW_MOVE(className)\
    className(className&&) = delete;\
    className& operator= (className&&) = delete


class NoCopyable {
protected:
    NoCopyable() {}
    virtual ~NoCopyable() {}

private:
    DISALLOW_COPY_AND_MOVE(NoCopyable);
};

}

#endif
