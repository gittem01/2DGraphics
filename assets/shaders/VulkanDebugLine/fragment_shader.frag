#version 460 core

layout (location = 0) out vec4 color;

layout (location = 0) in vec2 localPos;
layout (location = 1) in vec2 size;

layout (set = 0, binding = 1) uniform SharedUBO
{
    vec4 colour;
    vec4 data; // zoom, thickness, unused, unused
} sharedData;

float lineRel(vec2 a, vec2 b, vec2 c){
     return (b.x - a.x) * (c.y - a.y) - (b.y - a.y) * (c.x - a.x);
}

void main()
{
    vec2 p1 = vec2(size.x - size.y, 0.0f);
    vec2 p2 = vec2(-size.x + size.y, 0.0f);
    if (localPos.x < p2.x && length(localPos - p2) > size.y || localPos.x > p1.x && length(localPos - p1) > size.y){
        color = vec4(0, 0, 0, 0);
        return;
    }
    color = sharedData.colour;
}