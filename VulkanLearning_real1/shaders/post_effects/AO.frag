#version 450

#define PI 3.1415926535897932384626433832795
#define PI_HALF 1.5707963267948966192313216916398


layout(location = 0) in vec2 texCoords;

layout(location = 0) out vec4 outColor;

layout(set = 0, binding = 2) uniform sampler2D normalSpec;

float GTAOFastAcos(float x)
{
    float res = -0.156583 * abs(x) + PI_HALF;
    res *= sqrt(1.0 - abs(x));
    return x >= 0 ? res : PI - res;
}
 
float IntegrateArc(float h1, float h2, float n)
{
    float cosN = cos(n);
    float sinN = sin(n);
    return 0.25 * (-cos(2.0 * h1 - n) + cosN + 2.0 * h1 * sinN - cos(2.0 * h2 - n) + cosN + 2.0 * h2 * sinN);
}

void main()
{   
    //discard;
    outColor = vec4(texture(normalSpec, texCoords).r);
}


