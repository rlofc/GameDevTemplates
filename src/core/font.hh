#ifndef SRC_CONSTRUCTS_FONT_HH_INCLUDED
#define SRC_CONSTRUCTS_FONT_HH_INCLUDED

#include <fstream>
#include <map>

#include "shaders.hh"

namespace gdt {

/**
* Font holds resource texture and glyph information needed to create and draw
* text objects on screen.
* You would usually use the same font resource instance for a set of text objects
* and you would usually have a font instance created once for your application or
* scene lifespan.
* Font resources provide a text_pipeline material instance for the actual
* drawing operation of the text shader.
*/
template <typename GRAPHICS>
class font {
  public:
    struct glyph_data {
        gdt::math::vec2 pos;      // Position in texture
        gdt::math::vec2 dim;      // Glyph's width (x) and height (y)
        gdt::math::vec2 offset;   // Glyph's offset values for x and y
        gdt::math::vec2 orig_dim; // Glyph's full dimentions (x and y)
    };

  public:
    /**
    * Construct a new font resource.
    *
    * @param ctx Context instance implementing graphics_context
    * @param resource_file Prefix SDF font resource name (for both PNG and FNT files)
    */
    font(const graphics_context<GRAPHICS>& ctx, std::string resource_file = "res/fonts/sdf");

    /**
    * Returns the glyph descriptor of a character.
    */
    const glyph_data& get_glyph(int c) const;

    /**
    * Given two characters, find the kerning value to adjuest the
    * spacing between the two.
    */
    float get_kerning(int c1, int c2) const;

    // Simple getters
    float get_atlas_width() const;
    float get_atlas_height() const;
    const typename GRAPHICS::texture& atlas() const;
    const typename text_pipeline<GRAPHICS>::material& material() const;

  private:
    typename GRAPHICS::texture _atlas;
    typename text_pipeline<GRAPHICS>::material _material;
    std::map<int, glyph_data> _glyphs;
    std::map<int, std::map<int, float>> _kerning_pairs;

};

template <typename GRAPHICS>
font<GRAPHICS>::font(const graphics_context<GRAPHICS>& ctx, std::string resource_file):
  _atlas(ctx, resource_file+".png"),
  _material{&_atlas}
{
    std::ifstream f(resource_file+".fnt", std::ios::in);
    if (!f) throw std::runtime_error("Unable to open font resource file");
    uint32_t c, x, y, w, h, xoff, yoff, ow, oh;
    uint32_t n_glyphs, n_kerning_pairs;
    f >> n_glyphs;
    while (f.eof() == false && n_glyphs-- > 0) {
        f >> c >> x >> y >> w >> h >> xoff >> yoff >> ow >> oh;
        glyph_data g;
        g.pos.x = x + 12;
        g.pos.y = y + 12;
        g.offset.x = xoff;
        g.offset.y = yoff;
        g.dim.x = w;
        g.dim.y = h;
        g.orig_dim.x = ow;
        g.orig_dim.y = oh;
        _glyphs[c] = g;
    }
    if (f.eof()) {
        LOG_WARNING << "Font " << resource_file << " does not have kerning pairs.";
        return;
    }
    f >> n_kerning_pairs;
    uint32_t c1, c2;
    float k;
    while (f.eof() == false && n_kerning_pairs-- > 0) {
        f >> c1 >> c2 >> k;
        _kerning_pairs[c1][c2] = k;
    }
}

/**
* FONT TEMPLATE IMPLEMENTATION
*/

template <typename GRAPHICS>
const typename font<GRAPHICS>::glyph_data&
font<GRAPHICS>::get_glyph(int c) const
{
    return _glyphs.at(c);
}

template <typename GRAPHICS>
float
font<GRAPHICS>::get_atlas_width() const
{
    return _atlas.width;
}

template <typename GRAPHICS>
float
font<GRAPHICS>::get_atlas_height() const
{
    return _atlas.height;
}

template <typename GRAPHICS>
float
font<GRAPHICS>::get_kerning(int c1, int c2) const
{
    float ret = 0;
    if (_kerning_pairs.find(c1) != _kerning_pairs.end()) {
        if (_kerning_pairs.at(c1).find(c2) != _kerning_pairs.at(c1).end()) {
            ret = _kerning_pairs.at(c1).at(c2);
        }
    }
    return ret;
}

template <typename GRAPHICS>
const typename GRAPHICS::texture&
font<GRAPHICS>::atlas() const
{
    return _atlas;
}

template <typename GRAPHICS>
const typename text_pipeline<GRAPHICS>::material&
font<GRAPHICS>::material() const
{
    return _material;
}

}
#endif  // src/core/text_hh_INCLUDED
