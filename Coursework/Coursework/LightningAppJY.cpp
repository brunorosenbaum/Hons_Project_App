#include "LightningAppJY.h"

LightningAppJY::LightningAppJY()
{
	grid_Cell_mesh = nullptr;
	grid_bounds_mesh = nullptr;
	lightning_mesh_ = nullptr; 
}

LightningAppJY::~LightningAppJY()
{
	BaseApplication::~BaseApplication();
	delete lightning_Generator; lightning_Generator = NULL;
	delete grid_Cell_mesh; grid_Cell_mesh = NULL;
	delete grid_bounds_mesh; grid_bounds_mesh = NULL;
	delete lightning_mesh_; lightning_mesh_ = NULL; 
}

void LightningAppJY::init(HINSTANCE hinstance, HWND hwnd, int screenWidth, int screenHeight, Input* in, bool VSYNC, bool FULL_SCREEN)
{
	BaseApplication::init(hinstance, hwnd, screenWidth, screenHeight, in, VSYNC, FULL_SCREEN);

	lightning_SM = new LightningSM(renderer->getDevice(), hwnd);
	linear_SM = new LinearSM(renderer->getDevice(), hwnd);

	grid_Cell_mesh = new CellMesh(renderer->getDevice(), renderer->getDeviceContext());
	grid_bounds_mesh = new CellBoundsMesh(renderer->getDevice(), renderer->getDeviceContext());
	lightning_mesh_ = new LightningMesh(renderer->getDevice(), renderer->getDeviceContext());

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

		for(int j = 0; j < gridSize * 0.5; ++j)
		{
			xTransl = -difference_ * j /*+ sceneHalf*/;

			XMMATRIX scale_ = XMMatrixScaling(0.1, 0.1, 0.1);
			XMMATRIX translation_ = /*scale_ **/ XMMatrixTranslation(xTransl, yTransl, 0) * scale_;
			grid_Cell_mesh->sendData(renderer->getDeviceContext());
			linear_SM->setShaderParameters(renderer->getDeviceContext(), translation_, view, projection, false);
			linear_SM->render(renderer->getDeviceContext(), grid_Cell_mesh->getIndexCount());

		}
	}

	//TODO: FIGURE THIS OUT
	//For boundaries
	/*XMMATRIX bscale_ = XMMatrixScaling(gridSize/2, gridSize/2, 0);*/
	XMMATRIX btranslation_ = XMMatrixTranslation(0,-1, 0);

	grid_bounds_mesh->sendData(renderer->getDeviceContext());
	linear_SM->setShaderParameters(renderer->getDeviceContext(),  btranslation_ * world, view, projection, true);
	linear_SM->render(renderer->getDeviceContext(), grid_bounds_mesh->getIndexCount()); 
}

void LightningAppJY::initLightning(XMMATRIX world, XMMATRIX view, XMMATRIX projection)
{
	lightning_Generator->ProcessLightning();
	//float fDiff = -fSceneSize / g_lightningGenerator.GetGridSize();
	float difference_ = -1;
	float center_ = difference_ * 0.5;
	float startX, startY, endX, endY;

	LIGHTNING_TREE_NODE* nodePtr;
	LIGHTNING_TREE& lightning_tree = lightning_Generator->GetLightningTree();
	std::vector<LIGHTNING_TREE_NODE*> tree_nodes = lightning_tree.GetNodes();

	auto itr = tree_nodes.begin();
	while(itr != tree_nodes.end()) //Set xy coords of starting and end points of lightning segments
	{
		nodePtr = *itr;
		if(nodePtr && nodePtr->parent_)
		{
			startX = -difference_ * nodePtr->parent_->x_ + center_; 
			startY = difference_ * nodePtr->parent_->y_ + center_;
			endX = -difference_ * nodePtr->x_;
			endY = -difference_ * nodePtr->y_;
		}
		XMFLOAT2 start_ = XMFLOAT2(startX, startY);
		XMFLOAT2 end_ = XMFLOAT2(endX, endY);
		lightning_mesh_->sendData(renderer->getDeviceContext(), D3D_PRIMITIVE_TOPOLOGY_LINELIST);
		lightning_SM->setShaderParameters(renderer->getDeviceContext(), world, view, projection, start_, end_);
		lightning_SM->render(renderer->getDeviceContext(), lightning_mesh_->getIndexCount());

		++itr; 
	}
}

bool LightningAppJY::render()
{
	renderer->beginScene(0.19f, 0.03f, 0.36f, 1.0f);
	camera->update();
	// Get the world, view, projection, and ortho matrices from the camera and Direct3D objects.
	XMMATRIX worldMatrix = renderer->getWorldMatrix();
	XMMATRIX viewMatrix = camera->getViewMatrix();
	XMMATRIX projectionMatrix = renderer->getProjectionMatrix();

	initQuadGrid(worldMatrix, viewMatrix, projectionMatrix); 
	initLightning(worldMatrix, viewMatrix, projectionMatrix); 
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
