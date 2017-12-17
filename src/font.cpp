//
// no-dice
//

#include "font.hpp"

#include "bin-pack.hpp"

#include <Rk/guard.hpp>

#include <iostream>
#include <map>
#include <set>

#include <ft2build.h>
#include FT_FREETYPE_H

#include <hb.h>
#include <hb-ft.h>

#include <epoxy/gl.h>

namespace nd {
  namespace {
    struct AtlasRect {
      int width;//  () const { return abs (max (tcs[1] - tcs[0])); }
      int height;// () const { return abs (max (tcs[2] - tcs[1])); }
      v2i tcs[4];
      explicit operator bool () const {
        return width != 0 && height != 0;
      }
    };

    struct LoadedGlyph {
      AtlasRect rect;
      v2i bearings;
    };

    struct Bitmap {
      std::vector<u8> pixels;
      int width, height;
      int min_side () const { return std::min (width, height); }
      Bitmap () = default;
      Bitmap (std::vector<u8> pixels, int w, int h) :
        pixels (std::move (pixels)), width (w), height (h)
      { }
      Bitmap (Bitmap&&) = default;
      Bitmap& operator = (Bitmap&&) = default;
    };

    struct LoadingGlyph {
      Bitmap bitmap;
      v2i bearings;
    };

    Bitmap pack_bitmap (
      u8 const* src, int sp,
      int sw, int sh,
      bool rotate, bool flip = false)
    {
      std::vector<u8> dest (sw*sh);

      int const
        dw = rotate? sh : sw,
        dh = rotate? sw : sh;

      for (int dy = 0; dy != dh; dy++) {
        for (int dx = 0; dx != dw; dx++) {
          int const
            sx = rotate? dy      : dx,
            sy = rotate? dw-dx-1 : dy,
            dr = flip? dh-dy-1 : dy;
          dest[dr*dw + dx] = src[sy*sp + sx];
        }
      }

      return { std::move (dest), dw, dh };
    }
  }

  class FontImpl final : public Font {
    FT_Face face;
    hb_font_t* hb;
    Rk::RectPacker packer;
    uint glname;
    std::map<uint, LoadedGlyph> loaded;

    LoadingGlyph load_glyph (uint const index) {
      // load and render
      if (FT_Load_Glyph (face, index, FT_LOAD_RENDER))
        throw std::runtime_error ("FT_Load_Glyph failed");

      auto const& src = face->glyph->bitmap;
      auto bmp = pack_bitmap (
        src.buffer, src.pitch, (int) src.width, (int) src.rows, false, true);

      v2i const bearings {
        face->glyph->bitmap_left,
        face->glyph->bitmap_top - bmp.height
      };

      return { std::move (bmp), bearings };
    }

    uint cp_to_glyph (u32 const cp) {
      uint gi = 0;
      hb_font_get_nominal_glyph (hb, cp, &gi);
      return gi;
    }

    LoadedGlyph pack_glyph (uint const index, LoadingGlyph g) {
      // try to pack the glyph into the atlas
      v2i const dims { g.bitmap.width, g.bitmap.height };
      auto const [ok, rotated, where] = packer.add (dims);
      if (!ok) {
        std::cerr << "Failed to pack glyph\n";
        return get_glyph (0); // FIXME: is this u+fffd?
      }

      // zero-area glyphs don't even occupy the atlas
      if (where) {
        // rotate glyph if necessary
        if (rotated) {
          g.bitmap = pack_bitmap (
            g.bitmap.pixels.data (),
            g.bitmap.width,
            g.bitmap.width, g.bitmap.height,
            true
          );
        }

        // upload glyph
        glPixelStorei (GL_UNPACK_ALIGNMENT, 1);
        glTextureSubImage2D (
          glname,
          0,
          where.xmin (),  where.ymin (),
          g.bitmap.width, g.bitmap.height,
          GL_RED, GL_UNSIGNED_BYTE, g.bitmap.pixels.data ()
        );
      }

      // compute a quad
      auto const rect = rotated
        ? AtlasRect {
            where.height (), where.width (),
            { v2i { where.xmax (), where.ymin () },
              v2i { where.xmax (), where.ymax () },
              v2i { where.xmin (), where.ymax () },
              v2i { where.xmin (), where.ymin () }
            }
          }
        : AtlasRect {
            where.width (), where.height (),
            { v2i { where.xmin (), where.ymin () },
              v2i { where.xmax (), where.ymin () },
              v2i { where.xmax (), where.ymax () },
              v2i { where.xmin (), where.ymax () }
            }
          };

      // cache for later
      LoadedGlyph lg { rect, g.bearings };
      loaded[index] = lg;
      return lg;
    }

    LoadedGlyph get_glyph (uint index) {
      // have we already loaded the glyph?
      if (auto iter = loaded.find (index); iter != loaded.end ())
        return iter->second;

      return pack_glyph (index, load_glyph (index));
    }

    enum { w = 1024, h = w };

  public:
    FontImpl (FT_Face face, hb_font_t* hb) :
      face (face), hb (hb),
      packer (v2i {w,h})
    {
      glCreateTextures (GL_TEXTURE_2D, 1, &glname);
      glTextureStorage2D (glname, 1, GL_R8, w, h);
      glClearTexImage (glname, 0, GL_RED, GL_UNSIGNED_BYTE, "\x7f");

      v2i constexpr const attribs[] {
        { GL_TEXTURE_MIN_FILTER, GL_LINEAR },
        { GL_TEXTURE_MAG_FILTER, GL_NEAREST },
        { GL_TEXTURE_WRAP_S,     GL_CLAMP_TO_EDGE },
        { GL_TEXTURE_WRAP_T,     GL_CLAMP_TO_EDGE }
      };
      for (v2i attr : attribs)
        glTextureParameteri (glname, attr.x, attr.y);

      get_glyph (0);
    }

    ~FontImpl () {
      hb_font_destroy (hb);
      FT_Done_Face (face);
      glDeleteTextures (1, &glname);
    }

    void prime (CharRanges const& ranges) override {
      std::set<uint> added;
      std::vector<std::pair<uint, LoadingGlyph>> loading;

      for (auto range : ranges) {
        for (auto cp : range) {
          uint gi = cp_to_glyph (cp);
          if (auto i = added.find (gi); i == added.end ()) {
            added.insert (gi);
            loading.push_back (std::make_pair (gi, load_glyph (gi)));
          }
        }
      }

      std::sort (
        loading.begin (), loading.end (),
        [] (auto const& a, auto const& b) {
          return a.second.bitmap.min_side () > b.second.bitmap.min_side ();
        }
      );

      for (auto& load : loading)
        pack_glyph (load.first, std::move (load.second));
    }

    TextRun bake (StrRef str) override {
      auto* const buf = hb_buffer_create ();
      auto buf_guard = Rk::guard ([buf] { hb_buffer_destroy (buf); });

      { auto const n_units = str.length ();
        hb_buffer_add_utf8 (buf, str.data (), n_units, 0, n_units);
      }
      hb_buffer_guess_segment_properties (buf);
      hb_shape (hb, buf, nullptr, 0);

      uint const n_glyphs = hb_buffer_get_length (buf);
      TextRun run (glname, n_glyphs);

      { auto const* info = hb_buffer_get_glyph_infos     (buf, nullptr);
        auto const* mets = hb_buffer_get_glyph_positions (buf, nullptr);
        v2i pen {0,0};

        for (uint i = 0; i != n_glyphs; i++) {
          uint const index = info[i].codepoint; // actually an index

          auto const& m = mets[i];
          v2i const
            offs {m.x_offset,  m.y_offset},
            advs {m.x_advance, m.y_advance};

          if (index != 0) {
            LoadedGlyph const g = get_glyph (index);

            if (g.rect) {
              v2i const pos = ((pen + offs)>>6) + g.bearings;
              int const
                w = g.rect.width,
                h = g.rect.height;

              UIRect const baked (
                { pos+v2i{0,0}, g.rect.tcs[0] },
                { pos+v2i{w,0}, g.rect.tcs[1] },
                { pos+v2i{w,h}, g.rect.tcs[2] },
                { pos+v2i{0,h}, g.rect.tcs[3] }
              );

              run.add (baked);
            }
          }

          pen += advs;
        }
      }

      return run;
    }
  };

  class FontLoaderImpl final : public FontLoader {
    FT_Library ft;

  public:
    FontLoaderImpl () {
      if (FT_Init_FreeType (&ft))
        throw std::runtime_error ("FT_Init_FreeType failed");
    }

    ~FontLoaderImpl () {
      FT_Done_FreeType (ft);
    }

    Font::Shared load (StrRef path, int size_px) override {
      FT_Face face;
      { auto const path_str = to_string (path);
        if (FT_New_Face (ft, path_str.c_str (), 0, &face))
          throw std::runtime_error ("FT_New_Face failed");
      }
      auto face_guard = Rk::guard ([face] { FT_Done_Face (face); });

      if (FT_Set_Pixel_Sizes (face, size_px, size_px))
        throw std::runtime_error ("FT_Set_Pixel_Sizes failed");

      hb_font_t* const hb_font = hb_ft_font_create_referenced (face);
      if (!hb_font)
        throw std::runtime_error ("hb_ft_font_create_referenced failed");

      face_guard.relieve ();
      return std::make_shared<FontImpl> (face, hb_font);
    }
  };

  FontLoader::Shared make_font_loader () {
    return std::make_shared<FontLoaderImpl> ();
  }
}

