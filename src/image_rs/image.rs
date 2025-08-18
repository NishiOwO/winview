#![allow(non_camel_case_types)]

use core::{ffi::{c_char, c_int, c_void, CStr}, ptr::null_mut};
use std::{ffi::CString, fs::File, io::Read};

use alloc::{boxed::Box, string::{String, ToString}};
use image::{DynamicImage, EncodableLayout, ImageFormat};

extern crate alloc;

#[repr(C)]
struct wv_image {
    opaque: *mut c_void,
    width: c_int,
    height: c_int,
    fp: *mut c_void,
    name: *const c_char,
    close: *mut c_void,
    read: *mut c_void,
}

#[repr(C)]
struct imagersopaque_t {
    fmt: ImageFormat,
    image: DynamicImage,
    row_cursor: u32
}

impl imagersopaque_t {
    pub fn new(path: String) -> Result<Self, Box<dyn core::error::Error>>{ 
        // what format
        let mut buf = vec![];
        for byte in File::open(path)?.bytes() {
            buf.push(byte.unwrap());
        };

        Ok(Self {
            fmt: image::guess_format(&buf)?,
            image: image::load_from_memory(&buf)?,
            row_cursor: 0,
        })
    }
}

#[unsafe(no_mangle)]
extern "C" fn ImageRSDriverRead(ptr: *mut c_void) -> *mut u8 {
    assert!(!ptr.is_null());
    assert!(ptr.is_aligned());
    let wv = unsafe { (ptr as *mut wv_image).as_mut() }.unwrap();
    let img = unsafe { (wv.opaque as *mut imagersopaque_t).as_mut().unwrap() };

    let row = img.image.crop(0, img.row_cursor, img.image.width(), 1);
    img.row_cursor += 1;

    let rgba8 = row.into_rgba8();
    let bytes = rgba8.as_bytes().to_vec();
    return Box::leak(Box::new(bytes)).as_mut_ptr()
}
#[unsafe(no_mangle)]
extern "C" fn ImageRSDriverClose(ptr: *mut c_void) -> () {
    assert!(!ptr.is_null());
    assert!(ptr.is_aligned());
    let wv = unsafe { (ptr as *mut wv_image).as_mut() }.unwrap();
    let img = unsafe { (wv.opaque as *mut imagersopaque_t).as_mut().unwrap() };

    let _ = unsafe { Box::from_raw(img) };
}
#[unsafe(no_mangle)]
extern "C" fn ImageRSAllocate(path: *const c_char) -> *mut imagersopaque_t {
    let path_rs = unsafe { CStr::from_ptr(path) }.to_str().unwrap().to_string();
    match imagersopaque_t::new(path_rs) {
        Ok(a) => Box::leak(Box::new(a)) as *mut imagersopaque_t,
        Err(err) => {
            println!("error, returning null: {}",err);
            null_mut()
        },
    }
}
#[unsafe(no_mangle)]
unsafe extern "C" fn ImageRSGetImageValues(img: *mut imagersopaque_t, width: *mut c_int    , height: *mut c_int) -> *const c_char{
    assert!(!img.is_null());
    assert!(img.is_aligned());
    let img = unsafe {img.as_mut()}.unwrap();
    let _ = core::mem::replace( width.as_mut().unwrap(), img.image.width() as i32);
    let _ = core::mem::replace(height.as_mut().unwrap(), img.image.height() as i32);

    let path = match img.fmt {
        ImageFormat::Png => "PNG",
        ImageFormat::Jpeg => "JPEG",
        ImageFormat::Gif => "GIF",
        ImageFormat::WebP => "WEBP",
        ImageFormat::Pnm => "PNM",
        ImageFormat::Tiff => "TIFF",
        ImageFormat::Tga => "TGA",
        ImageFormat::Dds => "DDS",
        ImageFormat::Bmp => "BMP",
        ImageFormat::Ico => "ICO",
        ImageFormat::Hdr => "HDR",
        ImageFormat::OpenExr => "OPENEXR",
        ImageFormat::Farbfeld => "FARBFELD",
        ImageFormat::Avif => "AVIF",
        ImageFormat::Qoi => "QOI",
        ImageFormat::Pcx => "PCX",
        _ => todo!(),
    };

    return Box::leak(Box::new(CString::new(path).unwrap())).as_ptr();
}

//"avif", "bmp", "dds", "exr", "ff", "gif", "hdr", "ico", "pnm", "qoi", "tga", "tiff", "webp"