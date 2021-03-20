pub mod block_state;
pub mod model;

pub fn init() {
    model::init();
    block_state::init();
}
