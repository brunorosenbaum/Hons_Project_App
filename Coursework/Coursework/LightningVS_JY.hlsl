
cbuffer MatrixBuffer : register(b0)
{
    matrix worldMatrix;
    matrix viewMatrix;
    matrix projectionMatrix;
};

cbuffer DynVxBuffer : register(b1)
{
    float2 start;
    float2 end;
    float2 corner; 
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

};

OutputType main(InputType input)
{
    OutputType output;


   //if(input.position.y == 1) //Vertices 1, 2
   //{
   //     if (input.position.x == 0) //[2] (1, 1)
   //         input.position.xy = float2(end.x + corner.x, end.y - corner.y); 

   //     else //[1] (1, 0)
   //         input.position.xy = float2(end.x - corner.x, end.y + corner.y ); //
        
        
   //}
   // else //Vertices 0, 3
   // {
   //     if (input.position.x == 1) //[3] (1, 0)
   //         input.position.xy = float2(start.x - corner.x, start.y + corner.y);

   //     else //[0] (0, 0)
   //         input.position.xy = float2(start.x + corner.x, start.y - corner.y); //

   // }

    if (input.position.y == 0) //Vertices 1, 2
    {
        input.position.xy = float2(end.x, end.y);

        //if (input.position.x == 0) //[2] (1, 1)
        //    input.position.xy = float2(end.x, end.y);
        //else //[1] (1, 0)
        //    input.position.xy = float2(end.x, end.y); //
        
        
    }
    else //Vertices 0, 3
    {
        input.position.xy = float2(start.x, start.y);

        //if (input.position.x == 1) //[3] (1, 0)
        //    input.position.xy = float2(start.x, start.y);
        //else //[0] (0, 0)
        //    input.position.xy = float2(start.x, start.y); //

    }
	output.position = mul(input.position, worldMatrix);
	
    output.position = mul(output.position, viewMatrix);
    output.position = mul(output.position, projectionMatrix);

    output.tex = input.tex;
	// Calculate the normal vector against the world matrix only and normalise.
    output.normal = mul(input.normal, (float3x3) worldMatrix);
    output.normal = normalize(output.normal);

    return output;
}

