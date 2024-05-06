#pragma once
#include "BaseShader.h"

class LightningSM : public BaseShader
{
public:
	LightningSM(ID3D11Device* device, HWND hwnd);
	~LightningSM();

	void setShaderParameters(ID3D11DeviceContext* deviceContext, const XMMATRIX& world, const XMMATRIX& view, const XMMATRIX& projection, 
		XMFLOAT2 start, XMFLOAT2 end);
protected:
	void initShader(const wchar_t* vs, const wchar_t* ps);
private:
	ID3D11Buffer* matrixBuffer;
	ID3D11Buffer* dynVertxBuffer;
protected:
	
	struct DynamicVertexBufferType
	{
		XMFLOAT2 start;
		XMFLOAT2 end;
	};
	
};

