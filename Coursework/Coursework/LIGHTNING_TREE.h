#pragma once
#include <vector>

#define THICKNESS 1.0f
#define ATTENUATION 0.7f

//File stores node information for classes. Will be used in rational_solver. 
class LIGHTNING_TREE_NODE //Nodes of the tree. Has parent, children, and xy coords. 
{
public: 
	LIGHTNING_TREE_NODE():
	x_(0), y_(0), parent_(nullptr), isTarget_(false), isMainChannel(false),
	thickness(THICKNESS), attenuation(ATTENUATION)
	{
		children_.clear(); 
	}
	virtual  ~LIGHTNING_TREE_NODE(){}

	void AddChild(LIGHTNING_TREE_NODE* child)
	{
		if(child)
		{
			child->parent_ = this;
			children_.push_back(child); 
		}
	}

public:
	int x_, y_;
	LIGHTNING_TREE_NODE* parent_;
	std::vector<LIGHTNING_TREE_NODE*> children_;
	bool isTarget_; 
	bool isMainChannel; //If the current node is part of the main channel
	float thickness;
	float attenuation;
};


class LIGHTNING_TREE
{
public:
	LIGHTNING_TREE();
	virtual ~LIGHTNING_TREE(); 

	//Add child node
	void AddChild(int iParent_x, int iParent_y, int iChild_x, int iChild_y, bool isTarget = false); 
	//Clear all nodes
	void ClearNodes();
	//Set thickness of tree leaves
	void SetTreeThickness();
	void AttenuateThickness(LIGHTNING_TREE_NODE* nodePtr); 

	//Getsetters
	const LIGHTNING_TREE_NODE* GetRoot()const { return root_; }
	bool SetRoot(LIGHTNING_TREE_NODE* root_);

	std::vector<LIGHTNING_TREE_NODE*>& GetNodes() { return nodes_; }

private:
	LIGHTNING_TREE_NODE* root_; //Root of lightning tree
	std::vector<LIGHTNING_TREE_NODE*> nodes_; //Vector of child nodes
	
};

