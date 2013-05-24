/* -.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.

* File Name : CTreePointCloud.h

* Purpose :

* Creation Date : 21-05-2013

* Last Modified : Fri 24 May 2013 08:32:16 PM CST

* Created By : Philip Zhang 

_._._._._._._._._._._._._._._._._._._._._.*/

#ifndef __TREEPOINTCLOUD_INCLUDE__
#define __TREEPOINTCLOUD_INCLUDE__
#include <math3d.h>
#include <vector>

using namespace std;

class CTreeSkeleton;
class CVoxelModel;

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

float operator* (const Float3f& lhs, const Float3f& rhs);

Float3f operator* (const Float3f& lhs, float rhs);

Float3f operator* (float lhs, const Float3f& rhs);

Float3f operator- (const Float3f& lhs);

Float3f operator+ (const Float3f& lhs, const Float3f& rhs);

Float3f operator- (const Float3f& lhs, const Float3f& rhs);

bool operator== (const Float3f &lhs, const Float3f &rhs);

class CTreePointCloud
{
	friend class CVoxelModel;
	friend class CTreeSkeleton;
	// public methods
	public:
		CTreePointCloud();
		void Load(const char *filename);

	protected:
		// data members
		M3DVector3f m_vMaxRange;
		M3DVector3f m_vMinRange;
		Float3f m_root;
		vector<Float3f> m_vPoints;
};


#endif //__TREEPOINTCLOUD_INCLUDE__
