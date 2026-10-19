// Copyright © 2026 Giorgio Audrito and Luigi Rapetta. All Rights Reserved.
// Thanks to www.learnopengl.com for the initial structure!

#version 330 core

in vec2 TexCoord;
out vec4 FragColor;

uniform sampler2D u_texture;
uniform vec4 u_color;

void main() {
    float alpha = texture(u_texture, TexCoord).r;
    FragColor = vec4(u_color.rgb, u_color.a * alpha);
}
