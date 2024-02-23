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
	lightningSM = new LightningSM(renderer->getDevice(), hwnd); 
	aggregate = new QUAD_DBM_2D(renderer->getDevice(), renderer->getDeviceContext(), 256, 256, iterations);

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

	//This is what Display() does in src code
	aggregate->drawQuadtreeCells(renderer->getDevice(), renderer->getDeviceContext(), linearSM, worldMatrix, viewMatrix, projectionMatrix);
	aggregate->drawSegments(renderer->getDevice(), renderer->getDeviceContext(), lightningSM, worldMatrix, viewMatrix, projectionMatrix);
	//This is 'idle'
	addNodes();
	//See if input is a .lightning file
	readLightningFile(); 

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

	//if (aggregate) delete aggregate;
	//aggregate = new QUAD_DBM_2D(renderer->getDevice(), renderer->getDeviceContext(), inputWidth, inputHeight, iterations);
	bool success = aggregate->readImage(start, attractor, repulsor, terminators, inputWidth, inputHeight);

	// delete the memory
	delete[] input;
	delete[] start;
	delete[] repulsor;
	delete[] attractor;
	delete[] terminators;

	return success;
}

void App2_Cubes::addNodes() //CALLED IDLE() IN SRC CODE
{
	
		for (int x = 0; x < 100; x++)
		{
			bool success = aggregate->addParticle();

			if (!success)
			{
				cout << " No nodes left to add! Is your terminator reachable?" << endl;
				//exit(1);
				return;
			}

			if (aggregate->hitGround())
			{
				/*glutPostRedisplay();
				cout << endl << endl;*/

				// write out the DAG file
				string lightningFile = inputFile.substr(0, inputFile.size() - 3) + string("lightning");
				cout << " Intermediate file " << lightningFile << " written." << endl;
				aggregate->writeDAG(lightningFile.c_str());

				// render the final EXR file
				renderGlow(outputFile, 5);
				//delete aggregate;
				//exit(0);
			}
		}
	
}

////////////////////////////////////////////////////////////////////////////
// render the glow
////////////////////////////////////////////////////////////////////////////
void App2_Cubes::renderGlow(string filename, int scale)
{
	int w = aggregate->xDagRes() * scale;
	int h = aggregate->yDagRes() * scale;

	// draw the DAG
	float*& source = aggregate->renderOffscreen(scale);

	// if there is no input dimensions specified, else there were input
	// image dimensions, so crop it
	if (inputWidth == -1)
	{
		inputWidth = aggregate->inputWidth();
		inputHeight = aggregate->inputHeight();
	}

	// copy out the cropped version
	int wCropped = inputWidth * scale;
	int hCropped = inputHeight * scale;
	float* cropped = new float[wCropped * hCropped];
	cout << endl << " Generating EXR image width: " << wCropped << " height: " << hCropped << endl;
	for (int y = 0; y < hCropped; y++)
		for (int x = 0; x < wCropped; x++)
		{
			int uncroppedIndex = x + y * w;
			int croppedIndex = x + y * wCropped;
			cropped[croppedIndex] = source[uncroppedIndex];
		}

	//// create the filter
	//apsf.generateKernelFast();

	//// convolve with FFT
	//bool success = FFT::convolve(cropped, apsf.kernel(), wCropped, hCropped, apsf.res(), apsf.res());

	//if (success) {
	//	EXR::writeEXR(filename.c_str(), cropped, wCropped, hCropped);
	//	cout << " " << filename << " written." << endl;
	//}
	//else
	//	cout << " Final image generation failed." << endl;

	delete[] cropped;
}

void App2_Cubes::readLightningFile()
{

	// see if the input is a *.lightning file
	if (inputFile.size() > 10)
	{
		string postfix = inputFile.substr(inputFile.size() - 9, inputFile.size());

		cout << " Using intermediate file " << inputFile << endl;
		if (postfix == string("lightning"))
		{
			aggregate->readDAG(inputFile.c_str());
			renderGlow(outputFile, 5);
			delete aggregate;
			return; //??????????????? kms
		}
	}
}
