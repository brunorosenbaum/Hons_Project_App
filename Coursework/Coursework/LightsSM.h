#pragma once
#include <Light.h>

#include "BaseShader.h"

//Very basic shader, not crucial but maybe will be needed further on to be modified
//So it's better that it's all set up

class LightsSM :
    public BaseShader
{
private:

	struct LightBufferType
	{
		XMFLOAT4 ambient; //16 bytes
		XMFLOAT4 diffuse[2]; //32
		XMFLOAT4 position[2]; //32
		XMFLOAT3 direction; //12
		float padding; //4
	};

public:
	LightsSM(ID3D11Device* device, HWND hwnd);
	~LightsSM();

	void setShaderParameters(ID3D11DeviceContext* deviceContext, const XMMATRIX& world, const XMMATRIX& view, const XMMATRIX& projection, 
		Light* lights[]);

private:
	void initShader(const wchar_t* vs, const wchar_t* ps);

private:

	ID3D11Buffer* lightBuffer;
	ID3D11Buffer* matrixBuffer;
};

