#pragma once

#include "targetver.h"

#include <stdio.h>
#include <stdlib.h>
#include <tchar.h>
#include <math.h>
#include <float.h>

#include <xmmintrin.h>
#include <emmintrin.h>

__forceinline float selectMin(float a, float b)		{ return a<b ? a : b;	}
__forceinline float selectMax(float a, float b)		{ return a>b ? a : b;	}

struct Point
{
	float x,y,z;

	__forceinline			Point()														{}
	__forceinline			Point(float _x, float _y, float _z) : x(_x), y(_y), z(_z)	{}

	__forceinline	float	operator|(const Point& p)	const	{ return x*p.x + y*p.y + z*p.z;				}

	__forceinline	Point	operator^(const Point& p)	const
							{
								return Point(
								y * p.z - z * p.y,
								z * p.x - x * p.z,
								x * p.y - y * p.x );
							}

	__forceinline	Point	operator-(const Point& p)	const	{ return Point(x - p.x, y - p.y, z - p.z);	}
	__forceinline	Point	operator-()					const	{ return Point(-x, -y, -z);					}

	__forceinline	void	operator*=(float s)					{ x *= s; y *= s; z *= s;					}

	__forceinline			operator	const	float*() const	{ return &x; }
	__forceinline			operator			float*()		{ return &x; }

	__forceinline	void	Normalize()
							{
								float M = x*x + y*y + z*z;
								if(M)
								{
									M = 1.0f / sqrtf(M);
									x *= M;
									y *= M;
									z *= M;
								}
							}

};