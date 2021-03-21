use std::collections::HashMap;
use std::fs::File;

use lazy_static::lazy_static;

use crate::render::blocks::RenderBlock;

// cairo-rs uses internally std::ptr::NonNull, which for some reason,
// implements !Sync. This means it can't be shared across threads for
// whatever fucking reason. The borrow checker complains and the program
// won't compile.
// The force_send_sync crates makes possible to override the !Sync
// implementation, but is unsafe. But at least it compiles, and it works.
//
// Rust is cool and all but this motherfucker doesn't let me write what
// I want. (I am mildly infuriated right now because I have legit spent
// more than 3 hours looking for a solution to this problem)
type Store = force_send_sync::Sync<HashMap<String, Option<RenderBlock>>>;

lazy_static! {
    #[derive(Debug)]
    static ref STORE: Store = unsafe {
        let mut h = HashMap::new();

        for (name, render) in super::model::init() {
            h.insert(name, render);
        }

        force_send_sync::Sync::new(h)
    };
}

pub fn get_from_store(key: &str) -> Option<&RenderBlock> {
    STORE.get(key).unwrap_or(&None).as_ref()
}

pub fn test() {
    let model = get_from_store("crafting_table").unwrap();
    let surface = model.no_tint();
    surface.write_to_png(&mut File::create("test.png").unwrap())
        .unwrap();
}
