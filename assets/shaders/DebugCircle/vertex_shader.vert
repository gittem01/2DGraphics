#version 460 core

layout (location = 0) in vec2 vertex;

layout (location = 0) out vec4 localPos;
layout (location = 1) out vec2 worldPosition;

layout (set = 0, binding = 0) uniform SharedUboData
{
    mat4 orthoMatrix;
} sharedUboData;

layout (set = 0, binding = 1) uniform UBO
{
    mat4 modelMatrix;
} ubo;

void main()
{
    vec4 pos = sharedUboData.orthoMatrix * ubo.modelMatrix * vec4(vertex, 0, 1);

    gl_Position = pos;

    vec4 wp = ubo.modelMatrix * vec4(vertex, 0.0, 1.0f);
    localPos = vec4(vertex, 0, 0);
    worldPosition = wp.xy;
}