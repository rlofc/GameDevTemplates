#version 300 es
layout (location = 0) in vec3 av4_position;
layout (location = 1) in vec3 av3_normal;
layout (location = 2) in vec3 av3_tangent;
layout (location = 3) in vec2 av2_texcoord;
layout (location = 4) in vec4 av4_color;
layout (location = 7) in mat4 am4_transform;

out vec3 vv3_fragpos;
out vec2 vv2_texcoord;
out vec3 vv3_normal;
out vec3 vv3_tangent;

uniform mat4 um4_mvp; 
uniform mat4 um4_otr; 

void main()
{
    mat4 mwt = am4_transform;
    gl_Position = (um4_mvp * mwt ) * vec4(av4_position,1); 
    vec4 world_pos = mwt * vec4(av4_position, 1.0f);
    vv3_fragpos = world_pos.xyz; 
    vv2_texcoord = av2_texcoord;
    mat3 normal_matrix = transpose(inverse(mat3(mwt)));
    vv3_normal = (normal_matrix * av3_normal);
    vv3_tangent = (normal_matrix * av3_tangent);
}
