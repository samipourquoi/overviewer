use std::collections::HashMap;
use serde_json::Value;
use lazy_static::lazy_static;
use std::sync::Mutex;
use std::path::{Path, PathBuf};
use std::fs;
use serde::Deserialize;
use std::borrow::Borrow;
use std::ops::Deref;

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
    // let model = Model::parse(PathBuf::from("./assets/models/block/bricks.json"));
}

#[derive(Deserialize, Debug)]
pub struct Model {
    parent: Option<String>,
    pub textures: Option<HashMap<String, String>>,
    pub elements: Option<Vec<Element>>
}

#[derive(Deserialize, Debug)]
pub struct Element {
    from: [f32; 3],
    to: [f32; 3],
    rotation: Option<Rotation>,
    pub faces: Faces
}

#[derive(Deserialize, Debug)]
pub struct Rotation {
    origin: [f32; 3],
    axis: String,
    angle:  f32,
    rescale: Option<bool>
}

#[derive(Deserialize, Debug)]
pub struct Faces {
    pub up:    Option<Face>,
    pub down:  Option<Face>,
    pub north: Option<Face>,
    pub south: Option<Face>,
    pub east:  Option<Face>,
    pub west:  Option<Face>
}

#[derive(Deserialize, Debug)]
pub struct Face {
    pub uv: Option<[f32; 4]>,
    pub texture: String,
    pub cullface: Option<Direction>,
    pub rotation: Option<i32>,
    pub tintindex: Option<i32>
}

#[derive(Deserialize, Debug)]
#[serde(rename_all = "snake_case")]
pub enum Direction {
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

        let contents = fs::read_to_string(&path)
            .unwrap();
        let mut model: Model = serde_json::from_str(contents.as_str())
            .unwrap();
        if let Some(parent) = &model.parent {
            let path = identifier_to_path(parent.as_str());
            let parent = Model::parse(path);
            model.inherit(parent);
        }
        model.unfold_textures();

        model
    }

    // Each field in 'textures' can either be an identifier to a texture,
    // or a pointer to another key in 'textures'. If the latter, it will
    // start with an #.
    // This effectively makes it a linked list, so we 'unfold' it with this
    // function to make it easier to bind textures later on.
    fn unfold_textures(&mut self) {
        if self.textures.is_none() { return; }

        let textures = self.textures.clone()
            .unwrap();

        fn dereference_pointer(pointer: &String, textures: &HashMap<String, String>) -> String {
            if pointer.starts_with('#') {
                let shortened_ptr = pointer[1..].to_string()
                    .clone();
                let value = textures.get(&shortened_ptr);
                if value.is_none() {
                    return pointer.clone();
                }

                dereference_pointer(value.unwrap(), textures).clone()
            } else {
                pointer.clone()
            }
        }

        let textures: HashMap<_, _> = textures.iter()
            .map( |(k,v)| (k.clone(), dereference_pointer(v, &textures)))
            .collect();
        self.textures = Some(textures);
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
            self.textures = Some(b);
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
