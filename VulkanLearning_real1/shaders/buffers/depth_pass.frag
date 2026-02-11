#version 450
#define near 0.1
#define far 30.f

layout (location = 0) out vec4 outColor;

float LinearizeDepth(float depth) 
{
    float z = depth * 2.0 - 1.0; // back to NDC 
    return (2.0 * near * far) / (far + near - z * (far - near));	
}

void main()
{             
    float depth = LinearizeDepth(gl_FragCoord.z);
    outColor = vec4(vec3(depth), 1.0);
}
