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
#include "sorted_vector.h"
#include <iostream>
#include "benchmark_log.h"
#include "benchmark_assert.h"
using namespace std;

namespace OHOS {
namespace {

static constexpr ssize_t SVECTOR_START_POS_ASSIGN = 0;
static constexpr ssize_t SVECTOR_END_POS_ASSIGN = 9;
static constexpr ssize_t SVECTOR_START_POS_VALUE = 0;
static constexpr ssize_t SVECTOR_END_POS_VALUE = 10;
static constexpr ssize_t SVECTOR_INVALID_VALUE = -1;
const int VECTOR_SIZE_TEN = 10;
const int VECTOR_SIZE_TWENTY = 20;
const int ELEMENT_END_VALUE = 30;
const size_t EXPECTED_SIZE_AFTER_DUPLICATES = 30;

class BenchmarkSortedVector : public benchmark::Fixture {
public:
    void SetUp(const ::benchmark::State& state) override
    {
    }

    void TearDown(const ::benchmark::State& state) override
    {
    }

    BenchmarkSortedVector()
    {
        Iterations(iterations);
        Repetitions(repetitions);
        ReportAggregatesOnly();
    }

    ~BenchmarkSortedVector() override = default;

protected:
    const int32_t repetitions = 3;
    const int32_t iterations = 1000;
};

BENCHMARK_F(BenchmarkSortedVector, testDefaultConsAndAddAndSort)(benchmark::State& state)
{
    BENCHMARK_LOGD("SortedVector testDefaultConsAndAddAndSort start.");
    while (state.KeepRunning()) {
        SortedVector<int> svec;
        std::vector<int> vec;
        for (int i = 0; i < VECTOR_SIZE_TEN; i++) {
            vec.push_back(i);
        }

        for (int i = SVECTOR_END_POS_ASSIGN; i >= SVECTOR_START_POS_ASSIGN; i--) {
            svec.Add(i);
        }

        auto it = svec.Begin();
        auto it2 = vec.begin();
        for (; (it != svec.End()) and (it2 != vec.end()); it2++, it++) {
            AssertEqual(*it, *it2, "*it did not equal *it2 as expected.", state);
        }
    }
    BENCHMARK_LOGD("SortedVector testDefaultConsAndAddAndSort end.");
}

BENCHMARK_F(BenchmarkSortedVector, testConsFromSortedAllowDup)(benchmark::State& state)
{
    BENCHMARK_LOGD("SortedVector testConsFromSortedAllowDup start.");
    while (state.KeepRunning()) {
        SortedVector<int> svec;
        std::vector<int> vec;
        for (int i = 0; i < VECTOR_SIZE_TEN; i++) {
            vec.push_back(i);
        }

        for (int i = SVECTOR_END_POS_ASSIGN; i >= SVECTOR_START_POS_ASSIGN; i--) {
            svec.Add(i);
        }

        SortedVector<int> newSvec(svec);
        auto it = newSvec.Begin();
        auto it2 = vec.begin();
        for (; (it != newSvec.End()) and (it2 != vec.end()); it2++, it++) {
            AssertEqual(*it, *it2, "*it did not equal *it2 as expected.", state);
        }
    }
    BENCHMARK_LOGD("SortedVector testConsFromSortedAllowDup end.");
}

static void AddElement(SortedVector<int>& svec, std::vector<int>& vec)
{
    for (int i = 0; i < VECTOR_SIZE_TWENTY; i++) {
        vec.push_back(i);
    }

    for (int i = SVECTOR_END_POS_ASSIGN; i >= SVECTOR_START_POS_ASSIGN; i--) {
        svec.Add(i);
    }

    for (int i = 0; i < VECTOR_SIZE_TWENTY; i++) {
        svec.Add(i);
    }
}

BENCHMARK_F(BenchmarkSortedVector, testConsFromSortedNotAllowDuplicate)(benchmark::State& state)
{
    BENCHMARK_LOGD("SortedVector testConsFromSortedNotAllowDuplicate start.");
    while (state.KeepRunning()) {
        SortedVector<int> svec;
        std::vector<int> vec;
        AddElement(svec, vec);

        AssertEqual(static_cast<size_t>(EXPECTED_SIZE_AFTER_DUPLICATES), svec.Size(),
            "static_cast<size_t>(EXPECTED_SIZE_AFTER_DUPLICATES) did not equal svec.Size() as expected.", state);

        SortedVector<int, false> newSvec(svec);
        AssertEqual(static_cast<size_t>(VECTOR_SIZE_TWENTY), newSvec.Size(),
            "static_cast<size_t>(VECTOR_SIZE_TWENTY) did not equal newSvec.Size() as expected.", state);
        for (int i = 0; i < VECTOR_SIZE_TWENTY; i++) {
            AssertEqual(vec[i], newSvec[i], "vec[i] did not equal newSvec[i] as expected.", state);
        }
    }
    BENCHMARK_LOGD("SortedVector testConsFromSortedNotAllowDuplicate end.");
}

BENCHMARK_F(BenchmarkSortedVector, testConsFromSortedNotAllowToAlloworNotAllow)(benchmark::State& state)
{
    BENCHMARK_LOGD("SortedVector testConsFromSortedNotAllowToAlloworNotAllow start.");
    while (state.KeepRunning()) {
        SortedVector<int, false> svec;
        std::vector<int> vec;
        for (int i = 0; i < VECTOR_SIZE_TWENTY; i++) {
            vec.push_back(i);
        }

        for (int i = SVECTOR_START_POS_ASSIGN; i >= SVECTOR_START_POS_ASSIGN; i--) {
            svec.Add(i);
        }

        for (int i = 0; i < VECTOR_SIZE_TWENTY; i++) {
            svec.Add(i);
        }

        AssertEqual(static_cast<size_t>(VECTOR_SIZE_TWENTY), svec.Size(),
            "static_cast<size_t>(VECTOR_SIZE_TWENTY) did not equal svec.Size() as expected.", state);

        SortedVector<int> newSvecTrue(svec);
        AssertEqual(static_cast<size_t>(VECTOR_SIZE_TWENTY), newSvecTrue.Size(),
            "static_cast<size_t>(VECTOR_SIZE_TWENTY) did not equal newSvecTrue.Size() as expected.", state);
        for (int i = 0; i < VECTOR_SIZE_TWENTY; i++) {
            AssertEqual(vec[i], newSvecTrue[i], "vec[i] did not equal newSvecTrue[i] as expected.", state);
        }

        SortedVector<int> newSvecFalse(svec);
        AssertEqual(static_cast<size_t>(VECTOR_SIZE_TWENTY), newSvecFalse.Size(),
            "static_cast<size_t>(VECTOR_SIZE_TWENTY) did not equal newSvecFalse.Size() as expected.", state);
        for (int i = 0; i < VECTOR_SIZE_TWENTY; i++) {
            AssertEqual(vec[i], newSvecFalse[i], "vec[i] did not equal newSvecFalse[i] as expected.", state);
        }
    }
    BENCHMARK_LOGD("SortedVector testConsFromSortedNotAllowToAlloworNotAllow end.");
}

BENCHMARK_F(BenchmarkSortedVector, testoperatoreq)(benchmark::State& state)
{
    BENCHMARK_LOGD("SortedVector testoperatoreq start.");
    while (state.KeepRunning()) {
        SortedVector<int> svec;
        std::vector<int> vec;
        for (int i = 0; i < VECTOR_SIZE_TEN; i++) {
            vec.push_back(i);
        }

        for (int i = SVECTOR_END_POS_ASSIGN; i >= SVECTOR_START_POS_ASSIGN; i--) {
            svec.Add(i);
        }

        SortedVector<int> newSvec = svec;
        auto it = newSvec.Begin();
        auto it2 = svec.Begin();
        for (; (it != newSvec.End()) and (it2 != svec.End()); it2++, it++) {
            AssertEqual(*it, *it2, "*it did not equal *it2 as expected.", state);
        }
    }
    BENCHMARK_LOGD("SortedVector testoperatoreq end.");
}

BENCHMARK_F(BenchmarkSortedVector, testOperatorEqAllowToNotAllow)(benchmark::State& state)
{
    BENCHMARK_LOGD("SortedVector testOperatorEqAllowToNotAllow start.");
    while (state.KeepRunning()) {
        SortedVector<int> svec;
        std::vector<int> vec;
        AddElement(svec, vec);

        AssertEqual(static_cast<size_t>(EXPECTED_SIZE_AFTER_DUPLICATES), svec.Size(),
            "static_cast<size_t>(EXPECTED_SIZE_AFTER_DUPLICATES) did not equal svec.Size() as expected.", state);

        SortedVector<int, false> newSvec = svec;
        AssertEqual(static_cast<size_t>(VECTOR_SIZE_TWENTY), newSvec.Size(),
            "static_cast<size_t>(VECTOR_SIZE_TWENTY) did not equal newSvec.Size() as expected.", state);
        for (int i = 0; i < VECTOR_SIZE_TWENTY; i++) {
            AssertEqual(vec[i], newSvec[i], "vec[i] did not equal newSvec[i] as expected.", state);
        }
    }
    BENCHMARK_LOGD("SortedVector testOperatorEqAllowToNotAllow end.");
}

BENCHMARK_F(BenchmarkSortedVector, testOperatorEqNotAllowToAllowOrNotAllow)(benchmark::State& state)
{
    BENCHMARK_LOGD("SortedVector testOperatorEqNotAllowToAllowOrNotAllow start.");
    while (state.KeepRunning()) {
        SortedVector<int, false> svec;
        std::vector<int> vec;
        for (int i = 0; i < VECTOR_SIZE_TWENTY; i++) {
            vec.push_back(i);
        }

        for (int i = SVECTOR_END_POS_ASSIGN; i >= SVECTOR_START_POS_ASSIGN; i--) {
            svec.Add(i);
        }

        for (int i = 0; i < VECTOR_SIZE_TWENTY; i++) {
            svec.Add(i);
        }

        AssertEqual(static_cast<size_t>(VECTOR_SIZE_TWENTY), svec.Size(),
            "static_cast<size_t>(VECTOR_SIZE_TWENTY) did not equal svec.Size() as expected.", state);

        SortedVector<int, false> newSvecFalse = svec;
        SortedVector<int, true> newSvecTrue = svec;
        AssertEqual(static_cast<size_t>(VECTOR_SIZE_TWENTY), newSvecFalse.Size(),
            "static_cast<size_t>(VECTOR_SIZE_TWENTY) did not equal newSvecFalse.Size() as expected.", state);
        AssertEqual(static_cast<size_t>(VECTOR_SIZE_TWENTY), newSvecTrue.Size(),
            "static_cast<size_t>(VECTOR_SIZE_TWENTY) did not equal newSvecTrue.Size() as expected.", state);
        for (int i = 0; i < VECTOR_SIZE_TWENTY; i++) {
            AssertEqual(vec[i], newSvecFalse[i], "vec[i] did not equal newSvecFalse[i] as expected.", state);
            AssertEqual(vec[i], newSvecTrue[i], "vec[i] did not equal newSvecTrue[i] as expected.", state);
        }
    }
    BENCHMARK_LOGD("SortedVector testOperatorEqNotAllowToAllowOrNotAllow end.");
}

BENCHMARK_F(BenchmarkSortedVector, testOperatorEqAssignmentTwice)(benchmark::State& state)
{
    BENCHMARK_LOGD("SortedVector testOperatorEqAssignmentTwice start.");
    while (state.KeepRunning()) {
        SortedVector<int, true> svec;
        std::vector<int> vec;
        for (int i = VECTOR_SIZE_TWENTY; i < ELEMENT_END_VALUE; i++) {
            vec.push_back(i);
        }

        for (int i = 0; i < VECTOR_SIZE_TWENTY; i++) {
            svec.Add(i);
        }

        SortedVector<int, false> newSvecFalse;
        for (int i = VECTOR_SIZE_TWENTY; i < ELEMENT_END_VALUE; i++) {
            newSvecFalse.Add(i);
        }

        svec = newSvecFalse;
        AssertEqual(static_cast<size_t>(VECTOR_SIZE_TEN), svec.Size(),
            "static_cast<size_t>(VECTOR_SIZE_TEN) did not equal svec.Size() as expected.", state);
        for (int i = 0; i < VECTOR_SIZE_TEN; i++) {
            AssertEqual(vec[i], svec[i], "vec[i] did not equal svec[i] as expected.", state);
        }
    }
    BENCHMARK_LOGD("SortedVector testOperatorEqAssignmentTwice end.");
}

BENCHMARK_F(BenchmarkSortedVector, testoperatorconsteq)(benchmark::State& state)
{
    BENCHMARK_LOGD("SortedVector testoperatorconsteq start.");
    while (state.KeepRunning()) {
        SortedVector<int> svec;
        std::vector<int> vec;
        for (int i = 0; i < VECTOR_SIZE_TEN; i++) {
            vec.push_back(i);
        }

        for (int i = SVECTOR_END_POS_ASSIGN; i >= SVECTOR_START_POS_ASSIGN; i--) {
            svec.Add(i);
        }

        const SortedVector<int> newSvec = svec;
        auto it = newSvec.Begin();
        auto it2 = svec.Begin();
        for (; (it != newSvec.End()) and (it2 != svec.End()); it2++, it++) {
            AssertEqual(*it, *it2, "*it did not equal *it2 as expected.", state);
        }
    }
    BENCHMARK_LOGD("SortedVector testoperatorconsteq end.");
}

BENCHMARK_F(BenchmarkSortedVector, testsizeclearIsEmpty)(benchmark::State& state)
{
    BENCHMARK_LOGD("SortedVector testsizeclearIsEmpty start.");
    const size_t expectedSizeEmpty = 0;
    const size_t expectedSizeAfterAdd = 10;
    while (state.KeepRunning()) {
        SortedVector<int> svec;
        AssertTrue(svec.IsEmpty(), "svec.IsEmpty() did not equal true as expected.", state);
        AssertEqual(svec.Size(), static_cast<size_t>(expectedSizeEmpty),
            "svec.Size() did not equal static_cast<size_t>(expectedSizeEmpty) as expected.", state);

        for (int i = SVECTOR_END_POS_ASSIGN; i >= SVECTOR_START_POS_ASSIGN; i--) {
            svec.Add(i);
        }
        AssertEqual(svec.Size(), static_cast<const unsigned long>(expectedSizeAfterAdd),
            "svec.Size() did not equal static_cast<const unsigned long>(expectedSizeAfterAdd) as expected.", state);
        AssertFalse(svec.IsEmpty(), "svec.IsEmpty() did not equal false as expected.", state);

        svec.Clear();
        AssertTrue(svec.IsEmpty(), "svec.IsEmpty() did not equal true as expected.", state);
        AssertEqual(svec.Size(), static_cast<size_t>(expectedSizeEmpty),
            "svec.Size() did not equal static_cast<size_t>(expectedSizeEmpty) as expected.", state);
    }
    BENCHMARK_LOGD("SortedVector testsizeclearIsEmpty end.");
}

BENCHMARK_F(BenchmarkSortedVector, testCapasityandSetcapasity)(benchmark::State& state)
{
    BENCHMARK_LOGD("SortedVector testCapasityandSetcapasity start.");
    const size_t initialCapacity = 1000;
    const size_t reducedCapacity = 500;
    while (state.KeepRunning()) {
        SortedVector<int> svec;
        for (int i = SVECTOR_END_POS_ASSIGN; i >= SVECTOR_START_POS_ASSIGN; i--) {
            svec.Add(i);
        }

        svec.SetCapcity(initialCapacity);
        AssertEqual(svec.Capacity(), static_cast<size_t>(initialCapacity),
            "svec.Capacity() did not equal static_cast<size_t>(initialCapacity) as expected.", state);
        AssertLessThan(svec.SetCapcity(reducedCapacity), static_cast<ssize_t>(0),
            "svec.SetCapcity(reducedCapacity) was not less than static_cast<ssize_t>(0) as expected.", state);
        AssertEqual(svec.Capacity(), static_cast<size_t>(initialCapacity),
            "svec.Capacity() did not equal static_cast<size_t>(initialCapacity) as expected.", state);
    }
    BENCHMARK_LOGD("SortedVector testCapasityandSetcapasity end.");
}

BENCHMARK_F(BenchmarkSortedVector, testconstArray)(benchmark::State& state)
{
    BENCHMARK_LOGD("SortedVector testconstArray start.");
    while (state.KeepRunning()) {
        SortedVector<int> svec;
        for (int i = SVECTOR_END_POS_ASSIGN; i >= SVECTOR_START_POS_ASSIGN; i--) {
            svec.Add(i);
        }

        const int* pi = svec.Array();
        int arraySize = svec.Size();
        auto it = svec.Begin();
        int i = 0;
        for (; (i < arraySize) && (it != svec.End()); ++it, ++i) {
            AssertEqual(pi[i], *it, "pi[i] did not equal *it as expected.", state);
        }

        AssertEqual(i, arraySize, "i did not equal arraySize as expected.", state);
        AssertEqual(it, svec.End(), "it did not equal svec.End() as expected.", state);
    }
    BENCHMARK_LOGD("SortedVector testconstArray end.");
}

BENCHMARK_F(BenchmarkSortedVector, testeditArray)(benchmark::State& state)
{
    BENCHMARK_LOGD("SortedVector testeditArray start.");
    while (state.KeepRunning()) {
        SortedVector<int> svec;
        for (int i = SVECTOR_END_POS_ASSIGN; i >= SVECTOR_START_POS_ASSIGN; i--) {
            svec.Add(i);
        }

        int* pi = svec.EditArray();
        int arraySize = svec.Size();
        // compare and equal
        auto it = svec.Begin();
        int i = 0;
        for (; (i < arraySize) && (it != svec.End()); ++it, ++i) {
            AssertEqual(pi[i], *it, "pi[i] did not equal *it as expected.", state);
        }
        //  size equal
        AssertEqual(i, arraySize, "i did not equal arraySize as expected.", state);
        AssertEqual(it, svec.End(), "it did not equal svec.End() as expected.", state);

        //  fix value
        for (int t = 0; t < arraySize; ++t) {
            pi[t] += 1;
        }

        // compare  and equal add 1
        SortedVector<int> copyvec;
        it = copyvec.Begin();
        i = 0;
        for (; (i < arraySize) && (it != copyvec.End()); ++it, ++i) {
            AssertEqual(pi[i], *it + 1, "pi[i] did not equal *it + 1 as expected.", state);
        }

        // all renewed
        it = svec.Begin();
        i = 0;
        for (; (i < arraySize) && (it != svec.End()); ++it, ++i) {
            AssertEqual(pi[i], *it, "pi[i] did not equal *it as expected.", state);
        }
    }
    BENCHMARK_LOGD("SortedVector testeditArray end.");
}

BENCHMARK_F(BenchmarkSortedVector, testIndexOf)(benchmark::State& state)
{
    BENCHMARK_LOGD("SortedVector testIndexOf start.");
    const int searchValueTen = 10;
    const int invalidIndex = -1;
    while (state.KeepRunning()) {
        SortedVector<int> svec;
        for (int i = SVECTOR_END_POS_ASSIGN; i >= SVECTOR_START_POS_ASSIGN; i--) {
            svec.Add(i);
        }

        int arraySize = svec.Size();
        auto it = svec.Begin();
        int i = 0;
        for (; (i < arraySize) && (it != svec.End()); ++it, ++i) {
            AssertEqual(i, svec.IndexOf(i), "i did not equal svec.IndexOf(i) as expected.", state);
        }
        AssertEqual(invalidIndex, svec.IndexOf(searchValueTen),
            "invalidIndex did not equal svec.IndexOf(searchValueTen) as expected.", state);
    }
    BENCHMARK_LOGD("SortedVector testIndexOf end.");
}


BENCHMARK_F(BenchmarkSortedVector, testOrderof)(benchmark::State& state)
{
    BENCHMARK_LOGD("SortedVector testOrderof start.");
    const int searchValueNegTwo = -2;
    const int searchValueNegOne = -1;
    const int searchValueNine = 9;
    const int searchValueTen = 10;
    const size_t orderNotFound = 0;
    const size_t expectedOrderTen = 10;
    while (state.KeepRunning()) {
        SortedVector<int> svec;
        for (int i = SVECTOR_END_POS_ASSIGN; i >= SVECTOR_START_POS_ASSIGN; i--) {
            svec.Add(i);
        }

        size_t count = svec.Size();
        size_t order = 0;
        for (size_t i = 0; i < count; i++) {
            order = svec.OrderOf(i);
            AssertEqual((i + 1), order, "i + 1 did not equal order as expected.", state);
        }

        AssertEqual(orderNotFound, svec.OrderOf(searchValueNegTwo),
            "orderNotFound did not equal svec.OrderOf(searchValueNegTwo) as expected.", state);
        AssertEqual(orderNotFound, svec.OrderOf(searchValueNegOne),
            "orderNotFound did not equal svec.OrderOf(searchValueNegOne) as expected.", state);
        AssertEqual(expectedOrderTen, svec.OrderOf(searchValueNine),
            "expectedOrderTen did not equal svec.OrderOf(searchValueNine) as expected.", state);
        AssertEqual(expectedOrderTen, svec.OrderOf(searchValueTen),
            "expectedOrderTen did not equal svec.OrderOf(searchValueTen) as expected.", state);
    }
    BENCHMARK_LOGD("SortedVector testOrderof end.");
}

BENCHMARK_F(BenchmarkSortedVector, testoperatorAccess)(benchmark::State& state)
{
    BENCHMARK_LOGD("SortedVector testoperatorAccess start.");
    while (state.KeepRunning()) {
        SortedVector<int> svec;
        for (int i = SVECTOR_END_POS_ASSIGN; i >= SVECTOR_START_POS_ASSIGN; i--) {
            svec.Add(i);
        }

        for (int i = SVECTOR_START_POS_ASSIGN; i <= SVECTOR_END_POS_ASSIGN; i++) {
            AssertEqual(i, svec[i], "i did not equal svec[i] as expected.", state);
        }
    }
    BENCHMARK_LOGD("SortedVector testoperatorAccess end.");
}

BENCHMARK_F(BenchmarkSortedVector, testBack)(benchmark::State& state)
{
    BENCHMARK_LOGD("SortedVector testBack start.");
    while (state.KeepRunning()) {
        SortedVector<int> svec;
        for (int i = SVECTOR_END_POS_ASSIGN; i >= SVECTOR_START_POS_ASSIGN; i--) {
            svec.Add(i);
        }

        AssertEqual(SVECTOR_END_POS_ASSIGN, svec.Back(),
            "SVECTOR_END_POS_ASSIGN did not equal svec.Back() as expected.", state);
    }
    BENCHMARK_LOGD("SortedVector testBack end.");
}

BENCHMARK_F(BenchmarkSortedVector, testMirrorItemAt)(benchmark::State& state)
{
    BENCHMARK_LOGD("SortedVector testMirrorItemAt start.");
    const ssize_t mirrorStartIndex = -1; // Indicating the start index for mirror item test
    const ssize_t mirrorEndIndex = -10;  // Indicating the end index for mirror item test
    const ssize_t mirrorOffset = 10;     // Indicating the offset for calculating the mirror item
    while (state.KeepRunning()) {
        SortedVector<int> svec;
        for (int i = SVECTOR_END_POS_ASSIGN; i >= SVECTOR_START_POS_ASSIGN; i--) {
            svec.Add(i);
        }

        for (ssize_t i = mirrorStartIndex; i >= mirrorEndIndex; i--) {
            AssertEqual((i + mirrorOffset), svec.MirrorItemAt(i),
                "i + mirrorOffset did not equal svec.MirrorItemAt(i) as expected.", state);
        }

        for (ssize_t i = SVECTOR_START_POS_ASSIGN; i <= SVECTOR_END_POS_ASSIGN; i++) {
            AssertEqual(i, svec.MirrorItemAt(i), "i did not equal svec.MirrorItemAt(i) as expected.", state);
        }
    }
    BENCHMARK_LOGD("SortedVector testMirrorItemAt end.");
}

BENCHMARK_F(BenchmarkSortedVector, testEditItemAt)(benchmark::State& state)
{
    BENCHMARK_LOGD("SortedVector testEditItemAt start.");
    while (state.KeepRunning()) {
        SortedVector<int> svec;
        for (int i = SVECTOR_END_POS_ASSIGN; i >= SVECTOR_START_POS_ASSIGN; i--) {
            svec.Add(i);
        }

        for (ssize_t i = SVECTOR_START_POS_ASSIGN; i <= SVECTOR_END_POS_ASSIGN; i++) {
            svec.EditItemAt(i) += 1;
            AssertEqual((i + 1), svec.EditItemAt(i), "i + 1 did not equal svec.EditItemAt(i) as expected.", state);
        }
    }
    BENCHMARK_LOGD("SortedVector testEditItemAt end.");
}

BENCHMARK_F(BenchmarkSortedVector, testCopyCtorFromVector)(benchmark::State& state)
{
    BENCHMARK_LOGD("SortedVector testCopyCtorFromVector start.");
    while (state.KeepRunning()) {
        std::vector<int> vec;
        for (int i = SVECTOR_END_POS_ASSIGN; i >= SVECTOR_START_POS_ASSIGN; i--) {
            vec.push_back(i);
        }

        SortedVector<int> svec(vec);
        for (ssize_t i = SVECTOR_START_POS_ASSIGN; i <= SVECTOR_END_POS_ASSIGN; i++) {
            AssertEqual(i, svec[i], "i did not equal svec[i] as expected.", state);
        }
    }
    BENCHMARK_LOGD("SortedVector testCopyCtorFromVector end.");
}

BENCHMARK_F(BenchmarkSortedVector, testConsFromVectorToNotAllowDuplicate)(benchmark::State& state)
{
    BENCHMARK_LOGD("SortedVector testConsFromVectorToNotAllowDuplicate start.");
    static constexpr size_t consFromVectorVectorSize = 30;
    static constexpr size_t consFromVectorSortedVectorSize = 20;
    while (state.KeepRunning()) {
        std::vector<int> vec;
        for (int i = SVECTOR_END_POS_ASSIGN; i >= SVECTOR_START_POS_ASSIGN; i--) {
            vec.push_back(i);
        }

        for (int i = 19; i >= 0; i--) {
            vec.push_back(i);
        }

        AssertEqual(static_cast<size_t>(consFromVectorVectorSize), vec.size(),
            "static_cast<size_t>(consFromVectorVectorSize) did not equal vec.size() as expected.", state);

        SortedVector<int, false> svec(vec);
        AssertEqual(static_cast<size_t>(consFromVectorSortedVectorSize), svec.Size(),
            "static_cast<size_t>(consFromVectorSortedVectorSize) did not equal svec.Size() as expected.", state);
        for (ssize_t i = 0; i <= 19; i++) {
            AssertEqual(i, svec[i], "i did not equal svec[i] as expected.", state);
        }
    }
    BENCHMARK_LOGD("SortedVector testConsFromVectorToNotAllowDuplicate end.");
}

BENCHMARK_F(BenchmarkSortedVector, testMergevector)(benchmark::State& state)
{
    BENCHMARK_LOGD("SortedVector testMergevector start.");
    static constexpr int mergeStart = 10; // Represents the starting position for adding elements to std::vector
    static constexpr int mergeEnd = 20;   // Represents the ending position for adding elements to
                                          // std::vector and the expected size of SortedVector
    while (state.KeepRunning()) {
        SortedVector<int> svec;
        for (int i = SVECTOR_END_POS_ASSIGN; i >= SVECTOR_START_POS_ASSIGN; i--) {
            svec.Add(i);
        }

        std::vector<int> vec;
        for (int i = mergeStart; i < mergeEnd; i++) {
            vec.push_back(i);
        }

        svec.Merge(vec);

        for (ssize_t i = 0; i < mergeEnd; i++) {
            AssertEqual(i, svec[i], "i did not equal svec[i] as expected.", state);
        }
    }
    BENCHMARK_LOGD("SortedVector testMergevector end.");
}

BENCHMARK_F(BenchmarkSortedVector, testMergevectorNoduplicate)(benchmark::State& state)
{
    BENCHMARK_LOGD("SortedVector testMergevectorNoduplicate start.");
    static constexpr int initialMergeStart = 0;   // Represents the starting position for
                                                  // the first range of elements added to std::vector
    static constexpr int initialMergeEnd = 20;    // Represents the ending position for
                                                  // the first range of elements added to std::vector
    static constexpr int extendedMergeStart = 10; // Represents the starting position for
                                                  // the second range of elements added to std::vector
    static constexpr int extendedMergeEnd = 30;   // Represents the ending position for
                                                  // the second range of elements added to std::vector
                                                  // and the expected size of SortedVector after merge
    while (state.KeepRunning()) {
        SortedVector<int, false> svec;
        for (int i = SVECTOR_END_POS_ASSIGN; i >= SVECTOR_START_POS_ASSIGN; i--) {
            svec.Add(i);
        }

        std::vector<int> vec;
        for (int i = initialMergeStart; i < initialMergeEnd; i++) {
            vec.push_back(i);
        }

        for (int i = extendedMergeStart; i < extendedMergeEnd; i++) {
            vec.push_back(i);
        }

        svec.Merge(vec);

        AssertEqual(svec.Size(), static_cast<size_t>(extendedMergeEnd),
            "svec.Size() did not equal static_cast<size_t>(extendedMergeEnd) as expected.", state);
        for (ssize_t i = initialMergeStart; i < extendedMergeEnd; i++) {
            AssertEqual(i, svec[i], "i did not equal svec[i] as expected.", state);
        }
    }
    BENCHMARK_LOGD("SortedVector testMergevectorNoduplicate end.");
}

BENCHMARK_F(BenchmarkSortedVector, testMergesortedvectorNoduplicate)(benchmark::State& state)
{
    BENCHMARK_LOGD("SortedVector testMergesortedvectorNoduplicate start.");
    const size_t expectedSizeAfterMerge = 10; // Expected size of the vector after merging
                                              // two vectors without duplicates
    const ssize_t startValueForElements = 0;  // Start value of elements to be added to the vector
    const ssize_t endValueForElements = 9;    // End value of elements to be added to the vector
    while (state.KeepRunning()) {
        SortedVector<int, false> svec;
        SortedVector<int, false> svec2;
        for (int i = SVECTOR_END_POS_ASSIGN; i >= SVECTOR_START_POS_ASSIGN; i--) {
            svec.Add(i);
            svec2.Add(i);
        }

        svec.Merge(svec2);

        AssertEqual(static_cast<size_t>(expectedSizeAfterMerge), svec.Size(),
            "Expected size after merge did not equal svec.Size().", state);
        // 0,1,2,3,4,5,6,7,8,9
        for (ssize_t i = startValueForElements; i <= endValueForElements; i++) {
            AssertEqual(i, svec[i], "Element value did not match expected value.", state);
        }
    }
    BENCHMARK_LOGD("SortedVector testMergesortedvectorNoduplicate end.");
}

BENCHMARK_F(BenchmarkSortedVector, testMergesortedvector)(benchmark::State& state)
{
    BENCHMARK_LOGD("SortedVector testMergesortedvector start.");
    static constexpr ssize_t expectedMergedSize = 20; // Represents the expected size of SortedVector after merge
    static constexpr int duplicateFactor = 2; // Represents the factor of duplication for each element in SortedVector
    while (state.KeepRunning()) {
        SortedVector<int> svec;
        SortedVector<int> svec2;
        for (int i = SVECTOR_END_POS_ASSIGN; i >= SVECTOR_START_POS_ASSIGN; i--) {
            svec.Add(i);
            svec2.Add(i);
        }

        svec.Merge(svec2);

        // 0,0,1,1,2,2,3,3...
        for (ssize_t i = 0; i < expectedMergedSize; i++) {
            AssertEqual((i / duplicateFactor), svec[i],
                "(i / duplicateFactor) did not equal svec[i] as expected.", state);
        }
    }
    BENCHMARK_LOGD("SortedVector testMergesortedvector end.");
}

BENCHMARK_F(BenchmarkSortedVector, testAddNotAllowDuplicate)(benchmark::State& state)
{
    BENCHMARK_LOGD("SortedVector testAddNotAllowDuplicate start.");
    static constexpr int addNotAllowedSvectorSize = 10;
    while (state.KeepRunning()) {
        SortedVector<int, false> svec;
        for (int i = SVECTOR_END_POS_ASSIGN; i >= SVECTOR_START_POS_ASSIGN; i--) {
            AssertUnequal(svec.Add(i), static_cast<ssize_t>(SVECTOR_INVALID_VALUE),
                "svec.Add(i) was not different from static_cast<ssize_t>(-1) as expected.", state);
        }
        AssertEqual(static_cast<size_t>(addNotAllowedSvectorSize), svec.Size(),
            "static_cast<size_t>(addNotAllowedSvectorSize) did not equal svec.Size() as expected.", state);

        // duplicate
        for (int i = SVECTOR_END_POS_ASSIGN; i >= SVECTOR_START_POS_ASSIGN; i--) {
            AssertEqual(svec.Add(i), static_cast<ssize_t>(SVECTOR_INVALID_VALUE),
                "svec.Add(i) did not equal static_cast<ssize_t>(-1) as expected.", state);
            AssertEqual(static_cast<size_t>(addNotAllowedSvectorSize), svec.Size(),
                "static_cast<size_t>(addNotAllowedSvectorSize) did not equal svec.Size() as expected.", state);
        }

        for (ssize_t i = SVECTOR_START_POS_VALUE; i < SVECTOR_END_POS_VALUE; i++) {
            AssertEqual(i, svec[i], "i did not equal svec[i] as expected.", state);
        }
    }
    BENCHMARK_LOGD("SortedVector testAddNotAllowDuplicate end.");
}

BENCHMARK_F(BenchmarkSortedVector, testConsVectorAllowDuplicate)(benchmark::State& state)
{
    BENCHMARK_LOGD("SortedVector testConsVectorAllowDuplicate start.");
    while (state.KeepRunning()) {
        SortedVector<int> svec;
        std::vector<int> vec;
        for (int i = SVECTOR_END_POS_ASSIGN; i >= SVECTOR_START_POS_ASSIGN; i--) {
            vec.push_back(i);
            svec.Add(i);
        }

        SortedVector<int> newSvec(vec);
        auto it = newSvec.Begin();
        auto it2 = svec.Begin();
        for (; (it != newSvec.End()) and (it2 != vec.end()); it2++, it++) {
            AssertEqual(*it, *it2, "*it did not equal *it2 as expected.", state);
        }
    }
    BENCHMARK_LOGD("SortedVector testConsVectorAllowDuplicate end.");
}

BENCHMARK_F(BenchmarkSortedVector, testFront)(benchmark::State& state)
{
    BENCHMARK_LOGD("SortedVector testFront start.");
    static constexpr int frontSvectorFrontValue = 0;
    while (state.KeepRunning()) {
        SortedVector<int> svec;
        for (int i = SVECTOR_END_POS_ASSIGN; i >= SVECTOR_START_POS_ASSIGN; i--) {
            svec.Add(i);
        }

        AssertEqual(svec.Front(), frontSvectorFrontValue, "svec.Front() did not equal 0 as expected.", state);
    }
    BENCHMARK_LOGD("SortedVector testFront end.");
}

BENCHMARK_F(BenchmarkSortedVector, testPopBack)(benchmark::State& state)
{
    BENCHMARK_LOGD("SortedVector testPopBack start.");
    static constexpr int popBackSvectorBackValue = 8;
    static constexpr size_t popBackSvectorSize = 9;
    while (state.KeepRunning()) {
        SortedVector<int> svec;
        for (int i = SVECTOR_END_POS_ASSIGN; i >= SVECTOR_START_POS_ASSIGN; i--) {
            svec.Add(i);
        }

        svec.PopBack();
        AssertEqual(svec.Back(), popBackSvectorBackValue, "svec.Back() did not equal 8 as expected.", state);
        AssertEqual(svec.Size(), popBackSvectorSize, "svec.Size() did not equal 9 as expected.", state);
    }
    BENCHMARK_LOGD("SortedVector testPopBack end.");
}

BENCHMARK_F(BenchmarkSortedVector, testErase)(benchmark::State& state)
{
    BENCHMARK_LOGD("SortedVector testErase start.");
    const size_t erasePosition = 5;
    const int expectedValueAtErasePosition = 6;
    const size_t expectedSizeAfterErase = 9;
    while (state.KeepRunning()) {
        SortedVector<int> svec;
        for (int i = SVECTOR_END_POS_ASSIGN; i >= SVECTOR_START_POS_ASSIGN; i--) {
            svec.Add(i);
        }

        svec.Erase(erasePosition);
        AssertEqual(svec[erasePosition], expectedValueAtErasePosition,
            "svec[erasePosition] did not equal expectedValueAtErasePosition as expected.", state);
        AssertEqual(svec.Size(), expectedSizeAfterErase,
            "svec.Size() did not equal expectedSizeAfterErase as expected.", state);
    }
    BENCHMARK_LOGD("SortedVector testErase end.");
}
}  // namespace
}  // namespace OHOS
// Run the benchmark
BENCHMARK_MAIN();