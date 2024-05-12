//Adaptation to DX3D11 of Theodore Kim's
//<Fast Animation of Lightning Using An Adaptive Mesh> OpenGL source code

//This class solves Poisson's equation,
//which calculates the electric potential (phi) of a grid with boundary conditions.
//Each grid cell is represented by a CELL instance, which are the different nodes on a quadtree.
//Because the Poisson and Laplace equations are very similar, 
//the only implementation overhead required for the modified model is a minor change to the
//residual calculation in the conjugate gradient solver.

//This is THE SECOND class in the order. 
//Order: CELL -> CG_SOLVER -> QUAD_POISSON -> QUAD_DBM_2D
#pragma once
#ifndef CG_SOLVER_H
#define CG_SOLVER_H

#include <list>

#include "CELL.h"

////////////////////////////////////////////////////////////////////
/// \brief Conjugate gradient Poisson solver.
////////////////////////////////////////////////////////////////////
class CG_SOLVER
{
public:
	//! constructor
	CG_SOLVER(int maxDepth, int iterations = 10, int digits = 4); 
	//! destructor
	virtual ~CG_SOLVER();

	//! solve the Poisson problem
	virtual int solve(std::list<CELL*> cells);

	//! calculate the residual
	float calcResidual(std::list<CELL*> cells);

	//! accessor for the maximum number of iterations
	int& iterations() { return _iterations; }

protected:
	int _iterations;  ///< maximum number of iterations
	int _digits;      ///< desired digits of precision

	////////////////////////////////////////////////////////////////
	// conjugate gradient arrays (pointers to!!!)
	////////////////////////////////////////////////////////////////
	float* _direction;  ///< conjugate gradient 'd' array - growth direction
	float* _potential;  ///< conjugate gradient solution, 'x' array - THIS is phi
	float* _residual;   ///< conjugate gradient residual, 'r' array
	float* _q;          ///< conjugate gradient 'q' array

	int _arraySize;     ///< currently allocated array size
	int _listSize;      ///< current system size

	//! compute stencils once and store
	void calcStencils(std::list<CELL*> cells);

	//! reallocate the scratch arrays
	virtual void reallocate();

	//! physical lengths of various cell sizes
	float* _dx;
};

#endif

