#version 410 core

layout (location = 0) in vec2 vertex;
layout (location = 1) in vec2 instancedOffset;
layout (location = 2) in vec2 sizes;
layout (location = 3) in vec4 colours;

out vec4 colour;

uniform mat4 ortho;

void main()
{
    colour = colours;
    vec2 v = vec2(vertex.x * sizes.x, vertex.y * sizes.y);
    gl_Position = ortho * vec4(v + instancedOffset, 0.0, 1.0);
}
