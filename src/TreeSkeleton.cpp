#include <assert.h>
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>
#include <GLUtils.h>
#include <GLTools.h>
#include <GLMatrixStack.h>
#include <GLGeometryTransform.h>
#include <float.h>
#include "TreeSkeleton.h"
#include "TreePointCloud.h"
#include "VoxelModel.h"

extern GLTriangleBatch gb_sphereBatch;
extern GLBatch gb_cubeBatch;
extern GLMatrixStack gb_modelViewMatrix;
extern GLGeometryTransform gb_transformPipeline;
extern GLint locMVP;
extern GLint locMV;
extern GLint locNM;

void SetVoxelColor();
void SetUsedVoxelColor();

CSkeletonNode::CSkeletonNode(float x, float y, float z, float radius)
	:m_pParent(NULL), m_pPrev(NULL), m_pNext(NULL), m_pChild(NULL), m_radius(radius), t_no(0)
{
	m_pos[0] = x;
	m_pos[1] = y;
	m_pos[2] = z;
}

CSkeletonNode::~CSkeletonNode()
{
	// a father should release all its children's resources
	CSkeletonNode *pNode = m_pChild;
	while(pNode)
	{
		CSkeletonNode *tmp = pNode->m_pNext;
		delete pNode;
		pNode = tmp;
	}
}

/* construct a tree from the node recursively
CSkeletonNode::CSkeletonNode(const CSkeletonNode &rhs)
	:m_pParent(NULL), m_pBrother(NULL), m_pChild(NULL), m_radius(rhs.m_radius)
{
	memcpy(m_pos, rhs.m_pos, 3);
	
}
*/

void CSkeletonNode::DisplayMesh(char *path, CSkeletonNode* cur)
{
	int i = 1;
	if(cur == this)
	{
		char npath[100] = "(";
		strcat(npath, path);
		strcat(npath, ")");
		printf("%s\n", npath);
	}
	else
		printf("%s\n", path);
	CSkeletonNode *pNode = m_pChild;
	while(pNode)
	{
		char tmp[100], num[5];
		strcpy(tmp, path);
		sprintf(num, "%d", i++);
		strcat(tmp, num); 
		pNode->DisplayMesh(tmp, cur);
		pNode = pNode->m_pNext;
	}
}

void CSkeletonNode::DisplayMesh(CSkeletonNode *pCurNode, int slices, SetColor General, SetColor Special)
{
	if(pCurNode == this)
		Special();
	// gb_modelViewMatrix transforms
	gb_modelViewMatrix.PushMatrix();
	gb_modelViewMatrix.Translate(m_pos[0], m_pos[1], m_pos[2]);
	gb_modelViewMatrix.Scale(m_radius, m_radius, m_radius);
	glUniformMatrix4fv(locMVP, 1, GL_FALSE, gb_transformPipeline.GetModelViewProjectionMatrix());
	glUniformMatrix4fv(locMV, 1, GL_FALSE, gb_transformPipeline.GetModelViewMatrix());
	glUniformMatrix3fv(locNM, 1, GL_FALSE, gb_transformPipeline.GetNormalMatrix());
	gb_sphereBatch.Draw();
	gb_modelViewMatrix.PopMatrix();
	CSkeletonNode *pNode = m_pChild;
	while(pNode)
	{
		GLTriangleBatch cylinderBatch;
		float length = sqrt( (m_pos[0] - pNode->m_pos[0]) * (m_pos[0] - pNode->m_pos[0])
						   + (m_pos[1] - pNode->m_pos[1]) * (m_pos[1] - pNode->m_pos[1])
						   + (m_pos[2] - pNode->m_pos[2]) * (m_pos[2] - pNode->m_pos[2]) );
		gltMakeCylinder(cylinderBatch, m_radius, pNode->m_radius, length, slices, 1); 
		gb_modelViewMatrix.PushMatrix();
		gb_modelViewMatrix.Translate(m_pos[0], m_pos[1], m_pos[2]);
		gb_modelViewMatrix.Rotate(180.0, (pNode->m_pos[0] - m_pos[0]) / 2.0,
								 (pNode->m_pos[1] - m_pos[1]) / 2.0,
								 (pNode->m_pos[2] - m_pos[2] + length) / 2.0);
		glUniformMatrix4fv(locMVP, 1, GL_FALSE, gb_transformPipeline.GetModelViewProjectionMatrix());
		glUniformMatrix4fv(locMV, 1, GL_FALSE, gb_transformPipeline.GetModelViewMatrix());
		glUniformMatrix3fv(locNM, 1, GL_FALSE, gb_transformPipeline.GetNormalMatrix());
		cylinderBatch.Draw();
		gb_modelViewMatrix.PopMatrix();
		
		pNode->DisplayMesh(pCurNode, slices, General, Special);
		pNode = pNode->m_pNext;
	}
	if(pCurNode == this)
		General();
}

void CSkeletonNode::DisplayPoint()
{
	CSkeletonNode *pChild = m_pChild;
	while(pChild)
	{
		float *from = m_pos;
		float *to = pChild->m_pos;
		GLdouble height = sqrt( (from[0] - to[0]) * (from[0] - to[0])
							   + (from[1] - to[1]) * (from[1] - to[1])
							   + (from[2] - to[2]) * (from[2] - to[2]) );
		M3DMatrix44f trans, rot, prod;
		m3dTranslationMatrix44(trans, from[0], from[1], from[2]);
		m3dRotationMatrix44(rot, M3D_PI, to[0] - from[0], to[1] - from[1], 
							to[2] - from[2] + height);
		m3dMatrixMultiply44(prod, trans, rot);
		M3DVector3f pt, pOut;
		int num = (int)(m_radius * pChild->m_radius * height * 1e7);
		if(num < 10) num = 10;
		GLBatch pointBatch;
		//M3DVector3f *verts = new M3DVector3f[num];
		pointBatch.Begin(GL_POINTS, num);
		for(int i = 0; i < num; i++)
		{
			float angle = random() / float(RAND_MAX) * M3D_2PI;
			pt[2] = random() / float(RAND_MAX) * height;
			float radius = m_radius + (pChild->m_radius - m_radius) * pt[2] / height;
			float ratio = random() / float(RAND_MAX);
			pt[0] = radius * ratio * cos(angle);
			pt[1] = radius * ratio * sin(angle);
			m3dTransformVector3(pOut, pt, prod);
			//memcpy(verts[i], pOut, sizeof(float) * 3);
			//pointBatch.CopyVertexData3f(verts);
			pointBatch.Vertex3fv(pOut);
		}
		//delete verts;
		pointBatch.End();
		pointBatch.Draw();
		pChild->DisplayPoint();

		pChild = pChild->m_pNext;
	}
}

void CSkeletonNode::Write(FILE *fout, int no)
{
	static int curId = 0;
	curId ++;
	fprintf(fout, "%d %f %f %f %f\n", no, m_pos[0], m_pos[1], m_pos[2], m_radius);
	CSkeletonNode *pNode = m_pChild;
	int id = curId;
	while(pNode)
	{
		pNode->Write(fout, id);
		pNode = pNode->m_pNext;
	}
}

unsigned CSkeletonNode::WritePoint(FILE *fout)
{
	assert(fout != NULL);
	unsigned numVerts = 0;
	CSkeletonNode *pChild = m_pChild;
	while(pChild)
	{
		float *from = m_pos;
		float *to = pChild->m_pos;
		GLdouble height = sqrt( (from[0] - to[0]) * (from[0] - to[0])
						 + (from[1] - to[1]) * (from[1] - to[1])
						 + (from[2] - to[2]) * (from[2] - to[2]) );
		M3DMatrix44f trans, rot, prod;
		m3dTranslationMatrix44(trans, from[0], from[1], from[2]);
		m3dRotationMatrix44(rot, M3D_PI, to[0] - from[0], to[1] - from[1], 
							to[2] - from[2] + height);
		m3dMatrixMultiply44(prod, trans, rot);
		M3DVector3f pt, pOut;
		int num = (int)(m_radius * pChild->m_radius * height * 1e7);
		if(num < 10) num = 10;
		numVerts += num;
		for(int i = 0; i < num; i++)
		{
			float angle = random() / float(RAND_MAX) * M3D_2PI;
			pt[2] = random() / float(RAND_MAX) * height;
			float radius = m_radius + (pChild->m_radius - m_radius) * pt[2] / height;
			float ratio = random() / float(RAND_MAX);
			pt[0] = radius * ratio * cos(angle);
			pt[1] = radius * ratio * sin(angle);
			m3dTransformVector3(pOut, pt, prod);
			fprintf(fout, "%f %f %f\n", pOut[0], pOut[1], pOut[2]);
		}
		numVerts += pChild->WritePoint(fout);

		pChild = pChild->m_pNext;
	}
	return numVerts;
}

CTreeSkeleton::CTreeSkeleton()
	:m_pRoot(NULL), m_pCurNode(NULL), m_pPointCloud(new CTreePointCloud), 
	m_pVoxelModel(new CVoxelModel), m_slices(20)
{
}

CTreeSkeleton::~CTreeSkeleton()
{
}

// insert a child node to the current node
void CTreeSkeleton::Insert(float x, float y, float z, float radius)
{
	if(radius < 0)
	{
		radius = m_pCurNode->m_radius;
		x = x * 2 * radius + m_pCurNode->m_pos[0];
		y = y * 2 * radius + m_pCurNode->m_pos[1];
		z = z * 2 * radius + m_pCurNode->m_pos[2];
	}
	CSkeletonNode *pNode = new CSkeletonNode(x, y, z, radius);
	// if root node doesn't exist, create one.
	if(m_pRoot == NULL)
	{
		m_pRoot = pNode;
		m_pCurNode = pNode;
	}
	// just link the new node to the last child of the current node
	else
	{
		pNode->m_pParent = m_pCurNode;
		if(m_pCurNode->m_pChild)
		{
			CSkeletonNode *cur = m_pCurNode->m_pChild;
			while(cur->m_pNext)
			{
				cur = cur->m_pNext;
			}
			cur->m_pNext = pNode;
			pNode->m_pPrev = cur;
		}
		else
		{
			m_pCurNode->m_pChild = pNode;
		}
		m_pCurNode = pNode;
		if(m_pCurNode->m_radius > m_pCurNode->m_pParent->m_radius)
			m_pCurNode->m_radius = m_pCurNode->m_pParent->m_radius;
	}
}

// delete a subtree, mode 1 contains the current node,
// while mode 0 doesn't
void CTreeSkeleton::Delete(unsigned mode)
{
	if(m_pCurNode == NULL) return;
	if(mode == 0)
	{
		CSkeletonNode *pNode = m_pCurNode->m_pChild;
		while(pNode)
		{
			CSkeletonNode *tmp = pNode->m_pNext;
			delete pNode;
			pNode = tmp;
		}
		m_pCurNode->m_pChild = NULL;
	}
	if(mode == 1)
	{
		if(m_pCurNode == m_pRoot)
			m_pRoot = NULL;
		CSkeletonNode *tmpPrev = m_pCurNode->m_pPrev;
		CSkeletonNode *tmpNext = m_pCurNode->m_pNext;
		CSkeletonNode *tmpParent = m_pCurNode->m_pParent;
		delete m_pCurNode;
		m_pCurNode = tmpParent;
		// if the parent node exists, we should
		// reorganize its children since one is deleted
		if(m_pCurNode)
		{
			// if the first child is deleted
			if(tmpPrev == NULL)
				m_pCurNode->m_pChild = tmpNext;
			else
				tmpPrev->m_pNext = tmpNext;
			if(tmpNext != NULL)
				tmpNext->m_pPrev = tmpPrev;
		}
	}
}

void CTreeSkeleton::Descent()
{
	if(m_pCurNode == NULL)
		return;
	if(m_pCurNode->m_pChild != NULL)
		m_pCurNode = m_pCurNode->m_pChild;
}

void CTreeSkeleton::Ascent()
{
	if(m_pCurNode == NULL)
		return;
	if(m_pCurNode->m_pParent != NULL)
		m_pCurNode = m_pCurNode->m_pParent;
}

void CTreeSkeleton::Next()
{
	if(m_pCurNode == NULL)
		return;
	if(m_pCurNode->m_pNext)
		m_pCurNode = m_pCurNode->m_pNext;
	else if(m_pCurNode->m_pParent)
		m_pCurNode = m_pCurNode->m_pParent->m_pChild;
}

void CTreeSkeleton::Previous()
{
	if(m_pCurNode == NULL)
		return;
	if(m_pCurNode->m_pPrev)
		m_pCurNode = m_pCurNode->m_pPrev;
	else if(m_pCurNode->m_pParent)
	{
		CSkeletonNode *pNode = m_pCurNode;
		while(pNode->m_pNext)
		{
			pNode = pNode->m_pNext;
		}
		m_pCurNode = pNode;
	}
}

void CTreeSkeleton::Display(SetColor General, SetColor Special, unsigned mode)
{
	/* for terminal testing
	if(m_pRoot)
	{
		char arr[] = "1";
		m_pRoot->DisplayMesh(arr, m_pCurNode);
	}
	*/
	switch(mode)
	{
	case 0:		// for mesh view
		if(m_pRoot)
		{
			m_pRoot->DisplayMesh(m_pCurNode, m_slices, General, Special);
		}
		break;
	case 1:		// for point view
		if(m_pRoot)
		{
			srandom(0);
			m_pRoot->DisplayPoint();
		}
		break;
	}
}

void CTreeSkeleton::MoveCurNode(float vector[3])
{
	if(m_pCurNode)
	{
		m_pCurNode->m_pos[0] += vector[0];
		m_pCurNode->m_pos[1] += vector[1];
		m_pCurNode->m_pos[2] += vector[2];
	}
}

void CTreeSkeleton::ChangeRadius(float increase)
{
	if(m_pCurNode)
	{
		m_pCurNode->m_radius += increase;
	}
}

void CTreeSkeleton::Save(const char *filename, unsigned mode)
{
	if(m_pRoot == NULL) return;
	FILE *fout = fopen(filename, "w");
	if(fout == NULL) return;

	switch(mode)
	{
	case 0:		// save in skeleton mode
		m_pRoot->Write(fout, 0);
		break;
	case 1:		// save in point cloud mode
		{
			fprintf(fout,
					"ply\n"
					"format ascii 1.0\n"
					"element vertex ");
			long v = ftell(fout);
			fprintf(fout,
					"           \n"
					"property float x\n"
					"property float y\n"
					"property float z\n"
					"end_header\n");
			srandom(0);
			unsigned count = m_pRoot->WritePoint(fout);
			fseek(fout, v, SEEK_SET);
			fprintf(fout, "%u", count);
		
		}
		break;
	}

	fclose(fout);
}

void CTreeSkeleton::Load(const char *filename)
{
	// first of all we should delete all nodes
	// and release all the resoureces
	m_pCurNode = m_pRoot;
	Delete(1);

	int parentId, curId;
	float x, y, z, r;
	// then we read data to fill the tree
	FILE *fin = fopen(filename, "r");
	curId = 0;
	while(fscanf(fin, "%d %f %f %f %f", &parentId, &x, &y, &z, &r) == 5)
	{
		curId ++;
		while(m_pCurNode && m_pCurNode->t_no != parentId && m_pCurNode->m_pParent)
		{
			m_pCurNode = m_pCurNode->m_pParent;
		}
		if(!m_pCurNode || m_pCurNode->t_no == parentId)
		{
			this->Insert(x, y, z, r);
			m_pCurNode->t_no = curId;
		}
		else
			return;		// error
	}
	m_pCurNode = m_pRoot;
	fclose(fin);
}

// execute simplification on the current node
// and all its sub nodes
void CTreeSkeleton::Simplify(double max_angle, bool bCurNode)
{
	assert(max_angle > 0.0);
	// if it's null or an end node, return.
	if(!m_pCurNode)
		return;
	if(!m_pRoot)
		return;
	if(bCurNode == true)
	{
		CSkeletonNode *pChild = m_pCurNode->m_pChild;
		while(pChild)
		{
			pChild->Simplify(max_angle);
			pChild = pChild->m_pNext;
		}
	}
	else
	{
		m_pRoot->Simplify(max_angle);
		m_pCurNode = m_pRoot;
	}
}

void CSkeletonNode::Simplify(double max_angle)
{
	if(!m_pChild)
		return;
	if(m_pParent && m_pChild->m_pNext == NULL && m_pChild->m_pPrev == NULL)
	{
		double u[3] = {m_pos[0] - m_pParent->m_pos[0],
			m_pos[1] - m_pParent->m_pos[1],
			m_pos[2] - m_pParent->m_pos[2]};
		double v[3] = {m_pChild->m_pos[0] - m_pos[0],
			m_pChild->m_pos[1] - m_pos[1],
			m_pChild->m_pos[2] - m_pos[2]};
		double angle = u[0] * v[0] + u[1] * v[1] + u[2] * v[2];
		angle /= sqrt(u[0] * u[0] + u[1] * u[1] + u[2] * u[2])
			* sqrt(v[0] * v[0] + v[1] * v[1] + v[2] * v[2]);
		angle = acos(angle);
		if(angle < max_angle)
		{
			// do simplification things
			m_pChild->m_pParent = m_pParent;
			if(m_pPrev == NULL)
			{
				m_pParent->m_pChild = m_pChild;
				m_pChild->m_pPrev = NULL;
			}
			else
			{
				m_pPrev->m_pNext = m_pChild;
				m_pChild->m_pPrev = m_pPrev;
			}
			m_pChild->m_pNext = m_pNext;
			if(m_pNext)
				m_pNext->m_pPrev = m_pChild;
		}
	}
	CSkeletonNode *pChild = m_pChild;
	while(pChild)
	{
		CSkeletonNode *tmp = pChild->m_pNext;
		pChild->Simplify(max_angle);
		pChild = tmp;
	}
}

void CTreeSkeleton::Select(M3DVector3f pt)
{
	if(m_pRoot)
		m_pRoot->Select(pt, &m_pCurNode);
}

void CSkeletonNode::Select(M3DVector3f pt, CSkeletonNode **ppCurNode)
{
	static bool bFound = false;
	if(bFound == true)
		return;
	CSkeletonNode *pChild = m_pChild;
	while(pChild)
	{
		if(IsPointInFreeCylinder(m_pos, m_radius, pChild->m_pos, pChild->m_radius, pt))
		{
			*ppCurNode = this;
			bFound = true;
			return;
		}
		pChild->Select(pt, ppCurNode);
		pChild = pChild->m_pNext;
	}
}

void CSkeletonNode::CheckRange(M3DVector3f maxRange, M3DVector3f minRange)
{
	for(int i = 0; i < 3; i++)
	{
		if(m_pos[i] + m_radius > maxRange[i])
			maxRange[i] = m_pos[i] + m_radius;
		if(m_pos[i] - m_radius < minRange[i])
			minRange[i] = m_pos[i] - m_radius;
	}
	CSkeletonNode *pChild = m_pChild;
	while(pChild)
	{
		pChild->CheckRange(maxRange, minRange);
		pChild = pChild->m_pNext;
	}
}

// display the current voxel model
void CTreeSkeleton::DisplayVoxel()
{
	if(!m_pRoot)
		return;
	float *maxRange = m_pPointCloud->m_vMaxRange;
	float *minRange = m_pPointCloud->m_vMinRange;
	int nums[3];
	nums[0] = m_pVoxelModel->m_iSlicesX;
	nums[1] = m_pVoxelModel->m_iSlicesY;
	nums[2] = m_pVoxelModel->m_iSlicesZ;
	float unitLength = (maxRange[0] - minRange[0]) / nums[0];
	gb_modelViewMatrix.PushMatrix();
	for(int i = 0; i < nums[0]; i++)
	{
		for(int j = 0; j < nums[1]; j++)
		{
			for(int k = 0; k < nums[2]; k++)
			{
				if(m_pVoxelModel->m_vvvVoxels[i][j][k].isEmpty)
					continue;
				if(m_pVoxelModel->m_vvvVoxels[i][j][k].isUsed)
				{
					SetUsedVoxelColor();
					glPolygonMode(GL_FRONT, GL_FILL);
				}
				gb_modelViewMatrix.PushMatrix();
				gb_modelViewMatrix.Translate(minRange[0] + unitLength / 2 + unitLength * i, 
											 minRange[1] + unitLength / 2 + unitLength * j, 
											 minRange[2] + unitLength / 2 + unitLength * k);
				gb_modelViewMatrix.Scale(unitLength, unitLength, unitLength);

				glUniformMatrix4fv(locMVP, 1, GL_FALSE, gb_transformPipeline.GetModelViewProjectionMatrix());
				glUniformMatrix4fv(locMV, 1, GL_FALSE, gb_transformPipeline.GetModelViewMatrix());
				glUniformMatrix3fv(locNM, 1, GL_FALSE, gb_transformPipeline.GetNormalMatrix());
				gb_cubeBatch.Draw();			
				gb_modelViewMatrix.PopMatrix();
				if(m_pVoxelModel->m_vvvVoxels[i][j][k].isUsed)
				{
					SetVoxelColor();
					glPolygonMode(GL_FRONT, GL_LINE);
				}
			}
		}
	}
	gb_modelViewMatrix.PopMatrix();
}

void CTreeSkeleton::LinearRadius(float ratio)
{
	if(m_pRoot == NULL)
		return;
	m_pRoot->LinearRadius(ratio);
}

void CTreeSkeleton::SquareRadius()
{
	if(m_pRoot == NULL)
		return;
	m_pRoot->SquareRadius();
}

void CSkeletonNode::LinearRadius(float ratio)
{
	CSkeletonNode *pChild = m_pChild;
	int nChild = 0;
	while(pChild)
	{
		nChild ++;
		pChild = pChild->m_pNext;
	}
	if(nChild == 0)
		return;
	if(nChild > 1)
	{
		pChild = m_pChild;
		while(pChild)
		{
			pChild->m_radius = m_radius * ratio;
			pChild->LinearRadius(ratio);
			pChild = pChild->m_pNext;
		}
	}
	else
	{
		m_pChild->m_radius = m_radius;
		m_pChild->LinearRadius(ratio);
	}
}

void CSkeletonNode::SquareRadius()
{
	CSkeletonNode *pChild = m_pChild;
	int nChild = 0;
	while(pChild)
	{
		nChild ++;
		pChild = pChild->m_pNext;
	}
	if(nChild == 0)
		return;
	pChild = m_pChild;
	while(pChild)
	{
		pChild->m_radius = m_radius / sqrt((double)nChild);
		pChild->SquareRadius();
		pChild = pChild->m_pNext;
	}
}

void CTreeSkeleton::LoadPointCloud(const char *filename)
{
	//m_pCurNode = m_pRoot;
	//Delete(0);
	m_pPointCloud->Load(filename);
}

void CTreeSkeleton::LoadVoxelModel(int nSlicesX)
{
	m_pVoxelModel->IndexPoints(m_pPointCloud, nSlicesX);
	//m_pVoxelModel->ExtractSkeleton(this);
}

void CTreeSkeleton::ExtractSkeleton(unsigned mode)
{
	m_pVoxelModel->ExtractSkeleton(this, mode);
}
