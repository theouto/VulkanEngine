#version 450
#extension GL_EXT_nonuniform_qualifier : enable
#define NEAR 0.01f
#define FAR 200.f


layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec3 fragPosWorld;
layout(location = 2) in vec3 fragNormalWorld;
layout(location = 3) in vec2 fragUv;
layout(location = 4) in vec4 FragPosLightSpace[4];

layout(location = 0) out vec4 outColor;

//layout(set = 0, binding = 1) uniform sampler2D shadowMap;
layout(set = 0, binding = 2) uniform sampler2D depthMap;
layout(set = 0, binding = 3) uniform sampler2D normalSpec;

layout(set = 1, binding = 0) uniform sampler2D storageSampler[];
layout(set = 2, binding = 0) uniform sampler2D shadowStorage[];

layout(push_constant) uniform Push 
{
  mat4 modelMatrix;
  mat4 normalMatrix;
  uint RIDo;
  uint RID[7];
  float modifiers[4];
} push;

struct PointLight
{
	vec4 position;
	vec4 color;
};

struct DirectionalLight
{
  vec3 direction;
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
  mat4 lightSpaceMatrix[4]; //this is ugly
  vec3 lightPos;
} ubo;

const float M_PI = 3.1415926538;

const vec2 invAtan = vec2(0.1591, 0.3183);

//==============================================================================

vec2 SampleSphericalMap(vec3 v)
{
    vec2 uv = vec2(atan(v.z, v.x), asin(v.y));
    uv *= invAtan;
    uv += 0.5;
    return uv;
}

//==============================================================================

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

//==============================================================================

float DistributionGGX(vec3 N, vec3 H, float a)
{
    float a2     = a*a;
    float NdotH  = max(dot(N, H), 0.0);
    float NdotH2 = NdotH*NdotH;
	
    float nom    = a2;
    float denom  = (NdotH2 * (a2 - 1.0) + 1.0);
    denom        = M_PI * denom * denom;
	
    return nom / denom;
}

//==============================================================================

float GeometrySchlickGGX(float NdotV, float roughness)
{
    float r = (roughness + 1.0);
    float k = (r*r) / 8.0;

    float nom   = NdotV;
    float denom = NdotV * (1.0 - k) + k;

    return nom / denom;
}

float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2 = GeometrySchlickGGX(NdotV, roughness);
    float ggx1 = GeometrySchlickGGX(NdotL, roughness);

    return ggx1 * ggx2;
}

//==============================================================================

vec3 metallic(vec3 fres, float metal)
{
  vec3 kD = vec3(1.f) - fres;
  kD *= 1.f - metal;
  
  return kD; 
}

//==============================================================================

vec3 fresnelSchlick(float cosTheta, vec3 F0)
{
    return F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);
}

//==============================================================================

float rand(vec2 co) {
    return fract(sin(dot(co.xy, vec2(12.9898, 78.233))) * 43758.5453);
}

float calculateRandPCF(float currentDepth, vec2 uv, int image)
{
    float shadow = 0.f;
    
    int steps = 2;
    vec2 texelSize = 1.0 / textureSize(shadowStorage[image], 0);
    float bias = 0.00009f;
    for(int x = -steps; x <= steps; ++x)
    {
       for(int y = -steps; y <= steps; ++y)
       {
            vec2 randomOffset = vec2(rand(uv + vec2(x, y)), rand(uv - vec2(x, y))) * texelSize;

            float pcfDepth = texture(shadowStorage[image], uv + vec2(float(x)/steps, float(y)/steps) * texelSize + randomOffset*4).r; 
           shadow += currentDepth - bias < pcfDepth ? 1.0 : 0.0;        
       }    
    }
    shadow /= (steps * 2) * (steps * 2);
   
    return shadow;
}

float ShadowCalculation(vec3 lightDir, vec3 normal, vec3 pos)
{
    //hell is here
    int image;
    if (gl_FragCoord.z <= 1.f) image = 0;
    else if (gl_FragCoord.z > 1.f && gl_FragCoord.z <= 5.f) image = 1;
    else if (gl_FragCoord.z > 5.f && gl_FragCoord.z <= 25.f) image = 2;
    else image = 3;

  // perform perspective divide
    vec3 projCoords = FragPosLightSpace[image].xyz / FragPosLightSpace[image].w;
    vec2 uv = projCoords.xy * 0.5 + 0.5;
    float currentDepth = projCoords.z ;
    //projCoords = projCoords * 0.5 + 0.5; 

    float shadow = calculateRandPCF(currentDepth, uv, image);
    //float shadow = PCSS_DirectionalLight(vec3(uv, projCoords.z), 1.5f, pos);

    return clamp(shadow, 0.f, 1.f);
}

//==============================================================================

vec3 calculateSunLight(DirectionalLight sun, vec3 surfaceNormal, vec2 UVs, vec3 viewDirection, vec3 F0, vec3 cameraPosWorld)
{
    vec3 directionToLight = sun.direction;
    directionToLight = normalize(-directionToLight);
    float shadow = ShadowCalculation(directionToLight, surfaceNormal, cameraPosWorld);

    if (shadow == 0) return vec3(0.f);

    vec3 intensity = sun.color.xyz * sun.color.w;
    vec3 halfAngle = normalize(directionToLight + viewDirection);

    vec3 fres = fresnelSchlick(clamp(dot(halfAngle, viewDirection), 0.f, 1.f), F0);
 
    float diff = GeometrySmith(surfaceNormal, viewDirection, directionToLight ,texture(storageSampler[push.RID[1]], UVs).r * push.modifiers[0]);

    float specular = DistributionGGX(surfaceNormal, halfAngle, clamp(texture(storageSampler[push.RID[1]], UVs).x, 0.001f, 1.f) * push.modifiers[1]);

    vec3 numerator = specular * diff * fres;
    float denominator = 4.0 * max(dot(surfaceNormal, viewDirection), 0.0) * max(dot(surfaceNormal, directionToLight), 0.0) + 0.0001;
    vec3 spec = numerator / denominator;

    vec3 kD = metallic(fres, texture(storageSampler[push.RID[5]], UVs).r * push.modifiers[3]);

    float NdotL = max(dot(surfaceNormal, directionToLight), 0.f);
   

    return (shadow) * (kD * texture(storageSampler[push.RID[0]], UVs).rgb / M_PI + spec) * intensity * NdotL;
}

vec3 calculateLights(vec3 surfaceNormal, vec2 UVs, vec3 viewDirection, vec3 F0)
{
    vec3 Lo = vec3(0.f);

    for(int i = 0; i < ubo.numLights; i++)
    {
        PointLight light = ubo.pointLights[i];
		
        vec3 directionToLight = light.position.xyz - fragPosWorld;
		float attenuation = 1.0/dot(directionToLight, directionToLight); //distance squared
		
        directionToLight = normalize(directionToLight);

        vec3 intensity = light.color.xyz * light.color.w * attenuation;
        vec3 halfAngle = normalize(directionToLight + viewDirection); 

        vec3 fres = fresnelSchlick(clamp(dot(halfAngle, viewDirection), 0.f, 1.f), F0);
        
        float diff = GeometrySmith(surfaceNormal, viewDirection, directionToLight ,texture(storageSampler[push.RID[1]], UVs).r * push.modifiers[0]);

        /*
        vec3 diff = BurleyDiffuse(
              dot(directionToLight, surfaceNormal),
              dot(viewDirection, surfaceNormal),
              dot(halfAngle, directionToLight), 
              UVs);
        
        //diffcont += diff;
        */

        float specular = DistributionGGX(surfaceNormal, halfAngle, clamp(texture(storageSampler[push.RID[1]], UVs).x, 0.001f, 1.f) * push.modifiers[1]);

        vec3 numerator = specular * diff * fres;
        float denominator = 4.0 * max(dot(surfaceNormal, viewDirection), 0.0) * max(dot(surfaceNormal, directionToLight), 0.0) + 0.0001;
        vec3 spec = numerator / denominator;

        vec3 kD = metallic(fres, texture(storageSampler[push.RID[5]], UVs).r * push.modifiers[3]);

        float NdotL = max(dot(surfaceNormal, directionToLight), 0.f);

        Lo += (kD * texture(storageSampler[push.RID[0]], UVs).rgb / M_PI + spec) * intensity * NdotL;
    }

    return Lo;
}

vec3 calculateDiffuse(vec3 fragNormal, vec3 surfaceNormal, vec2 UVs, vec3 viewDirection, vec3 F0)
{
    vec3 directionToLight = vec3(0.f, -1.f, 0.f);
    vec3 intensity = (ubo.ambientLightColor.xyz * vec3(0.8, 0.8f, 1.2f)) * ubo.ambientLightColor.w * 80;

    vec3 halfAngle = normalize(directionToLight + viewDirection);

    vec3 fres = fresnelSchlick(clamp(dot(halfAngle, viewDirection), 0.f, 1.f), F0);
 
    float diff = GeometrySmith(surfaceNormal, viewDirection, directionToLight ,texture(storageSampler[push.RID[1]], UVs).r * push.modifiers[0]);

    float specular = 1.f;//DistributionGGX(surfaceNormal, halfAngle, clamp(texture(specular, UVs).x, 0.001f, 1.f));

    vec3 numerator = specular * diff * fres;
    float denominator = 4.0 * max(dot(surfaceNormal, viewDirection), 0.0) * max(dot(surfaceNormal, directionToLight), 0.0) + 0.0001;
    vec3 spec = numerator / denominator;

    vec3 kD = metallic(fres, texture(storageSampler[push.RID[5]], UVs).r * push.modifiers[3]);

    float NdotL = max(dot(surfaceNormal, directionToLight), 0.f);

    return (kD * texture(storageSampler[push.RID[0]], UVs).rgb / M_PI + spec) * intensity * NdotL;
}

float LinearizeDepth(float depth) 
{
  return NEAR * FAR / (FAR + depth * (NEAR - FAR));	
}

void main()
{
    vec2 projCoords = vec2(gl_FragCoord.x/ubo.width, gl_FragCoord.y/ubo.height);
    float currDepth = gl_FragCoord.z;

    float prePassDepth = texture(depthMap, projCoords).r;
    
    if (prePassDepth < currDepth) discard;

	vec3 cameraPosWorld = ubo.invView[3].xyz;
	vec3 viewDirection = normalize(cameraPosWorld - fragPosWorld); 

    vec2 UVs = fragUv;
    mat3 TBN = cotangent_frame(normalize(fragNormalWorld), viewDirection, UVs);
	
    vec3 diffuseLight = ubo.ambientLightColor.xyz * ubo.ambientLightColor.w;
	vec3 specularLight = vec3(0.0);

    DirectionalLight sun;
    sun.direction = ubo.lightPos;
    sun.color = vec4(1.f, 1.f, 0.7f, 1.5f);

    //vec2 boxuv = SampleSphericalMap(normalize(viewDirection));
    //vec3 boxcolor = texture(fakebox, boxuv).rgb;

	//UVs = parallaxOcclusionMapping(UVs, TBN * viewDirection);
    
	vec3 tangentNormal = texture(storageSampler[push.RID[2]], UVs).xyz * 255.f/127.f - 128.f/127.f;
	//vec3 surfaceNormal = texture(normalSpec, projCoords).rgb;
    vec3 surfaceNormal = normalize(TBN * tangentNormal);
    //vec3 surfaceNormal = normalize(fragNormalWorld);

    vec3 diffcont = vec3(0.f);
    //float diffcont = 0.f;

    vec3 F0 = vec3(0.04);
    float halfView = dot(normalize(viewDirection + surfaceNormal), surfaceNormal); 
    F0 = mix(F0, texture(storageSampler[push.RID[0]], UVs).rgb, texture(storageSampler[push.RID[5]], UVs).r * push.modifiers[3]);

    vec3 Lo = vec3(0.f);

    //vec3 diffuseLight = vec3(0.f);//vec3(0.02f, 0.01f, 0.08f);
    Lo += calculateSunLight(sun, surfaceNormal, UVs, viewDirection, F0, cameraPosWorld);
    Lo += calculateLights(surfaceNormal, UVs, viewDirection, F0);

    //https://www.youtube.com/watch?v=BFld4EBO2RE great video!
    vec3 lambda = exp(-0.0005f * currDepth * vec3(.5f, 1.f, 4.f));

    vec4 diffuse = texture(storageSampler[push.RID[0]], UVs) * (vec4(diffuseLight, 0.0) * texture(storageSampler[push.RID[4]], UVs).r * push.modifiers[2] + 
                    vec4(0.01f, 0.01f, 0.02f, 0.f));

    diffuse = vec4(lambda, 0.f) * diffuse + vec4((1 - lambda), 0.f) * vec4(0.1f, 0.1f, 0.1f, 0.f);

    outColor = diffuse + vec4(Lo, 0.f);
    //outColor = vec4(texture(storageSampler[push.RID[push.RIDo]], fragUv).rgb, 1.f);
}
