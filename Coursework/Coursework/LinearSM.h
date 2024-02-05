#pragma once
#include "BaseShader.h"

class LinearSM : public BaseShader
{
public:
	LinearSM(ID3D11Device* device, HWND hwnd);
	~LinearSM();

	void setShaderParameters(ID3D11DeviceContext* deviceContext, const XMMATRIX& world, const XMMATRIX& view, const XMMATRIX& projection);

protected:
	void initShader(const wchar_t* vs, const wchar_t* ps);
private: 
	ID3D11Buffer* matrixBuffer;

};

