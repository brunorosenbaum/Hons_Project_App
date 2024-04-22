#define gridSizeXY 128
#define groupthreads 16
#define gsmThreads 20
#define clusterHalf 4
#define pow_rho 3

float CalcDistance(float x1, float y1, float x2, float y2)
{
    if(x1 == x2 && y1 == y2)
        return 1.0f; 
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

groupshared Cell GSM[gsmThreads]; //Declare group shared memory. WHEN WE DO THIS ITS GONNA HAVE TO BE MORE THAN 16!!
//It'll be 20 threads sharing the same memory so 16 do work.
//16x16 = 256 & 20x20 = 400
//If we have 400 groups of threads sharing memory then we can select the 256 ones that we're processing data with
//And only work with those

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

    //Cluster offset in the y axis.
    //For GTid = (1, 1, 0) it'd be 32 - 4 + (128 - 4*2) * 1 = 120
	int groupOffsetY = GI - clusterHalf + (groupthreads - clusterHalf * 2) * Gid.y;
    //Cluster offset in the x axis.
    int clusterOffsetX = GI - clusterHalf + (groupthreads - clusterHalf * 2) * Gid.x;

    //Clamp their values from 0-127
    groupOffsetY = clamp(groupOffsetY, 0, gridSizeXY - 1);
    clusterOffsetX = clamp(clusterOffsetX, 0, gridSizeXY - 1);

    //-------------------------------------
    //For Leftmost & upper bound
    if(GTid.x < clusterHalf) //If group thread id is < 4 -- would it have to be GTid.x == 0?
    {
        int x_min = max(DTid.x - clusterHalf, 0);
        GSM[GTid.x] = Cells[x_min - 2, DTid.y]; //GSM[x = 0, y = 0] is 0, 0 for 20x20 gsm cells
        GSM[GTid.x].isCandidate = false;
        //It's (x_min - 2) bc we need to subtract 2x2 to find (0, 0) of 20x20 'grid'

        if (GTid.y < clusterHalf) //Covers corners bc it includes Y
        {
			int y_min = max(DTid.y - clusterHalf, 0);
            //Index would be GTid.x + 20 * GTid.y ?
            GSM[GTid.y /*+ clusterHalf * GTid.y*/] = Cells[x_min - 2, y_min - 2]; //Corner case
            GSM[GTid.y /*+ clusterHalf * GTid.y*/].isCandidate = false;
	        
        }
        
    }
    else if (GTid.y < clusterHalf) //Upper y axis only 
    {
        int y_min = max(DTid.y - clusterHalf, 0);
        GSM[/*GTid.x + clusterHalf * */GTid.y] = Cells[DTid.x, y_min - 2]; //Corner case
        GSM[/*GTid.x + clusterHalf **/ GTid.y].isCandidate = false;

    }

    //For rightmost & lower bound
    if(GTid.x >= groupthreads - clusterHalf)
    {
        int x_max = min(DTid.x + clusterHalf, gridSizeXY - 1);
        GSM[GTid.x] = Cells[x_max + 2, DTid.y];
        GSM[GTid.x].isCandidate = false; 

        if (GTid.y >= groupthreads - clusterHalf) //Covers corners bc it includes Y
        {
            int y_max = min(DTid.y + clusterHalf, gridSizeXY - 1);
            GSM[/*GTid.x + clusterHalf **/ GTid.y] = Cells[x_max + 2, y_max + 2]; //Corner case
            GSM[/*GTid.x + clusterHalf **/ GTid.y].isCandidate = false; 
	        
        }
    }
    else if(GTid.y >= groupthreads - clusterHalf) //Lower y bound only
    {
        int y_max = min(DTid.y + clusterHalf, gridSizeXY - 1);
        GSM[/*GTid.x + clusterHalf **/ GTid.y] = Cells[GTid.x, y_max + 2]; //Corner case
        GSM[/*GTid.x + clusterHalf **/ GTid.y].isCandidate = false; 
    }

    //if (gsm > groupthreads)//If it's bigger than 256?
    //{
    //    //Have two threads
    //}
    //else
    //{
	   // //Have one thread
    //}
    GroupMemoryBarrierWithGroupSync(); //Sync all threads

    //Whole array offset
    //int groupOffsetY = Gid.x + clusterOffsetY * gridSizeXY;

    //Make group shared cache memory - ask what exactly we're doing here
    //gsCache[GI] = Cells[arrayOffset]; //Are we sharing the memory of 8 contiguous cells? (since 128-120 = 8) 
    //isnt this gonna throw an error bc its out of bounds, since groupthreads is 16 and GI can be larger?

  

	uint clusterSize = 16; //They're the same, x or y since it's a square grid. 64x64
    uint cellIndex = DTid.y + gridSizeXY * DTid.x;
    float B = Cells[cellIndex].B;
    float P = Cells[cellIndex].P;
    float N = Cells[cellIndex].N;
    float r = CS_OutputBuffer[cellIndex].r;
    float phi; 


    GroupMemoryBarrierWithGroupSync();

    if (Cells[cellIndex].isCandidate)
    {
        int indx = 0;

        int x_Pos = DTid.x;
        int y_Pos = DTid.y;

        for (int xOffset = -16; xOffset < 17; ++xOffset) //Go through neighbors
        {
            //-1, 0, 1 <- These loops will go through these values
            int new_xPos = x_Pos + xOffset;
            new_xPos = new_xPos < 0 ? 0 : new_xPos; //If new x pos is less than 0, cannot be in the grid, therefore 0
            new_xPos = new_xPos >= gridSizeXY - 1 ? gridSizeXY - 1 : new_xPos;

            for (int yOffset = -16; yOffset < 17; ++yOffset)
            {
                int new_YPos = y_Pos + yOffset;
                new_YPos = new_YPos < 0 ? 0 : new_YPos;
                new_YPos = new_YPos >= gridSizeXY - 1 ? gridSizeXY - 1 : new_YPos;

                //Calcs
                r = CalcDistance(new_xPos, new_YPos,
										 DTid.x, DTid.y);

                r = pow(r, pow_rho);
                if(r == 0)
                    N += 0;
                else
            		N += 1.0f / r; 
                
            }
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

    //Shouls I be using this??
    //const int CELL_R::
    //NEIGHBORS_X_DIFFERENCE[8] = {
    //    0, 1, 1, 1, 0, -1, -1, -1};
    //    const int CELL_R::
    //    NEIGHBORS_Y_DIFFERENCE[8] = {
    //        -1, -1, 0, 1, 1, 1, 0, -1};

    //Instead of using a for loop, go through the neighbors x and y difference arrays
    //The offset will be neighbors[index] * offset?
    //And the offset will be groupThreadID + clusterRadius * 2?
    //GROUPTHREAD ID!! NOT DISPATCH THREAD ID!! THERES A DIFFERENCE!!
    //GTid is the x and y within the group!! which is a 16x16 grid!
    //-1 * (0 + 4*2) = -8
    //-1 * (1 + 4*2) = -9
    //Going through the values of the x neighbors with GTid = 0 is:
    //0, 8, 8, 8, 0, -8, -8, -8 <- Is this anything


    //What i know is the offset has to be clamped for out of bound samples so
    //If (GTid.x < clusterRadius) clamp to 0
    //If (GTid.x >= groupthreads - clusterRadius) //So if GTid >= 12 (16-4)
    //Clamp x to min(DTid.x + clusterRadius, 128-1); <-Makes sense

	//GroupMemoryBarrierWithGroupSync(); //Waits until all threads are done
  
}