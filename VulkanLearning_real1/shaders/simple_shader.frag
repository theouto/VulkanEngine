#version 450
#pragma glslify: ggx = require('glsl-ggx')

layout (location = 0) in vec3 fragColor;
layout (location = 1) in vec3 fragPosWorld;
layout (location = 2) in vec3 fragNormalWorld;
layout (location = 3) in vec2 fragUv;

layout (location = 0) out vec4 outColor;

layout(binding = 1) uniform sampler2D texSampler;
layout(binding = 2) uniform sampler2D specular;
layout(binding = 3) uniform sampler2D normal;

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

void main()
{
	vec3 diffuseLight = ubo.ambientLightColor.xyz * ubo.ambientLightColor.w;
	vec3 specularLight = vec3(0.0);
	vec3 surfaceNormal = normalize(fragNormalWorld);
	//vec3 surfaceNormal = normalize(texture(normal, fragUv).xyz * 2.0f - 1.0f) * normalize(fragNormalWorld);

	vec3 cameraPosWorld = ubo.invView[3].xyz;
	vec3 viewDirection = normalize(cameraPosWorld - fragPosWorld);

	for (int i = 0; i < ubo.numLights; i++)
	{
		//diffuse
		PointLight light = ubo.pointLights[i];
		vec3 directionToLight = light.position.xyz - fragPosWorld;
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
		
		specularLight += intensity * DistributionGGX(surfaceNormal, halfAngle, texture(specular, fragUv).x);
	}
	
	//outColor = vec4(diffuseLight * fragColor + specularLight * fragColor, 1.0);
	outColor = texture(texSampler, fragUv) * vec4(diffuseLight, 0.0) 
		+ texture(specular, fragUv).x * vec4(specularLight, 0.0f);
}