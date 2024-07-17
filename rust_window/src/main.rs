use crate::app::App;
use winit::event_loop::EventLoop;

mod app;

fn main() {
    let event_loop = EventLoop::new().unwrap();
    let mut app = App::default();
    event_loop.run_app(&mut app).expect("Run app failed");
}
