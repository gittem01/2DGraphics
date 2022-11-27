#version 460 core

struct triangle
{
    vec2 p1;
    vec2 p2;
    vec2 p3;
};

layout (location = 0) out vec4 color;

layout (location = 0) in vec2 localPos;

layout (set = 0, binding = 0) uniform UBO
{
    mat4 ortho;
    vec4 polyPoints[4];
    int numPoints;
} ubo;

layout (set = 0, binding = 1) uniform SharedUBO
{
    vec4 colour;
    vec4 data; // zoom, thickness, outerSmoothness, unused
} sharedData;

float getDist(vec2 a, vec2 b, vec2 c)
{
    float v1 = (b.x - a.x) * (a.y - c.y);
    float v2 = (a.x - c.x) * (b.y - a.y);
    float top = abs(v1 - v2);

    v1 = pow(b.x - a.x, 2);
    v2 = pow(b.y - a.y, 2);
    float bot = sqrt(v1 + v2);

    return top / bot;
}

void main()
{
    vec4 innerColour = vec4(0.0f, 0.5f, 1.0f, 0.1f);
    float d = 0.1f;
    d = d / (sharedData.data.x * sharedData.data.x);
    float smth = 1.0f;
    float minVal = 54426262.736f;
    for (int i1 = 0; i1 < ubo.numPoints; i1++)
    {
        int i2 = (i1 + 1) % ubo.numPoints;

        vec2 v1 = vec2(ubo.polyPoints[i1 / 2][(i1 % 2) * 2], ubo.polyPoints[i1 / 2][(i1 % 2) * 2 + 1]);
        vec2 v2 = vec2(ubo.polyPoints[i2 / 2][(i2 % 2) * 2], ubo.polyPoints[i2 / 2][(i2 % 2) * 2 + 1]);
        float dist = getDist(v1, v2, localPos);
        if (dist < d)
        {
            if (dist < minVal)
            {
                minVal = dist;
            }
        }
    }

    if (minVal != 54426262.736f)
    {
        float bias = 0.5f;
        float perct = (d * 0.5f - minVal) / (d * 0.5f);
        if (perct > bias)
        {
            float newPerct = (perct - bias) / (1 - bias);
            color = vec4(sharedData.colour.xyz, (1 - newPerct) * sharedData.colour.w);
        }
        else if (perct < -bias)
        {
            float newPerct = (-perct - bias) / (1 - bias);
            float alphaVal = sharedData.colour.w + newPerct * (innerColour.w - sharedData.colour.w);
            vec3 colourMix = sharedData.colour.xyz * (1 - newPerct * innerColour.w / alphaVal) + innerColour.xyz * newPerct * innerColour.w / alphaVal;
            color = vec4(colourMix, alphaVal);
            //color = vec4(sharedData.colour.xyz, 1 - newPerct);
        }
        else
        {
            color = sharedData.colour;
        }
    }
    else
    {
        color = innerColour;
    }
}