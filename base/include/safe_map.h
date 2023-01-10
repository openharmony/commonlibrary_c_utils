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

#ifndef UTILS_BASE_SAFE_MAP_H
#define UTILS_BASE_SAFE_MAP_H

#include <map>
#include <mutex>

namespace OHOS {

/**
 * @brief A thread-safe map implementation is provided.
 */
template <typename K, typename V>
class SafeMap {
public:
    SafeMap() {}

    ~SafeMap() {}

    SafeMap(const SafeMap& rhs)
    {
        map_ = rhs.map_;
    }

    SafeMap& operator=(const SafeMap& rhs)
    {
        if (&rhs != this) {
            map_ = rhs.map_;
        }

        return *this;
    }

    V& operator[](const K& key)
    {
        return map_[key];
    }

    /**
     * @brief Get the size of the map.
     *
     * when multithread calling Size() return a tmp status, some threads may
     * insert just after Size() call.
     */
    int Size()
    {
        std::lock_guard<std::mutex> lock(mutex_);
        return map_.size();
    }

    /**
     * @brief Determine whether the map is empty or not.
     *
     * when multithread calling Empty() return a tmp status, some threads may 
     * insert just after Empty() call.
     *
     * @return Return true if it is empty, otherwise returns false.
     */
    bool IsEmpty()
    {
        std::lock_guard<std::mutex> lock(mutex_);
        return map_.empty();
    }

    /**
     * @brief Insert a new element into the map.
     *
     * @param key The key to be inserted.
     * @param value The value to be inserted.
     * @return Return true if the insertion is successful, otherwise returns 
     * false.
     */
    bool Insert(const K& key, const V& value)
    {
        std::lock_guard<std::mutex> lock(mutex_);
        auto ret = map_.insert(std::pair<K, V>(key, value));
        return ret.second;
    }

    /**
     * @brief Insert elements into the map.
     *
     * @param key The key to be inserted.
     * @param value The value to be inserted.
     * @note Delete and then insert when the key exists, ensuring that the 
     * final value is inserted.
     */
    void EnsureInsert(const K& key, const V& value)
    {
        std::lock_guard<std::mutex> lock(mutex_);
        auto ret = map_.insert(std::pair<K, V>(key, value));
        // find key and cannot insert
        if (!ret.second) {
            map_.erase(ret.first);
            map_.insert(std::pair<K, V>(key, value));
            return;
        }
        return;
    }

    /**
     * @brief Search for elements in the map.
     *
     * @param Key The key to be found.
     * @param value The value corresponding to the found key.
     * @return Return true when the key exists, otherwise returns false.
     */
    bool Find(const K& key, V& value)
    {
        bool ret = false;
        std::lock_guard<std::mutex> lock(mutex_);

        auto iter = map_.find(key);
        if (iter != map_.end()) {
            value = iter->second;
            ret = true;
        }

        return ret;
    }

    /**
     * @brief Search for elements in the map and replace the `oldValue` 
     * corresponding to the key with `newValue`.
     *
     * @param Key The key to be found.
     * @param oldValue The value corresponding to the found key.
     * @param newValue The new value to insert.
     * @return Return true when the key exists, otherwise returns false.
     */
    bool FindOldAndSetNew(const K& key, V& oldValue, const V& newValue)
    {
        bool ret = false;
        std::lock_guard<std::mutex> lock(mutex_);
        if (map_.size() > 0) {
            auto iter = map_.find(key);
            if (iter != map_.end()) {
                oldValue = iter->second;
                map_.erase(iter);
                map_.insert(std::pair<K, V>(key, newValue));
                ret = true;
            }
        }

        return ret;
    }

    /**
     * @brief Delete key-value pairs whose key is key in the map.
     *
     * @param Key The key to be deleted.
     */
    void Erase(const K& key)
    {
        std::lock_guard<std::mutex> lock(mutex_);
        map_.erase(key);
    }

    /**
     * @brief Delete all key-value pairs stored in the map.
     */
    void Clear()
    {
        std::lock_guard<std::mutex> lock(mutex_);
        map_.clear();
        return;
    }

    using SafeMapCallBack = std::function<void(const K, V&)>;

    /**
     * @brief Iterate through the elements in the map.
     *
     * @param callback A specific function that performs custom operations on 
     * each KV key-value pair.
     */
    void Iterate(const SafeMapCallBack& callback)
    {
        std::lock_guard<std::mutex> lock(mutex_);
        if (!map_.empty()) {
            for (auto it = map_.begin(); it != map_.end(); it++) {
                callback(it -> first, it -> second);
            }
        }
    }

private:
    std::mutex mutex_;
    std::map<K, V> map_;
};

} // namespace OHOS
#endif
