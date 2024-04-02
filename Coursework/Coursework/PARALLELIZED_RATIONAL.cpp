#include "PARALLELIZED_RATIONAL.h"

#include <iostream>
#include <ostream>
#include <random>
#include <sstream>

#define ETA 3 //Eta (H or squiggly n) parameter
#define POW_OF_RHO 2 //Rho is P, or long p parameter. 

PARALLELIZED_RATIONAL::PARALLELIZED_RATIONAL(ID3D11Device* device_, ID3D11DeviceContext* deviceContext_, HWND hwnd) :
	eta_(ETA), power_of_Rho_(POW_OF_RHO), gridSize_(0), clusterSize_(0)
{
	//Clear all vectors
	boundary_Cells.clear();
	positive_Cells.clear();
	negative_Cells.clear();
	startpoint_Cells.clear();
	endpoint_Cells.clear();

	boundaryPotentials_.clear();
	positivePotentials_.clear();

	candidateMap_DS.clear();
	candidate_Cells.clear();
	all_Cells.clear();

	//Rng probably here
	std::random_device	rd;
	rng_.seed(rd());
	srand(static_cast<unsigned>(time(0)));
	//Init compute shader
	compute_shader = new CSBuffer(device_, hwnd);
	device = device_;
	deviceContext = deviceContext_; 

}

PARALLELIZED_RATIONAL::~PARALLELIZED_RATIONAL()
{
	ClearVectors();

}

bool PARALLELIZED_RATIONAL::LoadMap(const std::string& path) //Reads a .map file
{
	std::cout << "Load map file : " << path.c_str() << std::endl;
	// read map file
	std::ifstream in(path.c_str(), std::ios::in);
	if (!in)
	{
		std::cerr << "Cannot open " << path.c_str() << std::endl;
		return false;
	}
	// parse map
	std::string strLine;

	int iLine = 0;
	float fDefaultX = 0.5f;

	CELL_R* cellPtr;
	bool bMapStart = false;
	int iCellType;
	int iCellIndex = 0;
	int iCellX, iCellY;


	while (std::getline(in, strLine))
	{
		++iLine;

		if ('#' == strLine[0])
		{
			// ignore comments
		}
		else if (strLine.substr(0, 8) == "VERSION:")
		{
			// TODO : check version
			// ignore version
		}
		else if (strLine.substr(0, 10) == "GRID_SIZE:")
		{
			std::istringstream pos(strLine.substr(10));
			pos >> gridSize_;
		}
		else if (strLine.substr(0, 20) == "CLUSTERED_GRID_SIZE:")
		{
			std::istringstream pos(strLine.substr(20));
			pos >> clusterSize_;
		}
		else if (strLine.substr(0, 10) == "MAP_START:")
		{
			bMapStart = true;
		}
		else if (strLine.substr(0, 8) == "MAP_END:")
		{
			bMapStart = false;
		}
		else if (strLine.substr(0, 2) == "M:")
		{
			if (bMapStart)
			{
				std::istringstream pos(strLine.substr(2));

				for (int i = 0; i < gridSize_; ++i)
				{
					pos >> iCellType; //This works and it reads the numbers in the .map file. 
					//Therefore, it'll be 0 for empty, 1 for negative, and 2 for positive.

					iCellX = iCellIndex % gridSize_; //These are NOT the X and Y positions in the world or diagram!
					iCellY = iCellIndex / gridSize_; //These are x and y ON THE .MAP FILE!!!!

					cellPtr = new CELL_R(iCellX, iCellY, EMPTY_R, fDefaultX);
					//So the first will be (15, 0) bc it's in the .map file line number
					//Then (15, 1), (15, 2)... these numbers are the ones passed into the xy coords of each CELL. 

					if (cellPtr)
					{
						all_Cells.push_back(cellPtr);

						switch (iCellType)
						{
						case NEGATIVE_R: //If number read from .map == 1
						{
							cellPtr->SetCellType(NEGATIVE_R);
							//Here's where I find out that start point cells and negative cells are initialized in the same.
							//Startpoint cells can be one cell ptr, no need for a vector. TODO: EDIT LATER
							//So these are cells with the value type == 1.
								//Upon initialization, both these vectors will be of size 1. 
							startpoint_Cells.push_back(*cellPtr);
							negative_Cells.push_back(*cellPtr);

						}
						break;

						case POSITIVE_R://If number read from .map == 2
						{
							cellPtr->SetCellType(POSITIVE_R);
							//Same thing as before. Also, positive cells AND endpoint will be size 1 in this simulation. 
							endpoint_Cells.push_back(*cellPtr);
							positive_Cells.push_back(*cellPtr);
						}
						break;

						//Why do we break here and not inside? I have to ask that.
						//Why do we not have default for type == 0 (empty) cells?
							//Because the purpose of this method is to read where the start and endpoint are in the GRID. 
						}
						++iCellIndex;
					}
				}
			}
			else
			{
				std::cerr << "Map file is invalid !! (line: " << iLine << ", file: " << path.c_str() << ")" << std::endl;
			}
		}
	}

	if (bMapStart)
	{
		std::cerr << "MAP_END: is missed !! (line: " << iLine << ", file: " << path.c_str() << ")" << std::endl;
	}

	// initialize lightning tree
	initLightningTree();

	std::cout << "Finish loading map file (" << path.c_str() << ") !!" << std::endl;

	return true;
}

bool PARALLELIZED_RATIONAL::InitializeGrid(const std::string& path) //Load()
{
	ClearVectors();

	bool result = LoadMap(path);
	if (result)
	{
		CreateBoundaryCells();//Create boundary grid cells (just position, coords)
		CalcBoundaryPotential(); //Calc phi of all cells.
		CalcPositivePotential();//Potential of cells depending on their distance to the endpoint

		//Create clusters
		CreateClusterMap(clusterSize_);

		//Generate initial candidate map
		CreateCandidateMap();
		//And calculate phi for candidate cells (here's the rational method)
		CalcPotential_Rational();
	}
	return result;
}

#pragma region PRECOMPUTATION FUNCTIONS
//-----------------------------------------------------------PRECOMPUTATION---------------------------------------------
void PARALLELIZED_RATIONAL::CreateBoundaryCells() //I'm assuming this method creates the boundary grid, akin to the green channel
//in the .ppm files in Kims method. These should be positive in charge. 
{ //I think this is the equivalent to the quad_dbm_2D's drawQuadTree() 
	boundary_Cells.clear();
	boundary_Cells.reserve(gridSize_ * 4 + 4); //32*4 + 4 = 132

	CELL_R current_cell;
	//Add boundary charges
	for (int i = 0; i < gridSize_; ++i) //Initialize xy coords of grid
	{
		//Horizontal boundary cells
		current_cell.x = i; current_cell.y = -1;
		boundary_Cells.push_back(current_cell); //And add to vector

		if (endpoint_Cells.size() == 1) //Will always execute 
		{
			current_cell.x = i;			current_cell.y = gridSize_;		boundary_Cells.push_back(current_cell);
		}

		//Vertical boundary cells
		current_cell.x = -1; current_cell.y = i;
		boundary_Cells.push_back(current_cell); //And add to vector

		current_cell.x = gridSize_; current_cell.y = i;
		boundary_Cells.push_back(current_cell); //And add to vector

	}
	//4 corners - clockwise
	current_cell.x = -1; current_cell.y = -1; //Top left
	boundary_Cells.push_back(current_cell);

	current_cell.x = gridSize_; current_cell.y = -1; //Top right
	boundary_Cells.push_back(current_cell);

	current_cell.x = gridSize_; current_cell.y = gridSize_; //Bottom right
	boundary_Cells.push_back(current_cell);

	current_cell.x = -1; current_cell.y = gridSize_; //Bottom left
	boundary_Cells.push_back(current_cell);
}

void PARALLELIZED_RATIONAL::CalcBoundaryPotential()
{
	//Before we compute the potentials, we divide the charges
	//	into three types : positive charges, negative charges along the
	//	lightning path, and **boundary charges**.We then calculate the
	//	electric potentials based on those types separately as P, N, and **B**.

	/*m_vBoundaryPotential.clear();*/ //This was a vector of floats of the potentials,
	//But since we're using whole cell ptrs here we don't need to clear as these potentials
	//are all initialized to 0 upon creation
	/*m_vBoundaryPotential.reserve(m_iGridSize * m_iGridSize);*/ //<- Shouldn't be an issue. This space is already reserved in prev function
	boundaryPotentials_.reserve(gridSize_ * gridSize_);
	int iIndex = 0;
	CELL_R* current_Cell;
	float boundaryPhi; // Called b in the other code and in the formula
	float r; //distance

	for (int i = 0; i < gridSize_; ++i)
	{
		for (int j = 0; j < gridSize_; ++j)
		{
			current_Cell = all_Cells[iIndex];
			boundaryPhi = 0;

			if (current_Cell)
			{
				std::vector< CELL_R >::const_iterator bItr = boundary_Cells.begin();
				while (bItr != boundary_Cells.end()) //Iterate through boundary cells
				{
					//r = distance (scalar value) between all cells (i think) and boundary cells
					r = CalcDistance(bItr->x, bItr->y, current_Cell->x, current_Cell->y);
					if (power_of_Rho_ > 1) //RHO is the p parameter to prevent excessive branching TODO: REREAD THIS
					{//This condition will always be true then bc it's initialized by a macro 
						r = pow(r, power_of_Rho_); //Square the distance
					}

					boundaryPhi += 1.0f / r; //Add 1/squared distance between cells to B, which is the potential. 

					++bItr;
				}
			}

			//boundary_Cells[iIndex].potential = boundaryPhi; //B is the potential. Set it to that for each of the boundary cells
			////TODO: THIS IS GONNA THROW ERRORS. I KNOW THIS IS WRONG.
			////TODO: THIS IS NOT GOING TO GO THROUGH ALL THE BOUNDARY CELLS BC IT'S A NESTED LOOP. BE CAREFUL!! ASK!!
			boundaryPotentials_.push_back(boundaryPhi);
			++iIndex;
		}
	}
}

void PARALLELIZED_RATIONAL::CalcPositivePotential()
{
	//Before we compute the potentials, we divide the charges
	//	into three types : **positive charges**, negative charges along the
	//	lightning path, and boundary charges.We then calculate the
	//	electric potentials based on those types separately as **P**, N, and B.

	positivePotentials_.clear();
	positivePotentials_.assign(gridSize_ * gridSize_, 0.0f); //Assign value 0 to all positive elec potential cells
	CELL_R* current_Cell;
	float positivePhi = 0;
	float r;
	int iIndex = 0;


	for (int i = 0; i < gridSize_; ++i)
	{
		for (int j = 0; j < gridSize_; ++j)
		{
			current_Cell = all_Cells[iIndex];
			positivePhi = 0;

			if (current_Cell && current_Cell->type_ != POSITIVE_R) //Go through cells until they reach end cell
			{
				std::vector< CELL_R >::const_iterator pItr = positive_Cells.begin();
				while (pItr != positive_Cells.end())
				{
					r = CalcDistance(pItr->x, pItr->y, current_Cell->x, current_Cell->y);
					if (power_of_Rho_ > 1)
					{
						r = pow(r, power_of_Rho_);
					}
					//Set their potential to 1
					positivePhi += 1.0f / r;

					++pItr;
				}
			}
			positivePotentials_[iIndex] = positivePhi;
			iIndex++;
		}



	}
}
#pragma endregion

#pragma region ORGANIZATION_FUNCTIONS
void PARALLELIZED_RATIONAL::CreateClusterMap(int clusterSize)
{

	clusterSize_ = clusterSize; //size of multi scaled cluster grid map ( 8 x 8 ) = 16. Remember the grid is 64 x 64

	int regionSize = gridSize_ / clusterSize_; //64x64/8x8 = 4096/16 = 256.
	//Meaning there are 256 clusters in a 64x64 map

	clusters_.clear();
	clusters_.reserve(clusterSize_ * clusterSize_); //Make the cluster vector 16x16 = 256 (same as region size)

	CLUSTER current_cluster;


	//Initialize x and y of cell clusters in cluster vector
	for (int i = 0; i < clusterSize_; ++i) //y 
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
	while (negItr != negative_Cells.end())
	{
		x = negItr->x / regionSize; //x = x.pos/256
		y = negItr->y / regionSize; //y = y.pos/256

		iIndex = y * clusterSize_ + x;
		clusters_[iIndex].cluster_Cells.push_back(*negItr); //Fill all the clusters with the negative cells

		clusters_[iIndex].c_xSum += negItr->x; //Add the xy coords of negative cells to clusters
		clusters_[iIndex].c_ySum += negItr->y; //This is to calc the average xy pos

		//Calc avg xy pos of each cluster
		clusters_[iIndex].c_xAvg = (float)clusters_[iIndex].c_xSum / clusters_[iIndex].cluster_Cells.size();
		clusters_[iIndex].c_yAvg = (float)clusters_[iIndex].c_ySum / clusters_[iIndex].cluster_Cells.size();
		++negItr;
	}

}
#pragma endregion

#pragma region COMPUTATION FUNCTIONS
//-----------------------------------------------COMPUTATION FUNCTIONS----------------------------------------------------

void PARALLELIZED_RATIONAL::CreateCandidateMap() //Map refers to the C++ MAP DATA STRUCTURE.
//Maps index of all cells vector to elements of candidate cells vector.
{
	candidateMap_DS.clear();
	int iIndex;
	int cellX, cellY; //All cells vector x and y pos
	int candidateX, candidateY; //Candidate cells vector x and y pos
	int iChildIndex;

	auto itr = negative_Cells.begin(); //Go through negative cells
	while (itr != negative_Cells.end())
	{
		cellX = (*itr).x;
		cellY = (*itr).y;
		iIndex = cellY * gridSize_ + cellX;

		if (all_Cells[iIndex]) //Go through all cells
		{
			//Go through neighbors of each cell & update the candidates' xy coords to that
			for (int n = 0; n < 8; ++n)
			{
				//Candidate's xPos = current cell's xPos + n neighbor's xPos
				candidateX = cellX + CELL_R::NEIGHBORS_X_DIFFERENCE[n];
				//Candidate's yPos = current cell's yPos + n neighbor's yPos
				candidateY = cellY + CELL_R::NEIGHBORS_Y_DIFFERENCE[n];

				iChildIndex = candidateY * gridSize_ + candidateX;

				//If cells xy coords are within the gridsize
				if (candidateX >= 0 && candidateX < gridSize_
					&& candidateY >= 0 && candidateY < gridSize_
					&& all_Cells[iChildIndex])
				{
					if (all_Cells[iChildIndex]->type_ == EMPTY_R) //If cell is empty
					{
						//Find each iChildIndex element in candidate cell map
						std::map<int, CELL_R*>::iterator candidate_Itr = candidateMap_DS.find(iChildIndex);
						if (candidate_Itr == candidateMap_DS.end())
						{
							//Insert cell at the end of the map
							candidateMap_DS.insert(std::map<int, CELL_R*>::value_type(iChildIndex, all_Cells[iChildIndex]));
						}
					}
				}

			}
		}
		++itr;
	}


}

void PARALLELIZED_RATIONAL::CalcPotential_Rational()
{
	unsigned int totalCells = gridSize_ * gridSize_; //4096 cells
	if (all_Cells.size() < totalCells)
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
	int iClusterIndex;
	int iCandidateClusterX, iCandidateClusterY, iCandidateClusterIndex;

	int mapKey; //Value that'll be assigned to the int map key of the candidate cells map.
	CELL_R* current_Cell;

	if (candidateMap_DS.size() == 0)
	{
		std::cout << "There is no candidate cells to compute electric potential !!" << std::endl;

	}

	//Now, go through candidate cells
	auto mapItr = candidateMap_DS.begin();
	while (mapItr != candidateMap_DS.end())
	{
		//Assign to map values
		mapKey = mapItr->first;
		current_Cell = mapItr->second;

		if (current_Cell && current_Cell->type_ == EMPTY_R) //If current cell is empty
		{
			// -----------------------------------------------------------
			// for boundaries, use pre-computed values
			B = boundaryPotentials_[mapKey];
			//B = boundary_Cells[mapKey].potential;
			// -----------------------------------------------------------
			// for positive charges, use pre-computed value
			P = positivePotentials_[mapKey];
			// -----------------------------------------------------------
			// for negative charge cells 
			iCandidateClusterX = current_Cell->x / regionSize; //X
			iCandidateClusterY = current_Cell->y / regionSize; //Y
			iCandidateClusterIndex = iCandidateClusterY * clusterSize_ + iCandidateClusterX;
			iClusterIndex = 0;

			N = 0; //(phi = 0)

			//Use compute shader
			//Write candidate cell data to structured buffer
			compute_shader->createStructuredBuffer(device, sizeof(clusters_), clusters_.size(), &clusters_[0], &buffer0);
			compute_shader->createStructuredBuffer(device, sizeof(clusters_), clusters_.size(), nullptr, &bufferResult);
			//Write that structured buffer data to an srv buffer
			compute_shader->createBufferSRV(device, buffer0, &srvBuffer0);
			compute_shader->createBufferUAV(device, bufferResult, &resultUAV);
			compute_shader->runComputeShader(deviceContext, nullptr, 1, &srvBuffer0, resultUAV, 16, 16, 1); 
			
			ID3D11Buffer* cpuBuf = compute_shader->createCPUReadBuffer(device, deviceContext, bufferResult);
			D3D11_MAPPED_SUBRESOURCE MappedResource;
			DataBufferType* ptr_;
			deviceContext->Map(cpuBuf, 0, D3D11_MAP_READ, 0, &MappedResource); 
			ptr_ = (DataBufferType*)MappedResource.pData; 

			if(ptr_->phi_ == 0)
			{
				phi = 0.5;
				N = 0.5;

				phi = max(ptr_->phi_, 0.00001f);
				N = max(ptr_->N_, 0.00001f);
			}else
			{
				phi = ptr_->phi_;
				N = ptr_->N_; 
			}
			

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

void PARALLELIZED_RATIONAL::CalcPotential_Rational_SingleCell(CELL_R* candidate_cell)
{
	// calculate electric potential (Phi) for only candidate cells
	float phi;
	float r; //Distance of cells between each other
	float B, N, P; //Boundary, negative, positive phi. These are the ones from the formula.

	int regionSize = gridSize_ / clusterSize_;
	int iClusterIndex;
	int iCandidateClusterX, iCandidateClusterY, iCandidateClusterIndex;


	if (candidate_cell && candidate_cell->type_ == EMPTY_R) //If current cell is empty
	{
		int iKey = candidate_cell->y * gridSize_ + candidate_cell->x;

		// -----------------------------------------------------------
		// for boundaries, use pre-computed values
		B = boundaryPotentials_[iKey];
		// -----------------------------------------------------------
		// for positive charges, use pre-computed value
		P = positivePotentials_[iKey];
		// -----------------------------------------------------------
		// for negative charge cells 
		iCandidateClusterX = candidate_cell->x / regionSize; //X
		iCandidateClusterY = candidate_cell->y / regionSize; //Y
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
							candidate_cell->x, //and current cell's xy
							candidate_cell->y);
						if (power_of_Rho_ > 1) //r = r^p
						{
							r = pow(r, power_of_Rho_);
						}
						N += clusters_[iClusterIndex].cluster_Cells.size() / r;
					}
					else //For negative cells in the same cluster
					{
						auto nItr = clusters_[iClusterIndex].cluster_Cells.begin();
						while (nItr != clusters_[iClusterIndex].cluster_Cells.end())
						{
							r = CalcDistance(nItr->x, nItr->y, //Distance is magnitude between
								candidate_cell->x, candidate_cell->y); //Cluster's xy coords and current cell's
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

void PARALLELIZED_RATIONAL::Calc_Normalization()
{
	/*Once we have computed the electric potential between the
	candidate cells and other charged cells, we use the normalization equation, Equation 2.*/
	//I wonder if this is it?  If so, it calculates P(i), the probability of selection of each candidate cell

	int iIndex;

}

bool PARALLELIZED_RATIONAL::SelectCandidate(CELL_R& outNextCell) //Choose next lightning cell among candidates
{ //Done by calculating P(i)
	outNextCell.x = -1;
	outNextCell.y = -1;

	bool result = true;
	int iIndex = 0;

	std::vector<float> selection_Probability; //Vector of P(i)

	auto itr = candidate_Cells.begin();
	while (itr != candidate_Cells.end())
	{
		//P(i) is calculated with equation 2 (Y) or eq. 10 (K), and represents the probability of selection.
		//Apply eta parameter (squiggly n, for branching)
		//And store into a vector
		selection_Probability.push_back(pow(fabs(itr->selectionProb), eta_));
		++itr;
	}
	//Make a discrete distribution, with the weights being their selection probabilities.
	std::discrete_distribution<> distribution(selection_Probability.begin(), selection_Probability.end());

	iIndex = distribution(rng_); //iIndex is a random index from the distribution
	int iErrorCount = 0;

	//TODO: THE ERROR IS HERE, DISCRETE DISTRIBUTION GETS STUCK BECAUSE CANDIDATE CELLS.SIZE() == 0
	//TODO: THERE SHOULD BE SOMETHING IN THE CODE (IN UPDATE CANDIDATE MAP OR UPDATE CANDIDATE) RESETTING
	while (candidate_Cells.size() == iIndex) //So iIndex is the same size as candidate cells, right?
	{//I think this has to do with.. candidates not being suitable
		++iErrorCount;
		std::cout << "Index error !!!!!" << std::endl;
		if (iErrorCount >= 10)
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
void PARALLELIZED_RATIONAL::AllCellsToCandidates() //Turns all cells into candidates. Will be called in main. 
{
	candidateMap_DS.clear();
	int iIndex;

	for (int i = 0; i < gridSize_; ++i) //Columns
	{
		for (int j = 0; j < gridSize_; ++j) //Rows
		{
			iIndex = i * gridSize_ + j;
			if (all_Cells[iIndex] && all_Cells[iIndex]->type_ == EMPTY_R) //If cells exist and theyre empty
			{//Map all cells to candidate map
				candidateMap_DS.insert(std::map<int, CELL_R*>::value_type(iIndex, all_Cells[iIndex]));
			}
		}
	}
}

void PARALLELIZED_RATIONAL::UpdateCandidates()
{
	//Pretty much the same as CreateCandidateMap()
	candidate_Cells.clear();
	int iIndex;
	int p_x, p_y; //Potential cells x & y
	int c_x, c_y; //Candidate cells x & y
	int iNeighborIndex; //Neighbor cell index

	auto itr = negative_Cells.begin();
	while (itr != negative_Cells.end())
	{
		p_x = itr->x;
		p_y = itr->y;
		iIndex = p_y * gridSize_ + p_x;

		if (all_Cells[iIndex])
		{
			//Check neighbors
			for (int n = 0; n < 8; ++n) {
				c_x = p_x + CELL_R::NEIGHBORS_X_DIFFERENCE[n];
				c_y = p_y + CELL_R::NEIGHBORS_Y_DIFFERENCE[n];
				iNeighborIndex = c_y * gridSize_ + c_x;

				if (c_x >= 0 && c_x < gridSize_
					&& c_y >= 0 && c_y < gridSize_
					&& all_Cells[iNeighborIndex])
				{
					if (all_Cells[iNeighborIndex]->type_ == EMPTY_R)
					{
						if (all_Cells[iNeighborIndex]->potential != 0.0f)
						{
							int diff_x = p_x - c_x; //Differences in position between potential cells and candidate
							int diff_y = p_y - c_y;

							int x_index = c_y * gridSize_ + (c_x + diff_x);
							int y_index = (c_y + diff_y) * gridSize_ + c_x;

							bool xEmpty = true;
							bool yEmpty = true;

							if (x_index >= 0 && x_index < gridSize_ * gridSize_ && all_Cells[x_index]->type_ == NEGATIVE_R)
							{ //Cell is at the start. For x
								xEmpty = false;
							}
							if (y_index >= 0 && y_index < gridSize_ * gridSize_ && all_Cells[y_index]->type_ == NEGATIVE_R)
							{ //Cell is at the start. For x
								yEmpty = false;
							}
							if (xEmpty || yEmpty) //If cell is feasible to be a candidate
							{
								//Set its xy, parents xy and phi to new ones
								CELL_R candidate;
								candidate.x = c_x;
								candidate.y = c_y;
								candidate.parentX = p_x;
								candidate.parentY = p_y;
								candidate.selectionProb = all_Cells[iNeighborIndex]->potential;
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

void PARALLELIZED_RATIONAL::UpdateCandidateMap(const CELL_R& next_Cell)
{
	//Same as calcPotential_Rational()
	int x = next_Cell.x;
	int y = next_Cell.y;
	int iIndex = y * gridSize_ + x;

	//Remove cell from candidate map
	auto mapItr = candidateMap_DS.find(iIndex);
	if (mapItr != candidateMap_DS.end())
	{
		candidateMap_DS.erase(iIndex);
	}

	//Update electric potential (phi) for candidate cells
	float r;
	CELL_R* candidate_Cell;

	auto itr = candidateMap_DS.begin();
	while (itr != candidateMap_DS.end()) //Go through mapped candidates
	{
		candidate_Cell = itr->second;
		if (candidate_Cell)
		{
			r = CalcDistance(x, y, candidate_Cell->x, candidate_Cell->y);
			if (power_of_Rho_ > 1)
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
	if (all_Cells[iIndex])
	{
		int c_x, c_y;
		int iChildIndex;

		//Check neighbors
		for (int n = 0; n < 8; ++n)
		{
			c_x = x + CELL_R::NEIGHBORS_X_DIFFERENCE[n]; //Update xy coords
			c_y = y + CELL_R::NEIGHBORS_Y_DIFFERENCE[n];
			iChildIndex = c_y * gridSize_ + c_x;

			if (c_x >= 0 && c_x < gridSize_
				&& c_y >= 0 && c_y < gridSize_
				&& all_Cells[iChildIndex])
			{
				if (all_Cells[iChildIndex]->type_ == EMPTY_R)
				{
					auto itr = candidateMap_DS.find(iChildIndex);
					if (candidateMap_DS.end() == itr)
					{ //Insert at the end
						candidateMap_DS.insert(std::map<int, CELL_R*>::value_type(iChildIndex, all_Cells[iChildIndex]));
						// calculate electric potential for newly added candidate cells
						CalcPotential_Rational_SingleCell(all_Cells[iChildIndex]);
					}
				}
			}

		}
	}

}

void PARALLELIZED_RATIONAL::UpdateClusterMap(const CELL_R& next_Cell)
{
	int iIndex;
	int x, y;
	int iRegionSize = gridSize_ / clusterSize_;

	x = next_Cell.x / iRegionSize;
	y = next_Cell.y / iRegionSize;
	iIndex = y * clusterSize_ + x;

	clusters_[iIndex].cluster_Cells.push_back(next_Cell);
	clusters_[iIndex].c_xSum += next_Cell.x;
	clusters_[iIndex].c_ySum += next_Cell.y;
	clusters_[iIndex].c_xAvg = (float)clusters_[iIndex].c_xSum / clusters_[iIndex].cluster_Cells.size();
	clusters_[iIndex].c_yAvg = (float)clusters_[iIndex].c_ySum / clusters_[iIndex].cluster_Cells.size();
}

#pragma endregion

#pragma region LIGHTNING FUNCTIONS
void PARALLELIZED_RATIONAL::AddNewLightningPath(const CELL_R& newPath, bool bIsEndCell, bool bTargetCell)
{
	int iIndex = newPath.y * gridSize_ + newPath.x; //Grid index

	if (!bIsEndCell) //If not end cell
	{
		//Update cell type
		if (iIndex >= 0 && iIndex < all_Cells.size())
		{
			if (all_Cells[iIndex] && all_Cells[iIndex]->type_ == EMPTY_R)
			{//Only empty cells can be changed to other types

				all_Cells[iIndex]->SetCellType(NEGATIVE_R);//Set to negative
				negative_Cells.push_back(*all_Cells[iIndex]); //And add to negative cells vector
			}
		}
	}
	//Add lightning node to tree
	lightning_tree_.AddChild(newPath.parentX, newPath.parentY,
		newPath.x, newPath.y, bTargetCell);
}


bool PARALLELIZED_RATIONAL::ProcessLightning()
{
	bool isLooping = true;
	while (isLooping)
	{
		UpdateCandidates(); //Get candidates

		//And select next cell from candidates
		CELL_R next_Cell;
		bool result_ = SelectCandidate(*&next_Cell);

		if (result_)
		{
			int iEndX, iEndY;
			if (IsNearEndCell(next_Cell.x, next_Cell.y, iEndX, iEndY)) //If lightning is near end cell (attractor)
			{
				isLooping = false;
				AddNewLightningPath(*&next_Cell);
				//Add final target position
				next_Cell.parentX = next_Cell.x;
				next_Cell.parentY = next_Cell.y;
				next_Cell.x = iEndX;
				next_Cell.y = iEndY;
				AddNewLightningPath(*&next_Cell, true);
			}
			else
			{

				AddNewLightningPath(*&next_Cell);
				UpdateClusterMap(*&next_Cell);
				UpdateCandidateMap(*&next_Cell);

			}
		}
		if (!isLooping) //Process finished
		{
			ThinLightningTree();
		}
	}
	return true;
}

void PARALLELIZED_RATIONAL::initLightningTree()
{
	//Takes starting point cell and initializes it to parent/root of the tree.  
	CELL_R next_Cell;
	bool isRoot = true;

	auto itr = startpoint_Cells.begin(); //This code is literally gonna run once then
	while (itr != startpoint_Cells.end())
	{
		next_Cell.parentX = next_Cell.x;
		next_Cell.parentY = next_Cell.y;
		next_Cell.x = itr->x;
		next_Cell.y = itr->y;

		if (isRoot) //Since this method is called once (1 startpoint), then it'll only run this block of code
		{
			isRoot = false;
			//Set root of new tree
			LIGHTNING_TREE_NODE* rootPtr = new LIGHTNING_TREE_NODE();
			if (rootPtr)
			{
				rootPtr->x_ = next_Cell.x; //The root's pos will be (15, 0) then
				rootPtr->y_ = next_Cell.y;
				rootPtr->parent_ = NULL;
				lightning_tree_.SetRoot(rootPtr);

			}
		}
		else //This won't run. 
		{
			next_Cell.parentX = next_Cell.x;
			next_Cell.parentY = next_Cell.y;
			//Else, add child
			lightning_tree_.AddChild(next_Cell.parentX, next_Cell.parentY,
				next_Cell.x, next_Cell.y);
		}
		++itr;
	}


}

void PARALLELIZED_RATIONAL::ThinLightningTree() //Culls branches that diverge from main channel
{
	lightning_tree_.SetTreeThickness();
}
#pragma endregion

#pragma region HELPER FUNCTIONS
void PARALLELIZED_RATIONAL::ClearVectors()
{
	std::vector< CELL_R* >::iterator itr = all_Cells.begin();
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

	startpoint_Cells.clear();
	endpoint_Cells.clear();

	candidate_Cells.clear();
	candidateMap_DS.clear();

	all_Cells.clear();
	clusters_.clear();
	lightning_tree_.ClearNodes();
}

bool PARALLELIZED_RATIONAL::IsNearEndCell(int x, int y, int& outEndX, int& outEndY) const
{
	//Same code as previous functions but instead of checking if cell is empty you check if cell is an attractor
	bool result = false;
	outEndX = -1; outEndY = -1;

	if (x >= 0 && x < gridSize_ && y >= 0 && y < gridSize_)
	{
		int c_x, c_y;
		int iIndex;
		std::vector<int> outEndX_vector; std::vector<int> outEndY_vector;

		//Check neighbors
		for (int n = 0; n < 8; ++n)
		{
			c_x = x + CELL_R::NEIGHBORS_X_DIFFERENCE[n];
			c_y = y + CELL_R::NEIGHBORS_Y_DIFFERENCE[n];
			iIndex = c_y * gridSize_ + c_x;

			if (c_x >= 0 && c_x < gridSize_ && c_y >= 0 && c_y < gridSize_
				&& all_Cells[iIndex]) //Check cells
			{
				if (all_Cells[iIndex]->type_ == POSITIVE_R)
				{
					result = true;
					////Add candidate coords to vectors of end x and end y pos
					outEndX_vector.push_back(c_x);
					outEndY_vector.push_back(c_y);
				}
			}
			if (outEndX_vector.size() > 0) //If Vector is filled
			{
				int iCandidateIndex = 0;
				if (outEndX_vector.size() > 1) //If there's at least more than 1 element
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
#pragma endregion
