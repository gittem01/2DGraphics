#version 460 core

layout (location = 0) out vec4 FragColour;

layout (location = 0) in vec4 localPos;

layout (set = 0, binding = 1) uniform UBO
{
    vec4 colour;
    vec4 data; // zoom, lineThickness, unused, unused
} ubo;

float outerSmoothThicknessRatio = 0.5f;
vec4 outerColour = vec4(1.0f, 0.0f, 0.0f, 1.0f);

void main()
{
    float adjustedOuterLineWidth = ubo.data.y / (ubo.data.x * ubo.data.x);
    float len = length(localPos);
    if (len < 1.0f - adjustedOuterLineWidth)
    {
        FragColour = ubo.colour;
    }
    if (len < 1.0f)
    {
        float midDist = 1 - adjustedOuterLineWidth * 0.5f;
        float distFromMidDist = len - midDist;
        if (len >= midDist)
        {
            float perct = distFromMidDist / (adjustedOuterLineWidth * 0.5f);
            float reverseOuter = 1.0f - outerSmoothThicknessRatio;
            if (perct > reverseOuter)
            {
                perct = (perct - reverseOuter) * (1.0f / outerSmoothThicknessRatio);
                FragColour = vec4(outerColour.xyz, 1 - perct);
            }
            else
            {
                FragColour = outerColour;
            } 
        }
        else if (len >= 1 - adjustedOuterLineWidth)
        {
            float perct = -distFromMidDist / (adjustedOuterLineWidth * 0.5f);
            float reverseOuter = 1.0f - outerSmoothThicknessRatio;
            if (perct > reverseOuter)
            {
                perct = (perct - reverseOuter) * (1.0f / outerSmoothThicknessRatio);
                vec4 innerColour = vec4(ubo.colour.xyz * perct, perct * ubo.colour.w);
                FragColour = vec4(outerColour.xyz * (1 - perct), 1 - perct) + innerColour;
            }
            else
            {
                FragColour = outerColour;
            }
        }
    }
    else
    {
        FragColour = vec4(0, 0, 0, 0);
    }
}