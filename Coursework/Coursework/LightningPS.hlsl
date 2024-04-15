
struct InputType
{
    float4 position : SV_POSITION;
    float2 tex : TEXCOORD0;
    float3 normal : NORMAL;
    //float3 worldPosition : TEXCOORD1;

};

float4 main() : SV_TARGET
{
    
	return float4(0.808f, 1.f, 0.f, 1.0f);

}