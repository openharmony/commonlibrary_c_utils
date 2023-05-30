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

#![feature(file_create_new)]
use utils_rust::file_ex;
use std::fs::File;
use std::io::prelude::*;
use std::fs::remove_file;
use std::os::unix::io::{AsRawFd, RawFd};
use std::ffi::c_char;

pub const CONTENT_STR: &str = "TTtt@#$%^&*()_+~`";
pub const FILE_PATH: &str = "./tmp.txt";
pub const NULL_STR: &str = "";
pub const MAX_FILE_LENGTH: usize = 32 * 1024 * 1024;

// This code is converted from 43 functions of the utils_file_test.cpp.

pub fn create_test_file(path: &String, content: &String) -> bool {
    if let Ok(mut file) = File::create(path) {
        if let Err(err) = file.write_all(content.as_bytes()) {
            println!("Error writing to file: {}", err);
            return false;
        }
        true
    } else {
        println!("Failed to create file: {}", path);
        false
    }
}

pub fn remove_test_file(path: &String) -> Result<(), std::io::Error> {
    remove_file(path)
}

#[test]
fn test_load_string_from_file_001() {
    let mut str = String::new();
    let filename = String::from("/proc/meminfo");
    assert!(file_ex::ffi::RustLoadStringFromFile(&filename, &mut str));

    let mut str2 = String::new();
    let file = File::open(&filename).expect("Failed to open file");
    let fd: RawFd = file.as_raw_fd();
    assert!(file_ex::ffi::RustLoadStringFromFd(fd, &mut str2));
    assert_eq!(str.len(), str2.len());

    let mut buff: Vec<c_char> = Vec::new();
    let ret = file_ex::ffi::RustLoadBufferFromFile(&filename, &mut buff);
    assert!(ret);
    assert_eq!(str2.len(), buff.len());
}

#[test]
fn test_load_string_from_file_002() {
    let mut str = String::new();
    let filename = NULL_STR.to_string();
    assert!(!file_ex::ffi::RustLoadStringFromFile(&filename, &mut str));
    assert!(str.is_empty());
}

#[test]
fn test_load_string_from_file_003() {
    let mut str = String::new();
    let filename = FILE_PATH.to_string() + ".003";
    let content = NULL_STR.to_string();
    create_test_file(&filename, &content);
    assert!(file_ex::ffi::RustLoadStringFromFile(&filename, &mut str));
    let _err = remove_test_file(&filename);
    assert_eq!(str, content);
}

#[test]
fn test_load_string_from_file_004() {
    let mut str = String::new();
    let filename = FILE_PATH.to_string() + ".004";
    let content = CONTENT_STR.to_string();
    create_test_file(&filename, &content);
    assert!(file_ex::ffi::RustLoadStringFromFile(&filename, &mut str));
    let _err = remove_test_file(&filename);
    assert_eq!(str, content);
}

#[test]
fn test_load_string_from_file_005() {
    let mut str = String::new();
    let filename = FILE_PATH.to_string() + ".005";
    let content: String = "t".repeat(MAX_FILE_LENGTH);
    create_test_file(&filename, &content);
    assert!(file_ex::ffi::RustLoadStringFromFile(&filename, &mut str));
    let _err = remove_test_file(&filename);
    assert_eq!(str, content);
}

#[test]
fn test_load_string_from_file_006() {
    let mut str = String::new();
    let filename = FILE_PATH.to_string() + ".006";
    let content: String = "t".repeat(MAX_FILE_LENGTH + 1);
    create_test_file(&filename, &content);
    assert!(!file_ex::ffi::RustLoadStringFromFile(&filename, &mut str));
    let _err = remove_test_file(&filename);
    assert!(str.is_empty());
}

#[test]
fn test_load_string_from_fd_001() {
    let mut result = String::new();
    assert!(!file_ex::ffi::RustLoadStringFromFd(-1, &mut result));
    assert_eq!(result, "");
}

#[test]
fn test_load_string_from_fd_002() {
    let mut result = String::new();
    let filename =FILE_PATH.to_string() + ".008";
    let content = NULL_STR.to_string();
    create_test_file(&filename, &content);
    let file = File::open(&filename).expect("Failed to open file");
    let fd: RawFd = file.as_raw_fd();
    assert!(file_ex::ffi::RustLoadStringFromFd(fd, &mut result));
    let _err = remove_test_file(&filename);
    assert_eq!(result, content);
}

#[test]
fn test_load_string_from_fd_003() {
    let mut result = String::new();
    let filename = FILE_PATH.to_string() + ".009";
    let content = CONTENT_STR.to_string();
    create_test_file(&filename, &content);
    let file = File::open(&filename).expect("Failed to open file");
    let fd: RawFd = file.as_raw_fd();
    assert!(file_ex::ffi::RustLoadStringFromFd(fd, &mut result));
    let _err = remove_test_file(&filename);
    assert_eq!(result, content);
}

#[test]
fn test_load_string_from_fd_004() {
    let mut result = String::new();
    let filename = FILE_PATH.to_string() + ".010";
    let content: String = "t".repeat(MAX_FILE_LENGTH);
    create_test_file(&filename, &content);
    let file = File::open(&filename).expect("Failed to open file");
    let fd: RawFd = file.as_raw_fd();
    assert!(file_ex::ffi::RustLoadStringFromFd(fd, &mut result));
    let _err = remove_test_file(&filename);
    assert_eq!(result, content);
}

#[test]
fn test_load_string_from_fd_005() {
    let mut result = String::new();
    let filename = FILE_PATH.to_string() + ".011";
    let content: String = "t".repeat(MAX_FILE_LENGTH + 1);
    create_test_file(&filename, &content);
    let file = File::open(&filename).expect("Failed to open file");
    let fd: RawFd = file.as_raw_fd();
    assert!(!file_ex::ffi::RustLoadStringFromFd(fd, &mut result));
    let _err = remove_test_file(&filename);
    assert_ne!(result, content);
}

#[test]
fn test_load_string_from_fd_006() {
    let mut result = String::new();
    let filename = FILE_PATH.to_string() + ".012";
    let content = CONTENT_STR.to_string();
    create_test_file(&filename, &content);
    let fd: i32;
    {
        let file = File::open(&filename).expect("Failed to open file");
        fd = file.as_raw_fd();
    }
    assert!(!file_ex::ffi::RustLoadStringFromFd(fd, &mut result));
    let _err = remove_test_file(&filename);
    assert_eq!(result, "");
}

#[test]
fn test_save_string_to_file_001() {
    let path = FILE_PATH.to_string() + ".013";
    let content = CONTENT_STR.to_string();
    let new_content = NULL_STR.to_string();
    create_test_file(&path, &content);
    let ret = file_ex::ffi::RustSaveStringToFile(&path, &new_content, true);
    assert!(ret);

    let mut load_result = String::new();
    assert!(file_ex::ffi::RustLoadStringFromFile(&path, &mut load_result));
    let _err = remove_test_file(&path);
    assert_eq!(load_result, content);
}

#[test]
fn test_save_string_to_file_002() {
    let path = FILE_PATH.to_string() + ".014";
    let content = "Before truncated!".to_string();
    create_test_file(&path, &content);

    let new_content = CONTENT_STR.to_string();
    assert!(file_ex::ffi::RustSaveStringToFile(&path, &new_content, true));

    let mut load_result = String::new();
    assert!(file_ex::ffi::RustLoadStringFromFile(&path, &mut load_result));
    let _err = remove_test_file(&path);
    assert_eq!(load_result, new_content);
}

#[test]
fn test_save_string_to_file_003() {
    let path = FILE_PATH.to_string() + ".015";
    let content = "Before truncated!".to_string();
    create_test_file(&path, &content);

    let new_content = NULL_STR.to_string();
    assert!(file_ex::ffi::RustSaveStringToFile(&path, &new_content, true));

    let mut load_result = String::new();
    let ret = file_ex::ffi::RustLoadStringFromFile(&path, &mut load_result);
    let _err = remove_test_file(&path);
    assert!(ret);
    assert_eq!(load_result, content);
}

#[test]
fn test_save_string_to_file_004() {
    let path = FILE_PATH.to_string()+ ".016";
    let new_content = NULL_STR.to_string();
    let content = "Before truncated!".to_string();
    create_test_file(&path, &content);
    assert!(file_ex::ffi::RustSaveStringToFile(&path, &new_content, false));

    let mut load_result = String::new();
    let ret = file_ex::ffi::RustLoadStringFromFile(&path, &mut load_result);
    let _err = remove_test_file(&path);
    assert!(ret);
    assert_eq!(load_result, content);
}

#[test]
fn test_save_string_to_file_005() {
    let path = FILE_PATH.to_string()+ ".017";
    let content = "Before truncated!".to_string();
    create_test_file(&path, &content);

    let new_content = CONTENT_STR.to_string();
    assert!(file_ex::ffi::RustSaveStringToFile(&path, &new_content, false));

    let mut load_result = String::new();
    let ret = file_ex::ffi::RustLoadStringFromFile(&path, &mut load_result);
    let _err = remove_test_file(&path);
    assert!(ret);
    assert_eq!(load_result, content + &new_content);
}

#[test]
fn test_save_string_to_fd_001() {
    let mut content = String::new();
    let mut ret = file_ex::ffi::RustSaveStringToFd(0, &content);
    assert!(!ret);
    ret = file_ex::ffi::RustSaveStringToFd(-1, &content);
    assert!(!ret);

    content = CONTENT_STR.to_string();
    ret = file_ex::ffi::RustSaveStringToFd(0, &content);
    assert!(!ret);
    ret = file_ex::ffi::RustSaveStringToFd(-1, &content);
    assert!(!ret);
}

#[test]
fn test_save_string_to_fd_002() {
    let content = String::new();
    let filename = FILE_PATH.to_string() + ".019";
    let mut file = File::create_new(&filename).expect("Failed to create file");
    let mut fd: RawFd = file.as_raw_fd();
    let mut ret = file_ex::ffi::RustSaveStringToFd(fd, &content);
    assert!(ret);

    let mut load_result = String::new();
    file = File::open(&filename).expect("Failed to open file");
    fd = file.as_raw_fd();
    ret = file_ex::ffi::RustLoadStringFromFd(fd, &mut load_result);
    let _err = remove_test_file(&filename);
    assert!(ret);
    assert_eq!(load_result, "");
}

#[test]
fn test_save_string_to_fd_003() {
    let content = CONTENT_STR.to_string();
    let filename = FILE_PATH.to_string() + ".020";
    let mut file = File::create_new(&filename).expect("Failed to create file");
    let mut fd: RawFd = file.as_raw_fd();
    let mut ret = file_ex::ffi::RustSaveStringToFd(fd, &content);
    assert!(ret);

    let mut load_result = String::new();
    file = File::open(&filename).expect("Failed to open file");
    fd = file.as_raw_fd();
    ret = file_ex::ffi::RustLoadStringFromFd(fd, &mut load_result);
    let _err = remove_test_file(&filename);
    assert!(ret);
    assert_eq!(load_result, content);
}

#[test]
fn test_save_string_to_fd_004() {
    let content = CONTENT_STR.to_string();
    let filename = FILE_PATH.to_string() + ".021";
    File::create(&filename).expect("Failed to create file");
    let mut file = File::open(&filename).expect("Failed to open file");
    let mut fd: RawFd = file.as_raw_fd();
    let mut ret = file_ex::ffi::RustSaveStringToFd(fd, &content);
    assert!(!ret);

    let mut load_result = String::new();
    file = File::open(&filename).expect("Failed to open file");
    fd = file.as_raw_fd();
    ret = file_ex::ffi::RustLoadStringFromFd(fd, &mut load_result);
    let _err = remove_test_file(&filename);
    assert!(ret);
    assert_eq!(load_result, "");
}

#[test]
fn test_load_buffer_from_file_001() {
    let mut buff: Vec<c_char> = Vec::new();
    let filename = FILE_PATH.to_string() + ".022";
    let ret = file_ex::ffi::RustLoadBufferFromFile(&filename, &mut buff);
    assert!(!ret);
    assert_eq!(0, buff.len());
}

#[test]
fn test_load_buffer_from_file_002() {
    let mut buff: Vec<c_char> = Vec::new();
    let filename = FILE_PATH.to_string() + ".023";
    let content = "".to_string();
    create_test_file(&filename, &content);
    let ret = file_ex::ffi::RustLoadBufferFromFile(&filename, &mut buff);
    let _err = remove_test_file(&filename);
    assert!(ret);
    assert_eq!(0, buff.len());
}

#[test]
fn test_load_buffer_from_file_003() {
    let mut buff: Vec<c_char> = Vec::new();
    let filename = FILE_PATH.to_string() + ".024";
    let content = "TXB".to_string();
    create_test_file(&filename, &content);
    let ret = file_ex::ffi::RustLoadBufferFromFile(&filename, &mut buff);
    let _err = remove_test_file(&filename);
    assert!(ret);
    assert_eq!(3, buff.len());
    assert_eq!('T' as c_char, buff[0]);
    assert_eq!('X' as c_char, buff[1]);
    assert_eq!('B' as c_char, buff[2]);
}

#[test]
fn test_load_buffer_from_file_004() {
    let mut buff: Vec<c_char> = Vec::new();
    let filename = FILE_PATH.to_string() + ".025";
    let content = "t".repeat(MAX_FILE_LENGTH + 1);
    create_test_file(&filename, &content);
    let ret = file_ex::ffi::RustLoadBufferFromFile(&filename, &mut buff);
    let _err = remove_test_file(&filename);
    assert!(!ret);
    assert_eq!(0, buff.len());
}

#[test]
fn test_save_buffer_to_file_001() {
    let filename = FILE_PATH.to_string()  + ".026";
    let content = "ttxx".to_string();
    create_test_file(&filename, &content);
    let buff: Vec<c_char> = Vec::new();
    let ret = file_ex::ffi::RustSaveBufferToFile(&filename, &buff, false);
    assert!(ret);

    let mut load_result = String::new();
    let ret = file_ex::ffi::RustLoadStringFromFile(&filename, &mut load_result);
    let _err = remove_test_file(&filename);
    assert!(ret);
    assert_eq!(load_result, content);
}

#[test]
fn test_save_buffer_to_file_002() {
    let filename = FILE_PATH.to_string()  + ".027";
    let content = "ttxx".to_string();
    create_test_file(&filename, &content);

    let new_content: Vec<c_char> = vec!['x' as c_char, 'x' as c_char, 't' as c_char, 't' as c_char];
    let ret = file_ex::ffi::RustSaveBufferToFile(&filename, &new_content, true);
    assert!(ret);

    let mut load_result = String::new();
    let ret = file_ex::ffi::RustLoadStringFromFile(&filename, &mut load_result);
    let _err = remove_test_file(&filename);
    assert!(ret);
    assert_eq!(load_result, String::from_utf8_lossy(&new_content));
}

#[test]
fn test_save_buffer_to_file_003() {
    let filename = FILE_PATH.to_string() + ".028";
    let content = "ttxx".to_string();
    create_test_file(&filename, &content);

    let new_content: Vec<c_char> = vec!['x' as c_char, 'x' as c_char, 't' as c_char, 't' as c_char];
    let mut ret = file_ex::ffi::RustSaveBufferToFile(&filename, &new_content, false);
    assert!(ret);

    let mut load_result = String::new();
    ret = file_ex::ffi::RustLoadStringFromFile(&filename, &mut load_result);
    let _err = remove_test_file(&filename);
    assert!(ret);
    assert_eq!(&load_result, &(content + &String::from_utf8_lossy(&new_content)));
}

#[test]
fn test_string_exists_in_file_001() {
    let str_value = "abc".to_string();
    let filename = String::new();
    assert!(!file_ex::ffi::RustStringExistsInFile(&filename, &str_value, true));
    assert!(!str_value.is_empty());
}

#[test]
fn test_string_exists_in_file_002() {
    let str_value = NULL_STR.to_string();
    let filename = FILE_PATH.to_string() + ".030";
    let content = "hello world!".to_string();
    create_test_file(&filename, &content);
    assert!(!file_ex::ffi::RustStringExistsInFile(&filename, &str_value, true));
    let _err = remove_test_file(&filename);
}

#[test]
fn test_string_exists_in_file_003() {
    let str_value = "world".to_string();
    let filename = FILE_PATH.to_string() + ".031";
    let content = "hello world!".to_string();
    create_test_file(&filename, &content);
    assert!(file_ex::ffi::RustStringExistsInFile(&filename, &str_value, true));
    let _err = remove_test_file(&filename);
}

#[test]
fn test_string_exists_in_file_004() {
    let str_value1 = "t".repeat(MAX_FILE_LENGTH + 1);
    let str_value2 = "t".repeat(MAX_FILE_LENGTH);
    let filename = FILE_PATH.to_string() + ".032";
    let content = "t".repeat(MAX_FILE_LENGTH);
    create_test_file(&filename, &content);
    assert!(!file_ex::ffi::RustStringExistsInFile(&filename, &str_value1, true));
    assert!(file_ex::ffi::RustStringExistsInFile(&filename, &str_value2, true));
    let _err = remove_test_file(&filename);
}

#[test]
fn test_string_exists_in_file_005() {
    let str_value = "woRld".to_string();
    let filename = FILE_PATH.to_string() + ".033";
    let content = "hello world!".to_string();
    create_test_file(&filename, &content);
    assert!(file_ex::ffi::RustStringExistsInFile(&filename, &str_value, false));
    assert!(!file_ex::ffi::RustStringExistsInFile(&filename, &str_value, true));
    let _err = remove_test_file(&filename);
}

#[test]
fn test_string_exists_in_file_006() {
    let str_value1 = "woRld".to_string();
    let str_value2 = "123".to_string();
    let str_value3 = "llo ".to_string();
    let str_value4 = "123 w".to_string();
    let str_value5 = "hi".to_string();
    let filename = FILE_PATH.to_string() + ".034";
    let content = "Test, hello 123 World!".to_string();
    create_test_file(&filename, &content);
    assert!(file_ex::ffi::RustStringExistsInFile(&filename, &str_value1, false));
    assert!(!file_ex::ffi::RustStringExistsInFile(&filename, &str_value1, true));

    assert!(file_ex::ffi::RustStringExistsInFile(&filename, &str_value2, false));
    assert!(file_ex::ffi::RustStringExistsInFile(&filename, &str_value2, true));

    assert!(file_ex::ffi::RustStringExistsInFile(&filename, &str_value3, false));
    assert!(file_ex::ffi::RustStringExistsInFile(&filename, &str_value3, true));

    assert!(file_ex::ffi::RustStringExistsInFile(&filename, &str_value4, false));
    assert!(!file_ex::ffi::RustStringExistsInFile(&filename, &str_value4, true));

    assert!(!file_ex::ffi::RustStringExistsInFile(&filename, &str_value5, false));
    assert!(!file_ex::ffi::RustStringExistsInFile(&filename, &str_value5, true));
    let _err = remove_test_file(&filename);
}

#[test]
fn test_string_exists_in_file_007() {
    let str_value1 = "is".to_string();
    let str_value2 = "\n\ris".to_string();
    let filename = FILE_PATH.to_string() + ".035";
    let content = "Test, special string\n\ris ok".to_string();
    create_test_file(&filename, &content);
    assert!(file_ex::ffi::RustStringExistsInFile(&filename, &str_value1, false));
    assert!(file_ex::ffi::RustStringExistsInFile(&filename, &str_value1, true));

    assert!(file_ex::ffi::RustStringExistsInFile(&filename, &str_value2, false));
    assert!(file_ex::ffi::RustStringExistsInFile(&filename, &str_value2, true));
    let _err = remove_test_file(&filename);
}

#[test]
fn test_file_exist_001()
{
    let filepath = "/proc/meminfo".to_string();
    let filepath1 = "/proc/meminfo1".to_string();

    assert!(file_ex::ffi::RustFileExists(&filepath));
    assert!(!(file_ex::ffi::RustFileExists(&filepath1)));
}

#[test]
fn test_count_str_in_file_001()
{
    let str = "abc".to_string();
    let filename = "".to_string();
    assert_eq!(file_ex::ffi::RustCountStrInFile(&filename, &str, true), -1);
    assert!(!str.is_empty());
}

#[test]
fn test_count_str_in_file_002()
{
    let str = NULL_STR.to_string();
    let filename = FILE_PATH.to_string() + ".038";
    let content = "hello world!".to_string();
    create_test_file(&filename, &content);
    assert_eq!(file_ex::ffi::RustCountStrInFile(&filename, &str, true), -1);
    let _err = remove_test_file(&filename);
}

#[test]
fn test_count_str_in_file_003()
{
    let str1 = "t".repeat(MAX_FILE_LENGTH + 1);
    let str2 = "t".repeat(MAX_FILE_LENGTH);
    let filename = FILE_PATH.to_string() + ".039";
    let content = "t".repeat(MAX_FILE_LENGTH);
    create_test_file(&filename, &content);
    assert_eq!(file_ex::ffi::RustCountStrInFile(&filename, &str1, true), 0);
    assert_eq!(file_ex::ffi::RustCountStrInFile(&filename, &str2, true), 1);
    let _err = remove_test_file(&filename);
}

#[test]
fn test_count_str_in_file_004()
{
    let str1 = "very".to_string();
    let str2 = "VERY".to_string();
    let str3 = "abc".to_string();
    let filename = FILE_PATH.to_string() + ".040";
    let content = "This is very very long string for test.\n Very Good,\r VERY HAPPY.".to_string();
    create_test_file(&filename, &content);
    assert_eq!(file_ex::ffi::RustCountStrInFile(&filename, &str1, true), 2);
    assert_eq!(file_ex::ffi::RustCountStrInFile(&filename, &str1, false), 4);

    assert_eq!(file_ex::ffi::RustCountStrInFile(&filename, &str2, true), 1);
    assert_eq!(file_ex::ffi::RustCountStrInFile(&filename, &str2, false), 4);

    assert_eq!(file_ex::ffi::RustCountStrInFile(&filename, &str3, true), 0);
    let _err = remove_test_file(&filename);
}

#[test]
fn test_count_str_in_file_005()
{
    let str1 = "aba".to_string();
    let filename = FILE_PATH.to_string() + ".041";
    let content = "This is abababaBABa.".to_string();
    create_test_file(&filename, &content);
    assert_eq!(file_ex::ffi::RustCountStrInFile(&filename, &str1, true), 2);
    assert_eq!(file_ex::ffi::RustCountStrInFile(&filename, &str1, false), 3);
    let _err = remove_test_file(&filename);
}