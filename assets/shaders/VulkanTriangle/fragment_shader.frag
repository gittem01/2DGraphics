#version 450

layout (location = 0) out vec4 outFragColor;

layout (location = 0) in vec2 texCoord;

void main()
{
  outFragColor = vec4(texCoord, 1.0, 0.5);
}