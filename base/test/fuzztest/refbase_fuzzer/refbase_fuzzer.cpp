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

#include "refbase_fuzzer.h"

#include <thread>
#include <vector>

#include "fuzz_log.h"
#include "fuzzer/FuzzedDataProvider.h"
#include "refbase.h"
#include "rwlock.h"

using namespace std;

namespace OHOS {
const int MAX_THREADS = 10;
const int MAX_OPS = 200;
const int SLEEP_NANO_SECONDS = 10;

uint32_t GetThreadId()
{
    std::thread::id tid = this_thread::get_id();
    return *reinterpret_cast<uint32_t*>(&tid);
}

struct TestRefBase : public RefBase {
public:
    TestRefBase(bool* deleted, Utils::RWLock& rwLock) : deleted_(deleted), rwLock_(rwLock)
    {
        rwLock_.LockWrite();
        *deleted_ = false;
        rwLock_.UnLockWrite();
        ExtendObjectLifetime();
    }

    virtual ~TestRefBase()
    {
        rwLock_.LockWrite();
        *deleted_ = true;
        rwLock_.UnLockWrite();
    }

    void OnLastStrongRef(const void* objectId) override;
    void OnFirstStrongRef(const void* objectId) override;

private:
    bool* deleted_;
    Utils::RWLock& rwLock_;
};

void TestRefBase::OnLastStrongRef(const void* objectId)
{
    std::this_thread::sleep_for(std::chrono::nanoseconds(SLEEP_NANO_SECONDS));
}

void TestRefBase::OnFirstStrongRef(const void* objectId)
{
    std::this_thread::sleep_for(std::chrono::nanoseconds(SLEEP_NANO_SECONDS));
}

struct SingleThreadRefCounts {
    size_t strongCount = 0;
    size_t weakCount = 0;
    bool weakRefCounterExists = false;
    size_t weakRefCount = 0;
};

TestRefBase* g_ref;
bool g_refModified = false;
bool g_refDeleted = false;
Utils::RWLock g_deletedLock;
Utils::RWLock g_strongLock;
Utils::RWLock g_attemptLock;

const std::vector<std::function<void(SingleThreadRefCounts*, WeakRefCounter*&)>> decOperations = {
    [](SingleThreadRefCounts* refState, WeakRefCounter*&) {
        if (refState->strongCount > 0) {
            refState->strongCount--;
            bool shouldLock = refState->strongCount == 0 && refState->weakCount == 0 && refState->weakRefCount == 0;
            if (shouldLock) {
                g_strongLock.LockWrite();
            }
            FUZZ_LOGI("thread = %{public}u, DecStrongRef, refState->strongCount = %{public}zu", GetThreadId(),
                refState->strongCount);
            g_ref->DecStrongRef(nullptr);
            if (shouldLock) {
                g_strongLock.UnLockWrite();
            }
            g_refModified = true;
        }
    },

    [](SingleThreadRefCounts* refState, WeakRefCounter*& weakRef) {
        if (refState->weakRefCount > 0) {
            refState->weakRefCount--;
            refState->weakRefCounterExists = refState->weakRefCount > 0;
            bool shouldLock = refState->strongCount == 0 && refState->weakCount == 0 && refState->weakRefCount == 0;
            if (shouldLock) {
                g_strongLock.LockWrite();
            }
            FUZZ_LOGI("thread = %{public}u, weakRef->DecWeakRefCount, refState->weakRefCount = %{public}zu",
                GetThreadId(), refState->weakRefCount);
            weakRef->DecWeakRefCount(nullptr);
            if (shouldLock) {
                g_strongLock.UnLockWrite();
            }
            g_refModified = true;
        }
    },

    [](SingleThreadRefCounts* refState, WeakRefCounter*&) {
        if (refState->weakCount > 0) {
            refState->weakCount--;
            bool shouldLock = refState->strongCount == 0 && refState->weakCount == 0 && refState->weakRefCount == 0;
            if (shouldLock) {
                g_strongLock.LockWrite();
            }
            FUZZ_LOGI("thread = %{public}u, DecWeakRef, refState->weakCount = %{public}zu", GetThreadId(),
                refState->weakCount);
            g_ref->DecWeakRef(nullptr);
            if (shouldLock) {
                g_strongLock.UnLockWrite();
            }
            g_refModified = true;
        }
    },
};

const std::vector<std::function<void(SingleThreadRefCounts*, WeakRefCounter*&)>> readOrIncOperations = {
    [](SingleThreadRefCounts*, WeakRefCounter*&) {
        FUZZ_LOGI("thread = %{public}u, GetSptrRefCount", GetThreadId());
        g_ref->GetSptrRefCount();
    },

    [](SingleThreadRefCounts*, WeakRefCounter*&) {
        FUZZ_LOGI("thread = %{public}u, GetRefCounter", GetThreadId());
        RefCounter* refCounter = g_ref->GetRefCounter();
        if (refCounter != nullptr) {
            refCounter->GetRefCount();
        }
    },

    [](SingleThreadRefCounts*, WeakRefCounter*&) {
        FUZZ_LOGI("thread = %{public}u, GetWptrRefCount", GetThreadId());
        g_ref->GetWptrRefCount();
    },

    [](SingleThreadRefCounts*, WeakRefCounter*&) {
        FUZZ_LOGI("thread = %{public}u, IsAttemptAcquireSet", GetThreadId());
        g_ref->IsAttemptAcquireSet();
    },

    [](SingleThreadRefCounts*, WeakRefCounter*&) {
        FUZZ_LOGI("thread = %{public}u, IsExtendLifeTimeSet", GetThreadId());
        g_ref->IsExtendLifeTimeSet();
    },

    [](SingleThreadRefCounts* refState, WeakRefCounter*& weakRef) {
        if (refState->weakRefCounterExists) {
            FUZZ_LOGI("thread = %{public}u, weakRef->GetWeakRefCount", GetThreadId());
            weakRef->GetWeakRefCount();
        }
    },

    [](SingleThreadRefCounts* refState, WeakRefCounter*&) {
        g_attemptLock.LockRead();
        FUZZ_LOGI("thread = %{public}u, IncStrongRef", GetThreadId());
        g_ref->IncStrongRef(nullptr);
        refState->strongCount++;
        g_refModified = true;
        g_attemptLock.UnLockRead();
    },

    [](SingleThreadRefCounts* refState, WeakRefCounter*& weakRef) {
        if (!refState->weakRefCounterExists) {
            FUZZ_LOGI("thread = %{public}u, CreateWeakRef", GetThreadId());
            weakRef = g_ref->CreateWeakRef(nullptr);
            refState->weakRefCounterExists = true;
            // Only CreateWeakRef then release, will not delete RefBase, so g_refModified will not change.
        }
    },

    [](SingleThreadRefCounts* refState, WeakRefCounter*& weakRef) {
        if (refState->weakRefCounterExists) {
            FUZZ_LOGI("thread = %{public}u, weakRef->IncWeakRefCount", GetThreadId());
            weakRef->IncWeakRefCount(nullptr);
            refState->weakRefCount++;
            g_refModified = true;
        }
    },

    [](SingleThreadRefCounts* refState, WeakRefCounter*& weakRef) {
        if (refState->weakRefCounterExists) {
            g_attemptLock.LockWrite();
            FUZZ_LOGI("thread = %{public}u, weakRef->AttemptIncStrongRef", GetThreadId());
            weakRef->AttemptIncStrongRef(nullptr);
            refState->strongCount++;
            g_refModified = true;
            g_attemptLock.UnLockWrite();
        }
    },

    [](SingleThreadRefCounts* refState, WeakRefCounter*&) {
        FUZZ_LOGI("thread = %{public}u, IncWeakRef", GetThreadId());
        g_ref->IncWeakRef(nullptr);
        refState->weakCount++;
        g_refModified = true;
    },

    [](SingleThreadRefCounts* refState, WeakRefCounter*&) {
        g_attemptLock.LockWrite();
        FUZZ_LOGI("thread = %{public}u, AttemptAcquire", GetThreadId());
        g_ref->AttemptAcquire(nullptr);
        g_ref->IncStrongRef(nullptr);
        refState->strongCount++;
        g_refModified = true;
        g_attemptLock.UnLockWrite();
    },

    [](SingleThreadRefCounts* refState, WeakRefCounter*&) {
        g_attemptLock.LockWrite();
        FUZZ_LOGI("thread = %{public}u, AttemptIncStrongRef", GetThreadId());
        g_ref->AttemptIncStrongRef(nullptr);
        refState->strongCount++;
        g_refModified = true;
        g_attemptLock.UnLockWrite();
    },

    [](SingleThreadRefCounts* refState, WeakRefCounter*&) {
        g_attemptLock.LockWrite();
        FUZZ_LOGI("thread = %{public}u, AttemptIncStrong", GetThreadId());
        g_ref->AttemptIncStrong(nullptr);
        g_ref->IncStrongRef(nullptr);
        refState->strongCount++;
        g_refModified = true;
        g_attemptLock.UnLockWrite();
    },
};

// Clean up WeakRefCounter
void CleanUpWeakRefCounter(SingleThreadRefCounts& state, WeakRefCounter* newWeakRef)
{
    if (state.weakRefCounterExists) {
        FUZZ_LOGI("thread = %{public}u, delete newWeakRef", GetThreadId());
        delete newWeakRef;
    } else if (state.weakRefCount > 0) {
        for (; state.weakRefCount > 0; --state.weakRefCount) {
            bool shouldLock = state.strongCount == 0 && state.weakCount == 0 && state.weakRefCount == 0;
            if (shouldLock) {
                g_strongLock.LockWrite();
            }
            FUZZ_LOGI("thread = %{public}u, clean up DecWeakRefCount, refState->weakRefCount = %{public}zu",
                GetThreadId(), state.weakRefCount);
            newWeakRef->DecWeakRefCount(nullptr);
            if (shouldLock) {
                g_strongLock.UnLockWrite();
            }
        }
    }
}

// Clean up any weak references
void CleanUpWeakCounter(SingleThreadRefCounts& state)
{
    for (; state.weakCount > 0; state.weakCount--) {
        bool shouldLock = state.strongCount == 0 && state.weakCount == 1;
        if (shouldLock) {
            g_strongLock.LockWrite();
        }
        FUZZ_LOGI("thread = %{public}u, clean up DecWeakRef, refState->weakCount = %{public}zu", GetThreadId(),
            state.weakCount - 1);
        g_ref->DecWeakRef(nullptr);
        if (shouldLock) {
            g_strongLock.UnLockWrite();
        }
    }
}

// Clean up any strong references
void CleanUpStrongCounter(SingleThreadRefCounts& state)
{
    for (; state.strongCount > 0; state.strongCount--) {
        bool shouldLock = state.strongCount == 1;
        if (shouldLock) {
            g_strongLock.LockWrite();
        }
        FUZZ_LOGI("thread = %{public}u, clean up DecStrongRef, refState->strongCount = %{public}zu", GetThreadId(),
            state.strongCount - 1);
        g_ref->DecStrongRef(nullptr);
        if (shouldLock) {
            g_strongLock.UnLockWrite();
        }
    }
}

void TestLoop(const std::vector<uint8_t>& fuzzOps)
{
    SingleThreadRefCounts state;
    uint8_t lockedOpSize = readOrIncOperations.size();
    uint8_t totalOperationTypes = lockedOpSize + decOperations.size();
    WeakRefCounter* newWeakRef = nullptr;

    for (auto op : fuzzOps) {
        auto opVal = op % totalOperationTypes;
        if (opVal >= lockedOpSize) {
            decOperations[opVal % lockedOpSize](&state, newWeakRef);
        } else {
            bool shouldLock = state.strongCount == 0 && state.weakCount == 0 && state.weakRefCount == 0;
            if (shouldLock) {
                g_strongLock.LockRead();
                if (g_refDeleted) {
                    if (state.weakRefCounterExists) {
                        FUZZ_LOGI("thread = %{public}u, delete newWeakRef", GetThreadId());
                        delete newWeakRef;
                    }
                    FUZZ_LOGI("thread = %{public}u return", GetThreadId());
                    g_strongLock.UnLockRead();
                    return;
                }
            }
            readOrIncOperations[opVal](&state, newWeakRef);
            if (shouldLock) {
                g_strongLock.UnLockRead();
            }
        }
    }

    CleanUpWeakRefCounter(state, newWeakRef);
    CleanUpWeakCounter(state);
    CleanUpStrongCounter(state);
}

void RefbaseTestFunc(const uint8_t* data, size_t size, FuzzedDataProvider* dataProvider)
{
    FUZZ_LOGI("RefbaseTestFunc start");
    g_ref = new TestRefBase(&g_refDeleted, g_deletedLock);
    g_refModified = false;
    uint8_t threadNum = 1;
    vector<thread> threads = vector<thread>();
    threadNum = dataProvider->ConsumeIntegralInRange<uint8_t>(1, MAX_THREADS);

    for (uint8_t i = 0; i < threadNum; i++) {
        uint8_t opsSize = 1;
        opsSize = dataProvider->ConsumeIntegralInRange<uint8_t>(1, MAX_OPS);
        vector<uint8_t> ops = dataProvider->ConsumeBytes<uint8_t>(opsSize);
        thread threadTmp = thread(TestLoop, ops);
        threads.push_back(move(threadTmp));
    }

    for (thread& th : threads) {
        th.join();
    }

    if (!g_refModified && !g_refDeleted) {
        delete g_ref;
        g_ref = nullptr;
    }
    FUZZ_LOGI("RefbaseTestFunc end");
}

} // namespace OHOS

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    FuzzedDataProvider dataProvider(data, size);
    OHOS::RefbaseTestFunc(data, size, &dataProvider);
    return 0;
}
