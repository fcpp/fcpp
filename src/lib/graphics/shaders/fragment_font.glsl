// Copyright © 2020 Luigi Rapetta. All Rights Reserved.
// Thanks to www.learnopengl.com for the initial structure!

#version 330 core
in vec2 TexCoords;

out vec4 color;

uniform sampler2D u_text;
uniform vec3 u_textColor;

void main()
{    
    vec4 sampled = vec4(1.0, 1.0, 1.0, texture(u_text, TexCoords).r);
    color = vec4(u_textColor, 1.0) * sampled;
}
