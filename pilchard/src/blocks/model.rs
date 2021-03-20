use std::collections::HashMap;
use serde_json::Value;
use lazy_static::lazy_static;
use std::sync::Mutex;
use std::path::{Path, PathBuf};
use std::fs;
use serde::Deserialize;
use std::borrow::Borrow;

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
    // let model = Model::parse(PathBuf::from("./assets/models/block/cube_all.json"));
}

#[derive(Deserialize, Debug)]
struct Model {
    parent: Option<String>,
    textures: Option<HashMap<String, PathBuf>>,
    elements: Option<Vec<Element>>
}

#[derive(Deserialize, Debug)]
struct Element {
    from: [f32; 3],
    to: [f32; 3],
    rotation: Option<Rotation>,
    faces: Faces
}

#[derive(Deserialize, Debug)]
struct Rotation {
    origin: [f32; 3],
    axis: String,
    angle:  f32,
    rescale: Option<bool>
}

#[derive(Deserialize, Debug)]
struct Faces {
    up:    Option<Face>,
    down:  Option<Face>,
    north: Option<Face>,
    south: Option<Face>,
    east:  Option<Face>,
    west:  Option<Face>
}

#[derive(Deserialize, Debug)]
struct Face {
    uv: Option<[f32; 4]>,
    texture: String,
    cullface: Option<Direction>,
    rotation: Option<i32>,
    tintindex: Option<i32>
}

#[derive(Deserialize, Debug)]
#[serde(rename_all = "snake_case")]
enum Direction {
    Up,     Down,
    North,  South,
    East,   West,

    // 'bottom' can be seen once in the source (scaffolding_unstable.json),
    // and as I honestly don't want to dive into serde's documentation to see
    // how to make an alias for an enum, I later replace every occurrence of
    // Bottom by Down.
    Bottom
}

impl Default for Direction {
    fn default() -> Self {
        Self::Down
    }
}

impl Model {
    pub fn get(name: &str, block_states: &HashMap<String, Value>) -> Option<Self> {
        unimplemented!()
    }

    pub fn parse(path: PathBuf) -> Model {
        let contents = fs::read_to_string(path)
            .unwrap();
        let mut model: Model = serde_json::from_str(contents.as_str())
            .unwrap();
        if let Some(parent) = &model.parent {
            let path = identifier_to_path(parent.as_str());
            let parent = Model::parse(path);
            model.inherit(parent);
        }

        model
    }

    fn inherit(&mut self, parent: Model) {
        // 'elements' is overridden by inheritance
        // while 'textures' is merged.

        if self.elements.is_none() {
            self.elements = parent.elements;
        }

        if self.textures.is_none() {
            self.textures = parent.textures;
        } else {
            let mut a = parent.textures
                .unwrap_or(HashMap::new());
            let mut b = self.textures
                .as_ref()
                .unwrap()
                .clone();
            b.extend(a);
        }
    }
}

fn identifier_to_path(id: &str) -> PathBuf {
    let uri = if id.starts_with("minecraft:") {
        id.split_at("minecraft:".len()).1
    } else {
        id
    };

    Path::new("./assets/models")
        .join(uri)
        .with_extension("json")
}
