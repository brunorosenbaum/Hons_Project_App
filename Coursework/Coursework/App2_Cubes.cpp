#include "App2_Cubes.h"

App2_Cubes::App2_Cubes()
{
	plane_mesh_ = nullptr;
	cube_mesh_ = nullptr;
	line_mesh_ = nullptr;
	

}

void App2_Cubes::init(HINSTANCE hinstance, HWND hwnd, int screenWidth, int screenHeight, Input* in, bool VSYNC,
	bool FULL_SCREEN)
{
	BaseApplication::init(hinstance, hwnd, screenWidth, screenHeight, in, VSYNC, FULL_SCREEN);
	plane_mesh_ = new PlaneMesh(renderer->getDevice(), renderer->getDeviceContext());
	cube_mesh_ = new CubeMesh(renderer->getDevice(), renderer->getDeviceContext());
	line_mesh_ = new LineMesh(renderer->getDevice(), renderer->getDeviceContext());
	linearSM = new LinearSM(renderer->getDevice(), hwnd); 
	aggregate = new QUAD_DBM_2D(renderer->getDevice(), renderer->getDeviceContext(), 256, 256,  iterations);

}

App2_Cubes::~App2_Cubes()
{
	BaseApplication::~BaseApplication();
	if (plane_mesh_) { delete plane_mesh_; plane_mesh_ = 0; }
	if (cube_mesh_) { delete cube_mesh_; cube_mesh_ = 0; }
	if (line_mesh_) { delete line_mesh_; line_mesh_ = 0; }

}

bool App2_Cubes::frame()
{
	bool result = BaseApplication::frame();
	if (!result)
	{
		return false;
	}

	// Render the graphics.
	result = render();
	if (!result)
	{
		return false;
	}

	return true;
}

bool App2_Cubes::render()
{
	renderer->beginScene(0.19f, 0.03f, 0.36f, 1.0f);
	camera->update();
	// Get the world, view, projection, and ortho matrices from the camera and Direct3D objects.
	XMMATRIX worldMatrix = renderer->getWorldMatrix();
	XMMATRIX viewMatrix = camera->getViewMatrix();
	XMMATRIX projectionMatrix = renderer->getProjectionMatrix();

	// read in the *.ppm input file
	string inputFile = "examples/hint.ppm"; 
	if (!loadImages(inputFile))
	{
		cout << " ERROR: " << inputFile.c_str() << " is not a valid PPM file." << endl;
		return 1;
	}
	//line_mesh_->sendData(renderer->getDeviceContext());
	//linearSM->setShaderParameters(renderer->getDeviceContext(), worldMatrix, viewMatrix, projectionMatrix);
	//linearSM->render(renderer->getDeviceContext(), line_mesh_->getIndexCount());
	aggregate->drawQuadtreeCells(renderer->getDevice(), renderer->getDeviceContext(), linearSM, worldMatrix, viewMatrix, projectionMatrix);
	aggregate->drawSegments(renderer->getDevice(), renderer->getDeviceContext(), linearSM, worldMatrix, viewMatrix, projectionMatrix);

	// Render GUI
	gui();

	// Present the rendered scene to the screen.
	renderer->endScene();

	return true;
}

void App2_Cubes::gui()
{
	// Force turn off unnecessary shader stages.
	renderer->getDeviceContext()->GSSetShader(NULL, NULL, 0);
	renderer->getDeviceContext()->HSSetShader(NULL, NULL, 0);
	renderer->getDeviceContext()->DSSetShader(NULL, NULL, 0);
	ImGui::Text("FPS: %.2f", timer->getFPS());
	ImGui::Checkbox("Wireframe", &wireframeToggle); 
	// Render UI
	ImGui::Render();
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
}

bool App2_Cubes::loadImages(string inputFile)
{
	// load the files
	int inputWidth = -1;
	int inputHeight = -1;
	unsigned char* input = NULL;
	LoadPPM(inputFile.c_str(), input, inputWidth, inputHeight);

	unsigned char* start = new unsigned char[inputWidth * inputHeight];
	unsigned char* repulsor = new unsigned char[inputWidth * inputHeight];
	unsigned char* attractor = new unsigned char[inputWidth * inputHeight];
	unsigned char* terminators = new unsigned char[inputWidth * inputHeight];

	// composite RGB channels into one
	for (int x = 0; x < inputWidth * inputHeight; x++)
	{
		start[x] = (input[3 * x] == 255) ? 255 : 0;
		repulsor[x] = (input[3 * x + 1] == 255) ? 255 : 0;
		attractor[x] = (input[3 * x + 2] == 255) ? 255 : 0;
		terminators[x] = 0;

		if (input[3 * x] + input[3 * x + 1] + input[3 * x + 2] == 255 * 3)
		{
			terminators[x] = 255;
			start[x] = repulsor[x] = attractor[x] = 0;
		}
	}

	if (aggregate) delete aggregate;
	aggregate = new QUAD_DBM_2D(renderer->getDevice(), renderer->getDeviceContext(), inputWidth, inputHeight, iterations);
	bool success = aggregate->readImage(start, attractor, repulsor, terminators, inputWidth, inputHeight);

	// delete the memory
	delete[] input;
	delete[] start;
	delete[] repulsor;
	delete[] attractor;
	delete[] terminators;

	return success;
}