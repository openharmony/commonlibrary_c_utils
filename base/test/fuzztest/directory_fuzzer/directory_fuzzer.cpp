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

#include "directory_fuzzer.h"
#include "fuzz_log.h"
#include <fstream>
#include <sys/types.h>
#include "fuzzer/FuzzedDataProvider.h"
#include "directory_ex.h"

using namespace std;

namespace OHOS {
const uint8_t MAX_DIR_LENGTH = 15;
const uint8_t MAX_DIR_LEVEL = 10;
const uint8_t MAX_DIR_NUM = 10;

void DirectoryTestFunc(FuzzedDataProvider* dataProvider)
{
    FUZZ_LOGI("DirectoryTestFunc start");
    GetCurrentProcPath();

    string testDir = "/data/test_dir";
    ForceRemoveDirectory(testDir);

    string testFile = testDir + "test_file";
    ofstream file = ofstream(testFile, fstream::out);
    file.close();

    for (int i = 0; i < MAX_DIR_NUM; i++) {
        string path = testDir;
        int level = dataProvider->ConsumeIntegralInRange<int>(0, MAX_DIR_LEVEL);
        for (int j = 0; j < level; j++) {
            string name = dataProvider->ConsumeRandomLengthString(MAX_DIR_LENGTH);
            path = ExcludeTrailingPathDelimiter(path) + "/" + name;
        }
        ForceCreateDirectory(path);
    }

    string dirName = dataProvider->ConsumeRandomLengthString(MAX_DIR_LENGTH);
    string dirPath = testDir + dirName;
    ForceCreateDirectory(dirPath);

    string realPath1;
    string overLongPath = testDir + dataProvider->ConsumeRandomLengthString(PATH_MAX);
    PathToRealPath(overLongPath, realPath1);

    string fileName = dataProvider->ConsumeRandomLengthString(MAX_DIR_LENGTH);
    ExtractFileExt(fileName);

    string fileFullPath = testDir + fileName;
    mode_t mode = S_IRWXU | S_IRWXG | S_IRWXO;
    ChangeModeFile(fileFullPath, mode);
    ChangeModeDirectory(testDir, mode);
    ChangeModeDirectory("", mode);

    IsEmptyFolder(testDir);
    ForceRemoveDirectory(testDir);

    FUZZ_LOGI("DirectoryTestFunc end");
}

} // namespace OHOS

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    FuzzedDataProvider dataProvider(data, size);
    OHOS::DirectoryTestFunc(&dataProvider);
    return 0;
}
