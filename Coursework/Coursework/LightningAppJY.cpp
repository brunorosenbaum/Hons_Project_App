#include "LightningAppJY.h"

LightningAppJY::LightningAppJY()
{
	grid_Cell_mesh = nullptr; 
}

LightningAppJY::~LightningAppJY()
{
	BaseApplication::~BaseApplication();
	delete lightning_Generator; lightning_Generator = NULL;
	delete grid_Cell_mesh; grid_Cell_mesh = NULL;

}

void LightningAppJY::init(HINSTANCE hinstance, HWND hwnd, int screenWidth, int screenHeight, Input* in, bool VSYNC, bool FULL_SCREEN)
{
	BaseApplication::init(hinstance, hwnd, screenWidth, screenHeight, in, VSYNC, FULL_SCREEN);

	lightning_SM = new LightningSM(renderer->getDevice(), hwnd);
	linear_SM = new LinearSM(renderer->getDevice(), hwnd);

	grid_Cell_mesh = new CellMesh(renderer->getDevice(), renderer->getDeviceContext());

	lightning_Generator = new RATIONAL_SOLVER();

	sceneSize = screenWidth; sceneHalf = sceneSize * 0.5f;

}

bool LightningAppJY::frame()
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

void LightningAppJY::initQuadGrid(XMMATRIX world, XMMATRIX view, XMMATRIX projection)
{
	//For matrix transformation of vertices
	int gridSize = 10; //lightning_Generator->GetGridSize();
	float xTransl, yTransl;
	float difference_ = -1;

	for(int i = 0; i  < gridSize * 0.5; ++i)
	{
		yTransl = difference_ * i /*+ sceneHalf*/;

		XMMATRIX Ytranslation_ = XMMatrixTranslation(0, yTransl, 0);
		//XMMATRIX m =  translation_ * XMMatrixScaling(0.1, 0.1, 0.1);
		grid_Cell_mesh->sendData(renderer->getDeviceContext());
		linear_SM->setShaderParameters(renderer->getDeviceContext(), Ytranslation_, view, projection, false);
		linear_SM->render(renderer->getDeviceContext(), grid_Cell_mesh->getIndexCount());

		for(int j = 0; j < gridSize * 0.5; ++j)
		{
			xTransl = -difference_ * j /*+ sceneHalf*/;
			XMMATRIX Xtranslation_ = XMMatrixTranslation(xTransl, yTransl, 0);
			grid_Cell_mesh->sendData(renderer->getDeviceContext());
			linear_SM->setShaderParameters(renderer->getDeviceContext(), Xtranslation_, view, projection, false);
			linear_SM->render(renderer->getDeviceContext(), grid_Cell_mesh->getIndexCount());


		}
		

	}

}

bool LightningAppJY::render()
{
	renderer->beginScene(0.36f, 0.03f, 0.19f, 1.0f);
	camera->update();
	// Get the world, view, projection, and ortho matrices from the camera and Direct3D objects.
	XMMATRIX worldMatrix = renderer->getWorldMatrix();
	XMMATRIX viewMatrix = camera->getViewMatrix();
	XMMATRIX projectionMatrix = renderer->getProjectionMatrix();

	initQuadGrid(worldMatrix, viewMatrix, projectionMatrix); 

	// Render GUI
	gui();

	// Present the rendered scene to the screen.
	renderer->endScene();

	return true;
}

void LightningAppJY::gui()
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
