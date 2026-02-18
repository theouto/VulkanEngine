#version 450

#define PI 3.1415926535897932384626433832795
#define PI_HALF 1.5707963267948966192313216916398
#define near 0.1
#define far 30.f

layout(location = 0) in vec2 texCoords;

layout(location = 0) out vec4 outColor;

layout(set = 0, binding = 2) uniform sampler2D depthBuffer;
layout(set = 0, binding = 3) uniform sampler2D normalSpec;

layout(set = 0, binding = 0) uniform GlobalUbo 
{
  mat4 projection;
  mat4 view;
  mat4 invView;
  mat4 viewStat;
} ubo;

float LinearizeDepth(float depth) 
{
  return near * far / (far + depth * (near - far));	
}

/*
float FastAcos2(vec2 x)
{
    return (-0.69813170*x*x - 0.87266463)*x + 1.57079633;
}


float integrate_arc(vec2 h, float n, float cos_n) {
	vec2 tmp = cos_n + 2.0 * h * sin(n) - cos(2.0 * h - n);
	return 0.25 * (tmp.x + tmp.y);
}

vec3 screen_to_view_space(mat4 projection_matrix_inverse, vec3 screen_pos, bool handle_jitter) {
	vec3 ndc_pos = 2.0 * screen_pos - 1.0;
	return project_and_divide(projection_matrix_inverse, ndc_pos);
}

vec2 clamp01(vec2 a)
{
  return vec2(clamp(a.x, 0.f, 1.f), clamp(a.y, 0.f, 1.f));
}

vec2 traceSlice(vec2 xy, vec3 viewV, )


#define RADIUS 2.f
#define FALLOFF_START 1.5f
#define SLICES 4
#define HORIZON_STEPS 6
#define THICKNESS 0.4f

//https://github.com/Zenteon/ZenteonFX/blob/main/Shaders/Zenteon_SSAO_History.fx
void main() 
{
  
  vec2 AOacc = vec2(0.f);

  

  return vec4(vec3(clamp01(AOacc.x/AOacc.y)), 1.f)
  

  vec3 samplePos = vec3(texCoords, texture(depthBuffer, texCoords).r);

  vec2 texelSize = 1.0 / vec2(textureSize(depthBuffer, 0));
    float result = 0.0;
    for (int x = -2; x < 2; ++x) 
    {
        for (int y = -2; y < 2; ++y) 
        {
            vec2 offset = vec2(float(x), float(y)) * texelSize;
            result += texture(depthBuffer, texCoords + offset).r;
        }
    }

    outColor = vec4(vec3(LinearizeDepth(texture(depthBuffer, texCoords).r))/30.f, 1.f);
}

*/

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

    vec3 Visualize_0_3(float x)
    {
        const vec3 color0 = vec3(1.0, 0.0, 0.0);
        const vec3 color1 = vec3(1.0, 1.0, 0.0);
        const vec3 color2 = vec3(0.0, 1.0, 0.0);
        const vec3 color3 = vec3(0.0, 1.0, 1.0);
        vec3 color = mix(color0, color1, clamp(x - 0.0, 0.0, 1.0));
        color = mix(color, color2, clamp(x - 1.0, 0.0, 1.0));
        color = mix(color, color3, clamp(x - 2.0, 0.0, 1.0));
        return color;
    }
     
    vec3 GetCameraVec(vec2 uv)
    {   
        // Returns the vector from camera to the specified position on the camera plane (uv argument), located one unit away from the camera
        // This vector is not normalized.
        // The nice thing about this setup is that the returned vector from this function can be simply multiplied with the linear depth to get pixel's position relative to camera position.
        // This particular function does not account for camera rotation or position or FOV at all (since we don't need it for AO)
        // TODO: AO is dependent on FOV, this function is not!
        // The outcome of using this simplified function is that the effective AO range is larger when using larger FOV
        // Use something more accurate to get proper FOV-independent world-space range, however you will likely also have to adjust the SSAO constants below
        return vec3(uv.x * -2.0 + 1.0, uv.y * 2.0 * 16/9.f - 16/9.f, 1.0);
    }
     
    

float random(vec2 st)
{
    return fract(sin(dot(st.xy, vec2(12.9898,78.233))) * 43758.5453123);
}

#define angleOffset 0.f
#define spacialOffset 0.f

    #define SSAO_LIMIT 30
    #define SSAO_SAMPLES 16
    #define SSAO_RADIUS 2.5
    #define SSAO_FALLOFF 1.5
    #define SSAO_THICKNESSMIX 1
    #define SSAO_MAX_STRIDE 16
     
    void SliceSample(vec2 tc_base, vec2 aoDir, int i, float targetMip, vec3 ray, vec3 v, inout float closest)
    {
        vec2 uv = tc_base + aoDir * i;
        float depth = textureLod(depthBuffer, uv, targetMip).x;
        // Vector from current pixel to current slice sample
        vec3 p = GetCameraVec(uv) * depth - ray;
        // Cosine of the horizon angle of the current sample
        float current = dot(v, normalize(p));
        // Linear falloff for samples that are too far away from current pixel
        float falloff = clamp((SSAO_RADIUS - length(p)) / SSAO_FALLOFF, 0.0, 1.0);
        if(current > closest)
            closest = mix(closest, current, falloff);
        // Helps avoid overdarkening from thin objects
        closest = mix(closest, current, SSAO_THICKNESSMIX * falloff);
    }
    //https://forum.derivative.ca/t/implementing-different-ao-algo-gtao-with-glsl/207841/3
    void main()
    {   
        //outColor = vec4(texture(normalSpec, texCoords).rgb, 1.f);
        //return;
        discard;
        vec2 tc_original = texCoords;
        vec2 viewsizediv = vec2(1.0 / 1920, 1.0 / 1080);

        // Depth of the current pixel
        float dhere = LinearizeDepth(texture(depthBuffer, texCoords).r);
        // Vector from camera to the current pixel's position
        vec3 ray = GetCameraVec(tc_original) * dhere;
        
        const float normalSampleDist = 1.0;
        
        // Calculate normal from the 4 neighbourhood pixels
        vec2 uv = tc_original + vec2(viewsizediv.x * normalSampleDist, 0.0);
        vec3 p1 = ray - GetCameraVec(uv) * LinearizeDepth(texture(depthBuffer, uv).r);

        uv = tc_original + vec2(0.0, viewsizediv.y * normalSampleDist);
        vec3 p2 = ray - GetCameraVec(uv) * LinearizeDepth(texture(depthBuffer, uv).r);
        
        uv = tc_original + vec2(-viewsizediv.x * normalSampleDist, 0.0);
        vec3 p3 = ray - GetCameraVec(uv) * LinearizeDepth(texture(depthBuffer, uv).r);
        
        uv = tc_original + vec2(0.0, -viewsizediv.y * normalSampleDist);
        vec3 p4 = ray - GetCameraVec(uv) * LinearizeDepth(texture(depthBuffer, uv).r);
        
        vec3 normal1 = normalize(cross(p1, p2));
        vec3 normal2 = normalize(cross(p3, p4));
        
        vec3 normal = normalize(normal1 + normal2);
        
        // Calculate the distance between samples (direction vector scale) so that the world space AO radius remains constant but also clamp to avoid cache trashing
                float stride = min((1.0 / length(ray)) * SSAO_LIMIT, SSAO_MAX_STRIDE);
        vec2 dirMult = viewsizediv.xy * stride;
        // Get the view vector (normalized vector from pixel to camera)
        vec3 v = normalize(-ray);
        
        // Calculate slice direction from pixel's position
        float dirAngle = (PI / 16.0) * (((int(gl_FragCoord.x) + int(gl_FragCoord.y) & 3) << 2) + (int(gl_FragCoord.x) & 3)) + 
                          random(vec2(texCoords.x * cos(34534.12313), texCoords.y * 12391.11));
        vec2 aoDir = dirMult * vec2(sin(dirAngle), cos(dirAngle));
        
        // Project world space normal to the slice plane
        vec3 toDir = GetCameraVec(tc_original + aoDir);
        vec3 planeNormal = normalize(cross(v, -toDir));
        vec3 projectedNormal = normal - planeNormal * dot(normal, planeNormal);
        
        // Calculate angle n between view vector and projected normal vector
        vec3 projectedDir = normalize(normalize(toDir) + v);
        float n = GTAOFastAcos(dot(-projectedDir, normalize(projectedNormal))) - PI_HALF;
        
        // Init variables
        float c1 = -1.0;
        float c2 = -1.0;
        
        vec2 tc_base = tc_original + aoDir * (0.25 * ((int(gl_FragCoord.y) - int(gl_FragCoord.x)) & 3) - 0.375 + 0.f);
        
        const float minMip = 0.0;
        const float maxMip = 3.0;
        const float mipScale = 1.0 / 12.0;
        
        float targetMip = floor(clamp(pow(stride, 1.3) * mipScale, minMip, maxMip));
        
        // Find horizons of the slice
        for(int i = -1; i >= -SSAO_SAMPLES; i--)
        {
            SliceSample(tc_base, aoDir, i, targetMip, ray, v, c1);
        }
        for(int i = 1; i <= SSAO_SAMPLES; i++)
        {
            SliceSample(tc_base, aoDir, i, targetMip, ray, v, c2);
        }
        
        // Finalize
        float h1a = -GTAOFastAcos(c1);
        float h2a = GTAOFastAcos(c2);
        
        // Clamp horizons to the normal hemisphere
        float h1 = n + max(h1a - n, -PI_HALF);
        float h2 = n + min(h2a - n, PI_HALF);
        
        float visibility = mix(1.0, IntegrateArc(h1, h2, n), length(projectedNormal));
        
        outColor = vec4(vec3(visibility), 0.9f);
    }
