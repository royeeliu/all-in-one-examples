extern crate ffmpeg_next as ffmpeg;

fn main() {
    ffmpeg::init().unwrap();

    if let Some(codec) = ffmpeg::decoder::find_by_name("h264") {
        println!("Codec: {} ({})", codec.name(), codec.description());
    }
}
