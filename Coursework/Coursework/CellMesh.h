//Mesh for drawing cells
//Will be used to assist the QUAD_POISSON class in translating OpenGL code to D3D11 
#ifndef _CellMesh_H_
#define _CellMesh_H_

#include "BaseMesh.h"
#include "CELL.h"

class CellMesh :
    public BaseMesh
{
public:
    CellMesh(ID3D11Device* device, ID3D11DeviceContext* deviceContext, CELL* cell);
    ~CellMesh();
    void initBuffers(ID3D11Device* device, CELL* cell);
};

class CellBoundsMesh : public BaseMesh
{
public:
    CellBoundsMesh(ID3D11Device* device, ID3D11DeviceContext* deviceContext, CELL* cell);
    ~CellBoundsMesh();
    void initBuffers(ID3D11Device* device, CELL* cell);
    void sendData(ID3D11DeviceContext* deviceContext, D3D_PRIMITIVE_TOPOLOGY top = D3D10_PRIMITIVE_TOPOLOGY_LINESTRIP) override;

};
#endif
