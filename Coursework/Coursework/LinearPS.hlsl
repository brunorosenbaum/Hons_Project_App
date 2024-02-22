cbuffer BoolBuffer : register(b0)
{
    bool isRed; 
};
struct InputType
{
    float4 position : SV_POSITION;
    float2 tex : TEXCOORD0;
    float3 normal : NORMAL;
    //float3 worldPosition : TEXCOORD1;

};

float4 main() : SV_TARGET
{
    //if(isRed)
    //{
    //    return float4(1.0f, 0.0f, 0.0f, 0.1f);

    //}
    //else
    //{
    //return float4(1.0f, 1.0f, 1.0f, 0.1f);

    //}
    return float4(1.0f, 1.0f, 1.0f, 0.1f);
}