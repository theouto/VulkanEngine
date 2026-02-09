#version 450

#define NEAR 0.01
#define numBlockerSearchSamples 1
#define numPCFSamples 1

layout (location = 0) in vec3 fragColor;
layout (location = 1) in vec3 fragPosWorld;
layout (location = 2) in vec3 fragNormalWorld;
layout (location = 3) in vec2 fragUv;
layout (location = 4) in vec4 FragPosLightSpace;
layout (location = 5) in vec3 lightPos;
layout (location = 6) in mat4 lightSpaceMatrix;

layout (location = 0) out vec4 outColor;

layout(set = 0, binding = 1) uniform sampler2D shadowMap;

layout(set = 1, binding = 1) uniform sampler2D texSampler;
layout(set = 1, binding = 2) uniform sampler2D specular;
layout(set = 1, binding = 3) uniform sampler2D normals;
layout(set = 1, binding = 4) uniform sampler2D displacement;
layout(set = 1, binding = 5) uniform sampler2D AO;
layout(set = 1, binding = 6) uniform sampler2D metalness;

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
} ubo;

layout(push_constant) uniform Push
{
	mat4 modelMatrix;
	mat4 normalMatrix;
} push;

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

vec2 parallaxOcclusionMapping(vec2 texCoords, vec3 viewDir)
{	
    const float height_scale = 0.03f;
    const float numLayers = 64;
    float layerDepth = 1.0 / numLayers;
    float currentLayerDepth = 0.0;
    vec2 P = vec2(-1.0f * viewDir.y, viewDir.z) * height_scale; 
    vec2 deltaTexCoords = P / numLayers;

    vec2  currentTexCoords     = texCoords;
    float currentDepthMapValue = 1.0f - texture(displacement, currentTexCoords).r;
  
    while(currentLayerDepth < currentDepthMapValue)
    {
      currentTexCoords -= deltaTexCoords;
      currentDepthMapValue = 1.0f - texture(displacement, currentTexCoords).r;
      currentLayerDepth += layerDepth;  
    }
	
    vec2 prevTexCoords = currentTexCoords + deltaTexCoords;

    float afterDepth  = currentDepthMapValue - currentLayerDepth;
    float beforeDepth = ( 1.0f - texture(displacement, currentTexCoords).r) - currentLayerDepth + layerDepth;
 
    float weight = afterDepth / (afterDepth - beforeDepth);
    vec2 finalTexCoords = prevTexCoords * weight + currentTexCoords * (1.0 - weight);

	return currentTexCoords; 
}

//==============================================================================

vec3 BurleyDiffuse(float lightAng, float viewAng, float halfAng, vec2 UVs)
{
  float f90 = 0.5f + 2.f * ( texture(specular, UVs).r) * halfAng * halfAng;
  float Fl = pow((1 - lightAng), 5);
  float Fv = pow((1 - viewAng), 5);

  float t1 = 1 + (1 - f90) * Fl;
  float t2 = 1 + (1 - f90) * Fv;

  return texture(texSampler, UVs).xyz * ((1/M_PI) * t2 * t1);
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


float ShadowCalculation(vec3 lightDir, vec3 normal)
{
  // perform perspective divide
    vec3 projCoords = FragPosLightSpace.xyz / FragPosLightSpace.w;
    vec2 uv = projCoords.xy * 0.5 + 0.5;
    float currentDepth = projCoords.z ;
    float depth = texture(shadowMap, uv).x; 
    float shadow = 0.0;
    float bias = max(0.00005 * (1.0 - dot(normal, lightDir)), 0.00005); 
    
    
    int steps = 3;
    vec2 texelSize = 1.0 / textureSize(shadowMap, 0);
    for(int x = -steps; x <= steps; ++x)
    {
       for(int y = -steps; y <= steps; ++y)
       {
            vec2 randomOffset = vec2(rand(uv + vec2(x, y)), rand(uv - vec2(x, y))) * texelSize;

            float pcfDepth = texture(shadowMap, uv + vec2(float(x)/steps, float(y)/steps) * texelSize + randomOffset).r; 
           shadow += currentDepth - bias < pcfDepth ? 1.0 : 0.0;        
       }    
    }
    shadow /= (steps * 2) * (steps * 2);
   
    shadow = sin(shadow);

    //shadow = currentDepth - bias > closestDepth  ? 1.0 : 0.0;

    return clamp(shadow, 0.f, 1.f);
}

//====== dividers are not enough to contain the bloat in this fucking shader file ======

float compute_pcss(float pcf_width){
    vec4 io_lcs_position = lightSpaceMatrix * FragPosLightSpace;
    vec3 uv = io_lcs_position.xyz / io_lcs_position.w;
    uv = uv * 0.5 + 0.5;
    float real_depth = uv.z;
    float result = 0;

    float bias = 0.f;

    int step_count = 27 / 2;
    float step_uv = pcf_width / step_count;
    for(float x=-step_count;x<=step_count;x++){
        for(float y=-step_count;y<=step_count;y++){
            vec2 offset = vec2(x, y) * step_uv;
            float depth = texture(shadowMap, uv.xy + offset).r + bias;
            if(depth > real_depth){
                result += 1.0;
            }else{
                result += 0.5;
            }
        }
    }
    return result / (27 * 27);
}

float compute_blocker_distance(float average_blocker_depth){
    return average_blocker_depth * (100.f - 0.01f) +0.01f;
}

float compute_penumbra_width(float blocker_distance){
    float lvs_distance = -FragPosLightSpace.z; //shaded point's distance from the light in light view space
    return 10.f * (lvs_distance - blocker_distance) / blocker_distance / 512;
}

float compute_pcf_width(float penumbra_width){
    vec4 io_lcs_position = lightSpaceMatrix * FragPosLightSpace;
    vec3 uv = io_lcs_position.xyz / io_lcs_position.w;  //conversion from clip space to NDC
    uv = uv * 0.5 + 0.5;                                //conversion from the (-1;1) interval to the (0;1)
    float real_depth = uv.z;
    return penumbra_width * 0.01f / real_depth;
}

float compute_average_blocker_depth(float search_region_width){
    
    vec4 io_lcs_position = lightSpaceMatrix * FragPosLightSpace;
    vec3 uv = io_lcs_position.xyz / io_lcs_position.w;  //conversion from clip space to NDC
    uv = uv * 0.5 + 0.5;                                //conversion from the (-1;1) interval to the (0;1)
    float real_depth = uv.z;
    int blocker_count = 0;
    float blocker_depth_sum = 0;

    int step_count = 27 / 2;
    float step_uv = search_region_width / step_count;
    for(float x=-step_count;x<=step_count;x++){
        for(float y=-step_count;y<=step_count;y++){
            vec2 offset = vec2(x, y) * step_uv;
            float depth = texture(shadowMap, uv.xy + offset).r;
            if(depth < real_depth){
                blocker_count++;
                blocker_depth_sum += depth;
            }
        }
    }
    if(blocker_count == 0){
        return -1.0;
    }else{
        return blocker_depth_sum / blocker_count;
    }
}

float compute_search_region_width(){
    float lvs_distance = -FragPosLightSpace.z;  //shaded point's distance from the light in light view space
    return 10.f * (lvs_distance - 0.01f) / lvs_distance / 512;
}

float compute_pcss_shadow(){
    float search_region_width = compute_search_region_width();
    float average_blocker_depth = compute_average_blocker_depth(search_region_width);
    if(average_blocker_depth == -1.0){
        return 1.0;
    }
    float blocker_distance = compute_blocker_distance(average_blocker_depth);
    float penumbra_width = compute_penumbra_width(blocker_distance);
    float pcf_width = compute_pcf_width(penumbra_width);
    return compute_pcss(pcf_width);
}

//==============================================================================

vec3 calculateSunLight(DirectionalLight sun, vec3 surfaceNormal, vec2 UVs, vec3 viewDirection, vec3 F0, vec3 cameraPosWorld)
{
    vec3 directionToLight = sun.direction;
    directionToLight = normalize(-directionToLight);

    vec3 intensity = sun.color.xyz * sun.color.w;
    vec3 halfAngle = normalize(directionToLight + viewDirection);

    vec3 fres = fresnelSchlick(clamp(dot(halfAngle, viewDirection), 0.f, 1.f), F0);
 
    float diff = GeometrySmith(surfaceNormal, viewDirection, directionToLight ,texture(specular, UVs).r);

    float specular = DistributionGGX(surfaceNormal, halfAngle, clamp(texture(specular, UVs).x, 0.001f, 1.f));

    vec3 numerator = specular * diff * fres;
    float denominator = 4.0 * max(dot(surfaceNormal, viewDirection), 0.0) * max(dot(surfaceNormal, directionToLight), 0.0) + 0.0001;
    vec3 spec = numerator / denominator;

    vec3 kD = metallic(fres, texture(metalness, UVs).r);

    float NdotL = max(dot(surfaceNormal, directionToLight), 0.f);
    //float shadow = compute_pcss_shadow();
    float shadow = ShadowCalculation(directionToLight, surfaceNormal);

    return (shadow) * (kD * texture(texSampler, UVs).rgb / M_PI + spec) * intensity * NdotL;
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
        
        float diff = GeometrySmith(surfaceNormal, viewDirection, directionToLight ,texture(specular, UVs).r);

        /*
        vec3 diff = BurleyDiffuse(
              dot(directionToLight, surfaceNormal),
              dot(viewDirection, surfaceNormal),
              dot(halfAngle, directionToLight), 
              UVs);
        
        diffcont += diff;
        */ 

        float specular = DistributionGGX(surfaceNormal, halfAngle, clamp(texture(specular, UVs).x, 0.001f, 1.f));
        
        vec3 numerator = specular * diff * fres;
        float denominator = 4.0 * max(dot(surfaceNormal, viewDirection), 0.0) * max(dot(surfaceNormal, directionToLight), 0.0) + 0.0001;
        vec3 spec = numerator / denominator;

        vec3 kD = metallic(fres, texture(metalness, UVs).r);

        float NdotL = max(dot(surfaceNormal, directionToLight), 0.f);

        Lo += (kD * texture(texSampler, UVs).rgb / M_PI + spec) * intensity * NdotL;
    }

    return Lo;
}

vec3 calculateDiffuse(vec3 fragNormal, vec3 surfaceNormal, vec2 UVs, vec3 viewDirection, vec3 F0)
{
    vec3 directionToLight = vec3(0.f, -1.f, 0.f);
    vec3 intensity = (ubo.ambientLightColor.xyz * vec3(0.8, 0.8f, 1.2f)) * ubo.ambientLightColor.w * 80;

    vec3 halfAngle = normalize(directionToLight + viewDirection);

    vec3 fres = fresnelSchlick(clamp(dot(halfAngle, viewDirection), 0.f, 1.f), F0);
 
    float diff = GeometrySmith(surfaceNormal, viewDirection, directionToLight ,texture(specular, UVs).r);

    float specular = 1.f;//DistributionGGX(surfaceNormal, halfAngle, clamp(texture(specular, UVs).x, 0.001f, 1.f));

    vec3 numerator = specular * diff * fres;
    float denominator = 4.0 * max(dot(surfaceNormal, viewDirection), 0.0) * max(dot(surfaceNormal, directionToLight), 0.0) + 0.0001;
    vec3 spec = numerator / denominator;

    vec3 kD = metallic(fres, texture(metalness, UVs).r);

    float NdotL = max(dot(surfaceNormal, directionToLight), 0.f);

    return (kD * texture(texSampler, UVs).rgb / M_PI + spec) * intensity * NdotL;
}

void main()
{ 
    mat3 TBN = cotangent_frame(fragNormalWorld, fragPosWorld, fragUv);
	//vec3 diffuseLight = ubo.ambientLightColor.xyz * ubo.ambientLightColor.w;
	vec3 specularLight = vec3(0.0);
	
	vec3 cameraPosWorld = ubo.invView[3].xyz;
	vec3 viewDirection = normalize(cameraPosWorld - fragPosWorld); 

    DirectionalLight sun;
    sun.direction = lightPos;
    sun.color = vec4(1.f, 1.f, 0.7f, 1.5f);

    //vec2 boxuv = SampleSphericalMap(normalize(viewDirection));
    //vec3 boxcolor = texture(fakebox, boxuv).rgb;

	vec2 UVs = parallaxOcclusionMapping(fragUv*4, TBN * viewDirection);
    //vec2 UVs = fragUv;

	vec3 tangentNormal = texture(normals, UVs).rgb * 2.0 - 1.0;     
	vec3 surfaceNormal = normalize(normalize(TBN * tangentNormal));

    vec3 diffcont = vec3(0.f);
    //float diffcont = 0.f;

    vec3 F0 = vec3(0.04);
    float halfView = dot(normalize(viewDirection + surfaceNormal), surfaceNormal); 
    F0 = mix(F0, texture(texSampler, UVs).rgb, texture(metalness, UVs).r);

    vec3 Lo = vec3(0.f);

    vec3 diffuseLight = calculateDiffuse(normalize(fragNormalWorld), surfaceNormal, UVs, viewDirection, F0);
    Lo += calculateSunLight(sun, surfaceNormal, UVs, viewDirection, F0, cameraPosWorld);
    Lo += calculateLights(surfaceNormal, UVs, viewDirection, F0);

    vec4 diffuse = texture(texSampler, UVs) * (vec4(diffuseLight, 0.0) + vec4(0.01f, 0.01f, 0.02f, 0.f)) * texture(AO, UVs).r;

    //Depth
    /*
    float depth = gl_FragCoord.z;//fragPosWorld.z - cameraPosWorld.z;
    depth = depth * 2.f - 1.f;
    depth = (2.f * 0.01f * 100.f) / (100.f + 0.1f - depth * (100.f - 0.01f));
    depth /= 100.f;
    outColor = vec4(vec3(depth), 1.f);
    */

    //debug view. Hey, might come in handy, it did in the past!
    //ENV VIEW
    //outColor = vec4(diffuseLight, 0.0);
 
    //DIFF-Cont VIEW
    //outColor = vec4(diffcont, 1.f);

    //AO VIEW
    //outColor = vec4(1.f, 1.f, 1.f, 1.f) * texture(AO, UVs).r;
 
    //NORMAL VIEW
    //outColor = texture(normals, UVs);

    //LIGHT-IMPACT VIEW
    //outColor = texture(texSampler, UVs) * vec4(specularLight, 0.0f);

    //SKYBOX VIEW
    //outColor = vec4(boxcolor, 1.f);

    //FINAL VIEW
    outColor = diffuse + vec4(Lo, 0.f);
    //outColor = vec4(vec3(texture(shadowMap, fragUv).r), 1.f);
}
