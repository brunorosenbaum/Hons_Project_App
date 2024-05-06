//Class for sending resources to the cs
#pragma once
#include "BaseShader.h"

class CSBuffer :
public BaseShader
{
public: 
	CSBuffer(ID3D11Device* device, HWND hwnd);
	CSBuffer(ID3D11Device* device, HWND hwnd, const wchar_t* cs);

	~CSBuffer();

	
	ID3D11ShaderResourceView* getSRV() { return srv; }

	//Methods built using Walbourn's sample tutorial ----------------------------------------------------------------

	//Creates buffers that will be sent to the CS - INPUT RESOURCE
	HRESULT createStructuredBuffer(ID3D11Device* device, UINT elementSize, UINT elementCount, void* initData, ID3D11Buffer** bufferOutPtr);

	//Creates shader resource view (SRV) for structured buffers we created before - READ ONLY
	// Shader resource view is used to bind input resources to shaders.
	HRESULT createBufferSRV(ID3D11Device* device, ID3D11Buffer* inputBuffer, ID3D11ShaderResourceView** SRVOutPtr);

	//Creates unordered access view (UAV) for output resource.
	//Unordered resource view is used to bind output resources to Compute Shaders.
	HRESULT createBufferUAV(ID3D11Device* device, ID3D11Buffer* inputBuffer, ID3D11UnorderedAccessView** UAVOutPtr);

	//Create a CPU accessible buffer and copies the content of a GPU buffer into it
	ID3D11Buffer* createCPUReadBuffer(ID3D11Device* device, ID3D11DeviceContext* deviceContext, ID3D11Buffer* bufferPtr, 
		int elementSize, int elementCount);

	//Unbinds buffers
	void unbind(ID3D11DeviceContext* device);

	void runComputeShader(ID3D11DeviceContext* deviceContext, ID3D11Buffer* cbufferPtr, UINT numResources,
		ID3D11ShaderResourceView** SRV_Ptr,
		ID3D11UnorderedAccessView* pUnorderedAccessView, 
		UINT X, UINT Y, UINT Z); 

protected:
	void initShader(const wchar_t* cs, const wchar_t* blank); //We do this bc otherwise baseshader yells at me

private:
	ID3D11Buffer* clusterBuffer;

	ID3D11ShaderResourceView* srv; //WRITE ONLY
	ID3D11UnorderedAccessView* uav; //READ&WRITE
	ID3D11Resource* CS_Output; 

};

