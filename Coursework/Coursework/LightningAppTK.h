#pragma once

// CG METHOD
#include <chrono>

#include "DXF.h"
#include "LinearSM.h"
#include "LightningSM.h"
#include "QUAD_DBM_2D.h"
#include "ppm\ppm.hpp"

class LightningAppTK : public BaseApplication
{
public:

	LightningAppTK();
	~LightningAppTK();

	void init(HINSTANCE hinstance, HWND hwnd, int screenWidth, int screenHeight, Input* in, bool VSYNC, bool FULL_SCREEN);

	bool frame();
	bool loadImages(string inputFile);
	void addNodes(); 
	void renderGlow(string filename, int scale);
	void readLightningFile(); 

protected:
	bool render();
	void gui();

private:
	//Vars
	PlaneMesh* plane_mesh_;
	CubeMesh* cube_mesh_;
	LineMesh* line_mesh_; 

	LinearSM* linearSM;
	LightningSM* lightningSM; 

	QUAD_DBM_2D* aggregate;
	int iterations = 10;

	// input image info
	int inputWidth = -1;
	int inputHeight = -1;

	// input params
	string inputFile;
	string outputFile;

	//Timer
	std::chrono::steady_clock::time_point startTime_;
	std::chrono::steady_clock::time_point endTime_;
	string cgMeasurement = "cgMeasurement.csv";
	string cgFPS = "CGFps.csv"; 

};

