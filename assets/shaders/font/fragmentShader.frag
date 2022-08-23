#version 410 core

in vec2 TexCoords;
out vec4 color;

uniform sampler2D text;
uniform vec3 textColor;

void main()
{
    vec4 totalColor = vec4(0, 0, 0, 0);
    int gridsize = 25;
    float x = -sqrt(gridsize) / 2;
    float y = -sqrt(gridsize) / 2;
    float mult = 0.004f;
    for (int i = 0; i < gridsize; i++){
        totalColor += texture(text, vec2(TexCoords.x + x * mult, TexCoords.y + y * mult));
        x += 1;
        if (x > 1){
            x = -1;
            y += 1;
        }
    }

    totalColor /= gridsize;

    color = vec4(textColor, totalColor.r);
}