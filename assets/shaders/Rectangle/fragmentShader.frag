#version 330 core

out vec4 color;

in vec2 uvFrag;

uniform vec4 inColour;
uniform vec4 outColour;
uniform vec3 bgColour;
uniform vec2 scale;

float scaler = scale.x / scale.y;

float radiusy;
float radiusx; 
float tHoldy;
float tHoldx;

uniform float powVal;
uniform float uRadius;
uniform float uThold;
uniform int innerGlow;

float mDistance(vec2 v1, vec2 v2){
    return sqrt(pow(v1.x - v2.x, 2) + pow((v1.y - v2.y) * (1 / scaler), 2));
}

float calcNearness(float val, float fScaler){
    if (val > 0.5f){
        val = 1 - val;
        val *= fScaler;
        return 1 - val;
    }
    else{
        val *= fScaler;
        return 1.0f - val;
    }
}

void main()
{
    float baseRadius = uRadius;
    float baseThold = uThold;

    if (baseRadius > 0.5f){
        baseRadius = 0.5f;
    }
    if (baseRadius < 0.0f) baseRadius = 0.0f;

    if (scaler > 1){
        radiusy = baseRadius;
        radiusx = radiusy / scaler;
        tHoldy = baseThold;
        tHoldx = tHoldy / scaler;
    }
    else{
        radiusx = baseRadius;
        radiusy = radiusx * scaler;
        tHoldx = baseThold;
        tHoldy = tHoldx * scaler;
    }

    float plusValx;
    float plusValy;
    float divr;
    if (innerGlow == 1){
        plusValx = 0.0f;
        plusValy = 0.0f;
        divr = 1.0f;
    }
    else{
        plusValx = tHoldx / 2.0f;
        plusValy = tHoldy / 2.0f;
        divr = 2.0f;
    }

    int x, y;
    for (int i = 0; i < 4; i++){
        x = i & 1;
        y = (i >> 1) & 1;
        vec2 aPos = vec2(x - radiusx, y - radiusy);
        aPos.x *= 2 * x - 1;
        aPos.y *= 2 * y - 1;
        float dist = mDistance(uvFrag, aPos);
        float minDist = radiusx - tHoldx;
        float maxDist = radiusx;
        float midDist = (maxDist + minDist) * 0.5f;
        if (dist < maxDist && dist > minDist && 
            (aPos.x - uvFrag.x) * (x * 2 - 1) < 0 && (aPos.y - uvFrag.y) * (y * 2 - 1) < 0)
        {
            float plusValx = tHoldx / 2.0f;
            if (innerGlow == 1){
                plusValx = 0.0f;
            }
            float distDiff = abs(dist - radiusx + plusValx) * (divr / (maxDist - minDist));
            distDiff = pow(distDiff, powVal);
            if (dist < midDist || innerGlow == 1){
                color = outColour * (1 - distDiff) + inColour * (distDiff);
            }
            else{
                color = outColour * (1 - distDiff) + vec4(bgColour.xyz, 0) * (distDiff);
            }
            return;
        }
        if (dist >= radiusx && (aPos.x - uvFrag.x) * (x * 2 - 1) < 0 && (aPos.y - uvFrag.y) * (y * 2 - 1) < 0)
        {
            color = vec4(0, 0, 0, 0);
            return;
        }
    }

    if (1 - uvFrag.x < tHoldx || 1 - uvFrag.y < tHoldy || uvFrag.x < tHoldx || uvFrag.y < tHoldy){
        if ((1 - uvFrag.x < tHoldx || uvFrag.x < tHoldx) && calcNearness(uvFrag.y, 1.0f) <= calcNearness(uvFrag.x, scaler)){
            float mUvf;
            if (uvFrag.x < tHoldx){
                mUvf = 1 - uvFrag.x;
            }
            else{
                 mUvf = uvFrag.x;
            }
            float distDiff = abs(mUvf - 1 + plusValx) * (divr / tHoldx);
            distDiff = pow(distDiff, powVal);
            if (mUvf < 1 - plusValx || plusValx == 1){
                color = outColour * (1 - distDiff) + inColour * (distDiff);
            }
            else{
                color = outColour * (1 - distDiff) + vec4(bgColour.xyz, 0) * (distDiff);
            }
            return;
        }
        else if ((1 - uvFrag.y < tHoldy || uvFrag.y < tHoldy) && calcNearness(uvFrag.x, scaler) <= calcNearness(uvFrag.y, 1.0f)){
            float mUvf;
            if (uvFrag.y < tHoldy){
                mUvf = 1 - uvFrag.y;
            }
            else{
                 mUvf = uvFrag.y;
            }
            float distDiff = abs(mUvf - 1 + plusValy) * (divr / tHoldy);
            distDiff = pow(distDiff, powVal);
            if (mUvf < 1 - plusValy || plusValy == 1){
                color = outColour * (1 - distDiff) + inColour * (distDiff);
            }
            else{
                color = outColour * (1 - distDiff) + vec4(bgColour.xyz, 0) * (distDiff);
            }
            return;
        }
        else color = inColour;
    }
    else{
        color = inColour;
    }
}