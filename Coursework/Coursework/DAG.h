#pragma once
#include <map>
#include <vector>
#include "LightningMesh.h"
#ifndef DAG_H
#define DAG_H
////////////////////////////////////////////////////////////////////
/// \brief Directed acyclic graph that renders the final lightning
///A Directed Acyclic Graph (DAG) is a directed graph that does not contain any cycles.
///In a DAG, each edge has a direction, meaning it goes from one vertex (node) to another.
///This direction signifies a one-way relationship or dependency between nodes.
///Acyclic indicates that there are no cycles or closed loops within the graph.
///So, perfect to render a lightning structure. 
////////////////////////////////////////////////////////////////////
class LinearSM; //Fwd declare
///
class DAG
{
public:
    //! constructor
    DAG(int xRes, int yRes);
    //! destructor
    virtual ~DAG();

    //! build the stepped ladder <- The start of the lightning
    void buildLeader(int bottomHit);

    //! add DAG segment <- The branches of the lightning
    bool addSegment(int index, int neighbor);

    //! render
    void draw(ID3D11Device* device, ID3D11DeviceContext* deviceContext, LinearSM* shader,
        XMMATRIX world, XMMATRIX view, XMMATRIX projection)
    {
	    drawNode(_root, device, deviceContext, shader, world, view, projection);
    };

    //! draw to an offscreen buffer
    float*& drawOffscreen(int scale = 1);

    ////! read in a new DAG
    //void read(const char* filename);
    ////! write out the current DAG
    //void write(const char* filename);

    //! quadtree x resolution accessor
    int xRes() { return _xRes; };
    //! quadtree y resolution accessor
    int yRes() { return _yRes; };

    //! input image x resolution accessor
    int& inputWidth() { return _inputWidth; };
    //! input image y resolution accessor
    int& inputHeight() { return _inputHeight; };

private:
    //Lightning mesh
    LightningMesh* lightning_mesh_;
    //! x resolution of quadtree
    int _xRes;
    //! y resolution of quadtree
    int _yRes;
    //! physical length of one grid cell
    float _dx;
    //! physical length of one grid cell
    float _dy;

    ////////////////////////////////////////////////////////////////////
    /// \brief node for line segment tree - 'branches' of the lightning
    ////////////////////////////////////////////////////////////////////
    struct NODE {
        int index;
        std::vector<NODE*> neighbors;
        NODE* parent;
        bool leader; 
        bool secondary;
        int depth;
        NODE* maxDepthNode;
        float intensity;

        NODE(int indexIn) { //Empty node constructor
            index = indexIn;
            parent = NULL;
            leader = false;
            depth = 0;
            maxDepthNode = NULL;
        };
    };
    //! recursive destructor
    void deleteNode(NODE* root);

    //! root of DAG
    NODE* _root;

    //! hash table of DAG nodes
    std::map<int, NODE*> _hash;

    //! build side branch
    void buildBranch(NODE* node, int depth);
    //! draw a node to OpenGL
    void drawNode(NODE* root, ID3D11Device* device, ID3D11DeviceContext* deviceContext, LinearSM* shader,
        XMMATRIX world, XMMATRIX view, XMMATRIX projection);
    //! find the deepest node in a given subtree
    void findDeepest(NODE* root, NODE*& deepest);

    ////! read in a DAG node 
    //void readNode(FILE* file);
    ////! write out a DAG node 
    //void writeNode(NODE* root, FILE* file);

    //! total number of nodes in scene
    int _totalNodes;

    //! node that finally hit bottom
    int _bottomHit;

    //! set the line segment intensities
    void buildIntensity(NODE* root);

    //! brightness of secondary branch
    float _secondaryIntensity;
    //! brightness of primary branch
    float _leaderIntensity;

    ////////////////////////////////////////////////////////////////
    // offscreen buffer variables
    ////////////////////////////////////////////////////////////////
    //! offscreen buffer
    float* _offscreenBuffer;

    //! width of offscreen buffer
    int _width;

    //! height of offscreen buffer
    int _height;

    //! scale of offscreen buffer compared to original image
    int _scale;

    //! draw a given node in the DAG
    void drawOffscreenNode(NODE* root);

    //! rasterize a single line to the offscreen buffer
    void drawLine(int begin[], int end[], float intensity);

    //! input image x resolution
    int _inputWidth;
    //! input image y resolution
    int _inputHeight;
};

#endif

