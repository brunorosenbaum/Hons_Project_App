// Application.h
#ifndef _APP1_H
#define _APP1_H

// Includes
#include "DXF.h"
#include "TextureSM.h"
#include "LightsSM.h"

class App1 : public BaseApplication
{
public:

	App1();
	~App1();
	void init(HINSTANCE hinstance, HWND hwnd, int screenWidth, int screenHeight, Input* in, bool VSYNC, bool FULL_SCREEN);

	bool frame();

protected:
	bool render();
	void drawPlane(XMMATRIX& world, XMMATRIX& view, XMMATRIX& projection);
	void drawObjects(XMMATRIX& world, XMMATRIX& view, XMMATRIX& projection);
	float InvLerp(float a, float b, float t); 
	float Lerp(float a, float b, float t);
	float Remap(float iMin, float iMax, float oMin, float oMax, float v); 

	void gui();

private:

	//Geometry meshes
	PlaneMesh* plane_mesh_;
	SphereMesh* directional_light_sphere_;
	CubeMesh* cube_mesh_;
	CubeMesh* parent_segment_;
	
	CubeMesh* childsegments[2][2]; 
	CubeMesh* secondChildsegments[3]; 

	//Lights
	Light* lights_[2]; //Directional 1, point 2
	float light_direction_[3];
	float directional_position_[3]; 

	//Shader managers
	TextureSM* textureSM;
	LightsSM* lightsSM;

	//Generate random degrees between -12 and +12 that'll be the tilt in z and x axis
	float rotX = 0;
	float rotZ = 0;

	float rotXChild1 = 0;
	float rotZChild1 = 0;

	float rotXChild2 = 0;
	float rotZChild2 = 0;

	const int degrees = 16; 
};

#endif