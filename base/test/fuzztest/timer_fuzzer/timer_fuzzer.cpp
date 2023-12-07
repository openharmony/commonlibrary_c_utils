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

#include "timer_fuzzer.h"

#include <thread>

#include "fuzz_log.h"
#include "fuzzer/FuzzedDataProvider.h"
#include "securec.h"
#include "timer.h"

using namespace std;

namespace OHOS {
const uint8_t MAX_OPS = 100;
const uint8_t MAX_STRING_LENGTH = 255;
const int MAX_TIME_MS = 100;
std::atomic<int> g_data(0);

void TimeOutCallback()
{
    g_data++;
}

const std::vector<std::function<void(FuzzedDataProvider*, Utils::Timer&, vector<uint32_t>&)>> ops = {
    [](FuzzedDataProvider* dataProvider, Utils::Timer& timer, vector<uint32_t>& val) {
        uint32_t interval = dataProvider->ConsumeIntegralInRange<uint32_t>(0, MAX_TIME_MS);
        bool once = dataProvider->ConsumeBool();
        uint32_t timerId = timer.Register(TimeOutCallback, interval, once);
        val.push_back(timerId);
        FUZZ_LOGI("Register, interval = %{public}d, once = %{public}d, timerId = %{public}d", interval, once, timerId);
        uint32_t sleepTime = dataProvider->ConsumeIntegralInRange<uint32_t>(0, MAX_TIME_MS);
        std::this_thread::sleep_for(std::chrono::milliseconds(sleepTime));
        FUZZ_LOGI("sleep_for, time = %{public}d, g_data = %{public}d",
            sleepTime, g_data.load(std::memory_order_relaxed));
    },

    [](FuzzedDataProvider* dataProvider, Utils::Timer& timer, vector<uint32_t>& val) {
        if (val.size() == 0) {
            return;
        }
        size_t id = dataProvider->ConsumeIntegralInRange<size_t>(0, val.size() - 1);
        timer.Unregister(val[id]);
        FUZZ_LOGI("Unregister, timerId = %{public}d", val[id]);
        for (size_t i = id; i < val.size() - 1; i++) {
            swap(val[i], val[i + 1]);
        }
        val.pop_back();
    },
};

void TimerTestFunc(const uint8_t* data, size_t size, FuzzedDataProvider* dataProvider)
{
    FUZZ_LOGI("TimerTestFunc start");
    string teststr = dataProvider->ConsumeRandomLengthString(MAX_STRING_LENGTH);
    uint32_t timeoutMs = dataProvider->ConsumeIntegralInRange<uint32_t>(0, MAX_TIME_MS);
    Utils::Timer timer(teststr, timeoutMs);
    FUZZ_LOGI("Timer, str = %{public}s, ms = %{public}d", teststr.c_str(), timeoutMs);
    vector<uint32_t> timerIds;
    g_data = 0;

    while (dataProvider->remaining_bytes() > 0) {
        timer.Setup();
        FUZZ_LOGI("Setup");
        int opCnt = 0;
        while (dataProvider->remaining_bytes() > 0 && opCnt++ < MAX_OPS) {
            uint8_t op = dataProvider->ConsumeIntegral<uint8_t>() % ops.size();
            ops[op](dataProvider, timer, timerIds);
        }
        timer.Shutdown();
        timerIds.clear();
        FUZZ_LOGI("Shutdown");
    }
    FUZZ_LOGI("TimerTestFunc end");
}

} // namespace OHOS

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    FuzzedDataProvider dataProvider(data, size);
    OHOS::TimerTestFunc(data, size, &dataProvider);
    return 0;
}
