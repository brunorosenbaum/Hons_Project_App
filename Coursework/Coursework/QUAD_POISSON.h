#pragma once

//This class uses the solve() method in CG_SOLVER to
// return the number of iterations required to solve Poisson equation
//&
//It also draws, in the shape of quads, each cell of the stencil/each node of the quadtree
//&
//Subdivides the quadtree to a maxlevel for a 2d position based on the desired resolution
//&
//It subdivides the cell, its NSWE neighbors and its diagonal NSWE neighbors (8 points surrounding 1)
//&
//Balances the quadtree, building the neighbors.
//&
//Has getters for all leaves, the smallest leaves, and a single cell. 

//This is THE THIRD class in the order. 
//Order: CELL -> CG_SOLVER -> QUAD_POISSON -> QUAD_DBM_2D
#ifndef QUAD_POISSON_H
#define QUAD_POISSON_H

#include "CELL.h"
#include "CG_SOLVER.h"
#include "CellMesh.h"
#include <iostream>

class LinearSM; //Fwd declare

//////////////////////////////////////////////////////////////////////
/// \brief Quadtree Poisson solver and renderer
//////////////////////////////////////////////////////////////////////
class QUAD_POISSON
{
public:
    /// \brief quadtree constructor 
    ///
    /// \param xRes         maximum x resolution
    /// \param yRes         maximum y resolution
    /// \param iterations   maximum conjugate gradient iterations
    QUAD_POISSON(int xRes,
        int yRes,
        ID3D11Device* device, 
        ID3D11DeviceContext* deviceContext, int iterations = 10);

    //! destructor
    virtual ~QUAD_POISSON();

    /// \brief D3D11 drawing function
    /// 
    /// \param cell         internally used param, should always be NULL externally
    void draw(ID3D11Device* device, ID3D11DeviceContext* deviceContext, CELL* cell, 
        LinearSM* shader, XMMATRIX world, XMMATRIX view, XMMATRIX projection);
    /// \brief Draw a single cell  
    ///
    /// \param cell         quadtree cell to draw
    void drawCell(ID3D11Device* device, ID3D11DeviceContext* deviceContext,
        CELL* cell, LinearSM* shader,
        XMMATRIX world, XMMATRIX view, XMMATRIX projection);

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
    CellMesh* cellMesh;
    CellBoundsMesh* cellBoundsMesh; 
};

#endif

