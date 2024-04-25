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
groupshared Cell GSMCells[gsmThreads * gsmThreads];
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

    uint cellIndex = DTid.y + gridSizeXY * DTid.x;

    GSMCells[GTid.x + gsmThreads * GTid.y] = Cells[DTid.y + gridSizeXY * DTid.x];

    //-------------------------------------
    //For Leftmost & upper bound
    if(GTid.x < clusterHalf) //If group thread id is < 4 -- would it have to be GTid.x == 0?
    {
        //int x_min = max(DTid.x - clusterHalf, 0);
        int tmp = DTid.x - clusterHalf;
        int x_min = tmp > 0 ? tmp : 0;
        GSMCells[GTid.x + gsmThreads * GTid.y] = Cells[DTid.y + gridSizeXY * (x_min + 2)]; //GSMCells[x = 0, y = 0] is 0, 0 for 20x20 gsm cells
        GSMCells[(GTid.x + x_min) + gsmThreads * GTid.y].isCluster = false;
        //It's (x_min - 2) bc we need to subtract 2x2 to find (0, 0) of 20x20 'grid'

        if (GTid.y < clusterHalf) //Covers corners bc it includes Y
        {
			//int y_min = max(DTid.y - clusterHalf, 0);
            int tmp2 = DTid.y - clusterHalf;
            int y_min = tmp2 > 0 ? tmp2 : 0;
            //Index would be GTid.x + 20 * GTid.y ?
            GSMCells[GTid.x + gsmThreads * GTid.y] = Cells[(y_min + 2) + gridSizeXY * (x_min + 2)]; //Corner case
            GSMCells[(GTid.x + x_min) + gsmThreads * (GTid.y + y_min)].isCluster = false;
	        
        }
        
    }
    else if (GTid.y < clusterHalf) //Upper y axis only 
    {
        //int y_min = max(DTid.y - clusterHalf, 0);
        int tmp = DTid.y - clusterHalf; 
        int y_min = tmp > 0 ? tmp : 0; 
        GSMCells[GTid.x + gsmThreads * GTid.y] = Cells[(y_min + 2) + gridSizeXY * DTid.x]; //Corner case
        int testVar = GTid.x + gsmThreads * GTid.y; 
        GSMCells[GTid.x + gsmThreads * (GTid.y + y_min)].isCluster = false;

    }

    //For rightmost & lower bound
    if(GTid.x >= groupthreads - clusterHalf)
    {
        //int x_max = min(, gridSizeXY - 1);
        int tmp = DTid.x + clusterHalf;
        int x_max = tmp > gridSizeXY - 1 ? gridSizeXY - 1 : tmp; 
        GSMCells[GTid.x + gsmThreads * GTid.y] = Cells[DTid.y + gridSizeXY * (x_max - 2)];
        GSMCells[(GTid.x + x_max) + gsmThreads * GTid.y].isCluster = false;

        if (GTid.y >= groupthreads - clusterHalf) //Covers corners bc it includes Y
        {
            //int y_max = min(DTid.y + clusterHalf, gridSizeXY - 1);
            int tmp2 = DTid.y + clusterHalf;
            int y_max = tmp2 > gridSizeXY - 1 ? gridSizeXY - 1 : tmp2;
            GSMCells[GTid.x + gsmThreads * GTid.y] = Cells[(y_max - 2) + gridSizeXY * (x_max - 2)]; //Corner case
            GSMCells[(GTid.x + x_max) + gsmThreads * (GTid.y + y_max)].isCluster = false;
	        
        }
    }
    else if(GTid.y >= groupthreads - clusterHalf) //Lower y bound only
    {
        //int y_max = min(DTid.y + clusterHalf, gridSizeXY - 1);
        int tmp2 = DTid.y + clusterHalf;
        int y_max = tmp2 > gridSizeXY - 1 ? gridSizeXY - 1 : tmp2;
        GSMCells[GTid.x + gsmThreads * GTid.y] = Cells[(y_max - 2) + gridSizeXY * DTid.x]; //Corner case
        GSMCells[GTid.x + gsmThreads * (GTid.y + y_max)].isCluster = false;
    }

    //Clamping out of bounds - I'm probably doing this wrong
    int temp_x = min(DTid.x, gridSizeXY - 1); 
    int temp_y = min(DTid.y, gridSizeXY - 1); 
    GSMCells[(GTid.x + clusterHalf) + gsmThreads * GTid.y] = Cells[DTid.y + gridSizeXY * temp_x];
    //GSMCells[(GTid.x + clusterHalf) + gsmThreads * GTid.y].isCluster = false; 

    GSMCells[GTid.x + gsmThreads * (GTid.y + clusterHalf)] = Cells[temp_y + gridSizeXY * DTid.x];
    //GSMCells[GTid.x + gsmThreads * (GTid.y + clusterHalf)].isCluster = false;

    GSMCells[(GTid.x + clusterHalf) + gsmThreads * (GTid.y + clusterHalf)] = Cells[temp_y + gridSizeXY * temp_x];
    //GSMCells[(GTid.x + clusterHalf) + gsmThreads * (GTid.y + clusterHalf)].isCluster = false;


    //------------------------------------------
    //Sync all threads
    GroupMemoryBarrierWithGroupSync(); 
  //---------------------------------------------
    //Actual code, now with GSM

    float B = Cells[cellIndex].B;
    float P = Cells[cellIndex].P;
    float N = Cells[cellIndex].N;
    float r = CS_OutputBuffer[cellIndex].r;
    float phi;

    //for (int x_i = 2; x_i < gsmThreads - 2; ++x_i) //gsmThreads = 20 -> 20 - 2? Is this right
    //{
    //    for (int y_i = 2; y_i < gsmThreads - 2; ++y_i)
    //    {
	   //     if (Cells[cellIndex].isCandidate)
    //        {
    //            int clusterX = GTid.x + x_i;
    //            int clusterY = GTid.y + y_i;
    //            r = CalcDistance(clusterX, clusterY,
				//						 GTid.x, GTid.y);
    //            r = pow(r, pow_rho);
    //            if (r == 0)
    //                N += 0;
    //            else
    //                N += 1.0f / r;
    //            phi = (1.0f / B) * (1.0f / N) * P;

    //            CS_OutputBuffer[cellIndex].N = N;
    //            CS_OutputBuffer[cellIndex].r = r;
    //            CS_OutputBuffer[cellIndex].phi = phi;
    //        }
    //        else
    //        {
    //            r = 0;
    //            phi = Cells[cellIndex].phi;

    //            CS_OutputBuffer[cellIndex].N = N;
    //            CS_OutputBuffer[cellIndex].r = r;
    //            CS_OutputBuffer[cellIndex].phi = phi;
    //        }
    //    }
            
    //}


    //for (int k = 2; k < gsmThreads - 2; ++k) //Only accesses 16 of the 20. 
    //{
    //Shouls I be using this??
    //const int CELL_R::
    int NEIGHBORS_X_DIFFERENCE[8] = {
        0, 1, 1, 1, 0, -1, -1, -1};
    //    const int CELL_R::
    int    NEIGHBORS_Y_DIFFERENCE[8] = {
            -1, -1, 0, 1, 1, 1, 0, -1};

    if (/*Cells[cellIndex].isCandidate*/
        GSMCells[GTid.x + gsmThreads * GTid.y].isCandidate)
    {
        for (int k = 2; k < 18; ++k) //Only accesses 16 of the 20. 
        {
            int indx = GTid.x + gsmThreads * GTid.y;
            //int indy = GTid.x + gsmThreads * k; 
            int gsmX = indx % 20;
            int gsmY = indx / 20;
            //gsmX += NEIGHBORS_X_DIFFERENCE[k]; 


            if (GSMCells[GTid.x + gsmThreads * GTid.y].isCluster)
			{
				r = CalcDistance(gsmX, gsmY,
										 GTid.x, GTid.y);
				r = pow(r, pow_rho);
				if (r == 0)
					N += 0;
				else
					N += 1.0f / r;
				phi = (1.0f / B) * (1.0f / N) * P;

				CS_OutputBuffer[cellIndex].N = N;
				CS_OutputBuffer[cellIndex].r = r;
				CS_OutputBuffer[cellIndex].phi = phi;
				
				
	          //no measure
			}
	              
        }
    }
    else
    {
        r = 0;
        phi = Cells[cellIndex].phi;

        CS_OutputBuffer[cellIndex].N = N;
        CS_OutputBuffer[cellIndex].r = r;
        CS_OutputBuffer[cellIndex].phi = phi;
    }
    //}
            
    

    

    //if (Cells[cellIndex].isCandidate)
    //{
    //    int indx = 0;

    //    int x_Pos = DTid.x;
    //    int y_Pos = DTid.y;

    //    for (int xOffset = -16; xOffset < 17; ++xOffset) //Go through neighbors
    //    {
    //        //-1, 0, 1 <- These loops will go through these values
    //        int new_xPos = x_Pos + xOffset;
    //        new_xPos = new_xPos < 0 ? 0 : new_xPos; //If new x pos is less than 0, cannot be in the grid, therefore 0
    //        new_xPos = new_xPos >= gridSizeXY - 1 ? gridSizeXY - 1 : new_xPos;

    //        for (int yOffset = -16; yOffset < 17; ++yOffset)
    //        {
    //            int new_YPos = y_Pos + yOffset;
    //            new_YPos = new_YPos < 0 ? 0 : new_YPos;
    //            new_YPos = new_YPos >= gridSizeXY - 1 ? gridSizeXY - 1 : new_YPos;

    //            //Calcs
    //            r = CalcDistance(new_xPos, new_YPos,
				//						 DTid.x, DTid.y);

    //            r = pow(r, pow_rho);
    //            if(r == 0)
    //                N += 0;
    //            else
    //        		N += 1.0f / r; 
                
    //        }
    //    }

    //	phi = (1.0f / B) * (1.0f / N) * P;

    //    CS_OutputBuffer[cellIndex].N = N;
    //    CS_OutputBuffer[cellIndex].r = r;
    //    CS_OutputBuffer[cellIndex].phi = phi;
        
    //}
    //else //No changes
    //{
    //    r = 0; 
    //    phi = Cells[cellIndex].phi;

    //    CS_OutputBuffer[cellIndex].N = N;
    //    CS_OutputBuffer[cellIndex].r = r;
    //    CS_OutputBuffer[cellIndex].phi = phi;
    //}

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