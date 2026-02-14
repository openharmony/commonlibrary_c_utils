/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef UTILS_BASE_THREAD_SAFETY_MACROS_H
#define UTILS_BASE_THREAD_SAFETY_MACROS_H

#ifdef __cplusplus
extern "C" {
#endif

#if defined(__clang__)
// Clang compiler detected: enable thread safety attribute macros
#define THREAD_ANNOTATION_ATTRIBUTE__(x) __attribute__((x))
#else
// Non-Clang compiler: define macros as no-ops for compatibility
#define THREAD_ANNOTATION_ATTRIBUTE__(x)  /* no-op */
#endif

// Marks that a class/struct is a lockable capability (e.g., a Mutex).
// Use as: class Foo CAPABILITY("mutex") { ... };
#define CAPABILITY(x)           THREAD_ANNOTATION_ATTRIBUTE__(capability(x))

// Marks a lockable class that is reentrant (recursive lock allowed).
#define REENTRANT_CAPABILITY    THREAD_ANNOTATION_ATTRIBUTE__(reentrant_capability)

// Marks an RAII class (e.g., lock guard) that acquires a capability in its constructor
// and releases it in its destructor.
#define SCOPED_CAPABILITY       THREAD_ANNOTATION_ATTRIBUTE__(scoped_lockable)

// Marks that a shared variable (*member*) is guarded by a mutex.
// Any access to the variable should be performed with the mutex held.
#define GUARDED_BY(x)           THREAD_ANNOTATION_ATTRIBUTE__(guarded_by(x))

// Similar to GUARDED_BY, but for pointer variables. Indicates that the
// data pointed to by this pointer is protected by the given mutex.
#define PT_GUARDED_BY(x)        THREAD_ANNOTATION_ATTRIBUTE__(pt_guarded_by(x))

// Declares a lock ordering: this capability must always be acquired before the listed one(s).
// Use on a mutex/capability to prevent deadlock by enforcing a consistent acquisition order.
#define ACQUIRED_BEFORE(...)    THREAD_ANNOTATION_ATTRIBUTE__(acquired_before(__VA_ARGS__))

// Declares a lock ordering: this capability must always be acquired after the listed one(s).
// Use on a mutex/capability to enforce that it is taken only after the specified lock(s).
#define ACQUIRED_AFTER(...)     THREAD_ANNOTATION_ATTRIBUTE__(acquired_after(__VA_ARGS__))

// Function/METHOD pre-condition: exclusive lock(s) required to be held on entry.
// Specify one or more mutexes that must be locked by the caller before calling.
#define REQUIRES(...)           THREAD_ANNOTATION_ATTRIBUTE__(requires_capability(__VA_ARGS__))

// Function/METHOD pre-condition: shared lock(s) required (reader lock) on entry.
#define REQUIRES_SHARED(...)    THREAD_ANNOTATION_ATTRIBUTE__(requires_shared_capability(__VA_ARGS__))

// Function annotation: the function will acquire (lock) the given mutex(es).
// Should be used on methods that internally lock a mutex (exclusive).
#define ACQUIRE(...)            THREAD_ANNOTATION_ATTRIBUTE__(acquire_capability(__VA_ARGS__))

// Function annotation: the function will acquire (lock) the given mutex(es) in shared (reader) mode.
#define ACQUIRE_SHARED(...)     THREAD_ANNOTATION_ATTRIBUTE__(acquire_shared_capability(__VA_ARGS__))

// Function annotation: the function releases (unlocks) the given mutex(es) (previously exclusively held).
#define RELEASE(...)            THREAD_ANNOTATION_ATTRIBUTE__(release_capability(__VA_ARGS__))

// Function annotation: the function releases (unlocks) the given mutex(es) held in shared mode.
#define RELEASE_SHARED(...)     THREAD_ANNOTATION_ATTRIBUTE__(release_shared_capability(__VA_ARGS__))

// Function annotation: the function releases a mutex (either exclusive or shared).
// Use this for generic unlock functions that may unlock either mode.
#define RELEASE_GENERIC(...)    THREAD_ANNOTATION_ATTRIBUTE__(release_generic_capability(__VA_ARGS__))

// Function annotation: the function will try to acquire the mutex(es).
// The first parameter is a boolean indicating if a true return value means success.
#define TRY_ACQUIRE(...)        THREAD_ANNOTATION_ATTRIBUTE__(try_acquire_capability(__VA_ARGS__))

// Function annotation: the function will try to acquire the mutex(es) in shared mode.
#define TRY_ACQUIRE_SHARED(...) THREAD_ANNOTATION_ATTRIBUTE__(try_acquire_shared_capability(__VA_ARGS__))

// Function pre-condition: the caller must NOT hold the specified mutex(es) when calling (locks excluded).
// This helps avoid deadlocks by ensuring a function is not called while certain locks are held.
#define EXCLUDES(...)           THREAD_ANNOTATION_ATTRIBUTE__(locks_excluded(__VA_ARGS__))

// Assertion macro: can be placed in code to assert that the calling thread currently holds the given lock.
#define ASSERT_CAPABILITY(x)    THREAD_ANNOTATION_ATTRIBUTE__(assert_capability(x))

// Assertion macro: asserts that the calling thread holds the given lock in shared (reader) mode.
#define ASSERT_SHARED_CAPABILITY(x)	THREAD_ANNOTATION_ATTRIBUTE__(assert_shared_capability(x))

// Function annotation: indicates the function returns a reference or pointer to a mutex
// that it holds (returns a capability).
// Commonly used on getter methods that return a lock, allowing analysis to know returned value is a capability.
#define RETURN_CAPABILITY(x)    THREAD_ANNOTATION_ATTRIBUTE__(lock_returned(x))

// Disables thread safety analysis for the annotated function or section of code.
// Use this macro to suppress false positives or cases that the analyzer cannot deduce.
#define NO_THREAD_SAFETY_ANALYSIS	THREAD_ANNOTATION_ATTRIBUTE__(no_thread_safety_analysis)

#ifdef __cplusplus
} // extern "C"
#endif

#endif // UTILS_BASE_THREAD_SAFETY_MACROS_H