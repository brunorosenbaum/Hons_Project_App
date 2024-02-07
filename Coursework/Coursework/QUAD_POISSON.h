#pragma once
#ifndef QUAD_POISSON_H
#define QUAD_POISSON_H


#include "CELL.h"
#include "CG_SOLVER.h"
#include "CellMesh.h"
#include "LinearSM.h"

#include <iostream>


//////////////////////////////////////////////////////////////////////
/// \brief Quadtree Poisson solver and renderer
//////////////////////////////////////////////////////////////////////
class QUAD_POISSON : public BaseMesh
{
public:
    /// \brief quadtree constructor 
    ///
    /// \param xRes         maximum x resolution
    /// \param yRes         maximum y resolution
    /// \param iterations   maximum conjugate gradient iterations
    QUAD_POISSON(int xRes,
        int yRes,
        int iterations = 10, 
        ID3D11Device* device, 
        ID3D11DeviceContext* deviceContext);

    //! destructor
    virtual ~QUAD_POISSON();

    /// \brief D3D11 drawing function
    /// 
    /// \param cell         internally used param, should always be NULL externally
    void draw(ID3D11Device* device, ID3D11DeviceContext* deviceContext, CELL* cell = NULL);
    /// \brief Draw a single cell  
    ///
    /// \param cell         quadtree cell to draw
    /// \param r            red intensity to draw
    /// \param g            green intensity to draw
    /// \param b            blue intensity to draw
    void drawCell(CELL* cell,
        float r = 1.0f,
        float g = 0.0f,
        float b = 0.0f);

    //! Solve the Poisson problem
    int solve();

    /// \brief insert point at maximum subdivision level
    ///
    /// \param xPos         x position to insert at
    /// \param yPos         y position to insert at
    CELL* insert(float xPos, float yPos);

    /// \brief insert point at maximum subdivision level
    /// 
    /// \param xPos         grid cell x index to insert
    /// \param yPos         grid cell y index to insert
    CELL* insert(int xPos, int yPos) {
        return insert((float)xPos / _maxRes, (float)yPos / _maxRes);
    };

    /// \brief get all the leaf nodes
    /// \return Leaves are returned in the 'leaves' param
    void getAllLeaves(std::list<CELL*>& leaves, CELL* currentCell = NULL);

    /// \brief  get all the leaf nodes at finest subdivision level
    /// \return Leaves are  returned in the 'leaves' param
    std::list<CELL*>& getSmallestLeaves() { return _smallestLeaves; };

    //! maximum resolution accessor
    int& maxRes() { return _maxRes; }

    //! maximum depth accessor
    int& maxDepth() { return _maxDepth; };

    //! get leaf at coordinate (x,y)
    CELL* getLeaf(float xPos, float yPos);

private:
    //! root of the quadtree
    CELL* _root;

    //! maximum resolution of quadtree
    int _maxRes;

    //! maxmimum depth of quadtree
    int _maxDepth;

    //! dependant leaves
    std::list<CELL*> _emptyLeaves;

    //! smallest leaves
    std::list<CELL*> _smallestLeaves;

    //! current Poisson solver
    CG_SOLVER* _solver;

    //! balance quadtree
    void balance();

    //! get the leaf nodes not on the boundary
    void getEmptyLeaves(std::list<CELL*>& leaves, CELL* currentCell = NULL);

    //! build the neighbor lists of the cells
    void buildNeighbors();

    //! delete ghost cells
    void deleteGhosts(CELL* currentCell = NULL);

    //! Blue noise function
    //BLUE_NOISE* _noiseFunc;

    //! Blue noise sample locations
   // bool* _noise;

    //! check if a cell hits a noise node
    //void setNoise(CELL* cell);

    //D3D11 SHADER CONVERSION
    LinearSM* linearSM;
    CellMesh* cellMesh;
    CellBoundsMesh* cellBoundsMesh; 
};

#endif
