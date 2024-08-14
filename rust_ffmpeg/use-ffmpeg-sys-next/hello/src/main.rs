use std::{
    ffi::{CStr, CString},
    str::from_utf8_unchecked,
};

extern crate ffmpeg_sys_next as ffmpeg;

fn main() {
    let name = CString::new("h264").unwrap();
    let codec = unsafe { ffmpeg::avcodec_find_decoder_by_name(name.as_ptr()) };
    if codec.is_null() {
        println!("Codec not found");
        return;
    }

    let name = unsafe { from_utf8_unchecked(CStr::from_ptr((*codec).name).to_bytes()) };
    let long_name = unsafe { from_utf8_unchecked(CStr::from_ptr((*codec).long_name).to_bytes()) };
    println!("Codec: {} ({})", name, long_name);
}
