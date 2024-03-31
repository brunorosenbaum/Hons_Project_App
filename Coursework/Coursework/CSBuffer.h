#pragma once
#include "BaseShader.h"
#include "RATIONAL_SOLVER.h"
class CSBuffer :
public BaseShader
{
public: 
	CSBuffer(ID3D11Device* device, HWND hwnd);
	~CSBuffer();

	void runComputeShader(ID3D11DeviceContext* deviceContext,
		ID3D11ShaderResourceView* srv, std::vector<CLUSTER> clusters);
	void createClusterBuffer(ID3D11Device* device, std::vector<CLUSTER> clusters);

	void createOutputUAV();
	ID3D11ShaderResourceView* getSRV() { return srv; }
	void unbind(ID3D11DeviceContext* device); 

protected:
	void initShader(const wchar_t* cs, const wchar_t* blank); //We do this bc otherwise baseshader yells at me

private:
	ID3D11Buffer* clusterBuffer;

	/*struct DataBufferType
	{
		UINT x, y, z; 
	};*/

	ID3D11ShaderResourceView* srv; //WRITE ONLY
	ID3D11UnorderedAccessView* uav; //READ&WRITE


};

