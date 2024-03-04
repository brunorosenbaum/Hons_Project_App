#include "CELL_DERV.h"

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
