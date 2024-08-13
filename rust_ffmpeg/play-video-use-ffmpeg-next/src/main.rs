use softbuffer::{Context, Surface};
use std::{env, num::NonZeroU32, rc::Rc};
use winit::{
    application::ApplicationHandler,
    event::WindowEvent,
    event_loop::{ActiveEventLoop, EventLoop},
    window::{Window, WindowId},
};

extern crate ffmpeg_next as ffmpeg;
use ffmpeg::software::scaling;

pub struct App {
    ictx: ffmpeg::format::context::Input,
    video_stream_index: usize,
    decoder: ffmpeg::decoder::Video,
    window: Option<Rc<Window>>,
    surface: Option<Surface<Rc<Window>, Rc<Window>>>,
}

impl App {
    pub fn new(
        ictx: ffmpeg::format::context::Input,
        video_stream_index: usize,
        decoder: ffmpeg::decoder::Video,
    ) -> Self {
        App {
            ictx,
            video_stream_index,
            decoder,
            window: None,
            surface: None,
        }
    }
}

impl ApplicationHandler for App {
    fn resumed(&mut self, event_loop: &ActiveEventLoop) {
        println!("Resumed");
        if self.window.is_none() {
            let attr = Window::default_attributes().with_title("Soft Render Example");
            let window = event_loop.create_window(attr).unwrap();
            let window = Rc::new(window);
            let context = Context::new(window.clone()).unwrap();
            let surface = Surface::new(&context, window.clone()).unwrap();
            self.window = Some(window);
            self.surface = Some(surface);
        }
    }

    fn window_event(
        &mut self,
        event_loop: &ActiveEventLoop,
        _window_id: WindowId,
        event: WindowEvent,
    ) {
        match event {
            WindowEvent::RedrawRequested => {
                if let (Some(width), Some(height)) = {
                    let size = self.window.as_ref().unwrap().inner_size();
                    (NonZeroU32::new(size.width), NonZeroU32::new(size.height))
                } {
                    if let Some(surface) = self.surface.as_mut() {
                        surface.resize(width, height).unwrap();

                        let mut scaler = scaling::Context::get(
                            self.decoder.format(),
                            self.decoder.width(),
                            self.decoder.height(),
                            ffmpeg::util::format::Pixel::RGBA,
                            width.get(),
                            height.get(),
                            scaling::Flags::BILINEAR,
                        )
                        .unwrap();

                        let mut decoded = ffmpeg::frame::Video::empty();
                        while unsafe { decoded.is_empty() } {
                            let mut packet = ffmpeg::codec::packet::Packet::empty();
                            packet.read(&mut self.ictx).unwrap();
                            if packet.stream() == self.video_stream_index {
                                self.decoder.send_packet(&packet).unwrap();
                                self.decoder.receive_frame(&mut decoded).err();
                            }
                        }

                        if unsafe { decoded.is_empty() } {
                            return;
                        }
                        let mut rgb_frame = ffmpeg::frame::Video::empty();
                        scaler.run(&decoded, &mut rgb_frame).unwrap();

                        let mut buffer = surface.buffer_mut().unwrap();
                        let data = rgb_frame.data(0);
                        for y in 0..height.get() {
                            for x in 0..width.get() {
                                let index = (y * rgb_frame.stride(0) as u32 + x * 4) as usize;
                                let pixel = &data[index..index + 3];
                                let red = pixel[0] as u32;
                                let green = pixel[1] as u32;
                                let blue = pixel[2] as u32;
                                let index = y as usize * width.get() as usize + x as usize;
                                buffer[index] = blue | (green << 8) | (red << 16);
                            }
                        }
                        buffer.present().unwrap();
                    }
                }
                self.window.as_ref().unwrap().request_redraw();
            }
            WindowEvent::CloseRequested => {
                println!("Close requested");
                event_loop.exit();
            }
            _ => (),
        }
    }
}

fn main() {
    if env::args().len() < 2 {
        println!("ERROR: No input file.");
        return;
    }

    let file_name = env::args().nth(1).unwrap();
    println!("Input file: {}", file_name);

    let path = std::path::Path::new(&file_name);
    if !path.exists() {
        println!("ERROR: File not found.");
        return;
    }

    ffmpeg::init().unwrap();

    let mut ictx = ffmpeg::format::input(path).unwrap();
    let input = ictx
        .streams()
        .best(ffmpeg::media::Type::Video)
        .ok_or(ffmpeg::Error::StreamNotFound)
        .unwrap();

    let video_stream_index = input.index();
    let context_decoder =
        ffmpeg::codec::context::Context::from_parameters(input.parameters()).unwrap();
    let mut decoder = context_decoder.decoder().video().unwrap();
    println!(
        "\nVideo: {:?}, {}x{}",
        decoder.id(),
        decoder.width(),
        decoder.height()
    );

    let mut app = App::new(ictx, video_stream_index, decoder);

    let event_loop = EventLoop::new().unwrap();
    event_loop.run_app(&mut app).expect("Run app failed");
}
