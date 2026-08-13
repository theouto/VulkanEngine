#version 450

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec2 uv;

layout(location = 3) in mat3 instanceVariables;
layout(location = 6) in uint RID[6];

layout(location = 0) out vec3 fragPosWorld;
layout(location = 1) out vec3 fragNormalWorld;
layout(location = 2) out vec2 fragUv;
layout(location = 3) out uint fRID[6];

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
} ubo;

layout(push_constant) uniform Push {
    mat4 modelMatrix;
    mat4 normalMatrix;
} push;

void main() 
{
  int columns = 90;

  float offset = 1.5f;

  int index = gl_InstanceIndex;

  int fullRows = index/columns;
  int remainder = index%columns;
  float ydelta = fullRows*offset;
  float xdelta = remainder*offset;

  vec4 grid = vec4(position + vec3(xdelta, ydelta, 0.f), 1.f);

  /*
  vec4 instancePosition = vec4(position + instanceVariables[0], 0.f);
 
  mat4 scaleMatrix;
  scaleMatrix[0] = vec4(instanceVariables[2].x, 0, 0, 0);
  scaleMatrix[1] = vec4(0, push.scale[2].y, 0, 0);
  scaleMatrix[2] = vec4(0, 0, push.scale[2].z, 0);
  scaleMatrix[3] = vec4(0, 0, 0, 1);
  mat4 rotationMatrix = rotateZ(instanceVariables[1].x * rotator) * rotateY(instanceVariables[1].y * rotator) * rotateX(instanceVariables[1].z * rotator);

  mat4 appliedTransformation = scaleMatrix * rotationMatrix;
  */

  vec4 positionWorld = push.modelMatrix * grid;//vec4(position, 1.f);
  gl_Position = ubo.projection * ubo.view * positionWorld;

  fragNormalWorld = normalize(mat3(push.normalMatrix) * normal);
  fragPosWorld = positionWorld.xyz;
  fragUv = uv;
}
