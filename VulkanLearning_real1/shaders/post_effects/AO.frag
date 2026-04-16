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
#define Nd 2
#define Ns 2
#define THICKNESS 0.4f


//https://cybereality.com/screen-space-indirect-lighting-with-visibility-bitmask-improvement-to-gtao-ssao-real-time-ambient-occlusion-algorithm-glsl-shader-implementation/
void main() 
{

  vec2 projCoords = vec2(gl_FragCoord.x/ubo.width, gl_FragCoord.y/ubo.height);

  outColor = vec4(texture(normalSpec, projCoords).rgb, 1.f);

  return;
  //discard;
  uint indirect = 0u;
  uint occlusion = 0u;

  float visibility = 0.0;
  vec3 lighting = vec3(0.0);
  vec2 frontBackHorizon = vec2(0.0);
  vec2 aspect = vec2(ubo.height, ubo.width) / ubo.width;
  vec3 position = vec3(texCoords, texture(depthBuffer, texCoords).r);
  vec3 camera = normalize(-position);
  vec3 normal = normalize(texture(normalSpec, texCoords).rgb);

  float sliceRotation = PI_HALF / (Ns - 1.0);
  float sampleScale = (-RADIUS) / position.z;
  float sampleOffset = 0.01;
  float jitter = randf(int(gl_FragCoord.x), int(gl_FragCoord.y)) - 0.5;

  

  for (float slice = 0.0; slice < Nd + 0.5; slice += 1.0) {
        float phi = sliceRotation * (slice + jitter) + PI;
        vec2 omega = vec2(cos(phi), sin(phi));
        vec3 direction = vec3(omega.x, omega.y, 0.0);
        vec3 orthoDirection = direction - dot(direction, camera) * camera;
        vec3 axis = cross(direction, camera);
        vec3 projNormal = normal - axis * dot(normal, axis);
        float projLength = length(projNormal);

        float signN = sign(dot(orthoDirection, projNormal));
        float cosN = clamp(dot(projNormal, camera) / projLength, 0.0, 1.0);
        float n = signN * acos(cosN);

        for (float currentSample = 0.0; currentSample < Ns + 0.5; currentSample += 1.0) {
            float sampleStep = (currentSample + jitter) / Ns + sampleOffset;
            vec2 sampleUV = texCoords - sampleStep * sampleScale * omega * aspect;
            vec3 samplePosition = vec3(sampleUV, texture(depthBuffer, sampleUV).r);
            vec3 sampleNormal = normalize(texture(normalSpec, sampleUV).rgb);
            vec3 sampleDistance = samplePosition - position;
            float sampleLength = length(sampleDistance);
            vec3 sampleHorizon = sampleDistance / sampleLength;

            frontBackHorizon.x = dot(sampleHorizon, camera);
            frontBackHorizon.y = dot(normalize(sampleDistance - camera * THICKNESS), camera);

            frontBackHorizon = acos(frontBackHorizon);
            frontBackHorizon = clamp((frontBackHorizon + n + PI_HALF) / PI, 0.0, 1.0);

            indirect = updateSectors(frontBackHorizon.x, frontBackHorizon.y, 0u);

            /*
            lighting += (1.0 - float(bitCount(indirect & ~occlusion)) / float(sectorCount)) *
                sampleLight * clamp(dot(normal, sampleHorizon), 0.0, 1.0) *
                clamp(dot(sampleNormal, -sampleHorizon), 0.0, 1.0);
            */

            occlusion |= indirect;
        }
        visibility += 1.0 - float(bitCount(occlusion)) / float(sectorCount);
    }

    visibility /= Nd;
    lighting /= Nd;

    outColor = vec4(vec3(visibility), 1.f);  
}
