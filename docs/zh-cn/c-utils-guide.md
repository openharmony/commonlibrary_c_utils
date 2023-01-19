# C++公共基础库(c_utils)开发指导

## C++公共基础库概述

### 简介

C++公共基础类库为标准系统提供了一些常用的C++开发工具类，包括：

* 文件、路径、字符串相关操作的能力增强接口。
* 读写锁、信号量、定时器、线程增强及线程池等接口。
* 安全数据容器、数据序列化等接口。
* 各子系统的错误码相关定义。

## 使用智能指针管理动态分配内存对象

### 概述

#### 简介

智能指针是行为类似指针的类，在模拟指针功能的同时提供增强特性，如针对具有动态分配内存对象的自动内存管理等。

* 自动内存管理主要是指对超出生命周期的对象正确并自动地释放其内存空间，以避免出现内存泄漏等相关内存问题。
* 智能指针对每一个RefBase对象具有两种不同的引用形式。强引用持有对一个对象的直接引用。 具有强引用关系的对象在该强引用关系存在时同样也应当存在，也即该引用关系有效；弱引用持有对一个对象的间接引用。 具有弱引用关系的对象在该弱引用关系存在时并不保证存在。

> 注意：上述描述仅当正确使用智能指针时才成立。

#### 实现原理

* 智能指针通过引用计数来实现所指向对象内存的自动管理。每一个可以被智能指针管理的对象都持有一个引用计数器，引用计数器在相关引用计数清0时会调用一个用于销毁该对象的回调函数。

* 引用计数器记录了对应RefBase对象的两种不同引用计数值，以及对于其本身，即RefCounter对象的引用计数值。

### 涉及功能

#### OHOS::sptr

**模块:** **SmartPointer**

指向RefBase(或其子类)对象的强引用智能指针。

##### 具体描述

```cpp
template <typename T >
class OHOS::sptr;
```

指向RefBase(或其子类)对象的强引用智能指针。

**模板参数**:

* **T** 被sptr管理的具体类型。该类必须继承自RefBase基类。

其直接引用RefBase对象。

`#include <refbase.h>`

##### 接口说明

| 返回类型                                    | 名称                                                                               |
| --------------------------------------- | -------------------------------------------------------------------------------- |
|                                         | **sptr**()                                                                       |
| template <typename O \> <br>            | **sptr**(const sptr< O > & other)<br>拷贝构造函数，参数与当前sptr具有不同的管理类型(O)                |
|                                         | **sptr**(const sptr< T > & other)<br>拷贝构造函数。其以参数指定具体管理对象                         |
|                                         | **sptr**(sptr< T > && other)<br>移动构造函数                                           |
|                                         | **sptr**(T * other)<br>构造函数。其以参数指定具体管理对象                                         |
|                                         | **sptr**(WeakRefCount * p, bool force)<br>构造函数。仅用于wptr的promote操作                 |
|                                         | **~sptr**()                                                                      |
| void                                    | **clear**()<br>移除当前sptr与所管理对象的引用关系                                               |
| void                                    | **ForceSetRefPtr**(T * other)<br>强制更改被管理对象指针的指向                                  |
| T *                                     | **GetRefPtr**() const<br>获取sptr管理对象的指针                                           |
|                                         | **operator T***() const<br>类型转换运算符                                               |
| bool                                    | **operator!**() const<br>逻辑非运算符。检查sptr对象是否为空sptr对象                               |
| bool                                    | **operator!=**(const sptr< T > & other) const<br>sptr对象间的不等运算符                   |
| bool                                    | **operator!=**(const T * other) const<br>sptr对象与裸指针间的不等运算符                       |
| bool                                    | **operator!=**(const wptr< T > & other) const<br>sptr对象与wptr间的相等运算符               |
| T &                                     | **operator***() const<br>解引用运算符，其会返回wptr管理的RefBae对象                              |
| T *                                     | **operator->**() const<br>成员选择运算符，其将会返回被sptr管理对象的指定成员                            |
| template <typename O \> <br>sptr< T > & | **operator=**(const sptr< O > & other)<br>拷贝赋值运算符，参数为一个sptr对象，但与当前sptr对象具有不同管理类型 |
| sptr< T > &                             | **operator=**(const sptr< T > & other)<br>拷贝赋值运算符，参数与当前sptr对象具有相同管理类型            |
| sptr< T > &                             | **operator=**(const wptr< T > & other)<br>拷贝赋值运算符，参数为一个wptr对象，但与当前sptr对象具有相同管理类型  |
| sptr< T > &                             | **operator=**(sptr< T > && other)<br>移动构造运算符                                     |
| sptr< T > &                             | **operator=**(T * other)<br>拷贝赋值运算符，参数为待管理的具体对象                                  |
| bool                                    | **operator==**(const sptr< T > & other) const<br>sptr对象间的相等运算符                   |
| bool                                    | **operator==**(const T * other) const<br>sptr对象与裸指针间的相等运算符                       |
| bool                                    | **operator==**(constwptr< T > & other) const<br>sptr对象与wptr间的相等运算符               |

#### OHOS::wptr

**模块:** **SmartPointer**

指向RefBase(或其子类)对象的弱引用智能指针。

##### 具体描述

```cpp
template <typename T >
class OHOS::wptr;
```

指向RefBase(或其子类)对象的弱引用智能指针。

**模板参数**:

* **T** 被wptr管理的具体类型。该类必须继承自RefBase基类。

其间接引用RefBase对象；直接引用WeakRefCounter对象。

`#include <refbase.h>`

##### 接口说明

| 返回类型                                    | 名称                                                                                   |
| --------------------------------------- | ------------------------------------------------------------------------------------ |
|                                         | **wptr**()                                                                           |
| template <typename O \> <br>            | **wptr**(const sptr< O > & other)<br>拷贝构造函数。参数为一个sptr对象，且与当前wptr对象具有不同的管理类型(O)       |
|                                         | **wptr**(const sptr< T > & other)<br>拷贝构造函数。参数为一个sptr对象，但与当前wptr对象具有相同的管理类型          |
| template <typename O \> <br>            | **wptr**(const wptr< O > & other)<br>拷贝构造函数。参数与当前wptr对象具有不同的管理类型                      |
|                                         | **wptr**(const wptr< T > & other)<br>拷贝构造函数。参数与当前wptr对象具有相同的管理类型                      |
|                                         | **wptr**(T * other)<br>构造函数。其以参数指定具体管理对象                                             |
|                                         | **~wptr**()                                                                          |
| bool                                    | **AttemptIncStrongRef**(const void * objectId) const<br>尝试对被管理对象的强引用计数加一             |
| T *                                     | **GetRefPtr**() const<br>获取指向被管理RefBase对象的指针                                         |
| bool                                    | **operator!=**(const sptr< T > & other) const<br>wptr与输入sptr对象间的不等运算符                |
| bool                                    | **operator!=**(const T * other) const<br>wptr对象与裸指针间的不等运算符                           |
| bool                                    | **operator!=**(constwptr< T > & other) const<br>wptr对象间的不等运算符                        |
| T &                                     | **operator***() const<br>解引用运算符，其会返回wptr管理的RefBae对象                                  |
| T *                                     | **operator->**() const<br>成员选择操作符，其将会返回被wptr管理对象的指定成员                                |
| template <typename O \> <br>wptr< T > & | **operator=**(const sptr< O > & other)<br>拷贝赋值运算符，参数为一个sptr对象，但与当前wptr对象具有不同的管理类型(O) |
| wptr< T > &                             | **operator=**(const sptr< T > & other)<br>拷贝赋值运算符，参数为一个sptr对象，但与当前wptr对象具有相同的管理类型(T) |
| template <typename O \> <br>wptr< T > & | **operator=**(const wptr< O > & other)<br>拷贝赋值运算符，参数为一个wptr对象，但与当前wptr对象具有不同的管理类型(O)  |
| wptr< T > &                             | **operator=**(const wptr< T > & other)<br>拷贝赋值运算符，参数为一个wptr对象，且与当前wptr对象具有相同的管理类型(T)  |
| template <typename O \> <br>wptr< T > & | **operator=**(O * other)<br>拷贝赋值运算符，参数为待管理的具体对象                                      |
| wptr< T > &                             | **operator=**(T * other)<br>拷贝赋值运算符，参数为待管理的具体对象                                      |
| bool                                    | **operator==**(const sptr< T > & other) const<br>wptr与输入sptr对象间的相等运算符                |
| bool                                    | **operator==**(const T * other) const<br>wptr对象与裸指针间的相等运算符                           |
| bool                                    | **operator==**(const wptr< T > & other) const<br>wptr对象间的相等运算符                        |
| const sptr< T >                         | **promote**() const                                                                  |

### 使用示例

1. 引入智能指针功能头文件

   ```c++
   #include "refbase.h"
   ```

2. 各种使用方式

2.1 示例待管理类定义
```c++
class RefBaseTest : RefBase
{
   virtual void show()
   {
      cout<<"Show RefBaseTest"<<endl;
   }
}
```

```c++
class SubRefBaseTest : RefBaseTest
{
   void show()
   {
         cout<<"Show SubRefBaseTest"<<endl;
   }
}
```
2.2 基本用法
```c++
// 使用新创建智能指针，管理新创建对象
sptr<RefBaseTest> newSptr(new RefBaseTest);
wptr<RefBaseTest> newWptr(new RefBaseTest);

// 使用现存智能指针，管理新创建对象
newSptr = new RefBaseTest();
newWptr = new RefBaseTest();

// 使用新创建智能指针，指向其他现存智能指针管理对象
sptr<RefBaseTest> anotherSptr(new RefBaseTest);
wptr<RefBaseTest> anotherWptr(new RefBaseTest);
sptr<RefBaseTest> curSptr(anotherSptr);
wptr<RefBaseTest> curWptr(anotherWptr);

// 使用现存智能指针，指向其他现存智能指针管理对象
newSptr = anotherSptr;
newWptr = anotherWptr;

// 某指定类型的智能指针可以管理该类型的子类对象
Sptr<SubRefBaseTest> subSptr(new SubRefBaseTest());
Wptr<SubRefBaseTest> subWptr(new SubRefBaseTest());
curSptr = subSptr;
curWptr = subWptr;
// 或
Sptr<RefBaseTest> superSptr(subSptr);
Wptr<RefBaseTest> superSptr(subWptr);
// 可通过->运算符访问成员
curSptr->show();
curWptr->show();

// 可通过*运算符解引用
(*curSptr).show();
(*curSptr).show();
```

2.3 特殊用法

```c++
// 两种智能指针可以管理对方所管理的对象
sptr<RefBaseTest> scurSptr(new RefBaseTest);
wptr<RefBaseTest> scurWptr(new RefBaseTest);

wptr<RefBaseTest> snewWptr(scurSptr);
sptr<RefBaseTest> snewSptr(scurWptr);
// 或
sptr<RefBaseTest> soldSptr(new RefBaseTest);
wptr<RefBaseTest> soldWptr(new RefBaseTest);
soldSptr = scurWptr; // 原本的引用关系将被释放
soldWptr = scurSptr; // 同上

// wptr可升级为sptr
sptr<RefBaseTest> spromotedWptr = snewWptr.promote(); // 升级失败时返回空sptr对象，即未管理具体对象的sptr对象
```

### 常见问题

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

3. **智能指针不保证线程安全**，使用者需保证线程安全以避免对同一个sptr对象赋值等操作

4. **避免通过隐式转换构造智能指针对象**

   * 易造成误解。
   * 因编译器优化程度具有不确定的行为，易造成问题。
