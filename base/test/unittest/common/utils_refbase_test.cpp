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
#include <gtest/gtest.h>
#include <algorithm>
#include <fstream>
#include <iostream>
#include <vector>
#include <thread>
#include <map>
#include <mutex>
#include "refbase.h"
#include "singleton.h"

#include <future>
using namespace testing::ext;
using namespace std;

namespace OHOS {
namespace {
static constexpr int FLAG_OF_CONS = 1;
static constexpr int FLAG_OF_DEST = 2;
static int g_sptrCount = 0;
static int g_wptrCount = 0;
static int g_refbaseflag = 0;
static int g_freeFlag = 0;
static int g_onLastWeakRefFlag = 0;

class UtilsRefbaseTest : public testing::Test {
public:
    static void SetUpTestCase(void);
};

void UtilsRefbaseTest::SetUpTestCase(void)
{
    g_sptrCount = 0;
    g_wptrCount = 0;
    g_refbaseflag = 0;
}

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

    void OnLastWeakRef(const void *objectIda) override
    {
        g_onLastWeakRefFlag = 1;
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
    IRemoteObject() { ExtendObjectLifetime(); }
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
        checkCount_--;
    }

    RefBaseTestTracker(const RefBaseTestTracker &testTracker)
    {
        checkCount_++;
        value_ = testTracker.value_;
    }

    RefBaseTestTracker &operator=(const RefBaseTestTracker &testTracker)
    {
        checkCount_++;
        value_ = testTracker.value_;
        return *this;
    }

    RefBaseTestTracker(RefBaseTestTracker &&testTracker)
    {
        checkCount_++;
        value_ = testTracker.value_;
    }

    RefBaseTestTracker &operator=(RefBaseTestTracker &&testTracker)
    {
        checkCount_++;
        value_ = testTracker.value_;
        return *this;
    }

    static RefBaseTestTracker *GetInstance()
    {
        static RefBaseTestTracker instance;
        return &instance;
    }

    void InitTracker()
    {
        checkCount_ = 0;
        freeCount_ = 0;
        firstRefCount_ = 0;
        lastRefCount_ = 0;
    }

    void TrackObject(IRemoteObject *object)
    {
        std::lock_guard<std::mutex> lockGuard(objectMutex_);
        trackObjects_.emplace_back(object);
    }

    void TrackNewObject(IRemoteObject *object)
    {
        std::lock_guard<std::mutex> lockGuard(objectOnfirstMutex_);
        RefBaseTestTracker::firstRefCount_++;
    }

    void UntrackObject(IRemoteObject *object)
    {
        std::lock_guard<std::mutex> lockGuard(objectMutex_);
        auto iter = find(trackObjects_.begin(), trackObjects_.end(), object);
        if (iter != trackObjects_.end()) {
            trackObjects_.erase(iter);
        }
    }

    void TrackFreeObject(IRemoteObject *object)
    {
        std::lock_guard<std::mutex> lockGuard(objectOnfreeMutex_);
        RefBaseTestTracker::freeCount_++;
    }

    void PrintTrackResults()
    {
        std::lock_guard<std::mutex> lockGuard(objectMutex_);
        if (!trackObjects_.empty()) {
            for (auto o : trackObjects_) {
                std::cout << "object: " << o <<"strong: " << o->GetSptrRefCount() << ", weak:" << o->GetWptrRefCount() << std::endl;
            }
        }
        std::cout << "firstRefCount_: " << RefBaseTestTracker::firstRefCount_ << std::endl;
        std::cout << "lastRefCount_: " << RefBaseTestTracker::lastRefCount_ << std::endl;
        std::cout << "freeCount_: " << RefBaseTestTracker::freeCount_ << std::endl;
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


class IPCObjectProxy : public IRemoteObject
{
public:
    bool IsProxyObject() override { return 0; }
    string descriptor_;
    IPCObjectProxy(const string &descriptor)
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

class IPCProcessSkeleton : public virtual RefBase, public Singleton<IPCProcessSkeleton>
{
    friend class Singleton<IPCProcessSkeleton>;

private:
    IPCProcessSkeleton() = default;

public:
    ~IPCProcessSkeleton() override = default;

    std::mutex mutex_;
    std::map<string, wptr<IRemoteObject>> objects1_;

    void DumpMapObjects()
    {
        if (!objects1_.empty()) {
            for (auto &o : objects1_) {
                std::cout << "strong: " << o.second->GetSptrRefCount() << "weak:" << o.second->GetWptrRefCount() << std::endl;
            }
        }
    }
    IRemoteObject *QueryObjectInner(const string &descriptor)
    {
        auto it = objects1_.find(descriptor);
        if (it != objects1_.end())
        {
            it->second->AttemptAcquire(this);
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
            return it->second.GetRefPtr();
        }

        return nullptr;
    }

    IRemoteObject *FindOrNewObject(int handle)
    {
        std::lock_guard<std::mutex> lockGuard(mutex_);
        IRemoteObject *remoteObject = QueryObjectInner(to_string(handle));
        if (remoteObject != nullptr)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
            return remoteObject;
        }

        remoteObject = new IPCObjectProxy(to_string(handle));
        remoteObject->AttemptAcquire(this);
        objects1_.insert(std::pair<string, wptr<IRemoteObject>>(to_string(handle), remoteObject));
        return remoteObject;
    }

    bool DetachObject(IRemoteObject *object, string descriptor)
    {
        std::lock_guard<std::mutex> lockGuard(mutex_);
        if (object->GetSptrRefCount())
        {
            return false;
        }
        return (objects1_.erase(descriptor) > 0);
    }
};

void IPCObjectProxy::OnLastStrongRef(const void *objectId)
{
    std::lock_guard<std::mutex> lock(mutexLast_);
    (void)IPCProcessSkeleton::GetInstance().DetachObject(this, descriptor_);
    RefBaseTestTracker *tracker = RefBaseTestTracker::GetInstance();
    tracker->lastRefCount_++;
    std::this_thread::sleep_for(std::chrono::nanoseconds(10));
}

void IPCObjectProxy::OnFirstStrongRef(const void *objectId)
{
    std::this_thread::sleep_for(std::chrono::nanoseconds(10));
}

void IPCObjectProxy::RefPtrCallback()
{
    RefBaseTestTracker *tracker = RefBaseTestTracker::GetInstance();
    tracker->UntrackObject(this);
    tracker->TrackFreeObject(this);
    RefBase::RefPtrCallback();
}

constexpr int CYCLE_NUM1 = 100;
constexpr int CYCLE_NUM2 = 100;

int RegisterEventThread()
{
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

/*
 * @tc.name: testRefbaseOperateThreads001
 * @tc.desc: Refbase for threads
 */
HWTEST_F(UtilsRefbaseTest, testRefbaseOperateThreads001, TestSize.Level0)
{
    RefBaseTestTracker *tracker = RefBaseTestTracker::GetInstance();
    tracker->InitTracker();
    for (int n = 0; n < 1; n++) {
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
    EXPECT_EQ(tracker->firstRefCount_, tracker->freeCount_);
}

/*
 * @tc.name: testRefbaseOperateNull001
 * @tc.desc: Refbase for null
 */
HWTEST_F(UtilsRefbaseTest, testRefbaseOperateNull001, TestSize.Level0)
{
    RefBaseTestTracker *tracker = RefBaseTestTracker::GetInstance();
    tracker->InitTracker();

    auto remoteObject = new IPCObjectProxy("ss");
    remoteObject->AttemptAcquire(this);

    remoteObject->IncWeakRef(nullptr);
    remoteObject->IncStrongRef(nullptr);
    remoteObject->CheckIsAttemptAcquireSet(nullptr);
    remoteObject->DecStrongRef(nullptr);
    remoteObject->AttemptAcquire(this);

    remoteObject->IncStrongRef(nullptr);
    remoteObject->CheckIsAttemptAcquireSet(nullptr);
    remoteObject->DecStrongRef(nullptr);

    remoteObject->DecWeakRef(nullptr);
    EXPECT_EQ(tracker->firstRefCount_, tracker->freeCount_);
}

class RefBaseMemTest : public RefBase {
public:
    explicit RefBaseMemTest(int value): value_(value)
    {
        g_checkCount++;
    }

    ~RefBaseMemTest()
    {
        g_checkCount--;
    }

    RefBaseMemTest(const RefBaseMemTest &testRefbaseMem)
    {
        g_checkCount++;
        value_ = testRefbaseMem.value_;
    }

    RefBaseMemTest &operator=(const RefBaseMemTest &testRefbaseMem)
    {
        g_checkCount++;
        value_ = testRefbaseMem.value_;
        return *this;
    }

    RefBaseMemTest(RefBaseMemTest &&testRefbaseMem)
    {
        g_checkCount++;
        value_ = testRefbaseMem.value_;
    }

    RefBaseMemTest &operator=(RefBaseMemTest &&testRefbaseMem)
    {
        g_checkCount++;
        value_ = testRefbaseMem.value_;
        return *this;
    }

public:
    static inline int g_checkCount = 0;
    
private:
    int value_;
};

/*
 * @tc.name: testRefbaseOperateLeftValue001
 * @tc.desc: Refbase
 */
HWTEST_F(UtilsRefbaseTest, testRefbaseOperateLeftValue001, TestSize.Level0)
{
    RefBaseMemTest::g_checkCount = 0;
    {
        vector<RefBaseMemTest> refMemTestArray;
        sptr<RefBaseMemTest>refMemTestObj1 = new RefBaseMemTest(1);
        sptr<RefBaseMemTest>refMemTestObj2 = new RefBaseMemTest(2);
        refMemTestArray.push_back(*refMemTestObj1);
        refMemTestArray.push_back(*refMemTestObj2);
    }
    EXPECT_EQ(RefBaseMemTest::g_checkCount, 0);

    {
        vector<RefBaseMemTest> refMemTestArray;
        RefBaseMemTest refMemTestObj1(1);
        RefBaseMemTest refMemTestObj2(2);
        refMemTestArray.push_back(refMemTestObj1);
        refMemTestArray.push_back(refMemTestObj2);
    }
    EXPECT_EQ(RefBaseMemTest::g_checkCount, 0);
}

/*
 * @tc.name: testRefbaseOperateRightValue001
 * @tc.desc: Refbase
 */
HWTEST_F(UtilsRefbaseTest, testRefbaseOperateRightValue001, TestSize.Level0)
{
    RefBaseMemTest::g_checkCount = 0;
    {
        vector<RefBaseMemTest> refMemTestArray;
        sptr<RefBaseMemTest>refMemTestObj1 = new RefBaseMemTest(1);
        sptr<RefBaseMemTest>refMemTestObj2 = new RefBaseMemTest(2);
        refMemTestArray.emplace_back(*refMemTestObj1);
        refMemTestArray.emplace_back(*refMemTestObj2);
    }
    EXPECT_EQ(RefBaseMemTest::g_checkCount, 0);

    {
        vector<RefBaseMemTest> refMemTestArray;
        RefBaseMemTest refMemTestObj1(1);
        RefBaseMemTest refMemTestObj2(2);
        refMemTestArray.emplace_back(refMemTestObj1);
        refMemTestArray.emplace_back(refMemTestObj2);
    }
    EXPECT_EQ(RefBaseMemTest::g_checkCount, 0);
}

/*
 * @tc.name: testRefbaseAcquire001
 * @tc.desc: Refbase
 */
HWTEST_F(UtilsRefbaseTest, testRefbaseAcquire001, TestSize.Level0)
{
    RefBaseTest* testobject = new RefBaseTest();
    testobject->AttemptAcquire(this);

    g_freeFlag = 0;
    EXPECT_EQ(testobject->GetSptrRefCount(), 1);
    {
        EXPECT_TRUE(testobject->IsAttemptAcquireSet());
        testobject->CheckIsAttemptAcquireSet(this);
        sptr<RefBaseTest> sptrRef = testobject;
        EXPECT_EQ(sptrRef->GetSptrRefCount(), 1);
        EXPECT_FALSE(testobject->IsAttemptAcquireSet());
    }

    EXPECT_EQ(g_freeFlag, 1);
}

/*
 * @tc.name: testRefbaseOnLastWeakRef001
 * @tc.desc: Refbase
 */
HWTEST_F(UtilsRefbaseTest, testRefbaseOnLastWeakRef001, TestSize.Level0)
{
    RefBaseTest* testobject = new RefBaseTest();
    g_onLastWeakRefFlag = 0;

    testobject->OnLastWeakRef(this);
    EXPECT_EQ(g_onLastWeakRefFlag, 1);
    delete testobject;
}

/*
 * @tc.name: testRefbaseOnLastWeakRef002
 * @tc.desc: Refbase
 */
HWTEST_F(UtilsRefbaseTest, testRefbaseOnLastWeakRef002, TestSize.Level0)
{
    RefBase* testobject = new RefBase();
    g_onLastWeakRefFlag = 0;

    testobject->OnLastWeakRef(this);
    EXPECT_EQ(g_onLastWeakRefFlag, 0);
    delete testobject;
}

/*
 * @tc.name: testSptrefbase001
 * @tc.desc: Refbase
 */
HWTEST_F(UtilsRefbaseTest, testSptrefbase001, TestSize.Level0)
{
    sptr<RefBaseTest> testobject = new RefBaseTest();
    testobject->ExtendObjectLifetime();
    EXPECT_TRUE(testobject->IsExtendLifeTimeSet());
    EXPECT_EQ(g_refbaseflag, 1);
    wptr<RefBaseTest> weakObject(testobject);
    int count = testobject->GetWptrRefCount();
    EXPECT_EQ(count, 2);
    testobject = nullptr;

    sptr<RefBaseTest> strongObject = weakObject.promote();
    EXPECT_EQ(strongObject->GetSptrRefCount(), 1);
}

/*
 * @tc.name: testSptrefbaseRealease001
 * @tc.desc: Refbase
 */
HWTEST_F(UtilsRefbaseTest, testSptrefbaseRealease001, TestSize.Level0)
{
    sptr<RefBaseTest> testObject = new RefBaseTest();
    EXPECT_EQ(g_refbaseflag, 1);
    wptr<RefBaseTest> weakObject(testObject);
    testObject = nullptr;
    EXPECT_EQ(g_refbaseflag, FLAG_OF_DEST);
}

/*
 * @tc.name: testSptrefbaseRealease002
 * @tc.desc: Refbase
 */
HWTEST_F(UtilsRefbaseTest, testSptrefbaseRealease002, TestSize.Level0)
{
    wptr<RefBaseTest> testObject = new RefBaseTest();
    EXPECT_EQ(g_refbaseflag, 1);
    testObject = nullptr;
    EXPECT_EQ(g_refbaseflag, FLAG_OF_DEST);
}

/*
 * @tc.name: testSptrefbase002
 * @tc.desc: Refbase
 */
HWTEST_F(UtilsRefbaseTest, testSptrefbase002, TestSize.Level0)
{
    {
        sptr<RefBaseTest> testObject(new RefBaseTest());
        EXPECT_EQ(g_refbaseflag, 1);
    }
    EXPECT_EQ(g_refbaseflag, 2);
}

/*
 * @tc.name: testSptrefbase003
 * @tc.desc: Refbase
 */
HWTEST_F(UtilsRefbaseTest, testSptrefbase003, TestSize.Level0)
{
    sptr<RefBaseTest> testObject1(new RefBaseTest());
    sptr<RefBaseTest> testObject2 = testObject1.GetRefPtr();
    testObject2->SetRefPtr();
    EXPECT_TRUE(testObject1->GetTestRefPtrFlag());

    sptr<RefBaseTest> testObject3(testObject1);
    EXPECT_TRUE(testObject3->GetTestRefPtrFlag());

    sptr<RefBaseTest> testObject4 = testObject1;
    EXPECT_TRUE(testObject3->GetTestRefPtrFlag());

    bool ret = (testObject3 == testObject4);
    EXPECT_TRUE(ret);

    int refcount = testObject1->GetSptrRefCount();
    EXPECT_EQ(refcount, 4);

    sptr<RefBaseTest> testObject5(new RefBaseTest());
    ret = (testObject5 != testObject1);
    EXPECT_TRUE(ret);
}

/*
 * @tc.name: testSptrefbase004
 * @tc.desc: Refbase
 */
HWTEST_F(UtilsRefbaseTest, testSptrefbase004, TestSize.Level0)
{
    sptr<RefBaseTest> testObject1(new RefBaseTest());
    testObject1->SetRefPtr();
    RefBaseTest testObject2 = *testObject1;
    EXPECT_TRUE(testObject2.GetTestRefPtrFlag());

    auto testObject3 = testObject1;
    testObject1 = nullptr;
    testObject3 = nullptr;
    EXPECT_EQ(g_refbaseflag, 2);
}

/*
 * @tc.name: testSptrefbase005
 * @tc.desc: Refbase
 */
HWTEST_F(UtilsRefbaseTest, testSptrefbase005, TestSize.Level0)
{
    sptr<RefBaseTest> testObject1(new RefBaseTest());
    wptr<RefBaseTest> testObject2 = testObject1;
    EXPECT_EQ(testObject1->GetSptrRefCount(), 1);
    EXPECT_EQ(testObject1->GetWptrRefCount(), 2);
}

/*
 * @tc.name: testSptrefbase006
 * @tc.desc: Refbase
 */
HWTEST_F(UtilsRefbaseTest, testSptrefbase006, TestSize.Level0)
{
    sptr<RefBaseTest> testObject1;
    EXPECT_EQ(testObject1.GetRefPtr(), nullptr);
    testObject1 = new RefBaseTest();
    sptr<RefBaseTest> testObject2(testObject1);
    EXPECT_EQ(testObject1->GetSptrRefCount(), 2);
}

/*
 * @tc.name: testSptrefbase007
 * @tc.desc: Refbase
 */
HWTEST_F(UtilsRefbaseTest, testSptrefbase007, TestSize.Level0)
{
    const sptr<RefBaseTest> &testObject1 = new RefBaseTest();
    sptr<RefBaseTest> testObject2(testObject1);
    EXPECT_EQ(testObject1->GetSptrRefCount(), 2);
}

/*
 * @tc.name: testSptrefbase008
 * @tc.desc: Refbase
 */
HWTEST_F(UtilsRefbaseTest, testSptrefbase008, TestSize.Level0)
{
    sptr<RefBaseTest> testObject1;
    sptr<RefBaseTest> testObject2(testObject1);
    EXPECT_EQ(testObject2, nullptr);
}

/*
 * @tc.name: testSptrefbase009
 * @tc.desc: Refbase
 */
HWTEST_F(UtilsRefbaseTest, testSptrefbase009, TestSize.Level0)
{
    sptr<RefBaseTest> testObject0 = new RefBaseTest();
    sptr<RefBaseTest> testObject1 = move(testObject0);
    sptr<RefBaseTest> testObject2(testObject1);
    EXPECT_EQ(testObject0.GetRefPtr(), nullptr);
    EXPECT_EQ(testObject2.GetRefPtr(), testObject1.GetRefPtr());
}

/*
 * @tc.name: testSptrefbase010
 * @tc.desc: Refbase
 */
HWTEST_F(UtilsRefbaseTest, testSptrefbase010, TestSize.Level0)
{
    sptr<RefBaseTest> testObject1 = new RefBaseTest();
    sptr<RefBaseTest> testObject3(new RefBaseTest());
    sptr<RefBaseTest> &testObject2 = testObject3;
    testObject2 = testObject1;
    EXPECT_EQ(testObject2.GetRefPtr(), testObject1.GetRefPtr());

    const sptr<RefBaseTest> &testObject4 = new RefBaseTest();
    EXPECT_EQ(testObject1->GetSptrRefCount(), 2);
    testObject2 = testObject4;
    EXPECT_EQ(testObject2.GetRefPtr(), testObject4.GetRefPtr());
    EXPECT_EQ(testObject4->GetSptrRefCount(), 2);
    EXPECT_EQ(testObject1->GetSptrRefCount(), 1);
}

/*
 * @tc.name: testSptrefbase011
 * @tc.desc: Refbase
 */
HWTEST_F(UtilsRefbaseTest, testSptrefbase011, TestSize.Level0)
{
    sptr<RefBaseTest> testobject = sptr<RefBaseTest>::MakeSptr();
    testobject->ExtendObjectLifetime();
    EXPECT_TRUE(testobject->IsExtendLifeTimeSet());
    EXPECT_EQ(g_refbaseflag, 1);
    wptr<RefBaseTest> weakObject(testobject);
    int count = testobject->GetWptrRefCount();
    EXPECT_EQ(count, 2);
    testobject = nullptr;

    sptr<RefBaseTest> strongObject = weakObject.promote();
    EXPECT_EQ(strongObject->GetSptrRefCount(), 1);
}

/*
 * @tc.name: testSptrefbase012
 * @tc.desc: Refbase
 */
HWTEST_F(UtilsRefbaseTest, testSptrefbase012, TestSize.Level0)
{
    // test clear
    sptr<RefBaseTest> testObject1 = new RefBaseTest();
    testObject1.clear();
    ASSERT_EQ(testObject1.GetRefPtr(), nullptr);
}

/*
 * @tc.name: testSptrefbase013
 * @tc.desc: Refbase
 */
HWTEST_F(UtilsRefbaseTest, testSptrefbase013, TestSize.Level0)
{
    sptr<RefBaseTest> testObject1;
    wptr<RefBaseTest> testObject2 = new RefBaseTest();
    testObject1 = testObject2;
    ASSERT_EQ(testObject2->GetWptrRefCount(), 2);
    ASSERT_EQ(testObject1->GetSptrRefCount(), 1);
}

/*
 * @tc.name: testSptrefbase014
 * @tc.desc: Refbase
 */
HWTEST_F(UtilsRefbaseTest, testSptrefbase014, TestSize.Level0)
{
    sptr<RefBaseTest> testObject1(new RefBaseTest());
    const RefBaseTest *rawPointer = testObject1.GetRefPtr();
    ASSERT_TRUE(testObject1 == rawPointer);

    wptr<RefBaseTest> testObject2(new RefBaseTest());
    ASSERT_FALSE(testObject1 == testObject2);
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

/*
 * @tc.name: testRefbase005
 * @tc.desc: Refbase
 */
HWTEST_F(UtilsRefbaseTest, testRefbase005, TestSize.Level0)
{
    {
        sptr<SptrTest> testObject1(new SptrTest());
        testObject1->CreateSptr();
    }
    EXPECT_EQ(g_sptrCount, 0);
}

class SptrTest1;
class SptrTest2;
class SptrTest2 : public RefBase {
public:
    SptrTest2()
    {
        g_sptrCount++;
    }
    ~SptrTest2()
    {
        g_sptrCount--;
    }

private:
    sptr<SptrTest1> test;
};

class SptrTest1 : public RefBase {
public:
    SptrTest1()
    {
        g_sptrCount++;
    }
    ~SptrTest1()
    {
        g_sptrCount--;
    }

private:
    sptr<SptrTest2> test;
};

/*
 * @tc.name: testRefbase006
 * @tc.desc: Refbase
 */
HWTEST_F(UtilsRefbaseTest, testRefbase006, TestSize.Level0)
{
    {
        sptr<SptrTest1> testObject1(new SptrTest1());
        sptr<SptrTest2> testObject2(new SptrTest2());
        EXPECT_EQ(g_sptrCount, 2);
    }
    EXPECT_EQ(g_sptrCount, 0);
}

/*
 * @tc.name: testRefbase007
 * @tc.desc: test count of refcounter.
 */
HWTEST_F(UtilsRefbaseTest, testRefbase007, TestSize.Level0)
{
    sptr<RefBase> testObject1(new RefBase());
    EXPECT_EQ(testObject1->GetRefCounter()->GetRefCount(), 1);
    wptr<RefBase> testObject2(testObject1);
    EXPECT_EQ(testObject1->GetRefCounter()->GetRefCount(), 2); // 2: Refbase and WeakRefCounter
}

/*
 * @tc.name: testRefbase008
 * @tc.desc: test move constructor.
 */
HWTEST_F(UtilsRefbaseTest, testRefbase008, TestSize.Level0)
{
    RefBase baseObject1{};
    EXPECT_EQ(baseObject1.GetRefCounter()->GetRefCount(), 1);

    RefBase baseObject2{};
    EXPECT_EQ(baseObject2.GetRefCounter()->GetRefCount(), 1);
    baseObject2 = std::move(baseObject1);
    EXPECT_EQ(baseObject2.GetRefCounter()->GetRefCount(), 1);
    EXPECT_EQ(baseObject1.GetRefCounter(), nullptr);
    EXPECT_EQ(baseObject1.GetSptrRefCount(), 0);
    EXPECT_EQ(baseObject1.GetWptrRefCount(), 0);

    RefBase baseObject3{};
    EXPECT_EQ(baseObject3.GetRefCounter()->GetRefCount(), 1);
    baseObject3 = std::move(baseObject2);
    EXPECT_EQ(baseObject3.GetRefCounter()->GetRefCount(), 1);
    EXPECT_EQ(baseObject2.GetRefCounter(), nullptr);
    EXPECT_EQ(baseObject2.GetSptrRefCount(), 0);
    EXPECT_EQ(baseObject2.GetWptrRefCount(), 0);

    baseObject2 = std::move(baseObject1);
    EXPECT_EQ(baseObject1.GetRefCounter(), baseObject2.GetRefCounter());
}

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

/*
 * @tc.name: testWptrefbase001
 * @tc.desc: Copy constructor with same managed class type.
 */
HWTEST_F(UtilsRefbaseTest, testWptrefbase001, TestSize.Level0)
{
    // test wptr<T>::wptr(const wptr<T>&)
    wptr<WptrTest> testOrigWptrObject(new WptrTest());
    EXPECT_EQ(testOrigWptrObject->GetWptrRefCount(), 1);

    wptr<WptrTest> testTargetWptrObject1(testOrigWptrObject);

    EXPECT_EQ(testOrigWptrObject.GetRefPtr(), testTargetWptrObject1.GetRefPtr());
    EXPECT_EQ(&(*testOrigWptrObject), &(*testTargetWptrObject1));
    EXPECT_EQ(testTargetWptrObject1->GetWptrRefCount(), testOrigWptrObject->GetWptrRefCount());
    EXPECT_EQ(testTargetWptrObject1.GetWeakRefCount(), testOrigWptrObject.GetWeakRefCount());

    EXPECT_EQ(testTargetWptrObject1->GetWptrRefCount(), 1);
    EXPECT_EQ(testTargetWptrObject1.GetWeakRefCount(), 2);

    // test wptr<T>::operator=(const wptr<T>&)
    wptr<WptrTest> testTargetWptrObject2(new WptrTest());
    EXPECT_EQ(testTargetWptrObject2->GetWptrRefCount(), 1);

    testTargetWptrObject2 = testOrigWptrObject;

    EXPECT_EQ(testOrigWptrObject.GetRefPtr(), testTargetWptrObject2.GetRefPtr());
    EXPECT_EQ(&(*testOrigWptrObject), &(*testTargetWptrObject2));
    EXPECT_EQ(testTargetWptrObject2->GetWptrRefCount(), testOrigWptrObject->GetWptrRefCount());
    EXPECT_EQ(testTargetWptrObject2.GetWeakRefCount(), testOrigWptrObject.GetWeakRefCount());

    EXPECT_EQ(testTargetWptrObject2->GetWptrRefCount(), 1);
    EXPECT_EQ(testTargetWptrObject2.GetWeakRefCount(), 3);

    // test wptr<T>::wptr(const sptr<T>&)
    sptr<WptrTest> testOrigSptrObject(new WptrTest());
    EXPECT_EQ(testOrigSptrObject->GetSptrRefCount(), 1);

    wptr<WptrTest> testTargetWptrObject3(testOrigSptrObject);

    EXPECT_EQ(testOrigSptrObject.GetRefPtr(), testTargetWptrObject3.GetRefPtr());
    EXPECT_EQ(&(*testOrigSptrObject), &(*testTargetWptrObject3));
    EXPECT_EQ(testTargetWptrObject3->GetSptrRefCount(), testOrigSptrObject->GetSptrRefCount());
    EXPECT_EQ(testTargetWptrObject3->GetWptrRefCount(), testOrigSptrObject->GetWptrRefCount());

    EXPECT_EQ(testTargetWptrObject3->GetSptrRefCount(), 1);
    EXPECT_EQ(testTargetWptrObject3->GetWptrRefCount(), 2);
    EXPECT_EQ(testTargetWptrObject3.GetWeakRefCount(), 1);

    // test wptr<T>::operator=(const sptr<T>&)
    wptr<WptrTest> testTargetWptrObject4(new WptrTest());
    EXPECT_EQ(testTargetWptrObject4->GetWptrRefCount(), 1);

    testTargetWptrObject4 = testOrigSptrObject;

    EXPECT_EQ(testOrigSptrObject.GetRefPtr(), testTargetWptrObject4.GetRefPtr());
    EXPECT_EQ(&(*testOrigSptrObject), &(*testTargetWptrObject4));
    EXPECT_EQ(testTargetWptrObject4->GetSptrRefCount(), testOrigSptrObject->GetSptrRefCount());
    EXPECT_EQ(testTargetWptrObject4->GetWptrRefCount(), testOrigSptrObject->GetWptrRefCount());

    EXPECT_EQ(testTargetWptrObject4->GetSptrRefCount(), 1);
    EXPECT_EQ(testTargetWptrObject4->GetWptrRefCount(), 3);
    EXPECT_EQ(testTargetWptrObject4.GetWeakRefCount(), 1);
}

/*
 * @tc.name: testWptrefbase002
 * @tc.desc: Copy constructor with different managed class type.
 */
HWTEST_F(UtilsRefbaseTest, testWptrefbase002, TestSize.Level0)
{
    // test wptr<T>::wptr(const wptr<O>&)
    wptr<WptrTest2> testOrigWptrObject(new WptrTest2());
    EXPECT_EQ(testOrigWptrObject->GetWptrRefCount(), 1);

    wptr<WptrTest> testTargetWptrObject1(testOrigWptrObject);

    EXPECT_EQ(static_cast<void *>(testOrigWptrObject.GetRefPtr()),
              static_cast<void *>(testTargetWptrObject1.GetRefPtr()));
    EXPECT_EQ(static_cast<void *>(&(*testOrigWptrObject)),
              static_cast<void *>(&(*testTargetWptrObject1)));
    EXPECT_EQ(testTargetWptrObject1->GetWptrRefCount(), testOrigWptrObject->GetWptrRefCount());
    EXPECT_EQ(testTargetWptrObject1.GetWeakRefCount(), testOrigWptrObject.GetWeakRefCount());

    EXPECT_EQ(testTargetWptrObject1->GetWptrRefCount(), 1);
    EXPECT_EQ(testTargetWptrObject1.GetWeakRefCount(), 2);

    // test wptr<T>::operator=(const wptr<O>&)
    wptr<WptrTest> testTargetWptrObject2(new WptrTest());
    EXPECT_EQ(testTargetWptrObject2->GetWptrRefCount(), 1);

    testTargetWptrObject2 = testOrigWptrObject;

    EXPECT_EQ(static_cast<void *>(testOrigWptrObject.GetRefPtr()),
              static_cast<void *>(testTargetWptrObject2.GetRefPtr()));
    EXPECT_EQ(static_cast<void *>(&(*testOrigWptrObject)),
              static_cast<void *>(&(*testTargetWptrObject2)));
    EXPECT_EQ(testTargetWptrObject2->GetWptrRefCount(), testOrigWptrObject->GetWptrRefCount());
    EXPECT_EQ(testTargetWptrObject2.GetWeakRefCount(), testOrigWptrObject.GetWeakRefCount());

    EXPECT_EQ(testTargetWptrObject2->GetWptrRefCount(), 1);
    EXPECT_EQ(testTargetWptrObject2.GetWeakRefCount(), 3);

    // test wptr<T>::wptr(const sptr<O>&)
    sptr<WptrTest2> testOrigSptrObject(new WptrTest2());
    EXPECT_EQ(testOrigSptrObject->GetSptrRefCount(), 1);

    wptr<WptrTest> testTargetWptrObject3(testOrigSptrObject);

    EXPECT_EQ(static_cast<void *>(testOrigSptrObject.GetRefPtr()),
              static_cast<void *>(testTargetWptrObject3.GetRefPtr()));
    EXPECT_EQ(static_cast<void *>(&(*testOrigSptrObject)), static_cast<void *>(&(*testTargetWptrObject3)));
    EXPECT_EQ(testTargetWptrObject3->GetSptrRefCount(), testOrigSptrObject->GetSptrRefCount());
    EXPECT_EQ(testTargetWptrObject3->GetWptrRefCount(), testOrigSptrObject->GetWptrRefCount());

    EXPECT_EQ(testTargetWptrObject3->GetSptrRefCount(), 1);
    EXPECT_EQ(testTargetWptrObject3->GetWptrRefCount(), 2);
    EXPECT_EQ(testTargetWptrObject3.GetWeakRefCount(), 1);

    // test wptr<T>::operator=(const sptr<O>&)
    wptr<WptrTest> testTargetWptrObject4(new WptrTest());
    EXPECT_EQ(testTargetWptrObject4->GetWptrRefCount(), 1);

    testTargetWptrObject4 = testOrigSptrObject;

    EXPECT_EQ(static_cast<void *>(testOrigSptrObject.GetRefPtr()),
              static_cast<void *>(testTargetWptrObject4.GetRefPtr()));
    EXPECT_EQ(static_cast<void *>(&(*testOrigSptrObject)), static_cast<void *>(&(*testTargetWptrObject4)));
    EXPECT_EQ(testTargetWptrObject4->GetSptrRefCount(), testOrigSptrObject->GetSptrRefCount());
    EXPECT_EQ(testTargetWptrObject4->GetWptrRefCount(), testOrigSptrObject->GetWptrRefCount());

    EXPECT_EQ(testTargetWptrObject4->GetSptrRefCount(), 1);
    EXPECT_EQ(testTargetWptrObject4->GetWptrRefCount(), 3);
    EXPECT_EQ(testTargetWptrObject4.GetWeakRefCount(), 1);
}

/*
 * @tc.name: testWptrefbase003
 * @tc.desc: Refbase
 */
HWTEST_F(UtilsRefbaseTest, testWptrefbase003, TestSize.Level0)
{
    const wptr<WptrTest> &testObject1(new WptrTest());
    wptr<WptrTest> testObject2(testObject1);
    EXPECT_EQ(testObject1.GetRefPtr(), testObject2.GetRefPtr());
    EXPECT_EQ(testObject1->GetWptrRefCount(), 1);
    EXPECT_EQ(testObject2->GetWptrRefCount(), 1);
    EXPECT_EQ(testObject1.GetRefPtr(), testObject2.GetRefPtr());
}

/*
 * @tc.name: testWptrefbase004
 * @tc.desc: Refbase
 */
HWTEST_F(UtilsRefbaseTest, testWptrefbase004, TestSize.Level0)
{
    const sptr<WptrTest2> &testObject1(new WptrTest2());
    EXPECT_NE(testObject1, nullptr);
    wptr<WptrTest> testObject2 = testObject1;
    EXPECT_EQ(testObject1->GetWptrRefCount(), 2);
}

/*
 * @tc.name: testWptrefbase005
 * @tc.desc: wptr without managed object
 */
HWTEST_F(UtilsRefbaseTest, testWptrefbase005, TestSize.Level0)
{
    wptr<WptrTest> testObject3;
    EXPECT_EQ(testObject3.GetRefPtr(), nullptr);
}

/*
 * @tc.name: testWptrefbase006
 * @tc.desc: Refbase
 */
HWTEST_F(UtilsRefbaseTest, testWptrefbase006, TestSize.Level0)
{
    wptr<WptrTest> testObject1 = new WptrTest();
    wptr<WptrTest> &testObject2 = testObject1;
    EXPECT_EQ(testObject2->GetWptrRefCount(), 1);
}

/*
 * @tc.name: testWptrefbase007
 * @tc.desc: Refbase
 */
HWTEST_F(UtilsRefbaseTest, testWptrefbase007, TestSize.Level0)
{
    wptr<WptrTest2> testObject1 = new WptrTest2();
    wptr<WptrTest2> testObject2 = testObject1.GetRefPtr();
    EXPECT_EQ(testObject1->GetWptrRefCount(), 2);
}

/*
 * @tc.name: testWptrefbase008
 * @tc.desc: Refbase
 */
HWTEST_F(UtilsRefbaseTest, testWptrefbase008, TestSize.Level0)
{
    wptr<WptrTest> testObject1 = new WptrTest();
    wptr<WptrTest2> testObject2;
    testObject2 = testObject1.GetRefPtr();
    EXPECT_EQ(testObject1->GetWptrRefCount(), 2);
}

/*
 * @tc.name: testWptrefbase008
 * @tc.desc: Refbase
 */
HWTEST_F(UtilsRefbaseTest, testWptrefbase009, TestSize.Level0)
{
    // test bool operator==(const T *)
    wptr<WptrTest> testObject1 = new WptrTest();
    const WptrTest *rawPointer = testObject1.GetRefPtr();
    ASSERT_TRUE(testObject1 == rawPointer);

    // test bool operator==(const wptr &)
    wptr<WptrTest> testObject2 = testObject1;
    ASSERT_TRUE(testObject2 == testObject1);

    // test operator==(const sptr &)
    sptr<WptrTest> testObject3 = new WptrTest();
    ASSERT_FALSE(testObject2 == testObject3);
}

/*
 * @tc.name: testSptrWptrefbase001
 * @tc.desc: test interaction between sptr and wptr.
 */
HWTEST_F(UtilsRefbaseTest, testSptrWptrefbase001, TestSize.Level0)
{
    wptr<RefBase> testObject1(new RefBase());
    EXPECT_EQ(testObject1->GetWptrRefCount(), 1);
    {
        sptr<RefBase> testObject2{};
        testObject2 = testObject1;
        EXPECT_EQ(testObject2->GetSptrRefCount(), 1);
        EXPECT_EQ(testObject2->GetWptrRefCount(), 2); // 2: sptr and WeakRefCounter

        sptr<RefBase> testObject3 = testObject1.promote();
        EXPECT_EQ(testObject2->GetSptrRefCount(), 2); // 2: 2 sptrs
        EXPECT_EQ(testObject2->GetWptrRefCount(), 3); // 3: 2 sptrs and WeakRefCounter
        testObject2->ExtendObjectLifetime();
    }
    EXPECT_EQ(testObject1->GetWptrRefCount(), 1);
}

/*
 * @tc.name: testRefbaseDebug001
 * @tc.desc: Test for single thread. Tracker can be enabled after construction
 *           of sptr.
 */
HWTEST_F(UtilsRefbaseTest, testRefbaseDebug001, TestSize.Level1)
{
    sptr<RefBase> testObject1(new RefBase());
    testObject1->EnableTracker();
    sptr<RefBase> testObject2(testObject1);
    EXPECT_EQ(testObject2->GetSptrRefCount(), 2);
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    wptr<RefBase> testObject3(testObject2);
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    wptr<RefBase> testObject4(testObject3);
    EXPECT_EQ(testObject4->GetWptrRefCount(), 3);
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
}

/*
 * @tc.name: testRefbaseDebug002
 * @tc.desc: Test for single thread. Tracker can be enabled after construction
 *           of wptr.
 */
HWTEST_F(UtilsRefbaseTest, testRefbaseDebug002, TestSize.Level1)
{
    wptr<RefBase> testObject1(new RefBase());
    testObject1->EnableTracker();
    sptr<RefBase> testObject2 = testObject1.promote();
    EXPECT_EQ(testObject2->GetSptrRefCount(), 1);
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    wptr<RefBase> testObject3(testObject2);
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    wptr<RefBase> testObject4(testObject3);
    EXPECT_EQ(testObject4->GetWptrRefCount(), 3);
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
}

/*
 * @tc.name: testRefbaseWithDomainIdDebug001
 * @tc.desc: Test for single thread. Tracker can be enabled after construction
 *           of sptr.
 */
HWTEST_F(UtilsRefbaseTest, testRefbaseWithDomainIdDebug001, TestSize.Level1)
{
    sptr<RefBase> testObject1(new RefBase());
    testObject1->EnableTrackerWithDomainId(0xD001651);
    sptr<RefBase> testObject2(testObject1);
    EXPECT_EQ(testObject2->GetSptrRefCount(), 2);
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    wptr<RefBase> testObject3(testObject2);
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    wptr<RefBase> testObject4(testObject3);
    EXPECT_EQ(testObject4->GetWptrRefCount(), 3);
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
}

/*
 * @tc.name: testRefbaseWithDomainIdDebug002
 * @tc.desc: Test for single thread. Tracker can be enabled after construction
 *           of wptr.
 */
HWTEST_F(UtilsRefbaseTest, testRefbaseWithDomainIdDebug002, TestSize.Level1)
{
    wptr<RefBase> testObject1(new RefBase());
    testObject1->EnableTrackerWithDomainId(0xD001651);
    sptr<RefBase> testObject2 = testObject1.promote();
    EXPECT_EQ(testObject2->GetSptrRefCount(), 1);
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    wptr<RefBase> testObject3(testObject2);
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    wptr<RefBase> testObject4(testObject3);
    EXPECT_EQ(testObject4->GetWptrRefCount(), 3);
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
}

// This is a class which can be tracked when implemented.
class TestDebug : public RefBase {
public:
    TestDebug()
    {
        EnableTracker();
    }
};

class TestDebugWithDomainId : public RefBase {
public:
    TestDebugWithDomainId()
    {
        EnableTrackerWithDomainId(0xD001651);
    }
};

/*
 * @tc.name: testRefbaseDebug003
 * @tc.desc: Test for single thread. Tracker can be enabled with construction
 *           of sptr.
 */
HWTEST_F(UtilsRefbaseTest, testRefbaseDebug003, TestSize.Level1)
{
    sptr<TestDebug> testObject1(new TestDebug());
    sptr<TestDebug> testObject2(testObject1);
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    sptr<TestDebug> testObject3;
    EXPECT_EQ(testObject2->GetSptrRefCount(), 2);
    testObject3 = testObject2;
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    wptr<TestDebug> testObject4(testObject3);
    EXPECT_EQ(testObject4->GetWptrRefCount(), 4);
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
}

/*
 * @tc.name: testRefbaseDebug004
 * @tc.desc: Test for mult-thread.
 */
HWTEST_F(UtilsRefbaseTest, testRefbaseDebug004, TestSize.Level1)
{
    sptr<TestDebug> testObject1(new TestDebug());
    std::thread subThread {[&testObject1]() {
        sptr<TestDebug> subTestObject1(testObject1);
        EXPECT_EQ(testObject1->GetSptrRefCount(), 2);
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        wptr<TestDebug> subTestObject2(subTestObject1);
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        wptr<TestDebug> subTestObject3(subTestObject2);
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    }};
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    wptr<TestDebug> testObject2(testObject1);
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    wptr<TestDebug> testObject3(testObject2);
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    subThread.join();
    EXPECT_EQ(testObject3->GetWptrRefCount(), 2);
}

/*
 * @tc.name: testRefbaseWithDomainIdDebug003
 * @tc.desc: Test for single thread. Tracker can be enabled with construction
 *           of sptr.
 */
HWTEST_F(UtilsRefbaseTest, testRefbaseWithDomainIdDebug003, TestSize.Level1)
{
    sptr<TestDebugWithDomainId> testObject1(new TestDebugWithDomainId());
    sptr<TestDebugWithDomainId> testObject2(testObject1);
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    sptr<TestDebugWithDomainId> testObject3;
    EXPECT_EQ(testObject2->GetSptrRefCount(), 2);
    testObject3 = testObject2;
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    wptr<TestDebugWithDomainId> testObject4(testObject3);
    EXPECT_EQ(testObject4->GetWptrRefCount(), 4);
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
}

/*
 * @tc.name: testRefbaseWithDomainIdDebug004
 * @tc.desc: Test for mult-thread.
 */
HWTEST_F(UtilsRefbaseTest, testRefbaseWithDomainIdDebug004, TestSize.Level1)
{
    sptr<TestDebugWithDomainId> testObject1(new TestDebugWithDomainId());
    std::thread subThread {[&testObject1]() {
        sptr<TestDebugWithDomainId> subTestObject1(testObject1);
        EXPECT_EQ(testObject1->GetSptrRefCount(), 2);
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        wptr<TestDebugWithDomainId> subTestObject2(subTestObject1);
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        wptr<TestDebugWithDomainId> subTestObject3(subTestObject2);
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    }};
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    wptr<TestDebugWithDomainId> testObject2(testObject1);
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    wptr<TestDebugWithDomainId> testObject3(testObject2);
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    subThread.join();
    EXPECT_EQ(testObject3->GetWptrRefCount(), 2);
}

/*
 * @tc.name: testRefCounter001
 * @tc.desc: Test for RefCounter.
 */
HWTEST_F(UtilsRefbaseTest, testRefCounter001, TestSize.Level1)
{
    RefCounter *refCounterTest = new RefCounter();
#ifdef OHOS_PLATFORM
    refCounterTest->RemoveCanPromote();
    bool result = refCounterTest->IsCanPromoteValid();
    EXPECT_EQ(result, false);
#endif
    delete refCounterTest;
}
}  // namespace
}  // namespace OHOS
