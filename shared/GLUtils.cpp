/* -.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.

* File Name :

* Purpose :

* Creation Date : 10-05-2013

* Last Modified : Mon 27 May 2013 08:36:13 PM CST

* Created By : Philip Zhang 

_._._._._._._._._._._._._._._._._._._._._.*/

#include <GL/gl.h>
#include <GL/glut.h>
#include <math3d.h>
#include "GLUtils.h"

bool HitTest(int x, int y, M3DVector3f out, M3DMatrix44f modelview, M3DMatrix44f projection, unsigned selSize)
{
	GLint viewport[4];
	GLdouble modelviewd[16];
	GLdouble projectiond[16];
	GLfloat fDepthMax, winX, winY, winZ;
	GLdouble posX, posY, posZ;

	//glGetDoublev(GL_MODELVIEW_MATRIX, modelview);
	//glGetDoublev(GL_PROJECTION_MATRIX, projection);
	glGetFloatv(GL_DEPTH_CLEAR_VALUE, &fDepthMax);
	glGetIntegerv(GL_VIEWPORT, viewport);
	for(int i = 0; i < 16; i++)
	{
		modelviewd[i] = (double)modelview[i];
		projectiond[i] = (double)projectiond[i];
	}

	winX = (float)x;
	winY = (float)y;
	glReadBuffer(GL_FRONT);
	unsigned char imageData[3];
	glReadPixels(x, viewport[3] - y, 1, 1, GL_RGB, GL_UNSIGNED_BYTE, imageData);
	float z[selSize * selSize];
	int min_dep_loc = 0;
	for(unsigned s = 1; s <= selSize; s += 2)
	{
		glReadPixels(x - s / 2, viewport[3] - y - s / 2, s, s, GL_DEPTH_COMPONENT, GL_FLOAT, z);
		min_dep_loc = 0;
		for(int i = 1; i < s * s; i++)
		{
			if(z[i] < z[0]) min_dep_loc = i;
		}
		if(z[min_dep_loc] < fDepthMax)
		{
			winX = x - s / 2 + min_dep_loc % s;
			winY = viewport[3] - y - s / 2 + min_dep_loc / s;
			break;
		}
	}
	glReadBuffer(GL_BACK);
	if(z[min_dep_loc] >= fDepthMax) return 0;

	bool bSuccess = gluUnProject(winX, winY, z[min_dep_loc], modelviewd, projectiond, viewport, &posX, &posY, &posZ);
	out[0] = posX;
	out[1] = posY;
	out[2] = posZ;
	return bSuccess;

}

bool IsPointInFreeCylinder(M3DVector3f p0, float r0, M3DVector3f p1, float r1, M3DVector3f pt)
{
	// the point should be between the base and top
	M3DVector3f dir0, dir1;
	m3dSubtractVectors3(dir0, pt, p0);
	m3dSubtractVectors3(dir1, pt, p1);
	if(m3dDotProduct3(dir0, dir1) > 0)
		return false;
	// base radius should be larger than top radius
	float *from, *to;
	float base, top;
	if(r0 >= r1)
	{
		from = p0;
		to = p1;
		base = r0;
		top = r1;
	}
	else
	{
		from = p1;
		to = p0;
		base = r1;
		top = r0;
	}
	// First work out the distance from pt to line to - from
	// suppose n = (to - from).identity()
	// the middle axis line is x = from + t * n
	// the distance is |(from - pt) - ((from - pt) * n) * n|
	M3DVector3f n, sub, prod, dist;
	m3dSubtractVectors3(n, to, from);
	m3dNormalizeVector3(n);
	m3dSubtractVectors3(sub, from, pt);
	float dot = m3dDotProduct3(sub, n);
	m3dScaleVector3(n, dot);
	m3dSubtractVectors3(dist, sub, n);
	float distance = m3dGetVectorLength3(dist);
	if(distance > base || distance < top)
		return false;
	// calculate Pr
	M3DVector3f pr, res;
	pr[0] = distance / (top - base) * (to[0] - from[0]) + from[0];
	pr[1] = distance / (top - base) * (to[1] - from[1]) + from[1];
	pr[2] = distance / (top - base) * (to[2] - from[2]) + from[2];
	m3dSubtractVectors3(res, pt, pr);
	m3dSubtractVectors3(n, to, from);
	dot = m3dDotProduct3(res, n);
	if(dot > 0)
		return false;
	return true;
}
