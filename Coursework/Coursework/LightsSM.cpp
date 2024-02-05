#include "LightsSM.h"

LightsSM::LightsSM(ID3D11Device* device, HWND hwnd) : BaseShader(device, hwnd)
{
	initShader(L"LightVS.cso", L"LightPS.cso");
}

LightsSM::~LightsSM()
{
	// Release the layout.
	if (layout)
	{
		layout->Release();
		layout = 0;
	}

	// Release the light constant buffer.
	if (lightBuffer)
	{
		lightBuffer->Release();
		lightBuffer = 0;
	}



	//Release base shader components
	BaseShader::~BaseShader();
}

void LightsSM::initShader(const wchar_t* vs, const wchar_t* ps)
{
	D3D11_BUFFER_DESC lightBufferDesc;
	D3D11_BUFFER_DESC matrixBufferDesc;


	// Load (+ compile) shader files
	loadVertexShader(vs);
	loadPixelShader(ps);

	// Setup the description of the dynamic matrix constant buffer that is in the vertex shader.
	matrixBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	matrixBufferDesc.ByteWidth = sizeof(MatrixBufferType);
	matrixBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	matrixBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	matrixBufferDesc.MiscFlags = 0;
	matrixBufferDesc.StructureByteStride = 0;
	renderer->CreateBuffer(&matrixBufferDesc, NULL, &matrixBuffer);

	// Setup light buffer
	// Setup the description of the light dynamic constant buffer that is in the pixel shader.
	// Note that ByteWidth always needs to be a multiple of 16 if using D3D11_BIND_CONSTANT_BUFFER or CreateBuffer will fail.
	lightBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	lightBufferDesc.ByteWidth = sizeof(LightBufferType);
	lightBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	lightBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	lightBufferDesc.MiscFlags = 0;
	lightBufferDesc.StructureByteStride = 0;
	renderer->CreateBuffer(&lightBufferDesc, NULL, &lightBuffer);
}

void LightsSM::setShaderParameters(ID3D11DeviceContext* deviceContext, const XMMATRIX& world, const XMMATRIX& view, const XMMATRIX& projection, Light* lights[])
{
	HRESULT result;
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	MatrixBufferType* dataPtr;

	XMMATRIX tworld, tview, tproj;

	// Transpose the matrices to prepare them for the shader.
	tworld = XMMatrixTranspose(world);
	tview = XMMatrixTranspose(view);
	tproj = XMMatrixTranspose(projection);
	deviceContext->Map(matrixBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	dataPtr = (MatrixBufferType*)mappedResource.pData;
	dataPtr->world = tworld;// worldMatrix;
	dataPtr->view = tview;
	dataPtr->projection = tproj;
	deviceContext->Unmap(matrixBuffer, 0);

	deviceContext->VSSetConstantBuffers(0, 1, &matrixBuffer);

	//Send to pixel shader
	deviceContext->Map(lightBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	LightBufferType* lightPtr;
	lightPtr = (LightBufferType*)mappedResource.pData;

	lightPtr->ambient = lights[0]->getAmbientColour();
	//Directional
	lightPtr->diffuse[0] = lights[0]->getDiffuseColour();
	lightPtr->position[0] = XMFLOAT4(lights[0]->getPosition().x, lights[0]->getPosition().y, lights[0]->getPosition().z, 1);
	lightPtr->direction = lights[0]->getDirection();
	//Point light
	lightPtr->diffuse[1] = lights[1]->getDiffuseColour();
	lightPtr->position[1] = XMFLOAT4(lights[1]->getPosition().x, lights[1]->getPosition().y, lights[1]->getPosition().z, 1);

	lightPtr->padding = 0; 
	deviceContext->Unmap(lightBuffer, 0);

	deviceContext->PSSetConstantBuffers(0, 1, &lightBuffer);
}

