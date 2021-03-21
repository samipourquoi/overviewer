use std::path::{PathBuf, Path};

pub fn identifier_to_path(id: &str) -> PathBuf {
    let uri = if id.starts_with("minecraft:") {
        id.split_at("minecraft:".len()).1
    } else {
        id
    };
    let uri = uri.split_at("block/".len()).1;

    Path::new("./assets/textures")
        .join(uri)
        .with_extension("png")
}
