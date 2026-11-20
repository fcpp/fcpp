#version 300 es
precision highp float;
precision highp int;
out vec4 FragColor;

uniform vec4 u_color;

void main()
{
    FragColor = u_color;
}
// version 3.0 for WebGL -> Shader version has to be at first line, keeping comments at end of file 