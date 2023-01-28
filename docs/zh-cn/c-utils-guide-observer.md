# 观察者模式
## 概述
### 简介
定义了对象间的一种一对多依赖关系，使得每当一个对象状态发生改变时，其相关依赖对象皆得到通知并被自动更新。

## 涉及功能
### class Observable
#### 接口说明

|返回类型            | 名称           |
| -------------- | -------------- |
| virtual | **~Observable**() =default |
| void | **AddObserver**(const std::shared_ptr< Observer >& o)<br>把指定的观察者添加到观察者的集合中。  |
| int | **GetObserversCount**()<br>获取观察者的数量。  |
| void | **NotifyObservers**()<br>通知所有观察者，无相关数据传输。  |
| void | **NotifyObservers**(const ObserverArg* arg)<br>通知所有观察者，向观察者传入数据`arg`。  |
| void | **RemoveAllObservers**()<br>移除所有观察者。  |
| void | **RemoveObserver**(const std::shared_ptr< Observer >& o)<br>移除指定的观察者。  |

### class Observer
#### 接口说明
|返回类型            | 名称           |
| -------------- | -------------- |
| virtual | **~Observer**() =default |
| virtual void | **Update**(const Observable* o, const ObserverArg* arg) =0<br>观察者更新自己的函数接口。  |

## 使用示例

1. 示例代码(伪代码)

```c++
#include <iostream>
#include "../include/observer.h"

using namespace OHOS;
using namespace std;

// 被观察者类，一个图书列表
class BookList : public Observable {
public:
    BookList() { books_.clear(); }
    void AddBook(const string& book)
    {
        books_.insert(book);
        SetChanged();
        NotifyObservers();
    }

    void RemoveBook(const string& book)
    {
        books_.erase(book);
        SetChanged();
        NotifyObservers();
    }

    const set<string>& GetBooks() { return books_; }
private:
    set<string> books_;
};

// 观察者类，对图书感兴趣的人
class BookObserver : public Observer {
public:
    BookObserver() = default;
    explicit BookObserver(const string &name) : name_(name)
    {
    }
    void Update(const Observable* o, const ObserverArg* /* arg */) override
    {
        BookList* bookList = reinterpret_cast<BookList*>(const_cast<Observable*>(o));
        books_ = bookList->GetBooks();
        cout << name_ << " has been Notified" << endl;
    }

    int GetBooksCount() { return static_cast<int>(books_.size()); }
    bool BookExists(const string& book) { return books_.count(book) > 0;}
private:
    set<string> books_;
    string name_;
};

int main()
{
    BookList bookList;
    // 构造观察者
    shared_ptr<BookObserver> bookObserver1 = make_shared<BookObserver>("Mao");
    shared_ptr<BookObserver> bookObserver2 = make_shared<BookObserver>("Administrator");
    shared_ptr<BookObserver> bookObserver3 = make_shared<BookObserver>("You");
    // 添加观察者
    bookList.AddObserver(bookObserver1);
    bookList.AddObserver(bookObserver2);
    bookList.AddObserver(bookObserver3);
    // 向图书列表中添加书籍，此时会通知观察者
    bookList.AddBook("book1");
    // 移除所有观察者
    bookList.RemoveAllObservers();
    // 此时从图书列表中移除书籍，此时没有观察者被通知
    bookList.RemoveBook("book1");
}
```

2. 测试用例编译运行方法

- 测试用例代码参见base/test/unittest/common/utils_observer_test.cpp

- 使用开发者自测试框架，使用方法参见：[开发自测试执行框架-测试用例执行](https://gitee.com/openharmony/testfwk_developer_test#%E6%B5%8B%E8%AF%95%E7%94%A8%E4%BE%8B%E6%89%A7%E8%A1%8C)

- 使用以下具体命令以运行`observer.h`对应测试用例

```bash
run -t UT -tp utils -ts UtilsObserverTest
```