#version 450
#extension GL_EXT_nonuniform_qualifier : enable
#define NEAR 0.1
#define FAR 30.f

layout(location = 0) in vec3 fragPosWorld;
layout(location = 1) in vec3 fragNormalWorld;
layout(location = 2) in vec2 fragUv;
layout(location = 3) flat in uint fRID[6];

layout(set = 1, binding = 0) uniform sampler2D textures[];

layout(set = 0, binding = 2) uniform sampler2D depthMap;

layout (location = 0) out vec4 outColor;

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

layout(push_constant) uniform Push
{
  mat4 modelMatrix;
  mat4 normalMatrix;
  uint relevantRid[4];
}push;

float LinearizeDepth(float depth) 
{
  return NEAR * FAR / (FAR + depth * (NEAR - FAR));	
}

mat3 cotangent_frame( vec3 normal, vec3 worldPos, vec2 texCoord )
{
  vec3 Q1 = dFdx(worldPos);
  vec3 Q2 = dFdy(worldPos);
  vec2 st1 = dFdx(texCoord);
  vec2 st2 = dFdy(texCoord);

  vec3 N = normalize(normal);
  vec3 T = normalize(Q1 * st2.t - Q2 * st1.t);
  vec3 B = -normalize(cross(N, T));

  mat3 TBN = mat3(T, B, N);
  return TBN;
}

vec3 perturb_normal( vec3 N, vec3 V, vec2 texcoord ) 
{ 
  // assume N, the interpolated vertex normal and // V, the view vector (vertex to eye) 
  vec3 map = texture( textures[fRID[nonuniformEXT(2)]], texcoord ).rgb; 
  map = map * 255./127. - 128./127.;
  mat3 TBN = cotangent_frame( N, V, texcoord ); 
  return normalize( TBN * map ); 
}

void main()
{
    vec2 projCoords = vec2(gl_FragCoord.x/ubo.width, gl_FragCoord.y/ubo.height);
    float currDepth = gl_FragCoord.z;
    float prePassDepth = texture(depthMap, projCoords).r;

    if (prePassDepth < currDepth) discard;

    vec2 UVs = fragUv;

    vec3 surfaceNormal = normalize(fragNormalWorld);

    vec3 cameraPosWorld = ubo.invView[3].xyz;
	vec3 viewDirection = normalize(cameraPosWorld - fragPosWorld);

    surfaceNormal = perturb_normal(surfaceNormal, fragPosWorld, UVs);
    //surfaceNormal.z = -surfaceNormal.z;

    float spec = texture(textures[fRID[nonuniformEXT(1)]], UVs).r;

    outColor = vec4(surfaceNormal, spec);
}
