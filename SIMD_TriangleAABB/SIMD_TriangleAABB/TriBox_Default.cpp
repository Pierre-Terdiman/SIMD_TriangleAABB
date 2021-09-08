#include "stdafx.h"

/*
	Version1 based on original code: http://fileadmin.cs.lth.se/cs/Personal/Tomas_Akenine-Moller/code/tribox.txt

	Version2 based on "faster code": http://fileadmin.cs.lth.se/cs/Personal/Tomas_Akenine-Moller/code/tribox2.txt

	For some reason on my machine the original version is clearly faster.
*/

//#define USE_VERSION2

/********************************************************/
/* AABB-triangle overlap test code                      */
/* by Tomas Akenine-M?r									*/
/* Function: int triBoxOverlap(float boxcenter[3],      */
/*          float boxhalfsize[3],float triverts[3][3]); */
/* History:                                             */
/*   2001-03-05: released the code in its first version */
/*   2001-06-18: changed the order of the tests, faster */
/*                                                      */
/* Acknowledgement: Many thanks to Pierre Terdiman for  */
/* suggestions and discussions on how to optimize code. */
/* Thanks to David Hunt for finding a ">="-bug!         */
/********************************************************/

#ifdef USE_VERSION2
#define CROSS(dest,v1,v2)		\
	dest.x=v1.y*v2.z-v1.z*v2.y;	\
	dest.y=v1.z*v2.x-v1.x*v2.z;	\
	dest.z=v1.x*v2.y-v1.y*v2.x; 

#define DOT(v1,v2) (v1.x*v2.x+v1.y*v2.y+v1.z*v2.z)

#define FINDMINMAX(x0, x1, x2, minimum, maximum)	\
	minimum = selectMin(x0, x1);					\
	maximum = selectMax(x0, x1);					\
	minimum = selectMin(minimum, x2);				\
	maximum = selectMax(maximum, x2);

static __forceinline int planeBoxOverlap(const Point& normal, float d, const Point& maxbox)
{
	Point vmin,vmax;

	if(normal.x>0.0f)
	{
		vmin.x = -maxbox.x;
		vmax.x = maxbox.x;
	}
	else
	{
		vmin.x = maxbox.x;
		vmax.x = -maxbox.x;
	}

	if(normal.y>0.0f)
	{
		vmin.y = -maxbox.y;
		vmax.y = maxbox.y;
	}
	else
	{
		vmin.y = maxbox.y;
		vmax.y = -maxbox.y;
	}

	if(normal.z>0.0f)
	{
		vmin.z = -maxbox.z;
		vmax.z = maxbox.z;
	}
	else
	{
		vmin.z = maxbox.z;
		vmax.z = -maxbox.z;
	}

	if((normal|vmin) + d >  0.0f) return 0;
	if((normal|vmax) + d >= 0.0f) return 1;
	return 0;
}

/*======================== X-tests ========================*/
#define AXISTEST_X01(a, b, fa, fb)				\
	p0 = a*v0.y - b*v0.z;						\
	p2 = a*v2.y - b*v2.z;						\
	minimum = selectMin(p0, p2);				\
	maximum = selectMax(p0, p2);				\
	rad = fa * extents.y + fb * extents.z;		\
	if(minimum>rad || maximum<-rad) return 0;

#define AXISTEST_X2(a, b, fa, fb)				\
	p0 = a*v0.y - b*v0.z;						\
	p1 = a*v1.y - b*v1.z;						\
	minimum = selectMin(p0, p1);				\
	maximum = selectMax(p0, p1);				\
	rad = fa * extents.y + fb * extents.z;		\
	if(minimum>rad || maximum<-rad) return 0;

/*======================== Y-tests ========================*/
#define AXISTEST_Y02(a, b, fa, fb)				\
	p0 = -a*v0.x + b*v0.z;						\
	p2 = -a*v2.x + b*v2.z;						\
	minimum = selectMin(p0, p2);				\
	maximum = selectMax(p0, p2);				\
	rad = fa * extents.x + fb * extents.z;		\
	if(minimum>rad || maximum<-rad) return 0;

#define AXISTEST_Y1(a, b, fa, fb)				\
	p0 = -a*v0.x + b*v0.z;						\
	p1 = -a*v1.x + b*v1.z;						\
	minimum = selectMin(p0, p1);				\
	maximum = selectMax(p0, p1);				\
	rad = fa * extents.x + fb * extents.z;		\
	if(minimum>rad || maximum<-rad) return 0;

/*======================== Z-tests ========================*/
#define AXISTEST_Z12(a, b, fa, fb)				\
	p1 = a*v1.x - b*v1.y;						\
	p2 = a*v2.x - b*v2.y;						\
	minimum = selectMin(p1, p2);				\
	maximum = selectMax(p1, p2);				\
	rad = fa * extents.x + fb * extents.y;		\
	if(minimum>rad || maximum<-rad) return 0;

#define AXISTEST_Z0(a, b, fa, fb)				\
	p0 = a*v0.x - b*v0.y;						\
	p1 = a*v1.x - b*v1.y;						\
	minimum = selectMin(p0, p1);				\
	maximum = selectMax(p0, p1);				\
	rad = fa * extents.x + fb * extents.y;		\
	if(minimum>rad || maximum<-rad) return 0;

int OverlapTriangleBox_Default(const Point& boxcenter, const Point& extents, const Point& tp0, const Point& tp1, const Point& tp2)
{
	/*    use separating axis theorem to test overlap between triangle and box */
	/*    need to test for overlap in these directions: */
	/*    1) the {x,y,z}-directions (actually, since we use the AABB of the triangle */
	/*       we do not even need to test these) */
	/*    2) normal of the triangle */
	/*    3) crossproduct(edge from tri, {x,y,z}-directin) */
	/*       this gives 3x3=9 more tests */

	// This is the fastest branch on Sun - move everything so that the boxcenter is in (0,0,0)
	const Point v0 = tp0 - boxcenter;
	const Point v1 = tp1 - boxcenter;
	const Point v2 = tp2 - boxcenter;

	// compute triangle edges
	const Point e0 = v1 - v0;	// tri edge 0
	const Point e1 = v2 - v1;	// tri edge 1
	const Point e2 = v0 - v2;	// tri edge 2

	float minimum,maximum,rad,p0,p1,p2;  

	// Bullet 3: test the 9 tests first (this was faster)
	float fex = fabsf(e0.x); 
	float fey = fabsf(e0.y);
	float fez = fabsf(e0.z);
	AXISTEST_X01(e0.z, e0.y, fez, fey);
	AXISTEST_Y02(e0.z, e0.x, fez, fex);
	AXISTEST_Z12(e0.y, e0.x, fey, fex);

	fex = fabsf(e1.x);
	fey = fabsf(e1.y);
	fez = fabsf(e1.z);
	AXISTEST_X01(e1.z, e1.y, fez, fey);
	AXISTEST_Y02(e1.z, e1.x, fez, fex);
	AXISTEST_Z0(e1.y, e1.x, fey, fex);

	fex = fabsf(e2.x);
	fey = fabsf(e2.y);
	fez = fabsf(e2.z);
	AXISTEST_X2(e2.z, e2.y, fez, fey);
	AXISTEST_Y1(e2.z, e2.x, fez, fex);
	AXISTEST_Z12(e2.y, e2.x, fey, fex);

	// Bullet 1:
	//  first test overlap in the {x,y,z}-directions
	//  find minimum, maximum of the triangle each direction, and test for overlap in
	//  that direction -- this is equivalent to testing a minimal AABB around
	//  the triangle against the AABB

	// test in X-direction
	FINDMINMAX(v0.x, v1.x, v2.x, minimum, maximum);
	if(minimum>extents.x || maximum<-extents.x)
		return 0;

	// test in Y-direction
	FINDMINMAX(v0.y, v1.y, v2.y, minimum, maximum);
	if(minimum>extents.y || maximum<-extents.y)
		return 0;

	// test in Z-direction
	FINDMINMAX(v0.z, v1.z, v2.z, minimum, maximum);
	if(minimum>extents.z || maximum<-extents.z)
		return 0;

	// Bullet 2:
	//  test if the box intersects the plane of the triangle
	//  compute plane equation of triangle: normal*x+d=0
	Point normal;
	CROSS(normal,e0,e1);
	const float d=-DOT(normal,v0);	// plane eq: normal.x+d=0
	if(!planeBoxOverlap(normal, d, extents))
		return 0;

	return 1;	// box and triangle overlaps
}

#else

//! This macro quickly finds the min & max values among 3 variables
#define FINDMINMAX(x0, x1, x2, min, max)	\
	min = max = x0;							\
	if(x1<min) min=x1;						\
	if(x1>max) max=x1;						\
	if(x2<min) min=x2;						\
	if(x2>max) max=x2;

static __forceinline int planeBoxOverlap(const Point& normal, const float d, const Point& maxbox)
{
	Point vmin, vmax;
	for(int q=0;q<=2;q++)
	{
		if(normal[q]>0.0f)	{ vmin[q]=-maxbox[q]; vmax[q]=maxbox[q]; }
		else				{ vmin[q]=maxbox[q]; vmax[q]=-maxbox[q]; }
	}
	if((normal|vmin)+d>0.0f) return 0;
	if((normal|vmax)+d>=0.0f) return 1;

	return 0;
}

#define AXISTEST_X01(a, b, fa, fb)							\
	min = a*v0.y - b*v0.z;									\
	max = a*v2.y - b*v2.z;									\
	if(min>max) {const float tmp=max; max=min; min=tmp;	}	\
	rad = fa * extents.y + fb * extents.z;					\
	if(min>rad || max<-rad) return 0;

#define AXISTEST_X2(a, b, fa, fb)							\
	min = a*v0.y - b*v0.z;									\
	max = a*v1.y - b*v1.z;									\
	if(min>max) {const float tmp=max; max=min; min=tmp;	}	\
	rad = fa * extents.y + fb * extents.z;					\
	if(min>rad || max<-rad) return 0;

#define AXISTEST_Y02(a, b, fa, fb)							\
	min = b*v0.z - a*v0.x;									\
	max = b*v2.z - a*v2.x;									\
	if(min>max) {const float tmp=max; max=min; min=tmp;	}	\
	rad = fa * extents.x + fb * extents.z;					\
	if(min>rad || max<-rad) return 0;

#define AXISTEST_Y1(a, b, fa, fb)							\
	min = b*v0.z - a*v0.x;									\
	max = b*v1.z - a*v1.x;									\
	if(min>max) {const float tmp=max; max=min; min=tmp;	}	\
	rad = fa * extents.x + fb * extents.z;					\
	if(min>rad || max<-rad) return 0;

#define AXISTEST_Z12(a, b, fa, fb)							\
	min = a*v1.x - b*v1.y;									\
	max = a*v2.x - b*v2.y;									\
	if(min>max) {const float tmp=max; max=min; min=tmp;	}	\
	rad = fa * extents.x + fb * extents.y;					\
	if(min>rad || max<-rad) return 0;

#define AXISTEST_Z0(a, b, fa, fb)							\
	min = a*v0.x - b*v0.y;									\
	max = a*v1.x - b*v1.y;									\
	if(min>max) {const float tmp=max; max=min; min=tmp;	}	\
	rad = fa * extents.x + fb * extents.y;					\
	if(min>rad || max<-rad) return 0;

int OverlapTriangleBox_Default(const Point& boxcenter, const Point& extents, const Point& tp0, const Point& tp1, const Point& tp2)
{
	// This is the fastest branch on Sun - move everything so that the boxcenter is in (0,0,0)
	const Point v0 = tp0 - boxcenter;
	const Point v1 = tp1 - boxcenter;
	const Point v2 = tp2 - boxcenter;

	// use separating axis theorem to test overlap between triangle and box 
	// need to test for overlap in these directions: 
	// 1) the {x,y,z}-directions (actually, since we use the AABB of the triangle 
	//    we do not even need to test these) 
	// 2) normal of the triangle 
	// 3) crossproduct(edge from tri, {x,y,z}-directin) 
	//    this gives 3x3=9 more tests 

	{
		// First, test overlap in the {x,y,z}-directions
		float min,max;
		// Find min, max of the triangle in x-direction, and test for overlap in X
		FINDMINMAX(v0.x, v1.x, v2.x, min, max);
		if(min>extents.x || max<-extents.x)
			return 0;

		FINDMINMAX(v0.y, v1.y, v2.y, min, max);
		if(min>extents.y || max<-extents.y)
			return 0;

		FINDMINMAX(v0.z, v1.z, v2.z, min, max);
		if(min>extents.z || max<-extents.z)
			return 0;
	}

	// 2) Test if the box intersects the plane of the triangle
	// compute plane equation of triangle: normal*x+d=0
	// ### could be precomputed since we use the same leaf triangle several times
	const Point e0 = v1 - v0;
	const Point e1 = v2 - v1;
	if(1)
	{
		const Point normal = e0 ^ e1;
		const float d = -normal|v0;
		if(!planeBoxOverlap(normal, d, extents))
			return 0;
	}

	// 3) "Class III" tests - here we always do full tests since the box is a primitive (not a BV)
	{
		// compute triangle edges
		// - edges lazy evaluated to take advantage of early exits
		// - fabs precomputed (half less work, possible since extents are always >0)
		// - customized macros to take advantage of the null component
		// - axis vector discarded, possibly saves useless movs
		float rad;
		float min, max;

		const float fey0 = fabsf(e0.y);
		const float fez0 = fabsf(e0.z);
		AXISTEST_X01(e0.z, e0.y, fez0, fey0);
		const float fex0 = fabsf(e0.x);
		AXISTEST_Y02(e0.z, e0.x, fez0, fex0);
		AXISTEST_Z12(e0.y, e0.x, fey0, fex0);

		const float fey1 = fabsf(e1.y);
		const float fez1 = fabsf(e1.z);
		AXISTEST_X01(e1.z, e1.y, fez1, fey1);
		const float fex1 = fabsf(e1.x);
		AXISTEST_Y02(e1.z, e1.x, fez1, fex1);
		AXISTEST_Z0(e1.y, e1.x, fey1, fex1);

		const Point e2 = v0 - v2;
		const float fey2 = fabsf(e2.y);
		const float fez2 = fabsf(e2.z);
		AXISTEST_X2(e2.z, e2.y, fez2, fey2);
		const float fex2 = fabsf(e2.x);
		AXISTEST_Y1(e2.z, e2.x, fez2, fex2);
		AXISTEST_Z12(e2.y, e2.x, fey2, fex2);
	}
	return 1;
}

#endif