/* -.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.

* File Name : VoxelModel.cpp

* Purpose :

* Creation Date : 22-05-2013

* Last Modified : Fri 24 May 2013 07:12:41 PM CST

* Created By : Philip Zhang 

_._._._._._._._._._._._._._._._._._._._._.*/

#include "VoxelModel.h"
#include "TreePointCloud.h"
#include "TreeSkeleton.h"
#include <queue>
#include <float.h>

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

CVoxelModel::CVoxelModel()
{
	// for skl 4
	m_fThreshold = 0.5;
	m_fBranchRatio = 1.0 / 13.0;
	m_iMaxStep = 4;
	m_fAngleCos = 0.5;
	m_iSlicesX = 50;
}

void CVoxelModel::IndexPoints(CTreePointCloud *pPointCloud)
{
	m_pPointCloud = pPointCloud;
	m_pvPoints = &(pPointCloud->m_vPoints);
	float *pMinRange = pPointCloud->m_vMinRange;
	float *pMaxRange = pPointCloud->m_vMaxRange;
	m_fUnitLength = (pMaxRange[0] - pMinRange[0]) / (m_iSlicesX - 1); 
	// determin the slices in y and z direction based on nSlicesX
	m_iSlicesY = ceil((pMaxRange[1] - pMinRange[1]) / m_fUnitLength);
	m_iSlicesZ = ceil((pMaxRange[2] - pMinRange[2]) / m_fUnitLength);
	m_vvvVoxels.clear();
	
	m_vvvVoxels.resize(m_iSlicesX);
	for(int i = 0; i < m_vvvVoxels.size(); i++)
	{
		m_vvvVoxels[i].resize(m_iSlicesY);
		for(int j = 0; j < m_vvvVoxels[i].size(); j++)
		{
			m_vvvVoxels[i][j].resize(m_iSlicesZ);
		}
	}
	
	for(int i = 0; i < m_pvPoints->size(); i++)
	{
		int x, y, z;
		x = ((*m_pvPoints)[i].x - pMinRange[0]) / m_fUnitLength;
		y = ((*m_pvPoints)[i].y - pMinRange[1]) / m_fUnitLength;
		z = ((*m_pvPoints)[i].z - pMinRange[2]) / m_fUnitLength;
		m_vvvVoxels[x][y][z].indices.push_back(i);
		m_vvvVoxels[x][y][z].isEmpty = false;
	}
}

// note the change of current node in tree determines the extraction part.
// mode 0 represents flood from current level until leaf level...
// mode 1 represents just parallel flood one level
void CVoxelModel::ExtractSkeleton(CTreeSkeleton *tree, unsigned mode)
{
	CSkeletonNode *pCurNode = tree->m_pCurNode;
	// handle root node
	if(pCurNode == NULL)
	{
		return;
	}
	vector<CSkeletonNode *> vec_pChild;	// hold all first child nodes in the same level
	// fisrt find all first child nodes on the same level as the current node
	CSkeletonNode *pCurChild = (pCurNode->m_pParent) ? pCurNode->m_pParent->m_pChild : pCurNode;
	vec_pChild.push_back(tree->m_pRoot);
	while(true)
	{
		bool bFound = false;
		for(int i = 0; i < vec_pChild.size(); i++)
		{
			if(vec_pChild[i] == pCurChild)
			{
				bFound = true;
				break;
			}
		}
		if(bFound)
			break;
		vector<CSkeletonNode *> tmp;
		for(int i = 0; i < vec_pChild.size(); i++)
		{
			CSkeletonNode *pBrother = vec_pChild[i];
			while(pBrother)
			{
				tmp.push_back(pBrother->m_pChild);
				pBrother = pBrother->m_pNext;
			}
		}
		vec_pChild.clear();
		for(int i = 0; i < tmp.size(); i++)
			vec_pChild.push_back(tmp[i]);
	}

	if(mode == 0)
	{
		while(!vec_pChild.empty())
		{
			vector<CSkeletonNode *> vec_pBrother;
			// get all the nodes on current level
			for(int i = 0; i < vec_pChild.size(); i++)
			{
				CSkeletonNode *pBrother = vec_pChild[i];
				while(pBrother)
				{
					vec_pBrother.push_back(pBrother);
					pBrother = pBrother->m_pNext;
				}
			}
			vector<vector<Float3f> > nodes_points;
			vector<Index> nodes_index;
			vector<Float3f> nodes_pdirs;		// direction from parent
			int nodes_count = vec_pBrother.size();
			nodes_points.resize(nodes_count);

			// for all nodes on current level, expand branches
			// and push their first children in vec_pChild
			for(int i = 0; i < vec_pBrother.size(); i++)
			{
				CSkeletonNode *pBrother = vec_pBrother[i];
				float *curPos = pBrother->m_pos;
				Index ind;
				GetVoxelIndex(curPos, ind);
				nodes_index.push_back(ind);
				if(!pBrother->m_pParent)
					nodes_pdirs.push_back(Float3f(0.0, 1.0, 0.0));
				else
				{
					float *parPos = pBrother->m_pParent->m_pos;
					nodes_pdirs.push_back(Float3f(curPos[0] - parPos[0], 
												  curPos[1] - parPos[1],
												  curPos[2] - parPos[2]).identity());
				}
			}
			GetNeighborPoints(nodes_index, nodes_pdirs, nodes_points);
			for(int i = 0; i < vec_pBrother.size(); i++)
			{
				CSkeletonNode *pBrother = vec_pBrother[i];
				if(nodes_points[i].size() == 0)
					continue;
				// just to make life easier...	
				float *curPos = pBrother->m_pos;
				Float3f curPoint = Float3f(curPos[0], curPos[1], curPos[2]);
				vector<vector<Float3f> > dirs_points;
				dirs_points.resize(26);
				DividePoints(nodes_points[i], curPoint, dirs_points);
				vector<Float3f> dirs;
				GetBranchDirections(dirs_points, m_fBranchRatio, curPoint, dirs);
				tree->m_pCurNode = pBrother;

				for(int j = 0; j < dirs.size(); j++)
				{
					if(dirs[j] == Float3f(0.0, 0.0, 0.0))
						continue;
					float radius = CalculateRadius(dirs_points[j], curPoint, dirs[j]);
					float length = CalculateLength(dirs_points[j], curPoint, dirs[j]);
					Float3f dstPoint = curPoint + dirs[j] * length;
					Float3f nearPoint;
					FindNearestPoint(dstPoint, dirs_points[j], nearPoint);
					tree->Insert(nearPoint.x, nearPoint.y, nearPoint.z, radius);
					tree->m_pCurNode = pBrother;
				}
			}

			// generate new to-process nodes
			vec_pChild.clear();
			for(int i = 0; i < vec_pBrother.size(); i++)
			{
				if(vec_pBrother[i]->m_pChild)
					vec_pChild.push_back(vec_pBrother[i]->m_pChild);
			}
		}
	}
	if(mode == 1)
	{
		vector<CSkeletonNode *> vec_pBrother;
		// get all the nodes on current level
		for(int i = 0; i < vec_pChild.size(); i++)
		{
			CSkeletonNode *pBrother = vec_pChild[i];
			while(pBrother)
			{
				vec_pBrother.push_back(pBrother);
				pBrother = pBrother->m_pNext;
			}
		}
		vector<vector<Float3f> > nodes_points;
		vector<Index> nodes_index;
		vector<Float3f> nodes_pdirs;		// direction from parent
		int nodes_count = vec_pBrother.size();
		nodes_points.resize(nodes_count);

		// for all nodes on current level, expand branches
		// and push their first children in vec_pChild
		for(int i = 0; i < vec_pBrother.size(); i++)
		{
			CSkeletonNode *pBrother = vec_pBrother[i];
			float *curPos = pBrother->m_pos;
			Index ind;
			GetVoxelIndex(curPos, ind);
			nodes_index.push_back(ind);
			if(!pBrother->m_pParent)
				nodes_pdirs.push_back(Float3f(0.0, 1.0, 0.0));
			else
			{
				float *parPos = pBrother->m_pParent->m_pos;
				nodes_pdirs.push_back(Float3f(curPos[0] - parPos[0], 
											  curPos[1] - parPos[1],
											  curPos[2] - parPos[2]).identity());
			}
		}
		GetNeighborPoints(nodes_index, nodes_pdirs, nodes_points);
		for(int i = 0; i < vec_pBrother.size(); i++)
		{
			CSkeletonNode *pBrother = vec_pBrother[i];
			if(nodes_points[i].size() == 0)
				continue;
			// just to make life easier...	
			float *curPos = pBrother->m_pos;
			Float3f curPoint = Float3f(curPos[0], curPos[1], curPos[2]);
			vector<vector<Float3f> > dirs_points;
			dirs_points.resize(26);
			DividePoints(nodes_points[i], curPoint, dirs_points);
			vector<Float3f> dirs;
			GetBranchDirections(dirs_points, m_fBranchRatio, curPoint, dirs);
			tree->m_pCurNode = pBrother;

			for(int j = 0; j < dirs.size(); j++)
			{
				if(dirs[j] == Float3f(0.0, 0.0, 0.0))
					continue;
				float radius = CalculateRadius(dirs_points[j], curPoint, dirs[j]);
				float length = CalculateLength(dirs_points[j], curPoint, dirs[j]);
				Float3f dstPoint = curPoint + dirs[j] * length;
				Float3f nearPoint;
				FindNearestPoint(dstPoint, dirs_points[j], nearPoint);
				tree->Insert(nearPoint.x, nearPoint.y, nearPoint.z, radius);
				tree->m_pCurNode = pBrother;
			}
		}
	}
	tree->m_pCurNode = pCurNode;
}

void CVoxelModel::GetVoxelIndex(float *pt, Index &ind)
{
	float *pMinRange = m_pPointCloud->m_vMinRange;
	float *pMaxRange = m_pPointCloud->m_vMaxRange;
	ind.x = (pt[0] - pMinRange[0]) / m_fUnitLength;
	ind.y = (pt[1] - pMinRange[1]) / m_fUnitLength;
	ind.z = (pt[2] - pMinRange[2]) / m_fUnitLength;
}

// the index of voxel is given, and return its nearby points
void CVoxelModel::GetNeighborPoints(const vector<Index> &nodes_index, const vector<Float3f> &nodes_pdirs, vector<vector<Float3f> > &ret)
{
	// 26 flooding directions
	int dirs[26][3] = { {1, 0, 0}, {-1, 0, 0}, {0, 1, 0}, {0, -1, 0}, {0, 0, 1}, {0, 0, -1}, // first 6 basic directions
						{1, 1, 1}, {0, 1, 1}, {-1, 1, 1}, {1, 0, 1}, {-1, 0, 1}, {1, -1, 1}, {0, -1, 1}, {-1, -1, 1}, // front directions
						{1, 1, 0}, {1, -1, 0}, {-1, 1, 0}, {-1, -1, 0}, // middle directions
						{1, 1, -1}, {0, 1, -1}, {-1, 1, -1}, {1, 0, -1}, {-1, 0, -1}, {1, -1, -1}, {0, -1, -1}, {-1, -1, -1} // back directions
					  };

	int nodes_count = nodes_index.size();
	float *nodes_inc = new float[nodes_count];
	int *nodes_prev_count = new int[nodes_count];
	vector<vector<Index> > nodes_recent;
	nodes_recent.resize(nodes_count);
	for(int i = 0; i < nodes_count; i++)
	{
		nodes_inc[i] = FLT_MAX;
		nodes_prev_count[i] = 1;
		m_vvvVoxels[nodes_index[i].x][nodes_index[i].y][nodes_index[i].z].isUsed = true;
		nodes_recent[i].push_back(nodes_index[i]);
	}
	for(int i = 0; i < m_iMaxStep; i++)
	{
		for(int j = 0; j < nodes_count; j++)
		{
			if(nodes_inc[j] < m_fThreshold)
				continue;

			vector<Index> tmp;

			// flood to new voxels
			for(int t = 0; t < 26; t++)
			{
				for(int k = 0; k < nodes_recent[j].size(); k++)
				{
					Index index(nodes_recent[j][k].x + dirs[t][0], nodes_recent[j][k].y + dirs[t][1], nodes_recent[j][k].z + dirs[t][2]);
					Index flddir(index.x - nodes_index[j].x, index.y - nodes_index[j].y, index.z - nodes_index[j].z);
					if(nodes_pdirs[j] * Float3f(flddir.x, flddir.y, flddir.z) <= m_fAngleCos)
						continue;
					if(index.x < 0 || index.y < 0 || index.z < 0 || index.x >= m_iSlicesX || index.y >= m_iSlicesY || index.z >= m_iSlicesZ)
						continue;
					if(!m_vvvVoxels[index.x][index.y][index.z].isEmpty && !m_vvvVoxels[index.x][index.y][index.z].isUsed)
					{
						m_vvvVoxels[index.x][index.y][index.z].isUsed = true;
						tmp.push_back(index);
					}
				}
			}
			nodes_recent[j].clear();
			//copy(tmp.begin(), tmp.end(), nodes_recent.begin());
			for(int k = 0; k < tmp.size(); k++)
				nodes_recent[j].push_back(tmp[k]);
			// calculate the increasing ratio
			nodes_inc[j] = float(nodes_recent[j].size()) / nodes_prev_count[j];
			nodes_prev_count[j] = nodes_recent[j].size();
			// if greater than threshold, then push points into ret
			if(nodes_inc[j] >= m_fThreshold)
			{
				for(int k = 0; k < nodes_recent[j].size(); k++)
				{
					Index index(nodes_recent[j][k].x, nodes_recent[j][k].y, nodes_recent[j][k].z);
					for(int t = 0; t < m_vvvVoxels[index.x][index.y][index.z].indices.size(); t++)
					{
						ret[j].push_back((*m_pvPoints)[m_vvvVoxels[index.x][index.y][index.z].indices[t]]);
					}
				}
			}
			// else set voxels in the nodes_recent[j] to not used
			else
			{
				for(int k = 0; k < nodes_recent[j].size(); k++)
				{
					Index index(nodes_recent[j][k].x, nodes_recent[j][k].y, nodes_recent[j][k].z);
					m_vvvVoxels[index.x][index.y][index.z].isUsed = false;
				}
			}
		}
	}

	delete nodes_prev_count;
	delete nodes_inc;
}

void CVoxelModel::DividePoints(const vector<Float3f> &points, const Float3f &pt, vector<vector<Float3f> > & dirs_points)
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

void CVoxelModel::GetBranchDirections(const vector<vector<Float3f> > &dirs_points, float ratio, const Float3f &pt, vector<Float3f> &dirs)
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

float CVoxelModel::CalculateRadius(const vector<Float3f> &points, const Float3f &origin, const Float3f &dir)
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

float CVoxelModel::CalculateLength(const vector<Float3f> &points, const Float3f &origin, const Float3f &dir)
{
	float length;
	for(int i = 0; i < points.size(); i++)
	{
		float l = (points[i] - origin) * dir;
		length += l;
	}
	return length / points.size() * 2;
}

void CVoxelModel::FindNearestPoint(const Float3f &dstPoint, const vector<Float3f> &points, Float3f &out)
{
	float minDist = FLT_MAX;
	for(int i = 0; i < points.size(); i++)
	{
		if((points[i] - dstPoint).length() < minDist)
		{
			minDist = (points[i] - dstPoint).length();
			out = points[i]; 
		}
	}
}
