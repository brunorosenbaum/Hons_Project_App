#pragma once
#include "QUAD_POISSON.h"
#include "LineMesh.h"
#include "DAG.h"
#include "RNG.h"

#ifndef QUAD_DBM_2D_H 
#define QUAD_DBM_2D_H
////////////////////////////////////////////////////////////////////
/// \brief Quadtree DBM solver. This is the highest level class.
/// //This is THE LAST class in the order. 
//Order: CELL -> CG_SOLVER -> QUAD_POISSON -> QUAD_DBM_2D
////////////////////////////////////////////////////////////////////
class QUAD_DBM_2D
{
public:
    /// \brief DBM constructor 
    ///
    /// \param xRes         maximum x resolution
    /// \param yRes         maximum y resolution
    /// \param iterations   maximum conjugate gradient iterations
    QUAD_DBM_2D(ID3D11Device* device,
        ID3D11DeviceContext* deviceContext, int xRes = 128, int yRes = 128, int iterations = 10);

    //! destructor
    virtual ~QUAD_DBM_2D();

    //! add to aggregate - remember, aggregate is the whole lightning structure!
    bool addParticle();

    /// \brief Hit ground yet?
    /// \return returns true if a terminator as already been hit
    bool hitGround(CELL* cell = NULL);

    //! draw the quadtree cells to OpenGL - these are the CELLS and they will be in red!
    void drawQuadtreeCells(ID3D11Device* device,
        ID3D11DeviceContext* deviceContext, 
        LinearSM* shader, XMMATRIX world, XMMATRIX view, XMMATRIX projection);

    //! draw the DAG to OpenGL - This is the lightning! 
    void drawSegments(ID3D11Device* device, ID3D11DeviceContext* deviceContext, LightningSM* shader,
        XMMATRIX world, XMMATRIX view, XMMATRIX projection) {
        world *= XMMatrixTranslation(-0.5f, -0.5f, 0.0f);
        _dag->draw(device, deviceContext, shader, world, view, projection);
    };

    ////////////////////////////////////////////////////////////////
    // file IO
    ////////////////////////////////////////////////////////////////

    //! write everything to a file
    void writeFields(const char* filename);

    //! read everything from a file
    void readFields(const char* filename);

    /// \brief read in control parameters from an input file
    ///
    /// \param initial        initial pixels of lightning
    /// \param attractors     pixels that attract the lightning
    /// \param repulsors      pixels that repulse the lightning
    /// \param terminators    pixels that halt the simulation if hit
    /// \param xRes           x resolution of the image
    /// \param yRes           y resolution of the image
    ///
    /// \return Returns false if it finds something wrong with the images
    bool readImage(unsigned char* initial,
        unsigned char* attractors,
        unsigned char* repulsors,
        unsigned char* terminators,
        int xRes, int yRes);

    //! read in a new DAG
    void readDAG(const char* filename) { _dag->read(filename); };

    //! write out the current DAG
    void writeDAG(const char* filename) { _dag->write(filename); };

    /// \brief render to a software-only buffer
    ///
    /// \param scale      a (scale * xRes) x (scale * yRes) image is rendered
    float*& renderOffscreen(int scale = 1) { return _dag->drawOffscreen(scale); };

    //! access the DBM x resolution 
    int xRes() { return _xRes; };
    //! access the DBM y resolution
    int yRes() { return _yRes; };
    //! access the DAG x resolution
    int xDagRes() { return _dag->xRes(); };
    //! access the DAG y resolution
    int yDagRes() { return _dag->yRes(); };
    //! access the x resolution of the input image
    int inputWidth() { return _dag->inputWidth(); };
    //! access the y resolution of the input image
    int inputHeight() { return _dag->inputHeight(); };

private:
    void allocate(ID3D11Device* device, ID3D11DeviceContext* deviceContext);
    void deallocate();

    ////////////////////////////////////////////////////////////////////
    // dielectric breakdown model components
    ////////////////////////////////////////////////////////////////////

    // field dimensions
    int _xRes;
    int _yRes;
    int _maxRes;
    float _dx;
    float _dy;
    int _iterations;

    // which cell did it hit bottom with?
    int _bottomHit;

    DAG* _dag;

    QUAD_POISSON* _quadPoisson;

    // current candidate list
    std::vector<CELL*> _candidates;

    // check if any of the neighbors of cell should be added to the
    // candidate list
    void checkForCandidates(CELL* cell);

    // number of particles to add before doing another Poisson solve
    int _skips;

    // Mersenne Twister
    RNG _twister;
};

#endif


