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
 * @brief Provide management of file descriptor implemented in c_utils.
 *
 * Include Manager class UniqueFdAddDeletor, default Deleter class
 * DefaultDeleter and related global operator overload functions.
 */
#ifndef UNIQUE_FD_H
#define UNIQUE_FD_H

#include <unistd.h>

namespace OHOS {

/**
 * @brief Default implement of Deleter, including a static function to close the fd.
 *
 * Deleter is used for closing the file descriptor. Developer could implement
 * another Deleter to deal with different scenarios./n `Deleter::Close()` will
 * call when a UniqueFdAddDeletor object release the management of an fd, and
 * there's no any other UniqueFdAddDeletor objects to take over it.
 */
class DefaultDeleter {
public:
    /**
     * @brief Default function to close fd.
     *
     * Call `close()` if the input `fd` is valid(>=0).
     *
     * @param fd Input file descriptor.
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
 * @brief A file descriptor manager.
 *
 * To take the unique management of a file descriptor, avoiding double close
 * issue. Double close issue may wrongly close the fd which is just opened./n
 * The management of an fd can be delivered between different UniqueFdAddDeletor
 * objects. An fd will be closed if there's no UniqueFdAddDeletor take over the
 * management of it.
 *
 * @tparam Deleter Specified Deletor.
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
     * @brief Construct UniqueFdAddDeletor with managed fd.
     *
     * @param value Fd to be managed.
     */
    explicit UniqueFdAddDeletor(const int& value)
        : fd_(value)
    {
    }

    /**
     * @brief Construct UniqueFdAddDeletor with managed fd of -1.
     */
    UniqueFdAddDeletor()
        : fd_(-1)
    {
    }

    /**
     * @brief Deconstructor.
     *
     * `UniqueFdAddDeletor::Reset(-1)` will call to close the fd
     * and set it to -1.
     */
    ~UniqueFdAddDeletor() { Reset(-1); }

    /**
     * @brief Release the management of current fd, set it to -1.
     *
     * @return Return the original fd before release.
     * @note Released fd need to be taken over by another UniqueFdAddDeletor 
     * object, otherwise it must be closed manually.
     */
    int Release()
    {
        int tmp = fd_;
        fd_ = -1;
        return tmp;
    }

    // this is dangerous, when you use it , you should know it, donot operator on the ret
    /**
     * @brief Cast operator overload function.
     *
     * E.g. This function will call when pass UniqueFdAddDeletor object to
     * function who requires a parameter of `int`.
     *
     * @return Return current fd under management.
     */
    operator int() const { return Get(); } // NOLINT

    // this is dangerous, when you use it , you should know it, donot operator on the ret
    /**
     * @brief Get current fd under management, but not release it.
     *
     * @return Return current fd under management.
     */
    int Get() const
    {
        return fd_;
    }

    // we need move fd from one to another
    /**
     * @brief Move constuctor. Used for delivering fd between UniqueFdAddDeletor
     * objects.
     *
     * @param rhs Source UniqueFdAddDeletor
     */
    UniqueFdAddDeletor(UniqueFdAddDeletor&& rhs)
    {
        int rhsfd = rhs.Release();
        fd_ = rhsfd;
    }

    /**
     * @brief Move assignment operator overload function. Used for delivering fd
     * between UniqueFdAddDeletor objects.
     *
     * @param rhs Source UniqueFdAddDeletor
     * @return This object.
     * @note Fd of this object will be closed, then this object will take over
     * the fd managed by `rhs`.
     */
    UniqueFdAddDeletor& operator=(UniqueFdAddDeletor&& rhs)
    {
        int rhsfd = rhs.Release();
        Reset(rhsfd);
        return *this;
    }

    /**
     * @brief Equal operator overload function.
     *
     * Compare of the `rhs` and the fd managed by this object.
     *
     * @param rhs Input value.
     * @return Return true if equal, otherwise return false.
     */
    bool operator==(const int& rhs) const
    {
        return fd_ == rhs;
    }

    /**
     * @brief Not-equal operator overload function.
     *
     * Compare of the `rhs` and the fd managed by this object.
     *
     * @param rhs Input value.
     * @return Return true if not equal, otherwise return false.
     */
    bool operator!=(const int& rhs) const
    {
        return !(fd_ == rhs);
    }

    /**
     * @brief Equal-or-Greater-than operator overload function.
     *
     * Compare of the `rhs` and the fd managed by this object.
     *
     * @param rhs Input value.
     * @return Return true if equal to or greater than `rhs`,
     * otherwise return false.
     */
    bool operator>=(const int& rhs) const
    {
        return fd_ >= rhs;
    }

    /**
     * @brief Greater-than operator overload function.
     *
     * Compare of the `rhs` and the fd managed by this object.
     *
     * @param rhs Input value.
     * @return Return true if greater than `rhs`,
     * otherwise return false.
     */
    bool operator>(const int& rhs) const
    {
        return fd_ > rhs;
    }

    /**
     * @brief Equal-or-Less-than operator overload function.
     *
     * Compare of the `rhs` and the fd managed by this object.
     *
     * @param rhs Input value.
     * @return Return true if equal to or less than `rhs`,
     * otherwise return false.
     */
    bool operator<=(const int& rhs) const
    {
        return fd_ <= rhs;
    }

    /**
     * @brief Less-than operator overload function.
     *
     * Compare of the `rhs` and the fd managed by this object.
     *
     * @param rhs Input value.
     * @return Return true if less than `rhs`,
     * otherwise return false.
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
 * @brief Global Equal operator overload function.
 *
 * Compare of the `lhs` and the fd managed by `rhs`.
 *
 * @tparam Deleter Specified Deletor.
 * @param lhs Input value.
 * @param rhs Input UniqueFdAddDeletor object.
 * @return Return true if equal, otherwise return false.
 * @see DefaultDeleter
 */
template <typename Deleter = DefaultDeleter>
bool operator==(const int& lhs, const UniqueFdAddDeletor<Deleter>& rhs)
{
    return lhs == rhs.fd_;
}

/**
 * @brief Global Not-equal operator overload function.
 *
 * Compare of the `lhs` and the fd managed by `rhs`.
 *
 * @tparam Deleter Specified Deletor.
 * @param lhs Input value.
 * @param rhs Input UniqueFdAddDeletor object.
 * @return Return true if not equal, otherwise return false.
 * @see DefaultDeleter
 */
template <typename Deleter = DefaultDeleter>
bool operator!=(const int& lhs, const UniqueFdAddDeletor<Deleter>& rhs)
{
    return !(lhs == rhs.fd_);
}

/**
 * @brief Global Equal-or-Greater-than operator overload function.
 *
 * Compare of the `lhs` and the fd managed by `rhs`.
 *
 * @tparam Deleter Specified Deletor.
 * @param lhs Input value.
 * @param rhs Input UniqueFdAddDeletor object.
 * @see DefaultDeleter
 * @return Return true if `lhs` is equal to
 * or greater than fd managed by `rhs`.
 */
template <typename Deleter = DefaultDeleter>
bool operator>=(const int& lhs, const UniqueFdAddDeletor<Deleter>& rhs)
{
    return lhs >= rhs.fd_;
}

/**
 * @brief Global Greater-than operator overload function.
 *
 * Compare of the `lhs` and the fd managed by `rhs`.
 *
 * @tparam Deleter Specified Deletor.
 * @param lhs Input value.
 * @param rhs Input UniqueFdAddDeletor object.
 * @see DefaultDeleter
 * @return Return true if `lhs` is greater than fd managed by `rhs`.
 */
template <typename Deleter = DefaultDeleter>
bool operator>(const int& lhs, const UniqueFdAddDeletor<Deleter>& rhs)
{
    return lhs > rhs.fd_;
}

/**
 * @brief Global Equal-or-Less-than operator overload function.
 *
 * Compare of the `lhs` and the fd managed by `rhs`.
 *
 * @tparam Deleter Specified Deletor.
 * @param lhs Input value.
 * @param rhs Input UniqueFdAddDeletor object.
 * @see DefaultDeleter
 * @return Return true if `lhs` is equal to
 * or less than fd managed by `rhs`.
 */
template <typename Deleter = DefaultDeleter>
bool operator<=(const int& lhs, const UniqueFdAddDeletor<Deleter>& rhs)
{
    return lhs <= rhs.fd_;
}

/**
 * @brief Global Equal-or-Greater-than operator overload function.
 *
 * Compare of the `lhs` and the fd managed by `rhs`.
 *
 * @tparam Deleter Specified Deletor.
 * @param lhs Input value.
 * @param rhs Input UniqueFdAddDeletor object.
 * @see DefaultDeleter
 * @return Return true if `lhs` is less than fd managed by `rhs`.
 */
template <typename Deleter = DefaultDeleter>
bool operator<(const int& lhs, const UniqueFdAddDeletor<Deleter>& rhs)
{
    return lhs < rhs.fd_;
}

using UniqueFd = UniqueFdAddDeletor<DefaultDeleter>;
} // namespace OHOS
#endif
