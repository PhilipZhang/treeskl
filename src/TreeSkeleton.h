/* -.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.

* File Name : TreeSkeleton.h

* Purpose :

* Creation Date : 06-05-2013

* Last Modified : Tue 28 May 2013 06:39:12 PM CST

* Created By : Philip Zhang 

_._._._._._._._._._._._._._._._._._._._._.*/
#ifndef _TREESKELETON_INCLUDE_
#define _TREESKELETON_INCLUDE_
#include <stdio.h>
#include <vector>
using namespace std;

class CTreePointCloud;
class CVoxelModel;
class CTreeSkeleton;
class Float3f;

typedef void (*SetColor)(void);

class CSkeletonNode
{
	friend class CTreeSkeleton;
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
	void DisplayMesh(CSkeletonNode *pCurNode, int slices);
	void SkeletonToPoint(vector<Float3f> &);
	void Write(FILE *file, int no);
	unsigned WritePoint(FILE *fout);
	void Simplify(double, unsigned, CTreeSkeleton *);
	void Select(M3DVector3f pt, CSkeletonNode **pCurNode);
	void CheckRange(M3DVector3f maxRange, M3DVector3f minRange);
	void ChangeRadius(float increase, unsigned mode);
	void LinearRadius(float ratio);
	void SquareRadius();
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
	friend class CSkeletonNode;
	friend class CVoxelModel;

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
	void Display(unsigned mode);
	void DisplayVoxel();
	void MoveCurNode(float vector[3]);
	void ChangeRadius(float increase, unsigned mode);
	void Simplify(double, unsigned mode);
	void Select(M3DVector3f pt);
	void Load(const char *filename);
	void Save(const char *filename, unsigned mode);
	void LinearRadius(float ratio);
	void SquareRadius();
	void LoadPointCloud(const char *filename);
	void LoadVoxelModel(int sampleId);
	void ExtractSkeleton(unsigned mode = 0);
	void SkeletonToPoint();
	void DisplaySelected();

protected:
	CSkeletonNode *m_pRoot;
	CSkeletonNode *m_pCurNode;
	CTreePointCloud *m_pPointCloud;
	CVoxelModel *m_pVoxelModel;
	int m_slices;
};

#endif
