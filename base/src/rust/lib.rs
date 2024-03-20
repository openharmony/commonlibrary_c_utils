// Copyright (c) 2023 Huawei Device Co., Ltd.
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

//! Crate utils_rust. Provides rust version implementation of c_utils. Current
//! it essentially call C++ interfaces via FFI.

/// Module Ashmem. Provides interfaces to operate on ashmem(Anonymous shared
/// memory).
pub mod ashmem;

/// Module File_ex. Provides interfaces to operate on file.
/// Includes reading from or writting to files and searching for specified
/// strings.
pub mod file_ex;

/// Module Directory_ex. Provides interfaces to operate on directory.
pub mod directory_ex;
