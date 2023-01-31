# 有序vector

## 概述
### 简介
提供了一个可以对插入元素自动排序的vector，具有添加元素，删除最后一个元素，合并vector等功能。
## 涉及功能
### class SortedVector
#### 接口说明

|返回类型           |名称           |
| -------------- | -------------- |
| | **SortedVector**()<br>构造函数。  |
| | **SortedVector**(const SortedVector< TYPE, false >& rhs)<br>拷贝构造函数，不允许出现重复元素。 |
| | **SortedVector**(const SortedVector< TYPE, true >& rhs)<br>拷贝构造函数，允许出现重复元素。 |
| | **SortedVector**(const std::vector< TYPE >& orivect) |
| virtual | **~SortedVector**()<br>析构函数。  |
| ssize_t | **Add**(const TYPE& item)<br>在正确的位置添加一个新元素`item`。  |
| const TYPE* | **Array**() const<br>返回一个指向vector的第一个元素的const指针，用于访问vector的元素。  |
| const TYPE& | **Back**() const<br>返回vector末尾元素的引用。  |
| iterator | **Begin**()<br>返回非const类型的vector起始元素的迭代器。  |
| const_iterator | **Begin**() const<br>返回const类型的vector起始元素的迭代器。  |
| size_t | **Capacity**() const<br>返回在分配新的存储空间之前能存储的元素总数。  |
| void | **Clear**()<br>清空vector.  |
| TYPE* | **EditArray**()<br>返回一个指向vector的第一个元素的非const指针，用于访问vector的元素。  |
| TYPE& | **EditItemAt**(size_t index)<br>返回vector的`index`对应的元素值。  |
| iterator | **End**()<br>返回非const类型的vector末尾元素的迭代器。  |
| const_iterator | **End**() const<br>返回const类型的vector末尾元素的迭代器。  |
| iterator | **Erase**(size_t index)<br>如果`index`小于vector的size，就删除`index`处的元素。  |
| const TYPE& | **Front**() const<br>返回vector起始元素的引用。  |
| ssize_t | **IndexOf**(const TYPE& item) const<br>查找vector中元素值为`item`的索引。  |
| bool | **IsEmpty**() const<br>返回vector是否为空。  |
| size_t | **Merge**(const SortedVector< TYPE, AllowDuplicate >& sortedVector) |
| size_t | **Merge**(const std::vector< TYPE >& invec)<br>将`invec`合并到`vec_`中。  |
| const TYPE& | **MirrorItemAt**(ssize_t index) const<br>返回vector的元素值。  |
| SortedVector< TYPE, AllowDuplicate >& | **operator=**(const SortedVector< TYPE, false >& rhs)<br>拷贝赋值运算符。  |
| SortedVector< TYPE, AllowDuplicate >& | **operator=**(const SortedVector< TYPE, true >& rhs) |
| const TYPE& | **operator[]**(size_t index) const<br>根据输入的索引去访问vector的元素。  |
| size_t | **OrderOf**(const TYPE& item) const<br>查找应插入这个值为`item`的位置。  |
| void | **PopBack**()<br>删除vector的最后一个元素。  |
| ssize_t | **SetCapcity**(size_t size)<br>设置vector的capacity为`size`。  |
| size_t | **Size**() const<br>返回vector的元素个数。  |

## 使用示例

1. 示例代码(伪代码)

```c++
#include <iostream>
#include "../include/sorted_vector.h"

using namespace OHOS;
using namespace std;

constexpr int SIZE = 4;

int main()
{
    vector<int> vec;
    for (int i = 0; i < SIZE; i++) {
        vec.push_back(i);
    }

    SortedVector<int, false> sv(vec);

    vector<int> newVec;
    newVec.push_back(7);
    newVec.push_back(4);
    newVec.push_back(3);
    newVec.push_back(5);
    newVec.push_back(6);

    sv.Merge(newVec);
    
    for (vector<int>::iterator it = sv.Begin(); it != sv.End(); it++) {
        cout << "Merged result: " << *it << endl;
    }

    sv.Clear();
    cout << "Cleared size: " << sv.Size() << endl;
}
```

2. 测试用例编译运行方法

- 测试用例代码参见base/test/unittest/common/utils_sorted_vector_test.cpp

- 使用开发者自测试框架，使用方法参见：[开发自测试执行框架-测试用例执行](https://gitee.com/openharmony/testfwk_developer_test#%E6%B5%8B%E8%AF%95%E7%94%A8%E4%BE%8B%E6%89%A7%E8%A1%8C)

- 使用以下具体命令以运行`sorted_vector.h`对应测试用例

```bash
run -t UT -tp utils -ts UtilsSortedVectorTest
```