/* Copyright (c) 2023 Huawei Device Co., Ltd.
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

/*!
 * File_ex provides interfaces for operating on file.
 */

#[cxx::bridge(namespace = "OHOS")]
/// Module file_ex::ffi. Includes interfaces which will call c++ counterparts via FFI.
pub mod ffi {
    #[allow(dead_code)]
    unsafe extern "C++" {
        include!("commonlibrary/c_utils/base/include/file_ex.h");
    }
}
