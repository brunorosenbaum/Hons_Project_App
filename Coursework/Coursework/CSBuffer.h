#pragma once
#include "BaseShader.h"

class CSBuffer :
public BaseShader
{
public: 
	CSBuffer(ID3D11Device* device, HWND hwnd);
	~CSBuffer();

	void runComputeShader(ID3D11DeviceContext* deviceContext,
		XMINT3 xyz);

protected:
	void initShader(const wchar_t* cs, const wchar_t* ps);

private:
	ID3D11Buffer* dataBuffer;

	struct DataBufferType
	{
		UINT x, y, z; 
	};

};

