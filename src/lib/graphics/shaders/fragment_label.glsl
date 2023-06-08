// Copyright © 2020 Luigi Rapetta. All Rights Reserved.
// Thanks to www.learnopengl.com for the initial structure!

#version 330 core
out vec4 FragColor;
in vec2 TexCoord;

uniform vec4 u_color;
uniform sampler2D u_texture;

void main() {
    vec4 final = u_color;
    final.w *= texture(u_texture, TexCoord).r;
    FragColor = final;
}
