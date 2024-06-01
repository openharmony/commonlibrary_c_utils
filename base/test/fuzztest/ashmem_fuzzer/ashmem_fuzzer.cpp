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

#include "ashmem_fuzzer.h"
#include "fuzzer/FuzzedDataProvider.h"
#include "ashmem.h"

using namespace std;

namespace OHOS {
const int MAX_MEMORY_SIZE = 1024;
const int MAX_MEMORY_NAME_LEN = 10;

void AshmemTestFunc(FuzzedDataProvider* dataProvider)
{
    string name = dataProvider->ConsumeRandomLengthString(MAX_MEMORY_NAME_LEN);
    int memorySize = dataProvider->ConsumeIntegralInRange(0, MAX_MEMORY_SIZE);
    sptr<Ashmem> ashmem = Ashmem::CreateAshmem(name.c_str(), memorySize);
    if (ashmem == nullptr) {
        return;
    }

    bool ret = ashmem->MapReadAndWriteAshmem();
    if (ret != true) {
        return;
    }

    string memoryContent = dataProvider->ConsumeRandomLengthString(MAX_MEMORY_SIZE);
    ret = ashmem->WriteToAshmem(memoryContent.c_str(), memoryContent.size(), 0);
    if (ret != true) {
        return;
    }

    string memoryContent2 = dataProvider->ConsumeRandomLengthString(MAX_MEMORY_SIZE);
    ret = ashmem->WriteToAshmem(memoryContent2.c_str(), memoryContent2.size(), memoryContent.size());
    if (ret != true) {
        return;
    }

    ashmem->ReadFromAshmem(memoryContent.size(), 0);

    ashmem->ReadFromAshmem(memoryContent2.size(), memoryContent.size());

    int prot = dataProvider->ConsumeIntegral<int>();
    ashmem->SetProtection(prot);

    ashmem->UnmapAshmem();
    ashmem->CloseAshmem();
}

} // namespace OHOS

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    FuzzedDataProvider dataProvider(data, size);
    OHOS::AshmemTestFunc(&dataProvider);
    return 0;
}
