/* -.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.

* File Name :

* Purpose :

* Creation Date : 10-05-2013

* Last Modified : Mon 27 May 2013 02:21:33 PM CST

* Created By : Philip Zhang 

_._._._._._._._._._._._._._._._._._._._._.*/

#ifndef __GLUTILS_INCLUDE__
#define __GLUTILS_INCLUDE__
#include <math3d.h>

// click at (x,y) in viewport and get a hit point in world coordinate system
// return true if success, false if no point is hit.
bool HitTest(int x, int y, M3DVector3f out, M3DMatrix44f modelview, M3DMatrix44f projection, unsigned selSize);

// test whether a point is in a free cylinder
bool IsPointInFreeCylinder(M3DVector3f p0, 
						   float r0,
						   M3DVector3f p1,
						   float r1,
						   M3DVector3f pt/*point to test*/);

#endif
