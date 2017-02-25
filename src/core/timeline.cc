#include "timeline.hh"
namespace gdt {

timeline::timeline(bool paused)
{
    _current_span = _spans.end();
    _paused = paused;
}

timeline::~timeline() 
{
}

timeline& timeline::wait(float t)
{
    return span(t, [](float elapsed, float progress) {});
}

timeline& timeline::span(float t, timeline::span_callback_t f)
{
    _spans.push_back({t, t, f});
    _current_span = _spans.begin();
    return *this;
}

timeline& timeline::once(timeline::span_callback_t f)
{
    _spans.push_back({0, 0, f});
    _current_span = _spans.begin();
    return *this;
}

void timeline::update(const gdt::core_context& ctx)
{
    if (_paused) return;
    float c = 0;
    if (_current_span != _spans.end()) {
        c = _current_span->time;
        c = c - ctx.elapsed;
        if (c <= 0.0) {
            _current_span->span_callback(ctx.elapsed, 1.0);
            _current_span->time = 0.0;
            _current_span++;
            c = _current_span->time + c;
        }
    }
    if (_current_span != _spans.end()) {
        _current_span->time = c;
        _current_span->span_callback(ctx.elapsed,
                                     1.0 - _current_span->time / _current_span->otime);
    }
}

bool timeline::is_done() const {
    return _current_span == _spans.end();
}
}
