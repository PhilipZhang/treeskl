/* -.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.

* File Name : TreeSkeleton.h

* Purpose :

* Creation Date : 06-05-2013

* Last Modified : Thu 09 May 2013 10:36:43 PM CST

* Created By : Philip Zhang 

_._._._._._._._._._._._._._._._._._._._._.*/
#ifndef _TREESKELETON_INCLUDE_
#define _TREESKELETON_INCLUDE_
#include <stdio.h>

typedef void (*SetColor)(void);

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
	// for terminal mode testing.
	void DisplayMesh(char*, CSkeletonNode *pCurNode
					 );
	// for gui display
	void DisplayMesh(CSkeletonNode *pCurNode, int slices, SetColor General, SetColor Special);
	void DisplayPoint();
	void Write(FILE *file, int no);
	unsigned WritePoint(FILE *fout);
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
	void Display(SetColor General, SetColor Special, unsigned mode);
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
