/* -.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.

* File Name : VoxelModel.h

* Purpose :

* Creation Date : 22-05-2013

* Last Modified : Fri 24 May 2013 08:30:25 PM CST

* Created By : Philip Zhang 

_._._._._._._._._._._._._._._._._._._._._.*/

#ifndef __VOXELMODEL_INCLUDE__
#define __VOXELMODEL_INCLUDE__

#include <vector>
using namespace std;
class CTreeSkeleton;
class CTreePointCloud;
class Float3f;

struct Voxel
{
	Voxel();
	vector<int> indices;
	bool isUsed;
	bool isEmpty;
};

// used to index voxel
struct Index
{
	Index();
	Index(int X, int Y, int Z);
	int x, y, z;
};

class CVoxelModel
{
	friend class CTreeSkeleton;
public:
	CVoxelModel();
	void ExtractSkeleton(CTreeSkeleton *tree, unsigned mode);
	void IndexPoints(CTreePointCloud *pPointCloud);	// build a new indices.
	//vector<vector<vector<Voxel> > > *GetVoxelModel() const;
protected:
	void GetNeighborPoints(const vector<Index> &ind, const vector<Float3f> &nodes_pdirs, vector<vector<Float3f> >& ret);
	void DividePoints(const vector<Float3f> &points, const Float3f &pt, vector<vector<Float3f> > & dirs_points);
	void GetBranchDirections(const vector<vector<Float3f> > &dirs_points, float ratio, const Float3f &pt, vector<Float3f> &dirs);
	void FindNearestPoint(const Float3f &dstPoint, const vector<Float3f> &points, Float3f &out);
	float GetRootRadius();
	void GetVoxelIndex(float *pt, Index &ind);
	float CalculateLength(const vector<Float3f> &points, const Float3f &origin, const Float3f &dir);
	float CalculateRadius(const vector<Float3f> &points, const Float3f &origin, const Float3f &dir);
	// data members
	vector<vector<vector<Voxel> > > m_vvvVoxels;
	CTreePointCloud *m_pPointCloud;
	vector<Float3f> *m_pvPoints;
	int m_iSlicesX, m_iSlicesY, m_iSlicesZ;
	float m_fUnitLength;
	float m_fThreshold;
	float m_fBranchRatio;
	float m_fAngleCos;
	int m_iMaxStep;
};


#endif
