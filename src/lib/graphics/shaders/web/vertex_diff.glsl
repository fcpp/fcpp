#version 300 es
precision highp float;
precision highp int;
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
// version 3.0 for WebGL -> Shader version has to be at first line, keeping comments at end of file