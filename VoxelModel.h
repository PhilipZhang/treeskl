/* -.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.

* File Name : VoxelModel.h

* Purpose :

* Creation Date : 22-05-2013

* Last Modified : Wed 22 May 2013 09:18:35 PM CST

* Created By : Philip Zhang 

_._._._._._._._._._._._._._._._._._._._._.*/

#ifndef __VOXELMODEL_INCLUDE__
#define __VOXELMODEL_INCLUDE__

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
public:
	vector<vector<vector<Voxel> > > *GetVoxelModel() const;
protected:
	vector<vector<vector<Voxel> > > m_vvvVoxels;
	int m_iSlicesX, m_iSlicesY, m_iSlicesZ;
	int m_nSlicesX;
};


#endif
