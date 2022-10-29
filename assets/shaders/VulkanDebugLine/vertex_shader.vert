#version 460 core

#define PI 3.141592653589793238

layout (location = 0) in vec2 vertex;

layout (location = 0) out vec2 localPos;
layout (location = 1) out vec2 size;

layout (set = 0, binding = 0) uniform UBO
{
    mat4 ortho;
    vec4 linePoints;
} ubo;

layout (set = 0, binding = 1) uniform SharedUBO
{
    vec4 colour;
    vec4 data; // zoom, thickness, unused, unused
} sharedData;

vec2 rotateAroundOrigin(vec2 point, float angle)
{
    float s = sin(angle);
    float c = cos(angle);

    vec2 p;

    p.x = point.x * c - point.y * s;
    p.y = point.y * c + point.x * s;

    return p;
}

void main()
{
    vec2 diffVec = ubo.linePoints.zw - ubo.linePoints.xy;
    vec2 midPoint = (ubo.linePoints.xy + ubo.linePoints.zw) * 0.5f;
    float dist = length(diffVec);
    float angle;

    // scale, rotate, transform
    vec2 vCopy = vertex;
    // scale
    vCopy.x *= dist;
    vCopy.y *= sharedData.data.y / (sharedData.data.x * sharedData.data.x);

    // rotate
    if (diffVec.x == 0){
        angle = PI / 2;
    }
    else{
        angle = atan(diffVec.y / diffVec.x);
    }

    localPos = vCopy;
    size = vec2(dist * 0.5f, (sharedData.data.y / (sharedData.data.x * sharedData.data.x)) * 0.5f);

    vCopy = rotateAroundOrigin(vCopy, angle);
    // transform
    vCopy += midPoint;

    gl_Position = ubo.ortho * vec4(vCopy, 0.0, 1.0);
}