#pragma once
#include "BaseShader.h"

class LightningSM : public BaseShader
{
public:
	LightningSM(ID3D11Device* device, HWND hwnd);
	~LightningSM();

	void setShaderParameters(ID3D11DeviceContext* deviceContext, const XMMATRIX& world, const XMMATRIX& view, const XMMATRIX& projection, 
		XMMATRIX start, XMMATRIX end);
protected:
	void initShader(const wchar_t* vs, const wchar_t* ps);
private:
	ID3D11Buffer* matrixBuffer;
	ID3D11Buffer* translationBuffer;
protected:
	struct TranslationBufferType
	{
		XMMATRIX start;
		XMMATRIX end;
	};
};

