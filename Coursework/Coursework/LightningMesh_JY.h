#pragma once
#include <BaseMesh.h>

//Different structure from lightning mesh for the CG method as it has more vertices
class LightningMesh_JY : public BaseMesh
{
public:
	LightningMesh_JY(ID3D11Device* device, ID3D11DeviceContext* deviceContext); 
	~LightningMesh_JY();

	void sendData(ID3D11DeviceContext* deviceContext, D3D_PRIMITIVE_TOPOLOGY top = D3D_PRIMITIVE_TOPOLOGY_LINELIST);
	void initBuffers(ID3D11Device* device);

};

