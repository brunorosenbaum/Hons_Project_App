#include "RATIONAL_SOLVER.h"

#include <iostream>
#include <ostream>

#define ETA 2 //Eta (H or squiggly n) parameter
#define POW_OF_RHO 3 //Rho is P, or p parameter. 

RATIONAL_SOLVER::RATIONAL_SOLVER() :
eta_(ETA), power_of_Rho_(POW_OF_RHO), gridSize_(0)
{
	//Clear all vectors
	boundary_Cells.clear();
	positive_Cells.clear();
	negative_Cells.clear();

	candidate_Cells.clear();
	all_Cells.clear(); 

	//Rng probably here
}

RATIONAL_SOLVER::~RATIONAL_SOLVER()
{
	ClearVectors(); 

}

bool RATIONAL_SOLVER::LoadMap(const std::string& path)
{
	return false;
}

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

bool RATIONAL_SOLVER::InitializeGrid(const std::string& path)
{
	ClearVectors();
	bool result = LoadMap(path);
	if(result)
	{
		CreateBoundaryCells();//Create boundary grid cells (just position, coords)
		CalcBoundaryPotential(); //Calc phi of all cells.
		CalcPositivePotential();//Negative is not needed since cells default phi is 0. 

		//Generate initial candidate map
		CreateCandidateMap(); 
		//And calculate phi for candidate cells (here's the rational method)

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
				//Candidate's yPos = current cell's xPos + n neighbor's yPos
				candidateX = cellX + all_Cells[n]->neighbors[n]->center[0];
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
	unsigned int totalCells = gridSize_ * gridSize_; //256 cells
	if(all_Cells.size() < totalCells)
	{
		//Cell size error
		std::cout << "cell size error !! " << std::endl;
		return; 
	}

	// calculate electric potential (Phi) for only candidate cells
	float phi; 
	float r; //TODO: I HAVE NO CLUE ABOUT R
	float B, N, P; //Boundary, negative, positive phi. These are the ones from the formula.

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
			// for negative charge cells (phi = 0)
			N = 0;

			//TODO: I DON'T UNDERSTAND THE CLUSTERED CELLS THING 8(
			//TODO: AND R IS NEEDED TO CALCULATE P TT_TT
			//r = CalcDistance()


			//HERE IT IS!! THIS IS THE FORMULA ON THE PAPER PHI = P / (N X B)
			phi = (1.0f / B) * (1.0f / N) * P;

			current_Cell->N_ = N;
			current_Cell->P_ = P;
			current_Cell->B_ = B;
			current_Cell->potential = phi;
		}
		++mapItr; 
	}

}

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
