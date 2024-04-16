#define gridSize 128
#define groupthreads 16
#define clusterHalf 4
#define pow_rho 3

float CalcDistance(float x1, float y1, float x2, float y2)
{
    return sqrt((x2 - x1) * (x2 - x1) + (y2 - y1) * (y2 - y1));
}

float CalcAverage(int elements[(clusterHalf * 2) + 1])
{
    float sum; 
    for (int i = 0; i < clusterHalf * 2 + 1; ++i)
    {
        [unroll]
        sum += elements[i];
    }
    return sum / (clusterHalf * 2) + 1; 
}

struct Cell
{
    //int x, y; //We eont need this bc our threads are gonna figure this out (thread ID) 
    float phi;
    float N, P, B;
    int isCandidate; 
};

struct OutputDataType //This one is for read & write data that we need to pass around
{
    float r;
    float N;
    float phi; 
};


StructuredBuffer<Cell> Cells : register(t0); //srv
RWStructuredBuffer<OutputDataType> CS_OutputBuffer : register(u0); //We use u for unordered (UAV). Read & write. Output buffer 

groupshared Cell gsCache[groupthreads]; //Declare group shared memory. Size 16

[numthreads(groupthreads, groupthreads, 1)]
void main( uint3 DTid : SV_DispatchThreadID, 
    uint3 Gid : SV_GroupID, 
    uint GI : SV_GroupIndex)
{
    //Gid -> group offset in the dispatch call 3d array (posicion de la celda en el grupo de dispatch -
    ////en este caso en la grid de 64x64)
    
    //DTid ->  global thread offset within the dispatch call across three dimensions.
    //SV_DispatchThreadID (DTid) = (SV_GroupID (Gid) * NumThreadsPerGroup)+SV_GroupThreadID //WE WANT THIS .X AND .Y ARE THE XY FOR CELL

    //SV_GroupThreadID -> thread offset within the group across three dimensions (el id de la thread en la grid grande)

    //SV_GroupIndex (GI) --> flattened array index version of the SV_GroupThreadID.
    //GI = SV_GroupThreadID.z * NumThreadsPerGroup.y *NumThreadsPerGroup.x + SV_GroupThreadID.y*NumThreadsPerGroup.x + SV_GroupThreadID.x

    //-------------------------------------------------------------------------------------------------------------

    //Cluster offset in the y axis.
    //For GTid = (1, 1, 0) it'd be 32 - 4 + (128 - 4*2) * 1 = 120
	int clusterOffsetY = GI - clusterHalf + (groupthreads - clusterHalf * 2) * Gid.y;
    //Cluster offset in the x axis.
    int clusterOffsetX = GI - clusterHalf + (groupthreads - clusterHalf * 2) * Gid.x;

    //Clamp their values from 0-127
    clusterOffsetY = clamp(clusterOffsetY, 0, gridSize - 1);
    clusterOffsetX = clamp(clusterOffsetX, 0, gridSize - 1);

    //Whole array offset
    int arrayOffset = Gid.x + clusterOffsetY * gridSize;

    //Make group shared cache memory - ask what exactly we're doing here
    gsCache[GI] = Cells[arrayOffset]; //Are we sharing the memory of 8 contiguous cells? (since 128-120 = 8) 
    //isnt this gonna throw an error bc its out of bounds, since groupthreads is 16 and GI can be larger?

    GroupMemoryBarrierWithGroupSync(); //Sync all threads

	uint clusterSize = 16; //They're the same, x or y since it's a square grid. 64x64
    uint cellIndex = DTid.y * 128 + DTid.x;
    float B = Cells[cellIndex].B;
    float P = Cells[cellIndex].P;
    float N = Cells[cellIndex].N;
    float r = CS_OutputBuffer[cellIndex].r;
    float phi; 
    float avgY = 0;
    float avgX = 0;

    //GAUSSIAN BLUR LOGIC???
    //if(GI >= clusterHalf && 
    //    GI < (groupthreads - clusterHalf))
    //{
    //    if ((GI - clusterHalf + (groupthreads - clusterHalf * 2) * Gid.y) < gridSize) //Y threads?
    //    {
	   //     int clusterY[clusterHalf * 2 + 1]; //Size = 8
	   //     int tempIndex = 0;
	   //     [unroll]
    //		for (int i = -clusterHalf; i <= clusterHalf; ++i) //Will go from -4 to 4
	   //     {
	   //         clusterY[tempIndex] = DTid.y + i; //Tempindex from 0 to 7, will hold positions of cell.y +- 4
	   //         ++tempIndex; 
	   //     }
	   //     avgY = CalcAverage(clusterY); //Calc average y position of the neighboring cells
    //    }
    //    if ((GI - clusterHalf + (groupthreads - clusterHalf * 2) * Gid.x) < gridSize) //X threads
    //    {
    //        int clusterX[clusterHalf * 2 + 1];
    //        int tempIndex = 0;
	   //     [unroll]
    //        for (int i = -clusterHalf; i <= clusterHalf; ++i) //Will go from -4 to 4
    //        {
    //            clusterX[tempIndex] = DTid.x + i; //Tempindex from 0 to 7, will hold positions of cell.y +- 4
    //            ++tempIndex;
    //        }
    //        avgX = CalcAverage(clusterX); //Calc average y position of the neighboring cells
    //    }

    //}

    //MY LOGIC?
    //if (GI >= clusterHalf &&
    //   GI < (groupthreads - clusterHalf))
    //{

        int clusterY[clusterHalf * 2 + 1]; //Size = 8
        int clusterX[clusterHalf * 2 + 1]; //Size = 8
		int tempIndex = 0;


        for (int i = -clusterHalf; i <= clusterHalf; ++i)
        {
            clusterY[tempIndex] = DTid.y + i;
            clusterX[tempIndex] = DTid.x + i;
            ++tempIndex; 
        }
	  avgY = CalcAverage(clusterY); //Calc average y position of the neighboring cells
	  avgX = CalcAverage(clusterX); //Calc average x position of the neighboring cells

    //}

    GroupMemoryBarrierWithGroupSync();

    //if(Cells[cellIndex].isCandidate)
    //{
	   //   r = CalcDistance(avgX, avgY,
				//							 DTid.x, DTid.y);
    //    if (pow_rho > 1) //Isn't this kind of an unnecessary check if we're not dynamically changing this constant
    //    {
    //        r = pow(r, pow_rho);
    //    }
    //    N += clusterSize / r;
    //    phi = (1.0f / B) * (1.0f / N) * P;
        
    //}
    if (Cells[cellIndex].isCandidate)
    {
        int indx = 0;

        [unroll]
        for (int i = -clusterHalf; i <= clusterHalf; ++i)
        {
            r = CalcDistance(clusterX[indx].x, clusterX[indx].x,
											 DTid.x, DTid.y);
            r = pow(r, pow_rho);
            N += clusterSize / r;

            ++indx;
        }

        phi = (1.0f / B) * (1.0f / N) * P;

        CS_OutputBuffer[cellIndex].N = N;
        CS_OutputBuffer[cellIndex].r = r;
        CS_OutputBuffer[cellIndex].phi = phi;
        
    }
    else //No changes
    {
        r = 0; 
        phi = Cells[cellIndex].phi;

        CS_OutputBuffer[cellIndex].N = N;
        CS_OutputBuffer[cellIndex].r = r;
        CS_OutputBuffer[cellIndex].phi = phi;
    }
           


	//GroupMemoryBarrierWithGroupSync(); //Waits until all threads are done
  
}