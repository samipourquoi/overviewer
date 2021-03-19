use std::io::{Read, Seek};
use std::io::SeekFrom::{Start, Current};
use std::fs::File;
use miniz_oxide::inflate;
use std::error::Error;

const REGION_BASE_PATH: &str = "/Users/sami/Library/Application Support/minecraft/saves/Overviewer/region";

pub struct Chunk {}

impl Chunk {
    pub fn at(x: i32, z: i32) -> Option<Chunk> {
        let region_file_path = format!("{}/r.{}.{}.mca",
                                       REGION_BASE_PATH, x, z);
        let region_file = std::fs::File::open(region_file_path).ok()?;
        let raw_data = Chunk::read_data_from(region_file, x, z).ok()?;
        let _data = inflate::decompress_to_vec_zlib(&raw_data).ok()?;

        Some(Chunk {})
    }

    fn read_data_from(mut region_file: File, x: i32, z: i32) -> Result<Vec<u8>, Box<dyn Error>> {
        // Read the offset of the chunk data.
        let location_offset: usize = (4 * ((x & 31) + (z & 31) * 32)) as usize;
        let _timestamp_offset = location_offset + 4096;
        let mut location_header = [0; 4];

        region_file.seek(Start(location_offset as u64))?;
        region_file.read_exact(&mut location_header)?;

        // Read the chunk header data.
        let chunk_offset: usize = ((65536 * location_header[0] as u32 +
            256 * location_header[1] as u32 +
            1 * location_header[2] as u32
        ) * 4096) as usize;
        // if chunk_offset == 0 { return Err(()); }
        let mut chunk_header = [0; 5];

        region_file.seek(Start(chunk_offset as u64))?;
        region_file.read_exact(&mut chunk_header)?;

        // Read the chunk data.
        let chunk_data_length: usize = (16777216 * chunk_header[0] as u32 +
            65536 * chunk_header[1] as u32 +
            256 * chunk_header[2] as u32 +
            1 * chunk_header[3] as u32
            - 1) as usize;
        let mut chunk_data: Vec<u8> = vec![0; chunk_data_length];

        region_file.read_exact(&mut chunk_data)?;

        Ok(chunk_data)
    }
}
