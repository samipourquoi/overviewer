use pilchard::blocks::block_state::BlockState;
use std::path::Path;

fn main() {
    let path = Path::new("./assets/blockstates/bricks.json");
    BlockState::parse(path);
    // pilchard::model::init();
    // salmon::Chunk::at(-1, -1)
    //     .unwrap();
}
