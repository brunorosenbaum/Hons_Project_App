#include "CellMesh.h"
///////////////////////////////////////// CELLS MESH //////////////////////////////////////////////////////

CellMesh::CellMesh(ID3D11Device* device, ID3D11DeviceContext* deviceContext/*, CELL* cell*/)
{
	initBuffers(device);

}

CellMesh::~CellMesh()
{
	// Run parent deconstructor
	BaseMesh::~BaseMesh();
}


void CellMesh::initBuffers(ID3D11Device* device/*, CELL* cell*/)
{
    VertexType* vertices;
    unsigned long* indices;
    D3D11_BUFFER_DESC vertexBufferDesc, indexBufferDesc;
    D3D11_SUBRESOURCE_DATA vertexData, indexData;

    vertexCount = 6;
    indexCount = 6; 

    vertices = new VertexType[vertexCount];
    indices = new unsigned long[indexCount];

    vertices[0].position = XMFLOAT3(0, 0, 0.0f);
    vertices[1].position = XMFLOAT3(0, 1, 0.0f);
    vertices[2].position = XMFLOAT3(1, 1, 0.0f);
    vertices[3].position = XMFLOAT3(1, 0, 0.0f);
    vertices[4].position = XMFLOAT3(0, 0, 0.0f);
    vertices[5].position = XMFLOAT3(1, 1, 0.0f);


    //vertices[4].position = XMFLOAT3(cell->bounds[0], 1.0f - cell->bounds[1], 0.0f);

    // Load the index array with data.
    for (int i = 0; i < vertexCount; i++)
    {
        indices[i] = i;
    }
    // Set up the description of the static vertex buffer.
    vertexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
    vertexBufferDesc.ByteWidth = sizeof(VertexType) * vertexCount;
    vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    vertexBufferDesc.CPUAccessFlags = 0;
    vertexBufferDesc.MiscFlags = 0;
    vertexBufferDesc.StructureByteStride = 0;
    // Give the subresource structure a pointer to the vertex data.
    vertexData.pSysMem = vertices;
    vertexData.SysMemPitch = 0;
    vertexData.SysMemSlicePitch = 0;
    // Now create the vertex buffer.
    device->CreateBuffer(&vertexBufferDesc, &vertexData, &vertexBuffer);

    // Set up the description of the static index buffer.
    indexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
    indexBufferDesc.ByteWidth = sizeof(unsigned long) * indexCount;
    indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
    indexBufferDesc.CPUAccessFlags = 0;
    indexBufferDesc.MiscFlags = 0;
    indexBufferDesc.StructureByteStride = 0;
    // Give the subresource structure a pointer to the index data.
    indexData.pSysMem = indices;
    indexData.SysMemPitch = 0;
    indexData.SysMemSlicePitch = 0;
    // Create the index buffer.
    device->CreateBuffer(&indexBufferDesc, &indexData, &indexBuffer);

    // Release the arrays now that the vertex and index buffers have been created and loaded.
    delete[] vertices;
    vertices = 0;

    delete[] indices;
    indices = 0;
}
void CellMesh::sendData(ID3D11DeviceContext* deviceContext, D3D_PRIMITIVE_TOPOLOGY top)
{
    unsigned int stride;
    unsigned int offset;

    // Set vertex buffer stride and offset.
    stride = sizeof(VertexType);
    offset = 0;

    deviceContext->IASetVertexBuffers(0, 1, &vertexBuffer, &stride, &offset);
    deviceContext->IASetIndexBuffer(indexBuffer, DXGI_FORMAT_R32_UINT, 0);
    deviceContext->IASetPrimitiveTopology(top);
}


///////////////////////////////////////// CELLS BOUND MESH //////////////////////////////////////////////////////

CellBoundsMesh::CellBoundsMesh(ID3D11Device* device, ID3D11DeviceContext* deviceContext/*, CELL* cell*/)
{
    initBuffers(device);

}

CellBoundsMesh::~CellBoundsMesh()
{
    BaseMesh::~BaseMesh();

}


void CellBoundsMesh::initBuffers(ID3D11Device* device /*CELL* cell*/)
{
    VertexType* vertices;
    unsigned long* indices;
    D3D11_BUFFER_DESC vertexBufferDesc, indexBufferDesc;
    D3D11_SUBRESOURCE_DATA vertexData, indexData;

    vertexCount = 5;
    indexCount = 5;

    vertices = new VertexType[vertexCount];
    indices = new unsigned long[indexCount];

    vertices[0].position = XMFLOAT3(0, 0, 0.0f); //W
    vertices[1].position = XMFLOAT3(0, 1, 0.0f);//N
    vertices[2].position = XMFLOAT3(1, 1, 0.0f);//E
    vertices[3].position = XMFLOAT3(1, 0, 0.0f);//S
    vertices[4].position = XMFLOAT3(0, 0, 0.0f);//W
   

    // Load the index array with data.
    for (int i = 0; i < vertexCount; i++)
    {
        indices[i] = i;
    }
    // Set up the description of the static vertex buffer.
    vertexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
    vertexBufferDesc.ByteWidth = sizeof(VertexType) * vertexCount;
    vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    vertexBufferDesc.CPUAccessFlags = 0;
    vertexBufferDesc.MiscFlags = 0;
    vertexBufferDesc.StructureByteStride = 0;
    // Give the subresource structure a pointer to the vertex data.
    vertexData.pSysMem = vertices;
    vertexData.SysMemPitch = 0;
    vertexData.SysMemSlicePitch = 0;
    // Now create the vertex buffer.
    device->CreateBuffer(&vertexBufferDesc, &vertexData, &vertexBuffer);

    // Set up the description of the static index buffer.
    indexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
    indexBufferDesc.ByteWidth = sizeof(unsigned long) * indexCount;
    indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
    indexBufferDesc.CPUAccessFlags = 0;
    indexBufferDesc.MiscFlags = 0;
    indexBufferDesc.StructureByteStride = 0;
    // Give the subresource structure a pointer to the index data.
    indexData.pSysMem = indices;
    indexData.SysMemPitch = 0;
    indexData.SysMemSlicePitch = 0;
    // Create the index buffer.
    device->CreateBuffer(&indexBufferDesc, &indexData, &indexBuffer);

    // Release the arrays now that the vertex and index buffers have been created and loaded.
    delete[] vertices;
    vertices = 0;

    delete[] indices;
    indices = 0;
}

void CellBoundsMesh::sendData(ID3D11DeviceContext* deviceContext, D3D_PRIMITIVE_TOPOLOGY top)
{
    unsigned int stride;
    unsigned int offset;

    // Set vertex buffer stride and offset.
    stride = sizeof(VertexType);
    offset = 0;

    deviceContext->IASetVertexBuffers(0, 1, &vertexBuffer, &stride, &offset);
    deviceContext->IASetIndexBuffer(indexBuffer, DXGI_FORMAT_R32_UINT, 0);
    deviceContext->IASetPrimitiveTopology(top);
}


