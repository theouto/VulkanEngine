#version 450

layout(location = 0) in vec3 position; //to be deprecated
layout(location = 1) in vec3 color;
layout(location = 2) in vec3 normal;
layout(location = 3) in vec2 uv;

layout(location = 0) out vec3 fragColor;
layout(location = 1) out vec3 fragPosWorld;
layout(location = 2) out vec3 fragNormalWorld;
layout(location = 3) out vec2 fragUv;
layout(location = 4) out vec4 FragPosLightSpace[4];

const float PI = 3.1415926535897932384626433832795;

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
  //vec4 position[8192];
  //vec4 rotaton[8192];
  //vec4 scale[8192];
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
  float rotator = PI / 180.f;
  vec4 positionWorld = rotateZ(30 * rotator) * rotateY(30 * rotator) * rotateX(30 * rotator) * push.modelMatrix * vec4(position, 1.0);
  gl_Position = ubo.projection * ubo.view * positionWorld;

  fragNormalWorld = normalize(mat3(push.normalMatrix) * normal);
  fragPosWorld = positionWorld.xyz;
  fragColor = color;
  fragUv = uv;
  for (int i = 0; i < 4; i++)
  {
    FragPosLightSpace[i] = ubo.lightSpaceMatrix[i] * push.modelMatrix * vec4(position, 1.f);
  }
}
