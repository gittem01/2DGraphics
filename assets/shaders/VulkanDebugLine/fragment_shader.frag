#version 460 core

layout (location = 0) out vec4 color;

layout (location = 0) in vec2 localPos;
layout (location = 1) in vec2 size;

layout (set = 0, binding = 1) uniform SharedUBO
{
    vec4 colour;
    vec4 data; // zoom, thickness, outerSmoothness, unused
} sharedData;

float lineRel(vec2 a, vec2 b, vec2 c){
     return (b.x - a.x) * (c.y - a.y) - (b.y - a.y) * (c.x - a.x);
}

void main()
{
    float perctDist = 1.0f;
    vec2 p1 = vec2(size.x - size.y, 0.0f);
    vec2 p2 = vec2(-size.x + size.y, 0.0f);
    if (localPos.x < p2.x && length(localPos - p2) > size.y || localPos.x > p1.x && length(localPos - p1) > size.y){
        color = vec4(0, 0, 0.0f, 0.0f);
        return;
    }
    else
    {
        if (localPos.x < p2.x)
        {
            perctDist = length(localPos - p2) / (size.y / (sharedData.data.x * sharedData.data.x));
        }
        else if (localPos.x > p1.x)
        {
            perctDist = length(localPos  - p1) / (size.y / (sharedData.data.x * sharedData.data.x));
        }
        else
        {
            perctDist = abs(localPos.y / (size.y / (sharedData.data.x * sharedData.data.x)));
        }
    }

    perctDist /= sharedData.data.x * sharedData.data.x;
    if (perctDist > sharedData.data.z)
    {
        color = vec4(sharedData.colour.xyz, (1.0f - perctDist) * (1.0f / (1.0f - sharedData.data.z)));
    }
    else
    {
        color = sharedData.colour;
    }
}