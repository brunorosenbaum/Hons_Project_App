#include "CSBuffer.h"

CSBuffer::CSBuffer(ID3D11Device* device, HWND hwnd) : BaseShader(device, hwnd)
{
	initShader(L"LightningCS.cso", NULL);
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
	createOutputUAV();
}

void CSBuffer::runComputeShader(ID3D11DeviceContext* deviceContext, ID3D11ShaderResourceView* srv, 
	std::vector<CLUSTER> clusters)
{
	HRESULT result;
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	result = deviceContext->Map(clusterBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	size_t byteSize = clusters.size();
	memcpy_s(mappedResource.pData, byteSize, clusters.data(), byteSize);
	deviceContext->Unmap(clusterBuffer, 0); 
	
	//Dispatch is for sending the number of threads to the CS that are gonna do your algorithm
	//So, 'sending the amount of minions' that are gonna go do the job.
	//If we send (2, 3, 5) then the group we're gonna send is of size 2*3*5 = 30, but in undefined order
	deviceContext->Dispatch(16, 16, 1);

	deviceContext->CSSetShaderResources(0, 1, &srv);
	deviceContext->CSSetUnorderedAccessViews(0, 1, &uav, 0);
}

void CSBuffer::createClusterBuffer(ID3D11Device* device, std::vector<CLUSTER> clusters)
{
	HRESULT result;

	//Create cluster buffer description
	D3D11_BUFFER_DESC clusterBufDesc;
	clusterBufDesc.ByteWidth = sizeof(clusters);
	clusterBufDesc.Usage = D3D11_USAGE_DYNAMIC;
	clusterBufDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE; //We write to it from the cpu
	clusterBufDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	clusterBufDesc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
	clusterBufDesc.StructureByteStride = clusters.size(); //Is this even ok? total size of the buffer in bytes

	//Create subresource - which is p much 0
	D3D11_SUBRESOURCE_DATA clusterSubrsc;
	clusterSubrsc.pSysMem = 0; 
	clusterSubrsc.SysMemPitch = 0; //Idk what this is for rn
	clusterSubrsc.SysMemSlicePitch = 0;

	//Create buffer with cluster desc & subresource
	result = device->CreateBuffer(&clusterBufDesc, &clusterSubrsc, NULL);

	//Create SRV
	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
	srvDesc.Format = DXGI_FORMAT_UNKNOWN; //0, unknown format since were passing in a custom struct
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
	srvDesc.Buffer.FirstElement = 0; 
	srvDesc.Buffer.NumElements = clusters.size();

	result = device->CreateShaderResourceView(clusterBuffer, &srvDesc, &srv);



}

void CSBuffer::createOutputUAV()
{
	
//Make UAV
	D3D11_UNORDERED_ACCESS_VIEW_DESC descUAV;
	ZeroMemory(&descUAV, sizeof(descUAV));
	descUAV.Format = DXGI_FORMAT_R32G32B32A32_FLOAT; ;// DXGI_FORMAT_UNKNOWN;
	descUAV.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2D;
	descUAV.Texture2D.MipSlice = 0;
	renderer->CreateUnorderedAccessView(srv, &descUAV, &uav);

	//Make SRV
	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
	srvDesc.Format = textureDesc.Format;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MostDetailedMip = 0;
	srvDesc.Texture2D.MipLevels = 1;
	renderer->CreateShaderResourceView(m_tex, &srvDesc, &srv);

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



