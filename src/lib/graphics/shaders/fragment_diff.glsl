// Copyright © 2020 Luigi Rapetta. All Rights Reserved.
// Thanks to www.learnopengl.com for the initial structure!

#version 330 core
out vec4 FragColor;

in vec3 FragPos;
in vec3 Normal;
in vec3 ViewLightPos;

uniform float     u_ambientStrength;
uniform vec4      u_objectColor;
uniform vec3      u_lightColor;

void main()
{
    /* Keep in mind: lighting calculation are done in view-space in this case! */

    // Ambient lighting
    vec3 ambient = u_ambientStrength * u_lightColor;

    // Diffuse lighting
    vec3 normVec = normalize(Normal);
    vec3 lightDir = normalize(ViewLightPos - FragPos);
    float diffVal = max(dot(normVec, lightDir), 0.0);
    vec3 diffuse = diffVal * u_lightColor;

    // Final (not Phong: specular scrapped for performance)
    vec4 final = vec4(ambient + diffuse, 1.0) * u_objectColor;

    // Fragment color
    FragColor = final;
}
