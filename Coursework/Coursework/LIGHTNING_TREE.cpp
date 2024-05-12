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
				LIGHTNING_TREE_NODE* childPtr = new LIGHTNING_TREE_NODE(); 
				//TODO: IS THIS RECURSING TO INFINITY?
				if(childPtr)
				{
					childPtr->x_ = iChild_x; //Assign xy pos
					childPtr->y_ = iChild_y;
					childPtr->parent_ = parentPtr; //And its parent as the prev node
					childPtr->isTarget_ = isTarget;

					parentPtr->AddChild(childPtr); //It's recursive
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

void LIGHTNING_TREE::SetTreeThickness()
{ //This method is for culling the smaller branches
	if (nodes_.size() > 1 && root_) //If there are nodes and theres a root
	{
		root_->isMainChannel = true; //Set root as main channel
		root_->thickness = THICKNESS;
		root_->attenuation = 1.0f;

		//Find last node/leaf
		LIGHTNING_TREE_NODE* endNode = nodes_.back();
		if (endNode)
		{
			while (root_ != endNode) //While the end node is not the root (theres root + other nodes)
			{
				endNode->isMainChannel = true;
				endNode->thickness = THICKNESS;
				endNode->attenuation = 1.0f; //Its attenuation value will always be 1 if on main channel
				endNode = endNode->parent_; //Redefine ptr
			}
			AttenuateThickness(root_); 
		}
	}
}

void LIGHTNING_TREE::AttenuateThickness(LIGHTNING_TREE_NODE* nodePtr)
{ //This is a recursive function
	if(nodePtr)
	{
		if(!nodePtr->isMainChannel && nodePtr->parent_) //If not on the main channel but has a parent
		{//So, branching away
			//Reduce its attenuation
			nodePtr->attenuation = nodePtr->parent_->attenuation * ATTENUATION; //*0.7 everytime, so itll reduce more and more
			if(nodePtr->parent_->thickness > 1.0f)
			{
				nodePtr->thickness = nodePtr->parent_->thickness / 2.0f; //And half its thickness
			}
		}
		auto itr = nodePtr->children_.begin(); //Go through the nodes children
		while (itr != nodePtr->children_.end())
		{
			AttenuateThickness(*itr); //Recursion
			++itr; 
		}
	}
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
