use std::process::Command;
use std::path::Path;

fn main() {
    /* Download the assets if they aren't present. */
    if !Path::new("./assets")
        .is_dir()
    {
        Command::new("git")
            .args(&["clone", "-b", "assets", "--single-branch",
                "https://github.com/samipourquoi/overviewer.git", "assets"])
            .spawn()
            .expect("failed to start git process.");
    }
}