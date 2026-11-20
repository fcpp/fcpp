#version 300 es
precision highp float;
precision highp int;
layout (location = 0) in vec3 aPos;

uniform mat4 u_model;
uniform mat4 u_view;
uniform mat4 u_projection;

void main()
{
    gl_Position = u_projection * u_view * u_model * vec4(aPos, 1.0);
}
// version 3.0 for WebGL -> Shader version has to be at first line, keeping comments at end of file