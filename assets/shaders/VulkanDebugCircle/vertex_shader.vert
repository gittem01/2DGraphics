#version 460 core

layout (location = 0) in vec2 vertex;

layout (location = 0) out vec4 localPos;

layout (set = 0, binding = 0) uniform UBO
{
    mat4 orthoMatrix;
    mat4 modelMatrix;
} ubo;

void main()
{
    vec4 pos = ubo.orthoMatrix * ubo.modelMatrix * vec4(vertex, 0, 1);
    gl_Position = pos;

    localPos = vec4(vertex, 0, 0);
}