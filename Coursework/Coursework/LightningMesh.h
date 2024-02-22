#pragma once
#include "LineMesh.h"
class LightningMesh :
    public LineMesh
{
public:
	LightningMesh(ID3D11Device* device, ID3D11DeviceContext* deviceContext);
	LightningMesh(ID3D11Device* device, ID3D11DeviceContext* deviceContext, float xPosStart, float yPosStart, float xPosEnd, float yPosEnd);
	~LightningMesh();

	//void sendData(ID3D11DeviceContext*);
	void sendData(ID3D11DeviceContext* deviceContext, D3D_PRIMITIVE_TOPOLOGY top = D3D_PRIMITIVE_TOPOLOGY_LINELIST);

protected:
	void initBuffers(ID3D11Device* device);
	float xPosStart_ = 0;
	float yPosStart_ = 0;
	float xPosEnd_ = 0;
	float yPosEnd_ = 0; 
};

