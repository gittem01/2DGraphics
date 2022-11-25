#version 460 core


struct triangle
{
    vec2 p1;
    vec2 p2;
    vec2 p3;
};

layout (location = 0) out vec2 localPos;

layout (set = 0, binding = 0) uniform UBO
{
    mat4 ortho;
    vec4 polyPoints[4];
    int numPoints;
} ubo;

layout (set = 0, binding = 1) uniform SharedUBO
{
    vec4 colour;
    vec4 data; // zoom, thickness, unused, unused
} sharedData;

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
    gl_Position = ubo.ortho * vec4(v, 0.0, 1.0);
}