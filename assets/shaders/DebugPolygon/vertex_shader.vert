#version 460 core

layout (location = 0) out vec2 localPos;
layout (location = 1) out vec2 worldPosition;

layout (set = 0, binding = 0) uniform SharedUboData
{
    mat4 orthoMatrix;
} sharedUboData;

layout (set = 0, binding = 1) uniform UBO
{
    mat4 modelMatrix;
    vec4 polyPoints[4];
    int numPoints;
} ubo;

void main()
{
    int pos;
    int id = gl_VertexIndex;
    int md = id % 3;
    switch (md)
    {
        case 0:
            pos = 0;
            break;
        case 1:
            pos = id / 3 + 1;
            break;
        case 2:
            pos = id / 3 + 2;
            break;
        default:
            break;
    }

    vec2 v = vec2(ubo.polyPoints[pos / 2][(pos % 2) * 2], ubo.polyPoints[pos / 2][(pos % 2) * 2 + 1]);

    localPos = v;
    vec4 wp = ubo.modelMatrix * vec4(v, 0.0, 1.0f);
    worldPosition = wp.xy;

    gl_Position = sharedUboData.orthoMatrix * wp;
}