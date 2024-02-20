#pragma once

// Includes
#include "DXF.h"
#include "LinearSM.h"
#include "QUAD_DBM_2D.h"
#include "ppm\ppm.hpp"


class App2_Cubes : public BaseApplication
{
public:

	App2_Cubes();
	~App2_Cubes();

	void init(HINSTANCE hinstance, HWND hwnd, int screenWidth, int screenHeight, Input* in, bool VSYNC, bool FULL_SCREEN);

	bool frame();
	bool loadImages(string inputFile);
	void addNodes(); 
	void renderGlow(/*string filename,*/ int scale); 


protected:
	bool render();
	void gui();

private:
	//Vars
	PlaneMesh* plane_mesh_;
	CubeMesh* cube_mesh_;
	LineMesh* line_mesh_; 

	LinearSM* linearSM;
	
	QUAD_DBM_2D* aggregate;
	int iterations = 10;

	// input image info
	int inputWidth = -1;
	int inputHeight = -1;
};

