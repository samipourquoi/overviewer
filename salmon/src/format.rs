pub use v116::*;

mod v116;

pub trait Format {
    // Uses '&self' to be able to use polymorphism, as it can't be used
    // with static functions (as far as I'm aware).
    fn parse(&self, data: &[u8]) -> Option<()>;
}

pub fn get_interface(version: u32) -> impl Format {
    match version {
        1_16 => V116 {},
        _ => panic!("version {} is not supported.", version)
    }
}