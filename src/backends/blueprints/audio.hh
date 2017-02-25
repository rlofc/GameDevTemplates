#ifndef SRC_BLUEPRINTS_AUDIO_HH_INCLUDED
#define SRC_BLUEPRINTS_AUDIO_HH_INCLUDED

#include <type_traits>

#include "core/math.hh"

namespace gdt::blueprints::audio {

/**
 * Concrete sound objects should implement this interface.
 */
class sound {
  public:
    virtual void play() const = 0;
    virtual void stop() const = 0;
    virtual bool is_playing() const = 0;
    virtual void volume(float vol) = 0;
    virtual void position(math::vec3 pos) = 0;
};

/**
 * Audio backends should derive from this template
 * and provide their own sound implementation for
 * users to consume through application::sound.
 */
template <typename SOUND>
class backend {
  public:
    static_assert(std::is_base_of<gdt::blueprints::audio::sound, SOUND>::value,
                  "audio_backend sounds should derive from gdt::sound");
    using sound = SOUND;
};

}

namespace gdt {
using no_audio = gdt::blueprints::audio::backend<gdt::blueprints::audio::sound>;
}
#endif  // src/blueprints/audio_hh_INCLUDED
