#version 450

layout (location = 0) in vec3 fragColor;
layout (location = 1) in vec3 fragPosWorld;
layout (location = 2) in vec3 fragNormalWorld;
layout (location = 3) in vec2 fragUv;
layout (location = 4) in vec4 FragPosLightSpace;
layout (location = 5) in vec3 lightPos;
layout (location = 6) in mat4 lightSpaceMatrix;

layout (set = 1, location = 0) in sampler2D normals;

layout (location = 0) out vec4 outColor;

void main()
{
  mat3 TBN = cotangent_frame(fragNormalWorld, fragPosWorld, fragUv);
  vec3 tangentNormal = texture(normals, UVs).rgb * 2.0 - 1.0;     
  vec3 surfaceNormal = normalize(normalize(TBN * tangentNormal));

  outColor = vec4(surfaceNormal, 1.f);
}
