use std::collections::HashSet;
use std::fs;
use std::path::Path;

use serde_json::Value;

pub fn init() {
    let _block_states: Vec<_> = fs::read_dir(Path::new("./assets/blockstates"))
        .unwrap()
        .map(|a| a.unwrap().path())
        .map(|path| BlockState::parse(&path))
        .collect();
}

pub struct BlockState {
    variants: Vec<Variant>
}

type States = HashSet<(String, String)>;

pub struct Variant {
    model: String,
    x: i32,
    y: i32,
    uv_lock: bool,
    states: States,
}

impl BlockState {
    pub fn find(&self, states: States) -> Option<&Variant> {
        self.variants.iter()
            .find(|variant| variant.matches(states.clone()))
    }

    pub fn parse(path: &Path) -> Self {
        let contents = fs::read_to_string(path).unwrap();
        let json: Value = serde_json::from_str(contents.as_str()).unwrap();

        let variants = if !json["variants"].is_null() {
            parse_variants(&json)
        } else if !json["multipart"].is_null() {
            vec![]
        } else {
            panic!("malformed block state file: {:?}", path)
        };

        BlockState {
            variants
        }
    }
}

impl Variant {
    pub fn matches(&self, states: States) -> bool {
        self.states.eq(&states)
    }
}

fn parse_variants(json: &Value) -> Vec<Variant> {
    json["variants"]
        .as_object()
        .unwrap()
        .iter()
        .map(|(states, model)| {
            let states: States = states.split(',')
                .map(|state| {
                    let (k, v) = match state.find('=') {
                        Some(i) => state.split_at(i),
                        None => ("", "")
                    };
                    (k.to_string(), v.to_string())
                })
                .collect();

            // Each property can either be a single object, or an
            // array of object. This match is to transform it to single value
            // even if it is an array (it keeps the first element in that case).
            let variant = match model {
                Value::Object(obj)
                => obj,
                Value::Array(arr)
                => arr[0].as_object().unwrap(),
                _ => panic!()
            };

            Variant {
                model: variant.get("model").unwrap()
                    .as_str().unwrap()
                    .to_string(),
                x: variant.get("x")
                    .unwrap_or(&Value::from(0))
                    .as_i64().unwrap() as i32,
                y: variant.get("y")
                    .unwrap_or(&Value::from(0))
                    .as_i64().unwrap() as i32,
                uv_lock: variant.get("uvlock")
                    .unwrap_or(&Value::from(false))
                    .as_bool().unwrap(),
                states,
            }
        })
        .collect()
}
