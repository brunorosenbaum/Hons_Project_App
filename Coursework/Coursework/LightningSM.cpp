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

	D3D11_BUFFER_DESC boolBufferDesc;
	loadVertexShader(vs);
	loadPixelShader(ps);

	// Setup the description of the dynamic matrix constant buffer that is in the vertex shader.
	boolBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	boolBufferDesc.ByteWidth = sizeof(TranslationBufferType);
	boolBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	boolBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	boolBufferDesc.MiscFlags = 0;
	boolBufferDesc.StructureByteStride = 0;
	renderer->CreateBuffer(&boolBufferDesc, NULL, &translationBuffer);
}
void LightningSM::setShaderParameters(ID3D11DeviceContext* deviceContext, const XMMATRIX& world, const XMMATRIX& view,
	const XMMATRIX& projection, XMMATRIX start_, XMMATRIX end_)
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

	TranslationBufferType* translPtr;
	deviceContext->Map(translationBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	translPtr = (TranslationBufferType*)mappedResource.pData;
	//Transpose
	XMMATRIX tStart, tEnd;
	tStart = XMMatrixTranspose(start_); 
	tEnd = XMMatrixTranspose(end_); 
	translPtr->start = tStart; 
	translPtr->end = tEnd; 
	deviceContext->Unmap(translationBuffer, 0);
	deviceContext->VSSetConstantBuffers(1, 1, &translationBuffer);

}


