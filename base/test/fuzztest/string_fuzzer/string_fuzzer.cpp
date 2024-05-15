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

#include "string_fuzzer.h"

#include <cstdint>
#include <thread>
#include <vector>

#include "fuzz_log.h"
#include "fuzzer/FuzzedDataProvider.h"
#include "string_ex.h"

using namespace std;

namespace OHOS {
const uint8_t MAX_STRING_LENGTH = 255;

void StringTestFunc(FuzzedDataProvider* dataProvider)
{
    FUZZ_LOGI("StringTestFunc start");
    string str = dataProvider->ConsumeRandomLengthString(MAX_STRING_LENGTH);
    string src = dataProvider->ConsumeRandomLengthString(MAX_STRING_LENGTH);
    string dst = dataProvider->ConsumeRandomLengthString(MAX_STRING_LENGTH);
    ReplaceStr(str, src, dst);

    int value = dataProvider->ConsumeIntegral<int>();
    bool upper = dataProvider->ConsumeBool();
    DexToHexString(value, upper);

    string newStr = dataProvider->ConsumeRandomLengthString(MAX_STRING_LENGTH);
    IsAlphaStr(newStr);
    IsUpperStr(newStr);
    IsLowerStr(newStr);
    IsNumericStr(newStr);

    string sub = dataProvider->ConsumeRandomLengthString(MAX_STRING_LENGTH);
    IsSubStr(newStr, sub);

    string left = dataProvider->ConsumeRandomLengthString(MAX_STRING_LENGTH);
    string right = dataProvider->ConsumeRandomLengthString(MAX_STRING_LENGTH);
    string emptySubStr;
    GetFirstSubStrBetween(newStr, left, right, emptySubStr);
    FUZZ_LOGI("StringTestFunc end");
}
} // namespace OHOS

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    FuzzedDataProvider dataProvider(data, size);
    OHOS::StringTestFunc(&dataProvider);
    return 0;
}
