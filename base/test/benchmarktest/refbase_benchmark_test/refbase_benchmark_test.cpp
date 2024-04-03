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

#include <benchmark/benchmark.h>
#include <string>
#include <vector>
#include <algorithm>
#include <fstream>
#include <iostream>
#include <thread>
#include <map>
#include <mutex>
#include "refbase.h"
#include "singleton.h"
#include "benchmark_log.h"
#include "benchmark_assert.h"
#include <future>
using namespace std;

namespace OHOS {
namespace {

class BenchmarkRefbaseTest : public benchmark::Fixture {
    public:
        BenchmarkRefbaseTest()
        {
            Iterations(iterations);
            Repetitions(repetitions);
            ReportAggregatesOnly();
        }

        ~BenchmarkRefbaseTest() override = default;

        void SetUp(const ::benchmark::State &state) override
        {
        }

        void TearDown(const ::benchmark::State &state) override
        {
        }

    protected:
    const int32_t repetitions = 3;
    const int32_t iterations = 1000;
};

static constexpr int FLAG_OF_CONS = 1;
static constexpr int FLAG_OF_DEST = 2;
static int g_sptrCount = 0;
static int g_refbaseflag = 0;
static int g_freeFlag = 0;
constexpr int EXPECTED_REF_COUNT_ZERO = 0;
constexpr int EXPECTED_REF_COUNT_ONE = 1;
constexpr int EXPECTED_REF_COUNT_TWO = 2;
constexpr int EXPECTED_REF_COUNT_THREE = 3;
constexpr int EXPECTED_REF_COUNT_FOUR = 4;

class RefBaseTest : public RefBase {
public:
    RefBaseTest()
    {
        g_refbaseflag = FLAG_OF_CONS;
        isgetrefptr_ = false;
    }
    ~RefBaseTest()
    {
        g_refbaseflag = FLAG_OF_DEST;
    }

    void OnLastStrongRef(const void *objectId) override
    {
        g_freeFlag = 1;
    }

    void SetRefPtr()
    {
        isgetrefptr_ = true;
    }
    bool GetTestRefPtrFlag()
    {
        return isgetrefptr_;
    }

private:
    bool isgetrefptr_;
};

class IRemoteObject : public virtual RefBase {
public:
    IRemoteObject()
    {
        ExtendObjectLifetime();
    }
    virtual bool IsProxyObject() = 0;
    ~IRemoteObject() {}
};

class RefBaseTestTracker : public RefBase {
public:
    explicit RefBaseTestTracker(int value) : value_(value)
    {
        checkCount_++;
    }
    RefBaseTestTracker() = default;
    ~RefBaseTestTracker()
    {
        BENCHMARK_LOGD("RefbaseTest ~RefBaseTestTracker() is called.");
        checkCount_--;
    }

    RefBaseTestTracker(const RefBaseTestTracker &testTracker)
    {
        BENCHMARK_LOGD("RefbaseTest RefBaseTestTracker(const RefBaseTestTracker &testTracker) is called.");
        checkCount_++;
        value_ = testTracker.value_;
    }

    RefBaseTestTracker &operator=(const RefBaseTestTracker &testTracker)
    {
        BENCHMARK_LOGD("RefbaseTest RefBaseTestTracker &operator=(const RefBaseTestTracker &testTracker) is called.");
        checkCount_++;
        value_ = testTracker.value_;
        return *this;
    }

    RefBaseTestTracker(RefBaseTestTracker &&testTracker)
    {
        BENCHMARK_LOGD("RefbaseTest RefBaseTestTracker(RefBaseTestTracker &&testTracker) is called.");
        checkCount_++;
        value_ = testTracker.value_;
    }

    RefBaseTestTracker &operator=(RefBaseTestTracker &&testTracker)
    {
        BENCHMARK_LOGD("RefbaseTest RefBaseTestTracker &operator=(RefBaseTestTracker &&testTracker) is called.");
        checkCount_++;
        value_ = testTracker.value_;
        return *this;
    }

    static RefBaseTestTracker *GetInstance()
    {
        BENCHMARK_LOGD("RefbaseTest static RefBaseTestTracker *GetInstance() is called.");
        static RefBaseTestTracker instance;
        return &instance;
    }

    void InitTracker()
    {
        BENCHMARK_LOGD("RefbaseTest void InitTracker() is called.");
        checkCount_ = 0;
        freeCount_ = 0;
        firstRefCount_ = 0;
        lastRefCount_ = 0;
    }

    void TrackObject(IRemoteObject *object)
    {
        BENCHMARK_LOGD("RefbaseTest void TrackObject(IRemoteObject *object) is called.");
        std::lock_guard<std::mutex> lockGuard(objectMutex_);
        trackObjects_.emplace_back(object);
    }

    void TrackNewObject(IRemoteObject *object)
    {
        BENCHMARK_LOGD("RefbaseTest void TrackNewObject(IRemoteObject *object) is called.");
        std::lock_guard<std::mutex> lockGuard(objectOnfirstMutex_);
        RefBaseTestTracker::firstRefCount_++;
    }

    void UntrackObject(IRemoteObject *object)
    {
        BENCHMARK_LOGD("RefbaseTest void UntrackObject(IRemoteObject *object) is called.");
        std::lock_guard<std::mutex> lockGuard(objectMutex_);
        auto iter = find(trackObjects_.begin(), trackObjects_.end(), object);
        if (iter != trackObjects_.end()) {
            trackObjects_.erase(iter);
        }
    }

    void TrackFreeObject(IRemoteObject *object)
    {
        BENCHMARK_LOGD("RefbaseTest void TrackFreeObject(IRemoteObject *object) is called.");
        std::lock_guard<std::mutex> lockGuard(objectOnfreeMutex_);
        RefBaseTestTracker::freeCount_++;
    }

    void PrintTrackResults()
    {
        BENCHMARK_LOGD("RefbaseTest void PrintTrackResults() is called.");
        std::lock_guard<std::mutex> lockGuard(objectMutex_);
        if (!trackObjects_.empty()) {
            for (auto o : trackObjects_) {
                BENCHMARK_LOGD("object: %{public}p, strong: %{public}d, weak: %{public}d",
                    static_cast<void*>(o), o->GetSptrRefCount(), o->GetWptrRefCount());
            }
        }
        BENCHMARK_LOGD("firstRefCount_: %{public}d", RefBaseTestTracker::firstRefCount_);
        BENCHMARK_LOGD("lastRefCount_: %{public}d", RefBaseTestTracker::lastRefCount_);
        BENCHMARK_LOGD("freeCount_: %{public}d", RefBaseTestTracker::freeCount_);
    }

public:
    int checkCount_ = 0;
    int freeCount_ = 0;
    int firstRefCount_ = 0;
    int lastRefCount_ = 0;

private:

    std::vector<IRemoteObject *> trackObjects_;
    std::mutex objectMutex_;
    std::mutex objectOnfirstMutex_;
    std::mutex objectOnfreeMutex_;
    int value_;
};

class IPCObjectProxy : public IRemoteObject {
public:
    bool IsProxyObject() override
    {
        return 0;
    }
    string descriptor_;
    explicit IPCObjectProxy(const string &descriptor)
    {
        descriptor_ = descriptor;
        RefBaseTestTracker *tracker = RefBaseTestTracker::GetInstance();
        tracker->TrackObject(this);
        tracker->TrackNewObject(this);
    };
    ~IPCObjectProxy() {}
    void RefPtrCallback() override;
    void OnLastStrongRef(const void *objectId) override;
    void OnFirstStrongRef(const void *objectId) override;
    std::mutex mutexLast_;
};

const int SLEEP_FOR_ONE_MILLISECOND = 1;

class IPCProcessSkeleton : public virtual RefBase, public Singleton<IPCProcessSkeleton> {
    friend class Singleton<IPCProcessSkeleton>;

private:
    IPCProcessSkeleton() = default;

public:
    ~IPCProcessSkeleton() override = default;

    std::mutex mutex_;
    std::map<string, wptr<IRemoteObject>> objects1_;

    void DumpMapObjects()
    {
        BENCHMARK_LOGD("RefbaseTest void DumpMapObjects() is called.");
        if (!objects1_.empty()) {
            for (auto &o : objects1_) {
                BENCHMARK_LOGD("strong: %{public}d, weak: %{public}d",
                    o.second->GetSptrRefCount(), o.second->GetWptrRefCount());
            }
        }
    }
    IRemoteObject *QueryObjectInner(const string &descriptor)
    {
        BENCHMARK_LOGD("RefbaseTest IRemoteObject *QueryObjectInner(const string &descriptor) is called.");
        auto it = objects1_.find(descriptor);
        if (it != objects1_.end()) {
            it->second->AttemptAcquire(this);
            std::this_thread::sleep_for(std::chrono::milliseconds(SLEEP_FOR_ONE_MILLISECOND));
            return it->second.GetRefPtr();
        }

        return nullptr;
    }

    IRemoteObject *FindOrNewObject(int handle)
    {
        BENCHMARK_LOGD("RefbaseTest IRemoteObject *FindOrNewObject(int handle) is called.");
        std::lock_guard<std::mutex> lockGuard(mutex_);
        IRemoteObject *remoteObject = QueryObjectInner(to_string(handle));
        if (remoteObject != nullptr) {
            std::this_thread::sleep_for(std::chrono::milliseconds(SLEEP_FOR_ONE_MILLISECOND));
            return remoteObject;
        }

        remoteObject = new IPCObjectProxy(to_string(handle));
        remoteObject->AttemptAcquire(this);
        objects1_.insert(std::pair<string, wptr<IRemoteObject>>(to_string(handle), remoteObject));
        return remoteObject;
    }

    bool DetachObject(IRemoteObject *object, string descriptor)
    {
        BENCHMARK_LOGD("RefbaseTest bool DetachObject(IRemoteObject *object, string descriptor) is called.");
        std::lock_guard<std::mutex> lockGuard(mutex_);
        if (object->GetSptrRefCount()) {
            return false;
        }
        return (objects1_.erase(descriptor) > 0);
    }
};

const int SLEEP_FOR_TEN_NANOSECONDS = 10;

void IPCObjectProxy::OnLastStrongRef(const void *objectId)
{
    BENCHMARK_LOGD("RefbaseTest void IPCObjectProxy::OnLastStrongRef(const void *objectId) is called.");
    std::lock_guard<std::mutex> lock(mutexLast_);
    (void)IPCProcessSkeleton::GetInstance().DetachObject(this, descriptor_);
    RefBaseTestTracker *tracker = RefBaseTestTracker::GetInstance();
    tracker->lastRefCount_++;
    std::this_thread::sleep_for(std::chrono::nanoseconds(SLEEP_FOR_TEN_NANOSECONDS));
}

void IPCObjectProxy::OnFirstStrongRef(const void *objectId)
{
    BENCHMARK_LOGD("RefbaseTest void IPCObjectProxy::OnFirstStrongRef(const void *objectId) is called.");
    std::this_thread::sleep_for(std::chrono::nanoseconds(SLEEP_FOR_TEN_NANOSECONDS));
}

void IPCObjectProxy::RefPtrCallback()
{
    BENCHMARK_LOGD("RefbaseTest void IPCObjectProxy::RefPtrCallback() is called.");
    RefBaseTestTracker *tracker = RefBaseTestTracker::GetInstance();
    tracker->UntrackObject(this);
    tracker->TrackFreeObject(this);
    RefBase::RefPtrCallback();
}

constexpr int CYCLE_NUM1 = 2;
constexpr int CYCLE_NUM2 = 2;

int RegisterEventThread()
{
    BENCHMARK_LOGD("RefbaseTest int RegisterEventThread() is called.");
    auto &ipc = IPCProcessSkeleton::GetInstance();
    int handle = 10;
    for (int i = 0; i < CYCLE_NUM2; i++) {
        sptr<IRemoteObject> remote = ipc.FindOrNewObject(handle);
        remote->CheckIsAttemptAcquireSet(remote);
        if (remote) {
            remote->IsProxyObject();
        }
    }
    return 0;
}

class SptrTest : public RefBase {
public:
    SptrTest()
    {
        g_sptrCount++;
    }
    ~SptrTest()
    {
        g_sptrCount--;
    }
    void CreateSptr()
    {
        test1 = new SptrTest();
    }

private:
    sptr<SptrTest> test1;
};

class SptrTest1;
class SptrTest2;
class SptrTest2 : public RefBase {
public:
    SptrTest2()
    {
        BENCHMARK_LOGD("RefbaseTest SptrTest2() is called.");
        g_sptrCount++;
    }
    ~SptrTest2()
    {
        BENCHMARK_LOGD("RefbaseTest ~SptrTest2() is called.");
        g_sptrCount--;
    }

private:
    sptr<SptrTest1> test;
};

class SptrTest1 : public RefBase {
public:
    SptrTest1()
    {
        BENCHMARK_LOGD("RefbaseTest SptrTest1() is called.");
        g_sptrCount++;
    }
    ~SptrTest1()
    {
        BENCHMARK_LOGD("RefbaseTest ~SptrTest1() is called.");
        g_sptrCount--;
    }

private:
    sptr<SptrTest2> test;
};

class WptrTest : public RefBase {
public:
    WptrTest()
    {
        g_sptrCount++;
    }
    ~WptrTest()
    {
        g_sptrCount--;
    }
};

class WptrTest2 : public RefBase {
public:
    WptrTest2()
    {
        g_sptrCount++;
        flag_ = 0;
    }
    ~WptrTest2()
    {
        g_sptrCount--;
    }

private:
    int flag_;
};

class RefBaseMemTest : public RefBase {
public:
    explicit RefBaseMemTest(int value): value_(value)
    {
        BENCHMARK_LOGD("RefbaseTest explicit RefBaseMemTest(int value): value_(value) is called.");
        checkCount_++;
    }

    ~RefBaseMemTest()
    {
        BENCHMARK_LOGD("RefbaseTest ~RefBaseMemTest() is called.");
        checkCount_--;
    }

    RefBaseMemTest(const RefBaseMemTest &testRefbaseMem)
    {
        BENCHMARK_LOGD("RefbaseTest RefBaseMemTest(const RefBaseMemTest &testRefbaseMem) is called.");
        checkCount_++;
        value_ = testRefbaseMem.value_;
    }

    RefBaseMemTest &operator=(const RefBaseMemTest &testRefbaseMem)
    {
        BENCHMARK_LOGD("RefbaseTest RefBaseMemTest &operator=(const RefBaseMemTest &testRefbaseMem) is called.");
        checkCount_++;
        value_ = testRefbaseMem.value_;
        return *this;
    }

    RefBaseMemTest(RefBaseMemTest &&testRefbaseMem)
    {
        BENCHMARK_LOGD("RefbaseTest RefBaseMemTest(RefBaseMemTest &&testRefbaseMem) is called.");
        checkCount_++;
        value_ = testRefbaseMem.value_;
    }

    RefBaseMemTest &operator=(RefBaseMemTest &&testRefbaseMem)
    {
        BENCHMARK_LOGD("RefbaseTest RefBaseMemTest &operator=(RefBaseMemTest &&testRefbaseMem) is called.");
        checkCount_++;
        value_ = testRefbaseMem.value_;
        return *this;
    }

public:
    static inline int checkCount_ = 0;

private:
    int value_;
};

// This is a class which can be tracked when implemented.
class TestDebug : public RefBase {
public:
    TestDebug()
    {
        EnableTracker();
    }
};

/*
 * @tc.name: testRefbaseOperateThreads001
 * @tc.desc: Refbase for threads
 */
BENCHMARK_F(BenchmarkRefbaseTest, testRefbaseOperateThreads001)(benchmark::State &state)
{
    BENCHMARK_LOGD("RefbaseTest testRefbaseOperateThreads001 start.");
    constexpr int cycleNum = 1;
    while (state.KeepRunning()) {
        RefBaseTestTracker *tracker = RefBaseTestTracker::GetInstance();
        tracker->InitTracker();
        for (int n = 0; n < cycleNum; n++) {
            std::vector<std::future<int>> threads;
            for (int i = 0; i < CYCLE_NUM1; i++) {
                threads.emplace_back(std::async(RegisterEventThread));
            }

            for (auto &f : threads) {
                f.get();
            }
        }
        auto &ipc = IPCProcessSkeleton::GetInstance();
        ipc.DumpMapObjects();
        AssertEqual(tracker->firstRefCount_, tracker->freeCount_,
            "tracker->firstRefCount_ and tracker->freeCount_ are not equal", state);
    }
    BENCHMARK_LOGD("RefbaseTest testRefbaseOperateThreads001 end.");
}

/*
 * @tc.name: testRefbaseOperate001
 * @tc.desc: test AttemptAcquire, IncWeakRef, DecWeakRef, IncStrongRef, DecStrongRef of Refbase
 */
BENCHMARK_F(BenchmarkRefbaseTest, testRefbaseOperate001)(benchmark::State& state)
{
    BENCHMARK_LOGD("RefbaseTest testRefbaseOperate001 start.");
    while (state.KeepRunning()) {
        RefBaseTestTracker *tracker = RefBaseTestTracker::GetInstance();
        tracker->InitTracker();

        auto remoteObject = new IPCObjectProxy("ss");
        remoteObject->AttemptAcquire(this);
        remoteObject->IncWeakRef(this);
        remoteObject->IncStrongRef(this);
        remoteObject->CheckIsAttemptAcquireSet(this);
        remoteObject->DecStrongRef(this);
        remoteObject->AttemptAcquire(this);

        remoteObject->IncStrongRef(this);
        remoteObject->CheckIsAttemptAcquireSet(this);
        remoteObject->DecStrongRef(this);

        remoteObject->DecWeakRef(this);
        AssertEqual(tracker->firstRefCount_, tracker->freeCount_,
            "tracker->firstRefCount_ and tracker->freeCount_ are not equal", state);
    }
    BENCHMARK_LOGD("RefbaseTest testRefbaseOperate001 end.");
}

constexpr int REF_MEM_TEST_OBJ_1_PARAM = 1;
constexpr int REF_MEM_TEST_OBJ_2_PARAM = 2;

/*
 * @tc.name: testRefbaseOperateLeftValue001
 * @tc.desc: Refbase
 */
BENCHMARK_F(BenchmarkRefbaseTest, testRefbaseOperateLeftValue001)(benchmark::State& state)
{
    BENCHMARK_LOGD("RefbaseTest testRefbaseOperateLeftValue001 start.");
    while (state.KeepRunning()) {
        RefBaseMemTest::checkCount_ = 0;
        {
            vector<RefBaseMemTest> refMemTestArray;
            sptr<RefBaseMemTest>refMemTestObj1 = new RefBaseMemTest(REF_MEM_TEST_OBJ_1_PARAM);
            sptr<RefBaseMemTest>refMemTestObj2 = new RefBaseMemTest(REF_MEM_TEST_OBJ_2_PARAM);
            refMemTestArray.push_back(*refMemTestObj1);
            refMemTestArray.push_back(*refMemTestObj2);
        }
        AssertEqual(RefBaseMemTest::checkCount_, 0, "RefBaseMemTest::checkCount_ did not equal 0", state);

        {
            vector<RefBaseMemTest> refMemTestArray;
            RefBaseMemTest refMemTestObj1(REF_MEM_TEST_OBJ_1_PARAM);
            RefBaseMemTest refMemTestObj2(REF_MEM_TEST_OBJ_2_PARAM);
            refMemTestArray.push_back(refMemTestObj1);
            refMemTestArray.push_back(refMemTestObj2);
        }
        AssertEqual(RefBaseMemTest::checkCount_, 0, "RefBaseMemTest::checkCount_ did not equal 0", state);
    }
    BENCHMARK_LOGD("RefbaseTest testRefbaseOperateLeftValue001 end.");
}

/*
 * @tc.name: testRefbaseOperateRightValue001
 * @tc.desc: Refbase
 */
BENCHMARK_F(BenchmarkRefbaseTest, testRefbaseOperateRightValue001)(benchmark::State& state)
{
    BENCHMARK_LOGD("RefbaseTest testRefbaseOperateRightValue001 start.");
    while (state.KeepRunning()) {
        RefBaseMemTest::checkCount_ = 0;
        {
            vector<RefBaseMemTest> refMemTestArray;
            sptr<RefBaseMemTest>refMemTestObj1 = new RefBaseMemTest(REF_MEM_TEST_OBJ_1_PARAM);
            sptr<RefBaseMemTest>refMemTestObj2 = new RefBaseMemTest(REF_MEM_TEST_OBJ_2_PARAM);
            refMemTestArray.emplace_back(*refMemTestObj1);
            refMemTestArray.emplace_back(*refMemTestObj2);
        }
        AssertEqual(RefBaseMemTest::checkCount_, 0, "RefBaseMemTest::checkCount_ did not equal 0", state);

        {
            vector<RefBaseMemTest> refMemTestArray;
            RefBaseMemTest refMemTestObj1(REF_MEM_TEST_OBJ_1_PARAM);
            RefBaseMemTest refMemTestObj2(REF_MEM_TEST_OBJ_2_PARAM);
            refMemTestArray.emplace_back(refMemTestObj1);
            refMemTestArray.emplace_back(refMemTestObj2);
        }
        AssertEqual(RefBaseMemTest::checkCount_, 0, "RefBaseMemTest::checkCount_ did not equal 0", state);
    }
    BENCHMARK_LOGD("RefbaseTest testRefbaseOperateRightValue001 end.");
}

/*
 * @tc.name: testRefbaseAcquire001
 * @tc.desc: Refbase
 */
BENCHMARK_F(BenchmarkRefbaseTest, testRefbaseAcquire001)(benchmark::State& state)
{
    BENCHMARK_LOGD("RefbaseTest testRefbaseAcquire001 start.");
    while (state.KeepRunning()) {
        RefBaseTest* testobject = new RefBaseTest();
        testobject->AttemptAcquire(this);

        g_freeFlag = 0;
        AssertEqual(testobject->GetSptrRefCount(), EXPECTED_REF_COUNT_ONE,
            "testobject->GetSptrRefCount() did not equal EXPECTED_REF_COUNT_ONE", state);

        {
            AssertTrue(testobject->IsAttemptAcquireSet(),
                "testobject->IsAttemptAcquireSet() did not equal true", state);
            testobject->CheckIsAttemptAcquireSet(this);
            sptr<RefBaseTest> sptrRef = testobject;
            AssertEqual(sptrRef->GetSptrRefCount(), EXPECTED_REF_COUNT_ONE,
                "sptrRef->GetSptrRefCount() did not equal EXPECTED_REF_COUNT_ONE", state);
            AssertFalse(testobject->IsAttemptAcquireSet(),
                "testobject->IsAttemptAcquireSet() did not equal false", state);
        }

        AssertEqual(g_freeFlag, EXPECTED_REF_COUNT_ONE, "g_freeFlag did not equal EXPECTED_REF_COUNT_ONE", state);
    }
    BENCHMARK_LOGD("RefbaseTest testRefbaseAcquire001 end.");
}

/*
 * @tc.name: testSptrefbase001
 * @tc.desc: Refbase
 */
BENCHMARK_F(BenchmarkRefbaseTest, testSptrefbase001)(benchmark::State& state)
{
    BENCHMARK_LOGD("RefbaseTest testSptrefbase001 start.");
    while (state.KeepRunning()) {
        sptr<RefBaseTest> testobject = new RefBaseTest();
        testobject->ExtendObjectLifetime();
        AssertTrue(testobject->IsExtendLifeTimeSet(), "testobject->IsExtendLifeTimeSet() did not equal true", state);
        AssertEqual(g_refbaseflag, EXPECTED_REF_COUNT_ONE,
            "g_refbaseflag did not equal EXPECTED_REF_COUNT_ONE", state);
        wptr<RefBaseTest> weakObject(testobject);
        int count = testobject->GetWptrRefCount();
        AssertEqual(count, EXPECTED_REF_COUNT_TWO, "count did not equal EXPECTED_REF_COUNT_TWO", state);
        testobject = nullptr;

        sptr<RefBaseTest> strongObject = weakObject.promote();
        AssertEqual(strongObject->GetSptrRefCount(), EXPECTED_REF_COUNT_ONE,
            "GetSptrRefCount() did not equal EXPECTED_REF_COUNT_ONE", state);
    }
    BENCHMARK_LOGD("RefbaseTest testSptrefbase001 end.");
}

/*
 * @tc.name: testSptrefbaseRealease001
 * @tc.desc: Refbase
 */
BENCHMARK_F(BenchmarkRefbaseTest, testSptrefbaseRealease001)(benchmark::State& state)
{
    BENCHMARK_LOGD("RefbaseTest testSptrefbaseRealease001 start.");
    constexpr int expectedFlag = 1;
    while (state.KeepRunning()) {
        sptr<RefBaseTest> testObject = new RefBaseTest();
        AssertEqual(g_refbaseflag, expectedFlag, "g_refbaseflag did not equal expectedFlag", state);
        wptr<RefBaseTest> weakObject(testObject);
        testObject = nullptr;
        AssertEqual(g_refbaseflag, FLAG_OF_DEST, "g_refbaseflag did not equal FLAG_OF_DEST", state);
    }
    BENCHMARK_LOGD("RefbaseTest testSptrefbaseRealease001 end.");
}

/*
 * @tc.name: testSptrefbaseRealease002
 * @tc.desc: Refbase
 */
BENCHMARK_F(BenchmarkRefbaseTest, testSptrefbaseRealease002)(benchmark::State& state)
{
    BENCHMARK_LOGD("RefbaseTest testSptrefbaseRealease002 start.");
    constexpr int expectedFlag = 1;
    while (state.KeepRunning()) {
        wptr<RefBaseTest> testObject = new RefBaseTest();
        AssertEqual(g_refbaseflag, expectedFlag, "g_refbaseflag did not equal expectedFlag", state);
        testObject = nullptr;
        AssertEqual(g_refbaseflag, FLAG_OF_DEST, "g_refbaseflag did not equal FLAG_OF_DEST", state);
    }
    BENCHMARK_LOGD("RefbaseTest testSptrefbaseRealease002 end.");
}

/*
 * @tc.name: testSptrefbase002
 * @tc.desc: Refbase
 */
BENCHMARK_F(BenchmarkRefbaseTest, testSptrefbase002)(benchmark::State& state)
{
    BENCHMARK_LOGD("RefbaseTest testSptrefbase002 start.");
    constexpr int expectedRefbaseFlag1 = 1;
    constexpr int expectedRefbaseFlag2 = 2;
    while (state.KeepRunning()) {
        {
            sptr<RefBaseTest> testObject(new RefBaseTest());
            AssertEqual(g_refbaseflag, expectedRefbaseFlag1, "g_refbaseflag did not equal expectedRefbaseFlag1", state);
        }
        AssertEqual(g_refbaseflag, expectedRefbaseFlag2,
            "g_refbaseflag did not equal expectedRefbaseFlag2", state);
    }
    BENCHMARK_LOGD("RefbaseTest testSptrefbase002 end.");
}

/*
 * @tc.name: testSptrefbase003
 * @tc.desc: Refbase
 */
BENCHMARK_F(BenchmarkRefbaseTest, testSptrefbase003)(benchmark::State& state)
{
    BENCHMARK_LOGD("RefbaseTest testSptrefbase003 start.");
    while (state.KeepRunning()) {
        sptr<RefBaseTest> testObject1(new RefBaseTest());
        sptr<RefBaseTest> testObject2 = testObject1.GetRefPtr();
        testObject2->SetRefPtr();
        AssertTrue(testObject1->GetTestRefPtrFlag(), "testObject1->GetTestRefPtrFlag() did not equal true", state);

        sptr<RefBaseTest> testObject3(testObject1);
        AssertTrue(testObject3->GetTestRefPtrFlag(), "testObject3->GetTestRefPtrFlag() did not equal true", state);

        sptr<RefBaseTest> testObject4 = testObject1;
        AssertTrue(testObject3->GetTestRefPtrFlag(), "testObject3->GetTestRefPtrFlag() did not equal true", state);

        bool ret = (testObject3 == testObject4);
        AssertTrue(ret, "ret did not equal true", state);

        int refcount = testObject1->GetSptrRefCount();
        AssertEqual(refcount, EXPECTED_REF_COUNT_FOUR, "refcount did not equal EXPECTED_REF_COUNT_FOUR", state);

        sptr<RefBaseTest> testObject5(new RefBaseTest());
        ret = (testObject5 != testObject1);
        AssertTrue(ret, "ret did not equal true", state);
    }
    BENCHMARK_LOGD("RefbaseTest testSptrefbase003 end.");
}

/*
 * @tc.name: testSptrefbase004
 * @tc.desc: Refbase
 */
BENCHMARK_F(BenchmarkRefbaseTest, testSptrefbase004)(benchmark::State& state)
{
    BENCHMARK_LOGD("RefbaseTest testSptrefbase004 start.");
    constexpr int expectedRefbaseFlag = 2;
    while (state.KeepRunning()) {
        sptr<RefBaseTest> testObject1(new RefBaseTest());
        testObject1->SetRefPtr();
        RefBaseTest testObject2 = *testObject1;
        AssertTrue(testObject2.GetTestRefPtrFlag(), "testObject2.GetTestRefPtrFlag() did not equal true", state);

        auto testObject3 = testObject1;
        testObject1 = nullptr;
        testObject3 = nullptr;
        AssertEqual(g_refbaseflag, expectedRefbaseFlag, "g_refbaseflag did not equal expectedRefbaseFlag", state);
    }
    BENCHMARK_LOGD("RefbaseTest testSptrefbase004 end.");
}

/*
 * @tc.name: testSptrefbase005
 * @tc.desc: Refbase
 */
BENCHMARK_F(BenchmarkRefbaseTest, testSptrefbase005)(benchmark::State& state)
{
    BENCHMARK_LOGD("RefbaseTest testSptrefbase005 start.");
    while (state.KeepRunning()) {
        sptr<RefBaseTest> testObject1(new RefBaseTest());
        wptr<RefBaseTest> testObject2 = testObject1;
        AssertEqual(testObject1->GetSptrRefCount(), EXPECTED_REF_COUNT_ONE,
            "testObject1->GetSptrRefCount() did not equal EXPECTED_REF_COUNT_ONE", state);
        AssertEqual(testObject1->GetWptrRefCount(), EXPECTED_REF_COUNT_TWO,
            "testObject1->GetWptrRefCount() did not equal EXPECTED_REF_COUNT_TWO", state);
    }
    BENCHMARK_LOGD("RefbaseTest testSptrefbase005 end.");
}

/*
 * @tc.name: testSptrefbase006
 * @tc.desc: Refbase
 */
BENCHMARK_F(BenchmarkRefbaseTest, testSptrefbase006)(benchmark::State& state)
{
    BENCHMARK_LOGD("RefbaseTest testSptrefbase006 start.");
    while (state.KeepRunning()) {
        sptr<RefBaseTest> testObject1;
        AssertEqual(testObject1.GetRefPtr(), nullptr, "testObject1.GetRefPtr() did not equal nullptr", state);
        testObject1 = new RefBaseTest();
        sptr<RefBaseTest> testObject2(testObject1);
        AssertEqual(testObject1->GetSptrRefCount(), EXPECTED_REF_COUNT_TWO,
            "testObject1->GetSptrRefCount() did not equal EXPECTED_REF_COUNT_TWO", state);
    }
    BENCHMARK_LOGD("RefbaseTest testSptrefbase006 end.");
}

/*
 * @tc.name: testSptrefbase007
 * @tc.desc: Refbase
 */
BENCHMARK_F(BenchmarkRefbaseTest, testSptrefbase007)(benchmark::State& state)
{
    BENCHMARK_LOGD("RefbaseTest testSptrefbase007 start.");
    while (state.KeepRunning()) {
        const sptr<RefBaseTest> &testObject1 = new RefBaseTest();
        sptr<RefBaseTest> testObject2(testObject1);
        AssertEqual(testObject1->GetSptrRefCount(), EXPECTED_REF_COUNT_TWO,
            "testObject1->GetSptrRefCount() did not equal EXPECTED_REF_COUNT_TWO", state);
    }
    BENCHMARK_LOGD("RefbaseTest testSptrefbase007 end.");
}

/*
 * @tc.name: testSptrefbase008
 * @tc.desc: Refbase
 */
BENCHMARK_F(BenchmarkRefbaseTest, testSptrefbase008)(benchmark::State& state)
{
    BENCHMARK_LOGD("RefbaseTest testSptrefbase008 start.");
    while (state.KeepRunning()) {
        sptr<RefBaseTest> testObject1;
        sptr<RefBaseTest> testObject2(testObject1);
        AssertEqual(testObject2, nullptr, "testObject2 did not equal nullptr", state);
    }
    BENCHMARK_LOGD("RefbaseTest testSptrefbase008 end.");
}

/*
 * @tc.name: testSptrefbase009
 * @tc.desc: Refbase
 */
BENCHMARK_F(BenchmarkRefbaseTest, testSptrefbase009)(benchmark::State& state)
{
    BENCHMARK_LOGD("RefbaseTest testSptrefbase009 start.");
    while (state.KeepRunning()) {
        sptr<RefBaseTest> testObject0 = new RefBaseTest();
        sptr<RefBaseTest> testObject1 = move(testObject0);
        sptr<RefBaseTest> testObject2(testObject1);
        AssertEqual(testObject0.GetRefPtr(), nullptr, "testObject0.GetRefPtr() did not equal nullptr", state);
        AssertEqual(testObject2.GetRefPtr(), testObject1.GetRefPtr(),
            "testObject2.GetRefPtr() did not equal testObject1.GetRefPtr()", state);
    }
    BENCHMARK_LOGD("RefbaseTest testSptrefbase009 end.");
}

/*
 * @tc.name: testSptrefbase010
 * @tc.desc: Refbase
 */
BENCHMARK_F(BenchmarkRefbaseTest, testSptrefbase010)(benchmark::State& state)
{
    BENCHMARK_LOGD("RefbaseTest testSptrefbase010 start.");
    while (state.KeepRunning()) {
        sptr<RefBaseTest> testObject1 = new RefBaseTest();
        sptr<RefBaseTest> testObject3(new RefBaseTest());
        sptr<RefBaseTest> &testObject2 = testObject3;
        testObject2 = testObject1;
        AssertEqual(testObject2.GetRefPtr(), testObject1.GetRefPtr(),
            "testObject2.GetRefPtr() did not equal testObject1.GetRefPtr()", state);

        const sptr<RefBaseTest> &testObject4 = new RefBaseTest();
        AssertEqual(testObject1->GetSptrRefCount(), EXPECTED_REF_COUNT_TWO,
            "testObject1->GetSptrRefCount() did not equal EXPECTED_REF_COUNT_TWO", state);
        testObject2 = testObject4;
        AssertEqual(testObject2.GetRefPtr(), testObject4.GetRefPtr(),
            "testObject2.GetRefPtr() did not equal testObject4.GetRefPtr()", state);
        AssertEqual(testObject4->GetSptrRefCount(), EXPECTED_REF_COUNT_TWO,
            "testObject4->GetSptrRefCount() did not equal EXPECTED_REF_COUNT_TWO", state);
        AssertEqual(testObject1->GetSptrRefCount(), EXPECTED_REF_COUNT_ONE,
            "testObject1->GetSptrRefCount() did not equal EXPECTED_REF_COUNT_ONE", state);
    }
    BENCHMARK_LOGD("RefbaseTest testSptrefbase010 end.");
}

/*
 * @tc.name: testSptrefbase011
 * @tc.desc: test MakeSptr of sptr
 */
BENCHMARK_F(BenchmarkRefbaseTest, testSptrefbase011)(benchmark::State& state)
{
    BENCHMARK_LOGD("RefbaseTest testSptrefbase011 start.");
    while (state.KeepRunning()) {
        RefBase ref;
        sptr<RefBase> baseObject = sptr<RefBase>::MakeSptr(ref);
        AssertEqual(baseObject.GetRefPtr()->GetSptrRefCount(), EXPECTED_REF_COUNT_ONE,
            "baseObject.GetRefPtr()->GetSptrRefCount() did not equal EXPECTED_REF_COUNT_ONE", state);
    }
    BENCHMARK_LOGD("RefbaseTest testSptrefbase011 end.");
}

/*
 * @tc.name: testSptrefbase012
 * @tc.desc: test move assignment operator of sptr
 */
BENCHMARK_F(BenchmarkRefbaseTest, testSptrefbase012)(benchmark::State& state)
{
    BENCHMARK_LOGD("RefbaseTest testSptrefbase012 start.");
    while (state.KeepRunning()) {
        sptr<RefBase> baseObject1(new RefBase());
        sptr<RefBase> baseObject2;
        baseObject2 = std::move(baseObject1);
        AssertEqual(baseObject1.GetRefPtr(), nullptr, "baseObject1.GetRefPtr() did not equal nullptr", state);
        AssertEqual(baseObject2.GetRefPtr()->GetSptrRefCount(), EXPECTED_REF_COUNT_ONE,
            "baseObject2.GetRefPtr()->GetSptrRefCount() did not equal EXPECTED_REF_COUNT_ONE", state);
        sptr<RefBase> baseObject3(new RefBase());
        baseObject3 = std::move(baseObject2);
        AssertEqual(baseObject2.GetRefPtr(), nullptr, "baseObject2.GetRefPtr() did not equal nullptr", state);
        AssertEqual(baseObject3.GetRefPtr()->GetSptrRefCount(), EXPECTED_REF_COUNT_ONE,
            "baseObject3.GetRefPtr()->GetSptrRefCount() did not equal EXPECTED_REF_COUNT_ONE", state);
    }
    BENCHMARK_LOGD("RefbaseTest testSptrefbase012 end.");
}

/*
 * @tc.name: testSptrefbase013
 * @tc.desc: test copy Constructor for sptr with the managed class type (O)
 */
BENCHMARK_F(BenchmarkRefbaseTest, testSptrefbase013)(benchmark::State& state)
{
    BENCHMARK_LOGD("RefbaseTest testSptrefbase013 start.");
    while (state.KeepRunning()) {
        sptr<RefBaseTest> baseObject1(new RefBaseTest());
        sptr<RefBase> baseObject2(baseObject1);
        AssertEqual(baseObject1.GetRefPtr(), baseObject2.GetRefPtr(),
            "baseObject1.GetRefPtr() did not equal baseObject2.GetRefPtr()", state);
        AssertEqual(baseObject2.GetRefPtr()->GetSptrRefCount(), EXPECTED_REF_COUNT_TWO,
            "baseObject2.GetRefPtr()->GetSptrRefCount() did not equal EXPECTED_REF_COUNT_TWO", state);
    }
    BENCHMARK_LOGD("RefbaseTest testSptrefbase013 end.");
}

/*
 * @tc.name: testSptrefbase014
 * @tc.desc: test clear of sptr
 */
BENCHMARK_F(BenchmarkRefbaseTest, testSptrefbase014)(benchmark::State& state)
{
    BENCHMARK_LOGD("RefbaseTest testSptrefbase014 start.");
    while (state.KeepRunning()) {
        sptr<RefBaseTest> baseObject(new RefBaseTest());
        baseObject.clear();
        AssertEqual(baseObject.GetRefPtr(), nullptr, "baseObject.GetRefPtr() did not equal nullptr", state);
    }
    BENCHMARK_LOGD("RefbaseTest testSptrefbase014 end.");
}

/*
 * @tc.name: testSptrefbase015
 * @tc.desc: test boolean conversion operator of sptr
 */
BENCHMARK_F(BenchmarkRefbaseTest, testSptrefbase015)(benchmark::State& state)
{
    BENCHMARK_LOGD("RefbaseTest testSptrefbase015 start.");
    while (state.KeepRunning()) {
        sptr<RefBaseTest> baseObject1;
        AssertFalse(baseObject1, "baseObject1 did not equal false", state);
        sptr<RefBaseTest> baseObject2(new RefBaseTest());
        AssertTrue(baseObject2, "baseObject2 did not equal true", state);
    }
    BENCHMARK_LOGD("RefbaseTest testSptrefbase015 end.");
}

/*
 * @tc.name: testSptrefbase016
 * @tc.desc: test Copy assignment operator for sptr with
 * a different managed class type (O)
 */
BENCHMARK_F(BenchmarkRefbaseTest, testSptrefbase016)(benchmark::State& state)
{
    BENCHMARK_LOGD("RefbaseTest testSptrefbase016 start.");
    while (state.KeepRunning()) {
        sptr<RefBase> testObject0(new RefBase());
        sptr<RefBaseTest> testObject1(new RefBaseTest());
        testObject0 = testObject1;
        AssertEqual(testObject1->GetSptrRefCount(), EXPECTED_REF_COUNT_TWO,
            "GetSptrRefCount() did not equal EXPECTED_REF_COUNT_TWO", state);
    }
    BENCHMARK_LOGD("RefbaseTest testSptrefbase016 end.");
}

/*
 * @tc.name: testSptrefbase017
 * @tc.desc: test Equal-to operator between the sptr and wptr
 */
BENCHMARK_F(BenchmarkRefbaseTest, testSptrefbase017)(benchmark::State& state)
{
    BENCHMARK_LOGD("RefbaseTest testSptrefbase017 start.");
    while (state.KeepRunning()) {
        sptr<RefBase> testObject0(new RefBase());
        wptr<RefBase> testObject1(new RefBase());
        AssertUnequal(testObject0, testObject1, "testObject0 did not unequal testObject1", state);
    }
    BENCHMARK_LOGD("RefbaseTest testSptrefbase017 end.");
}

/*
 * @tc.name: testSptrefbase018
 * @tc.desc: test Not-equal-to operator between the sptr and wptr
 */
BENCHMARK_F(BenchmarkRefbaseTest, testSptrefbase018)(benchmark::State& state)
{
    BENCHMARK_LOGD("RefbaseTest testSptrefbase018 start.");
    while (state.KeepRunning()) {
        sptr<RefBase> testObject0(new RefBase());
        wptr<RefBase> testObject1(new RefBase());
        AssertUnequal(testObject0, testObject1, "testObject0 did not unequal testObject1", state);
    }
    BENCHMARK_LOGD("RefbaseTest testSptrefbase018 end.");
}

/*
 * @tc.name: testSptrefbase019
 * @tc.desc: test ForceSetRefPtr of sptr
 */
BENCHMARK_F(BenchmarkRefbaseTest, testSptrefbase019)(benchmark::State& state)
{
    BENCHMARK_LOGD("RefbaseTest testSptrefbase019 start.");
    while (state.KeepRunning()) {
        sptr<RefBase> testObject;
        AssertEqual(testObject.GetRefPtr(), nullptr, "testObject.GetRefPtr() did not equal nullptr", state);
        RefBase baseObject;
        testObject.ForceSetRefPtr(&baseObject);
        AssertEqual(testObject.GetRefPtr(), &baseObject, "testObject.GetRefPtr() did not equal &baseObject", state);
    }
    BENCHMARK_LOGD("RefbaseTest testSptrefbase019 end.");
}

/*
 * @tc.name: testRefbase001
 * @tc.desc: test copy of Refbase.
 */
BENCHMARK_F(BenchmarkRefbaseTest, testRefbase001)(benchmark::State& state)
{
    BENCHMARK_LOGD("RefbaseTest testRefbase001 start.");
    while (state.KeepRunning()) {
        RefBase baseObject1{};
        AssertEqual(baseObject1.GetRefCounter()->GetRefCount(), EXPECTED_REF_COUNT_ONE,
            "refs->GetRefCount() did not equal EXPECTED_REF_COUNT_ONE", state);
        RefBase baseObject2(baseObject1);
        AssertEqual(baseObject1.GetRefCounter()->GetRefCount(), EXPECTED_REF_COUNT_ONE,
            "refs->GetRefCount() did not equal EXPECTED_REF_COUNT_ONE", state);
        RefBase baseObject3;
        baseObject3 = baseObject2;
        AssertEqual(baseObject1.GetRefCounter()->GetRefCount(), EXPECTED_REF_COUNT_ONE,
            "refs->GetRefCount() did not equal EXPECTED_REF_COUNT_ONE", state);
    }
    BENCHMARK_LOGD("RefbaseTest testRefbase001 end.");
}

/*
 * @tc.name: testRefbase002
 * @tc.desc: test AttemptIncStrong of RefBase.
 */
BENCHMARK_F(BenchmarkRefbaseTest, testRefbase002)(benchmark::State& state)
{
    BENCHMARK_LOGD("RefbaseTest testRefbase002 start.");
    while (state.KeepRunning()) {
        RefBase *baseObject = new RefBase();
        baseObject->AttemptIncStrong(this);
        AssertTrue(baseObject->IsAttemptAcquireSet(), "baseObject->IsAttemptAcquireSet() did not equal true", state);
        delete baseObject;
    }
    BENCHMARK_LOGD("RefbaseTest testRefbase002 end.");
}

/*
 * @tc.name: testRefbase003
 * @tc.desc: test AttemptIncStrongRef of RefBase.
 */
BENCHMARK_F(BenchmarkRefbaseTest, testRefbase003)(benchmark::State& state)
{
    BENCHMARK_LOGD("RefbaseTest testRefbase003 start.");
    while (state.KeepRunning()) {
        RefBase *baseObject = new RefBase();
        baseObject->AttemptIncStrongRef(this);
        AssertEqual(baseObject->GetRefCounter()->GetRefCount(), 1, "refs->GetRefCount() did not equal 1", state);
        delete baseObject;
    }
    BENCHMARK_LOGD("RefbaseTest testRefbase003 end.");
}

/*
 * @tc.name: testRefbase004
 * @tc.desc: Refbase
 */
BENCHMARK_F(BenchmarkRefbaseTest, testRefbase004)(benchmark::State& state)
{
    BENCHMARK_LOGD("RefbaseTest testRefbase004 start.");
    while (state.KeepRunning()) {
        {
            sptr<SptrTest> testObject1(new SptrTest());
            testObject1->CreateSptr();
        }
        AssertEqual(g_sptrCount, EXPECTED_REF_COUNT_ZERO, "g_sptrCount did not equal EXPECTED_REF_COUNT_ZERO", state);
    }
    BENCHMARK_LOGD("RefbaseTest testRefbase004 end.");
}

/*
 * @tc.name: testRefbase005
 * @tc.desc: Refbase
 */
BENCHMARK_F(BenchmarkRefbaseTest, testRefbase005)(benchmark::State& state)
{
    BENCHMARK_LOGD("RefbaseTest testRefbase005 start.");
    while (state.KeepRunning()) {
        {
            sptr<SptrTest1> testObject1(new SptrTest1());
            sptr<SptrTest2> testObject2(new SptrTest2());
            AssertEqual(g_sptrCount, EXPECTED_REF_COUNT_TWO,
                "g_sptrCount did not equal EXPECTED_REF_COUNT_TWO", state);
        }
        AssertEqual(g_sptrCount, EXPECTED_REF_COUNT_ZERO, "g_sptrCount did not equal EXPECTED_REF_COUNT_ZERO", state);
    }
    BENCHMARK_LOGD("RefbaseTest testRefbase005 end.");
}

/*
 * @tc.name: testRefbase006
 * @tc.desc: test count of refcounter.
 */
BENCHMARK_F(BenchmarkRefbaseTest, testRefbase006)(benchmark::State& state)
{
    BENCHMARK_LOGD("RefbaseTest testRefbase006 start.");
    while (state.KeepRunning()) {
        sptr<RefBase> testObject1(new RefBase());
        AssertEqual(testObject1->GetRefCounter()->GetRefCount(), EXPECTED_REF_COUNT_ONE,
            "testObject1->GetRefCounter()->GetRefCount() did not equal EXPECTED_REF_COUNT_ONE", state);
        wptr<RefBase> testObject2(testObject1);
        AssertEqual(testObject1->GetRefCounter()->GetRefCount(), EXPECTED_REF_COUNT_TWO,
            "testObject1->GetRefCounter()->GetRefCount() did not equal EXPECTED_REF_COUNT_TWO", state);
    }
    BENCHMARK_LOGD("RefbaseTest testRefbase006 end.");
}

/*
 * @tc.name: testRefbase007
 * @tc.desc: test move constructor.
 */
BENCHMARK_F(BenchmarkRefbaseTest, testRefbase007)(benchmark::State& state)
{
    BENCHMARK_LOGD("RefbaseTest testRefbase007 start.");
    while (state.KeepRunning()) {
        RefBase baseObject1{};
        AssertEqual(baseObject1.GetRefCounter()->GetRefCount(), EXPECTED_REF_COUNT_ONE,
            "baseObject1.GetRefCounter()->GetRefCount() did not equal EXPECTED_REF_COUNT_ONE", state);

        RefBase baseObject2{};
        AssertEqual(baseObject2.GetRefCounter()->GetRefCount(), EXPECTED_REF_COUNT_ONE,
            "baseObject2.GetRefCounter()->GetRefCount() did not equal EXPECTED_REF_COUNT_ONE", state);
        baseObject2 = std::move(baseObject1);
        AssertEqual(baseObject2.GetRefCounter()->GetRefCount(), EXPECTED_REF_COUNT_ONE,
            "baseObject2.GetRefCounter()->GetRefCount() did not equal EXPECTED_REF_COUNT_ONE", state);
        AssertEqual(baseObject1.GetRefCounter(), nullptr, "baseObject1.GetRefCounter() did not equal nullptr", state);
        AssertEqual(baseObject1.GetSptrRefCount(), EXPECTED_REF_COUNT_ZERO,
            "baseObject1.GetSptrRefCount() did not equal EXPECTED_REF_COUNT_ZERO", state);
        AssertEqual(baseObject1.GetWptrRefCount(), EXPECTED_REF_COUNT_ZERO,
            "baseObject1.GetWptrRefCount() did not equal EXPECTED_REF_COUNT_ZERO", state);

        RefBase baseObject3{};
        AssertEqual(baseObject3.GetRefCounter()->GetRefCount(), EXPECTED_REF_COUNT_ONE,
            "baseObject3.GetRefCounter()->GetRefCount() did not equal EXPECTED_REF_COUNT_ONE", state);
        baseObject3 = std::move(baseObject2);
        AssertEqual(baseObject3.GetRefCounter()->GetRefCount(), EXPECTED_REF_COUNT_ONE,
            "baseObject3.GetRefCounter()->GetRefCount() did not equal EXPECTED_REF_COUNT_ONE", state);
        AssertEqual(baseObject2.GetRefCounter(), nullptr, "baseObject2.GetRefCounter() did not equal nullptr", state);
        AssertEqual(baseObject2.GetSptrRefCount(), EXPECTED_REF_COUNT_ZERO,
            "baseObject2.GetSptrRefCount() did not equal EXPECTED_REF_COUNT_ZERO", state);
        AssertEqual(baseObject2.GetWptrRefCount(), EXPECTED_REF_COUNT_ZERO,
            "baseObject2.GetWptrRefCount() did not equal EXPECTED_REF_COUNT_ZERO", state);

        baseObject2 = std::move(baseObject1);
        AssertEqual(baseObject1.GetRefCounter(), baseObject2.GetRefCounter(),
            "baseObject1.GetRefCounter() did not equal baseObject2.GetRefCounter()", state);

        RefBase baseObject4(std::move(baseObject3));
        AssertEqual(baseObject4.GetRefCounter()->GetRefCount(), EXPECTED_REF_COUNT_ONE,
            "refs->GetRefCount() did not equal EXPECTED_REF_COUNT_ONE", state);
    }
    BENCHMARK_LOGD("RefbaseTest testRefbase007 end.");
}

const int ONE_REF_COUNT = 1;
const int TWO_REF_COUNT = 2;
const int THREE_REF_COUNT = 3;

template <typename T1, typename T2>
void CompOrigWptrAndTarWptr001(const wptr<T1> &origWptr, const wptr<T2> &tarWptr, int value, benchmark::State& state)
{
    AssertEqual(static_cast<void *>(origWptr.GetRefPtr()), static_cast<void *>(tarWptr.GetRefPtr()),
        "testOrigWptrObject.GetRefPtr() did not equal tarWptr.GetRefPtr()", state);

    AssertEqual(static_cast<void *>(&(*origWptr)), static_cast<void *>(&(*tarWptr)),
        "&(*testOrigWptrObject) did not equal &(*tarWptr)", state);

    AssertEqual(tarWptr->GetWptrRefCount(), origWptr->GetWptrRefCount(),
        "tarWptr->GetWptrRefCount() did not equal testOrigWptrObject->GetWptrRefCount()", state);

    AssertEqual(tarWptr.GetWeakRefCount(), origWptr.GetWeakRefCount(),
        "tarWptr.GetWeakRefCount() did not equal testOrigWptrObject.GetWeakRefCount()", state);

    AssertEqual(tarWptr->GetWptrRefCount(), ONE_REF_COUNT,
        "tarWptr->GetWptrRefCount() did not equal ONE_REF_COUNT", state);

    if (value == TWO_REF_COUNT) {
        AssertEqual(tarWptr.GetWeakRefCount(), TWO_REF_COUNT,
            "tarWptr.GetWeakRefCount() did not equal TWO_REF_COUNT", state);
    }

    if (value == THREE_REF_COUNT) {
        AssertEqual(tarWptr.GetWeakRefCount(), THREE_REF_COUNT,
            "tarWptr.GetWeakRefCount() did not equal THREE_REF_COUNT", state);
    }
}

template <typename T1, typename T2>
void CompOrigSptrAndTarWptr002(const sptr<T1> &origSptr, const wptr<T2> &tarWptr, int value, benchmark::State& state)
{
    AssertEqual(static_cast<void *>(origSptr.GetRefPtr()), static_cast<void *>(tarWptr.GetRefPtr()),
        "origSptr.GetRefPtr() did not equal tarWptr.GetRefPtr()", state);

    AssertEqual(static_cast<void *>(&(*origSptr)), static_cast<void *>(&(*tarWptr)),
        "&(*origSptr) did not equal &(*tarWptr)", state);

    AssertEqual(tarWptr->GetSptrRefCount(), origSptr->GetSptrRefCount(),
        "tarWptr->GetSptrRefCount() did not equal origSptr->GetSptrRefCount()", state);

    AssertEqual(tarWptr->GetWptrRefCount(), origSptr->GetWptrRefCount(),
        "tarWptr->GetWptrRefCount() did not equal origSptr->GetWptrRefCount()", state);

    AssertEqual(tarWptr->GetSptrRefCount(), ONE_REF_COUNT,
        "tarWptr->GetSptrRefCount() did not equal ONE_REF_COUNT", state);

    if (value == TWO_REF_COUNT) {
        AssertEqual(tarWptr->GetWptrRefCount(), TWO_REF_COUNT,
            "tarWptr->GetWptrRefCount() did not equal TWO_REF_COUNT", state);
    }

    if (value == THREE_REF_COUNT) {
        AssertEqual(tarWptr->GetWptrRefCount(), THREE_REF_COUNT,
            "tarWptr->GetWptrRefCount() did not equal THREE_REF_COUNT", state);
    }

    AssertEqual(tarWptr.GetWeakRefCount(), ONE_REF_COUNT,
        "tarWptr.GetWeakRefCount() did not equal ONE_REF_COUNT", state);
}

/*
 * @tc.name: testWptrefbase001
 * @tc.desc: Copy constructor with same managed class type.
 */
BENCHMARK_F(BenchmarkRefbaseTest, testWptrefbase001)(benchmark::State& state)
{
    BENCHMARK_LOGD("RefbaseTest testWptrefbase001 start.");
    while (state.KeepRunning()) {
        // test wptr<T>::wptr(const wptr<T>&)
        wptr<WptrTest> testOrigWptrObject(new WptrTest());
        AssertEqual(testOrigWptrObject->GetWptrRefCount(), EXPECTED_REF_COUNT_ONE,
            "testOrigWptrObject->GetWptrRefCount() did not equal EXPECTED_REF_COUNT_ONE", state);

        wptr<WptrTest> testTargetWptrObject1(testOrigWptrObject);
        CompOrigWptrAndTarWptr001(testOrigWptrObject, testTargetWptrObject1, EXPECTED_REF_COUNT_TWO, state);

        // test wptr<T>::operator=(const wptr<T>&)
        wptr<WptrTest> testTargetWptrObject2(new WptrTest());
        AssertEqual(testTargetWptrObject2->GetWptrRefCount(), EXPECTED_REF_COUNT_ONE,
            "testTargetWptrObject2->GetWptrRefCount() did not equal EXPECTED_REF_COUNT_ONE", state);

        testTargetWptrObject2 = testOrigWptrObject;
        CompOrigWptrAndTarWptr001(testOrigWptrObject, testTargetWptrObject2, EXPECTED_REF_COUNT_THREE, state);

        // test wptr<T>::wptr(const sptr<T>&)
        sptr<WptrTest> testOrigSptrObject(new WptrTest());
        AssertEqual(testOrigSptrObject->GetSptrRefCount(), EXPECTED_REF_COUNT_ONE,
            "testOrigSptrObject->GetSptrRefCount() did not equal EXPECTED_REF_COUNT_ONE as expected.", state);

        wptr<WptrTest> testTargetWptrObject3(testOrigSptrObject);
        CompOrigSptrAndTarWptr002(testOrigSptrObject, testTargetWptrObject3, EXPECTED_REF_COUNT_TWO, state);

        // test wptr<T>::operator=(const sptr<T>&)
        wptr<WptrTest> testTargetWptrObject4(new WptrTest());
        AssertEqual(testTargetWptrObject4->GetWptrRefCount(), EXPECTED_REF_COUNT_ONE,
            "testTargetWptrObject4->GetWptrRefCount() did not equal EXPECTED_REF_COUNT_ONE as expected.", state);

        testTargetWptrObject4 = testOrigSptrObject;
        CompOrigSptrAndTarWptr002(testOrigSptrObject, testTargetWptrObject4, EXPECTED_REF_COUNT_THREE, state);
    }
    BENCHMARK_LOGD("RefbaseTest testWptrefbase001 end.");
}

/*
 * @tc.name: testWptrefbase002
 * @tc.desc: Copy constructor with different managed class type.
 */
BENCHMARK_F(BenchmarkRefbaseTest, testWptrefbase002)(benchmark::State& state)
{
    BENCHMARK_LOGD("RefbaseTest testWptrefbase002 start.");
    while (state.KeepRunning()) {
        // test wptr<T>::wptr(const wptr<O>&)
        wptr<WptrTest2> testOrigWptrObject(new WptrTest2());
        AssertEqual(testOrigWptrObject->GetWptrRefCount(), EXPECTED_REF_COUNT_ONE,
            "testOrigWptrObject->GetWptrRefCount() did not equal EXPECTED_REF_COUNT_ONE", state);

        wptr<WptrTest> testTargetWptrObject1(testOrigWptrObject);
        CompOrigWptrAndTarWptr001(testOrigWptrObject, testTargetWptrObject1, EXPECTED_REF_COUNT_TWO, state);

        // test wptr<T>::operator=(const wptr<O>&)
        wptr<WptrTest> testTargetWptrObject2(new WptrTest());
        AssertEqual(testTargetWptrObject2->GetWptrRefCount(), EXPECTED_REF_COUNT_ONE,
            "testTargetWptrObject2->GetWptrRefCount() did not equal EXPECTED_REF_COUNT_ONE", state);

        testTargetWptrObject2 = testOrigWptrObject;
        CompOrigWptrAndTarWptr001(testOrigWptrObject, testTargetWptrObject2, EXPECTED_REF_COUNT_THREE, state);

        // test wptr<T>::wptr(const sptr<O>&)
        sptr<WptrTest2> testOrigSptrObject(new WptrTest2());
        AssertEqual(testOrigSptrObject->GetSptrRefCount(), EXPECTED_REF_COUNT_ONE,
            "testOrigSptrObject->GetSptrRefCount() did not equal EXPECTED_REF_COUNT_ONE", state);

        wptr<WptrTest> testTargetWptrObject3(testOrigSptrObject);
        CompOrigSptrAndTarWptr002(testOrigSptrObject, testTargetWptrObject3, EXPECTED_REF_COUNT_TWO, state);

        // test wptr<T>::operator=(const sptr<O>&)
        wptr<WptrTest> testTargetWptrObject4(new WptrTest());
        AssertEqual(testTargetWptrObject4->GetWptrRefCount(), EXPECTED_REF_COUNT_ONE,
            "testTargetWptrObject4->GetWptrRefCount() did not equal EXPECTED_REF_COUNT_ONE", state);

        testTargetWptrObject4 = testOrigSptrObject;
        CompOrigSptrAndTarWptr002(testOrigSptrObject, testTargetWptrObject4, EXPECTED_REF_COUNT_THREE, state);
    }
    BENCHMARK_LOGD("RefbaseTest testWptrefbase002 end.");
}

/*
 * @tc.name: testWptrefbase003
 * @tc.desc: Refbase
 */
BENCHMARK_F(BenchmarkRefbaseTest, testWptrefbase003)(benchmark::State& state)
{
    BENCHMARK_LOGD("RefbaseTest testWptrefbase003 start.");
    while (state.KeepRunning()) {
        const wptr<WptrTest> &testObject1(new WptrTest());
        wptr<WptrTest> testObject2(testObject1);
        AssertEqual(testObject1.GetRefPtr(), testObject2.GetRefPtr(),
            "testObject1.GetRefPtr() did not equal testObject2.GetRefPtr()", state);
        AssertEqual(testObject1->GetWptrRefCount(), EXPECTED_REF_COUNT_ONE,
            "testObject1->GetWptrRefCount() did not equal EXPECTED_REF_COUNT_ONE", state);
        AssertEqual(testObject2->GetWptrRefCount(), EXPECTED_REF_COUNT_ONE,
            "testObject2->GetWptrRefCount() did not equal EXPECTED_REF_COUNT_ONE", state);
        AssertEqual(testObject1.GetRefPtr(), testObject2.GetRefPtr(),
            "testObject1.GetRefPtr() did not equal testObject2.GetRefPtr()", state);
    }
    BENCHMARK_LOGD("RefbaseTest testWptrefbase003 end.");
}

/*
 * @tc.name: testWptrefbase004
 * @tc.desc: Refbase
 */
BENCHMARK_F(BenchmarkRefbaseTest, testWptrefbase004)(benchmark::State& state)
{
    BENCHMARK_LOGD("RefbaseTest testWptrefbase004 start.");
    while (state.KeepRunning()) {
        const sptr<WptrTest2> &testObject1(new WptrTest2());
        AssertUnequal(testObject1, nullptr, "testObject1 was not different from nullptr", state);
        wptr<WptrTest> testObject2 = testObject1;
        AssertEqual(testObject1->GetWptrRefCount(), 2, "testObject1->GetWptrRefCount() did not equal 2", state);
    }
    BENCHMARK_LOGD("RefbaseTest testWptrefbase004 end.");
}

/*
 * @tc.name: testWptrefbase005
 * @tc.desc: wptr without managed object
 */
BENCHMARK_F(BenchmarkRefbaseTest, testWptrefbase005)(benchmark::State& state)
{
    BENCHMARK_LOGD("RefbaseTest testWptrefbase005 start.");
    while (state.KeepRunning()) {
        wptr<WptrTest> testObject3;
        AssertEqual(testObject3.GetRefPtr(), nullptr, "testObject3.GetRefPtr() did not equal nullptr", state);
    }
    BENCHMARK_LOGD("RefbaseTest testWptrefbase005 end.");
}

/*
 * @tc.name: testWptrefbase006
 * @tc.desc: Refbase
 */
BENCHMARK_F(BenchmarkRefbaseTest, testWptrefbase006)(benchmark::State& state)
{
    BENCHMARK_LOGD("RefbaseTest testWptrefbase006 start.");
    while (state.KeepRunning()) {
        wptr<WptrTest> testObject1 = new WptrTest();
        wptr<WptrTest> &testObject2 = testObject1;
        AssertEqual(testObject2->GetWptrRefCount(), EXPECTED_REF_COUNT_ONE,
            "testObject2->GetWptrRefCount() did not equal EXPECTED_REF_COUNT_ONE", state);
    }
    BENCHMARK_LOGD("RefbaseTest testWptrefbase006 end.");
}

/*
 * @tc.name: testWptrefbase007
 * @tc.desc: Refbase
 */
BENCHMARK_F(BenchmarkRefbaseTest, testWptrefbase007)(benchmark::State& state)
{
    BENCHMARK_LOGD("RefbaseTest testWptrefbase007 start.");
    while (state.KeepRunning()) {
        wptr<WptrTest2> testObject1 = new WptrTest2();
        wptr<WptrTest2> testObject2 = testObject1.GetRefPtr();
        AssertEqual(testObject1->GetWptrRefCount(), EXPECTED_REF_COUNT_TWO,
            "testObject1->GetWptrRefCount() did not equal EXPECTED_REF_COUNT_TWO", state);
    }
    BENCHMARK_LOGD("RefbaseTest testWptrefbase007 end.");
}

/*
 * @tc.name: testWptrefbase008
 * @tc.desc: Refbase
 */
BENCHMARK_F(BenchmarkRefbaseTest, testWptrefbase008)(benchmark::State& state)
{
    BENCHMARK_LOGD("RefbaseTest testWptrefbase008 start.");
    while (state.KeepRunning()) {
        wptr<WptrTest> testObject1 = new WptrTest();
        wptr<WptrTest2> testObject2;
        testObject2 = testObject1.GetRefPtr();
        AssertEqual(testObject1->GetWptrRefCount(), EXPECTED_REF_COUNT_TWO,
            "testObject1->GetWptrRefCount() did not equal EXPECTED_REF_COUNT_TWO", state);
    }
    BENCHMARK_LOGD("RefbaseTest testWptrefbase008 end.");
}

/*
 * @tc.name: testWptrefbase009
 * @tc.desc: test Equal-to operator between two wptrs
 */
BENCHMARK_F(BenchmarkRefbaseTest, testWptrefbase009)(benchmark::State& state)
{
    BENCHMARK_LOGD("RefbaseTest testWptrefbase009 start.");
    while (state.KeepRunning()) {
        wptr<RefBase> testObject0(new RefBase());
        wptr<RefBase> testObject1(new RefBase());
        AssertUnequal(testObject0, testObject1, "testObject0 did not unequal testObject1", state);
    }
    BENCHMARK_LOGD("RefbaseTest testWptrefbase009 end.");
}

/*
 * @tc.name: testWptrefbase010
 * @tc.desc: test Not-equal-to operator between two wptrs
 */
BENCHMARK_F(BenchmarkRefbaseTest, testWptrefbase010)(benchmark::State& state)
{
    BENCHMARK_LOGD("RefbaseTest testWptrefbase010 start.");
    while (state.KeepRunning()) {
        wptr<RefBase> testObject0(new RefBase());
        wptr<RefBase> testObject1(new RefBase());
        AssertUnequal(testObject0, testObject1, "testObject0 did not unequal testObject1", state);
    }
    BENCHMARK_LOGD("RefbaseTest testWptrefbase010 end.");
}

/*
 * @tc.name: testWptrefbase011
 * @tc.desc: test Equal-to operator between the wptr and input sptr object
 */
BENCHMARK_F(BenchmarkRefbaseTest, testWptrefbase011)(benchmark::State& state)
{
    BENCHMARK_LOGD("RefbaseTest testWptrefbase011 start.");
    while (state.KeepRunning()) {
        wptr<RefBase> testObject0(new RefBase());
        sptr<RefBase> testObject1(new RefBase());
        AssertUnequal(testObject0, testObject1, "testObject0 did not unequal testObject1", state);
        AssertEqual(testObject1->GetSptrRefCount(), EXPECTED_REF_COUNT_ONE,
            "GetSptrRefCount() did not equal EXPECTED_REF_COUNT_ONE", state);
    }
    BENCHMARK_LOGD("RefbaseTest testWptrefbase011 end.");
}

/*
 * @tc.name: testWptrefbase012
 * @tc.desc: test Not-equal-to operator between the wptr and input sptr object
 */
BENCHMARK_F(BenchmarkRefbaseTest, testWptrefbase012)(benchmark::State& state)
{
    BENCHMARK_LOGD("RefbaseTest testWptrefbase012 start.");
    while (state.KeepRunning()) {
        wptr<RefBase> testObject0(new RefBase());
        sptr<RefBase> testObject1(new RefBase());
        AssertUnequal(testObject0, testObject1, "testObject0 did not unequal testObject1", state);
        AssertEqual(testObject1->GetSptrRefCount(), EXPECTED_REF_COUNT_ONE,
            "GetSptrRefCount() did not equal EXPECTED_REF_COUNT_ONE", state);
    }
    BENCHMARK_LOGD("RefbaseTest testWptrefbase012 end.");
}

/*
 * @tc.name: testWptrefbase013
 * @tc.desc: test AttemptIncStrongRef of wptr
 */
BENCHMARK_F(BenchmarkRefbaseTest, testWptrefbase013)(benchmark::State& state)
{
    BENCHMARK_LOGD("RefbaseTest testWptrefbase013 start.");
    while (state.KeepRunning()) {
        RefBase *baseObject = new RefBase();
        wptr<RefBase> testObject(baseObject);
        testObject.AttemptIncStrongRef(this);
        AssertEqual(baseObject->GetSptrRefCount(), EXPECTED_REF_COUNT_ONE,
            "baseObject->GetSptrRefCount() did not equal EXPECTED_REF_COUNT_ONE", state);
    }
    BENCHMARK_LOGD("RefbaseTest testWptrefbase013 end.");
}

/*
 * @tc.name: testSptrWptrefbase001
 * @tc.desc: test interaction between sptr and wptr.
 */
BENCHMARK_F(BenchmarkRefbaseTest, testSptrWptrefbase001)(benchmark::State& state)
{
    BENCHMARK_LOGD("RefbaseTest testSptrWptrefbase001 start.");
    while (state.KeepRunning()) {
        wptr<RefBase> testObject1(new RefBase());
        AssertEqual(testObject1->GetWptrRefCount(), EXPECTED_REF_COUNT_ONE,
            "testObject1->GetWptrRefCount() did not equal EXPECTED_REF_COUNT_ONE", state);
        {
            sptr<RefBase> testObject2{};
            testObject2 = testObject1;
            AssertEqual(testObject2->GetSptrRefCount(), EXPECTED_REF_COUNT_ONE,
            "testObject2->GetSptrRefCount() did not equal EXPECTED_REF_COUNT_ONE", state);
            AssertEqual(testObject2->GetWptrRefCount(), EXPECTED_REF_COUNT_TWO,
            "testObject2->GetWptrRefCount() did not equal EXPECTED_REF_COUNT_TWO", state);

            sptr<RefBase> testObject3 = testObject1.promote();
            AssertEqual(testObject2->GetSptrRefCount(), EXPECTED_REF_COUNT_TWO,
            "testObject2->GetSptrRefCount() did not equal EXPECTED_REF_COUNT_TWO", state);
            AssertEqual(testObject2->GetWptrRefCount(), EXPECTED_REF_COUNT_THREE,
            "testObject2->GetWptrRefCount() did not equal EXPECTED_REF_COUNT_THREE", state);
            testObject2->ExtendObjectLifetime();
        }
        AssertEqual(testObject1->GetWptrRefCount(), EXPECTED_REF_COUNT_ONE,
            "testObject1->GetWptrRefCount() did not equal EXPECTED_REF_COUNT_ONE", state);
    }
    BENCHMARK_LOGD("RefbaseTest testSptrWptrefbase001 end.");
}

/*
 * @tc.name: testRefbaseDebug001
 * @tc.desc: Test for single thread. Tracker can be enabled after construction
 *           of sptr.
 */
BENCHMARK_F(BenchmarkRefbaseTest, testRefbaseDebug001)(benchmark::State& state)
{
    BENCHMARK_LOGD("RefbaseTest testRefbaseDebug001 start.");
    while (state.KeepRunning()) {
        sptr<RefBase> testObject1(new RefBase());
        testObject1->EnableTracker();
        sptr<RefBase> testObject2(testObject1);
        AssertEqual(testObject2->GetSptrRefCount(), EXPECTED_REF_COUNT_TWO,
            "testObject2->GetSptrRefCount() did not equal EXPECTED_REF_COUNT_TWO", state);
        wptr<RefBase> testObject3(testObject2);
        wptr<RefBase> testObject4(testObject3);
        AssertEqual(testObject4->GetWptrRefCount(), EXPECTED_REF_COUNT_THREE,
            "testObject4->GetWptrRefCount() did not equal EXPECTED_REF_COUNT_THREE", state);
    }
    BENCHMARK_LOGD("RefbaseTest testRefbaseDebug001 end.");
}

/*
 * @tc.name: testRefbaseDebug002
 * @tc.desc: Test for single thread. Tracker can be enabled after construction
 *           of wptr.
 */
BENCHMARK_F(BenchmarkRefbaseTest, testRefbaseDebug002)(benchmark::State& state)
{
    BENCHMARK_LOGD("RefbaseTest testRefbaseDebug002 start.");
    while (state.KeepRunning()) {
        wptr<RefBase> testObject1(new RefBase());
        testObject1->EnableTracker();
        sptr<RefBase> testObject2 = testObject1.promote();
        AssertEqual(testObject2->GetSptrRefCount(), EXPECTED_REF_COUNT_ONE,
            "testObject2->GetSptrRefCount() did not equal EXPECTED_REF_COUNT_ONE", state);
        wptr<RefBase> testObject3(testObject2);
        wptr<RefBase> testObject4(testObject3);
        AssertEqual(testObject4->GetWptrRefCount(), EXPECTED_REF_COUNT_THREE,
            "testObject4->GetWptrRefCount() did not equal EXPECTED_REF_COUNT_THREE", state);
    }
    BENCHMARK_LOGD("RefbaseTest testRefbaseDebug002 end.");
}

/*
 * @tc.name: testRefbaseDebug003
 * @tc.desc: Test for single thread. Tracker can be enabled with construction
 *           of sptr.
 */
BENCHMARK_F(BenchmarkRefbaseTest, testRefbaseDebug003)(benchmark::State& state)
{
    BENCHMARK_LOGD("RefbaseTest testRefbaseDebug003 start.");
    while (state.KeepRunning()) {
        sptr<TestDebug> testObject1(new TestDebug());
        sptr<TestDebug> testObject2(testObject1);
        sptr<TestDebug> testObject3;
        AssertEqual(testObject2->GetSptrRefCount(), EXPECTED_REF_COUNT_TWO,
            "testObject2->GetSptrRefCount() did not equal EXPECTED_REF_COUNT_TWO", state);
        testObject3 = testObject2;
        wptr<TestDebug> testObject4(testObject3);
        AssertEqual(testObject4->GetWptrRefCount(), EXPECTED_REF_COUNT_FOUR,
            "testObject4->GetWptrRefCount() did not equal EXPECTED_REF_COUNT_FOUR", state);
    }
    BENCHMARK_LOGD("RefbaseTest testRefbaseDebug003 end.");
}

/*
 * @tc.name: testRefbaseDebug004
 * @tc.desc: Test for mult-thread.
 */
BENCHMARK_F(BenchmarkRefbaseTest, testRefbaseDebug004)(benchmark::State& state)
{
    BENCHMARK_LOGD("RefbaseTest testRefbaseDebug004 start.");
    while (state.KeepRunning()) {
        sptr<TestDebug> testObject1(new TestDebug());
        std::thread subThread {[&testObject1, &state]() {
            sptr<TestDebug> subTestObject1(testObject1);
            AssertEqual(testObject1->GetSptrRefCount(), EXPECTED_REF_COUNT_TWO,
                "testObject1->GetSptrRefCount() did not equal EXPECTED_REF_COUNT_TWO", state);
            wptr<TestDebug> subTestObject2(subTestObject1);
            wptr<TestDebug> subTestObject3(subTestObject2);
        }};
        wptr<TestDebug> testObject2(testObject1);
        wptr<TestDebug> testObject3(testObject2);
        subThread.join();
        AssertEqual(testObject3->GetWptrRefCount(), EXPECTED_REF_COUNT_TWO,
            "testObject3->GetWptrRefCount() did not equal EXPECTED_REF_COUNT_TWO", state);
    }
    BENCHMARK_LOGD("RefbaseTest testRefbaseDebug004 end.");
}

/*
 * @tc.name: testWeakRefCounter001
 * @tc.desc: test CreateWeakRef of Refbase,
 *           and GetRefPtr, IncWeakRefCount, DecWeakRefCount, GetWeakRefCount, AttemptIncStrongRef of WeakRefCounter.
 */
BENCHMARK_F(BenchmarkRefbaseTest, testWeakRefCounter001)(benchmark::State& state)
{
    BENCHMARK_LOGD("RefbaseTest testWeakRefCounter001 start.");
    while (state.KeepRunning()) {
        RefBase *baseObject1 = new RefBase();
        RefBase *newRefbase = new RefBase();
        WeakRefCounter *newWeakRef = baseObject1->CreateWeakRef(newRefbase);
        AssertEqual(newWeakRef->GetRefPtr(), newRefbase,
            "WeakRefCounter->GetRefPtr() did not equal RefBase object", state);
        newWeakRef->IncWeakRefCount(this);
        newWeakRef->IncWeakRefCount(this);
        AssertEqual(newWeakRef->GetWeakRefCount(), 2, "WeakRefCounter->GetWeakRefCount() did not equal 2", state);
        newWeakRef->AttemptIncStrongRef(this);
        AssertEqual(baseObject1->GetRefCounter()->GetStrongRefCount(), EXPECTED_REF_COUNT_ONE,
            "RefCounter->GetStrongRefCount() did not equal EXPECTED_REF_COUNT_ONE", state);
        newWeakRef->DecWeakRefCount(this);
        AssertEqual(newWeakRef->GetWeakRefCount(), EXPECTED_REF_COUNT_ONE,
            "WeakRefCounter->GetWeakRefCount() did not equal EXPECTED_REF_COUNT_ONE", state);
        newWeakRef->DecWeakRefCount(this);
        delete newRefbase;
        delete baseObject1;
    }
    BENCHMARK_LOGD("RefbaseTest testWeakRefCounter001 end.");
}

/*
 * @tc.name: testRefCounter001
 * @tc.desc: Test for IncRefCount, DecRefCount and GetRefCount
 */
BENCHMARK_F(BenchmarkRefbaseTest, testRefCounter001)(benchmark::State& state)
{
    BENCHMARK_LOGD("RefbaseTest testRefCounter001 start.");
    while (state.KeepRunning()) {
        RefCounter *refs = new RefCounter();
        refs->IncRefCount();
        refs->IncRefCount();
        AssertEqual(refs->GetRefCount(), EXPECTED_REF_COUNT_TWO,
            "refs->GetRefCount() did not equal EXPECTED_REF_COUNT_TWO", state);
        refs->DecRefCount();
        AssertEqual(refs->GetRefCount(), EXPECTED_REF_COUNT_ONE,
            "refs->GetRefCount() did not equal EXPECTED_REF_COUNT_ONE", state);
        refs->DecRefCount();
    }
    BENCHMARK_LOGD("RefbaseTest testRefCounter001 end.");
}

/*
@tc.name: testRefCounter002
@tc.desc: Test for SetCallback, RemoveCallback and IsRefPtrValid
*/
BENCHMARK_F(BenchmarkRefbaseTest, testRefCounter002)(benchmark::State& state)
{
    BENCHMARK_LOGD("RefbaseTest testRefCounter002 start.");
    while (state.KeepRunning()) {
        RefCounter *refs = new RefCounter();
        int externalVariable = 0;

        // Use the SetCallback method to set a lambda function as a
        // callback that sets the value of externalVariable to 42 (an arbitrary value)
        refs->SetCallback([&externalVariable]() { externalVariable = 42; });
        AssertTrue(refs->IsRefPtrValid(), "refs->IsRefPtrValid() did not return true", state);
        refs->ExtendObjectLifetime();
        refs->IncWeakRefCount(this);
        refs->DecWeakRefCount(this);
        AssertEqual(externalVariable, 42, "externalVariable did not equal 42", state);
        refs->RemoveCallback();
        AssertFalse(refs->IsRefPtrValid(), "refs->IsRefPtrValid() did not return false", state);
        delete refs;
    }
    BENCHMARK_LOGD("RefbaseTest testRefCounter002 end.");
}

/*
 * @tc.name: testRefCounter003
 * @tc.desc: Test for IncStrongRefCount, DecStrongRefCount and GetStrongRefCount
 */
BENCHMARK_F(BenchmarkRefbaseTest, testRefCounter003)(benchmark::State& state)
{
    BENCHMARK_LOGD("RefbaseTest testRefCounter003 start.");
    while (state.KeepRunning()) {
        RefCounter *refs = new RefCounter();
        refs->IncStrongRefCount(this);
        int strongCnt = refs->IncStrongRefCount(this);
        AssertEqual(strongCnt, EXPECTED_REF_COUNT_ONE,
            "Before IncStrongRefCount, strong count was not EXPECTED_REF_COUNT_ONE", state);
        AssertEqual(refs->GetStrongRefCount(), EXPECTED_REF_COUNT_TWO,
            "After increments, refs->GetStrongRefCount() did not equal EXPECTED_REF_COUNT_TWO", state);
        refs->DecStrongRefCount(this);
        AssertEqual(refs->GetStrongRefCount(), EXPECTED_REF_COUNT_ONE,
            "After decrement, refs->GetStrongRefCount() did not equal EXPECTED_REF_COUNT_ONE", state);
        strongCnt = refs->DecStrongRefCount(this);
        AssertEqual(strongCnt, EXPECTED_REF_COUNT_ONE,
            "Before DecStrongRefCount, strong count was not EXPECTED_REF_COUNT_ONE", state);
        AssertEqual(refs->GetStrongRefCount(), EXPECTED_REF_COUNT_ZERO,
            "At the end, refs->GetStrongRefCount() did not equal EXPECTED_REF_COUNT_ZERO", state);
        delete refs;
    }
    BENCHMARK_LOGD("RefbaseTest testRefCounter003 end.");
}

/*
 * @tc.name: testRefCounter004
 * @tc.desc: Test for IncWeakRefCount, DecWeakRefCount and GetWeakRefCount
 */
BENCHMARK_F(BenchmarkRefbaseTest, testRefCounter004)(benchmark::State& state)
{
    BENCHMARK_LOGD("RefbaseTest testRefCounter004 start.");
    while (state.KeepRunning()) {
        RefCounter *refs = new RefCounter();
        refs->IncWeakRefCount(this);
        int count = refs->IncWeakRefCount(this);
        AssertEqual(count, EXPECTED_REF_COUNT_ONE,
            "Before IncWeakRefCount, weak count was not EXPECTED_REF_COUNT_ONE", state);
        AssertEqual(refs->GetWeakRefCount(), EXPECTED_REF_COUNT_TWO,
            "refs->GetWeakRefCount() did not equal EXPECTED_REF_COUNT_TWO", state);
        refs->DecWeakRefCount(this);
        count = refs->DecWeakRefCount(this);
        AssertEqual(count, EXPECTED_REF_COUNT_ONE,
            "Before DecWeakRefCount, weak count was not EXPECTED_REF_COUNT_ONE", state);
        AssertEqual(refs->GetWeakRefCount(), EXPECTED_REF_COUNT_ZERO,
            "At the end, refs->GetWeakRefCount() did not equal EXPECTED_REF_COUNT_ZERO", state);
        delete refs;
    }
    BENCHMARK_LOGD("RefbaseTest testRefCounter004 end.");
}

/*
 * @tc.name: testRefCounter005
 * @tc.desc: Test for SetAttemptAcquire, IsAttemptAcquireSet and ClearAttemptAcquire
 */
BENCHMARK_F(BenchmarkRefbaseTest, testRefCounter005)(benchmark::State& state)
{
    BENCHMARK_LOGD("RefbaseTest testRefCounter005 start.");
    while (state.KeepRunning()) {
        RefCounter *refs = new RefCounter();
        refs->SetAttemptAcquire();
        AssertTrue(refs->IsAttemptAcquireSet(), "refs->IsAttemptAcquireSet() did not return true", state);
        refs->ClearAttemptAcquire();
        AssertFalse(refs->IsAttemptAcquireSet(), "refs->IsAttemptAcquireSet() did not return false", state);
        delete refs;
    }
    BENCHMARK_LOGD("RefbaseTest testRefCounter005 end.");
}

/*
 * @tc.name: testRefCounter006
 * @tc.desc: Test for AttemptIncStrongRef
 */
BENCHMARK_F(BenchmarkRefbaseTest, testRefCounter006)(benchmark::State& state)
{
    BENCHMARK_LOGD("RefbaseTest testRefCounter006 start.");
    while (state.KeepRunning()) {
        RefCounter *refs = new RefCounter();
        int count = 0;
        bool result = refs->AttemptIncStrongRef(this, count);
        AssertTrue(result, "refs->AttemptIncStrongRef() did not return true", state);
        AssertEqual(count, INITIAL_PRIMARY_VALUE, "outCount did not equal INITIAL_PRIMARY_VALUE", state);
        AssertEqual(refs->GetStrongRefCount(), EXPECTED_REF_COUNT_ONE,
            "refs->GetStrongRefCount() did not equal EXPECTED_REF_COUNT_ONE", state);
        refs->DecStrongRefCount(this);
        result = refs->AttemptIncStrongRef(this, count);
        AssertFalse(result, "refs->AttemptIncStrongRef() did not return false", state);
        AssertEqual(refs->GetStrongRefCount(), EXPECTED_REF_COUNT_ZERO,
            "refs->GetStrongRefCount() did not equal EXPECTED_REF_COUNT_ZERO", state);
        refs->IncStrongRefCount(this);
        result = refs->AttemptIncStrongRef(this, count);
        AssertTrue(result, "refs->AttemptIncStrongRef() did not return true", state);
        AssertEqual(refs->GetStrongRefCount(), EXPECTED_REF_COUNT_TWO,
            "refs->GetStrongRefCount() did not equal EXPECTED_REF_COUNT_TWO", state);
        delete refs;
    }
    BENCHMARK_LOGD("RefbaseTest testRefCounter006 end.");
}

/*
 * @tc.name: testRefCounter007
 * @tc.desc: Test for AttemptIncStrong
 */
BENCHMARK_F(BenchmarkRefbaseTest, testRefCounter007)(benchmark::State& state)
{
    BENCHMARK_LOGD("RefbaseTest testRefCounter007 start.");
    constexpr int incrementByOne = 1;
    constexpr int incrementByThree = 3;
    while (state.KeepRunning()) {
        RefCounter *refs = new RefCounter();
        bool result = refs->AttemptIncStrong(this);
        AssertTrue(result, "refs->AttemptIncStrong() did not return true", state);
        AssertEqual(refs->GetStrongRefCount(), INITIAL_PRIMARY_VALUE + incrementByOne,
            "refs->GetStrongRefCount() did not equal INITIAL_PRIMARY_VALUE + incrementByOne", state);
        refs->DecStrongRefCount(this);
        result = refs->AttemptIncStrong(this);
        AssertTrue(result, "refs->AttemptIncStrong() did not return true", state);
        AssertEqual(refs->GetStrongRefCount(), INITIAL_PRIMARY_VALUE + incrementByOne,
            "refs->GetStrongRefCount() did not equal INITIAL_PRIMARY_VALUE + incrementByOne", state);
        refs->IncStrongRefCount(this);
        result = refs->AttemptIncStrong(this);
        AssertTrue(result, "refs->AttemptIncStrong() did not return true", state);
        AssertEqual(refs->GetStrongRefCount(), INITIAL_PRIMARY_VALUE + incrementByThree,
            "refs->GetStrongRefCount() did not equal INITIAL_PRIMARY_VALUE + incrementByThree", state);
        delete refs;
    }
    BENCHMARK_LOGD("RefbaseTest testRefCounter007 end.");
}

/*
 * @tc.name: testRefCounter008
 * @tc.desc: Test for IsLifeTimeExtended, ExtendObjectLifetime
 */
BENCHMARK_F(BenchmarkRefbaseTest, testRefCounter008)(benchmark::State& state)
{
    BENCHMARK_LOGD("RefbaseTest testRefCounter008 start.");
    while (state.KeepRunning()) {
        RefCounter *refs = new RefCounter();
        AssertFalse(refs->IsLifeTimeExtended(), "refs->IsLifeTimeExtended() did not return false", state);
        refs->ExtendObjectLifetime();
        AssertTrue(refs->IsLifeTimeExtended(), "refs->IsLifeTimeExtended did not return true", state);
        delete refs;
    }
    BENCHMARK_LOGD("RefbaseTest testRefCounter008 end.");
}
}  // namespace
}  // namespace OHOS
// Run the benchmark
BENCHMARK_MAIN();
