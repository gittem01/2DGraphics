#version 410 core

layout (location = 0) in vec3 vertex;
layout (location = 1) in vec3 normal;

out vec3 normalOut;

uniform mat4 model;
uniform mat4 ortho;

void main()
{
    gl_Position = ortho * model * vec4(vertex, 1.0);
    normalOut = normal;
}