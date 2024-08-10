use std::{
    env,
    ffi::{CStr, CString},
    ptr, slice,
};

use rusty_ffmpeg::ffi;

fn main() {
    let mut args = Vec::new();
    for arg in env::args() {
        args.push(arg);
    }
    if args.len() < 2 {
        println!("ERROR: No input file.");
        return;
    }

    let file_name = args[1].as_str();
    println!("Input file: {}", file_name);

    let mut format_context_ptr = unsafe { ffi::avformat_alloc_context() };
    if format_context_ptr.is_null() {
        panic!("ERROR could not allocate memory for Format Context");
    }

    let c_file_name = CString::new(file_name).expect("CString::new failed");
    if unsafe {
        ffi::avformat_open_input(
            &mut format_context_ptr,
            c_file_name.as_ptr(),
            ptr::null_mut(),
            ptr::null_mut(),
        )
    } != 0
    {
        panic!("ERROR could not open the file");
    }

    let format_context = unsafe { format_context_ptr.as_mut() }.unwrap();

    let format_name = unsafe { CStr::from_ptr((*format_context.iformat).name) }
        .to_str()
        .unwrap();

    println!(
        "format {}, duration {} us, bit_rate {}",
        format_name, format_context.duration, format_context.bit_rate
    );

    if unsafe { ffi::avformat_find_stream_info(format_context, ptr::null_mut()) } < 0 {
        panic!("ERROR could not get the stream info");
    }

    let streams = unsafe {
        slice::from_raw_parts(format_context.streams, format_context.nb_streams as usize)
    };

    for (_i, stream) in streams
        .iter()
        .map(|stream| unsafe { stream.as_ref() }.unwrap())
        .enumerate()
    {
        println!(
            "AVStream->time_base before open coded {}/{}",
            stream.time_base.num, stream.time_base.den
        );
        println!(
            "AVStream->r_frame_rate before open coded {}/{}",
            stream.r_frame_rate.num, stream.r_frame_rate.den
        );
        println!("AVStream->start_time {}", stream.start_time);
        println!("AVStream->duration {}", stream.duration);
        println!("finding the proper decoder (CODEC)");

        let local_codec_params = unsafe { stream.codecpar.as_ref() }.unwrap();
        let local_codec =
            unsafe { ffi::avcodec_find_decoder(local_codec_params.codec_id).as_ref() }
                .expect("ERROR unsupported codec!");

        match local_codec_params.codec_type {
            ffi::AVMEDIA_TYPE_VIDEO => {
                println!(
                    "Video Codec: resolution {} x {}",
                    local_codec_params.width, local_codec_params.height
                );
            }
            ffi::AVMEDIA_TYPE_AUDIO => {
                println!(
                    "Audio Codec: {} channels, sample rate {}",
                    local_codec_params.ch_layout.nb_channels, local_codec_params.sample_rate
                );
            }
            _ => {}
        };

        let codec_name = unsafe { CStr::from_ptr(local_codec.name) }
            .to_str()
            .unwrap();

        println!(
            "\tCodec {} ID {} bit_rate {}",
            codec_name, local_codec.id, local_codec_params.bit_rate
        );
    }

    unsafe {
        ffi::avformat_close_input(&mut (format_context as *mut _));
    }
}
