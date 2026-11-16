#version 300 es
precision highp float;
precision highp int;

in vec2 TexCoord;
out vec4 FragColor;

uniform sampler2D u_texture;
uniform vec4 u_color;

void main() {
    float alpha = texture(u_texture, TexCoord).r;
    FragColor = vec4(u_color.rgb, u_color.a * alpha);
}
// version 3.0 for WebGL -> Shader version has to be at first line, keeping comments at end of file