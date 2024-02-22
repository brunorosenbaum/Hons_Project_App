#include "DAG.h"
#include "LinearSM.h"

#include <iostream>

#include "LightningSM.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

DAG::DAG(int xRes, int yRes, ID3D11Device* device, ID3D11DeviceContext* deviceContext) :
    _xRes(xRes),
    _yRes(yRes),
    _width(xRes),
    _height(yRes),
    _dx(1.0f / xRes),
    _dy(1.0f / yRes),
    _root(NULL),
    _totalNodes(0),
    _bottomHit(-1),
    _secondaryIntensity(0.3f),
    _leaderIntensity(0.75f)
{
    (_dx < _dy) ? _dy = _dx : _dy = _dx;
    _offscreenBuffer = NULL;
    //lightning_mesh_ = nullptr;
    lightning_mesh_ = new LightningMesh(device, deviceContext);
}

DAG::~DAG()
{
    if (_offscreenBuffer) delete[] _offscreenBuffer;

    deleteNode(_root);
    delete lightning_mesh_; lightning_mesh_ = 0; 
}

//////////////////////////////////////////////////////////////////////
// delete the nodes
//////////////////////////////////////////////////////////////////////
void DAG::deleteNode(NODE* root)
{
    if (root == NULL) return;

    for (int x = 0; x < root->neighbors.size(); x++)
        deleteNode(root->neighbors[x]);

    delete root;
    root = NULL;
}

//////////////////////////////////////////////////////////////////////
// add line segment 'index' to segment list
//////////////////////////////////////////////////////////////////////
bool DAG::addSegment(int index, int neighbor)
{
    if (_root)
    {
        // find corresponding root in DAG
        std::map<int, NODE*>::iterator iter = _hash.find(neighbor);
        NODE* root = (*iter).second;

        if (root == NULL) return false;

        // add to DAG
        NODE* newNode = new NODE(index);
        newNode->parent = root;
        root->neighbors.push_back(newNode);

        // add to hash table
        _hash.insert(std::map<int, NODE*>::value_type(index, newNode));
    }
    else
    {
        // make the root
        _root = new NODE(neighbor);
        _hash.insert(std::map<int, NODE*>::value_type(neighbor, _root));

        // then do the add
        NODE* newNode = new NODE(index);
        newNode->parent = _root;
        _root->neighbors.push_back(newNode);

        // add to hash table
        _hash.insert(std::map<int, NODE*>::value_type(index, newNode));
    }

    _totalNodes++;

    return true;
}

//////////////////////////////////////////////////////////////////////
// build the leader chain
//////////////////////////////////////////////////////////////////////
void DAG::buildLeader(int bottomHit)
{
    _bottomHit = bottomHit;

    // get pointer to bottommost node
    std::map<int, NODE*>::iterator iter = _hash.find(bottomHit);
    NODE* child = (*iter).second;

    // crawl up the tree
    while (child != NULL)
    {
        // tag segment
        child->leader = true;
        child->secondary = false;

        // look for side branches
        for (int x = 0; x < child->neighbors.size(); x++)
            if (!(child->neighbors[x]->leader))
                buildBranch(child->neighbors[x], 1);

        // advance child
        child = child->parent;
    }
    buildIntensity(_root);
}

//////////////////////////////////////////////////////////////////////
// build the side branch
//////////////////////////////////////////////////////////////////////
void DAG::buildBranch(NODE* node, int depth)
{
    node->depth = depth;
    node->leader = false;

    // look for side branches
    for (int x = 0; x < node->neighbors.size(); x++)
        if (!(node->neighbors[x]->leader))
            buildBranch(node->neighbors[x], depth + 1);
}

//////////////////////////////////////////////////////////////////////
// draw the DAG segments
//////////////////////////////////////////////////////////////////////
void DAG::drawNode(NODE* root, 
    ID3D11Device* device, ID3D11DeviceContext* deviceContext, 
    LightningSM* shader,
    XMMATRIX world, XMMATRIX view, XMMATRIX projection)
{
    if (root == NULL) return;

    // draw segments
    int beginIndex = root->index;
    int begin[2];
    int end[2];
    float dWidth = _dx;
    float dHeight = _dy;

    for (int x = 0; x < root->neighbors.size(); x++)
    {
        // get end node
        NODE* endNode = root->neighbors[x];
        int endIndex = endNode->index;

        // draw segments
        begin[1] = beginIndex / _xRes;
        begin[0] = beginIndex - begin[1] * _xRes;
        end[1] = endIndex / _xRes;
        end[0] = endIndex - end[1] * _xRes;

        //This is for the intensity of the lightning color, so the parent is more intense than the children
        //TODO: Make a shader and pass these rgba color values in a buffer so they are rendered in the pixel shader
        /*if (_bottomHit != -1)
            glColor4f(endNode->intensity,
                endNode->intensity,
                endNode->intensity, 1.0f);
        else
            glColor4f(0.0f, 1.0f, 0.0f, 1.0f);*/

        //This is not the whole mesh. Each lightning_mesh_ represents a LINE, a SEGMENT!!
        float xStart = begin[0] * dWidth + dWidth * 0.5f;
        float yStart = 1.0f - begin[1] * dHeight + dHeight * 0.5f;
        float xEnd = end[0] * dWidth + dWidth * 0.5f;
        float yEnd = 1.0f - end[1] * dHeight + dHeight * 0.5f; 

        XMMATRIX mScale = XMMatrixScaling(dWidth, dHeight, 1.0f);

        XMMATRIX startTransl = XMMatrixTranslation(xStart, yStart, -0.25f); 
       // XMMATRIX startScale = XMMatrixScaling(xStart, yStart, 0.1f); 
        XMMATRIX endTransl = XMMatrixTranslation(xEnd, yEnd, -0.25f);
       
        XMMATRIX sTemp = mScale /** XMMatrixRotationZ(dHeight)*/ * startTransl;
        /*XMMatrixMultiply*/
        XMMATRIX eTemp = mScale /** XMMatrixRotationZ(dHeight)*/ * endTransl;
        lightning_mesh_->sendData(deviceContext, D3D_PRIMITIVE_TOPOLOGY_LINELIST);
        shader->setShaderParameters(deviceContext, world, view, projection, sTemp, eTemp);
        shader->render(deviceContext, lightning_mesh_->getIndexCount()); 
        /*glBegin(GL_LINES);
        glVertex3f(begin[0] * dWidth + dWidth * 0.5f, 1.0f - begin[1] * dHeight + dHeight * 0.5f, 0.1f);
        glVertex3f(end[0] * dWidth + dWidth * 0.5f, 1.0f - end[1] * dHeight + dHeight * 0.5f, 0.1f);
        glEnd();*/

        // call recursively
        if (endNode->neighbors.size() > 0)
            drawNode(endNode, device, deviceContext, shader, world, view, projection);
    }
}

//////////////////////////////////////////////////////////////////////
// set the intensity of the node
//////////////////////////////////////////////////////////////////////
void DAG::buildIntensity(NODE* root)
{
    if (root == NULL) return;

    // draw segments
    int beginIndex = root->index;
    int begin[2];
    int end[2];
    float dWidth = _dx;
    float dHeight = _dy;

    for (int x = 0; x < root->neighbors.size(); x++)
    {
        // get end node
        NODE* endNode = root->neighbors[x];
        int endIndex = endNode->index;

        // draw segments
        begin[1] = beginIndex / _xRes;
        begin[0] = beginIndex - begin[1] * _xRes;
        end[1] = endIndex / _xRes;
        end[0] = endIndex - end[1] * _xRes;

        // set color
        if (endNode->leader)
            endNode->intensity = _leaderIntensity;
        else
        {
            // find max depth of current channel
            if (endNode->maxDepthNode == NULL)
                findDeepest(endNode, endNode->maxDepthNode);

            int maxDepth = endNode->maxDepthNode->depth;

            // calc standard deviation
            float stdDev = -(float)(maxDepth * maxDepth) / (float)(log(_secondaryIntensity) * 2.0f);

            // calc falloff
            float eTerm = -(float)(endNode->depth) * (float)(endNode->depth);
            eTerm /= (2.0f * stdDev);
            eTerm = exp(eTerm) * 0.5f;
            endNode->intensity = eTerm;
        }

        // call recursively
        if (endNode->neighbors.size() > 0)
            buildIntensity(endNode);
    }
}

//////////////////////////////////////////////////////////////////////
// draw the tree offscreen
//////////////////////////////////////////////////////////////////////
float*& DAG::drawOffscreen(int scale)
{
    // allocate buffer
    _width = _xRes * scale;
    _height = _yRes * scale;
    _scale = scale;
    if (_offscreenBuffer) delete[] _offscreenBuffer;
    _offscreenBuffer = new float[_width * _height];

    // wipe the buffer
    for (int x = 0; x < _width * _height; x++)
        _offscreenBuffer[x] = 0.0f;

    // recursively draw the tree
    drawOffscreenNode(_root);

    return _offscreenBuffer;
}

//////////////////////////////////////////////////////////////////////
// draw the DAG segments
//////////////////////////////////////////////////////////////////////
void DAG::drawOffscreenNode(NODE* root)
{
    if (root == NULL) return;

    // draw segments
    int beginIndex = root->index;
    int begin[2];
    int end[2];
    float dWidth = _dx;
    float dHeight = _dy;

    for (int x = 0; x < root->neighbors.size(); x++)
    {
        // get end node
        NODE* endNode = root->neighbors[x];
        int endIndex = endNode->index;

        // get endpoints 
        begin[0] = beginIndex % _xRes * _scale;
        begin[1] = beginIndex / _xRes * _scale;
        end[0] = endIndex % _xRes * _scale;
        end[1] = endIndex / _xRes * _scale;

        // make sure the one with the smaller x comes first
        if (end[0] < begin[0])
        {
            int temp[] = { end[0], end[1] };
            end[0] = begin[0];
            end[1] = begin[1];

            begin[0] = temp[0];
            begin[1] = temp[1];
        }

        // rasterize
        drawLine(begin, end, endNode->intensity);

        // call recursively
        if (endNode->neighbors.size() > 0)
            drawOffscreenNode(endNode);
    }
}

//////////////////////////////////////////////////////////////////////
// rasterize the line
// I assume all the lines are purely horizontal, vertical or diagonal
// to avoid using something like Bresenham
//////////////////////////////////////////////////////////////////////
void DAG::drawLine(int begin[], int end[], float intensity)
{
    int scaledX = _xRes * _scale;

    // if it is a horizontal line
    if (begin[1] == end[1])
    {
        for (int x = begin[0]; x < end[0]; x++)
        {
            int index = x + end[1] * scaledX;
            if (intensity > _offscreenBuffer[index])
                _offscreenBuffer[index] = intensity;
        }
        return;
    }

    // if it is a vertical line
    if (begin[0] == end[0])
    {
        int bottom = (begin[1] > end[1]) ? end[1] : begin[1];
        int top = (begin[1] > end[1]) ? begin[1] : end[1];

        for (int y = bottom; y < top; y++)
        {
            int index = begin[0] + y * scaledX;
            if (intensity > _offscreenBuffer[index])
                _offscreenBuffer[index] = intensity;
        }
        return;
    }

    // else it is diagonal
    int slope = (begin[1] < end[1]) ? 1 : -1;
    int interval = end[0] - begin[0];
    for (int x = 0; x <= interval; x++)
    {
        int index = begin[0] + x + (begin[1] + x * slope) * scaledX;
        if (intensity > _offscreenBuffer[index])
            _offscreenBuffer[index] = intensity;
    }
}

//////////////////////////////////////////////////////////////////////
// find deepest depth from current root
//////////////////////////////////////////////////////////////////////
void DAG::findDeepest(NODE* root, NODE*& deepest)
{
    deepest = root;

    for (int x = 0; x < root->neighbors.size(); x++)
    {
        NODE* child = root->neighbors[x];
        NODE* candidate = NULL;
        findDeepest(child, candidate);
        if (candidate->depth > deepest->depth)
            deepest = candidate;
    }
}

//////////////////////////////////////////////////////////////////////
// dump out line segments 
//////////////////////////////////////////////////////////////////////
void DAG::write(const char* filename)
{
    // open file
    FILE* file;
    file = fopen(filename, "wb");

    // write out total number of DAG nodes
    fwrite((void*)&_totalNodes, sizeof(int), 1, file);
    fwrite((void*)&_xRes, sizeof(int), 1, file);
    fwrite((void*)&_yRes, sizeof(int), 1, file);
    fwrite((void*)&_dx, sizeof(float), 1, file);
    fwrite((void*)&_dy, sizeof(float), 1, file);
    fwrite((void*)&_bottomHit, sizeof(int), 1, file);
    fwrite((void*)&_inputWidth, sizeof(int), 1, file);
    fwrite((void*)&_inputHeight, sizeof(int), 1, file);

    // write out nodes
    writeNode(_root, file);

    fclose(file);
}

//////////////////////////////////////////////////////////////////////
// read in line segments
//////////////////////////////////////////////////////////////////////
void DAG::read(const char* filename)
{

    // erase old DAG
    deleteNode(_root);

    // open file
    FILE* file;
    file = fopen(filename, "rb");
    if (file == NULL)
    {
	    std::cout << "ERROR: " << filename << " is invalid." << std::endl;
        exit(1);
    }

    // read in total number of DAG nodes
    fread((void*)&_totalNodes, sizeof(int), 1, file);
    fread((void*)&_xRes, sizeof(int), 1, file);
    fread((void*)&_yRes, sizeof(int), 1, file);
    fread((void*)&_dx, sizeof(float), 1, file);
    fread((void*)&_dy, sizeof(float), 1, file);
    fread((void*)&_bottomHit, sizeof(int), 1, file);
    fread((void*)&_inputWidth, sizeof(int), 1, file);
    fread((void*)&_inputHeight, sizeof(int), 1, file);

    // clear _hash for use
    _hash.clear();

    // read in all the DAG nodes
    for (int x = 0; x <= _totalNodes; x++)
        readNode(file);

    fclose(file);

    if (_bottomHit != -1)
        buildLeader(_bottomHit);
}

//////////////////////////////////////////////////////////////////////
// write out a DAG node
//////////////////////////////////////////////////////////////////////
void DAG::writeNode(NODE* root, FILE* file)
{
    int x;

    // write out neighbors
    int numNeighbors = root->neighbors.size();
    for (x = 0; x < numNeighbors; x++)
        writeNode(root->neighbors[x], file);

    // write out this node
    fwrite((void*)&(root->index), sizeof(int), 1, file);
    int parent = (root->parent == NULL) ? -1 : root->parent->index;
    fwrite((void*)&parent, sizeof(int), 1, file);
    fwrite((void*)&(root->leader), sizeof(bool), 1, file);
    fwrite((void*)&(root->secondary), sizeof(bool), 1, file);
    fwrite((void*)&(root->depth), sizeof(int), 1, file);
    fwrite((void*)&numNeighbors, sizeof(int), 1, file);
    for (x = 0; x < numNeighbors; x++)
        fwrite((void*)&(root->neighbors[x]->index), sizeof(int), 1, file);
}

//////////////////////////////////////////////////////////////////////
// read in a DAG node
//////////////////////////////////////////////////////////////////////
void DAG::readNode(FILE* file)
{
    // read in DAG data
    int index;
    int parent;
    bool leader;
    bool secondary;
    int depth;
    int numNeighbors;
    float potential;
    fread((void*)&index, sizeof(int), 1, file);
    fread((void*)&parent, sizeof(int), 1, file);
    fread((void*)&leader, sizeof(bool), 1, file);
    fread((void*)&secondary, sizeof(bool), 1, file);
    fread((void*)&depth, sizeof(int), 1, file);
    fread((void*)&numNeighbors, sizeof(int), 1, file);

    // create the node
    NODE* node = new NODE(index);
    node->leader = leader;
    node->secondary = secondary;
    node->depth = depth;

    // look up neighbors 
    std::map<int, NODE*>::iterator iter;
    for (int x = 0; x < numNeighbors; x++)
    {
        // read in the child index
        int neighborIndex;
        fread((void*)&neighborIndex, sizeof(int), 1, file);

        // look up in hash table
        iter = _hash.find(neighborIndex);

        // push onto the neighbor vector
        node->neighbors.push_back((*iter).second);

        // set child's parent
        (*iter).second->parent = node;
    }

    // add to hash table
    _hash.insert(std::map<int, NODE*>::value_type(index, node));

    // search for root node
    if (parent == -1)
    {
        node->parent = NULL;
        _root = node;
    }
}
