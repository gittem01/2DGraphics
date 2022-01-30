#version 410 core
layout (location = 0) in vec2 vertex;
layout (location = 1) in vec2 uv;

out vec2 uvFrag;

uniform mat4 model;
uniform mat4 ortho;

void main()
{
    uvFrag = uv;
    gl_Position = ortho * model * vec4(vertex.xy, 0.0, 1.0);
}