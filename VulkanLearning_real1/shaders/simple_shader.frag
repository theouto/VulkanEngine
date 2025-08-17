#version 450

layout (location = 0) in vec3 fragColor;
layout (location = 1) in vec3 fragPosWorld;
layout (location = 2) in vec3 fragNormalWorld;
layout (location = 3) in vec2 fragUv;

layout (location = 0) out vec4 outColor;

layout(binding = 1) uniform sampler2D texSampler;
layout(binding = 2) uniform sampler2D specular;
layout(binding = 3) uniform sampler2D normal;
layout(binding = 4) uniform sampler2D displacement;
//layout(binding = 5) uniform sampler2D metalness;

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

//GGX - https://learnopengl.com/PBR/Theory
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

float GeometrySchlickGGX(float NdotV, float k)
{
    float nom   = NdotV;
    float denom = NdotV * (1.0 - k) + k;
	
    return nom / denom;
}
  
float GeometrySmith(vec3 N, vec3 V, vec3 L, float k)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx1 = GeometrySchlickGGX(NdotV, k);
    float ggx2 = GeometrySchlickGGX(NdotL, k);
	
    return ggx1 * ggx2;
}

//vec3 fresnelSchlick(float cosTheta, vec3 F0)
//{
//    return F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);
//}

vec2 parallaxOcclusionMapping(vec2 uv, vec3 viewDirection)
{
	
	//Parallax occlusion mapping quality
	float heightScale = 0.05f;
	const float minLayers = 8.0f;
	const float maxLayers = 64.0f;
	float numLayers = 64.0f; //mix(maxLayers, minLayers, abs(dot(normalize(fragNormalWorld), viewDirection)));
	float layerDepth = 1.0f / numLayers;
	float currentLayerDepth = 0.0f;
	
	vec2 S = vec2(viewDirection.z, -viewDirection.y) * heightScale;
	vec2 deltaUVs = S / numLayers;
	
	vec2 UVs = uv;
	float currentDepthMapValue = 1.0f - texture(displacement, UVs).r;
	
	while(currentLayerDepth < currentDepthMapValue)
	{
		UVs -= deltaUVs;
		currentDepthMapValue = 1.0f - texture(displacement, UVs).r;
		currentLayerDepth += layerDepth;
	}
	
	vec2 prevTexCoords = UVs + deltaUVs;
	float afterDepth  = currentDepthMapValue - currentLayerDepth;
	float beforeDepth = 1.0f - texture(displacement, prevTexCoords).r - currentLayerDepth + layerDepth;
	float weight = afterDepth / (afterDepth / beforeDepth);
	UVs = prevTexCoords * weight + UVs * (1.0f - weight);

	return UVs;

}

void main()
{
	vec3 diffuseLight = ubo.ambientLightColor.xyz * ubo.ambientLightColor.w;
	vec3 specularLight = vec3(0.0);

	//https://github.com/h3ck0r/VKEngine/blob/main/Engine/shaders/shader.frag
	vec3 T = normalize(mat3(push.normalMatrix) * vec3(1.0, 0.0, 0.0));
    vec3 B = normalize(mat3(push.normalMatrix) * vec3(0.0, 1.0, 0.0));
    vec3 N = normalize(mat3(push.normalMatrix) * fragNormalWorld);
    mat3 TBN = mat3(T, B, N); 

	//vec3 fresnel = vec3(0.0);
	
	vec3 cameraPosWorld = ubo.invView[3].xyz;
	vec3 viewDirection = normalize(cameraPosWorld - fragPosWorld);

	//vec2 UVs = parallaxOcclusionMapping(fragUv, viewDirection);
	vec2 UVs = fragUv;

	//if (UVs.x > 1.0 || UVs.y > 1.0 || UVs.x < 0.0 || UVs.y < 0.0) {discard;}

	vec3 tangentNormal = texture(normal, UVs).rgb * 2.0 - 1.0;     
	vec3 surfaceNormal = normalize(normalize(TBN * tangentNormal));
			
	//vec3 surfaceNormal = normalize(fragNormalWorld);
	

	for (int i = 0; i < ubo.numLights; i++)
	{
		//diffuse
		PointLight light = ubo.pointLights[i];
		vec3 directionToLight = light.position.xyz - fragPosWorld;
		
		//float attenuation = GeometrySmith(surfaceNormal, viewDirection, light.color.xyz, 0.0f);

		//FresnelSchlick
		//float cosTheta = dot(directionToLight, surfaceNormal);
		//vec3 F0 = vec3(0.04);
		//F0 = mix(F0, texture(texSampler, UVs).rgb, metalness);
		//fresnel = fresnelSchlick(cosTheta, F0);

		float attenuation = 1.0/dot(directionToLight, directionToLight); //distance squared
		directionToLight = normalize(directionToLight);

		float cosAngIncidence = max(dot(surfaceNormal, directionToLight), 0);
		vec3 intensity = light.color.xyz * light.color.w * attenuation;

		diffuseLight += intensity * cosAngIncidence;

		//specular
		vec3 halfAngle = normalize(directionToLight + viewDirection);
		//float blinnTerm = dot(surfaceNormal, halfAngle);
		//blinnTerm = clamp(blinnTerm, 0, 1);
		//blinnTerm = pow(blinnTerm, texture(specular, fragUv).x * 80.0); //higher == sharper
		//specularLight += intensity * blinnTerm;

		//GGX - https://learnopengl.com/PBR/Theory
		
		specularLight += intensity * DistributionGGX(surfaceNormal, halfAngle, texture(specular, UVs).x);
	}
	
	//outColor = vec4(surfaceNormal, 1.0);
	outColor = texture(texSampler, UVs) * vec4(diffuseLight, 0.0) 
		+  texture(texSampler, UVs) * vec4(specularLight, 0.0f);
}