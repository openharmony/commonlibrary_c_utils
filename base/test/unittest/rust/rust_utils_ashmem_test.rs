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

use std::ptr;
use std::ffi::{c_char, CStr, CString};
use utils_rust::ashmem;

const MEMORY_SIZE: i32 = 1024;
const MEMORY_NAME: &str = "Test SharedMemory";
const MEMORY_CONTENT: &str = "HelloWorld2023";

#[test]
fn test_ashmem_ffi_write_read_001()
{
    let c_name = CString::new(MEMORY_NAME).expect("CString::new Failed!");
    let ashmem = unsafe { ashmem::ffi::CreateAshmemStd(c_name.as_ptr(), MEMORY_SIZE) };
    assert!(!ashmem.is_null());
    assert_eq!(ashmem.GetAshmemSize(), MEMORY_SIZE);

    assert!(ashmem.MapAshmem(ashmem::PROT_READ | ashmem::PROT_WRITE));

    ashmem.UnmapAshmem();
    ashmem.CloseAshmem();
}

#[test]
fn test_ashmem_ffi_write_read_002()
{
    let c_name = CString::new(MEMORY_NAME).expect("CString::new Failed!");
    let ashmem = unsafe { ashmem::ffi::CreateAshmemStd(c_name.as_ptr(), MEMORY_SIZE) };
    assert!(!ashmem.is_null());

    assert!(ashmem.MapReadAndWriteAshmem());

    let c_content = CString::new(MEMORY_CONTENT).expect("CString::new Failed!");
    let c_content_ptr: *const ashmem::ffi::c_void = unsafe { ashmem::ffi::AsVoidPtr(c_content.as_ptr()) };

    unsafe {
        assert!(ashmem.WriteToAshmem(c_content_ptr, MEMORY_CONTENT.len().try_into().expect("Invalid content size."), 0));
        assert!(ashmem.WriteToAshmem(c_content_ptr, MEMORY_CONTENT.len().try_into().expect("Invalid content size."),
                MEMORY_CONTENT.len().try_into().expect("Invalid content size.")));
    }
    let readout_ptr: *const c_char;
    unsafe {
        readout_ptr = ashmem::ffi::AsCharPtr(ashmem.ReadFromAshmem(MEMORY_CONTENT.len().try_into().expect("Invalid content size."), 0));
    }
    assert!(!readout_ptr.is_null());

    // readout_ptr requires a valid nul terminator at the end of the string.
    let readout_c_str: &CStr = unsafe { CStr::from_ptr(readout_ptr) };
    let readout_str: &str = readout_c_str.to_str().expect("Bad encoding.");

    let mut expect = String::from(MEMORY_CONTENT);
    expect.push_str(MEMORY_CONTENT);
    assert_eq!(readout_str, expect.as_str());

    ashmem.UnmapAshmem();
    ashmem.CloseAshmem();
}

#[test]
fn test_ashmem_ffi_write_read_003()
{
    let c_name = CString::new(MEMORY_NAME).expect("CString::new Failed!");
    let ashmem = unsafe { ashmem::ffi::CreateAshmemStd(c_name.as_ptr(), MEMORY_SIZE) };
    assert!(!ashmem.is_null());

    assert!(ashmem.MapReadAndWriteAshmem());

    let c_content = CString::new(MEMORY_CONTENT).expect("CString::new Failed!");
    let c_content_ptr: *const ashmem::ffi::c_void = unsafe { ashmem::ffi::AsVoidPtr(c_content.as_ptr()) };

    unsafe {
        assert!(ashmem.WriteToAshmem(c_content_ptr, MEMORY_CONTENT.len().try_into().expect("Invalid content size."), 0));
    }

    ashmem.UnmapAshmem();
    assert!(ashmem.MapReadOnlyAshmem());

    unsafe {
        assert!(!ashmem.WriteToAshmem(c_content_ptr, MEMORY_CONTENT.len().try_into().expect("Invalid content size."),
                MEMORY_NAME.len().try_into().expect("Invalid content size.")));
    }

    let readout_ptr: *const c_char;
    unsafe {
        readout_ptr = ashmem::ffi::AsCharPtr(ashmem.ReadFromAshmem(MEMORY_CONTENT.len().try_into().expect("Invalid content size."), 0));
    }
    assert!(!readout_ptr.is_null());

    // readout_ptr requires a valid nul terminator at the end of the string.
    let readout_c_str: &CStr = unsafe { CStr::from_ptr(readout_ptr) };
    let readout_str: &str = readout_c_str.to_str().expect("Bad encoding.");

    assert_eq!(readout_str, MEMORY_CONTENT);

    ashmem.UnmapAshmem();
    ashmem.CloseAshmem();
}

#[test]
fn test_ashmem_ffi_write_read_004()
{
    let c_name = CString::new(MEMORY_NAME).expect("CString::new Failed!");
    let ashmem = unsafe { ashmem::ffi::CreateAshmemStd(c_name.as_ptr(), MEMORY_SIZE) };
    assert!(!ashmem.is_null());

    assert!(ashmem.MapReadAndWriteAshmem());
    let c_content = CString::new(MEMORY_CONTENT).expect("CString::new Failed!");
    let c_content_ptr: *const ashmem::ffi::c_void = unsafe { ashmem::ffi::AsVoidPtr(c_content.as_ptr()) };
    unsafe {
        assert!(ashmem.WriteToAshmem(c_content_ptr, MEMORY_CONTENT.len().try_into().expect("Invalid content size."), 0));
    }

    ashmem.UnmapAshmem();
    assert!(ashmem.SetProtection(ashmem::PROT_READ));
    assert!(!ashmem.MapReadAndWriteAshmem());
    assert!(ashmem.MapReadOnlyAshmem());

    let readout_ptr: *const c_char;
    unsafe {
        readout_ptr = ashmem::ffi::AsCharPtr(ashmem.ReadFromAshmem(MEMORY_CONTENT.len().try_into().expect("Invalid content size."), 0));
    }
    assert!(!readout_ptr.is_null());

    // readout_ptr requires a valid nul terminator at the end of the string.
    let readout_c_str: &CStr = unsafe { CStr::from_ptr(readout_ptr) };
    let readout_str: &str = readout_c_str.to_str().expect("Bad encoding.");
    assert_eq!(readout_str, MEMORY_CONTENT);

    ashmem.UnmapAshmem();
    ashmem.CloseAshmem();
}

#[test]
fn test_ashmem_ffi_write_read_005()
{
    let c_name = CString::new(MEMORY_NAME).expect("CString::new Failed!");
    let ashmem = unsafe { ashmem::ffi::CreateAshmemStd(c_name.as_ptr(), MEMORY_SIZE) };
    assert!(!ashmem.is_null());

    assert!(ashmem.MapReadAndWriteAshmem());
    assert!(ashmem.SetProtection(ashmem::PROT_READ));
    let c_content = CString::new(MEMORY_CONTENT).expect("CString::new Failed!");
    let c_content_ptr: *const ashmem::ffi::c_void = unsafe { ashmem::ffi::AsVoidPtr(c_content.as_ptr()) };
    unsafe {
        assert!(!ashmem.WriteToAshmem(c_content_ptr, MEMORY_CONTENT.len().try_into().expect("Invalid content size."), 0));
    }

    assert!(ashmem.MapReadOnlyAshmem());

    let readout_ptr: *const c_char;
    unsafe {
        readout_ptr = ashmem::ffi::AsCharPtr(ashmem.ReadFromAshmem(MEMORY_CONTENT.len().try_into().expect("Invalid content size."), 0));
    }
    assert!(!readout_ptr.is_null());

    // readout_ptr requires a valid nul terminator at the end of the string.
    let readout_c_str: &CStr = unsafe { CStr::from_ptr(readout_ptr) };
    let readout_str: &str = readout_c_str.to_str().expect("Bad encoding.");
    assert_eq!(readout_str, "");
    
    ashmem.UnmapAshmem();
    ashmem.CloseAshmem();
}

#[test]
fn test_ashmem_ffi_invalid_001()
{
    let c_name = CString::new(MEMORY_NAME).expect("CString::new Failed!");
    let ashmem = unsafe { ashmem::ffi::CreateAshmemStd(c_name.as_ptr(), -1) };
    assert!(ashmem.is_null());

    let ashmem = unsafe { ashmem::ffi::CreateAshmemStd(c_name.as_ptr(), MEMORY_SIZE) };
    assert!(!ashmem.is_null());

    assert!(!ashmem.SetProtection(-1));

    ashmem.CloseAshmem();
}

#[test]
fn test_ashmem_ffi_invalid_002()
{
    let c_name = CString::new(MEMORY_NAME).expect("CString::new Failed!");
    let ashmem = unsafe { ashmem::ffi::CreateAshmemStd(c_name.as_ptr(), MEMORY_SIZE) };
    assert!(!ashmem.is_null());

    ashmem.CloseAshmem();
    assert!(!ashmem.MapReadAndWriteAshmem());
}

#[test]
fn test_ashmem_ffi_invalid_003()
{
    let c_name = CString::new(MEMORY_NAME).expect("CString::new Failed!");
    let ashmem = unsafe { ashmem::ffi::CreateAshmemStd(c_name.as_ptr(), MEMORY_SIZE) };
    assert!(!ashmem.is_null());

    assert!(ashmem.MapReadAndWriteAshmem());

    ashmem.CloseAshmem();
    let c_content = CString::new(MEMORY_CONTENT).expect("CString::new Failed!");
    let c_content_ptr: *const ashmem::ffi::c_void = unsafe { ashmem::ffi::AsVoidPtr(c_content.as_ptr()) };
    unsafe {
        assert!(!ashmem.WriteToAshmem(c_content_ptr, MEMORY_CONTENT.len().try_into().expect("Invalid content size."), 0));
    }

    let readout_ptr: *const c_char;
    unsafe {
        readout_ptr = ashmem::ffi::AsCharPtr(ashmem.ReadFromAshmem(MEMORY_CONTENT.len().try_into().expect("Invalid content size."), 0));
    }
    assert!(readout_ptr.is_null());
}

#[test]
fn test_ashmem_ffi_invalid_004()
{
    let c_name = CString::new(MEMORY_NAME).expect("CString::new Failed!");
    let ashmem = unsafe { ashmem::ffi::CreateAshmemStd(c_name.as_ptr(), MEMORY_SIZE) };
    assert!(!ashmem.is_null());

    assert!(ashmem.MapReadAndWriteAshmem());

    ashmem.UnmapAshmem();
    let c_content = CString::new(MEMORY_CONTENT).expect("CString::new Failed!");
    let c_content_ptr: *const ashmem::ffi::c_void = unsafe { ashmem::ffi::AsVoidPtr(c_content.as_ptr()) };
    unsafe {
        assert!(!ashmem.WriteToAshmem(c_content_ptr, MEMORY_CONTENT.len().try_into().expect("Invalid content size."), 0));
    }

    let readout_ptr: *const c_char;
    unsafe {
        readout_ptr = ashmem::ffi::AsCharPtr(ashmem.ReadFromAshmem(MEMORY_CONTENT.len().try_into().expect("Invalid content size."), 0));
    }
    assert!(readout_ptr.is_null());

    ashmem.CloseAshmem();
}

#[test]
fn test_ashmem_ffi_invalid_005()
{
    let c_name = CString::new(MEMORY_NAME).expect("CString::new Failed!");
    let ashmem = unsafe { ashmem::ffi::CreateAshmemStd(c_name.as_ptr(), MEMORY_SIZE) };
    assert!(!ashmem.is_null());

    assert!(ashmem.SetProtection(ashmem::PROT_WRITE));
    assert!(!ashmem.SetProtection(ashmem::PROT_READ));
    assert!(!ashmem.SetProtection(ashmem::PROT_READ | ashmem::PROT_WRITE));
    assert!(ashmem.SetProtection(ashmem::PROT_NONE));
    assert!(!ashmem.SetProtection(ashmem::PROT_READ));
    assert!(!ashmem.SetProtection(ashmem::PROT_WRITE));

    ashmem.CloseAshmem();
}


#[test]
fn test_ashmem_ffi_invalid_006()
{
    let c_name = CString::new(MEMORY_NAME).expect("CString::new Failed!");
    let ashmem = unsafe { ashmem::ffi::CreateAshmemStd(c_name.as_ptr(), MEMORY_SIZE) };
    assert!(!ashmem.is_null());

    assert!(ashmem.MapReadAndWriteAshmem());
    let c_content = CString::new(MEMORY_CONTENT).expect("CString::new Failed!");
    let c_content_ptr: *const ashmem::ffi::c_void = unsafe { ashmem::ffi::AsVoidPtr(c_content.as_ptr()) };
    unsafe {
        assert!(!ashmem.WriteToAshmem(ptr::null(), MEMORY_CONTENT.len().try_into().expect("Invalid content size."), 0));
        assert!(!ashmem.WriteToAshmem(c_content_ptr, MEMORY_CONTENT.len().try_into().expect("Invalid content size."), MEMORY_SIZE));
        assert!(!ashmem.WriteToAshmem(c_content_ptr, MEMORY_CONTENT.len().try_into().expect("Invalid content size."), MEMORY_SIZE + 1));
        assert!(!ashmem.WriteToAshmem(c_content_ptr, MEMORY_CONTENT.len().try_into().expect("Invalid content size."), -1));
        assert!(!ashmem.WriteToAshmem(c_content_ptr, MEMORY_SIZE + 1, 0));
        assert!(!ashmem.WriteToAshmem(c_content_ptr, -1, 0));
    }

    ashmem.UnmapAshmem();
    ashmem.CloseAshmem();
}

#[test]
fn test_ashmem_write_read_001()
{
    let ashmem = unsafe { ashmem::create_ashmem_instance(MEMORY_NAME, MEMORY_SIZE) };
    assert!(ashmem.is_some());

    let ashmem = ashmem.unwrap();
    assert_eq!(ashmem.get_ashmem_size(), MEMORY_SIZE);
    assert_eq!(ashmem.get_protection(), ashmem::PROT_READ | ashmem::PROT_WRITE | ashmem::PROT_EXEC); // default protection mask.
    assert_ne!(ashmem.get_ashmem_fd(), -1);

    assert!(ashmem.map_ashmem(ashmem::PROT_READ | ashmem::PROT_WRITE));

    ashmem.unmap_ashmem();
    ashmem.close_ashmem();
}

#[test]
fn test_ashmem_write_read_002()
{
    let ashmem = unsafe { ashmem::create_ashmem_instance(MEMORY_NAME, MEMORY_SIZE) };
    assert!(ashmem.is_some());

    let ashmem = ashmem.unwrap();
    assert!(ashmem.map_read_write_ashmem());

    let c_content = CString::new(MEMORY_CONTENT).expect("CString::new Failed!");
    unsafe {
        assert!(ashmem.write_to_ashmem(c_content.as_ptr(), MEMORY_CONTENT.len().try_into().expect("Invalid content size."), 0));
        assert!(ashmem.write_to_ashmem(c_content.as_ptr(), MEMORY_CONTENT.len().try_into().expect("Invalid content size."),
                MEMORY_CONTENT.len().try_into().expect("Invalid content size.")));
    }
    let readout_ptr: *const c_char;
    unsafe {
        readout_ptr = ashmem.read_from_ashmem(MEMORY_CONTENT.len().try_into().expect("Invalid content size."), 0);
        assert!(!readout_ptr.is_null());
    }

    // readout_ptr requires a valid nul terminator at the end of the string.
    let readout_c_str: &CStr = unsafe { CStr::from_ptr(readout_ptr) };
    let readout_str: &str = readout_c_str.to_str().expect("Bad encoding.");

    let mut expect = String::from(MEMORY_CONTENT);
    expect.push_str(MEMORY_CONTENT);
    assert_eq!(readout_str, expect.as_str());

    ashmem.unmap_ashmem();
    ashmem.close_ashmem();
}

#[test]
fn test_ashmem_write_read_003()
{
    let ashmem = unsafe { ashmem::create_ashmem_instance(MEMORY_NAME, MEMORY_SIZE) };
    assert!(ashmem.is_some());

    let ashmem = ashmem.unwrap();
    assert!(ashmem.map_read_write_ashmem());

    let c_content = CString::new(MEMORY_CONTENT).expect("CString::new Failed!");
    unsafe {
        assert!(ashmem.write_to_ashmem(c_content.as_ptr(), MEMORY_CONTENT.len().try_into().expect("Invalid content size."), 0));
    }

    ashmem.unmap_ashmem();
    assert!(ashmem.map_read_only_ashmem());

    unsafe {
        assert!(!ashmem.write_to_ashmem(c_content.as_ptr(), MEMORY_CONTENT.len().try_into().expect("Invalid content size."),
                MEMORY_CONTENT.len().try_into().expect("Invalid content size.")));
    }

    let readout_ptr: *const c_char;
    unsafe {
        readout_ptr = ashmem.read_from_ashmem(MEMORY_CONTENT.len().try_into().expect("Invalid content size."), 0);
        assert!(!readout_ptr.is_null());
    }

    // readout_ptr requires a valid nul terminator at the end of the string.
    let readout_c_str: &CStr = unsafe { CStr::from_ptr(readout_ptr) };
    let readout_str: &str = readout_c_str.to_str().expect("Bad encoding.");

    assert_eq!(readout_str, MEMORY_CONTENT);

    ashmem.unmap_ashmem();
    ashmem.close_ashmem();
}

#[test]
fn test_ashmem_write_read_004()
{
    let ashmem = unsafe { ashmem::create_ashmem_instance(MEMORY_NAME, MEMORY_SIZE) };
    assert!(ashmem.is_some());

    let ashmem = ashmem.unwrap();
    assert!(ashmem.map_read_write_ashmem());

    let c_content = CString::new(MEMORY_CONTENT).expect("CString::new Failed!");
    unsafe {
        assert!(ashmem.write_to_ashmem(c_content.as_ptr(), MEMORY_CONTENT.len().try_into().expect("Invalid content size."), 0));
    }

    ashmem.unmap_ashmem();
    assert!(ashmem.set_protection(ashmem::PROT_READ));
    assert!(!ashmem.map_read_write_ashmem());
    assert!(ashmem.map_read_only_ashmem());

    let readout_ptr: *const c_char;
    unsafe {
        readout_ptr = ashmem.read_from_ashmem(MEMORY_CONTENT.len().try_into().expect("Invalid content size."), 0);
        assert!(!readout_ptr.is_null());
    }

    // readout_ptr requires a valid nul terminator at the end of the string.
    let readout_c_str: &CStr = unsafe { CStr::from_ptr(readout_ptr) };
    let readout_str: &str = readout_c_str.to_str().expect("Bad encoding.");
    assert_eq!(readout_str, MEMORY_CONTENT);

    ashmem.unmap_ashmem();
    ashmem.close_ashmem();
}

#[test]
fn test_ashmem_write_read_005()
{
    let ashmem = unsafe { ashmem::create_ashmem_instance(MEMORY_NAME, MEMORY_SIZE) };
    assert!(ashmem.is_some());

    let ashmem = ashmem.unwrap();
    assert!(ashmem.map_read_write_ashmem());
    assert!(ashmem.set_protection(ashmem::PROT_READ));
    let c_content = CString::new(MEMORY_CONTENT).expect("CString::new Failed!");
    unsafe {
        assert!(!ashmem.write_to_ashmem(c_content.as_ptr(), MEMORY_CONTENT.len().try_into().expect("Invalid content size."), 0));
    }

    assert!(ashmem.map_read_only_ashmem());

    let readout_ptr: *const c_char;
    unsafe {
        readout_ptr = ashmem.read_from_ashmem(MEMORY_CONTENT.len().try_into().expect("Invalid content size."), 0);
        assert!(!readout_ptr.is_null());
    }

    // readout_ptr requires a valid nul terminator at the end of the string.
    let readout_c_str: &CStr = unsafe { CStr::from_ptr(readout_ptr) };
    let readout_str: &str = readout_c_str.to_str().expect("Bad encoding.");
    assert_eq!(readout_str, "");

    ashmem.unmap_ashmem();
    ashmem.close_ashmem();
}

#[test]
fn test_ashmem_invalid_001()
{
    let ashmem = unsafe { ashmem::create_ashmem_instance(MEMORY_NAME, -1i32) };
    assert!(ashmem.is_none());

    let ashmem = unsafe { ashmem::create_ashmem_instance(MEMORY_NAME, MEMORY_SIZE) };
    assert!(ashmem.is_some());

    let ashmem = ashmem.unwrap();

    assert!(!ashmem.set_protection(-1));

    ashmem.close_ashmem();
}

#[test]
fn test_ashmem_invalid_002()
{
    let ashmem = unsafe { ashmem::create_ashmem_instance(MEMORY_NAME, MEMORY_SIZE) };
    assert!(ashmem.is_some());

    let ashmem = ashmem.unwrap();
    ashmem.close_ashmem();

    assert!(!ashmem.map_read_write_ashmem());
}


#[test]
fn test_ashmem_invalid_003()
{
    let ashmem = unsafe { ashmem::create_ashmem_instance(MEMORY_NAME, MEMORY_SIZE) };
    assert!(ashmem.is_some());

    let ashmem = ashmem.unwrap();

    assert!(ashmem.map_read_write_ashmem());

    ashmem.close_ashmem();
    let c_content = CString::new(MEMORY_CONTENT).expect("CString::new Failed!");
    unsafe {
        assert!(!ashmem.write_to_ashmem(c_content.as_ptr(), MEMORY_CONTENT.len().try_into().expect("Invalid content size."), 0));
    }

    let readout_ptr: *const c_char;
    unsafe {
        readout_ptr = ashmem.read_from_ashmem(MEMORY_CONTENT.len().try_into().expect("Invalid content size."), 0);
        assert!(readout_ptr.is_null());
    }
}

#[test]
fn test_ashmem_invalid_004()
{
    let ashmem = unsafe { ashmem::create_ashmem_instance(MEMORY_NAME, MEMORY_SIZE) };
    assert!(ashmem.is_some());

    let ashmem = ashmem.unwrap();

    assert!(ashmem.map_read_write_ashmem());

    ashmem.unmap_ashmem();
    let c_content = CString::new(MEMORY_CONTENT).expect("CString::new Failed!");
    unsafe {
        assert!(!ashmem.write_to_ashmem(c_content.as_ptr(), MEMORY_CONTENT.len().try_into().expect("Invalid content size."), 0));
    }

    let readout_ptr: *const c_char;
    unsafe {
        readout_ptr = ashmem.read_from_ashmem(MEMORY_CONTENT.len().try_into().expect("Invalid content size."), 0);
        assert!(readout_ptr.is_null());
    }

    ashmem.close_ashmem();
}

#[test]
fn test_ashmem_invalid_005()
{
    let ashmem = unsafe { ashmem::create_ashmem_instance(MEMORY_NAME, MEMORY_SIZE) };
    assert!(ashmem.is_some());

    let ashmem = ashmem.unwrap();

    assert!(ashmem.set_protection(ashmem::PROT_WRITE));
    assert!(!ashmem.set_protection(ashmem::PROT_READ));
    assert!(!ashmem.set_protection(ashmem::PROT_READ | ashmem::PROT_WRITE));
    assert!(ashmem.set_protection(ashmem::PROT_NONE));
    assert!(!ashmem.set_protection(ashmem::PROT_READ));
    assert!(!ashmem.set_protection(ashmem::PROT_WRITE));

    ashmem.close_ashmem();
}

#[test]
fn test_ashmem_invalid_006()
{
    let ashmem = unsafe { ashmem::create_ashmem_instance(MEMORY_NAME, MEMORY_SIZE) };
    assert!(ashmem.is_some());

    let ashmem = ashmem.unwrap();

    assert!(ashmem.map_read_write_ashmem());
    let c_content = CString::new(MEMORY_CONTENT).expect("CString::new Failed!");
    unsafe {
        assert!(!ashmem.write_to_ashmem(ptr::null(), MEMORY_CONTENT.len().try_into().expect("Invalid content size."), 0));
        assert!(!ashmem.write_to_ashmem(c_content.as_ptr(), MEMORY_CONTENT.len().try_into().expect("Invalid content size."), MEMORY_SIZE));
        assert!(!ashmem.write_to_ashmem(c_content.as_ptr(), MEMORY_CONTENT.len().try_into().expect("Invalid content size."), MEMORY_SIZE + 1));
        assert!(!ashmem.write_to_ashmem(c_content.as_ptr(), MEMORY_CONTENT.len().try_into().expect("Invalid content size."), -1));
        assert!(!ashmem.write_to_ashmem(c_content.as_ptr(), MEMORY_SIZE + 1, 0));
        assert!(!ashmem.write_to_ashmem(c_content.as_ptr(), -1, 0));
    }

    ashmem.unmap_ashmem();
    ashmem.close_ashmem();
}