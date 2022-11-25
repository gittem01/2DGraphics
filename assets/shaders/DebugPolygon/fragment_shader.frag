#version 460 core

layout (location = 0) out vec4 color;

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
    color = sharedData.colour;
}