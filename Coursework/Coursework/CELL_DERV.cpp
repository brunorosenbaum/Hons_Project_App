#include "CELL_DERV.h"

const int CELL_DERV::NEIGHBORS_X_DIFFERENCE[8] = {0,  1, 1, 1, 0, -1, -1, -1};
const int CELL_DERV::NEIGHBORS_Y_DIFFERENCE[8] = { -1, -1, 0, 1, 1,  1,  0, -1 };


CELL_DERV::CELL_DERV(float north, //Same constructor to call base class
    float east, //TODO: ASK IF THIS IS CORRECT?
    float south,
    float west,
    CELL* parent,
    int depth ) : CELL(north, east, south, west, parent, depth),
	N_(0.0f), P_(0.0f), B_(0.0f)
{
	//Call base class constrc
    //This way it'll all get initialized to 0
    
}

CELL_DERV::~CELL_DERV()
{
}
