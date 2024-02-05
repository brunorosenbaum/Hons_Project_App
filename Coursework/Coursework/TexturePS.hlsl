// Texture pixel/fragment shader
// Basic fragment shader for rendering textured geometry
#include "LightingCalcs.hlsli"


// Texture and sampler registers
Texture2D texture0 : register(t0);
SamplerState Sampler0 : register(s0);


cbuffer LightBuffer : register(b0)
{
    float4 ambient;
    float4 diffuse[2];
    float4 position[2];
    float3 direction;

};

struct InputType
{
    float4 position : SV_POSITION;
    float2 tex : TEXCOORD0;
    float3 normal : NORMAL;
    float3 worldPosition : TEXCOORD1;

};


float4 main(InputType input) : SV_TARGET
{
	float4 finalColor = calculateDirectionalLighting(direction, input.normal, diffuse[0], ambient);
    float4 pointColor = calculatePositionLighting(input.worldPosition, position[1], input.normal, diffuse[1], ambient);
    finalColor += pointColor;
	// Sample the pixel color from the texture using the sampler at this texture coordinate location.
    finalColor *= texture0.Sample(Sampler0, input.tex);
    return  finalColor; 
}