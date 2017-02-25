#version 300 es
precision lowp float; 
//#version 330 core
layout (location = 0) out vec3 ov3_position;
layout (location = 1) out vec3 ov3_normal;
layout (location = 2) out vec4 ov4_albedospec;
layout (location = 3) out vec4 ov4_fog;

in vec3 vv3_fragpos;
in vec2 vv2_texcoord;
in vec3 vv3_normal;
in vec3 vv3_tangent;

uniform sampler2D tex_diffuse;
uniform sampler2D tex_normal;
uniform sampler2D tex_specular;
uniform float fog_density;

vec3 calc_bumped_normal()
{
    vec3 normal = normalize(vv3_normal);
    vec3 tangent = normalize(vv3_tangent);
    tangent = normalize(tangent - dot(tangent, normal) * normal);
    vec3 bitangent = cross(tangent, normal);
    vec3 bump_map_normal = texture2D(tex_normal, vv2_texcoord).xyz;
    bump_map_normal = 2.0 * bump_map_normal - vec3(1.0, 1.0, 1.0);
    vec3 new_normal;
    mat3 t_b_n = mat3(tangent, bitangent, normal);
    new_normal = t_b_n * bump_map_normal;
    new_normal = normalize(new_normal);
    return new_normal;
}

float fogFactorExp2(
        const float dist,
        const float density
        ) {
    const float LOG2 = -1.442695;
    float d = density * dist;
    return 1.0 - clamp(exp2(d * d * LOG2), 0.0, 1.0);
}

void main()
{    
    ov3_position = vv3_fragpos;
    ov3_normal = calc_bumped_normal();
    ov4_albedospec.rgb = texture(tex_diffuse, vv2_texcoord).rgb;
    ov4_albedospec.a = texture(tex_specular, vv2_texcoord).r;
    float fog_distance = gl_FragCoord.z / gl_FragCoord.w;
    float fog_amount = fogFactorExp2(fog_distance, fog_density);
    ov4_fog.a = 1.0-fog_amount;
}
