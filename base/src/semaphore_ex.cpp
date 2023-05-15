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
#include "semaphore_ex.h"

#include <pthread.h>   // pthread_mutex_lock
#include <sys/types.h> // pid_t
#include <unistd.h>    // getpid
#include <fcntl.h>     // O_CREAT
#include <semaphore.h> // sem_t

#include <sstream>     // ostringstream
#include <iomanip>     // setw/setfill

using namespace std;

namespace OHOS {
void Semaphore::Wait()
{
    std::unique_lock<std::mutex> lck(mutex_);
    if (--count_ < 0) {
        cv_.wait(lck);
    }
}

void Semaphore::Post()
{
    std::unique_lock<mutex> lck(mutex_);
    if (++count_ <= 0) {
        cv_.notify_one();
    }
}

} // namespace OHOS
