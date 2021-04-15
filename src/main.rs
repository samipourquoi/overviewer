#![allow(dead_code, unused_variables)]

mod region;
mod render;
mod blocks;

use crate::region::chunk::Chunk;

fn main() {
    blocks::init();
    Chunk::at(-1, -1)
        .unwrap();
}
