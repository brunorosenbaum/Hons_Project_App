#include "CSBuffer.h"

CSBuffer::CSBuffer(ID3D11Device* device, HWND hwnd) : BaseShader(device, hwnd)
{
	initShader(L"LightningCS.cso", NULL);
}

CSBuffer::CSBuffer(ID3D11Device* device, HWND hwnd, const wchar_t* cs) : BaseShader(device, hwnd)
{
	loadComputeShader(cs);
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



void CSBuffer::initShader(const wchar_t* cs, const wchar_t* blank)
{
	loadComputeShader(cs);
	
}


void CSBuffer::unbind(ID3D11DeviceContext* device)
{
	ID3D11ShaderResourceView* nullSRV[] = { NULL };
	device->CSSetShaderResources(0, 1, nullSRV);

	// Unbind output from compute shader
	ID3D11UnorderedAccessView* nullUAV[] = { NULL };
	device->CSSetUnorderedAccessViews(0, 1, nullUAV, 0);

	// Disable Compute Shader
	device->CSSetShader(nullptr, nullptr, 0);
}

void CSBuffer::runComputeShader(ID3D11DeviceContext* deviceContext, ID3D11Buffer* cbufferPtr, UINT numResources, 
	ID3D11ShaderResourceView** SRV_Ptr, ID3D11UnorderedAccessView* UAV_Ptr, 
	UINT X, UINT Y, UINT Z)
{
	deviceContext->CSSetShader(computeShader, nullptr, 0); 
	deviceContext->CSSetShaderResources(0, numResources, SRV_Ptr);
	deviceContext->CSSetUnorderedAccessViews(0, 1, &UAV_Ptr, nullptr);
	if(cbufferPtr)
	{
		D3D11_MAPPED_SUBRESOURCE mappedResource;
		deviceContext->Map(cbufferPtr, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
		memcpy(mappedResource.pData, nullptr, 0); //Don't understand why the source is 0 memory
		deviceContext->Unmap(cbufferPtr, 0);
		deviceContext->CSSetConstantBuffers(0, 1, &cbufferPtr);
	}
	deviceContext->Dispatch(X, Y, Z);
	unbind(deviceContext); 
}

HRESULT CSBuffer::createStructuredBuffer(ID3D11Device* device, UINT elementSize, UINT elementCount, void* initData,
	ID3D11Buffer** bufferOutPtr)
{ //This method fills out the input resource from data processed in the CPU, to send to the GPU.
	//These will be used to, at the same time, fill their respective SRVs.
	*bufferOutPtr = nullptr;
	D3D11_BUFFER_DESC bufferDesc = { };
	bufferDesc.BindFlags = D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_SHADER_RESOURCE;
	bufferDesc.ByteWidth = elementSize * elementCount;
	bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE; //We write to it from the cpu?
	bufferDesc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED; 
	bufferDesc.StructureByteStride = elementSize;

	//Init data for the buffer

	D3D11_SUBRESOURCE_DATA initData_;
	initData_.pSysMem = initData;
	return device->CreateBuffer(&bufferDesc, initData ? &initData_ : nullptr, bufferOutPtr);

}

HRESULT CSBuffer::createBufferSRV(ID3D11Device* device, ID3D11Buffer* inputBuffer, ID3D11ShaderResourceView** SRVOutPtr)
{
	D3D11_BUFFER_DESC bufferDesc = {};
	inputBuffer->GetDesc(&bufferDesc);

	CD3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER; //It uses BUFFEREX in the sample project, but that's for raw buffers, which this isnt, so..?
	//I could use unknown too (ask)
	srvDesc.Buffer.FirstElement = 0;
	srvDesc.Format = DXGI_FORMAT_UNKNOWN;
	srvDesc.Buffer.NumElements = bufferDesc.ByteWidth / bufferDesc.StructureByteStride;

	return device->CreateShaderResourceView(inputBuffer, &srvDesc, SRVOutPtr); 
}

HRESULT CSBuffer::createBufferUAV(ID3D11Device* device, ID3D11Buffer* inputBuffer, ID3D11UnorderedAccessView** UAVOutPtr)
{
	D3D11_BUFFER_DESC bufferDesc = {};
	inputBuffer->GetDesc(&bufferDesc);

	D3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
	uavDesc.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
	uavDesc.Buffer.FirstElement = 0;
	uavDesc.Format = DXGI_FORMAT_UNKNOWN;
	uavDesc.Buffer.NumElements = bufferDesc.ByteWidth / bufferDesc.StructureByteStride;

	return device->CreateUnorderedAccessView(inputBuffer, &uavDesc, UAVOutPtr); 
}

ID3D11Buffer* CSBuffer::createCPUReadBuffer(ID3D11Device* device, ID3D11DeviceContext* deviceContext, ID3D11Buffer* bufferPtr, 
	int elementSize, int elementCount)
{
	ID3D11Buffer* cpuBuffer = nullptr;
	D3D11_BUFFER_DESC bufferDesc = {};
	bufferPtr->GetDesc(&bufferDesc);
	bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
	bufferDesc.Usage = D3D11_USAGE_STAGING; //usage directly reflects whether a resource is accessible by the CPU and/or GPU
	//Staging -> A resource that supports data transfer (copy) from the GPU to the CPU.
	bufferDesc.BindFlags = 0; bufferDesc.MiscFlags = 0;
	bufferDesc.ByteWidth = elementSize * elementCount;
	bufferDesc.StructureByteStride = elementSize; 
	device->CreateBuffer(&bufferDesc, nullptr, &cpuBuffer);
	deviceContext->CopyResource(cpuBuffer, bufferPtr);

	return cpuBuffer; 
}

