#include "CG_SOLVER.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CG_SOLVER::CG_SOLVER(int maxDepth, int iterations, int digits) :
    _iterations(iterations),
    _arraySize(0), _listSize(0), _digits(digits),
    _direction(NULL), _residual(NULL), _q(NULL), _potential(NULL)
{
    // compute the physical size of various grid cells
    _dx = new float[maxDepth + 1]; //Creates ptr to array of floats the size of the max depth + 1
    _dx[0] = 1.0f; //Physical length of the 1st cell will be 1.0f
    for (int x = 1; x <= maxDepth; x++) //And then, each subsequent cell, 
        _dx[x] = _dx[x - 1] * 0.5f; //its size will be half of the previous one
}

CG_SOLVER::~CG_SOLVER() //Getting rid of the allocated memory to the CG arrays
{
    if (_direction) delete[] _direction;
    if (_residual) delete[] _residual;
    if (_potential) delete[] _potential;
    if (_q) delete[] _q;
}

//////////////////////////////////////////////////////////////////////
// reallocate scratch arrays if necessary
//////////////////////////////////////////////////////////////////////
void CG_SOLVER::reallocate()
{
    // if we have enough size already, return
    if (_arraySize >= _listSize) return;

    // made sure it SSE aligns okay (SSE is the intel instruction set)
    _arraySize = _listSize * 2;
    if (_arraySize % 4)
        _arraySize += 4 - _arraySize % 4;

    // delete the old ones
    if (_direction) delete[] _direction;
    if (_residual) delete[] _residual;
    if (_q) delete[] _q;

    // allocate the new ones
    _direction = new float[_arraySize];
    _residual = new float[_arraySize];
    _q = new float[_arraySize];

    // wipe the new ones - set each cell's properties to 0
    for (int x = 0; x < _arraySize; x++)
        _direction[x] = _residual[x] = _q[x] = 0.0f;
}

//////////////////////////////////////////////////////////////////////
// conjugate gradient solver 
//////////////////////////////////////////////////////////////////////
int CG_SOLVER::solve(std::list<CELL*> cells)
{
    //This method takes in a list of CELL ptrs, and then creates an iterator to go through them.

    // counters
    int x, y, index;

    //Cell* type list iterator for our list. This goes through the grid, one cell at a time. 
    std::list<CELL*>::iterator cellIterator; 

    //Current number of iterations. Set to 0 at first.
    int i = 0; 

    // Precalculate stencils - bc Laplace's eq can be solved by
    //  limiting the possible values of variables in the model (this is called CONSTRAINT)
    //  of the grid cells according to the Laplacian stencil
    //  ... which is represented with a quadtree.
    //  Since it's a 5 point Laplacian stencil,
    //  it's a cell in the grid together with its NESW neighbors
    calcStencils(cells);

    // reallocate scratch arrays if necessary
    _listSize = cells.size();
    reallocate();

    // compute a new lexicographical order - orders the cells from the list to the iterator
    cellIterator = cells.begin();
    for (x = 0; x < _listSize; x++, cellIterator++)
        (*cellIterator)->index = x;

    // r = b - Ax
    calcResidual(cells); //Method to calculate the residual of the Poisson eq

    // copy residual into easy array
    // d = r
    cellIterator = cells.begin();
    float deltaNew = 0.0f; 
    for (x = 0; x < _listSize; x++, cellIterator++)
    {
        _direction[x] = _residual[x]; //Growth direction in each grid cell is determined by the residual value
        deltaNew += _residual[x] * _residual[x]; //Delta = residual^2
    }

    // delta0 = deltaNew
    float delta0 = deltaNew;

    //Eps = 10^(-digits of precision) = 10^(-8) = 0.00000001
    float eps = pow(10.0f, (float)-_digits);
    //Max R = 0.00000002
    float maxR = 2.0f * eps;
    
    while ((i < _iterations) && (maxR > eps)) //While we haven't reached the max number of iterations 
    {
        // q = Ad <-- Conjugate gradient equation.
        //CG is effective for systems of the form Ax = b
        //Where x is an unknown vector,
        // b is a known vector
        // and A is a known, square, symmetric, positive-definite matrix.

        //Therefore, q = vector where the lightning will grow (unknown)
        //A is be the transformation matrix for rendering each segment's position
        //And d has to be calculated depending on the weights of the neighbors and their elec potential (phi) values

		//First, Go through the cells
        cellIterator = cells.begin();
        for (y = 0; y < _listSize; y++, cellIterator++) 
        {
            CELL* currentCell = *cellIterator;
            CELL** neighbors = currentCell->neighbors; 

            float neighborSum = 0.0f; //Reset sum of neighbors each loop

            for (int x = 0; x < 4; x++) //Go through NEIGHBORS
            { //Use the precalculated laplace stencil coefficient to find the sum of the local neighbors.

                int j = x * 2;
                //Add its growth direction * stencil weight
                //((Remember growth direction is determined by the residual value - which is determined by b ))
                neighborSum += _direction[neighbors[j]->index] * currentCell->stencil[j]; //For the even neighbors
                if (neighbors[j + 1]) //Uneven neighbors
                    neighborSum += _direction[neighbors[j + 1]->index] * currentCell->stencil[j + 1];
            }
            //q is the steepest descent value of the conjugate gradient.
            //q = direction of current cell + its stencil weight - the sum of its neighbors
            //So, the vector where lightning will grow
            _q[y] = -neighborSum + _direction[y] * currentCell->stencil[8];
        }
        // alpha = deltaNew / (transpose(d) * q)
        float alpha = 0.0f; 
        for (x = 0; x < _listSize; x++)
            alpha += _direction[x] * _q[x];
        if (fabs(alpha) > 0.0f)
            alpha = deltaNew / alpha;

        // x = x + alpha * d - Update the potential of each cell, adding alpha * direction
        cellIterator = cells.begin();
        for (x = 0; x < _listSize; x++, cellIterator++)
            (*cellIterator)->potential += alpha * _direction[x]; 

        // r = r - alpha * q - Update residual value
        maxR = 0.0f;
        for (x = 0; x < _listSize; x++)
        {
            _residual[x] -= _q[x] * alpha; //Subtract cg solution * alpha from the residual
            maxR = (_residual[x] > maxR) ? _residual[x] : maxR; //And update the max residual depending on the previous calc
        }

        // deltaOld = deltaNew
        float deltaOld = deltaNew;

        // deltaNew = transpose(r) * r - We're making a new delta with the updated residual
        deltaNew = 0.0f;
        for (x = 0; x < _listSize; x++)
            deltaNew += _residual[x] * _residual[x];

        // beta = deltaNew / deltaOld - And now beta is the quotient of starting point delta + delta after solving cg
        //This is equation 12, which chooses a growth site. Beta would be p(i) in the paper
        float beta = deltaNew / deltaOld;

        // d = r + beta * d - Direction is updated by adding the residual to beta * previous direction
        for (x = 0; x < _listSize; x++)
            _direction[x] = _residual[x] + beta * _direction[x];

        // i = i + 1
        i++;
    }
    // return the number of iterations required to solve the poisson eq
    return i;
}

//////////////////////////////////////////////////////////////////////
// calculate the residuals
//////////////////////////////////////////////////////////////////////
float CG_SOLVER::calcResidual(std::list<CELL*> cells)
{
    //Because the Poisson and Laplace equations are very similar, 
    //the only implementation overhead required for the modified model
    //is a minor change to the residual calculation in the conjugate gradient solver

    float maxResidual = 0.0f;

    std::list<CELL*>::iterator cellIterator = cells.begin(); //Go through the cells
    for (int i = 0; i < _listSize; i++, cellIterator++)
    {
        CELL* currentCell = *cellIterator;
        float dx = _dx[currentCell->depth]; //Size (x) of the cell, depending on the tree's depth
        float neighborSum = 0.0f; //Set its neighbor sum to 0

        for (int x = 0; x < 4; x++) //Go through cell's neighbors
        {
            int i = x * 2; //i represents even neighbors, i+1 uneven
            //Add the cell's neighbor's elec potential * its stencil weight into the neighbor Sum. 
            neighborSum += currentCell->neighbors[i]->potential * currentCell->stencil[i]; //For even numbers
            if (currentCell->neighbors[i + 1]) //For uneven
                neighborSum += currentCell->neighbors[i + 1]->potential * currentCell->stencil[i + 1];
        }
        //The residual value for this cell will be Laplace's eq RHS - (negative sum of its neighbors + current cell's phi * stencil weight
        _residual[i] = currentCell->b - (-neighborSum + currentCell->potential * currentCell->stencil[8]);

        if (fabs(_residual[i]) > maxResidual) //Always set max residual to the largest value obtained
            maxResidual = fabs(_residual[i]); 
    }
    return maxResidual; //Then return it
}

//////////////////////////////////////////////////////////////////////
// compute stencils once and store
//In 2D, the Laplace equation can be solved by constraining the values of the grid
// cells according to the 5 point Laplacian stencil(Figure 1(b)).
//These constraints produce a linear system that can then be
// solved with an efficient solver such as conjugate gradient.
//////////////////////////////////////////////////////////////////////
void CG_SOLVER::calcStencils(std::list<CELL*> cells)
{
    //Takes in a list of the cells in the grid

	std::list<CELL*>::iterator cellIterator = cells.begin(); //Iterator 

    for (cellIterator = cells.begin(); cellIterator != cells.end(); cellIterator++) //Iterate through the list
    {
        CELL* currentCell = *cellIterator; //Current cell 
        float invDx = 1.0f / _dx[currentCell->depth]; //invDx = 1/(x length of the cell in that depth. the deeper, the smaller)
        //Therefore, as we go deeper in the tree --> invDx's value gets larger.
        //In the paper Dx == DELTAx

        // sum over faces TODO: HUH? Look up symmetric discretization on the paper
        float deltaSum = 0.0f; //Sum of all stencils
        float bSum = 0.0f; //sum of rhs term

        for (int x = 0; x < 4; x++) //Stencil weight is calculated 4 times - once per stencil member
        {
            int i = x * 2; //i = The index of each stencil. Values for this each iteration are 0, 2, 4, 6.
            currentCell->stencil[i] = 0.0f; //This is because neighbors 0, 2, 4 & 6 should always exist,
            currentCell->stencil[i + 1] = 0.0f; //and neighbors 1, 3, 5 & 7 may not exist if it's not very refined.
            //Either way both are initialized to 0. 

            if (currentCell->neighbors[i + 1] == NULL) { //If neighbors 1, 3, 5 & 7 do not contain data
                //We're only considering the data of 0, 2, 4 & 6 and adding it to the total stencil weight and boundary sums.

                // If current cell is at the same refinement level of its neighbors (case 1) ((Same size))
                if (currentCell->depth == currentCell->neighbors[i]->depth) {
                    deltaSum += invDx; //Add invDx to the sum of stencil weight
                    if (!currentCell->neighbors[i]->boundary)//If the neighbor's boundary is not gonna be included in solver
                        currentCell->stencil[i] = invDx; //Set stencil weight to invDx
                    else //If it's gonna be included
                        bSum += (currentCell->neighbors[i]->potential) * invDx; //Multiply neighbor's phi * 1/dx and add to the boundary sum
                }
                // else it is less refined (case 3) ((Current cell bigger than the neighbors))
                else {
                    deltaSum += 0.5f * invDx; //Add half of invDx to the sum of stencil weight (half the square) 
                    if (!currentCell->neighbors[i]->boundary)
                        currentCell->stencil[i] = 0.5f * invDx; //Set stencil weight to half of invDx
                    else
                        bSum += currentCell->neighbors[i]->potential * 0.5f * invDx; //phi * 0.5 * 1/dx
                }
            }
            // if the neighbor is at a lower level (case 2) - ((neighbors 1, 3, 5 & 7 contain data, and are bigger than current cell))
            //We're also considering the data of 1, 3, 5 & 7 and adding it to the total stencil weight and boundary sums.
            else {
                deltaSum += 2.0f * invDx; //Add double invDx to the sum of stencil weight
                if (!currentCell->neighbors[i]->boundary) 
                    currentCell->stencil[i] = invDx;
                else
                    bSum += currentCell->neighbors[i]->potential * invDx;
                if (!currentCell->neighbors[i + 1]->boundary)
                    currentCell->stencil[i + 1] = invDx;
                else
                    bSum += currentCell->neighbors[i + 1]->potential * invDx;
            }
        }

        currentCell->stencil[8] = deltaSum; //Central stencil weight = sum of the neighbors depending on size
        //LAPLACE'S EQUATION: Laplacian^2 * phi = 0
        currentCell->b = bSum; //Here, we set that 0 to the rhs sum. 
    }
}
