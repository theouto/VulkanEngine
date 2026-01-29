/*

        Copyright 2024 Etay Meiri

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/


#version 450

layout (location=0) in vec3 Dir;
layout (location=1) in vec3 posi;

layout (location=0) out vec4 out_Color;

//layout(binding = 2) uniform sampler2D cubeSampler;

layout(set = 1, binding = 1) uniform sampler2D cubeSampler;

layout(set = 0, binding = 0) uniform GlobalUbo 
{
  mat4 projection;
  mat4 view;
  mat4 invView;
  mat4 viewStat;
  vec4 ambientLightColor; // w is intensity
  //PointLight pointLights[10];
  int numLights;
} ubo;

const float M_PI = 3.1415926538;
const vec2 invAtan = vec2(0.1591, 0.3183);

vec2 SampleSphericalMap(vec3 v)
{
    vec2 uv = vec2(atan(v.z, v.x), asin(v.y));
    uv *= invAtan;
    uv += 0.5;
    return uv;
}

void main() 
{
    vec3 cameraPosWorld = ubo.viewStat[3].xyz;
	vec3 transDir = normalize(cameraPosWorld - posi);

    vec2 uv = SampleSphericalMap(transDir);
    vec3 texcolor = texture(cubeSampler,(-1.f * uv)).rgb;

    //fun experiment for a more dynamic sky, very crude, however
    
    /*
    float cosy = cos(uv.y * 1.5f);
    float cosx = abs(cos(uv.x * M_PI));
    float termx = clamp(cosx, mix(cosx, 0.6f, 0.7f), 1.f);
    float termy = cosy * cosy * 5.f;
    float atmosterm = clamp(termy, mix(termy, 0.5f, 0.6f), 5.f);
    vec3 texcolor = vec3(0.1f * (4 * termx * termx), 0.2f * termx, 1.f) * atmosterm;
    */

	out_Color = vec4(texcolor, 1.0);
}
