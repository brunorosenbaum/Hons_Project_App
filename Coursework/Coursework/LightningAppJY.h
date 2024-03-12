#pragma once
#include "CellMesh.h"
#include "DXF.h"
#include "LightningMesh.h"
#include "RATIONAL_SOLVER.h"
#include "LightningSM.h"
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
	void initLightning(XMMATRIX world, XMMATRIX view, XMMATRIX projection);

protected:
	bool render();
	void gui();

private:
	CellMesh* grid_Cell_mesh;
	CellBoundsMesh* grid_bounds_mesh;
	LightningMesh* lightning_mesh_; 

	LinearSM* linear_SM; 
	LightningSM* lightning_SM;
	RATIONAL_SOLVER* lightning_Generator;
	float sceneSize, sceneHalf;

};

