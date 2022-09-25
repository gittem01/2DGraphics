#version 450

layout (location = 0) in vec2 inPos;

layout (binding = 0) uniform UBO 
{
	mat4 orthoMatrix;
	mat4 modelMatrix;
} ubo;

out gl_PerVertex 
{
    vec4 gl_Position;   
};

void main() 
{
	gl_Position = ubo.orthoMatrix * ubo.modelMatrix * vec4(inPos.xy, 0.0, 1.0f);
}