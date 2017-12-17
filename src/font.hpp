//
// no-dice
//

#pragma once

#include "vector.hpp"
#include "frame.hpp"
#include "types.hpp"

namespace nd {
  class TextRun {
    std::vector<UIRect> glyphs;
    uint texname;

  public:
    explicit TextRun (uint texname, size_t res) :
      texname (texname)
    {
      glyphs.reserve (res);
    }

    TextRun (TextRun const&) = default;
    TextRun (TextRun&&) = default;

    void add (UIRect r) {
      glyphs.push_back (r);
    }

    UIRect const* begin () const {
      return glyphs.data ();
    }

    UIRect const* end () const {
      return begin () + glyphs.size ();
    }

    size_t length () const {
      return glyphs.size ();
    }

    uint get_texture () const {
      return texname;
    }
  };

  struct CharRange {
    u32 first, last;
    struct Iter {
      u32 cp;
      u32 operator * () const { return cp; }
      bool operator == (Iter const& i) const { return cp == i.cp; }
      bool operator != (Iter const& i) const { return cp != i.cp; }
      Iter& operator ++ () { cp++; return *this; }
    };
    Iter begin () const { return { first }; }
    Iter end   () const { return { last+1 }; }
  };

  using CharRanges = std::vector<CharRange>;

  class Font {
  public:
    using Shared = std::shared_ptr<Font>;
    virtual void prime (CharRanges const&) = 0;
    virtual TextRun bake (StrRef text) = 0;
  };

  class FontLoader {
  public:
    using Shared = std::shared_ptr<FontLoader>;
    virtual Font::Shared load (StrRef path, int size_px) = 0;
  };

  FontLoader::Shared make_font_loader ();
}

