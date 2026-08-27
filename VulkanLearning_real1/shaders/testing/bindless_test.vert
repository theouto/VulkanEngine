#version 450
#extension GL_EXT_scalar_block_layout : enable

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec2 uv;

layout(location = 3) in vec3 scale;
layout(location = 4) in vec3 rotation;
layout(location = 5) in vec3 translation;
layout(location = 6) in ivec3 RIDone;
layout(location = 7) in ivec3 RIDtwo;
layout(location = 8) in vec4 modifiers;

layout(location = 0) out vec3 fragPosWorld;
layout(location = 1) out vec3 fragNormalWorld;
layout(location = 2) out vec2 fragUv;
layout(location = 3) out vec4 FragPosLightSpace[4];
layout(location = 7) out uint fRID[6];
layout(location = 13) out vec4 fmodifiers;

const float PI = 3.1415926535897932384626433832795;
const float rotator = PI / 180.f;

struct PointLight
{
	vec4 position;
	vec4 color;
};

layout(std430, set = 0, binding = 0) uniform GlobalUbo 
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
  int padding;
  mat4 lightSpaceMatrix[4];
  vec3 lightPos;
} ubo;

layout(push_constant) uniform Push 
{
  mat4 modelMatrix;
  mat4 normalMatrix;
  uint RIDo;
  uint RID[7];
  //vec4 used for alignment reasons
} push;

//https://shader-tutorial.dev/basics/vertex-shader/
//and also
//https://www.geeksforgeeks.org/maths/rotation-matrix/
mat4 rotateZ(float angle)
{
  mat4 rotationMatrix;
  rotationMatrix[0] = vec4(cos(angle), sin(angle), 0, 0);
  rotationMatrix[1] = vec4(-sin(angle), cos(angle), 0, 0);
  rotationMatrix[2] = vec4(0, 0, 1, 0);
  rotationMatrix[3] = vec4(0, 0, 0, 1);
  return rotationMatrix;
}

mat4 rotateY(float angle)
{
  mat4 rotationMatrix;
  rotationMatrix[0] = vec4(cos(angle), 0, sin(angle), 0);
  rotationMatrix[1] = vec4(0, 1, 0, 0);
  rotationMatrix[2] = vec4(-sin(angle), 0, cos(angle), 0);
  rotationMatrix[3] = vec4(0, 0, 0, 1);
  return rotationMatrix;
}

mat4 rotateX(float angle)
{
  mat4 rotationMatrix;
  rotationMatrix[0] = vec4(1, 0, 0, 0);
  rotationMatrix[1] = vec4(0, cos(angle), -sin(angle), 0);
  rotationMatrix[2] = vec4(0, sin(angle), cos(angle), 0);
  rotationMatrix[3] = vec4(0, 0, 0, 1);
  return rotationMatrix;
}

void main()
{
  mat4 scaleMatrix =
  {
    vec4(scale.x, 0, 0, 0),
    vec4(0, scale.y, 0, 0),
    vec4(0, 0, scale.z, 0),
    vec4(0, 0, 0, 1)
  };

  mat4 invScaleMatrix =
  {
    vec4(1/scale.x, 0, 0, 0),
    vec4(0, 1/scale.y, 0, 0),
    vec4(0, 0, 1/scale.z, 0),
    vec4(0, 0, 0, 1)
  };

  mat4 rotationMatrix = rotateZ(rotation.x * rotator) * rotateY(rotation.y * rotator) * rotateX(rotation.z * rotator);

  mat4 mat = {push.modelMatrix[0], push.modelMatrix[1], push.modelMatrix[2], vec4(vec3(0.f), 1.f)};

  mat4 instanceMatrix = scaleMatrix * mat;
  instanceMatrix = rotationMatrix * instanceMatrix;

  instanceMatrix[3] = vec4(translation.x, -translation.y, translation.z, 1.f);

  vec4 positionWorld = instanceMatrix * vec4(position, 1.f);
  gl_Position = ubo.projection * ubo.view * positionWorld;

  vec3 nuNormal = normal;

  fragNormalWorld = normalize(mat3(rotationMatrix * invScaleMatrix * push.normalMatrix) * normal);
  fragPosWorld = positionWorld.xyz;
  fragUv = uv;
  fmodifiers = modifiers;

  for (int i = 0; i < 6; i++)
  {
    if (i < 3) {fRID[i] = RIDone[i];} else {fRID[i] = RIDtwo[i - 3];}
    if (i < 4) FragPosLightSpace[i] = ubo.lightSpaceMatrix[i] * positionWorld;
  }
}
