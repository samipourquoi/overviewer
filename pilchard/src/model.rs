use std::collections::HashMap;
use nbt::Value;
use lazy_static::lazy_static;
use std::sync::Mutex;
use std::path::{Path, PathBuf};

lazy_static! {
    static ref CACHE: Mutex<HashMap<&'static str, &'static str>> = {
        let mut m = HashMap::new();
        Mutex::new(m)
    };
}

pub fn init() {
    let models: Vec<_> = std::fs::read_dir("./assets/models/block")
        .expect("assets not found")
        .map( |file| file.unwrap().path() )
        .map(Model::parse)
        .collect();

    println!("{:?}", models);
}

struct Model;

impl Model {
    pub fn get(name: &str, block_states: &HashMap<String, Value>) -> Option<Self> {
        Some(Model {})
    }

    pub fn parse(path: PathBuf) -> () {

        ()
    }
}
