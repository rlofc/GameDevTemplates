#version 300 es
layout (location = 0) in vec3 av3_position;
layout (location = 1) in vec2 av2_texcoords;

out vec2 vv2_texcoord;

void main()
{
    gl_Position = vec4(av3_position, 1.0f);
    vv2_texcoord = av2_texcoords;
}
