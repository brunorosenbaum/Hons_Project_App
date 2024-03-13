#pragma once
#include <vector>

//#include "CELL.h"
//########################################################  CELL  #####################################################
enum CELL_TYPE_R
{
	EMPTY_R,
    NEGATIVE_R,
    POSITIVE_R
};

class CELL_R /*:
    public CELL*/
//Real talk how do I polymorphism here. Cause lots of functionality from CELL class is the same
////and that way I wouldn't repeat code - AS OF 12/3/2024: I GIVE UP THIS OVERCOMPLICATES EVERYTHING
{
public:
    //As of 12/3/24 im changing the whole cell_derv class so it has its own functionality and doesnt have to inherit
    //Im getting exceptions thrown bc of not being able to access the parent data in the first ones bc 

    //CELL_R(float north, //Same constructor to call base class
    //    float east, //TODO: ASK IF THIS IS CORRECT?
    //    float south,
    //    float west,
    //    CELL* parent = nullptr,
    //    int depth = 0);
    CELL_R(); 
 /*   CELL_R(int x, int y); 
    CELL_R(int x, int y, CELL_TYPE type, float phi); */
    ~CELL_R();

    void SetCellType(CELL_TYPE_R type);
public:
    //Neighbors. Clockwise for 2d. 
    static const int NEIGHBORS_X_DIFFERENCE[8];
    static const int NEIGHBORS_Y_DIFFERENCE[8];


    

    //Note: I don't know if Y's m_iX & m_iY xy coord values are the same as K's, bc K calculates the center
    //Quoting Y's paper: 'The issue arises mainly because
    /*we compute potentials by assuming that each charged point is
    in the center of a cell.As a result, the 1/r term cannot create
    extremely small values between even neighboring cells.'*/
    //^^^^ Using this as basis of why i shouldnt inherit from k's cell class

    int x, y; //XY coords on grid
    int parentX, parentY;
    float potential; //Phi
    float selectionProb; //Probability of being selected. P(i) in formula. b in K's code.
    bool isBoundary;
    CELL_TYPE_R type_;

    //IMPORTANT: THESE ARE THE VARIABLES FROM EACH CELL THAT'LL HOLD THE INFO FOR THE FORMULA PHI = P/(N X B)
    float N_;// previous electric potential for negative charges - THIS IS N
    float P_; //THIS IS P, the same but for positive cell charges
    float B_; //THIS IS B, for boundary cells

};



//###################################################   CLUSTER    ##################################################

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
	~CLUSTER(){}

public:
    int c_x, c_y; //XY pos of multi scaled grid map (cluster)
    int c_xSum, c_ySum; //Sum of xy coords in the same cluster
    int c_xAvg, c_yAvg;  //Averages of xy coords

    std::vector<CELL_R> cluster_Cells; //Cells in the same cluster
};

