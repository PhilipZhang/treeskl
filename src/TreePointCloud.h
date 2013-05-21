/* -.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.

* File Name : CTreePointCloud.h

* Purpose :

* Creation Date : 21-05-2013

* Last Modified : Tue 21 May 2013 10:29:28 PM CST

* Created By : Philip Zhang 

_._._._._._._._._._._._._._._._._._._._._.*/

#ifndef __TREEPOINTCLOUD_INCLUDE__
#define __TREEPOINTCLOUD_INCLUDE__
#include <math3d.h>
#include <vector>
using namespace std;

class CTreeSkeleton;

struct Float3f
{
	Float3f();
	Float3f(float X, float Y, float Z);
	~Float3f();
	float x, y, z;
	Float3f& operator +=(const Float3f& rhs);

	inline float length() const
	{
		return sqrt(x * x + y * y + z * z);
	}

	Float3f identity() const;

};

struct Voxel
{
	Voxel();
	vector<int> indices;
	bool isUsed;
	bool isEmpty;
};

struct Index
{
	Index();
	Index(int X, int Y, int Z);
	int x, y, z;
};

class CTreePointCloud
{
	// public methods
	public:
		CTreePointCloud();
		void Load(const char *filename);
		void ExtractSkeleton(CTreeSkeleton *tree);

	// auxiliary functions and data members
	protected:
		// index points in voxels
		void IndexPoints();
		void GetNeighborPoints(const Index &ind, vector<Float3f>& ret);
		void DividePoints(const vector<Float3f> &points, const Float3f &pt, vector<vector<Float3f> > & dirs_points);
		void GetBranchDirections(const vector<vector<Float3f> > &dirs_points, float ratio, const Float3f &pt, vector<Float3f> &dirs);
		void GetVoxelIndex(float *pt, Index &ind);
		float CalculateLength(const vector<Float3f> &points, const Float3f &origin, const Float3f &dir);
		float CalculateRadius(const vector<Float3f> &points, const Float3f &origin, const Float3f &dir);
		// data members
		M3DVector3f m_vMaxRange;
		M3DVector3f m_vMinRange;
		vector<Float3f> m_vPoints;
		vector<vector<vector<Voxel> > > m_vvvVoxels;
		int m_iSlicesX, m_iSlicesY, m_iSlicesZ;
		int m_nSlicesX;
		float m_fUnitLength;
		float m_fThreshold;
		float m_fBranchRatio;
		int m_iMaxStep;
};


#endif //__TREEPOINTCLOUD_INCLUDE__
