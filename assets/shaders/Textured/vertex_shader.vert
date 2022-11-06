#version 450

layout (location = 0) in vec2 inPos;
layout (location = 1) in vec2 i_texCoord;

layout (location = 0) out vec2 o_texCoord;
layout (location = 1) out vec2 worldPosition;

layout (set = 0, binding = 0) uniform UBO
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
	o_texCoord = i_texCoord;
	vec4 wp = ubo.modelMatrix * vec4(inPos.xy, 0.0, 1.0f);
	worldPosition = wp.xy;

	gl_Position = ubo.orthoMatrix * wp;
}