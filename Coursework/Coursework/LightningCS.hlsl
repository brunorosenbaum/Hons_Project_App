#define groupthreads 16
#define pow_rho 3

float CalcDistance(float x1, float y1, float x2, float y2)
{
    return sqrt((x2 - x1) * (x2 - x1) + (y2 - y1) * (y2 - y1));
}

struct Cell
{
    //int x, y; //We eont need this bc our threads are gonna figure this out (thread ID) 
    float phi;
    float N, P, B;
    int isCandidate; 
};

//struct Cluster
//{
//    int x, y;
//    int xSum, ySum;
//    int xAvg, yAvg;
//};


struct OutputDataType //This one is for read & write data that we need to pass around
{
    float r;
    float N;
    float phi; 
};

StructuredBuffer<Cell> Cells : register(t0); //Why do we register it as t tho? This is the reading one
//StructuredBuffer<Cell> CellInput : register(t1);
RWStructuredBuffer<OutputDataType> CS_OutputBuffer : register(u0); //We use u for unordered (UAV). Read & write. Output buffer 

[numthreads(16, 16, 1)]
void main( uint3 DTid : SV_DispatchThreadID, 
    uint3 Gid : SV_GroupID, 
    uint GI : SV_GroupIndex)
{
    //numthreads -> 1x1x1

    //Gid -> group offset in the dispatch call 3d array (posicion de la celda en el grupo de dispatch -
    ////en este caso en la grid de 64x64)
    
    //DTid ->  global thread offset within the dispatch call across three dimensions.
    //SV_DispatchThreadID (DTid) = (SV_GroupID (Gid) * NumThreadsPerGroup)+SV_GroupThreadID //WE WANT THIS .X AND .Y ARE THE XY FOR CELL

    //SV_GroupThreadID -> thread offset within the group across three dimensions (el id de la thread en la grid grande)

    //SV_GroupIndex (GI) --> flattened array index version of the SV_GroupThreadID.
    //GI = SV_GroupThreadID.z * NumThreadsPerGroup.y *NumThreadsPerGroup.x + SV_GroupThreadID.y*NumThreadsPerGroup.x + SV_GroupThreadID.x

    //ASK ABOUT GROUPSHARED
    //-------------------------------------------------------------------------------------------------------------
    uint clusterSize = 16; //They're the same, x or y since it's a square grid. 64x64
    uint cellIndex = DTid.y * 128 + DTid.x;
    //uint clustIndex = 0;
    float N = CS_OutputBuffer[cellIndex].N;
    float r = CS_OutputBuffer[cellIndex].r;
    //groupshared Cell gCache[CacheSize];

    //Get rid of loops 
    //if (clustIndex != cellIndex) //Negative charge cells NOT in same cluster
    //{

    if(Cells[cellIndex].isCandidate)
    {
	      r = CalcDistance(DTid.x + 1, DTid.y + 1,
											 DTid.x, DTid.y);
        if (pow_rho > 1) //Isn't this kind of an unnecessary check if we're not dynamically changing this constant
        {
            r = pow(r, pow_rho);
        }
        N += clusterSize / r;
        CS_OutputBuffer[cellIndex].N = N;
        CS_OutputBuffer[cellIndex].r = r;
    }
      
    //}
    else //Negative cells in same cluster
    {
        //for (int i = 0; i < clusterSize; ++i) //This for loop should be only the dynamic size of the cells cluster
        //{
        //    r = CalcDistance(DTid.x + 1, DTid.x + 1,
								//		 DTid.x, DTid.y);
        //    if (pow_rho > 1) //Isn't this kind of an unnecessary check if we're not dynamically changing this constant
        //    {
        //        r = pow(r, pow_rho);
        //    }
        //    N += 1.0f / r;
        //}
        N = Cells[cellIndex].N; 
        r = 0; 
    }
           


	GroupMemoryBarrierWithGroupSync(); //Waits until all threads are done

    float B = Cells[cellIndex].B;
    float P = Cells[cellIndex].P;
   
  
    CS_OutputBuffer[cellIndex].phi = (1.0f / B) * (1.0f / N) * P;
	
    //CS_OutputBuffer[index].phi = (1.0f / 0.5) * (1.0f / CS_OutputBuffer[index].N) * 0.5;
    //if (CS_OutputBuffer[index].phi == 0)
    //{
    //    CS_OutputBuffer[index].phi = 0.5;

    //}
    CS_OutputBuffer[cellIndex].phi = max(CS_OutputBuffer[cellIndex].phi, 0.5f);
    //CS_OutputBuffer[index].N = max(CS_OutputBuffer[index].N, 0.5f);

    //CS_OutputBuffer[candIndex].phi = 

}