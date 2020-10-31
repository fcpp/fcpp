// Copyright © 2020 Luigi Rapetta. All Rights Reserved.

#version 330 core
out vec4 FragColor;

in vec3 FragPos;
in vec3 Normal;
in vec3 ViewLightPos;

uniform float     u_ambientStrength;
uniform float     u_specularStrength;
uniform int       u_specularShininess;
uniform vec3      u_objectColor;
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

    // Specular lighting
    vec3 reflectDir = reflect(-lightDir, normVec);
    vec3 viewDir = normalize(-FragPos);
    float specVal = pow(max(dot(viewDir, reflectDir), 0.0), u_specularShininess);
    vec3 specular = u_specularStrength * specVal * u_lightColor;

    // Phong (final)
    vec3 phong = (ambient + diffuse + specular) * u_objectColor;

    // Fragment color
    FragColor = vec4(phong, 1.0);
}