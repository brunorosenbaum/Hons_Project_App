#define groupthreads 16
#define pow_rho 3

float CalcDistance(float x1, float y1, float x2, float y2)
{
    return sqrt((x2 - x1) * (x2 - x1) + (y2 - y1) * (y2 - y1));
}


struct DataType //Lightning data type. Read only..?
{
    uint2 clusterPos; //XY
    uint2 candidateClusterPos; //XY
    uint clusterSize;
    bool isClusterEmpty; 
};

struct OutputDataType //This one is for read & write data that we need to pass around
{
    float r;
    float N;
};
//TODO: SEND THESE AS UAVs etc
StructuredBuffer<DataType> CS_InputBuffer : register(t0); //Why do we register it as t tho? This is the reading one
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
    uint clusterSize = DTid.x; //They're the same, x or y since it's a square grid. 64x64
    uint index = DTid.y * 4 + DTid.x; 

    for (int cy = 0; cy < DTid.y; ++cy) //This'd run 64 times, careful bc clusters are 16
    {
        for (int cx = 0; cx < clusterSize; ++cx)
        {
	        if(!CS_InputBuffer[index].isClusterEmpty())
	        {
                CS_OutputBuffer[index].r = CalcDistance(vars here);
                if(pow_rho > 1) //Isn't this kind of an unnecessary check if we're not dynamically changing this constant
                {
                    CS_OutputBuffer[index].r = pow(CS_OutputBuffer[index].r, pow_rho); 
                }
                CS_OutputBuffer[index].N += clusterSize / CS_OutputBuffer[index].r; 
            }
            else
            {
	            //Same as below but different
            }
            //++clusterindex
        }
    }



        GroupMemoryBarrierWithGroupSync
        (); //Waits until all threads are done
    
    
}