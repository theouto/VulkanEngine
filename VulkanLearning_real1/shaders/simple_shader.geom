#version 450

layout (triangles) in;
layout (triangle_strip, max_vertices = 3) out;

layout(location = 0) out vec3 Normal;
layout(location = 1) out vec3 color;
layout(location = 2) out vec2 texCoord;

layout(location = 0) in DATA
{
	vec3 Normal;
	vec3 color;
	vec2 texCoord;
	mat4 projection;
} data_in[];

void main()
{
	gl_position = data_in[0].projection * gl_in[0].gl_position;
	Normal = data_in[0].Normal;
	color = data_in[0].color;
	texCoord = data_in[0].texCoord;
	EmitVertex();

	gl_position = data_in[1].projection * gl_in[1].gl_position;
	Normal = data_in[1].Normal;
	color = data_in[1].color;
	texCoord = data_in[1].texCoord;
	EmitVertex();

	gl_position = data_in[2].projection * gl_in[2].gl_position;
	Normal = data_in[2].Normal;
	color = data_in[2].color;
	texCoord = data_in[2].texCoord;
	EmitVertex();

	EndPrimitive();
}