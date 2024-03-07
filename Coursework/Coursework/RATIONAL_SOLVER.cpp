#include "RATIONAL_SOLVER.h"

#include <iostream>
#include <ostream>
#include <random>

#define ETA 2 //Eta (H or squiggly n) parameter
#define POW_OF_RHO 3 //Rho is P, or p parameter. 

RATIONAL_SOLVER::RATIONAL_SOLVER() :
eta_(ETA), power_of_Rho_(POW_OF_RHO), gridSize_(0), clusterSize_(0)
{
	//Clear all vectors
	boundary_Cells.clear();
	positive_Cells.clear();
	negative_Cells.clear();

	candidate_Cells.clear();
	all_Cells.clear(); 

	//Rng probably here
	std::random_device	rd;
	rng_.seed(rd());
}

RATIONAL_SOLVER::~RATIONAL_SOLVER()
{
	ClearVectors(); 

}

bool RATIONAL_SOLVER::LoadMap(const std::string& path)
{
	return false;
}

#pragma region PRECOMPUTATION FUNCTIONS
//-----------------------------------------------------------PRECOMPUTATION---------------------------------------------
void RATIONAL_SOLVER::CreateBoundaryCells() //I'm assuming this method creates the boundary grid, akin to the green channel
//in the .ppm files in Kims method. These should be positive in charge. 
{ //I think this is the equivalent to the quad_dbm_2D's drawQuadTree() 
	boundary_Cells.clear();
	boundary_Cells.reserve(gridSize_ * 4 + 4);

	//They don't use a pointer in here... this is gonna cause a memory leak isn't it
	CELL_DERV* current_cell = new CELL_DERV(0, 0, 0, 0, nullptr, 0); 
	//Add boundary charges
	for(int i = 0; i < gridSize_; ++i) //Initialize xy coords of grid
	{
		//Horizontal boundary cells
		current_cell->center[0] = i; current_cell->center[1] = 0;
		boundary_Cells.push_back(*current_cell); //And add to vector

		//if (1 == m_vEndCells.size()) //This code will only execute if there is one endcell. 
		//{ //I don't get it yet
		current_cell->center[0] = i;			current_cell->center[1] = gridSize_;		boundary_Cells.push_back(*current_cell);
		//}

		//Vertical boundary cells
		current_cell->center[0] = 0; current_cell->center[1] = i; 
		boundary_Cells.push_back(*current_cell); //And add to vector

		current_cell->center[0] = gridSize_; current_cell->center[1] = i; 
		boundary_Cells.push_back(*current_cell); //And add to vector

	}
	//4 corners - clockwise
	current_cell->center[0] = 0; current_cell->center[1] = 0; //Top left
	boundary_Cells.push_back(*current_cell); 

	current_cell->center[0] = gridSize_; current_cell->center[1] = 0; //Top right
	boundary_Cells.push_back(*current_cell); 

	current_cell->center[0] = gridSize_; current_cell->center[1] = gridSize_; //Bottom right
	boundary_Cells.push_back(*current_cell); 

	current_cell->center[0] = 0; current_cell->center[1] = gridSize_; //Bottom left
	boundary_Cells.push_back(*current_cell);
}

void RATIONAL_SOLVER::CalcBoundaryPotential()
{
	//Before we compute the potentials, we divide the charges
	//	into three types : positive charges, negative charges along the
	//	lightning path, and **boundary charges**.We then calculate the
	//	electric potentials based on those types separately as P, N, and **B**.

	/*m_vBoundaryPotential.clear();*/ //This was a vector of floats of the potentials,
	//But since we're using whole cell ptrs here we don't need to clear as these potentials
	//are all initialized to 0 upon creation
	/*m_vBoundaryPotential.reserve(m_iGridSize * m_iGridSize);*/ //<- Shouldn't be an issue. This space is already reserved in prev function

	int iIndex = 0;
	CELL_DERV* current_Cell;
	float boundaryPhi; // Called B in the other code and in the formula
	float r; //Is B the RHS, or phi? 

	for (int i = 0; i < gridSize_; ++i)
	{
		for (int j = 0; j < gridSize_; ++j)
		{
			current_Cell = all_Cells[iIndex];
			boundaryPhi = 0;

			if (current_Cell)
			{
				std::vector< CELL_DERV >::const_iterator bItr = boundary_Cells.begin();
				while (bItr != boundary_Cells.end()) //Iterate through boundary cells
				{
					//r = distance (scalar value) between all cells (i think) and boundary cells
					r = CalcDistance(bItr->center[0], bItr->center[1], current_Cell->center[0], current_Cell->center[1]);
					if (power_of_Rho_ > 1) //RHO is the p parameter to prevent excessive branching TODO: REREAD THIS
					{//This condition will always be true then bc it's initialized by a macro 
						r = pow(r, power_of_Rho_); //Square the distance
					}

					boundaryPhi += 1.0f / r; //Add 1/squared distance between cells to B, which is the potential. 

					++bItr;
				}
			}

			boundary_Cells[iIndex].potential = boundaryPhi; //B is the potential. Set it to that for each of the boundary cells
			//TODO: THIS IS GONNA THROW ERRORS. I KNOW THIS IS WRONG.
			//TODO: THIS IS NOT GOING TO GO THROUGH ALL THE BOUNDARY CELLS BC IT'S A NESTED LOOP. BE CAREFUL!! ASK!!

			++iIndex;
		}
	}
}

void RATIONAL_SOLVER::CalcPositivePotential()
{
	//Before we compute the potentials, we divide the charges
	//	into three types : **positive charges**, negative charges along the
	//	lightning path, and boundary charges.We then calculate the
	//	electric potentials based on those types separately as **P**, N, and B.

	
	//m_vPositivePotential.assign(m_iGridSize * m_iGridSize, 0.0f); //Assign value 0 to all positive elec potential cells
	CELL_DERV* current_Cell; 
	float positivePhi = 0;
	float r;
	int iIndex = 0;


	for (int i = 0; i < gridSize_; ++i)
	{
		for(int j = 0; j < gridSize_; ++j)
		{
			current_Cell = all_Cells[iIndex];

			/*if (pCell && E_CT_END != pCell->m_eType)*/
			if (current_Cell && current_Cell->state != ATTRACTOR) //Go through cells until they reach end cell
			{
				std::vector< CELL_DERV >::const_iterator pItr = positive_Cells.begin();
				while (pItr != positive_Cells.end())
				{
					r = CalcDistance(pItr->center[0], pItr->center[1], current_Cell->center[0], current_Cell->center[1]);
					if (power_of_Rho_ > 1)
					{
						r = pow(r, power_of_Rho_);
					}
					//Set their potential to 1
					positivePhi += 1.0f / r;

					++pItr;
				}
			}
		} 

		positive_Cells[iIndex].potential = positivePhi;
		iIndex++; 
	}
}
#pragma endregion

#pragma region ORGANIZATION_FUNCTIONS
void RATIONAL_SOLVER::CreateClusterMap(int clusterSize)
{

	clusterSize_ = clusterSize; //size of multi scaled cluster grid map ( 8 x 8 ) = 16. Remember the grid is 64 x 64

	int regionSize = gridSize_ / clusterSize_; //64x64/8x8 = 4096/16 = 256.
	//Meaning there are 256 clusters in a 64x64 map

	clusters_.clear();
	clusters_.reserve(clusterSize_ * clusterSize_); //Make the cluster vector 16x16 = 256 (same as region size)

	CLUSTER current_cluster;

	//Initialize x and y of cell clusters in cluster vector
	for(int i = 0; i < clusterSize_; ++i) //y 
	{
		for (int j = 0; j < clusterSize_; ++j) //x
		{
			current_cluster.c_x = j;
			current_cluster.c_y = i;
			clusters_.push_back(current_cluster); 
		}
	}
	int x, y;
	int iIndex;

	auto negItr = negative_Cells.begin();
	while(negItr != negative_Cells.end())
	{
		x = negItr->center[0] / regionSize; //x = x.pos/256
		y = negItr->center[1] / regionSize; //y = y.pos/256

		iIndex = y * clusterSize_ + x;
		clusters_[iIndex].cluster_Cells.push_back(*negItr); //Fill all the clusters with the negative cells

		clusters_[iIndex].c_xSum += negItr->center[0]; //Add the xy coords of negative cells to clusters
		clusters_[iIndex].c_ySum += negItr->center[1]; //This is to calc the average xy pos

		//Calc avg xy pos of each cluster
		clusters_[iIndex].c_xAvg = (float)clusters_[iIndex].c_xSum / clusters_[iIndex].cluster_Cells.size(); 
		clusters_[iIndex].c_yAvg = (float)clusters_[iIndex].c_ySum / clusters_[iIndex].cluster_Cells.size(); 
		++negItr; 
	}

}
#pragma endregion

#pragma region COMPUTATION FUNCTIONS
//-----------------------------------------------COMPUTATION FUNCTIONS----------------------------------------------------
bool RATIONAL_SOLVER::InitializeGrid(const std::string& path) //Load()
{
	ClearVectors();

	bool result = LoadMap(path);
	if(result)
	{
		CreateBoundaryCells();//Create boundary grid cells (just position, coords)
		CalcBoundaryPotential(); //Calc phi of all cells.
		CalcPositivePotential();//Negative is not needed since cells default phi is 0. 

		//Create clusters
		CreateClusterMap(clusterSize_); 

		//Generate initial candidate map
		CreateCandidateMap(); 
		//And calculate phi for candidate cells (here's the rational method)
		CalcPotential_Rational();
	}
	return result; 
}

void RATIONAL_SOLVER::CreateCandidateMap() //Map refers to the C++ MAP DATA STRUCTURE.
//Maps index of all cells vector to elements of candidate cells vector.
{
	candidateMap_DS.clear();
	int iIndex;
	int cellX, cellY; //All cells vector x and y pos
	int candidateX, candidateY; //Candidate cells vector x and y pos
	int iChildIndex;

	std::vector< CELL_DERV >::iterator itr = negative_Cells.begin(); //Go through negative cells
	while (itr != negative_Cells.end())
	{
		cellX = (*itr).center[0]; 
		cellY = (*itr).center[1];
		iIndex = cellY * gridSize_ + cellX;

		if(all_Cells[iIndex]) //Go through all cells
		{
			//Go through neighbors of each cell & update the candidates' xy coords to that
			for(auto n : *all_Cells[iIndex]->neighbors)
			{
				//Candidate's xPos = current cell's xPos + n neighbor's xPos
				candidateX = cellX + all_Cells[n]->neighbors[n]->center[0];
				//Candidate's yPos = current cell's yPos + n neighbor's yPos
				candidateX = cellX + all_Cells[n]->neighbors[n]->center[1];
				iChildIndex = cellY * gridSize_ + cellX;

				//If cells xy coords are within the gridsize
				if(cellX >= 0 && cellX < gridSize_ && cellY >= 0 && cellY < gridSize_ && all_Cells[iChildIndex])
				{
					if(all_Cells[iChildIndex]->state == EMPTY) //If cell is empty
					{
						//Find each iChildIndex element in candidate cell map
						std::map<int, CELL_DERV*>::iterator candidate_Itr = candidateMap_DS.find(iChildIndex);
						if(candidate_Itr == candidateMap_DS.end())
						{
							//Insert cell at the end of the map
							candidateMap_DS.insert(std::map<int, CELL_DERV*>::value_type(iChildIndex, all_Cells[iChildIndex]));
						}
					}
				}

			}
		}
		++itr; 
	}


}

void RATIONAL_SOLVER::CalcPotential_Rational()
{
	unsigned int totalCells = gridSize_ * gridSize_; //4096 cells
	if(all_Cells.size() < totalCells)
	{
		//Cell size error
		std::cout << "cell size error !! " << std::endl;
		return; 
	}

	// calculate electric potential (Phi) for only candidate cells
	float phi; 
	float r; //Distance of cells between each other
	float B, N, P; //Boundary, negative, positive phi. These are the ones from the formula.

	int regionSize = gridSize_ / clusterSize_; 
	int iClusterIndex; //TODO: THIS SEEMS KINDA IFFY TO HAVE INDEXES NO?
	int iCandidateClusterX, iCandidateClusterY, iCandidateClusterIndex; 

	int mapKey; //Value that'll be assigned to the int map key of the candidate cells map.
	CELL_DERV* current_Cell;

	if(candidateMap_DS.size() == 0)
	{
		std::cout << "There is no candidate cells to compute electric potential !!" << std::endl;
		return;
	}

	//Now, go through candidate cells
	auto mapItr = candidateMap_DS.begin();
	while(mapItr != candidateMap_DS.end())
	{
		//Assign to map values
		mapKey = mapItr->first;
		current_Cell = mapItr->second;

		if(current_Cell && current_Cell->state == EMPTY) //If current cell is empty
		{
			// -----------------------------------------------------------
			// for boundaries, use pre-computed values
			B = boundary_Cells[mapKey].potential;
			// -----------------------------------------------------------
			// for positive charges, use pre-computed value
			P = positive_Cells[mapKey].potential;
			// -----------------------------------------------------------
			// for negative charge cells 
			iCandidateClusterX = current_Cell->center[0] / regionSize; //X
			iCandidateClusterY = current_Cell->center[1] / regionSize; //Y
			iCandidateClusterIndex = iCandidateClusterY * clusterSize_ + iCandidateClusterX; 
			iClusterIndex = 0; 

			N = 0; //(phi = 0)

			//Use clustered cells to calculate equation 4.
			//Theory: Potential depends on distance to each type of cell.
			//A potential at a point x that satisfies Laplace's eq = avg potential of a 'virtual' sphere at point x
			//This potential is described by eq 3, but tldr doesn't generate interesting branching.
			//This is bc potentials between candidates and other cells can be very similar.
			//Instead we cluster cells together and use eq. 4:
			//Where approx potential = (1/r)^rho; r = distance between a cell and other cluster cells

			//Remember each cluster is 16 columns x 16 rows.
			for (int cy = 0; cy < clusterSize_; cy++) //Cluster columns
			{
				for (int cx = 0; cx < clusterSize_; cx++) //Cluster rows
				{
					if(!clusters_[iClusterIndex].cluster_Cells.empty()) //If there's cells in the cluster
					{
						if(iClusterIndex != iCandidateClusterIndex) //If negative charge cells are NOT in the same cluster
						{
							r = CalcDistance(clusters_[iClusterIndex].c_xAvg, //Distance is the magnitude between
								clusters_[iClusterIndex].c_yAvg,//current cluster's average xy
								current_Cell->center[0], //and current cell's xy
								current_Cell->center[1]);
							if(power_of_Rho_ > 1) //r = r^p
							{
								r = pow(r, power_of_Rho_); 
							}
							//TODO: FIND OUT
							//We add [cluster size? idk if 16 help TT__TT]/r to the negative potential
							N += clusters_[iClusterIndex].cluster_Cells.size() / r;
						}
						else //For negative cells in the same cluster
						{
							auto nItr = clusters_[iClusterIndex].cluster_Cells.begin();
							while(nItr != clusters_[iClusterIndex].cluster_Cells.end())
							{
								r = CalcDistance(nItr->center[0], nItr->center[1], //Distance is magnitude between
									current_Cell->center[0], current_Cell->center[1]); //Cluster's xy coords and current cell's
								if(power_of_Rho_>1)
								{
									r = pow(r, power_of_Rho_);
								}
								N += 1.0f / r; //Eq. 4 relationship.
								++nItr; 
							}

						}
					}
					++iClusterIndex; 
				}
			}

			//Use equation 5: PHI = P / (N X B)
			phi = (1.0f / B) * (1.0f / N) * P;

			//Because we divide positive potentials with those of negative ones,
			////we can generate stronger negative potentials among nearby negative charges.
			current_Cell->N_ = N;
			current_Cell->P_ = P;
			current_Cell->B_ = B;
			current_Cell->potential = phi;
		}
		++mapItr; 
	}

}

void RATIONAL_SOLVER::CalcPotential_Rational_SingleCell(CELL_DERV* candidate_cell)
{
	// calculate electric potential (Phi) for only candidate cells
	float phi;
	float r; //Distance of cells between each other
	float B, N, P; //Boundary, negative, positive phi. These are the ones from the formula.

	int regionSize = gridSize_ / clusterSize_;
	int iClusterIndex; 
	int iCandidateClusterX, iCandidateClusterY, iCandidateClusterIndex;


	if (candidate_cell && candidate_cell->state == EMPTY) //If current cell is empty
	{
		int iKey = candidate_cell->center[1] * gridSize_ + candidate_cell->center[0];

		// -----------------------------------------------------------
		// for boundaries, use pre-computed values
		B = boundary_Cells[iKey].potential;
		// -----------------------------------------------------------
		// for positive charges, use pre-computed value
		P = positive_Cells[iKey].potential;
		// -----------------------------------------------------------
		// for negative charge cells 
		iCandidateClusterX = candidate_cell->center[0] / regionSize; //X
		iCandidateClusterY = candidate_cell->center[1] / regionSize; //Y
		iCandidateClusterIndex = iCandidateClusterY * clusterSize_ + iCandidateClusterX;
		iClusterIndex = 0;

		N = 0; //(phi = 0)

		for (int cy = 0; cy < clusterSize_; cy++) //Cluster columns
		{
			for (int cx = 0; cx < clusterSize_; cx++) //Cluster rows
			{
				if (!clusters_[iClusterIndex].cluster_Cells.empty()) //If there's cells in the cluster
				{
					if (iClusterIndex != iCandidateClusterIndex) //If negative charge cells are NOT in the same cluster
					{
						r = CalcDistance(clusters_[iClusterIndex].c_xAvg, //Distance is the magnitude between
							clusters_[iClusterIndex].c_yAvg,//current cluster's average xy
							candidate_cell->center[0], //and current cell's xy
							candidate_cell->center[1]);
						if (power_of_Rho_ > 1) //r = r^p
						{
							r = pow(r, power_of_Rho_);
						}
						//We add [cluster size? idk if 16 help TT__TT]/r to the negative potential
						N += clusters_[iClusterIndex].cluster_Cells.size() / r;
					}
					else //For negative cells in the same cluster
					{
						auto nItr = clusters_[iClusterIndex].cluster_Cells.begin();
						while (nItr != clusters_[iClusterIndex].cluster_Cells.end())
						{
							r = CalcDistance(nItr->center[0], nItr->center[1], //Distance is magnitude between
								candidate_cell->center[0], candidate_cell->center[1]); //Cluster's xy coords and current cell's
							if (power_of_Rho_ > 1)
							{
								r = pow(r, power_of_Rho_);
							}
							N += 1.0f / r; //Eq. 4 relationship.
							++nItr;
						}

					}
				}
				++iClusterIndex;
			}
		}

		//Use equation 5: PHI = P / (N X B)
		phi = (1.0f / B) * (1.0f / N) * P;

		//Because we divide positive potentials with those of negative ones,
		////we can generate stronger negative potentials among nearby negative charges.
		candidate_cell->N_ = N;
		candidate_cell->P_ = P;
		candidate_cell->B_ = B;
		candidate_cell->potential = phi;
	}

}

void RATIONAL_SOLVER::Calc_Normalization()
{
	/*Once we have computed the electric potential between the
	candidate cells and other charged cells, we use the normalization equation, Equation 2.*/
	//I wonder if this is it?  If so, it calculates P(i), the probability of selection of each candidate cell

	int iIndex;

}

bool RATIONAL_SOLVER::SelectCandidate(CELL_DERV& outNextCell) //Choose next lightning cell among candidates
{ //Done by calculating P(i)
	outNextCell.center[0] = 0;
	outNextCell.center[1] = 0;

	bool result = true;
	int iIndex = 0;

	std::vector<float> selection_Probability; //Vector of P(i)

	auto itr = candidate_Cells.begin();
	while(itr != candidate_Cells.end())
	{
		//P(i) is calculated with equation 2 (Y) or eq. 10 (K), and represents the probability of selection.
		//Apply eta parameter (squiggly n, for branching)
		//And store into a vector
		selection_Probability.push_back(pow(fabs(itr->b), eta_));
		++itr; 
	}
	//Make a discrete distribution, with the weights being their selection probabilities.
	std::discrete_distribution<> distribution(selection_Probability.begin(), selection_Probability.end());

	iIndex = distribution(rng_); //iIndex is a random index from the distribution
	int iErrorCount = 0;

	while(candidate_Cells.size() == iIndex) //So iIndex is the same size as candidate cells, right?
	{//I think this has to do with.. candidates not being suitable
		++iErrorCount;
		std::cout << "Index error !!!!!" << std::endl;
		if(iErrorCount >= 10)
		{
			result = false;
			break; 
		}
		iIndex = distribution(rng_); 
	}
	if (result) //If result is true and there are no errors, select the (random index)th from the candidate list
		outNextCell = candidate_Cells[iIndex]; //To be the next lightning cell

	return result; 
}
#pragma endregion

#pragma region UPDATING FUNCTIONS
//-------------------------------------------UPDATING FUNCTIONS----------------------------------------------------------
void RATIONAL_SOLVER::AllCellsToCandidates() //Turns all cells into candidates. Will be called in main. 
{
	candidateMap_DS.clear();
	int iIndex;

	for(int i = 0; i < gridSize_; ++i) //Columns
	{
		for (int j = 0; j < gridSize_; ++j) //Rows
		{
			iIndex = i * gridSize_ + j;
			if(all_Cells[iIndex] && all_Cells[iIndex]->state == EMPTY) //If cells exist and theyre empty
			{//Map all cells to candidate map
				candidateMap_DS.insert(std::map<int, CELL_DERV*>::value_type(iIndex, all_Cells[iIndex]));
			}
		}
	}
}

void RATIONAL_SOLVER::UpdateCandidates()
{
	//Pretty much the same as CreateCandidateMap()
	candidate_Cells.clear();
	int iIndex;
	int p_x, p_y; //Potential cells x & y
	int c_x, c_y; //Candidate cells x & y
	int iNeighborIndex; //Neighbor cell index

	auto itr = negative_Cells.begin();
	while(itr != negative_Cells.end())
	{
		p_x = itr->center[0]; 
		p_y = itr->center[1];
		iIndex = p_y * gridSize_ + p_x;

		if(all_Cells[iIndex])
		{
			//Check neighbors
			for(auto n : *all_Cells[iIndex]->neighbors){
				c_x = p_x + all_Cells[n]->neighbors[n]->center[0];
				c_y = p_y + all_Cells[n]->neighbors[n]->center[1];
				iNeighborIndex = c_y * gridSize_ + c_x;

				if (c_x >= 0 && c_x < gridSize_
					&& c_y >= 0 && c_y < gridSize_
					&& all_Cells[iNeighborIndex])
				{
					if(all_Cells[iNeighborIndex]->state == EMPTY) 
					{
						if(all_Cells[iNeighborIndex]->potential != 0)
						{
							int diff_x = p_x - c_x; //Differences in position between potential cells and candidate
							int diff_y = p_y - c_y;

							int x_index = c_y * gridSize_ + (c_x + diff_x);
							int y_index = (c_y + diff_y) * gridSize_ + c_x;

							bool xEmpty = true;
							bool yEmpty = true;

							if(x_index >= 0 && x_index < gridSize_ * gridSize_ && all_Cells[x_index]->state == REPULSOR)
							{ //Cell is at the start. For x
								xEmpty = false; 
							}
							if (y_index >= 0 && y_index < gridSize_ * gridSize_ && all_Cells[y_index]->state == REPULSOR)
							{ //Cell is at the start. For x
								yEmpty = false;
							}
							if(xEmpty || yEmpty) //If cell is feasible to be a candidate
							{
								//Set its xy, parents xy and phi to new ones
								CELL_DERV candidate(0, 0, 0, 0);
								candidate.center[0] = c_x; 
								candidate.center[1] = c_y;
								candidate.parent->center[0] = p_x; 
								candidate.parent->center[1] = p_y;
								candidate.b = all_Cells[iNeighborIndex]->potential;
								candidate_Cells.push_back(candidate); //And push to candidate cells
							}

						}
					}
				}
			}
		}
		++itr; 
	}

}

void RATIONAL_SOLVER::UpdateCandidateMap(const CELL_DERV& next_Cell)
{
	//Same as calcPotential_Rational()
	int x = next_Cell.center[0];
	int y = next_Cell.center[1];
	int iIndex = y * gridSize_ + x;

	//Remove cell from candidate map
	auto mapItr = candidateMap_DS.find(iIndex);
	if(mapItr != candidateMap_DS.end())
	{
		candidateMap_DS.erase(iIndex); 
	}

	//Update electric potential (phi) for candidate cells
	float r;
	CELL_DERV* candidate_Cell;
	auto itr = candidateMap_DS.begin();
	while(itr != candidateMap_DS.end()) //Go through mapped candidates
	{
		candidate_Cell = itr->second;
		if(candidate_Cell)
		{
			r = CalcDistance(x, y, candidate_Cell->center[0], candidate_Cell->center[1]);
			if(power_of_Rho_>1)
			{
				r = pow(r, power_of_Rho_);
			}
			candidate_Cell->N_ += 1.0f / r; //Last negative potential
			//P / (B · N)
			candidate_Cell->potential = (1.0f / candidate_Cell->B_) * (1.0f / candidate_Cell->N_) * candidate_Cell->P_; 
		}
		++itr; 
	}

	//And now add candidate cells for new lightning cell
	if(all_Cells[iIndex])
	{
		int c_x, c_y;
		int iChildIndex;

		//Check neighbors
		for (auto n : *all_Cells[iIndex]->neighbors)
		{
			c_x = x + all_Cells[n]->neighbors[n]->center[0]; //Update xy coords
			c_y = y + all_Cells[n]->neighbors[n]->center[1];
			iChildIndex = c_y * gridSize_ + c_x;

			if(c_x >= 0 && c_x < gridSize_
				&& c_y>= 0 && c_y < gridSize_
				&& all_Cells[iChildIndex])
			{
				if(all_Cells[iChildIndex]->state != EMPTY)
				{
					auto itr = candidateMap_DS.find(iChildIndex);
					if(candidateMap_DS.end() == itr)
					{ //Insert at the end
						candidateMap_DS.insert(std::map<int, CELL_DERV*>::value_type(iChildIndex, all_Cells[iChildIndex]));
						// calculate electric potential for newly added candidate cells
						CalcPotential_Rational_SingleCell(all_Cells[iChildIndex]); 
					}
				}
			}

		}
	}

}

void RATIONAL_SOLVER::UpdateClusterMap(const CELL_DERV& next_Cell)
{
	int iIndex;
	int x, y;
	int iRegionSize = gridSize_ / clusterSize_;

	x = next_Cell.center[0] / iRegionSize; 
	y = next_Cell.center[1] / iRegionSize;
	iIndex = y * clusterSize_ + x;

	clusters_[iIndex].cluster_Cells.push_back(next_Cell);
	clusters_[iIndex].c_xSum += next_Cell.center[0];
	clusters_[iIndex].c_ySum += next_Cell.center[1];
	clusters_[iIndex].c_xAvg = (float)clusters_[iIndex].c_xSum / clusters_[iIndex].cluster_Cells.size();
	clusters_[iIndex].c_yAvg = (float)clusters_[iIndex].c_ySum / clusters_[iIndex].cluster_Cells.size();
}

#pragma endregion

#pragma region LIGHTNING FUNCTIONS
void RATIONAL_SOLVER::AddNewLightningPath(const CELL_DERV& newPath, bool bIsEndCell, bool bTargetCell)
{
	int iIndex = newPath.center[1] * gridSize_ + newPath.center[0]; //Grid index

	if(!bIsEndCell) //If not end cell
	{
		//Update cell type
		if(iIndex >= 0 && iIndex < all_Cells.size())
		{
			if(all_Cells[iIndex] && all_Cells[iIndex]->state == EMPTY)
			{//Only empty cells can be changed to other types
				
				all_Cells[iIndex]->state = NEGATIVE; //Set to negative
				negative_Cells.push_back(*all_Cells[iIndex]); //And add to negative cells vector
			}
		}
	}
	//Add lightning node to tree

}


bool RATIONAL_SOLVER::ProcessLightning()
{
	bool isLooping = true;
	while(isLooping)
	{
			UpdateCandidates(); //Get candidates

		//And select next cell from candidates
		CELL_DERV next_Cell(0, 0, 0, 0);
		bool result_ = SelectCandidate(next_Cell); 

		if(result_)
		{
			int iEndX, iEndY; 
			if(IsNearEndCell(next_Cell.center[0], next_Cell.center[1], iEndX, iEndY)) //If lightning is near end cell (attractor)
			{
				isLooping = false;
				AddNewLightningPath();
				//Add final target position
				next_Cell.parent->center[0] = next_Cell.center[0]; 
				next_Cell.parent->center[1] = next_Cell.center[1];
				next_Cell.center[0] = iEndX; 
				next_Cell.center[1] = iEndY;
				AddNewLightningPath(); 
			}
			else
			{
				AddNewLightningPath();
				UpdateClusterMap(next_Cell);
				UpdateCandidateMap(next_Cell);
			}
		}
	}
	return true;
}
#pragma endregion

#pragma region HELPER FUNCTIONS
void RATIONAL_SOLVER::ClearVectors()
{
	std::vector< CELL_DERV* >::iterator itr = all_Cells.begin();
	while (itr != all_Cells.end())
	{
		if (*itr) //Safe delete
		{
			delete* itr;
			*itr = nullptr;
		}
		++itr;
	}
	boundary_Cells.clear();
	positive_Cells.clear();
	negative_Cells.clear();
	candidate_Cells.clear();
	all_Cells.clear();
}
bool RATIONAL_SOLVER::IsNearEndCell(int x, int y, int& outEndX, int& outEndY) const
{
	//Same code as previous functions but instead of checking if cell is empty you check if cell is an attractor
	bool result = false;
	outEndX = 0; outEndY = 0;

	if(x >= 0 && x < gridSize_ && y >= 0 && y < gridSize_)
	{
		int c_x, c_y;
		int iIndex;
		std::vector<int> outEndX_vector; std::vector<int> outEndY_vector;

		//Check neighbors
		for (auto n : *all_Cells[iIndex]->neighbors)
		{
			c_x = x + all_Cells[n]->neighbors[n]->center[0];
			c_y = y + all_Cells[n]->neighbors[n]->center[1];
			iIndex = c_y * gridSize_ + c_x;
			if(c_x >= 0 && c_x < gridSize_ && c_y >= 0 && gridSize_ 
				&& all_Cells[iIndex]) //Check cells
			{
				if( all_Cells[iIndex]->state == ATTRACTOR)
				{
					result = true;
					////Add candidate coords to vectors of end x and end y pos
					outEndX_vector.push_back(c_x);
					outEndY_vector.push_back(c_y);
				}
			}
			if(outEndX_vector.size() > 0) //If Vector is filled
			{
				int iCandidateIndex = 0;
				if(outEndX_vector.size() > 1) //If there's at least more than 1 element
				{
					//Randomly choose new candidate
					iCandidateIndex = rand() % outEndX_vector.size(); 
				}
				outEndX = outEndX_vector[iCandidateIndex];
				outEndY = outEndY_vector[iCandidateIndex];
			}

		}
	}
	return result; 
}
#pragma region
