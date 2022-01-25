#version 330 core
layout (location = 0) in vec2 vertex;

uniform mat4 model;
uniform mat4 ortho;

void main()
{
    gl_Position = ortho * model * vec4(vertex.xy, 0.0, 1.0);
}