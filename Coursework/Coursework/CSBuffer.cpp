#include "CSBuffer.h"

CSBuffer::CSBuffer(ID3D11Device* device, HWND hwnd) : BaseShader(device, hwnd)
{
	initShader(L"LightningCS.cso", L"LightningPS.cso");
}


CSBuffer::~CSBuffer()
{
	// Release the layout.
	if (layout)
	{
		layout->Release();
		layout = 0;
	}
	BaseShader::~BaseShader();
}

void CSBuffer::runComputeShader(ID3D11DeviceContext* deviceContext, XMINT3 threadNum)
{
	HRESULT result;
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	DataBufferType* dataPtr;

	//Map resource to the pointer of type DataBuffer
	deviceContext->Map(dataBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	dataPtr = (DataBufferType*)mappedResource.pData;
	dataPtr->x = threadNum.x; //Basically works like memcpy
	dataPtr->y = threadNum.y;
	dataPtr->z = threadNum.z;
	deviceContext->Unmap(dataBuffer, 0); //Unmap

	//Set constant buffer & send databuffer containing whatever info.
	//In this case it has the number of threads but it can be anything that the CS can later work with
	deviceContext->CSSetConstantBuffers(0, 1, &dataBuffer);

	//Dispatch is for sending the number of threads to the CS that are gonna do your algorithm
	//So, 'sending the amount of minions' that are gonna go do the job.
	//If we send (2, 3, 5) then the group we're gonna send is of size 2*3*5 = 30, but in undefined order
	deviceContext->Dispatch(threadNum.x, threadNum.y, threadNum.z); 
}

void CSBuffer::initShader(const wchar_t* cs, const wchar_t* ps)
{
	D3D11_BUFFER_DESC csBufferDesc;
	loadComputeShader(cs); 
	loadPixelShader(ps);

	// Setup the description of the dynamic data buffer
	csBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	csBufferDesc.ByteWidth = sizeof(DataBufferType);
	csBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	csBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	csBufferDesc.MiscFlags = 0;
	csBufferDesc.StructureByteStride = 0;
	renderer->CreateBuffer(&csBufferDesc, NULL, &dataBuffer);

}

