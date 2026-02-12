#version 450

#define PI 3.1415926535897932384626433832795
#define PI_HALF 1.5707963267948966192313216916398
#define near 0.1
#define far 30.f

layout(location = 0) in vec2 texCoords;
layout(location = 1) in vec3 fragPosWorld;

layout(location = 0) out vec4 outColor;

layout(set = 0, binding = 2) uniform sampler2D depthBuffer;

layout(set = 0, binding = 0) uniform GlobalUbo 
{
  mat4 projection;
  mat4 view;
  mat4 invView;
  mat4 viewStat;
} ubo;

float LinearizeDepth(float depth) 
{
    float z = depth * 2.0 - 1.0; // back to NDC 
    return (2.0 * near * far) / (far + near - z * (far - near));	
}

float FastAcos2(vec2 x)
{
    return (-0.69813170*x*x - 0.87266463)*x + 1.57079633;
}

float integrate_arc(vec2 h, float n, float cos_n) {
	vec2 tmp = cos_n + 2.0 * h * sin(n) - cos(2.0 * h - n);
	return 0.25 * (tmp.x + tmp.y);
}

vec3 screen_to_view_space(mat4 projection_matrix_inverse, vec3 screen_pos, bool handle_jitter) {
	vec3 ndc_pos = 2.0 * screen_pos - 1.0;
	return project_and_divide(projection_matrix_inverse, ndc_pos);
}

vec2 clamp01(vec2 a)
{
  return vec2(clamp(a.x, 0.f, 1.f), clamp(a.y, 0.f, 1.f));
}

vec2 traceSlice(vec2 xy, vec3 viewV, )

#define RADIUS 2.f
#define FALLOFF_START 1.5f
#define SLICES 4
#define HORIZON_STEPS 6
#define RADIUS

//https://github.com/Zenteon/ZenteonFX/blob/main/Shaders/Zenteon_SSAO_History.fx
void main() 
{
  vec2 AOacc = vec2(0.f);

  

  return vec4(vec3(clamp01(AOacc.x/AOacc.y)), 1.f)
}
