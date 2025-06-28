#version 330 core
in float vPointSize;
in float vAlpha;
out vec4 FragColor;

void main()
{
    vec2 coord = gl_PointCoord * 2.0 - 1.0;
    float dist = length(coord);

    if (dist <= 1.0)
        FragColor = vec4(vAlpha, 1.0-vAlpha,(1.0-vAlpha)/2.0, vAlpha);
    else
        discard;
}