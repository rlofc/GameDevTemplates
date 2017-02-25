#version 300 es
#ifdef GL_ES
precision mediump float;
#endif

uniform sampler2D tex;

in vec2 vv2_fragtexcoord;
out vec4 ov4_frag_color;

uniform float smoothing;
uniform float outline;
uniform vec4 color;

void main() {
    vec4 v_color = color;
    float distance = texture2D(tex, vv2_fragtexcoord).a;
    float alpha = smoothstep(0.5 - smoothing, 0.5 + smoothing, distance) * v_color.a;
    ov4_frag_color = vec4(v_color.rgb, alpha);
}
