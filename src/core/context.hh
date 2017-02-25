#ifndef GDT_CONTEXT_HEADER_INCLUDED
#define GDT_CONTEXT_HEADER_INCLUDED

#include <functional>

#include <chrono>
#include <map>
#include "imgui/imgui.h"

// Context is a single object managed by the application and designed to flow
// through the call stack to any function. Its purpose is to provide access to
// core application values and backends.
// The context is split to different aspects by means of (careful) multiple
// inheritance. This gives different backends the ability to use only their
// part, without introducing inter-aspect dependencies.
namespace gdt {

/**
 * gdt::context is also composed from this core_context. The core context
 * provides the frame elapsed time, useful in many time-based game state
 * updates.
 *
 * You can directly access `elapsed` through your game's specified context
 * type.
 */
struct core_context {
    float elapsed;
    std::function<void()> quit;

    struct measurement {
        mutable long long _b;
        mutable long long _total;
        mutable int avg = 0;
        mutable long long _count = 0;
        void begin() const {
                _b = std::chrono::high_resolution_clock::now().time_since_epoch().count();
        }
        void end() const {
           _count++;
           int d = std::chrono::high_resolution_clock::now().time_since_epoch().count() - _b;
           _total += d;
           avg = _total / _count;
        }
    };

    mutable std::map<std::string, measurement> _m;

    const measurement & measure(std::string what) const {
        return _m[what];
    }

    void imgui() const {
        ImGui::Separator();
        float total = 0;
        for (auto & m : _m) {
            float avg = m.second.avg;
            avg /= 1000000.0f;
            ImGui::Text("%s: %fms", m.first.c_str(), avg );
            total += avg;
        }
        ImGui::Text("TOTAL: %fms", total );
        ImGui::Separator();
    }
};

/**
 * The platform_context is a component of the default gdt::context template.
 * You will normally not be using this template in your game code.
 */
template <typename PLATFORM>
struct platform_context {
    PLATFORM* p;
    const PLATFORM* get_platform() const
    {
        return p;
    }
};

/**
 * The graphics_context is a component of the default gdt::context template.
 * You will normally not be using this template in your game code.
 */
template <typename GRAPHICS>
struct graphics_context {
    GRAPHICS* graphics;
    const GRAPHICS* get_graphics() const
    {
        return graphics;
    }
};

/**
 * The physics_context is a component of the default gdt::context template.
 * You will normally not be using this template in your game code.
 */
template <typename PHYSICS>
struct physics_context {
    mutable PHYSICS* physics;
};

/**
 * The audio_context is a component of the default gdt::context template.
 * You will normally not be using this template in your game code.
 */
template <typename AUDIO>
struct audio_context {
    mutable AUDIO* audio;
};


/**
 * When you start your application or game, GDT will create your main backend
 * objects as part of you application instance. These objects are later required
 * to create different game assets. Instead of having a set of singletons,
 * GDT uses dependency injection and asks you to provide a special packaged
 * object called gdt::context to many different constructors.
 *
 * You can subclass the generic context to create your own 
 * customized context with additional state (for example, a scripting engine 
 * of your choice):
 *
 *     template <typename PLATFORM, typename GRAPHICS, typename AUDIO, typename PHYSICS>
 *     struct my_context : gdt::context<PLATFORM, GRAPHICS, AUDIO, PHYSICS> {
 *         // additional state goes here
 *     };
 *
 * You will have to specify either your subclassed context type or the default
 * one when you create your application type:
 *
 *     using my_game = gdt::application< ... , my_context>;
 *
 * In any case, always use the context type provided by your specified application
 * type:
 *
 *     using my_game = gdt::application<..., my_context>;
 *
 *     void any_method_accepting_context(const my_game::context & ctx) {
 *         ....
 *     }
 *
 */
template <typename PLATFORM, typename GRAPHICS, typename AUDIO, typename PHYSICS>
struct context : platform_context<PLATFORM>,
                 graphics_context<GRAPHICS>,
                 audio_context<AUDIO>,
                 physics_context<PHYSICS>,
                 core_context {
};
};

#endif  // GDT_CONTEXT_HEADER_INCLUDED
