use crate::blocks::model::{Model, Face, Element, Direction};
use std::fs::File;
use crate::blocks::textures;
use std::path::PathBuf;

const TILE_WIDTH: i32 = 28;
const TILE_HEIGHT: i32 = 32;
const TILE_TOP_HEIGHT: i32 = 16;

pub struct BoundTextures {

}

impl Model {
    pub fn render(&self) {
        if self.elements.is_none() {
            return;
        }

        self.elements.as_ref().map( |elements| {
            let a: Vec<_> = elements.iter()
                .map( |e| e.render(&self) )
                .collect();
        } );
    }
}

impl Element {
    // Returns a tuple with the first element being the isometric
    // render of the face, and the second one being the texture
    // to tint and overlay on top of the previous one, if the
    // face needs to be tinted.
    pub fn render(&self, model: &Model)
        -> (cairo::Surface, Option<cairo::Surface>)
    {
        let surface = cairo::ImageSurface::create(
            cairo::Format::ARgb32,
            TILE_WIDTH, TILE_HEIGHT)
            .unwrap();
        let context = cairo::Context::new(&surface);

        let render_face = |direction: Direction| {
            |face: &Face| {
                let block = face.render(model, direction);
                context.set_source_surface(&block, 0.0, 0.0);
                context.paint();
            }
        };

        self.faces.up   .as_ref().map(render_face(Direction::Up));
        self.faces.south.as_ref().map(render_face(Direction::South));
        self.faces.east .as_ref().map(render_face(Direction::East));

        let mut file = File::create("./test.png").unwrap();
        surface.write_to_png(&mut file);

        unimplemented!()
    }
}

impl Face {
    pub fn render(&self, model: &Model, direction: Direction) -> cairo::ImageSurface {
        let path = model.get_texture_path(self.texture.clone())
            .unwrap_or(PathBuf::from("./assets/textures/block/air.png"));
        let mut file = File::open(path)
            .unwrap();
        let texture_surface = cairo::ImageSurface::create_from_png(&mut file)
            .unwrap();

        deform(&texture_surface, direction)
    }

    pub fn is_tinted(&self) -> bool {
        self.tintindex.is_some()
    }
}

fn deform(surface: &cairo::Surface, direction: Direction)
    -> cairo::ImageSurface
{
    let iso_surface = cairo::ImageSurface::create(
        cairo::Format::ARgb32,
        TILE_WIDTH, TILE_HEIGHT)
        .unwrap();
    let iso_context = cairo::Context::new(&iso_surface);

    const COS_30: f64 = 0.86602540378;

    let matrix = match direction {
        Direction::Up => cairo::Matrix::new(
            COS_30, 0.5,
            -COS_30, 0.5,
            14.0, 0.0),
        Direction::South => cairo::Matrix::new(
            COS_30, 0.5,
            0.0, 1.0,
            0.0, (TILE_TOP_HEIGHT / 2) as f64
        ),
        Direction::East => cairo::Matrix::new(
            COS_30, -0.5,
            0.0, 1.0,
            (TILE_WIDTH / 2) as f64, TILE_TOP_HEIGHT as f64
        ),
        _ => panic!("invalid direction")
    };

    iso_context.transform(matrix);
    iso_context.set_source_surface(&surface, 0.0, 0.0);
    iso_context.paint();

    iso_surface
}
