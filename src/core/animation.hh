#ifndef SRC_CONSTRUCTS_ANIMATION_HH_INCLUDED
#define SRC_CONSTRUCTS_ANIMATION_HH_INCLUDED

#include <functional>
#include <map>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>
#include <cmath>

#include "math.hh"
#include "context.hh"
#include "logger.hh"

namespace gdt {

struct skeleton;

/**
 * TODO: document this
 */
struct frame {
    std::vector<int> bone_parents;
    std::vector<gdt::math::vec3> bone_positions;
    std::vector<gdt::math::quat> bone_rotations;
    std::vector<gdt::math::mat4> bone_transforms;
    std::vector<gdt::math::mat4> bone_inv_transforms;
    bool baked = false;

    gdt::math::mat4 bone_transform(int i)
    {
        gdt::math::mat4 prev;
        gdt::math::mat4 ret = gdt::math::mat4::id();
        if (this->bone_parents[i] != -1) {
            gdt::math::mat4 prev = this->bone_transform(this->bone_parents[i]);
            ret = ret * prev;
        }
        gdt::math::mat4 pos = gdt::math::mat4::translation(this->bone_positions[i]);
        gdt::math::mat4 rot = gdt::math::mat4::rotation_quat(this->bone_rotations[i]);
        ret = ret * pos;
        ret = ret * rot;
        return ret;
    }

    void bake_transforms()
    {
        int bones = bone_parents.size();
        this->bone_transforms.clear();
        this->bone_inv_transforms.clear();
        for (int i = 0; i < bones; i++) {
            gdt::math::mat4 t = this->bone_transform(i);
            this->bone_transforms.push_back(t);
            this->bone_inv_transforms.push_back(t.inverse());
        }
        this->baked = true;
    }
};

/**
 * TODO: document this
 */
struct bone {
    std::string name;
    int parent;
    bone(std::string n, int p)
    {
        name = n;
        parent = p;
    }
};

/**
 * TODO: document this
 */
struct skeleton {
    std::vector<bone> bones;
    frame rest;
    int n_bones() const
    {
        return bones.size();
    }
};

/**
 * TODO: document this
 */
class animation {
  private:
    skeleton _skeleton;
    std::vector<frame> _frames;
    mutable float animation_time = 0;
    bool _loop = true;

  public:
    animation(std::string filename, const skeleton& s, bool loop = true);

    void update(const gdt::core_context& ctx)
    {
        animation_time += ctx.elapsed;
    }
    void reset()
    {
        animation_time = 0;
    }

    static frame interpolate(const frame& f0, const frame& f1, float amount)
    {
        frame interpolated;
        interpolated.bone_parents = f0.bone_parents;
        int nbones = f0.bone_positions.size();
        for (int i = 0; i < nbones; i++) {
            interpolated.bone_positions.push_back(
                gdt::math::vec3::lerp(f0.bone_positions[i], f1.bone_positions[i], amount));
            interpolated.bone_rotations.push_back(
                gdt::math::quat::slerp(f0.bone_rotations[i], f1.bone_rotations[i], amount));
        }
        interpolated.bake_transforms();
        return interpolated;
    }

    frame current_frame() const
    {
        float frame_time = 1.0 / 24;
        if (_loop == false && animation_time > frame_time * (_frames.size() - 1))
            return _frames[_frames.size() - 1];
        float time = std::fmod(animation_time, frame_time * (_frames.size() - 1));
        float amount = std::fmod(time / frame_time, 1.0);
        const frame& f0 = _frames[time / frame_time + 0];
        const frame& f1 = _frames[time / frame_time + 1];

        return animation::interpolate(f0, f1, amount);
    }

    template <typename SHADER>
    void bind(const SHADER& s) const
    {
        frame frame = current_frame();
        gdt::math::mat4 bone_matrices[64];
        gdt::math::vec4 quat_reals[64];
        gdt::math::vec4 quat_duals[64];
        for (int j = 0; j < _skeleton.n_bones(); j++) {
            gdt::math::mat4 base = _skeleton.rest.bone_inv_transforms[j];
            gdt::math::mat4 ani = frame.bone_transforms[j];
            bone_matrices[j] = ani * base;
            quat_reals[j] = bone_matrices[j].as_quat_dual().real;
            quat_duals[j] = bone_matrices[j].as_quat_dual().dual;
        }
        s.bind_reals(quat_reals, _skeleton.n_bones());
        s.bind_duals(quat_duals, _skeleton.n_bones());
    }
};

/**
 * TODO: document this
 */
class animixer {
    skeleton _skeleton;
    struct strip {
        animation* a;
        float duration = 0;
        float elapsed = 0;
    };
    // implement stack-like behavior
    // so:
    //      [next_anim, duration = 1s]
    //      [current_anim, duration = 0]
    // after 0.5s
    //      [next_anim, duration = 0.5s]
    //      [current_anim, duration = 0]
    // after 0.5s
    //      [next_anim is now current, duration = 0]
    std::vector<strip> _strips;

  public:
    animixer(const skeleton& s);

    /**
     * transition to a new animation.
     *
     * @param a animation to transition to
     * @param fader_duration how long will the transition last
     */
    void play(animation* a, float fader_duration)
    {
        if (_strips.size() == 2) return;
        a->reset();
        _strips.push_back({a, fader_duration, 0});
    }

    const skeleton & get_skeleton() const {return _skeleton;}

    void update(const core_context& ctx)
    {
        int pop = 0;
        for (auto& s : _strips) {
            s.a->update(ctx);
            if (s.duration > 0) {
                s.elapsed += ctx.elapsed;
                if (s.elapsed > s.duration) {
                    s.duration = 0;
                    pop++;
                }
            }
        }
        while (pop > 0) {
            _strips.erase(_strips.begin());
            pop--;
        }
    }

    template <typename SHADER>
    void bind(const SHADER& s) const
    {
        if (_strips.begin() == _strips.end())
            throw std::runtime_error("no animations in animixer");
        frame frame = _strips.begin()->a->current_frame();
        for (auto i = _strips.cbegin() + 1; i != _strips.end(); i++) {
            frame =
                animation::interpolate(frame, i->a->current_frame(), i->elapsed / i->duration);
        }
        gdt::math::mat4 bone_matrices[64];
        gdt::math::vec4 quat_reals[64];
        gdt::math::vec4 quat_duals[64];
        for (int j = 0; j < _skeleton.n_bones(); j++) {
            gdt::math::mat4 base = _skeleton.rest.bone_inv_transforms[j];
            gdt::math::mat4 ani = frame.bone_transforms[j];
            bone_matrices[j] = ani * base;
            quat_reals[j] = bone_matrices[j].as_quat_dual().real;
            quat_duals[j] = bone_matrices[j].as_quat_dual().dual;
        }
        s.bind_reals(quat_reals, _skeleton.n_bones());
        s.bind_duals(quat_duals, _skeleton.n_bones());
    }
};
}

#endif  // src/core/animation_hh_INCLUDED
