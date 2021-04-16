/// The whole rendering process goes like this:
///
/// > 1. For each pixel in the textures used in each face, keep a point in memory
/// > with a pointer to the face/surface they're on.
/// >
/// > 2. Compute each point coordinate to an isometric projection.
/// >
/// > 3. Determine which points are culled and remove those which are not.
/// >
/// > 4. Do the inverse of step 2 to bring back points to their original position.
/// >
/// > 5. Get all the points that belong to each surface.
/// >
/// > 6. Create a 16x16 mask (the same size as the textures) from the points
/// > positions for each surface, and apply that mask to the corresponding face's texture.
/// >
/// > 7. Project each face's texture to an isometric projection, and put them all
/// > together to form one render. However, separate the faces that are tinted to the
/// > ones that are not. The former will get re-rendered each time (when needed) while
/// > the latter will not.
///
/// Step #2 and #4 can be eliminated if we change the angle from which we check if
/// a point is culled or not in step #3.

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

#[derive(Debug, Clone)]
struct VirtualPoint<'a> {
    coordinates: (f64, f64, f64),
    pixel: (i32, i32),
    face: &'a Face
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

    pub fn render2(&self) -> Option<RenderBlock> {
        // First, we keep track of each pixel of each texture.
        let virtualized: Vec<_> = self.elements.as_ref()?
            .iter()
            .map(Element::virtualize)
            .collect();
        let virtualized: Vec<VirtualPoint> = virtualized.clone()
            .concat();

        None
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

    /// Virtualizes each visible pixel of the element (step #1).
    /// In the isometric projection, (0,0,0) corresponds to the
    /// bottommost vertex at the front, while (15,15,15) corresponds
    /// to the uppermost vertex at the back.
    /// Just as in the game, +x is towards east and +z is towards south.
    ///
    /// For now, that method assumes that every block is composed of 1x1px surfaces
    /// or any whole number. Luckily, I don't there is any block in the current version of the game
    /// (as I'm writing this, 1.16.4) that doesn't fit this requirement.
    pub fn virtualize(&self) -> Vec<VirtualPoint> {
        let mut points = vec![];
        if let Some(face) = &self.faces.up {
            let direction = Direction::Up;
            let uv = face.uv.unwrap_or([0.0, 0.0, 16.0, 16.0]);

            // Amount of pixels shown on the face on their
            // respective axis.
            let amount_x = (uv[0] - uv[2]).abs() as i32;
            let amount_y = (uv[1] - uv[3]).abs() as i32;

            let [dx,dy,dz] = self.from;

            // x and y are the coordinates of each pixel of the visible part of the texture.
            for x in 0..(amount_x - 1) {
                for y in 0..(amount_y - 1) {
                    let coordinates = match direction {
                        Direction::East  => (15.0            , (x as f32) + dy , (y as f32) + dz ),
                        Direction::South => ((x as f32) + dx , (y as f32) + dy , 15.0            ),
                        Direction::Up    => ((x as f32) + dx , 15.0            , (y as f32) + dz ),
                        _                => unreachable!()
                    };

                    let point = VirtualPoint {
                        // we can't cast tuples...
                        coordinates: (coordinates.0 as f64, coordinates.1 as f64, coordinates.2 as f64),
                        pixel: (x, y),
                        face
                    };
                    points.push(point);
                }
            }
        }

        points
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
