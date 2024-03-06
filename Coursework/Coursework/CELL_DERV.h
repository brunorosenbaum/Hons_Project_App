#pragma once
#include <vector>

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

class CLUSTER //Class that holds small groups of cells in proximity. Necessary to compute the average x and y coords
//of a same group of cells. Used for equation 4, where we calculate the potential of a cell taking into account
//the distance between it and nearby cells (r).
////This is done to generate stronger negative potentials among nearby negative charges.
{
public:
    CLUSTER(): c_x(0), c_y(0), c_xSum(0), c_ySum(0), c_xAvg(0), c_yAvg(0)
    {
        cluster_Cells.clear(); 
    }
    virtual ~CLUSTER();

public:
    int c_x, c_y; //XY pos of multi scaled grid map (cluster)
    int c_xSum, c_ySum; //Sum of xy coords in the same cluster
    int c_xAvg, c_yAvg;  //Averages of xy coords

    std::vector<CELL_DERV> cluster_Cells; //Cells in the same cluster
};

