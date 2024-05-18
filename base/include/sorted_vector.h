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
 * @brief Provides a sorted vector, in which all items are sorted automatically.
 */
template <class TYPE, bool AllowDuplicate = true>
class SortedVector {
public:
    using value_type = TYPE;
    using size_type = std::size_t;
    using iterator = typename std::vector<TYPE>::iterator;
    using const_iterator = typename std::vector<TYPE>::const_iterator;

    /**
     * @brief Creates a `SortedVector` object.
     */
    SortedVector();

    SortedVector(const SortedVector<TYPE, false>& rhs);

    SortedVector(const SortedVector<TYPE, true>& rhs);

    SortedVector(const std::vector<TYPE>& orivect);

    /**
     * @brief Destroys this `SortedVector` object.
     */
    virtual ~SortedVector() {}

    /**
     * @brief An overloaded assignment operator function that assigns a sorted
     * vector to this vector.
     *
     * @param TYPE Indicates the type of items.
     * @param AllowDuplicate Specifies whether duplicated items are allowed.
     */
    SortedVector<TYPE, AllowDuplicate>& operator=(const SortedVector<TYPE, false>& rhs);
    SortedVector<TYPE, AllowDuplicate>& operator=(const SortedVector<TYPE, true>& rhs);

    /**
     * @brief Clears this vector.
     */
    inline void Clear() { vec_.clear(); }

    /**
     * @brief Obtains the number of items in this vector.
     */
    inline size_t Size() const { return vec_.size(); }

    /**
     * @brief Checks whether this vector is empty.
     *
     * @return Returns `true` if the vector is empty; returns `false` otherwise.
     */
    inline bool IsEmpty() const { return vec_.empty(); }

    /**
     * @brief Obtains the capacity of this vector, that is, the number of items
     * that can be stored before the system reallocates new storage space.
     */
    inline size_t Capacity() const { return vec_.capacity(); }

    /**
     * @brief Sets the capacity of this vector.
     *
     * @param size Indicates the capacity to set.
     *
     * @return Returns the capacity if the operation is successful;
     * returns `CAPACITY_NOT_CHANGED`(-1) otherwise.
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
     * @brief Accesses the first item in this vector.
     *
     * @return Returns a constant pointer to the first item.
     */
    inline const TYPE* Array() const { return vec_.data(); }

    /**
     * @brief Accesses the first item in this vector while editing it.
     *
     * @note When using this function, you should ensure that the vector
     * is sorted.
     *
     * @return Returns a non-constant pointer to the first item.
     */
    TYPE* EditArray() { return vec_.data(); }

    /**
     * @brief Obtains the index of the first occurrence of an item
     * in this vector.
     *
     * @param item Indicates the item.
     *
     * @return Returns the index if the item is found;
     * returns `NOT_FOUND`(-1) otherwise.
     *
     */
    ssize_t IndexOf(const TYPE& item) const;

    /**
     * @brief Obtains the index where an item should be inserted into
     * this vector.
     *
     * @param item Indicates the item.
     *
     * @return Returns the index.
     */
    size_t OrderOf(const TYPE& item) const;

    /**
     * @brief Accesses an item with the specified index.
     *
     * @param index Indicates the index.
     *
     * @return Returns the item value.
     */
    inline const TYPE& operator[](size_t index) const { return vec_[index]; }

    /**
     * @brief Obtains the reference to the last item in this vector.
     *
     * @return Returns the reference to the last item.
     */
    const TYPE& Back() const { return vec_.back(); }

    /**
     * @brief Obtains the reference to the first item in this vector.
     *
     * @return Returns the reference to the first item.
     */
    const TYPE& Front() const { return vec_.front(); }

    /**
     * @brief Removes the last item from this vector.
     */
    void PopBack() { return vec_.pop_back(); }

    /**
     * @brief Obtains the value of an item with the specified index
     * in this vector.
     *
     * @param index Indicates the index.
     *
     * @return Returns vector[vector.size() + `index`] if `index` is
     * less than 0; returns vector[`index`] otherwise.
     */
    const TYPE& MirrorItemAt(ssize_t index) const
    {
        if (index < 0) {
            return *(vec_.end() + index);
        }
        return *(vec_.begin() + index);
    };

    // Modify the array.
    /**
     * @brief Adds an item to this vector.
     *
     * @return Returns the index of the item if the operation is successful;
     * returns `ADD_FAIL`(-1) otherwise.
     */
    ssize_t Add(const TYPE& item);

    /**
     * @brief Obtains the reference to an item with the specified index
     * in this vector.
     *
     * @param index Indicates the index.
     *
     * @return Returns the reference to the item.
     */
    TYPE& EditItemAt(size_t index)
    {
        return vec_[index];
    }

    /**
     * @brief Merges a vector into this vector.
     *
     * If `AllowDuplicate` is set to `false`, the current vector should
     * perform deduplication.
     *
     * @param invec Indicates the vector to be merged.
     *
     * @return Returns the size of the merged vector.
     */
    size_t Merge(const std::vector<TYPE>& invec);
    size_t Merge(const SortedVector<TYPE, AllowDuplicate>& sortedVector);

    /**
     * @brief Erases the item with the specified index from this vector.
     *
     * @param index Indicates the index.
     *
     * @return Returns an iterator to the last item if the index is
     * greater than or equal to the size of the vector;
     * returns the item next to the deleted item otherwise.
     */
    iterator Erase(size_t index)
    {
        if (index >= vec_.size()) {
            return vec_.end();
        }
        return vec_.erase(vec_.begin() + index);
    }

    /**
     * @brief Obtains an iterator of the non-const type to the first item
     * in this vector.
     *
     * @return Returns an iterator to the first item.
     */
    iterator Begin()
    {
        return vec_.begin();
    }

    /**
     * @brief Obtains an iterator of the const type to the first item
     * in this vector.
     *
     * @return Returns an iterator to the first item.
     */
    const_iterator Begin() const
    {
        return vec_.begin();
    }

    /**
     * @brief Obtains an iterator of the non-const type to the last item
     * in this vector.
     *
     * @return Returns an iterator to the last item.
     */
    iterator End()
    {
        return vec_.end();
    }

    /**
     * @brief Obtains an iterator of the const type to the last item
     * in this vector.
     *
     * @return Returns an iterator to the last item.
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
