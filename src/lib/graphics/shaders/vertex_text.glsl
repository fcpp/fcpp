// Copyright © 2026 Giorgio Audrito and Luigi Rapetta. All Rights Reserved.
// Thanks to www.learnopengl.com for the initial structure!

#version 330 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTexCoord;

out vec2 TexCoord;

uniform mat4 u_projection;

void main() {
    gl_Position = u_projection * vec4(aPos, 1.0);
    TexCoord = aTexCoord;
}
