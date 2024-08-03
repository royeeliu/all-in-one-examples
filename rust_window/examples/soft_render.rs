use std::{num::NonZeroU32, rc::Rc};

use softbuffer::{Context, Surface};
use winit::{
    application::ApplicationHandler,
    event::WindowEvent,
    event_loop::{ActiveEventLoop, EventLoop},
    window::{Window, WindowId},
};

#[derive(Default)]
pub struct App {
    window: Option<Rc<Window>>,
    surface: Option<Surface<Rc<Window>, Rc<Window>>>,
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

                        let mut buffer = surface.buffer_mut().unwrap();
                        for y in 0..height.get() {
                            for x in 0..width.get() {
                                let red = x % 255;
                                let green = y % 255;
                                let blue = (x * y) % 255;
                                let index = y as usize * width.get() as usize + x as usize;
                                buffer[index] = blue | (green << 8) | (red << 16);
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
