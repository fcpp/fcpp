// Copyright © 2020 Luigi Rapetta. All Rights Reserved.
// Thanks to www.learnopengl.com for the initial structure!

#version 330 core
layout (location = 0) in vec4 vertex; // <vec2 pos, vec2 tex>

out vec2 TexCoords;

uniform mat4 u_projection;

void main()
{
    gl_Position = u_projection * vec4(vertex.xy, 0.0, 1.0);
    TexCoords = vertex.zw;
}  
