#define gridSizeXY 128
#define groupthreads 16
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

//groupshared Cell gsCache[groupthreads]; //Declare group shared memory. WHEN WE DO THIS ITS GONNA HAVE TO BE MORE THAN 16!!

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
    clusterOffsetY = clamp(clusterOffsetY, 0, gridSizeXY - 1);
    clusterOffsetX = clamp(clusterOffsetX, 0, gridSizeXY - 1);

    //Whole array offset
    //int arrayOffset = Gid.x + clusterOffsetY * gridSizeXY;

    //Make group shared cache memory - ask what exactly we're doing here
    //gsCache[GI] = Cells[arrayOffset]; //Are we sharing the memory of 8 contiguous cells? (since 128-120 = 8) 
    //isnt this gonna throw an error bc its out of bounds, since groupthreads is 16 and GI can be larger?

    GroupMemoryBarrierWithGroupSync(); //Sync all threads

	uint clusterSize = 16; //They're the same, x or y since it's a square grid. 64x64
    uint cellIndex = DTid.y + gridSizeXY * DTid.x;
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

  //      int clusterY[clusterHalf * 2 + 1]; //Size = 8
  //      int clusterX[clusterHalf * 2 + 1]; //Size = 8
		//int tempIndex = 0;


  //      for (int i = -clusterHalf; i <= clusterHalf; ++i)
  //      { //So for example for DTid = (63, 0); 
  //          clusterY[tempIndex] = DTid.y + i; //-4, -3, -2, -1, 0, 1, 2, 3 -> This doesn't cover all
  //          clusterX[tempIndex] = DTid.x + i; //59, 60... 66 -> Same thing
  //          ++tempIndex;
  //          //There's also the issue where the 1st candidates coords are (63, 0)
  //          ////but this code only runs when DTid.y = 63, DTid.x = 0 (the opposite)
  //          ///But I can't find where I'm flattening the array wrongly
  //      }
	 // avgY = CalcAverage(clusterY); //Calc average y position of the neighboring cells
	 // avgX = CalcAverage(clusterX); //Calc average x position of the neighboring cells

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

        //[unroll]
        //for (int i = -clusterHalf; i <= clusterHalf; ++i)
        //{
        //    //In DTid = (0, 63) (appears as candidate) we would have:
        //    //CalcDistance = (-4, 59, 0, 63) = sqrt(32) = 5.65
        //    r = CalcDistance(clusterX[indx], clusterY[indx],
								//			 DTid.x, DTid.y);
        //    //r = 5.65
        //    r = pow(r, pow_rho); //r = 5.65^3 = 181.01
        //    N += clusterSize / r; // N += 16/181 = 0.0883; 
        //    //Problem here: I don't know if N is being summed correctly (+=). 

        //    ++indx;
        //}
        int x_Pos = DTid.x;
        int y_Pos = DTid.y;

        for (int xOffset = -1; xOffset < 2; ++xOffset) //Go through neighbors
        {
            //-1, 0, 1 <- These loops will go through these values
            int new_xPos = x_Pos + xOffset;
            new_xPos = new_xPos < 0 ? 0 : new_xPos; //If new x pos is less than 0, cannot be in the grid, therefore 0
            new_xPos = new_xPos >= gridSizeXY - 1 ? gridSizeXY - 1 : new_xPos;

            for (int yOffset = -1; yOffset < 2; ++yOffset)
            {
                int new_YPos = y_Pos + yOffset;
                new_YPos = new_YPos < 0 ? 0 : new_YPos;
                new_YPos = new_YPos >= gridSizeXY - 1 ? gridSizeXY - 1 : new_YPos;

                //Calcs
                r = CalcDistance(new_xPos, new_YPos,
										 DTid.x, DTid.y);

                r = pow(r, pow_rho); 
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