#pragma once

// Includes
#include "DXF.h"
#include "LinearSM.h"

class App2_Cubes : public BaseApplication
{
public:

	App2_Cubes();
	~App2_Cubes();

	void init(HINSTANCE hinstance, HWND hwnd, int screenWidth, int screenHeight, Input* in, bool VSYNC, bool FULL_SCREEN);

	bool frame();

protected:
	bool render();
	void gui();

private:
	//Vars
	PlaneMesh* plane_mesh_;
	CubeMesh* cube_mesh_;
	LinearSM* linearSM; 
};

