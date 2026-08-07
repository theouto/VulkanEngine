#version 450
layout(location = 0) in vec3 position;

layout(push_constant) uniform Push {
    mat4 modelMatrix;   
    mat4 lightSpaceMatrix;
    vec3 lightPos;
} push;

void main() 
{
  int index = gl_InstanceIndex;
  vec4 grid = vec4(position.x + index, position.y - index, position.z + index, 1.0);

  gl_Position = push.lightSpaceMatrix * push.modelMatrix * grid;//vec4(position, 1.0);
}
