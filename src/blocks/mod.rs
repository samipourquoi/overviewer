pub mod block_state;
pub mod model;
pub mod textures;
pub mod provider;

pub fn init() {
    provider::test();
    block_state::init();
}
