#pragma once
#include <map>
#include <string>
#include <vector>

#include "CELL_DERV.h"

class RATIONAL_SOLVER
{
public:
	RATIONAL_SOLVER(); 
	~RATIONAL_SOLVER();

	bool LoadMap(const std::string& path); 
	//Pre-computation functions
	void CreateBoundaryCells(); //Create boundary grid cells (just position, coords)
	// pre-computation of electric potential for each type of cells
	void	CalcBoundaryPotential();
	void	CalcPositivePotential();
	//Hear me out: we do NOT need to initialize negative potentials for the negative cells
	//Bc both base constructors initialize potential value to 0. 

	//Computation funcs
	bool InitializeGrid(const std::string& path);
	void CreateCandidateMap();
	void CalcPotential_Rational(); 

private:
	//Helper funcs
	float	CalcDistance(float x1, float y1, float x2, float y2) const //Vector magnitude (distance) between 2 points
	{
		return sqrt((x2 - x1) * (x2 - x1) + (y2 - y1) * (y2 - y1));
	}
	void ClearVectors(); 

private:
	//Should we make these cell_derv* instead? 
	std::vector<CELL_DERV> boundary_Cells; //Serve as boundary condition. 'Borders' of simulation. 
	std::vector<CELL_DERV> positive_Cells; //With >0 potential (phi > 0)
	std::vector<CELL_DERV> negative_Cells; //With 0 potential (phi == 0)

	std::vector<CELL_DERV*> all_Cells;

	//Candidates
	std::map<int, CELL_DERV*> candidateMap_DS; //Map DATA STRUCTURE for each candidate cell. Each has a different position. This is for potential calcs.
	std::vector<CELL_DERV> candidate_Cells; //Vector of candidate cells. They may have the same position, but each has a different parent.This is for selecting next lightning branch growth.  


	// initial state
	CELL_DERV* startPoint_Cell; //Initial negative charge
	CELL_DERV* endPoint_Cell; //Initial positive charge

	int gridSize_; //Size of the grid. Both use 256x256. This value is set to 64 tho bc its * 4 (64*4 = 256)
	int eta_;
	int power_of_Rho_; 
};

