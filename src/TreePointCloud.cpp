/* -.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.

* File Name : CTreePointCloud.cpp

* Purpose :

* Creation Date : 21-05-2013

* Last Modified : Wed 22 May 2013 09:18:32 PM CST

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

Voxel::Voxel()
{
	isUsed = false;
	isEmpty = true;
}

Index::Index()
{
}

Index::Index(int X, int Y, int Z) : x(X), y(Y), z(Z)
{
}

CTreePointCloud::CTreePointCloud()
{
	m_vMinRange[0] = m_vMinRange[1] = m_vMinRange[2] = FLT_MAX; 
	m_vMinRange[0] = m_vMinRange[1] = m_vMinRange[2] = FLT_MIN; 
	m_fThreshold = 0.8;
	m_fBranchRatio = 1.0 / 13.0;
	m_iMaxStep = 6;
	m_nSlicesX = 18;
}

void CTreePointCloud::IndexPoints()
{
	for(int i = 0; i < m_vPoints.size(); i++)
	{
		int x, y, z;
		x = (m_vPoints[i].x - m_vMinRange[0]) / m_fUnitLength;
		y = (m_vPoints[i].y - m_vMinRange[1]) / m_fUnitLength;
		z = (m_vPoints[i].z - m_vMinRange[2]) / m_fUnitLength;
		m_vvvVoxels[x][y][z].indices.push_back(i);
		m_vvvVoxels[x][y][z].isEmpty = false;
	}
}

void CTreePointCloud::Load(const char *filename)
{
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
	// determin the slices in y and z direction based on nSlicesX
	m_fUnitLength = (m_vMaxRange[0] - m_vMinRange[0]) / m_nSlicesX; 
	m_iSlicesX = m_nSlicesX + 1;
	m_iSlicesY = ceil((m_vMaxRange[1] - m_vMinRange[1]) / m_fUnitLength);
	m_iSlicesZ = ceil((m_vMaxRange[2] - m_vMinRange[2]) / m_fUnitLength);
	
	m_vvvVoxels.resize(m_iSlicesX);
	for(int i = 0; i < m_vvvVoxels.size(); i++)
	{
		m_vvvVoxels[i].resize(m_iSlicesY);
		for(int j = 0; j < m_vvvVoxels[i].size(); j++)
		{
			m_vvvVoxels[i][j].resize(m_iSlicesZ);
		}
	}
	
	ifs.close();

	IndexPoints();
}

// note the change of current node determines the extraction part.
void CTreePointCloud::ExtractSkeleton(CTreeSkeleton *tree)
{
	CSkeletonNode *pCurNode = tree->m_pCurNode;
	// handle root node
	if(pCurNode == NULL)
	{
		return;
	}
	float *curPos = pCurNode->m_pos;
	Index ind;
	GetVoxelIndex(curPos, ind);
	vector<Float3f> points;
	GetNeighborPoints(ind, points);
	if(points.size() == 0)
		return;
	// just to make life easier...	
	Float3f curPoint = Float3f(curPos[0], curPos[1], curPos[2]);
	vector<vector<Float3f> > dirs_points;
	dirs_points.resize(26);
	DividePoints(points, curPoint, dirs_points);
	vector<Float3f> dirs;
	GetBranchDirections(dirs_points, m_fBranchRatio, curPoint, dirs);
	for(int i = 0; i < dirs.size(); i++)
	{
		if(dirs[i] == Float3f(0.0, 0.0, 0.0))
			continue;
		float radius = CalculateRadius(dirs_points[i], curPoint, dirs[i]);
		float length = CalculateLength(dirs_points[i], curPoint, dirs[i]);
		Float3f dstPoint = curPoint + dirs[i] * length;
		//float dest[3] = { dstPoint.x, dstPoint.y, dstPoint.z };
		//Index index;
		//GetVoxelIndex(dest, index);
		//m_vvvVoxels[index.x][index.y][index.z].isUsed = false;
		tree->Insert(dstPoint.x, dstPoint.y, dstPoint.z, radius);
		tree->m_pCurNode = pCurNode;
	}
	CSkeletonNode *pChild = pCurNode->m_pChild;
	while(pChild)
	{
		// set current node and recursive call ExtractSkeleton()
		tree->m_pCurNode = pChild;
		ExtractSkeleton(tree);
		pChild = pChild->m_pNext;
	}
}

void CTreePointCloud::GetVoxelIndex(float *pt, Index &ind)
{
	ind.x = (pt[0] - m_vMinRange[0]) / m_fUnitLength;
	ind.y = (pt[1] - m_vMinRange[1]) / m_fUnitLength;
	ind.z = (pt[2] - m_vMinRange[2]) / m_fUnitLength;
}

// the index of voxel is given, and return its nearby points
void CTreePointCloud::GetNeighborPoints(const Index &ind, vector<Float3f> &ret)
{
	int ix = ind.x, iy = ind.y, iz = ind.z;
	if(ix < 0 || iy < 0 || iz < 0 || ix >= m_iSlicesX || iy >= m_iSlicesY || iz >= m_iSlicesZ)
		return;
	// first check whether used or empty
	Voxel vol = m_vvvVoxels[ix][iy][iz];
	if(vol.isUsed || vol.isEmpty)
		return;
	// then flood to its neighbors
	// until it reaches the threshold
	int dirs[26][3] = { {1, 0, 0}, {-1, 0, 0}, {0, 1, 0}, {0, -1, 0}, {0, 0, 1}, {0, 0, -1}, // first 6 basic directions
						{1, 1, 1}, {0, 1, 1}, {-1, 1, 1}, {1, 0, 1}, {-1, 0, 1}, {1, -1, 1}, {0, -1, 1}, {-1, -1, 1}, // front directions
						{1, 1, 0}, {1, -1, 0}, {-1, 1, 0}, {-1, -1, 0}, // middle directions
						{1, 1, -1}, {0, 1, -1}, {-1, 1, -1}, {1, 0, -1}, {-1, 0, -1}, {1, -1, -1}, {0, -1, -1}, {-1, -1, -1} // back directions
					  };

	vector<Index> recent;
	recent.push_back(ind);
	float inc = FLT_MAX;
	int step_count = 0;
	int prev_count = 1;
	while(inc >= m_fThreshold)
	{
		vector<Index> tmp;
		// process recent voxels
		for(int i = 0; i < recent.size(); i++)
		{
			Index index(recent[i].x, recent[i].y, recent[i].z);
			if(!m_vvvVoxels[index.x][index.y][index.z].isEmpty && !m_vvvVoxels[index.x][index.y][index.z].isUsed)
			{
				m_vvvVoxels[index.x][index.y][index.z].isUsed = true;
				for(int j = 0; j < m_vvvVoxels[index.x][index.y][index.z].indices.size(); j++)
				{
					ret.push_back(m_vPoints[m_vvvVoxels[index.x][index.y][index.z].indices[j]]);
				}
			}
		}

		// flood to new voxels
		for(int i = 0; i < recent.size(); i++)
		{
			for(int j = 0; j < 26; j++)
			{
				Index index(recent[i].x + dirs[i][0], recent[i].y + dirs[i][1], recent[i].z + dirs[i][2]);
				if(index.x < 0 || index.y < 0 || index.z < 0 || index.x >= m_iSlicesX || index.y >= m_iSlicesY || index.z >= m_iSlicesZ)
					continue;
				if(!m_vvvVoxels[index.x][index.y][index.z].isEmpty && !m_vvvVoxels[index.x][index.y][index.z].isUsed)
				{
					tmp.push_back(index);
				}
			}
		}
		recent.clear();
		//copy(tmp.begin(), tmp.end(), recent.begin());
		for(int i = 0; i < tmp.size(); i++)
			recent.push_back(tmp[i]);
		inc = float(recent.size()) / prev_count;
		prev_count = recent.size();
		if(++step_count > m_iMaxStep)
			break;
	}
}

void CTreePointCloud::DividePoints(const vector<Float3f> &points, const Float3f &pt, vector<vector<Float3f> > & dirs_points)
{
	Float3f dirs[26] = { 
						Float3f(1, 0, 0).identity(), 
						Float3f(-1, 0, 0).identity(), 
						Float3f(0, 1, 0).identity(), 
						Float3f(0, -1, 0).identity(), 
						Float3f(0, 0, 1).identity(), 
						Float3f(0, 0, -1).identity(), // first 6 basic directions
						Float3f(1, 1, 1).identity(), 
						Float3f(0, 1, 1).identity(), 
						Float3f(-1, 1, 1).identity(), 
						Float3f(1, 0, 1).identity(), 
						Float3f(-1, 0, 1).identity(), 
						Float3f(1, -1, 1).identity(), 
						Float3f(0, -1, 1).identity(), 
						Float3f(-1, -1, 1).identity(), // front directions
						Float3f(1, 1, 0).identity(), 
						Float3f(1, -1, 0).identity(), 
						Float3f(-1, 1, 0).identity(), 
						Float3f(-1, -1, 0).identity(), // middle directions
						Float3f(1, 1, -1).identity(), 
						Float3f(0, 1, -1).identity(), 
						Float3f(-1, 1, -1).identity(), 
						Float3f(1, 0, -1).identity(), 
						Float3f(-1, 0, -1).identity(), 
						Float3f(1, -1, -1).identity(), 
						Float3f(0, -1, -1).identity(), 
						Float3f(-1, -1, -1).identity() // back directions
	};
	dirs_points.resize(26);
	for(int i = 0; i < points.size(); i++)
	{
		float maxCos = -1.0f;
		int maxIndex = -1;
		Float3f d = (points[i] - pt).identity();
		for(int j = 0; j < 26; j++)
		{
			if(d * dirs[j] > maxCos)
			{
				maxIndex = j;
				maxCos = d * dirs[j];
			}
		}
		dirs_points[maxIndex].push_back(points[i]);
	}
}

void CTreePointCloud::GetBranchDirections(const vector<vector<Float3f> > &dirs_points, float ratio, const Float3f &pt, vector<Float3f> &dirs)
{
	dirs.resize(26);
	int total_count = 0;
	for(int i = 0; i < dirs_points.size(); i++)
	{
		total_count += dirs_points[i].size();
		dirs[i] = Float3f(0.0, 0.0, 0.0);
	}
	if(total_count == 0)
		return;
	
	for(int i = 0; i < dirs_points.size(); i++)
	{
		// if greater than average, then this is a branch direction
		// calculate the average directions of the points
		if(dirs_points[i].size() / (float)total_count >= ratio)
		{
			Float3f d = Float3f(0, 0, 0);
			for(int j = 0; j < dirs_points[i].size(); j++)
			{
				d += dirs_points[i][j] - pt;
			}
			dirs[i] = d.identity();
		}
	}
}

float CTreePointCloud::CalculateRadius(const vector<Float3f> &points, const Float3f &origin, const Float3f &dir)
{
	float radius;
	for(int i = 0; i < points.size(); i++)
	{
		// calculate the distance from points[i] to line
		Float3f r = origin + (points[i] - origin) * dir / (dir * dir) * dir - points[i];
		radius += r.length();
	}
	return radius / points.size() * 2;
}

float CTreePointCloud::CalculateLength(const vector<Float3f> &points, const Float3f &origin, const Float3f &dir)
{
	float length;
	for(int i = 0; i < points.size(); i++)
	{
		float l = (points[i] - origin) * dir;
		length += l;
	}
	return length / points.size() * 2;
}

