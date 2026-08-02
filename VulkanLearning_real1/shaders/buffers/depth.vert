#version 450

layout(location = 0) in vec3 position;

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

layout(push_constant) uniform Push 
{
  mat4 modelMatrix;
  mat4 normalMatrix;
} push;

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
  vec4 positionWorld = push.modelMatrix * vec4(position, 1.0);
  gl_Position = ubo.projection * ubo.view * positionWorld;
}
