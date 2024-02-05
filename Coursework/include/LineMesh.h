// LineMesh.h
// A simple mesh for demostrating the geometry shader
// Instead producing a triangle list, produce a line list.
// This list is a line and can be used by the geometry shader to generate geometry.


#ifndef _LineMesh_H_
#define _LineMesh_H_

#include "BaseMesh.h"

using namespace DirectX;

class LineMesh : public BaseMesh
{

public:
	LineMesh(ID3D11Device* device, ID3D11DeviceContext* deviceContext);
	~LineMesh();

	//void sendData(ID3D11DeviceContext*);
	void sendData(ID3D11DeviceContext* deviceContext, D3D_PRIMITIVE_TOPOLOGY top = D3D10_PRIMITIVE_TOPOLOGY_LINELIST) override;

protected:
	void initBuffers(ID3D11Device* device);

};

#endif