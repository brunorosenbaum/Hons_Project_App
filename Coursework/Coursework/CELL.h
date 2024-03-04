//Adaptation to DX3D11 of Theodore Kim's
//<Fast Animation of Lightning Using An Adaptive Mesh> OpenGL source code

#ifndef CELL_H
#define CELL_H
//'CELL' is a class representing each cell in a Laplacian growth grid
//in the DBM simulation.
//Each cell instance holds information regarding each cell.
//Since this algorithm uses quadtrees to solve Laplace's equation,
//Coincidentally each cell is also a data structure for the quadtree.

//This is THE FIRST class in the order. 
//Order: CELL -> CG_SOLVER -> QUAD_POISSON -> QUAD_DBM_2D
#pragma once
#include <vcruntime.h>
//////////////////////////////////////////////////////////////////////
/// \enum Possible states of the cell in the DBM simulation
//////////////////////////////////////////////////////////////////////
enum CELL_STATE { EMPTY, NEGATIVE, POSITIVE, REPULSOR, ATTRACTOR };

//////////////////////////////////////////////////////////////////////
/// \brief Basic cell data structure of the quadtree
//////////////////////////////////////////////////////////////////////
class CELL
{
public:
    //! normal cell constructor
    CELL(float north, //The floats represent the phi (electric potential)
        float east,// values of its neighbours NESW
        float south,
        float west,
        CELL* parent = nullptr,
        int depth = 0);

    //! ghost cell constructor  
    CELL(int depth = 0);

    //! destructor
    ~CELL(); 

    CELL* children[4]; //! The children of the node in the quadtree - akin to Laplacian stencil
  /*!
      Winding order of children is: CLOCKWISE STARTING TOP LEFT

      \verbatim
        _________
        |   |   |
        | 0 | 1 |
        |___|___|
        |   |   |
        | 3 | 2 |
        |___|___|
      \endverbatim */
    
    float bounds[4];//! The physical bounds of the current grid cell
     /*!
         Winding order of bounds is:

         \verbatim
           0 - north
           1 - east
           2 - south
           3 - west
         \endverbatim */

    CELL* neighbors[8];//! The neighbors in the balanced quadtree
       /*!
         winding order of the neighbors is:

         \verbatim
              | 0  | 1  |
          ____|____|____|_____
              |         |
            7 |         |  2
          ____|         |_____
              |         |
            6 |         |  3
          ____|_________|_____
              |    |    |
              | 5  |  4 |
         \endverbatim

         Neighbors 0,2,4,6 should always exist. Depending on
         if the neighbor is on a lower refinement level,
         neighbors 1,3,5,7 may or may not exist. If they are not
         present, the pointer value should ne NULL.  */

    float stencil[9];//! Poisson/Laplace stencil coefficients
       /*!
         winding order of the stencil coefficients:

         \verbatim
              | 0  | 1  |
          ____|____|____|_____
              |         |
           7  |         |  2
          ____|    8    |_____
              |         |
           6  |         |  3
          ____|_________|_____
              |    |    |
              | 5  | 4  |
         \endverbatim
         Stencils 0,2,4,6 should always exist. Depending on
         if the neighbor is on a lower refinement level,
         stencils 1,3,5,7 may or may not exist. If they are not
         present, the pointer value should be NULL.    */

    float center[2];    ///< center of the cell (xy coordinates)
    int depth;          ///< current tree depth
    bool candidate;     ///< already a member of candidate list? (for growth direction)

    CELL* parent;       ///< parent node in the quadtree
    CELL_STATE state;   ///< DBM state of the cell

    void refine();      ///< subdivide the cell (quadtree subdivisions)


    ////////////////////////////////////////////////////////////////
  // solver-related variables
  ////////////////////////////////////////////////////////////////
    bool boundary;      ///< True if boundary node is to be included in the solver
    float potential;    ///< current electric potential (phi)
    float b;            ///< right hand side of the linear system (laplaces eq)
    float residual;     ///< residual in the linear solver
    int index;          ///< lexicographic index for the solver (alphabetic order? ask)
    ////////////////////////////////////////////////////////////////
  // neighbor lookups
  ////////////////////////////////////////////////////////////////
    CELL* northNeighbor();  ///< lookup northern neighbor
    CELL* southNeighbor();  ///< lookup southern neighbor
    CELL* westNeighbor();   ///< lookup western neighbor
    CELL* eastNeighbor();   ///< lookup eastern neighbor
};
#endif

