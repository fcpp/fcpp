// Copyright © 2020 Luigi Rapetta. All Rights Reserved.
// Thanks to www.learnopengl.com for the initial structure!

#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;

out vec3 FragPos;
out vec3 Normal;
out vec3 ViewLightPos;

uniform mat3 u_normal;
uniform mat4 u_model;
uniform mat4 u_view;
uniform mat4 u_projection;
uniform vec3 u_lightPos;

void main()
{
    gl_Position = u_projection * u_view * u_model * vec4(aPos, 1.0);
    FragPos = vec3(u_view * u_model * vec4(aPos, 1.0));
    Normal = u_normal * aNormal;
    ViewLightPos = vec3(u_view * vec4(u_lightPos, 1.0));
}
