/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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
 * @file mapped_file.h
 *
 * @brief Provides classes for memory-mapped files implemented in c_utils.
 */

#ifndef UTILS_BASE_MAPPED_FILE_H
#define UTILS_BASE_MAPPED_FILE_H

#include <cstdint>
#include <sys/stat.h>
#include <fcntl.h>
#include <string>
#include <unistd.h>
#include "errors.h"

namespace OHOS {
namespace Utils {

enum class MapMode : uint8_t {
    DEFAULT = 0,
    PRIVATE = 2,
    SHARED = DEFAULT,
    READ_ONLY = 4,
    READ_WRITE = DEFAULT,
    CREATE_IF_ABSENT = 8
};

class MappedFile {
public:
    static constexpr off_t DEFAULT_LENGTH = -1LL;

    // constructors and assignment operators
    explicit MappedFile(std::string& path,
                        MapMode mode = MapMode::DEFAULT,
                        off_t offset = 0,
                        off_t size = DEFAULT_LENGTH,
                        const char *hint = nullptr);

    MappedFile(const MappedFile& other) = delete;
    MappedFile(MappedFile&& other) noexcept;
    MappedFile& operator=(const MappedFile& other) = delete;
    MappedFile& operator=(MappedFile&& other) noexcept;
    virtual ~MappedFile();

    // mapping
    ErrCode Normalize();
    ErrCode Map();
    ErrCode Unmap();
    ErrCode TurnNext();
    ErrCode Resize();
    ErrCode Resize(off_t newSize, bool sync = false);
    ErrCode Clear(bool force = false);

    // info
    inline off_t Size() const
    {
        return size_;
    }

    inline off_t StartOffset() const
    {
        return offset_;
    }

    inline off_t EndOffset() const
    {
        return size_ == DEFAULT_LENGTH ? -1LL : offset_ + size_ - 1LL;
    }

    inline static off_t PageSize()
    {
        return pageSize_;
    }

    inline char* Begin() const
    {
        return data_;
    }

    inline char* End() const
    {
        return data_ == nullptr ? nullptr : data_ + size_ - 1;
    }

    inline char* RegionStart() const
    {
        return rStart_;
    }

    inline char* RegionEnd() const
    {
        return rEnd_;
    }

    inline bool IsMapped() const
    {
        return isMapped_;
    }

    inline bool IsNormed() const
    {
        return isNormed_;
    }

    inline const std::string& GetPath() const
    {
        return path_;
    }

    inline const char* GetHint() const
    {
        return hint_;
    }

    inline MapMode GetMode() const
    {
        return mode_;
    }

    inline int GetFd() const
    {
        return fd_;
    }

    bool ChangeOffset(off_t offset);
    bool ChangeSize(off_t size);
    bool ChangePath(const std::string& path);
    bool ChangeHint(const char* hint);
    bool ChangeMode(MapMode mode);

private:
    inline static off_t RoundSize(off_t input)
    {
        return (input % PageSize() == 0) ? input : (input / PageSize() + 1) * PageSize();
    }

    bool ValidMappedSize(off_t& targetSize, const struct stat& stat);
    bool NormalizePath();
    bool NormalizeSize();
    void NormalizeMode();
    bool OpenFile();
    bool SyncFileSize(off_t newSize);
    void Reset();

    char* data_ = nullptr;
    char* rStart_ = nullptr;
    char* rEnd_ = nullptr;
    bool isMapped_ = false;
    bool isNormed_ = false;
    bool isNewFile_ = false;

    std::string path_;
    off_t size_;
    static off_t pageSize_;
    off_t offset_;
    MapMode mode_;
    int fd_ = -1;
    int mapProt_ = 0;
    int mapFlag_ = 0;
    int openFlag_ = 0;
    const char *hint_;
};

inline MapMode operator&(MapMode a, MapMode b)
{
    return static_cast<MapMode>(static_cast<uint8_t>(a) & static_cast<uint8_t>(b));
}

inline MapMode operator|(MapMode a, MapMode b)
{
    return static_cast<MapMode>(static_cast<uint8_t>(a) | static_cast<uint8_t>(b));
}

inline MapMode operator^(MapMode a, MapMode b)
{
    return static_cast<MapMode>(static_cast<uint8_t>(a) ^ static_cast<uint8_t>(b));
}

inline MapMode operator~(MapMode a)
{
    return static_cast<MapMode>(~static_cast<uint8_t>(a));
}

inline MapMode& operator|=(MapMode& a, MapMode b)
{
    return a = a | b;
}

inline MapMode& operator&=(MapMode& a, MapMode b)
{
    return a = a & b;
}

inline MapMode& operator^=(MapMode& a, MapMode b)
{
    return a = a ^ b;
}

} // namespace Utils
} // namespace OHOS
#endif