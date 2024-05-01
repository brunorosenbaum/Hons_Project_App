#define gridSizeXY 128
#define groupthreads 16
#define gsmThreads 20
#define clusterHalf 2
#define pow_rho 3

float CalcDistance(float x1, float y1, float x2, float y2)
{
    if(x1 == x2 && y1 == y2)
        return 0.0f; 
    return sqrt((x2 - x1) * (x2 - x1) + (y2 - y1) * (y2 - y1));
}

float CalcAverage(int elements[16])
{
    float sum; 
    for (int i = 0; i < 16; ++i)
    {
        [unroll]
        sum += elements[i];
    }
    return sum / 16; 
}

struct Cell
{
    //int x, y; //We eont need this bc our threads are gonna figure this out (thread ID) 
    float phi;
    float N, P, B;
    int isCandidate;
    int isCluster; 
};

struct OutputDataType //This one is for read & write data that we need to pass around
{
    float r;
    float N;
    float phi; 
};


StructuredBuffer<Cell> Cells : register(t0); //srv
RWStructuredBuffer<OutputDataType> CS_OutputBuffer : register(u0); //We use u for unordered (UAV). Read & write. Output buffer 

//groupshared Cell GSMCells[gsmThreads][gsmThreads]; //Declare group shared memory. WHEN WE DO THIS ITS GONNA HAVE TO BE MORE THAN 16!!
//It'll be 20 threads sharing the same memory so 16 do work.
//16x16 = 256 & 20x20 = 400
//If we have 400 groups of threads sharing memory then we can select the 256 ones that we're processing data with
//And only work with those
//groupshared Cell GSMCells[gsmThreads][gsmThreads];
groupshared Cell GSM_Cells[gsmThreads * gsmThreads]; 
[numthreads(groupthreads, groupthreads, 1)]
void main( uint3 DTid : SV_DispatchThreadID, 
    uint3 Gid : SV_GroupID,
	uint3 GTid : SV_GroupThreadID,
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

    int cellIndex = DTid.y * gridSizeXY + DTid.x;

    //Group offset
    int globalXOffset = Gid.x * 16;
    int globalYOffset = Gid.y * 16;

    //Subtract 2 from DTid to get the (0, 0) of the given 20x20 group in the greater grid (128x128)
    globalXOffset = globalXOffset - 2 < 0 ? 0 : (globalXOffset - 2);
    globalYOffset = globalYOffset - 2 < 0 ? 0 : (globalYOffset - 2);
    int2 groupStartIndex = int2(globalXOffset, globalYOffset); //Offset between the groups, PURPLE area
    //^^^^ This is not flattened

    //Flattened group shared memory index
    int gsmIndex = GTid.y * 16 + GTid.x; //For max value 256

    //Now, in our 16x16, we have [0~255] cells we have to sample with one or two threads
    //And a remaining 144, since (20x20 = 400) - 256 = 144.

    int gsmx = gsmIndex % 20; //Getting x and y pos in the 20x20 grid
    int gsmy = gsmIndex / 20;
    
    int x_GridIndex = groupStartIndex.x + gsmx; //Index of the cell we want to sample in the global grid
    int y_GridIndex = groupStartIndex.y + gsmy;

    int globalIndex = y_GridIndex * gridSizeXY + x_GridIndex;

    //Get the cell
    GSM_Cells[gsmIndex] = Cells[globalIndex];

    gsmIndex += 256;
    if (gsmIndex < 400) //We sample again
    { //some threads grab 2 & read 2 but we only write to 1
        
        GSM_Cells[gsmIndex] = Cells[globalIndex];

    }
    GroupMemoryBarrierWithGroupSync();

    //Each thread loads one of the inner cells

    //GSM Array is 20x20, but the Thread Group is 16x16
    //First we assign the group id to our gsm
    //Say our thread group id is (5, 5)
    //Then number (5, 5) of our 20x20 grid is assigned the data stored in (5, 5) of our 16x16 grid
    //Therefore, by shifting by 2 our group thread id, we have the index on the 20x20 grid correspondent to it

	//---------------------------------------------
    //Actual code, now with GSM

    


    uint gx = GTid.x + 2;
    uint gy = GTid.y + 2;
    uint localIndex = (gy * 20 + gx);
    //uint localIndextemp = GTid.y * 20 + GTid.x;
    float B = GSM_Cells[localIndex].B;
    float P = GSM_Cells[localIndex].P;
    float N = GSM_Cells[localIndex].N;
    float r = 0;
    float phi = GSM_Cells[localIndex].phi;
    
    int2 minIndex = int2(gx - 4, gy - 4);
    int2 maxIndex = int2(gx + 4, gy + 4);

    if (GSM_Cells[localIndex].isCandidate)
    {
        for (int yi = minIndex.y; yi < maxIndex.y; ++yi)
        {
            for (int xi = minIndex.x; xi < maxIndex.x; ++xi)
            {
                int tempx = xi > 17 ? 17 : xi;
                int tempy = yi > 17 ? 17 : yi;
                tempx = tempx < 2 ? 2 : tempx;
                tempy = tempy < 2 ? 2 : tempy;
                r = CalcDistance(tempx, tempy,
									 gx, gy);
                r = pow(r, pow_rho);
                if (r == 0)
                {
                    N += 0;
                }
                else
                {
                    N += 1.0f / r;
                    phi = (1.0f / B) * (1.0f / N) * P;
                }
               
            }

        }
        
    }
    GroupMemoryBarrierWithGroupSync();

    CS_OutputBuffer[cellIndex].N = N;
    CS_OutputBuffer[cellIndex].r = r;
    CS_OutputBuffer[cellIndex].phi = phi;

}


  
