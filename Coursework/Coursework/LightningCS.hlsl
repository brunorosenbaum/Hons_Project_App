#define groupthreads 16
#define pow_rho 3

float CalcDistance(float x1, float y1, float x2, float y2)
{
    return sqrt((x2 - x1) * (x2 - x1) + (y2 - y1) * (y2 - y1));
}

struct Cell
{
    int x, y;
    float phi;
    float N, P, B; 
};

struct Cluster
{
    int x, y;
    int xSum, ySum;
    int xAvg, yAvg; 
    /*Cell clusterCells[16]*/ //Youve to define this
};


struct OutputDataType //This one is for read & write data that we need to pass around
{
    float r;
    float N;
    float phi; 
};

//TODO: SEND THESE AS UAVs etc

StructuredBuffer<Cluster> ClusterInput : register(t0); //Why do we register it as t tho? This is the reading one
StructuredBuffer<Cell> CellInput : register(t1); //Why do we register it as t tho? This is the reading one
RWStructuredBuffer<OutputDataType> CS_OutputBuffer : register(u0); //We use u for unordered (UAV). Read & write. Output buffer 

[numthreads(1, 1, 1)]
void main( uint3 DTid : SV_DispatchThreadID, 
    uint3 Gid : SV_GroupID, 
    uint GI : SV_GroupIndex)
{
    //numthreads -> 1x1x1

    //Gid -> group offset in the dispatch call 3d array (posicion de la celda en el grupo de dispatch -
    ////en este caso en la grid de 64x64)
    
    //DTid ->  global thread offset within the dispatch call across three dimensions.
    //SV_DispatchThreadID (DTid) = (SV_GroupID (Gid) * NumThreadsPerGroup)+SV_GroupThreadID

    //SV_GroupThreadID -> thread offset within the group across three dimensions (el id de la thread en la grid grande)

    //SV_GroupIndex (GI) --> flattened array index version of the SV_GroupThreadID.
    //GI = SV_GroupThreadID.z * NumThreadsPerGroup.y *NumThreadsPerGroup.x + SV_GroupThreadID.y*NumThreadsPerGroup.x + SV_GroupThreadID.x

    //ASK ABOUT GROUPSHARED
    //-------------------------------------------------------------------------------------------------------------
    uint clusterSize = 16; //They're the same, x or y since it's a square grid. 64x64
    uint candIndex = DTid.y * 4 + DTid.x;
    uint clustIndex = 0;
    float N = CS_OutputBuffer[candIndex].N;
    float r = CS_OutputBuffer[candIndex].r; 

    for (int cy = 0; cy < DTid.y; ++cy) //This'd run 64 times, careful bc clusters are 16
    {
        for (int cx = 0; cx < clusterSize; ++cx)
        {
           
            if ( clustIndex != candIndex) //Negative charge charge cells NOT in same cluster
            {
               r = CalcDistance(ClusterInput[candIndex].xAvg, ClusterInput[candIndex].yAvg,
											 CellInput[candIndex].x, CellInput[candIndex].y);
                if (pow_rho > 1) //Isn't this kind of an unnecessary check if we're not dynamically changing this constant
                {
                    r = pow(r, pow_rho);
                }
                N += clusterSize / r;
            }
            else //Negative cells in same cluster
            {
                for (int i = 0; i < clusterSize; ++i)
                {
                    r = CalcDistance(ClusterInput[candIndex].xAvg, ClusterInput[candIndex].yAvg,
										 CellInput[candIndex].x, CellInput[candIndex].y);
                    if (pow_rho > 1) //Isn't this kind of an unnecessary check if we're not dynamically changing this constant
                    {
                        r = pow(r, pow_rho);
                    }
                    N += 1.0f / r;
                }

            }

            ++clustIndex; 
        }
    }

	GroupMemoryBarrierWithGroupSync(); //Waits until all threads are done

    float B = CellInput[candIndex].B;
    float P = CellInput[candIndex].P;
    CS_OutputBuffer[candIndex].N = N;
    CS_OutputBuffer[candIndex].r = r;
  
    CS_OutputBuffer[candIndex].phi = (1.0f / B) * (1.0f / N) * P;
	
    //CS_OutputBuffer[index].phi = (1.0f / 0.5) * (1.0f / CS_OutputBuffer[index].N) * 0.5;
    //if (CS_OutputBuffer[index].phi == 0)
    //{
    //    CS_OutputBuffer[index].phi = 0.5;

    //}
    CS_OutputBuffer[candIndex].phi = max(CS_OutputBuffer[candIndex].phi, 0.5f);
    //CS_OutputBuffer[index].N = max(CS_OutputBuffer[index].N, 0.5f);

    //CS_OutputBuffer[candIndex].phi = 

}