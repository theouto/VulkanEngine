#version 450
layout(location = 0) in vec3 position;

layout(push_constant) uniform Push {
    mat4 modelMatrix;   
    mat4 lightSpaceMatrix;
    vec3 lightPos;
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

  gl_Position = push.lightSpaceMatrix * push.modelMatrix * grid;//vec4(position, 1.0);
}
