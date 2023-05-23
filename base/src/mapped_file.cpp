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
#include "mapped_file.h"

#include <fcntl.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <unistd.h>
#include "common_mapped_file_errors.h"
#include "errors.h"
#include "file_ex.h"
#include "utils_log.h"

namespace OHOS {
namespace Utils {
off_t MappedFile::pageSize_ = static_cast<off_t>(sysconf(_SC_PAGESIZE));

MappedFile::MappedFile(std::string& path, MapMode mode, off_t offset, off_t size, const char *hint)
    :path_(path), size_(size), offset_(offset), mode_(mode), hint_(hint) {}

bool MappedFile::ValidMappedSize(off_t& targetSize, const struct stat& stb)
{
    off_t max = RoundSize(stb.st_size) - offset_; // Avoid mapped size excessing
                                                  // that of the file more than a page,
    if (max > 0) {                                // since write operation on it may raise signal 7.
        targetSize = targetSize > max ? max : targetSize;
    } else {
        return false;
    }

    return true;
}
bool MappedFile::NormalizePath()
{
    char canonicalPath[PATH_MAX];
    if (realpath(path_.c_str(), canonicalPath) == nullptr) {
        if (errno != ENOENT) {
            UTILS_LOGE("%{public}s get realpath failed.", __FUNCTION__);
            return false;
        }
    } else {
        path_ = canonicalPath;
    }

    return true;
}

bool MappedFile::NormalizeSize()
{
    if (size_ == 0 || size_ < DEFAULT_LENGTH) {
        UTILS_LOGE("%{public}s: Failed. Invalid mapping size: %{public}lld",
                   __FUNCTION__, static_cast<long long>(size_));
        return false;
    }

    errno = 0;
    if (!NormalizePath()) {
        UTILS_LOGE("%{public}s normalize path failed. %{public}s", __FUNCTION__, strerror(errno));
        return false;
    }
    if (!FileExists(path_)) {
        if ((mode_ & MapMode::CREATE_IF_ABSENT) == MapMode::DEFAULT) {
            UTILS_LOGE("%{public}s: Failed. %{public}s", __FUNCTION__, strerror(errno));
            return false;
        }

        if (size_ == DEFAULT_LENGTH) {
            size_ = PageSize();
        }

        isNewFile_ = true;
    } else {
        struct stat stb = {0};
        // Calculate specified mapping size
        if (stat(path_.c_str(), &stb) != 0) {
            UTILS_LOGW("%{public}s: Failed. Get file size failed! Mapped size will be that of a page.", __FUNCTION__);
            size_ = PageSize();
        }

        if (size_ == DEFAULT_LENGTH) {
            size_ = stb.st_size;
        }

        // Get valid size
        if (!ValidMappedSize(size_, stb)) {
            UTILS_LOGE("%{public}s: Failed. Invalid params. Specified size: %{public}lld, File size: %{public}lld", \
                       __FUNCTION__, static_cast<long long>(size_), static_cast<long long>(stb.st_size));
            return false;
        }
    }

    return true;
}

void MappedFile::NormalizeMode()
{
    mode_ &= (MapMode::PRIVATE | MapMode::READ_ONLY | MapMode::CREATE_IF_ABSENT);

    openFlag_ = O_CLOEXEC;
    if (mode_ == MapMode::DEFAULT) {
        mapFlag_ = MAP_SHARED;
        mapProt_ = PROT_READ | PROT_WRITE;
        openFlag_ |= O_RDWR;
    } else {
        if ((mode_ & MapMode::PRIVATE) != MapMode::DEFAULT) {
            mapFlag_ = MAP_PRIVATE;
        } else {
            mapFlag_ = MAP_SHARED;
        }

        if ((mode_ & MapMode::READ_ONLY) != MapMode::DEFAULT) {
            mapProt_ = PROT_READ;
            openFlag_ |= O_RDONLY;
        } else {
            mapProt_ = PROT_READ | PROT_WRITE;
            openFlag_ |= O_RDWR;
        }

        if ((mode_ & MapMode::CREATE_IF_ABSENT) != MapMode::DEFAULT) {
            openFlag_ |= O_CREAT;
        }
    }
}

ErrCode MappedFile::Normalize()
{
    if (isNormed_) {
        UTILS_LOGD("%{public}s: Already normalized.", __FUNCTION__);
        return ERR_INVALID_OPERATION;
    }

    // resolve params for mapping region
    // offset
    if (offset_ < 0 || (offset_ % PageSize() != 0)) {
        UTILS_LOGE("%{public}s: Failed. Invalid offset: %{public}lld", __FUNCTION__, static_cast<long long>(offset_));
        return ERR_INVALID_VALUE;
    }

    // size
    if (!NormalizeSize()) {
        UTILS_LOGE("%{public}s: Failed. Cannot normalize size.", __FUNCTION__);
        return ERR_INVALID_VALUE;
    }

    // Set open flags, mapping types and protections
    NormalizeMode();

    isNormed_ = true;
    return MAPPED_FILE_ERR_OK;
}

bool MappedFile::OpenFile()
{
    int fd = open(path_.c_str(), openFlag_, S_IRWXU | S_IRGRP | S_IROTH);
    if (fd == -1) {
        UTILS_LOGE("%{public}s: Failed. Cannot open file - %{public}s.", __FUNCTION__, strerror(errno));
        return false;
    }

    if (isNewFile_) {
        if (!NormalizePath()) {
            UTILS_LOGE("%{public}s normalize path failed. %{public}s", __FUNCTION__, strerror(errno));
            return false;
        }
        if (ftruncate(fd, EndOffset() + 1) == -1) {
            UTILS_LOGD("%{public}s: Failed. Cannot change file size: %{public}s.", __FUNCTION__, strerror(errno));
            if (close(fd) == -1) {
                UTILS_LOGW("%{public}s: Failed. Cannot close the file: %{public}s.", \
                           __FUNCTION__, strerror(errno));
            }
            if (unlink(path_.c_str()) == -1) {
                UTILS_LOGW("%{public}s: Failed. Cannot unlink the file: %{public}s.", \
                           __FUNCTION__, strerror(errno));
            }
            return false;
        }
        isNewFile_ = false;
    }

    fd_ = fd;
    return true;
}

ErrCode MappedFile::Map()
{
    if (isMapped_) {
        UTILS_LOGD("%{public}s: Failed. Already mapped.", __FUNCTION__);
        return ERR_INVALID_OPERATION;
    }

    // Normalize params
    ErrCode res = Normalize();
    if (res != MAPPED_FILE_ERR_OK && res != ERR_INVALID_OPERATION) {
        UTILS_LOGD("%{public}s: Normalize Failed.", __FUNCTION__);
        return res;
    }

    // Open file to get its fd
    if (fd_ == -1) {
        if (!OpenFile()) {
            UTILS_LOGD("%{public}s: Open Failed.", __FUNCTION__);
            return MAPPED_FILE_ERR_FAILED;
        }
    }

    // Try map
    void* data = MAP_FAILED;
    do {
        data = mmap(reinterpret_cast<void*>(const_cast<char *>(hint_)),
                    static_cast<size_t>(size_),
                    mapProt_,
                    mapFlag_,
                    fd_,
                    offset_);
        if (data == MAP_FAILED && hint_ != nullptr) {
            UTILS_LOGW("%{public}s: Mapping Failed. %{public}s, retry with a null hint.", \
                       __FUNCTION__, strerror(errno));
            hint_ = nullptr;
        } else {
            break;
        }
    } while (true);

    if (data == MAP_FAILED) {
        UTILS_LOGE("%{public}s: Mapping Failed. %{public}s", __FUNCTION__, strerror(errno));
        return MAPPED_FILE_ERR_FAILED;
    }

    rStart_ = reinterpret_cast<char*>(data);
    // set region boundary.
    rEnd_ = rStart_ + (RoundSize(size_) - 1LL);
    // set segment start
    data_ = rStart_;
    isMapped_ = true;

    return MAPPED_FILE_ERR_OK;
}

ErrCode MappedFile::Unmap()
{
    if (!isMapped_) {
        UTILS_LOGD("%{public}s: Failed. Already unmapped.", __FUNCTION__);
        return ERR_INVALID_OPERATION;
    }

    if (!isNormed_) {
        UTILS_LOGW("%{public}s. Try unmapping with params changed.", __FUNCTION__);
    }

    if (munmap(rStart_, static_cast<size_t>(size_)) == -1) {
        UTILS_LOGD("%{public}s: Failed. %{public}s.", __FUNCTION__, strerror(errno));
        return MAPPED_FILE_ERR_FAILED;
    }

    rStart_ = nullptr;
    rEnd_ = nullptr;
    data_ = nullptr;
    isMapped_ = false;
    return MAPPED_FILE_ERR_OK;
}

bool MappedFile::SyncFileSize(off_t newSize)
{
    if (newSize > size_) {
        struct stat stb = {0};
        if (stat(path_.c_str(), &stb) != 0) {
            UTILS_LOGD("%{public}s: Failed. Cannot get file size: %{public}s.", __FUNCTION__, strerror(errno));
            return false;
        } else if (offset_ + newSize <= stb.st_size) {
            UTILS_LOGW("%{public}s: Failed. Unextend file size, no need to sync.", __FUNCTION__);
        } else {
            if (ftruncate(fd_, offset_ + newSize) == -1) {
                UTILS_LOGD("%{public}s: Failed. Cannot extend file size: %{public}s.", __FUNCTION__, strerror(errno));
                return false;
            }
        }
    }

    return true;
}

ErrCode MappedFile::Resize(off_t newSize, bool sync)
{
    if (newSize == DEFAULT_LENGTH) {
        struct stat stb = {0};
        if (stat(path_.c_str(), &stb) != 0) {
            UTILS_LOGW("%{public}s: Failed. Get file size failed! Mapped size will be that of a page.", __FUNCTION__);
            newSize = PageSize();
        }

        if (newSize == DEFAULT_LENGTH) {
            newSize = stb.st_size;
        }
    }

    if (newSize == 0 || newSize < DEFAULT_LENGTH || newSize == size_) {
        UTILS_LOGD("%{public}s: Failed. Cannot remap with the same /negative size.", __FUNCTION__);
        return ERR_INVALID_OPERATION;
    }

    if (!isMapped_) {
        UTILS_LOGD("%{public}s: Failed. Invalid status. mapped:%{public}d, normed:%{public}d", \
                   __FUNCTION__, isMapped_, isNormed_);
        return ERR_INVALID_OPERATION;
    }

    if (sync) {
        if (!SyncFileSize(newSize)) {
            UTILS_LOGD("%{public}s: Sync Failed.", __FUNCTION__);
            return MAPPED_FILE_ERR_FAILED;
        }
    } else {
        struct stat stb = {0};
        if (stat(path_.c_str(), &stb) != 0) {
            UTILS_LOGW("%{public}s: Failed. Cannot get file size: %{public}s.", __FUNCTION__, strerror(errno));
            return ERR_INVALID_OPERATION;
        }

        if (!ValidMappedSize(newSize, stb)) {
            UTILS_LOGD("%{public}s: Failed. Invalid params.", __FUNCTION__);
            return ERR_INVALID_VALUE;
        }
    }

    void* newData = mremap(rStart_, static_cast<size_t>(size_), static_cast<size_t>(newSize), MREMAP_MAYMOVE);
    if (newData == MAP_FAILED) {
        UTILS_LOGD("%{public}s: Failed. %{public}s", __FUNCTION__, strerror(errno));
        return MAPPED_FILE_ERR_FAILED;
    }

    rStart_ = reinterpret_cast<char*>(newData);
    size_ = newSize;
    // set region boundary.
    rEnd_ = rStart_ + RoundSize(size_) - 1;
    // set segment start.
    data_ = rStart_;

    return MAPPED_FILE_ERR_OK;
}

ErrCode MappedFile::Resize()
{
    if (isMapped_) {
        UTILS_LOGD("%{public}s: Failed. No param changes detected or unmapping required before resize.", __FUNCTION__);
        return ERR_INVALID_OPERATION;
    }

    int res = Map();
    if (res != MAPPED_FILE_ERR_OK) {
        UTILS_LOGD("%{public}s: Failed. Remapping failed.", __FUNCTION__);
        return res;
    }

    return MAPPED_FILE_ERR_OK;
}

ErrCode MappedFile::TurnNext()
{
    if (!isNormed_) {
        UTILS_LOGD("%{public}s: Failed. Cannot turnNext with params changed.", __FUNCTION__);
        return ERR_INVALID_OPERATION;
    }

    struct stat stb = {0};
    int ret = stat(path_.c_str(), &stb);
    if (ret != 0) {
        UTILS_LOGD("%{public}s: Failed. Get file size failed.", __FUNCTION__);
        return MAPPED_FILE_ERR_FAILED;
    }
    if (EndOffset() + 1 >= stb.st_size) {
        UTILS_LOGD("%{public}s: Failed. No contents remained.", __FUNCTION__);
        return ERR_INVALID_OPERATION;
    }

    // save original params
    off_t oldSize = size_;
    off_t oldOff = offset_;
    const char* oldHint = hint_;

    // if mapped, rStart_ and rEnd_ are viable
    if (isMapped_) {
        char* curEnd = End();
        // case 1: remap needed
        if (curEnd == rEnd_) {
            // check if larger than exact file size.
            if (EndOffset() + 1 + size_ > stb.st_size) {
                size_ = stb.st_size - EndOffset() - 1;
            }
            isNormed_ = false;
            hint_ = rStart_;
            offset_ += oldSize;

            ErrCode res = Unmap();
            if (res == MAPPED_FILE_ERR_OK) {
                res = Resize();
            }
            if (res != MAPPED_FILE_ERR_OK) {
                UTILS_LOGE("%{public}s Failed. Fail to UnMap/Resize.", __FUNCTION__);
                // restore
                offset_ = oldOff;
                size_ = oldSize;
                hint_ = oldHint;
                isNormed_ = true;
            }
            return res;
        }

        // case 2: no need to remap, but to adjust boundary.
        if (curEnd + oldSize > rEnd_) { // otherwise keep original "size_"
            size_ = rEnd_ - curEnd;
        }
        data_ += oldSize;
        offset_ += oldSize;
        return MAPPED_FILE_ERR_OK;
    }

    // if not mapped, turnNext() will set offset to next page of PageSize()
    offset_ += PageSize();
    isNormed_ = false;

    return MAPPED_FILE_ERR_OK;
}

void MappedFile::Reset()
{
    isNormed_ = false;
    isMapped_ = false;
    isNewFile_ = false;

    rStart_ = nullptr;
    rEnd_ = nullptr;
    data_ = nullptr;
    path_ = "";
    size_ = DEFAULT_LENGTH;
    offset_ = 0;
    mode_ = MapMode::DEFAULT;
    fd_ = -1;
    mapProt_ = 0;
    mapFlag_ = 0;
    openFlag_ = 0;
    hint_ = nullptr;
}

ErrCode MappedFile::Clear(bool force)
{
    if (isMapped_) {
        ErrCode res = Unmap();
        if (!force && res != MAPPED_FILE_ERR_OK) {
            UTILS_LOGD("%{public}s failed. UnMapping Failed.", __FUNCTION__);
            return res;
        }
    }

    if (fd_ != -1 && close(fd_) == -1) {
        UTILS_LOGD("%{public}s: Failed. Cannot close the file: %{public}s.", \
                   __FUNCTION__, strerror(errno));
        return MAPPED_FILE_ERR_FAILED;
    }
    Reset();
    return MAPPED_FILE_ERR_OK;
}

MappedFile::~MappedFile()
{
    if (isMapped_) {
        ErrCode res = Unmap();
        if (res != MAPPED_FILE_ERR_OK) {
            UTILS_LOGW("%{public}s: File unmapping failed, it will be automatically  \
                       unmapped when the process is terminated.", __FUNCTION__);
        }
    }

    if (fd_ != -1 && close(fd_) == -1) {
        UTILS_LOGE("%{public}s: Failed. Cannot close the file: %{public}s.", \
                   __FUNCTION__, strerror(errno));
    }
}

MappedFile::MappedFile(MappedFile&& other) noexcept
    : data_(other.data_), rStart_(other.rStart_), rEnd_(other.rEnd_), isMapped_(other.isMapped_),
    isNormed_(other.isNormed_), isNewFile_(other.isNewFile_), path_(std::move(other.path_)), size_(other.size_),
    offset_(other.offset_), mode_(other.mode_), fd_(other.fd_), mapProt_(other.mapProt_), mapFlag_(other.mapFlag_),
    openFlag_(other.openFlag_), hint_(other.hint_)
{
    other.Reset();
}

MappedFile& MappedFile::operator=(MappedFile&& other) noexcept
{
    Clear(true);

    data_ = other.data_;
    rStart_ = other.rStart_;
    rEnd_ = other.rEnd_;
    isMapped_ = other.isMapped_;
    isNormed_ = other.isNormed_;
    isNewFile_ = other.isNewFile_;
    path_ = other.path_;
    size_ = other.size_;
    offset_ = other.offset_;
    mode_ = other.mode_;
    fd_ = other.fd_;
    mapProt_ = other.mapProt_;
    mapFlag_ = other.mapFlag_;
    openFlag_ = other.openFlag_;
    hint_ = other.hint_;

    other.Reset();

    return *this;
}

bool MappedFile::ChangeOffset(off_t val)
{
    if (offset_ != val) {
        if (!isMapped_ || Unmap() == MAPPED_FILE_ERR_OK) {
            offset_ = val;
            isNormed_ = false;

            return true;
        }

        UTILS_LOGW("%{public}s: Change params failed. Unmapping failed.", __FUNCTION__);
    }
    return false;
}

bool MappedFile::ChangeSize(off_t val)
{
    if ((val > 0 || val == DEFAULT_LENGTH) && size_ != val) {
        if (!isMapped_ || Unmap() == MAPPED_FILE_ERR_OK) {
            size_ = val;
            isNormed_ = false;

            return true;
        }

        UTILS_LOGW("%{public}s: Change params failed. Unmapping failed.", __FUNCTION__);
    }
    return false;
}

bool MappedFile::ChangePath(const std::string& val)
{
    if (path_ != val) {
        if (!isMapped_ || Unmap() == MAPPED_FILE_ERR_OK) {
            if (fd_ != -1 && close(fd_) == -1) {
                UTILS_LOGW("%{public}s: Failed. Cannot close the file: %{public}s.", \
                           __FUNCTION__, strerror(errno));
                return false;
            }
            path_ = val;
            isNormed_ = false;
            fd_ = -1;

            return true;
        } else {
            UTILS_LOGW("%{public}s: Change params failed. Unmapping failed.", __FUNCTION__);
        }
    }
    return false;
}

bool MappedFile::ChangeHint(const char* val)
{
    if (hint_ != val) {
        if (!isMapped_ || Unmap() == MAPPED_FILE_ERR_OK) {
            hint_ = val;
            isNormed_ = false;

            return true;
        } else {
            UTILS_LOGW("%{public}s: Change params failed. Unmapping failed.", __FUNCTION__);
        }
    }
    return false;
}

bool MappedFile::ChangeMode(MapMode val)
{
    if (mode_ != val) {
        if (!isMapped_ || Unmap() == MAPPED_FILE_ERR_OK) {
            mode_ = val;
            isNormed_ = false;

            return true;
        } else {
            UTILS_LOGW("%{public}s: Change params failed. Unmapping failed.", __FUNCTION__);
        }
    }
    return false;
}

} // namespace Utils
} // namespace OHOS