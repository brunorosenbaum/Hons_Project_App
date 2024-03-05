#include "RATIONAL_SOLVER.h"

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
	std::vector< CELL_DERV* >::iterator itr = all_Cells.begin();
	while (itr != all_Cells.end())
	{
		if(*itr) //Safe delete
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
