#version 460 core

layout (location = 0) out vec4 FragColor;

layout (location = 0) in vec4 localPos;

layout (set = 0, binding = 1) uniform UBO
{
    vec4 colour;
} ubo;

void main()
{
    float len = length(localPos);
    if (len < 1.0f)
    {
        FragColor = ubo.colour;
    }
    else
    {
        FragColor = vec4(0, 0, 0, 0);
    }
}