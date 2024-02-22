#include "LightingCalcs.hlsli"
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
    
    float4 directionalColor = calculateDirectionalLighting(direction, input.normal, diffuse[0], ambient);
    float4 pointColor = calculatePositionLighting(input.worldPosition, position[1], input.normal, diffuse[1], ambient); 
    float4 finalColor = directionalColor + pointColor; 
  return  finalColor; 
}