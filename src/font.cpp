//
// no-dice
//

#include "font.hpp"

#include "bin-pack.hpp"

#include <Rk/clamp.hpp>
#include <Rk/guard.hpp>

#include <iostream>
#include <fstream>
#include <map>
#include <set>

#include <ft2build.h>
#include FT_FREETYPE_H

#include <hb.h>
#include <hb-ft.h>

#include <epoxy/gl.h>

namespace nd {
  enum class FontMode { fixed, scalable };

  namespace {
    struct AtlasRect {
      int width, height;
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

  /*Bitmap bitmap_from_ft_mono (FT_Bitmap const& src) {
      int const dw = src.width, dh = src.rows;
      std::vector<u8> dest (dw*dh);

      for (int dy = 0; dy != dh; dy++) {
        for (int dx = 0; dx != dw; dx++) {
          int const
            sxo = dx >> 3,
            sxs = dx & 7,
            sy = dy,
            dr = dh-dy-1;
          u8 const s = src.buffer[sy*src.pitch + sxo];
          dest[dr*dw + dx] = ((s << sxs) & 0x80)? 0xff : 0x00;
        }
      }

      return { std::move (dest), dw, dh };
    }*/

    Bitmap bitmap_from_ft_grey (FT_Bitmap const& src) {
      int const dw = src.width, dh = src.rows;
      std::vector<u8> dest (dw*dh);

      for (int dy = 0; dy != dh; dy++) {
        for (int dx = 0; dx != dw; dx++) {
          int const sx = dx, sy = dy, dr = dh-dy-1;
          dest[dr*dw + dx] = src.buffer[sy*src.pitch + sx];
        }
      }

      return { std::move (dest), dw, dh };
    }

    Bitmap bitmap_rotate (Bitmap const& src) {
      std::vector<u8> dest (src.pixels.size ());

      int const dw = src.height, dh = src.width;

      for (int dy = 0; dy != dh; dy++) {
        for (int dx = 0; dx != dw; dx++) {
          int const sx = dy, sy = dw-dx-1;
          dest[dy*dw + dx] = src.pixels[sy*src.width + sx];
        }
      }

      return { std::move (dest), dw, dh };
    }

    Bitmap bitmap_expand (Bitmap const& src, int r, u8 fill = 0x00) {
      int const
        dw = src.width  + 2*r,
        dh = src.height + 2*r;
      std::vector<u8> dest (dw*dh, fill);

      for (int sy = 0; sy != src.height; sy++) {
        for (int sx = 0; sx != src.width; sx++) {
          int const
            dx = sx + r,
            dy = sy + r;
          dest[dy*dw + dx] = src.pixels[sy*src.width + sx];
        }
      }

      return { std::move (dest), dw, dh };
    }

    Bitmap bitmap_distfield (Bitmap src, int r) {
      src = bitmap_expand (std::move (src), r);

      int const dw = src.width, dh = src.height;
      std::vector<u8> df (dw*dh);

      for (int dy = 0; dy != dh; dy++) {
        for (int dx = 0; dx != dw; dx++) {
          bool const inside = src.pixels[dy*src.width + dx] > 0x7f;
          float min_dist = 100000.f;
          for (int
            sy  = std::max (0,          dy-r);
            sy != std::min (src.height, dy+r+1);
            sy++)
          {
            for (int
              sx  = std::max (0,         dx-r);
              sx != std::min (src.width, dx+r+1);
              sx++)
            {
              bool const other_inside = src.pixels[sy*src.width + sx] > 0x7f;
              if (inside == other_inside)
                continue;

              float const dist = abs (v2f {float(sx-dx), float(sy-dy)});
              min_dist = std::min (min_dist, dist);
            }
          }

          float const
            k = 1.f / std::sqrt (2*r*r),
            out = 0.5f + 0.5f*k*(inside? (min_dist-1.f) : -min_dist);
          df [dy*dw + dx] = 255.f*Rk::clamp (out, 0.f, 1.f);
        }
      }

      return { std::move (df), dw, dh };
    }

    Bitmap bitmap_reduce (Bitmap const& src, int k) {
      int const
        dw = src.width  / k,
        dh = src.height / k;
      std::vector<u8> dest (dw*dh);

      for (int dy = 0; dy != dh; dy++) {
        for (int dx = 0; dx != dw; dx++) {
          u32 acc = 0;
          for (int sy = dy*k; sy != (dy+1)*k; sy++) {
            for (int sx = dx*k; sx != (dx+1)*k; sx++)
              acc += src.pixels[sy*src.width + sx];
          }
          dest[dy*dw + dx] = acc/(k*k);
        }
      }

      return { std::move (dest), dw, dh };
    }
  }

  class FontImpl final : public Font {
    FT_Face face;
    hb_font_t* hb;
    FontMode mode;
    int scale;
    Rk::RectPacker packer;
    uint glname;
    std::map<uint, LoadedGlyph> loaded;

    LoadingGlyph load_glyph (uint const index) {
      // load and render
      auto const flags = (mode == FontMode::scalable)
        ? FT_LOAD_TARGET_LIGHT//FT_LOAD_TARGET_MONO
        : FT_LOAD_TARGET_NORMAL;
      if (FT_Load_Glyph (face, index, flags | FT_LOAD_RENDER))
        throw std::runtime_error ("FT_Load_Glyph failed");

      auto const& src = face->glyph->bitmap;

      Bitmap bmp;
      int r;
      if (mode == FontMode::scalable) {
        r = 15;
      //bmp = bitmap_from_ft_mono (src);
        bmp = bitmap_from_ft_grey (src);
        bmp = bitmap_distfield (std::move (bmp), r);
        bmp = bitmap_reduce (std::move (bmp), scale);
      }
      else {
        r = 0;
        bmp = bitmap_from_ft_grey (src);
      }

      v2i const bearings {
        face->glyph->bitmap_left/scale - r,
        face->glyph->bitmap_top /scale - bmp.height + r
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
      v2i const dims = v2i {
        g.bitmap.width  + alignment () - 1,
        g.bitmap.height + alignment () - 1
      } / alignment ();
      auto const [ok, rotated, where_scaled] = packer.add (dims);
      auto const where = Rk::Rect::with_corners (
        where_scaled.mins () * alignment (),
        where_scaled.maxs () * alignment ()
      );
      if (!ok) {
        std::cerr << "Failed to pack glyph\n";
        return get_glyph (0); // FIXME: is this u+fffd?
      }

      if (where) { // zero-area glyphs don't even occupy the atlas
        // rotate glyph if necessary
        if (rotated)
          g.bitmap = bitmap_rotate (std::move (g.bitmap));

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

    enum {
      w = 4096,
      h = w,
      n_levels = 4
    };

    int alignment () const {
      if (mode == FontMode::scalable) return 1 << (n_levels - 1);
      else return 1;
    }

  public:
    FontImpl (FT_Face face, hb_font_t* hb, FontMode mode, int scale) :
      face (face), hb (hb), mode (mode), scale (scale),
      packer (v2i { w / alignment (), h / alignment () })
    {
      glCreateTextures (GL_TEXTURE_2D, 1, &glname);
      int nl = (mode==FontMode::scalable)? n_levels : 1;
      glTextureStorage2D (glname, nl, GL_R8, w, h);
      glClearTexImage (glname, 0, GL_RED, GL_UNSIGNED_BYTE, nullptr);//"\x7f");

      v2i constexpr const attribs[] {
        { GL_TEXTURE_MIN_FILTER, GL_NEAREST },
        { GL_TEXTURE_MAG_FILTER, GL_NEAREST },
        { GL_TEXTURE_WRAP_S,     GL_CLAMP_TO_EDGE },
        { GL_TEXTURE_WRAP_T,     GL_CLAMP_TO_EDGE }
      };
      for (v2i attr : attribs)
        glTextureParameteri (glname, attr.x, attr.y);

      get_glyph (cp_to_glyph (0xfffd));
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

    void fix () override {
      v2i constexpr const attribs[] {
        { GL_TEXTURE_MAX_LEVEL,  n_levels - 1 },
        { GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR },
        { GL_TEXTURE_MAG_FILTER, GL_LINEAR }
      };
      for (v2i attr : attribs)
        glTextureParameteri (glname, attr.x, attr.y);
      glGenerateTextureMipmap (glname);
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
            offs = v2i {m.x_offset,  m.y_offset } / scale,
            advs = v2i {m.x_advance, m.y_advance} / scale;

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

    void dump (StrRef path) override {
      std::vector<u8> pixels (w*h);
      glPixelStorei (GL_PACK_ALIGNMENT, 1);
      glGetTextureImage (
        glname, 0,
        GL_RED, GL_UNSIGNED_BYTE, pixels.size (), pixels.data ());
      std::ofstream f (to_string (path));
      f << "P5\n" << w << " " << h << "\n255\n";
      f.write ((char const*) pixels.data (), pixels.size ());
    }
  };

  class FontLoaderImpl final : public FontLoader {
    FT_Library ft;

    Shared<Font> load_impl (StrRef path, int px, int reduce, FontMode mode) {
      FT_Face face;
      { auto const path_str = to_string (path);
        if (FT_New_Face (ft, path_str.c_str (), 0, &face))
          throw std::runtime_error ("FT_New_Face failed");
      }
      auto face_guard = Rk::guard ([face] { FT_Done_Face (face); });

      if (FT_Set_Pixel_Sizes (face, px, px))
        throw std::runtime_error ("FT_Set_Pixel_Sizes failed");

      hb_font_t* const hb_font = hb_ft_font_create_referenced (face);
      if (!hb_font)
        throw std::runtime_error ("hb_ft_font_create_referenced failed");

      face_guard.relieve ();
      return std::make_shared<FontImpl> (face, hb_font, mode, reduce);
    }

  public:
    FontLoaderImpl () {
      if (FT_Init_FreeType (&ft))
        throw std::runtime_error ("FT_Init_FreeType failed");
    }

    ~FontLoaderImpl () {
      FT_Done_FreeType (ft);
    }

    Shared<Font> load (StrRef path, int size_px) override {
      return load_impl (path, size_px, 1, FontMode::fixed);
    }

    Shared<Font> load_scalable (StrRef path) override {
      return load_impl (path, 256, 4, FontMode::scalable);
    }
  };

  Shared<FontLoader> make_font_loader () {
    return std::make_shared<FontLoaderImpl> ();
  }
}

