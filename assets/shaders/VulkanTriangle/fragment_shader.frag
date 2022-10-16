#version 450

layout (location = 0) out vec4 outFragColor;

layout (location = 0) in vec2 texCoord;

layout(set = 1, binding = 0) uniform sampler2D tex;

layout (set = 0, binding = 1) uniform UBO
{
	int numPortals;
} ubo;

layout(std140, set = 2, binding = 0) readonly buffer LightBuffer {
	vec4 portals[];
} lightBuffer;

void main()
{
	outFragColor = texture(tex, texCoord) * ubo.numPortals;
}