#define gridSizeXY 128
#define groupthreads 16
#define gsmThreads 20
#define pow_rho 3

float CalcDistance(float x1, float y1, float x2, float y2)
{
    if (x1 == x2 && y1 == y2)
        return 0.0f;
    return sqrt((x2 - x1) * (x2 - x1) + (y2 - y1) * (y2 - y1));
}

struct PositiveCell
{
    int x, y; 
};

struct OutputDataType //This one is for read & write data that we need to pass around
{
    float positivePhi_; 
};


StructuredBuffer<PositiveCell> PosiCell : register(t0); //srv2
RWStructuredBuffer<OutputDataType> CS_OutputBuffer : register(u0); //We use u for unordered (UAV). Read & write. Output buffer 

[numthreads(groupthreads, groupthreads, 1)]
void main(uint3 DTid : SV_DispatchThreadID,
    uint3 Gid : SV_GroupID,
	uint3 GTid : SV_GroupThreadID,
    uint GI : SV_GroupIndex)
{

    int cellIndex = DTid.y * gridSizeXY + DTid.x;

    float r = 0;
    float positivePhi = 0; 

    GroupMemoryBarrierWithGroupSync();

   
    if(DTid.y != PosiCell[0].y && DTid.x != PosiCell[0].x)
    {
        r = CalcDistance(DTid.x, DTid.y, PosiCell[0].x, PosiCell[0].y);
        r = pow(r, pow_rho);
        positivePhi += 1.0f / r;
        CS_OutputBuffer[cellIndex].positivePhi_ = positivePhi;

    }

	GroupMemoryBarrierWithGroupSync();


}


  
