#version 410 core

out vec4 color;

in vec3 normalOut;

uniform vec4 colour;

void main()
{    
    color = colour;
}