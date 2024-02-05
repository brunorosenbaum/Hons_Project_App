#include "App2_Cubes.h"

App2_Cubes::App2_Cubes()
{
	plane_mesh_ = nullptr;
	cube_mesh_ = nullptr;


}

void App2_Cubes::init(HINSTANCE hinstance, HWND hwnd, int screenWidth, int screenHeight, Input* in, bool VSYNC,
	bool FULL_SCREEN)
{
	BaseApplication::init(hinstance, hwnd, screenWidth, screenHeight, in, VSYNC, FULL_SCREEN);
	plane_mesh_ = new PlaneMesh(renderer->getDevice(), renderer->getDeviceContext());
	cube_mesh_ = new CubeMesh(renderer->getDevice(), renderer->getDeviceContext());
	linearSM = new LinearSM(renderer->getDevice(), hwnd); 
}

App2_Cubes::~App2_Cubes()
{
	BaseApplication::~BaseApplication();
	if (plane_mesh_) { delete plane_mesh_; plane_mesh_ = 0; }
	if (cube_mesh_) { delete cube_mesh_; cube_mesh_ = 0; }

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

	//plane_mesh_->sendData(renderer->getDeviceContext());

	cube_mesh_->sendData(renderer->getDeviceContext());
	linearSM->setShaderParameters(renderer->getDeviceContext(), worldMatrix, viewMatrix, projectionMatrix);
	linearSM->render(renderer->getDeviceContext(), cube_mesh_->getIndexCount());
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

	// Render UI
	ImGui::Render();
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
}
