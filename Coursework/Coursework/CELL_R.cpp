#include "CELL_R.h"

const int CELL_R::NEIGHBORS_X_DIFFERENCE[8] = {0,  1, 1, 1, 0, -1, -1, -1};
const int CELL_R::NEIGHBORS_Y_DIFFERENCE[8] = { -1, -1, 0, 1, 1,  1,  0, -1 };


CELL_R::CELL_R() :
	x(-1), y(-1), parentX(-1), parentY(-1),
	potential(0), selectionProb(0), isBoundary(false), type_(EMPTY_R),
	 N_(0), P_(0), B_(0)
{
	
}

CELL_R::CELL_R(int x, int y, CELL_TYPE_R type, float phi) :
	x(x), y(y), parentX(-1), parentY(-1),
	potential(phi), selectionProb(0), isBoundary(false), type_(EMPTY_R),
	N_(0), P_(0), B_(0)
{
	if (EMPTY_R == type)	isBoundary = false;
	else						isBoundary = true;
}

CELL_R::~CELL_R()
{
}

void CELL_R::SetCellType(CELL_TYPE_R type)
{
	type_ = type;

	switch(type)
	{
	case EMPTY_R:
		isBoundary = false;
		potential = 0.0f; 
		break;
	case NEGATIVE_R:
		isBoundary = true;
		potential = 0.0f;
		break;
	case POSITIVE_R:
		isBoundary = true;
		potential = 1.0f;
		break;
	}
}
