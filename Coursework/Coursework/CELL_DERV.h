#pragma once
#include "CELL.h"
class CELL_DERV :
    public CELL
//Real talk how do I polymorphism here. Cause lots of functionality from CELL class is the same
////and that way I wouldn't repeat code
{
public:
    CELL_DERV(float north, //Same constructor to call base class
        float east, //TODO: ASK IF THIS IS CORRECT?
        float south,
        float west,
        CELL* parent = nullptr,
        int depth = 0);
    ~CELL_DERV(); 
public:
    //IMPORTANT: THESE ARE THE VARIABLES FROM EACH CELL THAT'LL HOLD THE INFO FOR THE FORMULA PHI = P/(N X B)
    float N_;// previous electric potential for negative charges - THIS IS N
    float P_; //THIS IS P, the same but for positive cell charges
    float B_; //THIS IS B, for boundary cells

    //Note: I don't know if Y's m_iX & m_iY xy coord values are the same as K's, bc K calculates the center
    //Quoting Y's paper: 'The issue arises mainly because
    /*we compute potentials by assuming that each charged point is
    in the center of a cell.As a result, the 1/r term cannot create
    extremely small values between even neighboring cells.'*/
};

