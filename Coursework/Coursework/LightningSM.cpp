#include "LightningSM.h"

LightningSM::LightningSM(ID3D11Device* device, HWND hwnd) : BaseShader(device, hwnd)
{

	initShader(L"LightningVS.cso", L"LightningPS.cso");

}

LightningSM::~LightningSM()
{
	// Release the layout.
	if (layout)
	{
		layout->Release();
		layout = 0;
	}
	BaseShader::~BaseShader();

}

void LightningSM::initShader(const wchar_t* vs, const wchar_t* ps)
{
	D3D11_BUFFER_DESC matrixBufferDesc;
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

	D3D11_BUFFER_DESC dynvertxBuffer;
	loadVertexShader(vs);
	loadPixelShader(ps);
	// Setup the description of the dynamic matrix constant buffer that is in the vertex shader.
	dynvertxBuffer.Usage = D3D11_USAGE_DYNAMIC;
	dynvertxBuffer.ByteWidth = sizeof(DynamicVertexBufferType);
	dynvertxBuffer.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	dynvertxBuffer.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	dynvertxBuffer.MiscFlags = 0;
	dynvertxBuffer.StructureByteStride = 0;
	renderer->CreateBuffer(&dynvertxBuffer, NULL, &dynVertxBuffer);
}
void LightningSM::setShaderParameters(ID3D11DeviceContext* deviceContext, const XMMATRIX& world, const XMMATRIX& view,
	const XMMATRIX& projection, XMFLOAT2 start_, XMFLOAT2 end_)
{
	HRESULT result;
	D3D11_MAPPED_SUBRESOURCE mappedResource; 
	MatrixBufferType* dataPtr;

	XMMATRIX tworld, tview, tproj;
	// Transpose the matrices to prepare them for the shader.
	tworld = XMMatrixTranspose(world);
	tview = XMMatrixTranspose(view);
	tproj = XMMatrixTranspose(projection);
	//Disable GPU access to the data (using write discard)
	deviceContext->Map(matrixBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	dataPtr = (MatrixBufferType*)mappedResource.pData; 
	dataPtr->world = tworld;// worldMatrix;
	dataPtr->view = tview;
	dataPtr->projection = tproj;
	deviceContext->Unmap(matrixBuffer, 0);

	deviceContext->VSSetConstantBuffers(0, 1, &matrixBuffer);

	D3D11_MAPPED_SUBRESOURCE VXmappedResource; //Initialize variable to 0,
	ZeroMemory(&VXmappedResource, sizeof(D3D11_MAPPED_SUBRESOURCE));//as you'll use it to change the resource
	//Pass into this array the data the new lines must be offset to 
	XMFLOAT2 vertxTransform[] = {
		XMFLOAT2(start_.x, start_.y),
		XMFLOAT2(end_.x, end_.y)
	};
	DynamicVertexBufferType* vertexPtr;

	//  Disable GPU access to the vertex buffer data.
	deviceContext->Map(dynVertxBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	//  Update the vertex buffer here.
	vertexPtr = (DynamicVertexBufferType*)mappedResource.pData;
	vertexPtr->start = vertxTransform[0]; //Then initialize the struct's members to that
	vertexPtr->end = vertxTransform[1];
	//  Reenable GPU access to the vertex buffer data.
	deviceContext->Unmap(dynVertxBuffer, 0);
	deviceContext->VSSetConstantBuffers(1, 1, &dynVertxBuffer);

}


