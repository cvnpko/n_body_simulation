#version 330 core
out vec4 FragColor;

void main()
{
    vec2 coord = gl_PointCoord * 2.0 - 1.0;
    float dist = length(coord);

    if (dist <= 1.0)
        FragColor = vec4(0.0, 0.0, 0.0, 1.0);
    else
        discard;
}