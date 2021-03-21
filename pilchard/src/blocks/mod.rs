pub mod block_state;
pub mod model;
pub mod textures;

pub fn init() {
    model::init();
    block_state::init();
}
