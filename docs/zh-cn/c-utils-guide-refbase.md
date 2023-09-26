# 使用智能指针管理动态分配内存对象

## 概述

智能指针是行为类似指针的类，在模拟指针功能的同时提供增强特性，如针对具有动态分配内存对象的自动内存管理等。

* 自动内存管理主要是指对超出生命周期的对象正确并自动地释放其内存空间，以避免出现内存泄漏等相关内存问题。
* 智能指针对每一个RefBase对象具有两种不同的引用形式。强引用持有对一个对象的直接引用。 具有强引用关系的对象在该强引用关系存在时同样也应当存在，也即该引用关系有效；弱引用持有对一个对象的间接引用。 具有弱引用关系的对象在该弱引用关系存在时并不保证存在。

> 注意：上述描述仅当正确使用智能指针时才成立。

### 实现原理

* 智能指针通过引用计数来实现所指向对象内存的自动管理。每一个可以被智能指针管理的对象都持有一个引用计数器，引用计数器在相关引用计数清0时会调用一个用于销毁该对象的回调函数。

* 引用计数器记录了对应RefBase对象的两种不同引用计数值，以及对于其本身，即RefCounter对象的引用计数值。

## 涉及功能

### OHOS::sptr

**模块:** **SmartPointer**

指向RefBase(或其子类)对象的强引用智能指针。

#### 具体描述

```cpp
template <typename T >
class OHOS::sptr;
```

指向RefBase(或其子类)对象的强引用智能指针。

**模板参数**:

* **T** 被sptr管理的具体类型。该类必须继承自RefBase基类。

其直接引用RefBase对象。

`#include <refbase.h>`

#### 接口说明

| 返回类型                                    | 名称                                                                               |
| --------------------------------------- | -------------------------------------------------------------------------------- |
|                                         | **sptr**()                                                                       |
|sptr< T >                                | template <typename... Args><br>**MakeSptr**(Args&&... args)<br>构造T类型的被管理对象并创建sptr管控，传递参数args为T类型构造函数所需参数<br> **注意:强烈建议使用该方法构造sptr并管控对象，可以避免对象指针对外暴露，将对象的生命周期完全处于智能指针的管控之下**   |
| template <typename O \> <br>            | **sptr**(const sptr< O >& other)<br>拷贝构造函数，参数与当前sptr具有不同的管理类型(O)                |
|                                         | **sptr**(const sptr< T >& other)<br>拷贝构造函数。其以参数指定具体管理对象                         |
|                                         | **sptr**(sptr< T >&& other)<br>移动构造函数                                           |
|                                         | **sptr**(T* other)<br>构造函数。其以参数指定具体管理对象<br> **注意: 不建议使用对象指针的形式构造sptr对象，这会导致对象的生命周期不完全在sptr的看护下，很可能误用造成对象提前释放**                                         |
|                                         | **sptr**(WeakRefCount* p, bool force)<br>构造函数。仅用于wptr的promote操作                 |
|                                         | **~sptr**()                                                                      |
| void                                    | **clear**()<br>移除当前sptr与所管理对象的引用关系                                               |
| void                                    | **ForceSetRefPtr**(T* other)<br>强制更改被管理对象指针的指向                                  |
| T*                                     | **GetRefPtr**() const<br>获取sptr管理对象的指针                                           |
|                                         | **operator T***() const<br>类型转换运算符                                               |
|                                    | **operator bool**() const<br>布尔类型转换运算符。检查sptr对象是否为空对象                               |
| bool                                    | **operator!=**(const sptr< T >& other) const<br>sptr对象间的不等运算符                   |
| bool                                    | **operator!=**(const T* other) const<br>sptr对象与裸指针间的不等运算符                       |
| bool                                    | **operator!=**(const wptr< T >& other) const<br>sptr对象与wptr间的相等运算符               |
| T&                                     | **operator***() const<br>解引用运算符，其会返回wptr管理的RefBae对象                              |
| T*                                     | **operator->**() const<br>成员选择运算符，其将会返回被sptr管理对象的指定成员                            |
| template <typename O \> <br>sptr< T >& | **operator=**(const sptr< O >& other)<br>拷贝赋值运算符，参数为一个sptr对象，但与当前sptr对象具有不同管理类型 |
| sptr< T >&                             | **operator=**(const sptr< T >& other)<br>拷贝赋值运算符，参数与当前sptr对象具有相同管理类型            |
| sptr< T >&                             | **operator=**(const wptr< T >& other)<br>拷贝赋值运算符，参数为一个wptr对象，但与当前sptr对象具有相同管理类型  |
| sptr< T >&                             | **operator=**(sptr< T >&& other)<br>移动构造运算符                                     |
| sptr< T >&                             | **operator=**(T* other)<br>拷贝赋值运算符，参数为待管理的具体对象<br>**注意: 不建议以指针赋值的形式创建sptr对象，这会导致对象的生命周期不完全在sptr的看护下，很可能误用造成对象提前释放**                                  |
| bool                                    | **operator==**(const sptr< T >& other) const<br>sptr对象间的相等运算符                   |
| bool                                    | **operator==**(const T* other) const<br>sptr对象与裸指针间的相等运算符                       |
| bool                                    | **operator==**(constwptr< T >& other) const<br>sptr对象与wptr间的相等运算符               |

### OHOS::wptr

**模块:** **SmartPointer**

指向RefBase(或其子类)对象的弱引用智能指针。

#### 具体描述

```cpp
template <typename T >
class OHOS::wptr;
```

指向RefBase(或其子类)对象的弱引用智能指针。

**模板参数**:

* **T** 被wptr管理的具体类型。该类必须继承自RefBase基类。

其间接引用RefBase对象；直接引用WeakRefCounter对象。

`#include <refbase.h>`

#### 接口说明

| 返回类型                                    | 名称                                                                                   |
| --------------------------------------- | ------------------------------------------------------------------------------------ |
|                                         | **wptr**()                                                                           |
| template <typename O \> <br>            | **wptr**(const sptr< O >& other)<br>拷贝构造函数。参数为一个sptr对象，且与当前wptr对象具有不同的管理类型(O)       |
|                                         | **wptr**(const sptr< T >& other)<br>拷贝构造函数。参数为一个sptr对象，但与当前wptr对象具有相同的管理类型          |
| template <typename O \> <br>            | **wptr**(const wptr< O >& other)<br>拷贝构造函数。参数与当前wptr对象具有不同的管理类型                      |
|                                         | **wptr**(const wptr< T >& other)<br>拷贝构造函数。参数与当前wptr对象具有相同的管理类型                      |
|                                         | **wptr**(T* other)<br>构造函数。其以参数指定具体管理对象                                             |
|                                         | **~wptr**()                                                                          |
| bool                                    | **AttemptIncStrongRef**(const void* objectId) const<br>尝试对被管理对象的强引用计数加一             |
| T*                                     | **GetRefPtr**() const<br>获取指向被管理RefBase对象的指针                                         |
| bool                                    | **operator!=**(const sptr< T >& other) const<br>wptr与输入sptr对象间的不等运算符                |
| bool                                    | **operator!=**(const T* other) const<br>wptr对象与裸指针间的不等运算符                           |
| bool                                    | **operator!=**(constwptr< T >& other) const<br>wptr对象间的不等运算符                        |
| T&                                     | **operator***() const<br>解引用运算符，其会返回wptr管理的RefBae对象                                  |
| T*                                     | **operator->**() const<br>成员选择操作符，其将会返回被wptr管理对象的指定成员                                |
| template <typename O \> <br>wptr< T >& | **operator=**(const sptr< O >& other)<br>拷贝赋值运算符，参数为一个sptr对象，但与当前wptr对象具有不同的管理类型(O) |
| wptr< T >&                             | **operator=**(const sptr< T >& other)<br>拷贝赋值运算符，参数为一个sptr对象，但与当前wptr对象具有相同的管理类型(T) |
| template <typename O \> <br>wptr< T >& | **operator=**(const wptr< O >& other)<br>拷贝赋值运算符，参数为一个wptr对象，但与当前wptr对象具有不同的管理类型(O)  |
| wptr< T >&                             | **operator=**(const wptr< T >& other)<br>拷贝赋值运算符，参数为一个wptr对象，且与当前wptr对象具有相同的管理类型(T)  |
| template <typename O \> <br>wptr< T >& | **operator=**(O* other)<br>拷贝赋值运算符，参数为待管理的具体对象                                      |
| wptr< T >&                             | **operator=**(T* other)<br>拷贝赋值运算符，参数为待管理的具体对象                                      |
| bool                                    | **operator==**(const sptr< T >& other) const<br>wptr与输入sptr对象间的相等运算符                |
| bool                                    | **operator==**(const T* other) const<br>wptr对象与裸指针间的相等运算符                           |
| bool                                    | **operator==**(const wptr< T >& other) const<br>wptr对象间的相等运算符                        |
| const sptr< T >                         | **promote**() const<br>将该wptr提升为sptr                                                   |

## 使用示例

1. 使用方法(伪代码)

```
#include "../include/refbase.h"
#include <iostream>

using namespace std;
using namespace OHOS;

// 管理目标类
class RefBaseTest : public RefBase {
public:
    virtual void Access()
    {
        cout<<"Access RefBaseTest::Show"<<endl;
    }

    ~RefBaseTest() override
    {
        cout << "RefBaseTest destroyed" << endl;
    }
};

// 管理目标类的子类
class SubRefBaseTest : public RefBaseTest {
public:
    void Access() override
    {
        cout<<"Access SubRefBaseTest::Show"<<endl;
    }

    ~SubRefBaseTest() override
    {
        cout << "SubRefBaseTest destroyed" << endl;
    }
};

int main()
{
    // 1. 使用新创建智能指针，管理新创建对象
    sptr<RefBaseTest> newSptr(new RefBaseTest());
    wptr<RefBaseTest> newWptr(new RefBaseTest());

    // 2. 使用上述智能指针，管理另一个新创建对象
    // 原管理对象析构
    newSptr = new RefBaseTest();
    newWptr = new RefBaseTest();

    // 3. 使用新创建智能指针，指向其他现存智能指针管理对象
    sptr<RefBaseTest> curSptr(newSptr);
    wptr<RefBaseTest> curWptr(newWptr);

    if (curSptr->GetSptrRefCount() == 2 && curSptr->GetWptrRefCount() == 2 && // 2: count
       curWptr->GetWptrRefCount() == 1) {
        cout << "curSptr run as expected" << endl;
    }

    // 4. 使用现存智能指针管理其托管类型的子类对象
    sptr<SubRefBaseTest> subSptr(new SubRefBaseTest());
    wptr<SubRefBaseTest> subWptr(new SubRefBaseTest());

    curSptr = subSptr;
    curWptr = subWptr;

    // 5. 通过->运算符访问成员"
    curSptr->Access();
    curWptr->Access();

    // 6. 通过*运算符解引用
    (*curSptr).Access();
    (*curSptr).Access();

    // 7. 两种智能指针可以管理对方所管理的对象
    sptr<RefBaseTest> scurSptr(new RefBaseTest);
    wptr<RefBaseTest> scurWptr(new RefBaseTest);

    wptr<RefBaseTest> snewWptr(scurSptr);

    sptr<RefBaseTest> soldSptr(new RefBaseTest);
    wptr<RefBaseTest> soldWptr(new RefBaseTest);
    soldSptr = scurWptr; // sptr仅可通过拷贝赋值管理wptr所管理对象
    soldWptr = scurSptr; // 原本的引用关系将被释放

    if (snewWptr->GetWptrRefCount() == 3 && soldSptr->GetSptrRefCount() == 1 && // 3: count
        soldWptr->GetWptrRefCount() == 3) { // 3: count
            cout << "Smart Pointer assignment run as expected" << endl;
        }
    // 8. wptr可升级为sptr
    sptr<RefBaseTest> spromotedWptr = snewWptr.promote(); // 升级失败时返回空sptr对象，即未管理具体对象的sptr对象
    if (spromotedWptr->GetSptrRefCount() == 2 && spromotedWptr->GetWptrRefCount() == 4) { // 2, 4: count
        cout << "Promote run as expected" << endl;
    }

    return 0;
}
```

2. 测试用例编译运行方法

- 测试用例代码参见 base/test/unittest/common/utils_refbase_test.cpp

- 使用开发者自测试框架，使用方法参见：[开发自测试执行框架-测试用例执行](https://gitee.com/openharmony/testfwk_developer_test#%E6%B5%8B%E8%AF%95%E7%94%A8%E4%BE%8B%E6%89%A7%E8%A1%8C)

- 使用以下具体命令以运行`refbase.h`对应测试用例

```bash
run -t UT -tp utils -ts UtilsRefBaseTest
```

3. debug功能
RefTracker作为debug工具被添加入refbase文件中，以便开发者对RefBase相关问题进行定位。该功能需要重新编译动态库替换系统原有动态库来上机使用（如是静态依赖则需开发者独立审视使能方法）。
- 全局追踪

全局追踪功能通过编译宏控制，可以追踪全局的RefBase及其子类的轨迹，但同时会对整机性能造成影响。
全局追踪中我们提供了立即打印模式及非立即打印模式。立即打印模式会在每次引用计数发生变化时对计数进行打印。非立即打印模式会在RefBase及其子类对象析构时对轨迹进行打印。

全局追踪、立即打印编译命令：
```
./build.sh --product-name xxx --ccache --build-target commonlibrary/c_utils/base:utils --gn-args c_utils_debug_refbase=true --gn-args c_utils_track_all=true --gn-args c_utils_print_track_at_once=true
```
全局追踪、非立即打印编译命令：
```
./build.sh --product-name xxx --ccache --build-target commonlibrary/c_utils/base:utils --gn-args c_utils_debug_refbase=true --gn-args c_utils_track_all=true
```
- 独立追踪

独立追踪功能同样能通过编译宏控制。我们为开发者提供了RefBase::EnableTracker()接口来对某个具体实例使能追踪功能。独立追踪对性能影响很小，可以忽略不计。在独立追踪中我们能同样提供了立即打印及非立即打印模式。

独立追踪、立即打印编译命令：
```
./build.sh --product-name xxx --ccache --build-target commonlibrary/c_utils/base:utils --gn-args c_utils_debug_refbase=true --gn-args c_utils_print_track_at_once=true
```
独立追踪、非立即打印编译命令：
```
./build.sh --product-name xxx --ccache --build-target commonlibrary/c_utils/base:utils --gn-args c_utils_debug_refbase=true
```

- 使用方法

编译动态库，编译产物路径为`./out/xxx/commonlibrary/c_utils/libutils.z.so`。

编译产物需要推入系统进行替换，64位系统位于`/system/lib64/`，32位系统位于`/system/lib/`。

追踪结果通过log打印。格式如下：
```
// 立即打印
(sptr pointer) start tracking
(sptr pointer) call (RefBase pointer). strong: x weak: x refcnnt: x
...
(sptr pointer) call (RefBase pointer). strong: x weak: x refcnnt: x
(sptr pointer) end tracking

// 非立即打印
(sptr pointer) start backtrace
(sptr pointer) call (RefBase pointer). strong: x weak: x refcnnt: x PID: xxx TID: xxx
...
(sptr pointer) call (RefBase pointer). strong: x weak: x refcnnt: x PID: xxx TID: xxx
(sptr pointer) end backtrace
```

## 常见问题

1. **使用本实现智能指针时，同时使用裸指针或std标准库智能指针(std::shared_ptr)**

   * 会造成管理冲突，导致非法访问以及未定义行为，如内存重复释放。
     * 因此也不推荐先创建裸指针后，再使用智能指针管理。
```c++
RefBase* a = new RefBase();
sptr<RefBase> s = a;
// 或
sptr<RefBase> s(a); // 裸指针a容易被误delete,造成sptr功能失常
```

2. **智能指针需构造在栈上，管理的对象需要在堆上(动态分配对象)**

   * 智能指针若构造在堆上则不符合定义。
   * 管理对象若构造在栈上，则会自动释放，错误绕开智能指针管控。

3. **智能指针不保证线程安全**，使用者需保证线程安全以避免同时对同一个sptr对象赋值等操作

4. **避免通过隐式转换构造智能指针对象**

   * 易造成误解。
   * 因编译器优化程度具有不确定的行为，易造成问题。

5. **不建议使用对象指针构造智能指针对象**

   * 外部提前以指针形式释放对象后，继续通过智能指针中使用
   * sptr引用计数为0释放对象，对象指针依旧在外被继续使用

```cpp
Refbase *a = new Refbase(arg1, arg2);
sptr<Refbase> sp1 = a; // 不建议，对象指针a暴露在外，存在风险
sptr<Refbase> sp2(a); // 不建议，对象指针a暴露在外，存在风险
sptr<Refbase> sp3 = sptr<Refbase>::MakeSptr(arg1, arg2); // 建议，在内部构造Refbase对象，直接交与sptr管控使用
```
6. **wptr使用注意**

   * 在未设置**ExtendObjectLifetime**的情况下，wptr不参与被管理对象的生命周期控制，对象生命周期由sptr的引用计数控制，但在极特殊情况下存在例外

```cpp
// 由于历史设计原因，可以在sptr不存在的情况下，基于对象指针创建wptr对象。
// 在未设置ExtendObjectLifetime，且无sptr被创建的特殊少见情况下，为了防止内存泄漏，在wptr引用计数归0时会释放管理对象
Refbase *a = new Refbase(arg1, arg2);
wptr<Refbase> wp1(a);
wp1 = nullptr; // 弱引用计数归0，对象释放，应避免再次手动释放

wptr<Refbase> wp2 = new Refbase(arg1, arg2);
wp2 = nullptr; // 弱引用计数归0，对象释放，这种情况无法手动释放, 如果wptr不能控制对象释放则必然会发生内存泄漏
```