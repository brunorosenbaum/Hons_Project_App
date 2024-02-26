#include "LightningMesh.h"

LightningMesh::LightningMesh(ID3D11Device* device, ID3D11DeviceContext* deviceContext)
: LineMesh(device, deviceContext)
{
	initBuffers(device);
}

LightningMesh::LightningMesh(ID3D11Device* device, ID3D11DeviceContext* deviceContext, 
	float xPosStart, float yPosStart, float xPosEnd, float yPosEnd)
	: LineMesh(device, deviceContext)
{
	initBuffers(device);
	xPosStart_ = xPosStart;
	yPosStart_ = yPosStart;
	xPosEnd_ = xPosEnd;
	yPosEnd_ = yPosEnd; 
}

LightningMesh::~LightningMesh()
{
	LineMesh::~LineMesh(); 
}

void LightningMesh::initBuffers(ID3D11Device* device)
{
	VertexType* vertices;
	unsigned long* indices;
	D3D11_BUFFER_DESC vertexBufferDesc, indexBufferDesc;
	D3D11_SUBRESOURCE_DATA vertexData, indexData;

	vertexCount = 2;
	indexCount = 2;


	vertices = new VertexType[vertexCount];
	indices = new unsigned long[indexCount];

	//// Load the vertex array with data.
	//vertices[0].position = XMFLOAT3(xPosStart_, yPosStart_, 0.0f);  // Segment start
	//vertices[1].position = XMFLOAT3(xPosEnd_, yPosEnd_, 0.0f);  // Segment end

	// Load the vertex array with data.
	vertices[0].position = XMFLOAT3(0, 0, 0.0f);  // Segment start
	vertices[1].position = XMFLOAT3(0, 1, 0.0f);  // Segment end

	// Load the index array with data.
	indices[0] = 0;  // Start
	indices[1] = 1;  // End

	//https://learn.microsoft.com/en-us/windows/win32/direct3d11/how-to--use-dynamic-resources
	// Step 1 goes here
	// Set up the description of the static vertex buffer.
	//If you want to make dynamic changes to resources, the buffer description's usage has to be set to dynamic
	vertexBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	vertexBufferDesc.ByteWidth = sizeof(VertexType) * vertexCount;
	vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vertexBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE; //Set it as writeable instead of 0!!
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

void LightningMesh::sendData(ID3D11DeviceContext* deviceContext, D3D_PRIMITIVE_TOPOLOGY top)
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



