#version 450
#define NEAR 0.1
#define FAR 30.f

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec3 fragPosWorld;
layout(location = 2) in vec3 fragNormalWorld;
layout(location = 3) in vec2 fragUv;

layout(set = 1, binding = 3) uniform sampler2D normals;
layout(set = 1, binding = 2) uniform sampler2D specular;
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

float LinearizeDepth(float depth) 
{
  return NEAR * FAR / (FAR + depth * (NEAR - FAR));	
}

mat3 cotangent_frame( vec3 N, vec3 p, vec2 uv )
{
    // get edge vectors of the pixel triangle
    vec3 dp1 = dFdx(p);
    vec3 dp2 = dFdy(p);
    vec2 duv1 = dFdx(uv);
    vec2 duv2 = dFdy(uv);

    // solve the linear system
    vec3 dp2perp = cross( dp2, N );
    vec3 dp1perp = cross( N, dp1 );
    vec3 T = dp2perp * duv1.x + dp1perp * duv2.x;
    vec3 B = dp2perp * duv1.y + dp1perp * duv2.y;

    // construct a scale-invariant frame 
    float invmax = inversesqrt( max( dot(T,T), dot(B,B) ) );
    return mat3( T * invmax, B * invmax, N );
}

vec3 perturb_normal( vec3 N, vec3 V, vec2 texcoord ) 
{ 
  // assume N, the interpolated vertex normal and // V, the view vector (vertex to eye) 
  vec3 map = texture( normals, texcoord ).xyz; 
  map = map * 255./127. - 128./127.; 
  map.y = -map.y; 
  mat3 TBN = cotangent_frame( N, V, texcoord ); 
  return normalize( TBN * map ); 
}

void main()
{
    vec2 projCoords = vec2(gl_FragCoord.x/ubo.width, gl_FragCoord.y/ubo.height);
    float currDepth = LinearizeDepth(gl_FragCoord.z);
    float prePassDepth = LinearizeDepth(texture(depthMap, projCoords).r);
    
    if (prePassDepth < currDepth) discard;

    vec2 UVs = fragUv;

    vec3 surfaceNormal = normalize(fragNormalWorld);

    vec3 cameraPosWorld = ubo.invView[3].xyz;
	vec3 viewDirection = normalize(cameraPosWorld - fragPosWorld);

    surfaceNormal = perturb_normal(surfaceNormal, viewDirection, UVs);

    float spec = texture(specular, UVs).r;

    outColor = vec4(surfaceNormal, spec);
}
