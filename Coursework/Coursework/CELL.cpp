#include "CELL.h"
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

// normal cell constructor
CELL::CELL(float north, float east, float south, float west, CELL* parent, int depth) :
    parent(parent), depth(depth), index(-1), candidate(false),
    boundary(false), potential(0.0f), state(EMPTY)
{
    for (int x = 0; x < 4; x++) //Set children of a cell to 0
        children[x] = nullptr;
    for (int x = 0; x < 8; x++) //Same with its neighbors
        neighbors[x] = nullptr;

    //Where the bounds of the cell are in space (xy coords)
    bounds[0] = north; bounds[1] = east; bounds[2] = south; bounds[3] = west;

	//To calculate the center of the cell (xy coords)
    center[0] = (bounds[1] + bounds[3]) * 0.5f;
    center[1] = (bounds[0] + bounds[2]) * 0.5f;
}

// ghost cell constructor (ghost cells dont have charge)
CELL::CELL(int depth) : parent(NULL), depth(depth), index(-1), candidate(false),
boundary(true), potential(0.0f), state(EMPTY)
{
    for (int x = 0; x < 4; x++)
        children[x] = NULL;
    for (int x = 0; x < 8; x++)
        neighbors[x] = NULL;

    bounds[0] = 0.0f; bounds[1] = 0.0f; bounds[2] = 0.0f; bounds[3] = 0.0f;
    center[0] = 0.0f; center[1] = 0.0f;
}

CELL::~CELL() {
    int x;

    for (x = 0; x < 4; x++)
        if (children[x] != NULL)
        {
            delete children[x];
            children[x] = NULL;
        }
}
//////////////////////////////////////////////////////////////////////
// refine current cell. This means subdividing the quadtree
//////////////////////////////////////////////////////////////////////
void CELL::refine() {
    if (children[0] != NULL) return; //Return if current cell has children
    //Else:
    float center[] = { (bounds[0] + bounds[2]) * 0.5f, (bounds[1] + bounds[3]) * 0.5f }; //Calc center coords of cell

    //Divide into four new cells, set their parent to this one AND add depth in the tree
    children[0] = new CELL(bounds[0], center[1], center[0], bounds[3], this, depth + 1);
    children[1] = new CELL(bounds[0], bounds[1], center[0], center[1], this, depth + 1);
    children[2] = new CELL(center[0], bounds[1], bounds[2], center[1], this, depth + 1);
    children[3] = new CELL(center[0], center[1], bounds[2], bounds[3], this, depth + 1);
    //Their potentials will be set to 0 using this constructor so im not understanding smth here but oki
    children[0]->potential = potential;
    children[1]->potential = potential;
    children[2]->potential = potential;
    children[3]->potential = potential;
}

//////////////////////////////////////////////////////////////////////
// return north neighbor to current cell
//////////////////////////////////////////////////////////////////////
CELL* CELL::northNeighbor()
{
    // if it is the root
    if (this->parent == NULL) return NULL;

    // if it is the southern child of the parent
    if (parent->children[3] == this) return parent->children[0];
    if (parent->children[2] == this) return parent->children[1];

    // else look up higher
    CELL* mu = parent->northNeighbor();

    // if there are no more children to look at,
    // this is the answer
    if (mu == NULL || mu->children[0] == NULL) return mu;
    // if it is the NW child of the parent
    else if (parent->children[0] == this) return mu->children[3];
    // if it is the NE child of the parent
    else return mu->children[2];
}

//////////////////////////////////////////////////////////////////////
// return south neighbor to current cell
//////////////////////////////////////////////////////////////////////
CELL* CELL::southNeighbor()
{
    // if it is the root
    if (this->parent == NULL) return NULL;

    // if it is the northern child of the parent
    if (parent->children[0] == this) return parent->children[3];
    if (parent->children[1] == this) return parent->children[2];

    // else look up higher
    CELL* mu = parent->southNeighbor();

    // if there are no more children to look at,
    // this is the answer
    if (mu == NULL || mu->children[0] == NULL) return mu;
    // if it is the SW child of the parent
    else if (parent->children[3] == this) return mu->children[0];
    // if it is the SE child of the parent
    else return mu->children[1];
}

//////////////////////////////////////////////////////////////////////
// return west neighbor to current cell
//////////////////////////////////////////////////////////////////////
CELL* CELL::westNeighbor()
{
    // if it is the root
    if (this->parent == NULL) return NULL;

    // if it is the eastern child of the parent
    if (parent->children[1] == this) return parent->children[0];
    if (parent->children[2] == this) return parent->children[3];

    // else look up higher
    CELL* mu = parent->westNeighbor();

    // if there are no more children to look at,
    // this is the answer
    if (mu == NULL || mu->children[0] == NULL) return mu;
    // if it is the NW child of the parent
    else if (parent->children[0] == this) return mu->children[1];
    // if it is the SW child of the parent
    else return mu->children[2];
}

//////////////////////////////////////////////////////////////////////
// return east neighbor to current cell
//////////////////////////////////////////////////////////////////////
CELL* CELL::eastNeighbor()
{
    // if it is the root
    if (this->parent == NULL) return NULL;

    // if it is the western child of the parent
    if (parent->children[0] == this) return parent->children[1];
    if (parent->children[3] == this) return parent->children[2];

    // else look up higher
    CELL* mu = parent->eastNeighbor();

    // if there are no more children to look at,
    // this is the answer
    if (mu == NULL || mu->children[0] == NULL) return mu;
    // if it is the NE child of the parent
    else if (parent->children[1] == this) return mu->children[0];
    // if it is the SE child of the parent
    else return mu->children[3];
}
