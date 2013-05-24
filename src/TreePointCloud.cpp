/* -.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.

* File Name : CTreePointCloud.cpp

* Purpose :

* Creation Date : 21-05-2013

* Last Modified : Fri 24 May 2013 09:36:37 PM CST

* Created By : Philip Zhang 

_._._._._._._._._._._._._._._._._._._._._.*/

#include "TreePointCloud.h"
#include "TreeSkeleton.h"
#include <float.h>
#include <fstream>
#include <sstream>

Float3f::Float3f()
{
}

Float3f::Float3f(float X, float Y, float Z) : x(X), y(Y), z(Z)
{
}

Float3f::~Float3f()
{
}

Float3f& Float3f::operator +=(const Float3f& rhs)
{
	this->x += rhs.x;
	this->y += rhs.y;
	this->z += rhs.z;
	return *this;
}

Float3f Float3f::identity() const
{
	float len = length();
	if(len == 0)
		return Float3f(0.0f, 0.0f, 0.0f);
	return Float3f(x / len, y / len, z / len);
}

float operator* (const Float3f& lhs, const Float3f& rhs)
{
	return lhs.x * rhs.x + lhs.y * rhs.y + lhs.z* rhs.z;
}

Float3f operator* (const Float3f& lhs, float rhs)
{
	return Float3f(lhs.x * rhs, lhs.y * rhs, lhs.z * rhs);
}

Float3f operator* (float lhs, const Float3f& rhs)
{
	return Float3f(lhs * rhs.x , lhs * rhs.y, lhs * rhs.z);
}

Float3f operator- (const Float3f& lhs)
{
	return Float3f(-lhs.x, -lhs.y, -lhs.z);
}

Float3f operator+ (const Float3f& lhs, const Float3f& rhs)
{
	return Float3f(lhs.x + rhs.x, lhs.y + rhs.y, lhs.z + rhs.z);
}

Float3f operator- (const Float3f& lhs, const Float3f& rhs)
{
	return Float3f(lhs.x - rhs.x, lhs.y - rhs.y, lhs.z - rhs.z);
}

bool operator== (const Float3f &lhs, const Float3f &rhs)
{
	if(lhs.x == rhs.x && lhs.y == rhs.y && lhs.z == rhs.z)
		return true;
	return false;
}

CTreePointCloud::CTreePointCloud()
{
}

void CTreePointCloud::Load(const char *filename)
{
	m_vMinRange[0] = m_vMinRange[1] = m_vMinRange[2] = FLT_MAX; 
	m_vMinRange[0] = m_vMinRange[1] = m_vMinRange[2] = FLT_MIN; 
	m_vPoints.clear();
	fstream ifs(filename);
	char buffer[100];
	// read the point cloud data into m_vPoints, 
	// and find the boundaries of the data.
	while(ifs.getline(buffer, 100))
	{
		istringstream line(buffer);
		float coor[3];
		line >> coor[0] >> coor[1] >> coor[2];			
		for(int i = 0; i < 3; i++)
		{
			if(coor[i] < m_vMinRange[i])
				m_vMinRange[i] = coor[i];
			if(coor[i] > m_vMaxRange[i])
				m_vMaxRange[i] = coor[i];
		}
		m_vPoints.push_back(Float3f(coor[0], coor[1], coor[2]));
	}
	
	ifs.close();
}

