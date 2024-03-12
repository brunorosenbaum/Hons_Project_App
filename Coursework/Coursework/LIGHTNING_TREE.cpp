#include "LIGHTNING_TREE.h"

LIGHTNING_TREE::LIGHTNING_TREE() : root_(nullptr)
{
	nodes_.clear(); 
}

LIGHTNING_TREE::~LIGHTNING_TREE()
{
	ClearNodes(); 
}

void LIGHTNING_TREE::AddChild(int iParent_x, int iParent_y, int iChild_x, int iChild_y, bool isTarget)
{
	if(root_)
	{
		LIGHTNING_TREE_NODE* parentPtr = NULL;
		auto itr = nodes_.begin();
		while(itr != nodes_.end())
		{
			parentPtr = (*itr); //Assign current node as parent
			if(parentPtr && parentPtr->x_ == iParent_x && parentPtr->y_ == iParent_y)
			{
				//Add child node
				LIGHTNING_TREE_NODE* childPtr = new LIGHTNING_TREE_NODE(); //TODO: CAUSING MEMORY LEAK
				//TODO: IS THIS RECURSING TO INFINITY?
				if(childPtr)
				{
					childPtr->x_ = iChild_x; //Assign xy pos
					childPtr->y_ = iChild_y;
					childPtr->parent_ = parentPtr; //And its parent as the prev node
					childPtr->isTarget_ = isTarget;

					parentPtr->AddChild(childPtr); //It's recursive btw
					nodes_.push_back(childPtr);
					break; 
				}
			}
			++itr; 
		}
	}
}

void LIGHTNING_TREE::ClearNodes() //Safely deletes nodes
{
	auto itr = nodes_.begin();
	while(itr != nodes_.end())
	{
		if(*itr)
		{
			delete* itr;
			*itr = NULL;
			++itr; 
		}
	}
	root_ = nullptr;
	nodes_.clear(); 
}

bool LIGHTNING_TREE::SetRoot(LIGHTNING_TREE_NODE* root) //Safely sets root by setting parent to null
{
	ClearNodes();
	if(root)
	{
		root_ = root;
		root_->parent_ = NULL;
		nodes_.push_back(root_);
		return true; 
	}
	return false;
}
