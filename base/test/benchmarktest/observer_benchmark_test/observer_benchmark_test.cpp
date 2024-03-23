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
#include "observer.h"
#include <algorithm>
#include <iostream>
#include <fstream>
#include "benchmark_log.h"
#include "benchmark_assert.h"
using namespace std;

namespace OHOS {
namespace {

class BenchmarkObserverTest : public benchmark::Fixture {
public:
    void SetUp(const ::benchmark::State& state) override
    {
    }

    void TearDown(const ::benchmark::State& state) override
    {
    }

    BenchmarkObserverTest()
    {
        Iterations(iterations);
        Repetitions(repetitions);
        ReportAggregatesOnly();
    }

    ~BenchmarkObserverTest() override = default;

protected:
    const int32_t repetitions = 3;
    const int32_t iterations = 1000;
};

const int EXPECTED_COUNT_ZERO = 0;
const int EXPECTED_COUNT_ONE = 1;
const int EXPECTED_COUNT_TWO = 2;

class BookList : public Observable {
public:
    BookList() { books_.clear(); }
    void AddBook(const string& book)
    {
        BENCHMARK_LOGD("ObserverTest void AddBook is called.");
        books_.insert(book);
        SetChanged();
        NotifyObservers();
    }

    void RemoveBook(const string& book)
    {
        BENCHMARK_LOGD("ObserverTest void RemoveBook is called.");
        books_.erase(book);
        SetChanged();
        NotifyObservers();
    }

    void NoChangeNotify()
    {
        BENCHMARK_LOGD("ObserverTest void NoChangeNotify is called.");
        if (HasChanged() == false) {
            NotifyObservers();
        }
    }

    const set<string>& GetBooks() { return books_; }
private:
    set<string> books_;
};

class BookObserver : public Observer {
public:
    virtual void Update(const Observable* o, const ObserverArg* /* arg */)
    {
        BookList* bookList = reinterpret_cast<BookList*>(const_cast<Observable*>(o));
        books_ = bookList->GetBooks();
    }

    int GetBooksCount() { return static_cast<int>(books_.size()); }
    bool BookExists(const string& book) { return books_.count(book) > 0;}
private:
    set<string> books_;
};

/*
 * @tc.name: test_Observer
 * @tc.desc: Test add null or repeat observer to the observable object.
 */
BENCHMARK_F(BenchmarkObserverTest, test_Observer)(benchmark::State& state)
{
    BENCHMARK_LOGD("ObserverTest test_Observer start.");
    while (state.KeepRunning()) {
        BookList bookList;
        bookList.AddObserver(nullptr);
        shared_ptr<BookObserver> bookObserver1 = make_shared<BookObserver>();
        bookList.AddObserver(bookObserver1);
        bookList.AddObserver(bookObserver1);
        bookList.NoChangeNotify();
        int ret = bookList.GetObserversCount();
        AssertEqual(ret, 1, "ret did not equal 1 as expected.", state);
    }
    BENCHMARK_LOGD("ObserverTest test_Observer end.");
}

void AddObservers(BookList& bookList, shared_ptr<BookObserver>& bookObserver1, shared_ptr<BookObserver>& bookObserver2,
    shared_ptr<BookObserver>& bookObserver3, benchmark::State& state)
{
        bookList.AddObserver(bookObserver1);
        bookList.AddObserver(bookObserver2);
        bookList.AddObserver(bookObserver3);
        bookList.AddBook("book1");

        AssertEqual(bookObserver1->GetBooksCount(), EXPECTED_COUNT_ONE,
            "bookObserver1->GetBooksCount() did not equal EXPECTED_COUNT_ONE as expected.", state);
        AssertEqual(bookObserver2->GetBooksCount(), EXPECTED_COUNT_ONE,
            "bookObserver2->GetBooksCount() did not equal EXPECTED_COUNT_ONE as expected.", state);
        AssertEqual(bookObserver3->GetBooksCount(), EXPECTED_COUNT_ONE,
            "bookObserver3->GetBooksCount() did not equal EXPECTED_COUNT_ONE as expected.", state);
}

BENCHMARK_F(BenchmarkObserverTest, test_ObserverNotify)(benchmark::State& state)
{
    BENCHMARK_LOGD("ObserverTest test_ObserverNotify start.");
    while (state.KeepRunning()) {
        BookList bookList;
        shared_ptr<BookObserver> bookObserver1 = make_shared<BookObserver>();
        shared_ptr<BookObserver> bookObserver2 = make_shared<BookObserver>();
        shared_ptr<BookObserver> bookObserver3 = make_shared<BookObserver>();
        AddObservers(bookList, bookObserver1, bookObserver2, bookObserver3, state);

        bookList.RemoveObserver(bookObserver1);
        bookList.RemoveBook("book1");
        AssertEqual(bookObserver1->GetBooksCount(), EXPECTED_COUNT_ONE,
            "bookObserver1->GetBooksCount() did not equal EXPECTED_COUNT_ONE as expected.", state);
        AssertEqual(bookObserver2->GetBooksCount(), EXPECTED_COUNT_ZERO,
            "bookObserver2->GetBooksCount() did not equal EXPECTED_COUNT_ZERO as expected.", state);
        AssertEqual(bookObserver3->GetBooksCount(), EXPECTED_COUNT_ZERO,
            "bookObserver3->GetBooksCount() did not equal EXPECTED_COUNT_ZERO as expected.", state);

        bookList.RemoveObserver(bookObserver2);
        bookList.AddBook("book2");
        bookList.AddBook("book3");
        AssertEqual(bookObserver1->GetBooksCount(), EXPECTED_COUNT_ONE,
            "bookObserver1->GetBooksCount() did not equal EXPECTED_COUNT_ONE as expected.", state);
        AssertEqual(bookObserver2->GetBooksCount(), EXPECTED_COUNT_ZERO,
            "bookObserver2->GetBooksCount() did not equal EXPECTED_COUNT_ZERO as expected.", state);
        AssertEqual(bookObserver3->GetBooksCount(), EXPECTED_COUNT_TWO,
            "bookObserver3->GetBooksCount() did not equal EXPECTED_COUNT_TWO as expected.", state);
    }
    BENCHMARK_LOGD("ObserverTest test_ObserverNotify end.");
}

BENCHMARK_F(BenchmarkObserverTest, test_RemoveAllObserver)(benchmark::State& state)
{
    BENCHMARK_LOGD("ObserverTest test_RemoveAllObserver start.");
    while (state.KeepRunning()) {
        BookList bookList;
        shared_ptr<BookObserver> bookObserver1 = make_shared<BookObserver>();
        shared_ptr<BookObserver> bookObserver2 = make_shared<BookObserver>();
        shared_ptr<BookObserver> bookObserver3 = make_shared<BookObserver>();
        AddObservers(bookList, bookObserver1, bookObserver2, bookObserver3, state);

        bookList.RemoveAllObservers();
        bookList.RemoveBook("book1");
        AssertEqual(bookObserver1->GetBooksCount(), EXPECTED_COUNT_ONE,
            "bookObserver1->GetBooksCount() did not equal EXPECTED_COUNT_ONE as expected.", state);
        AssertEqual(bookObserver2->GetBooksCount(), EXPECTED_COUNT_ONE,
            "bookObserver2->GetBooksCount() did not equal EXPECTED_COUNT_ONE as expected.", state);
        AssertEqual(bookObserver3->GetBooksCount(), EXPECTED_COUNT_ONE,
            "bookObserver3->GetBooksCount() did not equal EXPECTED_COUNT_ONE as expected.", state);
        AssertEqual(bookList.GetObserversCount(), EXPECTED_COUNT_ZERO,
            "bookList.GetObserversCount() did not equal EXPECTED_COUNT_ZERO as expected.", state);
    }
    BENCHMARK_LOGD("ObserverTest test_RemoveAllObserver end.");
}
}  // namespace
}  // namespace OHOS
// Run the benchmark
BENCHMARK_MAIN();