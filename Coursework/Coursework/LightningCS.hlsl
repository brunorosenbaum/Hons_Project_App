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
groupshared Cell GSMCells[gsmThreads][gsmThreads];
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

    uint cellIndex = DTid.y + gridSizeXY * DTid.x;

    //Subtract 2 from DTid to get the (0, 0) of the given 20x20 group in the greater grid (128x128)
    int x = DTid.x - 2;
    int y = DTid.y - 2;
    uint startIndex = y + gridSizeXY * x;
    uint gsmIndex = GTid.x + GTid.y * 20; //I have my doubts, won't it be 20 instead of 16?
    //Now, in our 16x16, we have [0~255] cells we have to sample with one or two threads
    //And a remaining 144, since (20x20 = 400) - 256 = 144.
    int gsmx = gsmIndex % 20; //Getting x and y pos in the 20x20 grid
    int gsmy = gsmIndex / 20;
    int x_GridIndex = x + gsmx; //Index of the cell we want to sample in the 400 grid
    int y_GridIndex = y + gsmy;
    //Get the cell
    GSM_Cells[y_GridIndex + 20 * x_GridIndex] = Cells[startIndex];

    gsmIndex += 256;
    if (gsmIndex < 400) //We sample again
    {
        gsmx = gsmIndex % 20;
        gsmy = gsmIndex / 20;
        y_GridIndex = y + gsmy;
        x_GridIndex = x + gsmx;
        GSM_Cells[y_GridIndex + 20 * x_GridIndex] = Cells[startIndex];

    }
    GroupMemoryBarrierWithGroupSync();

    //GSMCells[GTid.x + gsmThreads * GTid.y] = Cells[cellIndex];
    //Each thread loads one of the inner cells

    //GSM Array is 20x20, but the Thread Group is 16x16
    //First we assign the group id to our gsm
    //Say our thread group id is (5, 5)
    //Then number (5, 5) of our 20x20 grid is assigned the data stored in (5, 5) of our 16x16 grid
    //Therefore, by shifting by 2 our group thread id, we have the index on the 20x20 grid correspondent to it
    //int gsmx = GTid.x + 2;
    //int gsmy = GTid.y + 2; 
    //GSMCells[gsmx][gsmy] = Cells[cellIndex];
    //GSMCells[gsmx][gsmx].isCluster = true;
    //GSMCells[GTid.x][GTid.y].isCluster = true;
    //for (int i = 2; i < gsmThreads - 2; ++i)
    //{
    //    for (int j = 2; j < gsmThreads - 2; ++j)
    //    {
            
    //        GSMCells[i][j].isCluster = true; 
    //    }

    //}

   //     if (GI < 144) //If group index < 144 
   //     { //Thread loads a second cell
	    
   //     }

   // //-------------------------------------
   // //For Leftmost & upper bound
   // //If the group thread IN THE 20X20 is less than 2, which is what we've switched forward (indexes [0, y] or [1, y])
   // //It means this thread is out of bounds
   // if (GTid.x < clusterHalf) 
   // { 
   //     int tmp = DTid.x - clusterHalf; //
   // 	int x_min = tmp > 0 ? tmp : 0; //Set x as 0
   //     int valx = clusterHalf - GTid.x; //
   //     GSMCells[gsmx + valx][gsmy] = Cells[/*DTid.y + gridSizeXY * x_min*/cellIndex]; //GSMCells[x = 0, y = 0] is 0, 0 for 20x20 gsm cells
   //     GSMCells[GTid.x][GTid.y].isCluster = false;
   //     //GSMCells[GTid.x][GTid.y].isCandidate = false;
   //     if (GTid.y < clusterHalf) //Covers corners bc it includes Y
   //     {
			////If GTid.y < 4
   //         //Then this'd be true for GTid.x == 0, 1, 2, 3
   //         int tmp2 = DTid.y - clusterHalf; 
   //         int y_min = tmp2 > 0 ? tmp2 : 0; //Clamp at 0 since an index cannot be negative
   //         int valy = clusterHalf - GTid.y; 
   //         //Now the index in the GSM (20x20) cells is (y_min + 2) - it's shifted to fit the 16x16 grid
   //         //So if GTid.y == 2 (in a 16x16 grid) -> y_min == 0 -> GSMCells y = 2 (where the 16x16 starts)
   //         GSMCells[gsmx + valx][gsmy + valy] = Cells[/*y_min + gridSizeXY * x_min*/cellIndex]; //Corner case
   //         GSMCells[GTid.x][GTid.y].isCluster = false;
   //         //GSMCells[GTid.x][GTid.y].isCandidate = false;
   //     }
        
        
        
   // }
   // else if (GTid.y < clusterHalf) //Upper y axis only 
   // {
   //     //int y_min = max(DTid.y - clusterHalf, 0);
   //     int tmp = DTid.y - clusterHalf; 
   //     int y_min = tmp > 0 ? tmp : 0;
   //     int valy = clusterHalf - GTid.y; 
   //     GSMCells[gsmx][gsmy + valy] = Cells[/*y_min + gridSizeXY * DTid.x*/cellIndex]; //Corner case
   //     GSMCells[GTid.x][GTid.y].isCluster = false;
   //     //GSMCells[GTid.x][GTid.y].isCandidate = false;

   // }

   // //For rightmost & lower bound
   // if (GTid.x >= groupthreads - clusterHalf) 
   // {
   //     int tmp = DTid.x + clusterHalf;
   //     int x_max = tmp > groupthreads - 1 ? groupthreads - 1 : tmp;
   //     int valx = groupthreads - GTid.x; 
   //     GSMCells[gsmx - valx][gsmy] = Cells[/*DTid.y + gridSizeXY * x_max*/cellIndex];
   //     //GSMCells[GTid.x][GTid.y].isCandidate = false;
   //     GSMCells[GTid.x][GTid.y].isCluster = false; 


   //     if (GTid.y >= groupthreads - clusterHalf) //Covers corners bc it includes Y
   //     {
   //         //int y_max = min(DTid.y + clusterHalf, gridSizeXY - 1);
   //         int tmp2 = DTid.y + clusterHalf;
   //         int y_max = tmp2 > groupthreads - 1 ? groupthreads - 1 : tmp2;
   //         int valy = groupthreads - GTid.y; 
   //         GSMCells[gsmx - valx][gsmy - valy] = Cells[/*y_max + gridSizeXY * x_max*/cellIndex]; //Corner case
   //         //GSMCells[GTid.x][GTid.y].isCandidate = false;
   //         GSMCells[GTid.x][GTid.y].isCluster = false; 

   //     }
        
   // }
   // else if (GTid.y >= groupthreads - clusterHalf) //Lower y bound only
   // {
   //     //int y_max = min(DTid.y + clusterHalf, gridSizeXY - 1);
   //     int tmp2 = DTid.y + clusterHalf;
   //     int y_max = tmp2 > groupthreads - 1 ? groupthreads - 1 : tmp2;
   //     int valy = groupthreads - GTid.y; 
   //     GSMCells[gsmx][gsmy - valy] = Cells[/*y_max + gridSizeXY * DTid.x*/cellIndex]; //Corner case
   //     GSMCells[GTid.x][GTid.y].isCluster = false;
   //     //GSMCells[GTid.x][GTid.y].isCandidate = false;
   // }

    //Clamping out of bounds - I'm probably doing this wrong
    //int temp_x = min(DTid.x, gridSizeXY - 1); 
    //int temp_y = min(DTid.y, gridSizeXY - 1); 
    //GSMCells[GTid.x + clusterHalf][GTid.y] = Cells[DTid.y + gridSizeXY * temp_x];
    //GSMCells[GTid.x + clusterHalf][GTid.y].isCandidate = false; 

    //GSMCells[GTid.x][GTid.y + clusterHalf] = Cells[temp_y + gridSizeXY * DTid.x];
    //GSMCells[GTid.x][GTid.y + clusterHalf].isCandidate = false; 

    //GSMCells[GTid.x + clusterHalf][GTid.y + clusterHalf] = Cells[temp_y + gridSizeXY * temp_x];
    //GSMCells[GTid.x + clusterHalf][GTid.y + clusterHalf].isCandidate = false; 
    

    ////------------------------------------------
    ////Sync all threads
    //GroupMemoryBarrierWithGroupSync(); 
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
    if (/*GSM_Cells[startIndex].isCandidate*/
        Cells[cellIndex].isCandidate)
    {
        for (int i = 0; i < 16; ++i)
        {
	    
            int x_i = (i % 16) + gsmx;
            int y_i = (i / 16) + gsmy;
            r = CalcDistance(x_i, y_i,
									 gsmx, gsmy);
            r = pow(r, pow_rho);
            if (r == 0)
            {
                N += 0;
                phi += 0;
            }
            else
            {
                N += 1.0f / r;
                phi = (1.0f / B) * (1.0f / N) * P;
            }
                        

            CS_OutputBuffer[cellIndex].N = N;
            CS_OutputBuffer[cellIndex].r = r;
            CS_OutputBuffer[cellIndex].phi = phi;
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

}

    //    uint2 minIndex = uint2(GTid.x, GTid.y);
    //uint2 maxIndex = uint2(GTid.x + 4, GTid.y + 4);

    //if (GSM_Cells[startIndex].isCandidate)
    //{
    //    for (uint y = minIndex.y; y < maxIndex.y; ++y)
    //    {
    //        for (uint x = minIndex.x; x < maxIndex.x; ++x)
    //        {
    //            //if (GSMCells[x][y].isCluster)
    //            //{
    //                r = CalcDistance(x, y,
				//					 gsmx, gsmy); //15, 0
    //                r = pow(r, pow_rho);
    //                if (r == 0)
    //                {
	   //                 N += 0;
    //                    phi += 0;
    //                }
                        
    //                else
    //                {
	   //                 N += 1.0f / r;
    //            		phi = (1.0f / B) * (1.0f / N) * P;
    //                }
                        

    //                CS_OutputBuffer[cellIndex].N = N;
    //                CS_OutputBuffer[cellIndex].r = r;
    //                CS_OutputBuffer[cellIndex].phi = phi;
    //            //}
               
    //        }

    //    }
    //}
    //else
    //{
    //    r = 0;
    //    phi = Cells[cellIndex].phi;

    //    CS_OutputBuffer[cellIndex].N = N;
    //    CS_OutputBuffer[cellIndex].r = r;
    //    CS_OutputBuffer[cellIndex].phi = phi;
    //}

        //for (int i = 0; i < 20 * 20; ++i)
        //{
        //    if ( /*Cells[cellIndex].isCandidate*/
        //GSMCells[i].isCandidate)
        //    {
            
        //        int indx = GTid.x + gsmThreads * GTid.y;
        //        indx += i;

        //        int gsmX = indx % (16 * 16);
        //        int gsmY = indx / (16 * 16);
        ////gsmX += NEIGHBORS_X_DIFFERENCE[k]; 
        //    //gsmX += GTid.x;
        //    //gsmY += GTid.y;
        //        if (GSMCells[indx].isCluster)
        //        {
        //            r = CalcDistance(gsmX, gsmY,
								//	 GTid.x, GTid.y); //15, 0
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
           
	              
        //    }
        //    else
        //    {
        //        r = 0;
        //        phi = Cells[cellIndex].phi;

        //        CS_OutputBuffer[cellIndex].N = N;
        //        CS_OutputBuffer[cellIndex].r = r;
        //        CS_OutputBuffer[cellIndex].phi = phi;
        //    }
        //}
        
    //}
            
	//if(GI >= clusterHalf && GI <(groupthreads - clusterHalf) &&
 //       (Gid.x * (groupthreads - 2 * clusterHalf) + GI - clusterHalf) < gridSizeXY &&
 //       (Gid.y * (groupthreads - 2 * clusterHalf) + GI - clusterHalf) < gridSizeXY)
	//{
 //       for (int i = 0; i < 400; ++i)
 //       {
 //           if (test_GSM[GI + i].isCandidate)
 //           {
 //               //if (GSMCells[x][y].isCluster)
 //               //{
 //               int gsmX = (GI + i) % (16 * 16);
 //               int gsmY = (GI + i) / (16 * 16);
 //                       r = CalcDistance(gsmX, gsmY,
	//								 GTid.x, GTid.y); //15, 0
 //                       r = pow(r, pow_rho);
 //                       if (r == 0)
 //                           N += 0;
 //                       else
 //                           N += 1.0f / r;
 //                       phi = (1.0f / B) * (1.0f / N) * P;

 //                       CS_OutputBuffer[cellIndex].N = N;
 //                       CS_OutputBuffer[cellIndex].r = r;
 //                       CS_OutputBuffer[cellIndex].phi = phi;
 //               //}
               
                   
 //           }
 //           else
 //           {
 //               r = 0;
 //               phi = Cells[cellIndex].phi;

 //               CS_OutputBuffer[cellIndex].N = N;
 //               CS_OutputBuffer[cellIndex].r = r;
 //               CS_OutputBuffer[cellIndex].phi = phi;
 //           }
 //       }

 //   }    



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
  
