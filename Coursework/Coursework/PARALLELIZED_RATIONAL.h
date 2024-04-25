#pragma once
#include <map>
#include <random>
#include <string>
#include <vector>
#include <fstream>

#include "CELL_R.h"
#include "LIGHTNING_TREE.h"
#include "CSBuffer.h"


class PARALLELIZED_RATIONAL
{

public:
	PARALLELIZED_RATIONAL(ID3D11Device* device, ID3D11DeviceContext* deviceContext, HWND hwnd);
	~PARALLELIZED_RATIONAL();

	//Grid functions----------------------------------------------------------------------
	bool LoadMap(const std::string& path);
	bool InitializeGrid(const std::string& path); //This is Load() in the other code

	//Pre-computation functions-----------------------------------------------------------
	void CreateBoundaryCells(); //Create boundary grid cells (just position, coords)
	// pre-computation of electric potential for each type of cells
	void	CalcBoundaryPotential();
	void	CalcPositivePotential();
	//Hear me out: we do NOT need to initialize negative potentials for the negative cells
	//Bc both base constructors initialize potential value to 0. 

	//Organizing into clusters-------------------------------------------------------
	void CreateClusterMap(int clusterSize); //Organize negative cells into clusters and calculate their avg xy pos

	//Computation funcs------------------------------------------------------------------
	void CreateCandidateMap();
	void CalcPotential_Rational();
	void CalcPotential_Rational_SingleCell(CELL_R* candidate_cell);
	void Calc_Normalization();
	bool SelectCandidate(CELL_R& outNextCell);

	//Updating funcs---------------------------------------------------------------------
	void AllCellsToCandidates();
	void UpdateCandidates();
	void UpdateCandidateMap(const CELL_R& next_Cell);
	void UpdateClusterMap(const CELL_R& next_Cell);

	//Lightning formation funcs-------------------------------------------------------------------------
	void AddNewLightningPath(const CELL_R& newPath, bool bIsEndCell = false, bool bTargetCell = false);
	bool ProcessLightning();
	void initLightningTree();
	void ThinLightningTree();

	//Getsetters
	LIGHTNING_TREE& GetLightningTree() { return lightning_tree_; }
	int GetGridSize()const { return gridSize_; }

private:
	//Helper funcs
	float	CalcDistance(float x1, float y1, float x2, float y2) const //Vector magnitude (distance) between 2 points
	{
		return sqrt((x2 - x1) * (x2 - x1) + (y2 - y1) * (y2 - y1));
	}
	void ClearVectors();
	bool IsNearEndCell(int x, int y, int& outEndX, int& outEndY) const; //Returns true if cell is near ground target cell

private: //------------- Grid and potential vars

	//Should we make these cell_derv* instead? 
	std::vector<CELL_R> boundary_Cells; //Serve as boundary condition. 'Borders' of simulation. 
	std::vector<CELL_R> positive_Cells; //With >0 potential (phi > 0)
	std::vector<CELL_R> negative_Cells; //With 0 potential (phi == 0)
	std::vector<CELL_R> startpoint_Cells; //Initial negative charge
	std::vector<CELL_R> endpoint_Cells; //Initial positive charge

	//Vectors of potentials bc im so tired
	std::vector<float> boundaryPotentials_;
	std::vector<float> positivePotentials_;

	//Cells in the grid
	std::vector<CELL_R*> all_Cells;
	std::vector<CLUSTER> clusters_; //Vector of clusters: cells in the same region

	//Candidates
	std::map<int, CELL_R*> candidateMap_DS; //Map DATA STRUCTURE for each candidate cell. Each has a different position. This is for potential calcs.
	std::vector<CELL_R> candidate_Cells; //Vector of candidate cells. They may have the same position, but each has a different parent.This is for selecting next lightning branch growth.  


	int gridSize_; //Size of the grid. Both use 256x256. This value is set to 64 tho bc its * 4 (64*4 = 256)
	int clusterSize_; //Size of cluster. Set to 16.
	int eta_;
	int power_of_Rho_;
	//Mersenne Twister 19937 generator. Pseudo-random generator of 32-bit numbers with a state size of 19937 bits.
	std::mt19937 rng_;

private: //---------Lightning tree and cs
	LIGHTNING_TREE lightning_tree_;

	CSBuffer* compute_shader;
	ID3D11Device* device = nullptr;
	ID3D11DeviceContext* deviceContext = nullptr;

	//TEST - WILL USE WITH RATIONAL METHOD
	ID3D11Buffer* clusterBuffer = nullptr; //These are the buffers we're gonna write our cell info to 
	ID3D11Buffer* cellBuffer = nullptr; 
	ID3D11Buffer* CPUReadBuffer = nullptr;
	ID3D11Buffer* bufferResult = nullptr;
	ID3D11ShaderResourceView* srvBuffer0 = nullptr;
	ID3D11ShaderResourceView* srvBuffer1 = nullptr;
	/*ID3D11ShaderResourceView* srvBuffer1 = nullptr;*/
	ID3D11UnorderedAccessView* resultUAV = nullptr;


	//struct DataBufferType
	//{
	//	float r_[128*128], N_[128*128], phi_[128*128];
	//};
	struct DataBufferType
	{
		float r_, N_, phi_;
	};

	struct GPUCellData
	{
		//int x, y;
		float phi;
		float N, P, B;
		int isCandidate = false;
		int isCluster = true; 
	};
	struct GPUClusterData
	{
		int x, y;
		int xSum, ySum;
		int xAvg, yAvg;
		GPUCellData clusterCells[16];
	};
	
	GPUCellData gpuCellsArray[128][128];
};

