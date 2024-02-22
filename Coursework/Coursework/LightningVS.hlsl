
cbuffer MatrixBuffer : register(b0)
{
	matrix worldMatrix;
	matrix viewMatrix;
	matrix projectionMatrix;
};

cbuffer TranslationBuffer : register(b1)
{
	matrix start;
	matrix end; 
};

struct InputType
{
	float4 position : POSITION;
	float2 tex : TEXCOORD0;
	float3 normal : NORMAL;
};

struct OutputType
{
	float4 position : SV_POSITION;
	float2 tex : TEXCOORD0;
	float3 normal : NORMAL;
	//float3 worldPosition : TEXCOORD1;

};

OutputType main(InputType input)
{
	OutputType output;
	
    float4 temp = input.position;
    if (temp.y == 1) //If the vertex's position is (1, 0) which means this is the end, 
    { //Multiply by translation matrix on x and y for segment end
        matrix m = mul(worldMatrix, end);
        output.position = mul(input.position, m);
		
    }
	else //It's the start
    { //Multiply by translation matrix on x and y for segment start
        matrix m = mul(worldMatrix , start);
		output.position = mul(input.position, m);
		
    }
	
	output.position = mul(output.position, viewMatrix);
	output.position = mul(output.position, projectionMatrix);

	output.tex = input.tex;
	// Calculate the normal vector against the world matrix only and normalise.
	output.normal = mul(input.normal, (float3x3) worldMatrix);
	output.normal = normalize(output.normal);

	return output;
}

