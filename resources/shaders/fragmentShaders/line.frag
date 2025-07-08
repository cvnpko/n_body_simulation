#version 330 core

in float vColorIndex;
out vec4 FragColor;

vec3 getColor(float index) {
    if (index == 1.0) return vec3(1.0, 0.0, 0.0);
    if (index == 2.0) return vec3(0.0, 1.0, 0.0);
    if (index == 3.0) return vec3(0.0, 0.0, 1.0);
    return vec3(1.0);
}

void main()
{
    FragColor = vec4(getColor(vColorIndex), 1.0);
}