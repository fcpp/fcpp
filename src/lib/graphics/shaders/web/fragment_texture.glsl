#version 300 es
precision highp float;
precision highp int;
out vec4 FragColor;

in vec2 TexCoord;

uniform bool u_drawTexture;
uniform vec4 u_color;
uniform sampler2D u_texture;

void main() {
    vec4 final = u_color;
    if (u_drawTexture) final *= texture(u_texture, TexCoord);
        
    FragColor = final;
}
// version 3.0 for WebGL -> Shader version has to be at first line, keeping comments at end of file