/* -.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.

* File Name : TreeSkeleton.h

* Purpose :

* Creation Date : 06-05-2013

* Last Modified : Mon 06 May 2013 10:53:35 PM CST

* Created By : Philip Zhang 

_._._._._._._._._._._._._._._._._._._._._.*/
#ifndef _TREESKELETON_INCLUDE_
#define _TREESKELETON_INCLUDE_
#include <stdio.h>

class CSkeletonNode
{
public:
	CSkeletonNode(float x,
				  float y,
				  float z,
				  float radius);
	~CSkeletonNode();
	CSkeletonNode(const CSkeletonNode &rhs);
	CSkeletonNode & operator=(const CSkeletonNode &rhs);
	void DisplayMesh(char*, CSkeletonNode *pCurNode
					 );
	void DisplayPoint(CSkeletonNode *pCurNode);
	void Write(FILE *file, int no);
	void Simplify(double);
public:
	CSkeletonNode *m_pParent;
	CSkeletonNode *m_pPrev;
	CSkeletonNode *m_pNext;
	CSkeletonNode *m_pChild;
	float m_pos[3];
	float m_radius;
	
	// temp variable for loading
	int t_no;
};

class CTreeSkeleton
{
public:
	CTreeSkeleton();
	~CTreeSkeleton();
	void Insert(float x,
				float y,
				float z,
				float radius);
	void Delete(unsigned mode);
	void Descent();
	void Ascent();
	void Next();
	void Previous();
	void Display();
	void MoveCurNode(float vector[3]);
	void ChangeRadius(float increase);
	void Simplify(double);
	void Load(const char *filename);
	void Save(const char *filename, unsigned mode);
protected:
	CSkeletonNode *m_pRoot;
	CSkeletonNode *m_pCurNode;
	int m_slices;
};

#endif
