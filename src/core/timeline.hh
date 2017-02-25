#ifndef SRC_CONSTRUCTS_TIMELINE_HH_INCLUDED
#define SRC_CONSTRUCTS_TIMELINE_HH_INCLUDED

#include <functional>
#include <vector>

#include "context.hh"

namespace gdt {

/**
 * timeline
 *
 */
class timeline {
  public:
    using span_callback_t = std::function<void(float, float)>;

    timeline(bool paused = false);
    timeline& operator=(const timeline&) = delete;
    timeline(const timeline&) = delete;
    virtual ~timeline();

    timeline& wait(float t);
    timeline& span(float t, span_callback_t f);
    timeline& once(span_callback_t f);
    void update(const gdt::core_context& ctx);
    void resume() { _paused = false; }
    bool is_done() const;

  private:
    struct stored_span {
        float otime;
        float time;
        span_callback_t span_callback;
    };
    std::vector<stored_span> _spans;
    std::vector<stored_span>::iterator _current_span;
    bool _paused = false;
};
}
#endif  // SRC_CONSTRUCTS_TIMELINE_HH_INCLUDED
