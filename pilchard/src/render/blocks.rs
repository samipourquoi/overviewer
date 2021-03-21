use std::fmt::Debug;
use std::fs::File;

use crate::blocks::model::{Direction, Element, Face, Model};

const TILE_WIDTH: i32 = 28;
const TILE_HEIGHT: i32 = 32;
const TILE_TOP_HEIGHT: i32 = 16;

#[derive(Debug)]
pub struct RenderBlock {
    static_surface: cairo::ImageSurface,
    dyn_surface: cairo::ImageSurface,
}

impl RenderBlock {
    pub fn no_tint(&self) -> &cairo::ImageSurface {
        &self.static_surface
    }

    pub fn tinted(&self, color: u32) -> cairo::ImageSurface {
        let surface = cairo::ImageSurface::create(
            cairo::Format::ARgb32,
            TILE_WIDTH, TILE_HEIGHT)
            .unwrap();
        let context = cairo::Context::new(&surface);
        context.set_source_surface(&self.static_surface, 0.0, 0.0);
        context.paint();
        context.set_source_surface(&self.tint(color), 0.0, 0.0);
        context.paint();

        surface
    }

    fn tint(&self, color: u32) -> cairo::ImageSurface {
        let surface = cairo::ImageSurface::from(self.dyn_surface.clone());
        let context = cairo::Context::new(&surface);

        let r = ((color & 0xFF0000) >> 16) as f64;
        let g = ((color & 0x00FF00) >> 8) as f64;
        let b = (color & 0x0000FF) as f64;
        context.set_operator(cairo::Operator::Multiply);
        context.rectangle(0.0, 0.0, TILE_WIDTH as f64, TILE_HEIGHT as f64);
        context.set_source_rgba(r / 255.0, g / 255.0, b / 255.0, 1.0);
        context.fill();

        surface
    }
}

impl Model {
    pub fn render(&self) -> Option<RenderBlock> {
        let elements = self.elements.as_ref()?;
        let elements: Vec<_> = elements.iter()
            .map(|el| el.render(&self))
            .collect();
        let final_static = cairo::ImageSurface::create(
            cairo::Format::ARgb32,
            TILE_WIDTH, TILE_HEIGHT)
            .unwrap();
        let static_context = cairo::Context::new(&final_static);
        let final_dyn = cairo::ImageSurface::create(
            cairo::Format::ARgb32,
            TILE_WIDTH, TILE_HEIGHT)
            .unwrap();
        let dyn_context = cairo::Context::new(&final_dyn);

        for (static_el, dyn_el) in elements {
            static_context.set_source_surface(&static_el, 0.0, 0.0);
            static_context.paint();

            if let Some(dyn_el) = dyn_el {
                dyn_context.set_source_surface(&dyn_el, 0.0, 0.0);
                dyn_context.paint();
            }
        }

        Some(RenderBlock {
            static_surface: final_static,
            dyn_surface: final_dyn,
        })
    }
}

impl Element {
    // Returns a tuple with the first element being the isometric
    // render of the face, and the second one being the texture
    // to tint and overlay on top of the previous one, if the
    // face needs to be tinted.
    pub fn render(&self, model: &Model)
                  -> (cairo::ImageSurface, Option<cairo::ImageSurface>)
    {
        // The static surface refers the base image that always
        // stays the same, while the dynamic surface refers to the image
        // that will get tinted for each render.
        let static_surface = cairo::ImageSurface::create(
            cairo::Format::ARgb32,
            TILE_WIDTH, TILE_HEIGHT)
            .unwrap();
        let static_context = cairo::Context::new(&static_surface);

        let dyn_surface = cairo::ImageSurface::create(
            cairo::Format::ARgb32,
            TILE_WIDTH, TILE_HEIGHT)
            .unwrap();
        let dyn_context = cairo::Context::new(&dyn_surface);
        let is_any_face_tinted = vec![
            &self.faces.up, &self.faces.south, &self.faces.east
        ].iter().any(|face| face.is_some() && face.as_ref().unwrap().is_tinted());

        let render_face = |direction: Direction| {
            |face: &Face| {
                let block = face.render(model, direction);
                let context = if face.is_tinted() {
                    &dyn_context
                } else {
                    &static_context
                };
                context.set_source_surface(&block, 0.0, 0.0);
                context.paint();
            }
        };
        self.faces.up.as_ref().map(render_face(Direction::Up));
        self.faces.south.as_ref().map(render_face(Direction::South));
        self.faces.east.as_ref().map(render_face(Direction::East));

        (static_surface,
         if is_any_face_tinted { Some(dyn_surface) } else { None })
    }
}

impl Face {
    pub fn render(&self, model: &Model, direction: Direction) -> cairo::ImageSurface {
        let path = model.get_texture_path(self.texture.clone());

        if let Some(path) = path {
            let texture_surface = cairo::ImageSurface::
            create_from_png(&mut File::open(path).unwrap())
                .unwrap();
            deform(&texture_surface, direction)
        } else {
            cairo::ImageSurface::create(
                cairo::Format::ARgb32,
                16, 16).unwrap()
        }
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

    // Transformation matrices that apply a scaling
    // and a shearing. It also applies a rotation for the TOP side.
    // I've found these values a while ago and I don't really remember
    // how I got them, but at least it works.
    // > http://jeroenhoek.nl/articles/svg-and-isometric-projection.html
    let matrix = match direction {
        Direction::Up => cairo::Matrix::new(
            COS_30, 0.5,
            -COS_30, 0.5,
            14.0, 0.0),
        Direction::South => cairo::Matrix::new(
            COS_30, 0.5,
            0.0, 1.0,
            0.0, (TILE_TOP_HEIGHT / 2) as f64,
        ),
        Direction::East => cairo::Matrix::new(
            COS_30, -0.5,
            0.0, 1.0,
            (TILE_WIDTH / 2) as f64, TILE_TOP_HEIGHT as f64,
        ),
        _ => unreachable!()
    };

    iso_context.transform(matrix);
    iso_context.set_source_surface(&surface, 0.0, 0.0);
    iso_context.paint();

    iso_surface
}
