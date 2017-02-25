#version 300 es
layout (location = 0) in vec3 av4_position;
layout (location = 1) in vec3 av3_normal;
layout (location = 2) in vec3 av3_tangent;
layout (location = 3) in vec2 av2_texcoord;
layout (location = 4) in vec4 av4_color;
layout (location = 5) in vec3 av3_bindices;
layout (location = 6) in vec3 av3_bweights;
layout (location = 7) in mat4 av4_transform;

uniform vec4 uv4_quat_reals[64];
uniform vec4 uv4_quat_duals[64];

uniform mat4 um4_mvp; 

out vec3 vv3_fragpos;
out vec2 vv2_texcoord;
out vec3 vv3_normal;
out vec3 vv3_tangent;

vec3 quat_dual_mul_pos(vec4 real, vec4 dual, vec3 v) {
    return v + 2.0 * cross(real.xyz, cross(real.xyz, v) + real.w*v) +
        2.0 * (real.w * dual.xyz - dual.w * real.xyz + cross(real.xyz, dual.xyz));
}

vec3 quat_dual_mul_rot(vec4 real, vec4 dual, vec3 v) {
    return v + 2.0 * cross(real.xyz, cross(real.xyz, v) + real.w*v);
}

void main()
{
    vec4 real = uv4_quat_reals[int(av3_bindices.x)] * av3_bweights.x +
        uv4_quat_reals[int(av3_bindices.y)] * av3_bweights.y +
        uv4_quat_reals[int(av3_bindices.z)] * av3_bweights.z;

    vec4 dual = uv4_quat_duals[int(av3_bindices.x)] * av3_bweights.x +
        uv4_quat_duals[int(av3_bindices.y)] * av3_bweights.y +
        uv4_quat_duals[int(av3_bindices.z)] * av3_bweights.z;

    dual = dual / length(real);
    real = real / length(real);

    vec3 blendpos    = quat_dual_mul_pos(real, dual, av4_position);
    vec3 blendnorm   = quat_dual_mul_rot(real, dual, av3_normal);
    vec3 blendtang   = quat_dual_mul_rot(real, dual, av3_tangent);

    mat4 mwt = av4_transform;

    blendnorm   = mat3(mwt) * blendnorm;
    blendtang   = mat3(mwt) * blendtang;

    vec4 world_pos = mwt * vec4(blendpos, 1.0);
    gl_Position = (um4_mvp * world_pos);
    vv3_fragpos = world_pos.xyz; 
    vv2_texcoord = av2_texcoord;
    vv3_normal = blendnorm.xyz;
    vv3_tangent = blendtang.xyz;
}
