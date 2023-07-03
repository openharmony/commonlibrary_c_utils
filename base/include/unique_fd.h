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

/**
 * @file unique_fd.h
 *
 * @brief Provides APIs to manage file descriptors (FDs) implemented in c_utils.
 *
 * The manager class `UniqueFdAddDeletor`,
 * the default deleter class `DefaultDeleter`,
 * and related global overloaded operator functions are provided.
 */
#ifndef UNIQUE_FD_H
#define UNIQUE_FD_H

#include <unistd.h>

namespace OHOS {

/**
 * @brief Provides the default implementation for a deleter,
 * including a static function to close FDs.
 *
 * The deleter is used for closing FDs. You can implement a deleter to
 * deal with a different scenario. When `Deleter::Close()` is called to enable
 * a `UniqueFdAddDeletor` object to release the management of an FD,
 * the FD can no longer be taken over by other `UniqueFdAddDeletor` objects.
 */
class DefaultDeleter {
public:
    /**
     * @brief Default function to close an FD.
     *
     * Call `close()` if the input FD is valid (greater than or equal to 0).
     *
     * @param fd Indicates an FD.
     */
    static void Close(int fd)
    {
        if (fd >= 0) {
            close(fd);
        }
    }
};

template <typename Deleter>
class UniqueFdAddDeletor;
template <typename Deleter>
bool operator==(const int& lhs, const UniqueFdAddDeletor<Deleter>& rhs);
template <typename Deleter>
bool operator!=(const int& lhs, const UniqueFdAddDeletor<Deleter>& rhs);
template <typename Deleter>
bool operator>=(const int& lhs, const UniqueFdAddDeletor<Deleter>& rhs);
template <typename Deleter>
bool operator>(const int& lhs, const UniqueFdAddDeletor<Deleter>& rhs);
template <typename Deleter>
bool operator<=(const int& lhs, const UniqueFdAddDeletor<Deleter>& rhs);
template <typename Deleter>
bool operator<(const int& lhs, const UniqueFdAddDeletor<Deleter>& rhs);

/**
 * @brief Defines an FD manager.
 *
 * To ensure unique management on an FD, avoid the double-close issue,
 * which may cause a file to be incorrectly closed./n
 * The management of an FD can be delivered between `UniqueFdAddDeletor`
 * objects. An FD will be closed if no `UniqueFdAddDeletor` object is available
 * to take over its management.
 *
 * @tparam Deleter Indicates a deleter.
 * @see DefaultDeleter
 */
template <typename Deleter = DefaultDeleter>
class UniqueFdAddDeletor final {
    friend bool operator==<Deleter>(const int& lhs, const UniqueFdAddDeletor<Deleter>& rhs);

    friend bool operator!=<Deleter>(const int& lhs, const UniqueFdAddDeletor<Deleter>& rhs);

    friend bool operator>=<Deleter>(const int& lhs, const UniqueFdAddDeletor<Deleter>& rhs);

    friend bool operator><Deleter>(const int& lhs, const UniqueFdAddDeletor<Deleter>& rhs);

    friend bool operator<=<Deleter>(const int& lhs, const UniqueFdAddDeletor<Deleter>& rhs);

    friend bool operator< <Deleter>(const int& lhs, const UniqueFdAddDeletor<Deleter>& rhs);

public:
    /**
     * @brief Creates a `UniqueFdAddDeletor` object to manage an FD.
     *
     * @param value Indicates the FD to be managed.
     */
    explicit UniqueFdAddDeletor(const int& value)
        : fd_(value)
    {
    }

    /**
     * @brief Constructor used to create a `UniqueFdAddDeletor` object
     * with the FD set to `-1`.
     */
    UniqueFdAddDeletor()
        : fd_(-1)
    {
    }

    /**
     * @brief Destructor used to destroy this `UniqueFdAddDeletor` object.
     *
     * This function is used to close the FD and set the FD to `-1`.
     */
    ~UniqueFdAddDeletor() { Reset(-1); }

    /**
     * @brief Releases the management on the current FD and sets it to `-1`.
     *
     * @return Returns the original FD before release.
     * @note The released FD needs to be taken over by another
     * `UniqueFdAddDeletor` object; otherwise, it must be closed manually.
     */
    int Release()
    {
        int tmp = fd_;
        fd_ = -1;
        return tmp;
    }

    // this is dangerous, when you use it , you should know it,
    // do not operator on the ret
    /**
     * @brief An overloaded cast operator function.
     *
     * This function will be called when passing a `UniqueFdAddDeletor` object
     * to a function that requires a parameter of `int`.
     *
     * @return Returns the current FD under management.
     */
    operator int() const { return Get(); } // NOLINT

    // this is dangerous, when you use it , you should know it, do not operator
    // on the ret
    /**
     * @brief Obtains the current FD under management, without releasing it.
     *
     * @return Returns the current FD.
     */
    int Get() const
    {
        return fd_;
    }

    // we need move fd from one to another
    /**
     * @brief Move constructor used to deliver the management of an FD from a
     * `UniqueFdAddDeletor` object to this object.
     *
     * @param rhs Indicates the source `UniqueFdAddDeletor` object.
     */
    UniqueFdAddDeletor(UniqueFdAddDeletor&& rhs)
    {
        int rhsfd = rhs.Release();
        fd_ = rhsfd;
    }

    /**
     * @brief Overloaded move assignment operator function used to deliver the
     * management of an FD from a `UniqueFdAddDeletor` object to this object.
     *
     * @param rhs Indicates the source `UniqueFdAddDeletor` object.
     * @return Returns this `UniqueFdAddDeletor` object.
     * @note The current FD manged by this `UniqueFdAddDeletor` object will be
     * closed, and this object will take over
     * the FD originally managed by `rhs`.
     */
    UniqueFdAddDeletor& operator=(UniqueFdAddDeletor&& rhs)
    {
        int rhsfd = rhs.Release();
        Reset(rhsfd);
        return *this;
    }

    /**
     * @brief Checks whether the FD managed by this object and that managed by
     * the source object are equal.
     *
     * @param rhs Indicates the source `UniqueFdAddDeletor` object.
     *
     * @return Returns `true` if the two FDs are equal; returns `false`
     * otherwise.
     */
    bool operator==(const int& rhs) const
    {
        return fd_ == rhs;
    }

    /**
     * @brief Checks whether the FD managed by this object and that managed by
     * the source object are not equal.
     *
     * @param rhs Indicates the source `UniqueFdAddDeletor` object.
     *
     * @return Returns `true` if the two FDs are not equal; returns `false`
     * otherwise.
     */
    bool operator!=(const int& rhs) const
    {
        return !(fd_ == rhs);
    }

    /**
     * @brief Checks whether the FD managed by this object is greater than or
     * equal to that managed by the source object.
     *
     * @param rhs Indicates the source `UniqueFdAddDeletor` object.
     *
     * @return Returns `true` if FD managed by this object is greater than or
     * equal to that managed by the source object; returns `false` otherwise.
     */
    bool operator>=(const int& rhs) const
    {
        return fd_ >= rhs;
    }

    /**
     * @brief Checks whether the FD managed by this object is greater than that
     * managed by the source object.
     *
     * @param rhs Indicates the source `UniqueFdAddDeletor` object.
     *
     * @return Returns `true` if FD managed by this object is greater than that
     * managed by the source object; returns `false` otherwise.
     */
    bool operator>(const int& rhs) const
    {
        return fd_ > rhs;
    }

    /**
     * @brief Checks whether the FD managed by this object is less than or equal
     * to that managed by the source object.
     *
     * @param rhs Indicates the source `UniqueFdAddDeletor` object.
     *
     * @return Returns `true` if FD managed by this object is less than or equal
     * to that managed by the source object; returns `false` otherwise.
     */
    bool operator<=(const int& rhs) const
    {
        return fd_ <= rhs;
    }

    /**
     * @brief Checks whether the FD managed by this object is less than that
     * managed by the source object.
     *
     * @param rhs Indicates the source `UniqueFdAddDeletor` object.
     *
     * @return Returns `true` if FD managed by this object is less than that
     * managed by the source object; returns `false` otherwise.
     */
    bool operator<(const int& rhs) const
    {
        return fd_ < rhs;
    }

private:
    int fd_ = -1;

    void Reset(int newValue)
    {
        if (fd_ >= 0) {
            Deleter::Close(fd_);
        }
        fd_ = newValue;
    }

    // disallow copy ctor and copy assign
    UniqueFdAddDeletor(const UniqueFdAddDeletor& rhs) = delete;
    UniqueFdAddDeletor& operator=(const UniqueFdAddDeletor& rhs) = delete;
};

/**
* @brief Checks whether the FD managed by two objects (specified by `lhs` and
* `rhs` respectively) are equal.
*
 * @tparam Deleter Indicates a deleter.
 * @param lhs Indicates the first `UniqueFdAddDeletor` object.
 * @param rhs Indicates the second `UniqueFdAddDeletor` object.
 *
 * @return Returns `true` if the two FDs are equal; returns `false` otherwise.
 * @see DefaultDeleter
 */
template <typename Deleter = DefaultDeleter>
bool operator==(const int& lhs, const UniqueFdAddDeletor<Deleter>& rhs)
{
    return lhs == rhs.fd_;
}

/**
* @brief Checks whether the FD managed by two objects (specified by `lhs` and
* `rhs` respectively) are not equal.
*
 * @tparam Deleter Indicates a deleter.
 * @param lhs Indicates the first `UniqueFdAddDeletor` object.
 * @param rhs Indicates the second `UniqueFdAddDeletor` object.
 *
 * @return Returns `true` if the two FDs are not equal; returns `false`
 * otherwise.
 * @see DefaultDeleter
 */
template <typename Deleter = DefaultDeleter>
bool operator!=(const int& lhs, const UniqueFdAddDeletor<Deleter>& rhs)
{
    return !(lhs == rhs.fd_);
}

/**
 * @brief Checks whether the FD managed by `lhs` is greater than or equal to
 * that managed by `rhs`.
 *
 * @tparam Deleter Indicates a deleter.
 * @param lhs Indicates the first `UniqueFdAddDeletor` object.
 * @param rhs Indicates the second `UniqueFdAddDeletor` object.
 *
 * @return Returns `true` if the FD managed by `lhs` is greater than or equal to
 * that managed by `rhs`; returns `false` otherwise.
 * @see DefaultDeleter
 */
template <typename Deleter = DefaultDeleter>
bool operator>=(const int& lhs, const UniqueFdAddDeletor<Deleter>& rhs)
{
    return lhs >= rhs.fd_;
}

/**
 * @brief Checks whether the FD managed by `lhs` is greater than that
 * managed by `rhs`.
 *
 * @tparam Deleter Indicates a deleter.
 * @param lhs Indicates the first `UniqueFdAddDeletor` object.
 * @param rhs Indicates the second `UniqueFdAddDeletor` object.
 *
 * @return Returns `true` if the FD managed by `lhs` is greater than that
 * managed by `rhs`; returns `false` otherwise.
 * @see DefaultDeleter
 */
template <typename Deleter = DefaultDeleter>
bool operator>(const int& lhs, const UniqueFdAddDeletor<Deleter>& rhs)
{
    return lhs > rhs.fd_;
}

/**
 * @brief Checks whether the FD managed by `lhs` is less than or equal to that
 * managed by `rhs`.
 *
 * @tparam Deleter Indicates a deleter.
 * @param lhs Indicates the first `UniqueFdAddDeletor` object.
 * @param rhs Indicates the second `UniqueFdAddDeletor` object.
 *
 * @return Returns `true` if the FD managed by `lhs` is less than or equal to
 * that managed by `rhs`; returns `false` otherwise.
 * @see DefaultDeleter
 */
template <typename Deleter = DefaultDeleter>
bool operator<=(const int& lhs, const UniqueFdAddDeletor<Deleter>& rhs)
{
    return lhs <= rhs.fd_;
}

/**
 * @brief Checks whether the FD managed by `lhs` is less than that
 * managed by `rhs`.
 *
 * @tparam Deleter Indicates a deleter.
 * @param lhs Indicates the first `UniqueFdAddDeletor` object.
 * @param rhs Indicates the second `UniqueFdAddDeletor` object.
 *
 * @return Returns `true` if the FD managed by `lhs` is less than that
 * managed by `rhs`; returns `false` otherwise.
 * @see DefaultDeleter
 */
template <typename Deleter = DefaultDeleter>
bool operator<(const int& lhs, const UniqueFdAddDeletor<Deleter>& rhs)
{
    return lhs < rhs.fd_;
}

using UniqueFd = UniqueFdAddDeletor<DefaultDeleter>;
} // namespace OHOS
#endif
