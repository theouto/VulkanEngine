#version 450

layout (location = 0) in vec3 fragColor;
layout (location = 1) in vec3 fragPosWorld;
layout (location = 2) in vec3 fragNormalWorld;
layout (location = 3) in vec2 fragUv;
//layout (location = 4) in mat3 TBN;

layout (location = 0) out vec4 outColor;

layout(set = 1, binding = 1) uniform sampler2D texSampler;
layout(set = 1, binding = 2) uniform sampler2D specular;
layout(set = 1, binding = 3) uniform sampler2D normals;
layout(set = 1, binding = 4) uniform sampler2D displacement;
layout(set = 1, binding = 5) uniform sampler2D AO;
//layout(set = 1, binding = 6) uniform sampler2D roughness;

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

/* divider for cleanliness */

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

/* divider for cleanliness */

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

/* divider for cleanliness */
vec3 BurleyDiffuse(float lightAng, float viewAng, float halfAng, vec2 UVs)
{
  float f90 = 0.5f + 2.f * texture(specular, UVs).r * pow(cos(halfAng), 2);

  return texture(texSampler, UVs).xyz * (1/M_PI) * (1 + (f90-1) * pow((1-cos(lightAng)), 5))*(1 + (f90 - 1) * pow((1 - cos(viewAng)), 5));
}



vec2 parallaxOcclusionMapping(vec2 texCoords, vec3 viewDir)
{	
    const float height_scale = 0.07f;
    // number of depth layers
    const float numLayers = 64;
    // calculate the size of each layer
    float layerDepth = 1.0 / numLayers;
    // depth of current layer
    float currentLayerDepth = 0.0;
    // the amount to shift the texture coordinates per layer (from vector P)
    vec2 P = vec2(-1.0f * viewDir.y, viewDir.z) * height_scale; 
    vec2 deltaTexCoords = P / numLayers;


    // get initial values
    vec2  currentTexCoords     = texCoords;
    float currentDepthMapValue = 1.0f - texture(displacement, currentTexCoords).r;
  
    while(currentLayerDepth < currentDepthMapValue)
    {
      // shift texture coordinates along direction of P
      currentTexCoords -= deltaTexCoords;
      // get depthmap value at current texture coordinates
      currentDepthMapValue = 1.0f - texture(displacement, currentTexCoords).r;  
      // get depth of next layer
      currentLayerDepth += layerDepth;  
    }
	
    
    // get texture coordinates before collision (reverse operations)
    vec2 prevTexCoords = currentTexCoords + deltaTexCoords;

    // get depth after and before collision for linear interpolation
    float afterDepth  = currentDepthMapValue - currentLayerDepth;
    float beforeDepth = ( 1.0f - texture(displacement, currentTexCoords).r) - currentLayerDepth + layerDepth;
 
    // interpolation of texture coordinates
    float weight = afterDepth / (afterDepth - beforeDepth);
    vec2 finalTexCoords = prevTexCoords * weight + currentTexCoords * (1.0 - weight);

	return currentTexCoords; 
}

/* divider for cleanliness */

void main()
{
    mat3 TBN = cotangent_frame(fragNormalWorld, fragPosWorld, fragUv);
	vec3 diffuseLight = ubo.ambientLightColor.xyz * ubo.ambientLightColor.w;
	vec3 specularLight = vec3(0.0);

	//vec3 fresnel = vec3(0.0);
	
	vec3 cameraPosWorld = ubo.invView[3].xyz;
	vec3 viewDirection = normalize(cameraPosWorld - fragPosWorld); 

	vec2 UVs = parallaxOcclusionMapping(fragUv, TBN * viewDirection);
	//vec2 UVs = fragUv;
    //if (UVs.x > 1.0 || UVs.y > 1.0 || UVs.x < 0.0 || UVs.y < 0.0) {discard;}

	vec3 tangentNormal = texture(normals, UVs).rgb * 2.0 - 1.0;     
	vec3 surfaceNormal = normalize(normalize(TBN * tangentNormal));
	
	//vec3 surfaceNormal = normalize(fragNormalWorld);
	

	for (int i = 0; i < ubo.numLights; i++)
	{
		//diffuse
        
		PointLight light = ubo.pointLights[i];
		vec3 directionToLight = light.position.xyz - fragPosWorld;
		float attenuation = 1.0/dot(directionToLight, directionToLight); //distance squared
		directionToLight = normalize(directionToLight);

		float cosAngIncidence = max(dot(surfaceNormal, directionToLight), 0);
	    vec3 intensity = light.color.xyz * light.color.w * attenuation;

		//diffuseLight += intensity * cosAngIncidence;
        

		//specular
		vec3 halfAngle = normalize(directionToLight + viewDirection);
	
        
        diffuseLight += intensity * BurleyDiffuse(
              acos(dot(directionToLight, surfaceNormal)),
              acos(dot(viewDirection, surfaceNormal)),
              acos(dot(halfAngle, directionToLight)), 
              UVs);
        
		
        specularLight += intensity * DistributionGGX(surfaceNormal, halfAngle, texture(specular, UVs).x);
	}


    //DIFFUSE VIEW
    //outColor = vec4(diffuseLight, 0.0);
    
    //AO VIEW
    //outColor = vec4(1.f, 1.f, 1.f, 1.f) * texture(AO, UVs).r;
    
    //NORMAL VIEW
    //outColor = texture(normals, UVs);

    //SPECULAR VIEW
    outColor = texture(texSampler, UVs) * vec4(specularLight, 0.0f);

    //FINAL VIEW
    //outColor = texture(texSampler, UVs) * vec4(diffuseLight, 0.0) * texture(AO, UVs).r +  texture(texSampler, UVs) * vec4(specularLight, 0.0f);
}
