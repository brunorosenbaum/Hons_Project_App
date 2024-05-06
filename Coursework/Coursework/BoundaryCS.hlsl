#define gridSizeXY 128
#define groupthreads 16
#define pow_rho 3

float CalcDistance(float x1, float y1, float x2, float y2)
{
    if (x1 == x2 && y1 == y2)
        return 0.0f;
    return sqrt((x2 - x1) * (x2 - x1) + (y2 - y1) * (y2 - y1));
}

struct BoundaryCell
{
    int x, y;
};

struct OutputDataType //This one is for read & write data that we need to pass around
{
    float boundaryPhi_;
};


StructuredBuffer<BoundaryCell> BoundaryCells : register(t0); //srv2
RWStructuredBuffer<OutputDataType> CS_OutputBuffer : register(u0); //We use u for unordered (UAV). Read & write. Output buffer 

[numthreads(groupthreads, groupthreads, 1)]
void main(uint3 DTid : SV_DispatchThreadID,
    uint3 Gid : SV_GroupID,
	uint3 GTid : SV_GroupThreadID,
    uint GI : SV_GroupIndex)
{

    int cellIndex = DTid.y * gridSizeXY + DTid.x;

    float r = 0;
    float boundaryPhi = 0;

    GroupMemoryBarrierWithGroupSync();
    [unroll]
    for (int i = 0; i < 516; ++i) //Number of boundary cells. Hard-coded is bad practice but it's temporary
    {
        
            r = CalcDistance(DTid.x, DTid.y, BoundaryCells[i].x, BoundaryCells[i].y);
            r = pow(r, pow_rho);
            boundaryPhi += 1.0f / r;

        
    }
	CS_OutputBuffer[cellIndex].boundaryPhi_ = boundaryPhi;
    GroupMemoryBarrierWithGroupSync();
    



}


  
