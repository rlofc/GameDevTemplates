#version 300 es
precision lowp float; 

struct t_directional_light
{
    vec3 color;
    float ambient_intensity;
    float diffuse_intensity;
    vec3 direction;
}; 

uniform sampler2D tex_diffuse;
uniform sampler2D tex_normal;
uniform sampler2D tex_specular;

uniform vec3 uv3_eyepos;

in vec2 vv2_texcoord;
in vec3 vv3_worldpos;
in vec3 vv3_normal; 
in vec3 vv3_tangent; 

out vec4 ov4_frag_color;

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

void main() { 
    vec3 normal = calc_bumped_normal();
    t_directional_light dir_light;
    dir_light.color = vec3(1.0,1.0,1.0);
    dir_light.ambient_intensity = 0.2;
    dir_light.diffuse_intensity = 0.8;
    dir_light.direction = vec3(-0.7,0.5,0.9);
    vec4 ambient_color = vec4(dir_light.color * 
            dir_light.ambient_intensity, 1.0);
    float diffuse_factor = dot(normalize(normal), 
            -dir_light.direction);
    vec4 specular_color = vec4(0, 0, 0, 0);

    vec4 diffuse_color;
    if (diffuse_factor > 0.0) {
        diffuse_color = vec4(dir_light.color * 
                dir_light.diffuse_intensity * diffuse_factor, 1.0);

        vec3 vertex_to_eye = normalize(uv3_eyepos - vv3_worldpos);
        vec3 light_reflect = normalize(reflect(dir_light.direction, 
                    normal));
        float specular_factor = dot(vertex_to_eye, light_reflect);
        if (specular_factor > 0.0) {
            specular_factor = pow(specular_factor, 1.0);
            specular_color = vec4(dir_light.color * 1.0 * specular_factor, 
                    1.0) * texture2D(tex_specular, vv2_texcoord);
        }
    }
    else {
        diffuse_color = vec4(0, 0, 0, 0);
    }
    vec4  c = texture2D(tex_diffuse, vv2_texcoord);
    ov4_frag_color = c* (ambient_color + diffuse_color + specular_color);
}
