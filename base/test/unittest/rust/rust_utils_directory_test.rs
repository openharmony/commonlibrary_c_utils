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

use utils_rust::directory_ex;
use cxx::let_cxx_string;
use std::fs;
use std::fs::File;

// This code is converted from 22 functions of the utils_directory_test.cpp.

#[test]
fn test_get_current_proc_full_file_name_001()
{
    let str_base_name = "/data/test/rust_utils_directory_test";
    let str_filename = directory_ex::ffi::RustGetCurrentProcFullFileName();
    assert_eq!(str_base_name, str_filename);
}

#[test]
fn test_get_current_proc_path_001()
{
    let str_path_name = "/data/test/";
    let str_cur_path_name = directory_ex::ffi::RustGetCurrentProcPath();
    assert_eq!(str_path_name, str_cur_path_name);
}

#[test]
fn test_extract_file_path_001()
{
    let str_file_path = "/data/test/";
    let str_path = directory_ex::ffi::RustExtractFilePath(&directory_ex::ffi::RustGetCurrentProcFullFileName());
    assert_eq!(str_file_path, str_path);
}

#[test]
fn test_extract_file_name_001()
{
    let str_base_name = "rust_utils_directory_test";
    let str_name = directory_ex::ffi::RustExtractFileName(&directory_ex::ffi::RustGetCurrentProcFullFileName());
    assert_eq!(str_base_name, str_name);
}

#[test]
fn test_extract_file_ext_001()
{
    let str_base_name = "test/test.txt";
    let str_type_name = directory_ex::ffi::RustExtractFileExt(&str_base_name.to_string());
    assert_eq!(str_type_name, "txt");
}

#[test]
fn test_extract_file_ext_002()
{
    let str_base_name = "test/test_txt";
    let str_type_name = directory_ex::ffi::RustExtractFileExt(&str_base_name.to_string());
    assert_eq!(str_type_name, "");
}

#[test]
fn test_exclude_trailing_path_delimiter_001()
{
    let str_result = "data/test/UtilsDirectoryTest";
    let str_name = directory_ex::ffi::RustExcludeTrailingPathDelimiter(&str_result.to_string());
    assert_eq!(str_result, str_name);
}

#[test]
fn test_include_trailing_path_delimiter_001()
{
    let str_result = "data/test/UtilsDirectoryTest/";
    let str_name = directory_ex::ffi::RustIncludeTrailingPathDelimiter(&str_result.to_string());
    assert_eq!(str_result, str_name);
}

#[test]
fn test_get_dir_files_001()
{
    let resultfile = ["/data/test/TestFile.txt", "/data/test/UtilsDirectoryTest"];
    File::create(resultfile[0]).expect("Failed to create file");
    File::create(resultfile[1]).expect("Failed to create file");

    let path = String::from("/data/");
    let mut files: Vec<String> = Vec::new();
    directory_ex::ffi::RustGetDirFiles(&path, &mut files);

    let mut found = false;
    for element in files.iter() {
        if element == resultfile[0] {
            found = true;
            break;
        }
    }
    assert!(found);

    found = false;
    for element in files.iter() {
        if element == resultfile[1] {
            found = true;
            break;
        }
    }
    assert!(found);

    let_cxx_string!(file1 = "/data/test/TestFile.txt");
    let_cxx_string!(file2 = "/data/test/UtilsDirectoryTest");
    directory_ex::ffi::RemoveFile(&file1);
    directory_ex::ffi::RemoveFile(&file2);
}

#[test]
fn test_force_create_directory_001()
{
    let_cxx_string!(dirpath = "/data/test_dir/test2/test3");
    let mut ret = directory_ex::ffi::ForceCreateDirectory(&dirpath);
    assert!(ret);
    ret = directory_ex::ffi::IsEmptyFolder(&dirpath);
    assert!(ret);
}

#[test]
fn test_force_remove_directory_001()
{
    let_cxx_string!(dirpath = "/data/test_dir/test2/test4");
    directory_ex::ffi::ForceCreateDirectory(&dirpath);
    let ret = directory_ex::ffi::ForceRemoveDirectory(&dirpath);
    assert!(ret);
}

#[test]
fn test_force_remove_directory_002()
{
    let_cxx_string!(dirpath = "/data/test/utils_directory_tmp/");
    let ret = directory_ex::ffi::ForceRemoveDirectory(&dirpath);
    assert!(!ret);
}

#[test]
fn test_remove_file_001()
{
    let_cxx_string!(dirpath = "/data/test_dir");
    let mut ret = directory_ex::ffi::ForceCreateDirectory(&dirpath);
    assert!(ret);
    match File::create("/data/test_dir/test.txt") {
        Ok(_file) => {
            let_cxx_string!(filename = "/data/test_dir/test.txt");
            ret = directory_ex::ffi::RemoveFile(&filename);
            assert!(ret);
        },
        Err(error) => {
            println!("create file error: {}", error);
        }
    }
    ret = directory_ex::ffi::ForceRemoveDirectory(&dirpath);
    assert!(ret);
}

#[test]
fn test_get_folder_size_001()
{
    let_cxx_string!(dirpath = "/data/test_folder/");
    let mut ret = directory_ex::ffi::ForceCreateDirectory(&dirpath);
    assert!(ret);
    fs::write("/data/test_folder/test.txt", "This is a line.\nThis is another line.\n").unwrap();
    let resultsize = directory_ex::ffi::GetFolderSize(&dirpath);
    let resultcomp = 38;
    assert_eq!(resultsize, resultcomp);

    let mut mode = directory_ex::S_IRWXU | directory_ex::S_IRWXG | directory_ex::S_IRWXO;
    let_cxx_string!(txt = "/data/test_folder/test.txt");
    ret = directory_ex::ffi::ChangeModeFile(&txt, &mode);
    assert!(ret);

    mode = directory_ex::S_IRUSR  | directory_ex::S_IRGRP | directory_ex::S_IROTH;
    ret = directory_ex::ffi::ChangeModeDirectory(&dirpath, &mode);
    assert!(ret);

    ret = directory_ex::ffi::ForceRemoveDirectory(&dirpath);
    assert!(ret);
}

#[test]
fn test_change_mode_file_001()
{
    let_cxx_string!(dirpath = "/data/test/utils_directory_tmp/test.txt");
    let mode = directory_ex::S_IRWXU | directory_ex::S_IRWXG | directory_ex::S_IRWXO;
    let ret = directory_ex::ffi::ChangeModeFile(&dirpath, &mode);
    assert!(!ret);
}

#[test]
fn test_change_mode_directory_001()
{
    let_cxx_string!(dirpath = "");
    let mode = directory_ex::S_IRUSR  | directory_ex::S_IRGRP | directory_ex::S_IROTH;
    let ret = directory_ex::ffi::ChangeModeDirectory(&dirpath, &mode);
    assert!(!ret);

    let resultsize = directory_ex::ffi::GetFolderSize(&dirpath);
    let resultcomp = 0;
    assert_eq!(resultsize, resultcomp);
}

#[test]
fn test_path_to_real_path_001()
{
    let path = "/data/test".to_string();
    let mut realpath = String::new();
    let ret = directory_ex::ffi::RustPathToRealPath(&path, &mut realpath);
    assert!(ret);
    assert_eq!(path, realpath);
}

#[test]
fn test_path_to_real_path_002()
{
    let path = "/data/../data/test".to_string();
    let mut realpath = String::new();
    let ret = directory_ex::ffi::RustPathToRealPath(&path, &mut realpath);
    assert!(ret);
    assert_eq!("/data/test", realpath);
}

#[test]
fn test_path_to_real_path_003()
{
    let path = "./".to_string();
    let mut realpath = String::new();
    let ret = directory_ex::ffi::RustPathToRealPath(&path, &mut realpath);
    assert!(ret);
    assert_eq!("/data/test", realpath);
}

#[test]
fn test_path_to_real_path_004()
{
    let path = String::new();
    let mut realpath = String::new();
    let ret = directory_ex::ffi::RustPathToRealPath(&path, &mut realpath);
    assert!(!ret);
}

#[test]
fn test_path_to_real_path_005()
{
    let path = "/data/test/data/test/data/test/data/test/data/test/data/ \
    test/data/test/data/test/data/test/data/test/data/test/data/test/data/ \
    test/data/test/data/test/data/test/data/test/data/test/data/test/data/ \
    test/data/test/data/test/data/test/data/test/data/test/data/test/data/ \
    test/data/test/data/test/data/test".to_string();
    let mut realpath = String::new();
    let ret = directory_ex::ffi::RustPathToRealPath(&path, &mut realpath);
    assert!(!ret);
}

#[test]
fn test_path_to_real_path_006()
{
    let path: String = "x".repeat(directory_ex::PATH_MAX);
    let mut realpath = String::new();
    let ret = directory_ex::ffi::RustPathToRealPath(&path, &mut realpath);
    assert!(!ret);
}