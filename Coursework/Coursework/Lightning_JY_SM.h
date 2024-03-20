#pragma once
#include "BaseShader.h"
class Lightning_JY_SM :
    public BaseShader
{
public:
	Lightning_JY_SM(ID3D11Device* device, HWND hwnd);
	~Lightning_JY_SM();

	void setShaderParameters(ID3D11DeviceContext* deviceContext, const XMMATRIX& world, const XMMATRIX& view, const XMMATRIX& projection,
		XMFLOAT2 start, XMFLOAT2 end, XMFLOAT2 corner);
protected:
	void initShader(const wchar_t* vs, const wchar_t* ps) override;
private:
	ID3D11Buffer* matrixBuffer;
	ID3D11Buffer* dynVertxBuffer;
protected:
	
	struct DynamicVertexBufferType
	{
		XMFLOAT2 start;
		XMFLOAT2 end;
		XMFLOAT2 corner;
		XMFLOAT2 pad; 
	};
};

