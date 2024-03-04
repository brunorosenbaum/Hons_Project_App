#pragma once
#include <vector>

#include "CELL_DERV.h"

class RATIONAL_SOLVER
{
public:
	RATIONAL_SOLVER(); 
	~RATIONAL_SOLVER();


private:
	std::vector<CELL_DERV> boundary_Cells; //Serve as boundary condition. 'Borders' of simulation. 
	std::vector<CELL_DERV> positive_Cells; //With >0 potential (phi > 0)
	std::vector<CELL_DERV> negative_Cells; //With 0 potential (phi == 0)

	std::vector<CELL_DERV> candidate_Cells; 

};

