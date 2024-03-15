#include "LightningAppJY.h"

LightningAppJY::LightningAppJY()
{
	grid_Cell_mesh = nullptr;
	grid_bounds_mesh = nullptr;
	lightning_mesh_ = nullptr;
	tree_nodes.clear(); 
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
	lightning_Generator->InitializeGrid("res/lightning_32.map"); 

	sceneSize = screenWidth; sceneHalf = sceneSize * 0.5f;

	initLightning(); 
}

void LightningAppJY::initLightning()
{
	lightning_Generator->ProcessLightning();
	//float fDiff = -fSceneSize / g_lightningGenerator.GetGridSize();

	LIGHTNING_TREE& lightning_tree = lightning_Generator->GetLightningTree();
	tree_nodes = lightning_tree.GetNodes(); //Initialize to nodes from lightning tree

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
		yTransl = /*difference_ **/ i /*+ sceneHalf*/;

		for(int j = 0; j < gridSize * 0.5; ++j)
		{
			xTransl = -/*difference_ **/ j /*+ sceneHalf*/;

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

void LightningAppJY::drawLightning(XMMATRIX world, XMMATRIX view, XMMATRIX projection)
{
	LIGHTNING_TREE_NODE* nodePtr;
	float difference_ = -1;
	float center_ = difference_ * 0.5;
	float startX, startY, endX, endY;

	auto itr = tree_nodes.begin();
	int i = 0; 
	while (itr != tree_nodes.end() && i < 2) //Set xy coords of starting and end points of lightning segments
	{
		nodePtr = *itr;
		if (nodePtr && nodePtr->parent_) //If is NOT root
		{
			if (nodePtr->parent_) {
			startX = -difference_ * nodePtr->parent_->x_ + center_;
			startY = difference_ * nodePtr->parent_->y_ + center_;
			endX = -difference_ * nodePtr->x_;
			endY = -difference_ * nodePtr->y_;
			}
			
		}
		else //If it IS root
		{
			startX = -difference_ + center_;
			startY = difference_ + center_;
			endX = -difference_ * nodePtr->x_;
			endY = -difference_ * nodePtr->y_;
		}
		
		float xDiff = endX - startX; 
		float yDiff = endY - startY; 
		float fThetaInRadians = atan2f(yDiff, xDiff);
		if (0 == yDiff && xDiff < 0) fThetaInRadians = 0.0f;

		XMFLOAT2 vCorner; 
		vCorner.x = sinf(fThetaInRadians);
		vCorner.y = cosf(fThetaInRadians);


		XMFLOAT2 start_ = XMFLOAT2(startX + vCorner.x, startY - vCorner.y);
		XMFLOAT2 end_ = XMFLOAT2(endX + vCorner.x, endY + vCorner.y);


		lightning_mesh_->sendData(renderer->getDeviceContext(), D3D_PRIMITIVE_TOPOLOGY_LINELIST);
		lightning_SM->setShaderParameters(renderer->getDeviceContext(), world, view, projection, start_, end_);
		lightning_SM->render(renderer->getDeviceContext(), lightning_mesh_->getIndexCount());

		++itr; ++i; 
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

	//initQuadGrid(worldMatrix, viewMatrix, projectionMatrix); 
	drawLightning(worldMatrix, viewMatrix, projectionMatrix); 
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
