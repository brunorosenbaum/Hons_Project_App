#pragma once
#include "CellMesh.h"
#include "DXF.h"
#include "LightningMesh_JY.h"
#include "RATIONAL_SOLVER.h"
#include "Lightning_JY_SM.h"
#include "LinearSM.h"

class LightningAppJY : public BaseApplication
{
public:

	LightningAppJY();
	~LightningAppJY();

	void init(HINSTANCE hinstance, HWND hwnd, int screenWidth, int screenHeight, Input* in, bool VSYNC, bool FULL_SCREEN);
	bool frame();

	//Lightning rendering funcs
	void initQuadGrid(XMMATRIX world, XMMATRIX view, XMMATRIX projection);
	void initLightning();
	void drawLightning(XMMATRIX world, XMMATRIX view, XMMATRIX projection);

protected:
	bool render();
	void gui();

private:
	CellMesh* grid_Cell_mesh;
	CellBoundsMesh* grid_bounds_mesh;
	LightningMesh_JY* lightning_mesh_; 

	LinearSM* linear_SM; 
	Lightning_JY_SM* lightning_SM;
	RATIONAL_SOLVER* lightning_Generator;
	float sceneSize, sceneHalf;

	std::vector<LIGHTNING_TREE_NODE*> tree_nodes; 

};

