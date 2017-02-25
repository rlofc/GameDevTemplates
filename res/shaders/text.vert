#version 300 es
layout(location = 0) in vec3 av4_position;
layout(location = 1) in vec2 av2_texcoord;

uniform mat4 mvp;
uniform mat4 otr;

out vec2 vv2_fragtexcoord;

void main(void)
{
    vv2_fragtexcoord = av2_texcoord;
    mat4 otrx = otr;
    gl_Position = (mvp * otrx) * vec4(av4_position, 1);
}
