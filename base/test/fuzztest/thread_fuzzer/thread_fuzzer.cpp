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

#include "thread_fuzzer.h"
#include "fuzz_log.h"
#include <iostream>
#include <cstdio>
#include <ctime>
#include <unistd.h>
#include <sys/prctl.h>
#include <sys/resource.h>
#include "fuzzer/FuzzedDataProvider.h"
#include "thread_ex.h"

using namespace std;

namespace OHOS {
const std::string& DEFAULT_THREAD_NAME = "default";
const int MAX_STACK_SIZE = 1024;
const int MAX_PRIORITY = 10;
using ThreadRunFunc = bool (*)(int& data);

bool TestRun(int &data)
{
    sleep(1);
    ++data;
    return false;
}

class TestThread : public OHOS::Thread {
public:
    TestThread(const int data, const bool readyToWork, int priority, ThreadRunFunc runFunc)
        : data_(data), priority_(priority), name_(DEFAULT_THREAD_NAME), readyToWork_(readyToWork), runFunc_(runFunc)
        {};

    TestThread() = delete;
    ~TestThread() {}

    bool ReadyToWork() override;

    int data_;
    int priority_;
    std::string name_;
protected:
    bool Run() override;

private:
    bool readyToWork_;
    ThreadRunFunc runFunc_;
};

bool TestThread::ReadyToWork()
{
    return readyToWork_;
}

bool TestThread::Run()
{
    priority_ = getpriority(PRIO_PROCESS, 0);
    char threadName[MAX_THREAD_NAME_LEN + 1] = {0};
    prctl(PR_GET_NAME, threadName, 0, 0);
    name_ = threadName;

    if (runFunc_ != nullptr) {
        return (*runFunc_)(data_);
    }

    return false;
}

void ThreadTestFunc(FuzzedDataProvider* dataProvider)
{
    bool readyToWork = dataProvider->ConsumeBool();
    bool priority = dataProvider->ConsumeIntegralInRange(0, MAX_PRIORITY);
    auto t = std::make_unique<TestThread>(0, readyToWork, priority, TestRun);

    int stacksize = dataProvider->ConsumeIntegralInRange(0, MAX_STACK_SIZE);
    string name = dataProvider->ConsumeRandomLengthString(MAX_THREAD_NAME_LEN);
    bool newPriority = dataProvider->ConsumeIntegralInRange(0, MAX_PRIORITY);
    auto result = t->Start(name, newPriority, stacksize);
    if (result != ThreadStatus::OK) {
        return;
    }
    t->ReadyToWork();
    t->IsExitPending();
    t->IsRunning();
    t->NotifyExitSync();
}

} // namespace OHOS

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    FuzzedDataProvider dataProvider(data, size);
    OHOS::ThreadTestFunc(&dataProvider);
    return 0;
}
