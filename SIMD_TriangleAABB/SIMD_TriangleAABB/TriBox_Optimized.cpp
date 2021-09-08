#include "stdafx.h"

__forceinline __m128 CrossV(const __m128 a, const __m128 b)
{
	const __m128 r1 = _mm_shuffle_ps(a, a, _MM_SHUFFLE(3, 1, 0, 2));
	const __m128 r2 = _mm_shuffle_ps(b, b, _MM_SHUFFLE(3, 0, 2, 1));
	const __m128 l1 = _mm_shuffle_ps(a, a, _MM_SHUFFLE(3, 0, 2, 1));
	const __m128 l2 = _mm_shuffle_ps(b, b, _MM_SHUFFLE(3, 1, 0, 2));
	return _mm_sub_ps(_mm_mul_ps(l1, l2), _mm_mul_ps(r1,r2));
}

__forceinline __m128 DotV(const __m128 a, const __m128 b)
{
	const __m128 t0 = _mm_mul_ps(a, b);								//	aw*bw | az*bz | ay*by | ax*bx
	const __m128 t1 = _mm_shuffle_ps(t0, t0, _MM_SHUFFLE(1,0,3,2));	//	ay*by | ax*bx | aw*bw | az*bz
	const __m128 t2 = _mm_add_ps(t0, t1);							//	ay*by + aw*bw | ax*bx + az*bz | aw*bw + ay*by | az*bz + ax*bx
	const __m128 t3 = _mm_shuffle_ps(t2, t2, _MM_SHUFFLE(2,3,0,1));	//	ax*bx + az*bz | ay*by + aw*bw | az*bz + ax*bx | aw*bw + ay*by
	return _mm_add_ps(t3, t2);										//	ax*bx + az*bz + ay*by + aw*bw 
																	//	ay*by + aw*bw + ax*bx + az*bz
																	//	az*bz + ax*bx + aw*bw + ay*by
																	//	aw*bw + ay*by + az*bz + ax*bx
}

#define SSE_CONST4(name, val) static const __declspec(align(16)) unsigned int name[4] = { (val), (val), (val), (val) }
#define SSE_CONST(name) *(const __m128i *)&name
#define SSE_CONSTF(name) *(const __m128 *)&name

#define LoadU(v)	_mm_loadu_ps(&v.x)

static __forceinline int TestClassIII(const __m128& e0V, const __m128 v0V, const __m128 v1V, const __m128 v2V, const Point& extents)
{
	SSE_CONST4(maskV,	0x7fffffff);

	const __m128 e0XZY_V = _mm_shuffle_ps(e0V, e0V, _MM_SHUFFLE(3,0,2,1));

	const __m128 v0XZY_V = _mm_shuffle_ps(v0V, v0V, _MM_SHUFFLE(3,0,2,1));
	const __m128 p0V = _mm_sub_ps(_mm_mul_ps(v0V, e0XZY_V), _mm_mul_ps(v0XZY_V, e0V));

	const __m128 v1XZY_V = _mm_shuffle_ps(v1V, v1V, _MM_SHUFFLE(3,0,2,1));
	const __m128 p1V = _mm_sub_ps(_mm_mul_ps(v1V, e0XZY_V), _mm_mul_ps(v1XZY_V, e0V));

	const __m128 v2XZY_V = _mm_shuffle_ps(v2V, v2V, _MM_SHUFFLE(3,0,2,1));
	const __m128 p2V = _mm_sub_ps(_mm_mul_ps(v2V, e0XZY_V), _mm_mul_ps(v2XZY_V, e0V));

	__m128 minV = _mm_min_ps(p0V, p1V);
	minV = _mm_min_ps(minV, p2V);

	const __m128 extentsV = LoadU(extents);
	const __m128 fe0ZYX_V = _mm_and_ps(e0V, SSE_CONSTF(maskV));

	const __m128 fe0XZY_V = _mm_shuffle_ps(fe0ZYX_V, fe0ZYX_V, _MM_SHUFFLE(3,0,2,1));
	const __m128 extentsXZY_V = _mm_shuffle_ps(extentsV, extentsV, _MM_SHUFFLE(3,0,2,1));
	__m128 radV = _mm_add_ps(_mm_mul_ps(extentsV, fe0XZY_V), _mm_mul_ps(extentsXZY_V, fe0ZYX_V));

	const unsigned int test = _mm_movemask_ps(_mm_cmpgt_ps(minV, radV));
//	if(test&7)
//		return 0;

	__m128 maxV = _mm_max_ps(p0V, p1V);
	maxV = _mm_max_ps(maxV, p2V);

	radV = _mm_sub_ps(_mm_setzero_ps(), radV);

	const unsigned int test2 = test|_mm_movemask_ps(_mm_cmpgt_ps(radV, maxV));
	if(test2&7)
		return 0;
	return 1;
}

int OverlapTriangleBox_Optimized(const Point& boxcenter, const Point& extents, const Point& p0, const Point& p1, const Point& p2)
{
	const __m128 BoxCenterV = LoadU(boxcenter);
	const __m128 v0V = _mm_sub_ps(LoadU(p0), BoxCenterV);
	const __m128 v1V = _mm_sub_ps(LoadU(p1), BoxCenterV);
	const __m128 v2V = _mm_sub_ps(LoadU(p2), BoxCenterV);

	if(1)
	{
		__m128 extentsV = LoadU(extents);
		if(1)
		{
			const unsigned int MaskI = 0x7fFFffFF;
			__m128 cV = v0V;
			cV = _mm_and_ps(cV, _mm_load1_ps((const float*)&MaskI));
			const unsigned int test = _mm_movemask_ps(_mm_sub_ps(cV, extentsV));
			if((test&7)==7)
				return 1;
		}

		__m128 minV = _mm_min_ps(v0V, v1V);
		minV = _mm_min_ps(minV, v2V);

		const unsigned int test = _mm_movemask_ps(_mm_cmpgt_ps(minV, extentsV));
		if(test&7)
			return 0;

		__m128 maxV = _mm_max_ps(v0V, v1V);
		maxV = _mm_max_ps(maxV, v2V);

		extentsV = _mm_sub_ps(_mm_setzero_ps(), extentsV);

//		const unsigned int test2 = _mm_movemask_ps(_mm_cmpge_ps(extentsV, maxV));
		const unsigned int test2 = _mm_movemask_ps(_mm_cmpgt_ps(extentsV, maxV));
		if(test2&7)
			return 0;
	}

	const __m128 e0V = _mm_sub_ps(v1V, v0V);
	const __m128 e1V = _mm_sub_ps(v2V, v1V);
	{
		SSE_CONST4(signV,	0x80000000);
		const __m128 normalV = CrossV(e0V, e1V);
		const __m128 dV = DotV(normalV, v0V);

		const __m128 extentsV = LoadU(extents);
		__m128 normalSignsV = _mm_and_ps(normalV, SSE_CONSTF(signV));
		const __m128 maxV = _mm_or_ps(extentsV, normalSignsV);

		__m128 tmpV = DotV(normalV, maxV);
		const unsigned int test2 = _mm_movemask_ps(_mm_cmpgt_ps(dV, tmpV));
		if(test2&7)
			return 0;

		normalSignsV = _mm_xor_ps(normalSignsV, SSE_CONSTF(signV));
		const __m128 minV = _mm_or_ps(extentsV, normalSignsV);

		tmpV = DotV(normalV, minV);
		const unsigned int test = _mm_movemask_ps(_mm_cmpgt_ps(tmpV, dV));
		if(test&7)
			return 0;
	}

	if(1)
	{
		if(!TestClassIII(e0V, v0V, v1V, v2V, extents))
			return 0;
		if(!TestClassIII(e1V, v0V, v1V, v2V, extents))
			return 0;
		const __m128 e2V = _mm_sub_ps(v0V, v2V);
		if(!TestClassIII(e2V, v0V, v1V, v2V, extents))
			return 0;
	}
	return 1;
}
