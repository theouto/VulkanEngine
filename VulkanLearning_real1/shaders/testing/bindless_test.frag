#version 450
#extension GL_EXT_nonuniform_qualifier : enable

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec3 fragPosWorld;
layout(location = 2) in vec3 fragNormalWorld;
layout(location = 3) in vec2 fragUv;

layout(location = 0) out vec4 outColor;

layout(binding = 2) uniform sampler2D storageSampler[];

layout(push_constant) uniform Push 
{
  mat4 modelMatrix;
  mat4 normalMatrix;
  uint RID;
} push;

struct PointLight
{
	vec4 position;
	vec4 color;
};

layout(set = 0, binding = 0) uniform GlobalUbo 
{
  mat4 projection;
  mat4 view;
  mat4 invView;
  mat4 viewStat;
  vec4 ambientLightColor; // w is intensity
  PointLight pointLights[10];
  int numLights;
  int width;
  int height;
} ubo;

void main()
{
  vec2 projCoords = vec2(gl_FragCoord.x/ubo.width, gl_FragCoord.y/ubo.height);

  outColor = vec4(texture(storageSampler[3], fragUv).rgb, 1.f);
}
