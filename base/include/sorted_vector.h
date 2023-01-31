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

#ifndef UTILS_BASE_SORTED_VECTOR_H
#define UTILS_BASE_SORTED_VECTOR_H

#include <algorithm>
#include <functional>
#include <iostream>
#include <sys/types.h>
#include <vector>

namespace OHOS {

/**
 * @brief The order of the items in it can be maintained automatically.
 */
template <class TYPE, bool AllowDuplicate = true>
class SortedVector {

public:
    using value_type = TYPE;
    using size_type = std::size_t;
    using iterator = typename std::vector<TYPE>::iterator;
    using const_iterator = typename std::vector<TYPE>::const_iterator;

    /**
     * @brief Construct a new SortedVector object.
     */
    SortedVector();

    SortedVector(const SortedVector<TYPE, false>& rhs);

    SortedVector(const SortedVector<TYPE, true>& rhs);

    SortedVector(const std::vector<TYPE>& orivect);

    /**
     * @brief Destroy the SortedVector object.
     */
    virtual ~SortedVector() {}

    /**
     * @brief Copy assignment operator.
     *
     * @param TYPE Type of the items.
     * @param AllowDuplicate Specify if duplicated items are allowed.
     */
    SortedVector<TYPE, AllowDuplicate>& operator=(const SortedVector<TYPE, false>& rhs);
    SortedVector<TYPE, AllowDuplicate>& operator=(const SortedVector<TYPE, true>& rhs);

    /**
     * @brief Empty the vector.
     */
    inline void Clear() { vec_.clear(); }

    /**
     * @brief Return number of items in the vector.
     */
    inline size_t Size() const { return vec_.size(); }

    /**
     * @brief Return whether or not the vector is empty.
     *
     * @return Return true if vector is empty, false otherwise.
     */
    inline bool IsEmpty() const { return vec_.empty(); }

    /**
     * @brief Return how many items can be stored before reallocating new 
     * storage space.
     */
    inline size_t Capacity() const { return vec_.capacity(); }

    /**
     * @brief Set the vector's capacity to `size`.
     *
     * @return Return `size` if the vector's capacity is changed to `size`, 
     * otherwise return `CAPACITY_NOT_CHANGED`(-1).
     */
    ssize_t SetCapcity(size_t size)
    {
        if (size < vec_.capacity()) {
            return CAPCITY_NOT_CHANGED;
        }

        vec_.reserve(size);
        return size;
    }

    // Cstyle access
    /**
     * @brief Return a const pointer to the first item of a vector, which is 
     * used to access the item.
     */
    inline const TYPE* Array() const { return vec_.data(); }

    /**
     * @brief Return a non-const pointer to the first item of a vector that is 
     * used to access the item of the vector.
     *
     * @note When use it , you should make sure it is sorted.
     */
    TYPE* EditArray() { return vec_.data(); }

    /**
     * @brief Find the first index of the 'item' value in the vector.
     *
     * @param item Target value.
     * @return If found, the index value is returned, otherwise `NOT_FOUND`(-1)
     * is returned.
     */
    ssize_t IndexOf(const TYPE& item) const;

    /**
     * @brief Find where this `item` should be inserted.
     *
     * @param item Target value.
     * @return Return the index where the value should be inserted.
     */
    size_t OrderOf(const TYPE& item) const;

    /**
     * @brief Access vector items based on the input `index`.
     *
     * @param index `index` value.
     * @return Return the value of the item corresponding to the `index`.
     */
    inline const TYPE& operator[](size_t index) const { return vec_[index]; }

    /**
     * @brief Return a reference to the item at the end of the vector.
     */
    const TYPE& Back() const { return vec_.back(); }

    /**
     * @brief Return a reference to the vector starting item.
     */
    const TYPE& Front() const { return vec_.front(); }

    /**
     * @brief Remove the last item of the vector.
     */
    void PopBack() { return vec_.pop_back(); }

    /**
     * @brief Return the item value of a vector.
     *
     * @param index `index` value.
     * @return If `index` is less than 0, the value of 
     * vector[vector.size() + `index`] is returned, otherwise the value of 
     * vector[`index`] is returned.
     */
    const TYPE& MirrorItemAt(ssize_t index) const
    {
        if (index < 0) {
            return *(vec_.end() + index);
        }
        return *(vec_.begin() + index);
    };

    // modify the array
    /**
     * @brief Add a new 'item' in the correct place.
     *
     * @return Return the position of the new item on success, otherwise it 
     * returns `ADD_FAIL`(-1)
     */
    ssize_t Add(const TYPE& item);

    /**
     * @brief Return reference of the item based on `index`.
     *
     * @param index `index` value.
     */
    TYPE& EditItemAt(size_t index)
    {
        return vec_[index];
    }


    /**
     * @brief Merge a vector into this one.
     *
     * If the value of `AllowDuplicate` is false, it is vec_ to deduplicate.
     *
     * @param invec The vector to be merged.
     * @return Return the size of the merged vec_.
     */
    size_t Merge(const std::vector<TYPE>& invec);
    size_t Merge(const SortedVector<TYPE, AllowDuplicate>& sortedVector);

    /**
     * @brief If `index` is less than the vector's size, delete the item at 
     * `index`.
     *
     * @param index `index` value.
     * @return If `index` is greater than or equal to the size of the vector, 
     * the iterator of the last item is returned, otherwise the iterator of the
     * next item of the deleted item is returned.
     */
    iterator Erase(size_t index)
    {
        if (index >= vec_.size()) {
            return vec_.end();
        }
        return vec_.erase(vec_.begin() + index);
    }

    /**
     * @brief An iterator that returns a vector starting item of a non-const 
     * type.
     */
    iterator Begin()
    {
        return vec_.begin();
    }

    /**
     * @brief An iterator that returns the vector starting item of type const.
     */
    const_iterator Begin() const
    {
        return vec_.begin();
    }

    /**
     * @brief Return an iterator for an item at the end of a vector of a 
     * non-const type.
     */
    iterator End()
    {
        return vec_.end();
    }

    /**
     * @brief Return an iterator for the item at the end of a vector of type 
     * const.
     */
    const_iterator End() const
    {
        return vec_.end();
    }

    static const ssize_t NOT_FOUND = -1;
    static const ssize_t ADD_FAIL = -1;
    static const ssize_t CAPCITY_NOT_CHANGED = -1;

private:
    std::vector<TYPE> vec_;
};

template <class TYPE, bool AllowDuplicate>
inline SortedVector<TYPE, AllowDuplicate>::SortedVector()
    : vec_() {}

template <class TYPE, bool AllowDuplicate>
SortedVector<TYPE, AllowDuplicate>::SortedVector(const SortedVector<TYPE, false>& rhs)
{
    // this class: AllowDuplicate or Not AllowDuplicate same type
    std::copy(rhs.Begin(), rhs.End(), std::back_inserter(vec_));
}

template <class TYPE, bool AllowDuplicate>
SortedVector<TYPE, AllowDuplicate>::SortedVector(const SortedVector<TYPE, true>& rhs)
{

    if (AllowDuplicate) {
        std::copy(rhs.Begin(), rhs.End(), std::back_inserter(vec_));
    } else {
        // AllowDuplicate to Not AllowDuplicate
        std::unique_copy(rhs.Begin(), rhs.End(), std::back_inserter(vec_));
    }
}

// copy operator
template <class TYPE, bool AllowDuplicate>
SortedVector<TYPE, AllowDuplicate>& SortedVector<TYPE, AllowDuplicate>::operator=(const SortedVector<TYPE, false>& rhs)
{
    // this class: AllowDuplicate or Not AllowDuplicate same type
    vec_.clear();
    std::copy(rhs.Begin(), rhs.End(), std::back_inserter(vec_));
    return *this;
}

// copy operator
template <class TYPE, bool AllowDuplicate>
SortedVector<TYPE, AllowDuplicate>& SortedVector<TYPE, AllowDuplicate>::operator=(const SortedVector<TYPE, true>& rhs)
{
    vec_.clear();

    if (AllowDuplicate) {
        std::copy(rhs.Begin(), rhs.End(), std::back_inserter(vec_));
    } else {
        // AllowDuplicate to Not AllowDuplicate
        std::unique_copy(rhs.Begin(), rhs.End(), std::back_inserter(vec_));
    }

    return *this;
}

template <class TYPE, bool AllowDuplicate>
ssize_t SortedVector<TYPE, AllowDuplicate>::IndexOf(const TYPE& item) const
{
    if (vec_.empty()) {
        return NOT_FOUND;
    }

    auto it = std::lower_bound(std::begin(vec_), std::end(vec_), item);
    if (it == vec_.end() || !(*it == item)) {
        return NOT_FOUND;
    }
    return it - vec_.begin();
}

template <class TYPE, bool AllowDuplicate>
size_t SortedVector<TYPE, AllowDuplicate>::OrderOf(const TYPE& item) const
{
    auto it = std::upper_bound(vec_.begin(), vec_.end(), item);
    return it - vec_.begin();
}

template <class TYPE, bool AllowDuplicate>
ssize_t SortedVector<TYPE, AllowDuplicate>::Add(const TYPE& item)
{
    ssize_t index = IndexOf(item);
    if (index != NOT_FOUND && !AllowDuplicate) {
        return ADD_FAIL;
    }

    auto it = std::upper_bound(vec_.begin(), vec_.end(), item);
    it = vec_.insert(it, item);
    return it - vec_.begin();
}

template <class TYPE, bool AllowDuplicate>
SortedVector<TYPE, AllowDuplicate>::SortedVector(const std::vector<TYPE>& invec)
{
    if (invec.empty()) {
        return;
    }

    std::vector<TYPE> newvector(invec);
    std::sort(newvector.begin(), newvector.end());
    if (AllowDuplicate) {
        vec_.swap(newvector);
    } else {
        std::unique_copy(newvector.begin(), newvector.end(), std::back_inserter(vec_));
    }
}

template <class TYPE, bool AllowDuplicate>
size_t SortedVector<TYPE, AllowDuplicate>::Merge(const std::vector<TYPE>& invec)
{
    SortedVector<TYPE, AllowDuplicate> sortedVector(invec);
    Merge(sortedVector);
    return vec_.size();
}

template <class TYPE, bool AllowDuplicate>
size_t SortedVector<TYPE, AllowDuplicate>::Merge(const SortedVector<TYPE, AllowDuplicate>& sortedVector)
{
    std::vector<TYPE> newVec;
    std::merge(vec_.begin(), vec_.end(), sortedVector.Begin(), sortedVector.End(), std::back_inserter(newVec));
    if (!AllowDuplicate) {
        vec_.clear();
        std::unique_copy(newVec.begin(), newVec.end(), std::back_inserter(vec_));
    } else {
        vec_.swap(newVec);
    }
    return vec_.size();
}

} // namespace OHOS
#endif
