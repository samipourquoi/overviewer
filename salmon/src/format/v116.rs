use std::collections::HashMap;

use nbt::Value;

pub struct V116;

impl super::Format for V116 {
    fn parse(&self, data: &[u8]) -> Option<()> {
        let nbt = nbt::Blob::from_reader(&mut data.clone()).ok()?;

        let sections = match nbt.get("Level")? {
            Value::Compound(m) =>
                match m.get("Sections")? {
                    Value::List(l) => Some(l),
                    _ => None
                },
            _ => None
        }?;

        let empty_hash: HashMap<String, Value> = HashMap::new();
        let mut blocks = vec![empty_hash; 16 * 256 * 16];

        for section in sections {
            if let Value::Compound(section) = section {
                parse_section(section, &mut blocks);
            }
        }

        println!("{:?}", blocks);

        Some(())
    }
}

fn parse_section(section: &HashMap<String, Value>,
                 blocks: &mut Vec<HashMap<String, Value>>) -> Option<()> {
    let palette = match section.get("Palette")? {
        Value::List(l) => Some(l),
        _ => None
    }?;

    let block_length = (palette.len() as f32).log2().ceil().max(4.0) as u32;
    let per_line = (64.0 / (block_length as f32)).floor() as u32;
    let bits_range = ((2 as u64).pow(block_length) - 1) as u64;

    let block_states = match section.get("BlockStates")? {
        Value::LongArray(arr) => Some(arr),
        _ => None
    }?;

    let y = match section.get("Y")? {
        Value::Byte(b) => Some(b.clone() as usize),
        _ => None
    }?;
    let mut blocks_index = y * 16 * 16 * 16;

    for line in block_states {
        // Having the line as a i64 cause problems when
        // computing the 'palette_index' and give the wrong result.
        // We just cast it to an u64 to solve this problem.
        let line = &(*line as u64);

        for i in 0..per_line {
            let shift = i * block_length;
            let bits_range = (bits_range << shift) as u64;
            let palette_index = (line & bits_range) >> shift;

            let block = &palette[palette_index as usize];

            if let Value::Compound(h) = block {
                blocks[blocks_index] = h.clone();
            }

            blocks_index += 1;
        }
    }

    Some(())
}
