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
    gl_Position = ubo.orthoMatrix * ubo.modelMatrix * vec4(vertex, 0, 1);
    localPos = vec4(vertex, 0, 0);
}