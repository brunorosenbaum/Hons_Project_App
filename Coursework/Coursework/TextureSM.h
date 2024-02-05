#pragma once
#include <Light.h>

#include "BaseShader.h"
class TextureSM :
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
	TextureSM(ID3D11Device* device, HWND hwnd);
	~TextureSM();

	void setShaderParameters(ID3D11DeviceContext* deviceContext, const XMMATRIX& world, const XMMATRIX& view, const XMMATRIX& projection, 
		ID3D11ShaderResourceView* texture, Light* lights[]);

protected:
	void initShader(const wchar_t* vs, const wchar_t* ps);

private:
	ID3D11Buffer* lightBuffer;
	ID3D11Buffer* matrixBuffer;
	ID3D11SamplerState* samplerState;

};

