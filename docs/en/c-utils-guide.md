# Developement Guidelines for c_utils

## Overview

### Introduction

The commonlibrary/c_utils repository provides the following commonly used C++ utility classes for standard system:

* Enhanced APIs for operations related to files, paths, and strings.
* APIs related to the read-write lock, semaphore, timer, thread, and thread pool.
* APIs related to the security data container and data serialization.
* Error codes for each subsystem.

## Use Smart-Pointer to manag dynamically allocated object

### Overview

#### Introduction

Smart Pointer are pointer-like classes. They simulates a pointer while providing added features, such as automatic memory management.

* Automatic memory management is mainly about deallocating the related memory at the correct time when an object beyonds its life cycle.
* There are two different references for a single object. Strong Reference holds a pointer directly pointing to the object. Objects which are strong referenced ought to be alive/existed as long as these strong references exists, thus the references are still valid; Weak Reference holds a pointer indirectly pointing to the object. Objects which are weak referenced are not guaranteed to be alive/existed even if their weak references exist.

> Notice: Descriptions above are valid only when smart pointers are properly used.

#### Principle

* Via reference counts, Smart-Pointer achieves auto-management of memory for the object pointed by it. Every object which can be managed holds a reference counter. The counter will destroy the object through a callback method when related counts reach 0.

* Reference counter records two kinds of counts of references to the corresponded RefBase object, and a count of reference to the RefCounter itself.

### Related Interfaces
#### OHOS::sptr

Strong reference smart pointer to a RefBase(or its subclass) object.

##### Detailed Description

```cpp
template <typename T >
```

Strong reference smart pointer to a RefBase(or its subclass) object.
**Template Parameters**:

* **T** Specific class type managed by sptr. This class must inherit from RefBase.

Directly reference the RefBase object.

`#include <refbase.h>`

##### Public Functions

| Return Type                             | Name                                                                                                                         |
| --------------------------------------- | ---------------------------------------------------------------------------------------------------------------------------- |
|                                         | **sptr**()                                                                                                                   |
| template <typename O \> <br>            | **sptr**(const sptr< O > & other)<br>Copy Constructor for sptr with different managed class type(O)                          |
|                                         | **sptr**(const sptr< T > & other)<br>Copy Constructor for sptr with different managed class type(T)                          |
|                                         | **sptr**(sptr< T > && other)<br>Move constructor.                                                                            |
|                                         | **sptr**(T * other)<br>Constructor with specified object to be managed.                                                      |
|                                         | **sptr**(WeakRefCounter * p, bool force)<br>Constructor only used in promote process of wptr.                                |
|                                         | **~sptr**()                                                                                                                  |
| void                                    | **clear**()<br>Remove the reference to the managed object held by current sptr.                                              |
| void                                    | **ForceSetRefPtr**(T * other)<br>Set the pointer to the managed object.                                                      |
| T *                                     | **GetRefPtr**() const<br>Get the pointer to the managed object.                                                              |
|                                         | **operator T***() const<br>Type conversion operator.                                                                         |
| bool                                    | **operator!**() const<br>Logical-NOT operator. Check if sptr is a "null sptr".                                               |
| bool                                    | **operator!=**(const sptr< T > & other) const<br>Not-equal-to operator between sptrs.                                        |
| bool                                    | **operator!=**(const T * other) const<br>Not-equal-to operator between sptr and a raw pointer.                               |
| bool                                    | **operator!=**(const wptr< T > & other) const<br>Not-equal-to operator between sptr and a wptr.                              |
| T &                                     | **operator***() const<br>Dereference operator. It will return the object managed by this sptr.                               |
| T *                                     | **operator->**() const<br>Member selection operator. It will return the specified member of the object managed by this sptr. |
| template <typename O \> <br>sptr< T > & | **operator=**(const sptr< O > & other)<br>Copy assignment operator for sptr with different managed class type(O)             |
| sptr< T > &                             | **operator=**(const sptr< T > & other)<br>Copy assignment operator for sptr with same managed class type(T)                  |
| sptr< T > &                             | **operator=**(const wptr< T > & other)<br>Copy assignment operator for wptr with same managed class type(T)                  |
| sptr< T > &                             | **operator=**(sptr< T > && other)<br>Move assignment operator.                                                               |
| sptr< T > &                             | **operator=**(T * other)<br>Copy assignment operator with specified object to be managed.                                    |
| bool                                    | **operator==**(const sptr< T > & other) const<br>Equal-to operator between sptrs.                                            |
| bool                                    | **operator==**(const T * other) const<br>Equal-to operator between sptr and a raw pointer.                                   |
| bool                                    | **operator==**(const wptr< T > & other) const<br>Equal-to operator between sptr and a wptr.                                  |

#### OHOS::wptr

Weak reference smart pointer to a RefBase(or its subclass) object.

##### Detailed Description

```cpp
template <typename T >
```

Weak reference smart pointer to a RefBase(or its subclass) object.

**Template Parameters**:

* **T** Specific class type managed by wptr. This class must inherit from RefBase.

It indirectly references the RefBase object, and directly references the WeakRefCounter object.

`#include <refbase.h>`

##### Public Functions

| Return Type                                                             | Name                                                                                                                                             |
| ----------------------------------------------------------------------- | ------------------------------------------------------------------------------------------------------------------------------------------------ |
|                                                                         | **wptr**()                                                                                                                                       |
| template <typename O \> <br>                                            | **wptr**(const sptr< O > & other)<br>Copy constructor for sptr with different managed class type(O)                                              |
|                                                                         | **wptr**(const sptr< T > & other)<br>Copy constructor for sptr with same managed class type(T)                                                   |
| template <typename O \> <br>                                            | **wptr**(const wptr< O > & other)<br>Copy constructor for wptr with different managed class type(O)              |
|                                                                         | **wptr**(const wptr< T > & other)<br>Copy constructor for wptr with same managed class type(T)                   |
|                                                                         | **wptr**(T * other)<br>Constructor with specified object to be managed.                                                                          |
|                                                                         | **~wptr**()                                                                                                                                      |
| bool                                                                    | **AttemptIncStrongRef**(const void * objectId) const<br>Attempt to increment the strong reference count of the managed object.                   |
| T *                                                                     | **GetRefPtr**() const<br>Get the pointer to the RefBase object.                                                                                  |
| bool                                                                    | **operator!=**(const sptr< T > & other) const<br>Not-Equal-to operator between wptr and a input sptr object.                                     |
| bool                                                                    | **operator!=**(const T * other) const<br>Not-equal-to operator between wptr and a raw pointer.                                                   |
| bool                                                                    | **operator!=**(const wptr< T > & other) const<br>Not-equal-to operator between two wptrs.                        |
| T &                                                                     | **operator***() const<br>Dereference operator. It will return the object managed by this wptr.                                                   |
| T *                                                                     | **operator->**() const<br>Member selection operator. It will return the specified member of the object managed by this wptr.                     |
| template <typename O \> <br>wptr< T > & | **operator=**(const sptr< O > & other)<br>Copy assignment operator for sptr with different managed class type(O)                                 |
| wptr< T > &                             | **operator=**(const sptr< T > & other)<br>Copy assignment operator for sptr with same managed class type(T)                                      |
| template <typename O \> <br>wptr< T > & | **operator=**(const wptr< O > & other)<br>Copy assignment operator for wptr with different managed class type(O) |
| wptr< T > &                             | **operator=**(const wptr< T > & other)<br>Copy assignment operator for wptr with same managed class type(T)      |
| template <typename O \> <br>wptr< T > & | **operator=**(O * other)<br>Copy assignment operator with specified object to be managed.                                                        |
| wptr< T > &                             | **operator=**(T * other)<br>Copy assignment operator with specified object to be managed.                                                        |
| bool                                                                    | **operator==**(const sptr< T > & other) const<br>Equal-to operator between wptr and a input sptr object.                                         |
| bool                                                                    | **operator==**(const T * other) const<br>Equal-to operator between wptr and a raw pointer.                                                       |
| bool                                                                    | **operator==**(const wptr< T > & other) const<br>Equal-to operator between two wptrs.                            |
| const sptr< T >                                                         | **promote**() const<br>Promote a wptr to an sptr.                                                                                                |

### Example

1. Include the Header File

```c++
#include "refbase.h"
```

2. Usages
2.1 Definition of Example Target Class
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
2.2 Basic Usages

```c++

sptr<RefBaseTest> newSptr(new RefBaseTest);
wptr<RefBaseTest> newWptr(new RefBaseTest);

newSptr = new RefBaseTest();
newWptr = new RefBaseTest();

sptr<RefBaseTest> anotherSptr(new RefBaseTest);
wptr<RefBaseTest> anotherWptr(new RefBaseTest);
sptr<RefBaseTest> curSptr(anotherSptr);
wptr<RefBaseTest> curWptr(anotherWptr);

newSptr = anotherSptr;
newWptr = anotherWptr;

Sptr<SubRefBaseTest> subSptr(new SubRefBaseTest());
Wptr<SubRefBaseTest> subWptr(new SubRefBaseTest());
curSptr = subSptr;
curWptr = subWptr;
// or
Sptr<RefBaseTest> superSptr(subSptr);
Wptr<RefBaseTest> superSptr(subWptr);

curSptr->show();
curWptr->show();

(*curSptr).show();
(*curSptr).show();
```

2.3 Special Usages

```c++

sptr<RefBaseTest> scurSptr(new RefBaseTest);
wptr<RefBaseTest> scurWptr(new RefBaseTest);

wptr<RefBaseTest> snewWptr(scurSptr);
sptr<RefBaseTest> snewSptr(scurWptr);
// or
sptr<RefBaseTest> soldSptr(new RefBaseTest);
wptr<RefBaseTest> soldWptr(new RefBaseTest);
soldSptr = scurWptr; // Original reference will be eliminated
soldWptr = scurSptr;

// wptr -> sptr
sptr<RefBaseTest> spromotedWptr = snewWptr.promote(); // Original weak reference still exists
```

### FAQ

1. **Avoid using it combined with raw pointer and smart pointer(std::shared_ptr, etc.) in `std` simultaneously**

   * It will cause conflict in management，and thus undefined behavior.
     * Using smart pointer to manage a valid raw pointer is not recommended.

```c++
RefBase* a = new RefBase();
sptr<RefBase> s = a;
// or
sptr<RefBase> s(a); // raw pointer is easily mistakenly deleted
```

2. **Smart pointer should be constructed on the stack，object to be managed should on the heap(dynamically allocated)**

   * Smart pointer on the heap doesn't conform with the definition.
   * Object on the stack will be destroyed automatically, which makes the object bypass the management from smart pointer.

3. **Smart point is not ThreadSafe**

4. **Avoid constructing smart pointer via implicit conversion**

   * Easily mislead other developers.
   * Depend on specific compiler, exact behaviors can not be guaranteed.
