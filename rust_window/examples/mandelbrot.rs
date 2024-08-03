use std::{num::NonZeroU32, rc::Rc};

use num::Complex;
use softbuffer::{Context, Surface};
use winit::{
    application::ApplicationHandler,
    dpi::PhysicalSize,
    event::WindowEvent,
    event_loop::{ActiveEventLoop, EventLoop},
    window::{Window, WindowId},
};

fn pixel_to_point(
    bounds: (usize, usize),
    pixel: (usize, usize),
    upper_left: Complex<f64>,
    lower_right: Complex<f64>,
) -> Complex<f64> {
    let (width, height) = (
        lower_right.re - upper_left.re,
        upper_left.im - lower_right.im,
    );
    Complex {
        re: upper_left.re + pixel.0 as f64 * width / bounds.0 as f64,
        im: upper_left.im - pixel.1 as f64 * height / bounds.1 as f64,
    }
}

fn escape_time(c: Complex<f64>, limit: u32) -> Option<u32> {
    let mut z = Complex { re: 0.0, im: 0.0 };
    for i in 0..limit {
        if z.norm_sqr() > 4.0 {
            return Some(i);
        }
        z = z * z + c;
    }
    None
}

#[derive(Default)]
pub struct App {
    window: Option<Rc<Window>>,
    surface: Option<Surface<Rc<Window>, Rc<Window>>>,
}

impl ApplicationHandler for App {
    fn resumed(&mut self, event_loop: &ActiveEventLoop) {
        println!("Resumed");
        if self.window.is_none() {
            let attr = Window::default_attributes()
                .with_title("Mandelbrot Set")
                .with_inner_size(PhysicalSize::new(1600, 1200));
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

                        let mut buffer = surface.buffer_mut().unwrap();
                        for y in 0..height.get() {
                            for x in 0..width.get() {
                                let point = pixel_to_point(
                                    (width.get() as usize, height.get() as usize),
                                    (x as usize, y as usize),
                                    Complex { re: -1.2, im: 0.35 },
                                    Complex { re: -1.0, im: 0.2 },
                                );
                                let color = match escape_time(point, 255) {
                                    None => 0,
                                    Some(count) => {
                                        let red = count as u8;
                                        let green = 255 - red;
                                        let blue = 0;
                                        blue as u32 | ((green as u32) << 8) | ((red as u32) << 16)
                                    }
                                };
                                let index = y as usize * width.get() as usize + x as usize;
                                buffer[index] = color;
                            }
                        }
                        buffer.present().unwrap();
                    }
                }
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
    let event_loop = EventLoop::new().unwrap();
    let mut app = App::default();
    event_loop.run_app(&mut app).expect("Run app failed");
}
