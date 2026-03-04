#version 450

#define PI 3.1415926535897932384626433832795
#define PI_HALF 1.5707963267948966192313216916398
#define near 0.1
#define far 30.f

layout(location = 0) in vec2 texCoords;

layout(location = 0) out vec4 outColor;

layout(set = 0, binding = 2) uniform sampler2D depthBuffer;
layout(set = 0, binding = 3) uniform sampler2D normalSpec;

struct PointLight
{
	vec4 position;
	vec4 color;
};

// https://graphics.stanford.edu/%7Eseander/bithacks.html
uint bitCounter(uint value) {
    value = value - ((value >> 1u) & 0x55555555u);
    value = (value & 0x33333333u) + ((value >> 2u) & 0x33333333u);
    return ((value + (value >> 4u) & 0xF0F0F0Fu) * 0x1010101u) >> 24u;
}

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

// https://cdrinmatane.github.io/posts/ssaovb-code/
const uint sectorCount = 32u;
uint updateSectors(float minHorizon, float maxHorizon, uint outBitfield) {
    uint startBit = uint(minHorizon * float(sectorCount));
    uint horizonAngle = uint(ceil((maxHorizon - minHorizon) * float(sectorCount)));
    uint angleBit = horizonAngle > 0u ? uint(0xFFFFFFFFu >> (sectorCount - horizonAngle)) : 0u;
    uint currentBitfield = angleBit << startBit;
    return outBitfield | currentBitfield;
}

// https://blog.demofox.org/2022/01/01/interleaved-gradient-noise-a-different-kind-of-low-discrepancy-sequence/
float randf(int x, int y) {
    return mod(52.9829189 * mod(0.06711056 * float(x) + 0.00583715 * float(y), 1.0), 1.0);
}

#define RADIUS 2.f
#define FALLOFF_START 1.5f
#define Nd 4
#define Ns 6
#define THICKNESS 0.4f


//https://cybereality.com/screen-space-indirect-lighting-with-visibility-bitmask-improvement-to-gtao-ssao-real-time-ambient-occlusion-algorithm-glsl-shader-implementation/
void main() 
{
  discard;
  /*
  uint indirect = 0u;
  uint occlusion = 0u;

  float visibility = 0.0;
  vec3 lighting = vec3(0.0);
  vec2 frontBackHorizon = vec2(0.0);
  vec2 aspect = screenSize.yx / screenSize.x;
  vec3 position = texture(screenPosition, fragUV).rgb
  vec3 camera = normalize(-position);
  vec3 normal = normalize(texture(screenNormal, fragUV).rgb);

  float sliceRotation = twoPi / (sliceCount - 1.0);
  float sampleScale = (-sampleRadius * projection[0][0]) / position.z;
  float sampleOffset = 0.01;
  float jitter = randf(int(gl_FragCoord.x), int(gl_FragCoord.y)) - 0.5;

  for (int i = 0; i < Nd; i++)
  {

  }
  */
}
