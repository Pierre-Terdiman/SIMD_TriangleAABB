#include "stdafx.h"
#include "Random.h"
#include "TriBox.h"

static __forceinline void StartProfile(unsigned int& val)
{
	__asm{
		cpuid
		rdtsc
		mov		ebx, val
		mov		[ebx], eax
	}
}

static __forceinline void EndProfile(unsigned int& val)
{
	__asm{
		cpuid
		rdtsc
		mov		ebx, val
		sub		eax, [ebx]
		mov		[ebx], eax
	}
}

enum BoxTriangleOverlapTest
{
	BTO_BEST_CASE,		// Triangle fully inside box, fastest early exit
	BTO_WORST_CASE,		// Triangle touches box but no early exit
	BTO_NO_COLLISIONS,
	BTO_MIXED
};

static const Point center(-1.1f, 2.2f, 3.3f);
static const Point extents(4.4f, 5.5f, 6.6f);

static void computeRandomPoint(BasicRandom& random, Point& p0, Point& p1, Point& p2, float offset0, float offset1, float offset2)
{
	{
		const float cx = random.randomFloat() + offset0;
		const float cy = random.randomFloat() + offset0;
		const float cz = random.randomFloat() + offset0;
		p0.x = center.x + extents.x*cx;
		p0.y = center.y + extents.y*cy;
		p0.z = center.z + extents.z*cz;
	}

	{
		const float cx = random.randomFloat() + offset1;
		const float cy = random.randomFloat() + offset1;
		const float cz = random.randomFloat() + offset1;
		p1.x = center.x + extents.x*cx;
		p1.y = center.y + extents.y*cy;
		p1.z = center.z + extents.z*cz;
	}

	{
		const float cx = random.randomFloat() + offset2;
		const float cy = random.randomFloat() + offset2;
		const float cz = random.randomFloat() + offset2;
		p2.x = center.x + extents.x*cx;
		p2.y = center.y + extents.y*cy;
		p2.z = center.z + extents.z*cz;
	}
}

static unsigned int	gNbHits = 0;
static Point*		gTris = NULL;

static void InitTest(BoxTriangleOverlapTest type)
{
	BasicRandom	random(42);

	Point p0, p1, p2;
	const unsigned int nb = 16384;
	for(unsigned int i=0;i<nb;i++)
	{
		if(type==BTO_BEST_CASE)
		{
			computeRandomPoint(random, p0, p1, p2, 0.5f, 0.5f, 0.5f);
		}
		else if(type==BTO_WORST_CASE)
		{
			computeRandomPoint(random, p0, p1, p2, 1.5f, 1.5f, 0.5f);
		}
		else if(type==BTO_NO_COLLISIONS)
		{
			computeRandomPoint(random, p0, p1, p2, 1000.0f, 1000.0f, 1000.0f);
		}
		else if(type==BTO_MIXED)
		{
			random.unitRandomPt(p0);
			random.unitRandomPt(p1);
			random.unitRandomPt(p2);

			const float scale = (0.5f + random.randomFloat()) * 20.0f;

			p0 *= scale;
			p1 *= scale;
			p2 *= scale;
		}

		gTris[i*3+0] = p0;
		gTris[i*3+1] = p1;
		gTris[i*3+2] = p2;
	}
}

template<const int version>
static unsigned int RunTest()
{
	const Point center(-1.1f, 2.2f, 3.3f);
	const Point extents(4.4f, 5.5f, 6.6f);

	const Point* Tris = gTris;

	unsigned int Time;
	unsigned int nbHits=0;
	const unsigned int nb = 16384;
	StartProfile(Time);
	{
		for(unsigned int i=0;i<nb;i++)
		{
			const Point& p0 = Tris[i*3+0];
			const Point& p1 = Tris[i*3+1];
			const Point& p2 = Tris[i*3+2];

			int b;
			if(version==0)
				b = OverlapTriangleBox_Default(center, extents, p0, p1, p2);
			else if(version==1)
				b = OverlapTriangleBox_Optimized(center, extents, p0, p1, p2);
			if(b)
				nbHits++;
		}
	}
	EndProfile(Time);
	gNbHits = nbHits;
	return Time;
}

int main(int argc, char* argv[])
{
	{
		_clearfp();

	   unsigned int x86_cw;
	   unsigned int sse2_cw;
		#define _MCW_ALL _MCW_DN | _MCW_EM | _MCW_IC | _MCW_RC | _MCW_PC
		__control87_2(_CW_DEFAULT | _DN_FLUSH, _MCW_ALL, &x86_cw, &sse2_cw);
		_clearfp();
	}

	gTris = new Point[16384*3];

	int Hits0[4], Time0[4];
	for(int i=0;i<4;i++)
	{
		InitTest(BoxTriangleOverlapTest(i));
		unsigned int Time = RunTest<0>();
		printf("Time (default): %d\n", Time/1024);
		Time0[i] = Time/1024;
		Hits0[i] = gNbHits;
	}

	int Hits1[4], Time1[4];
	for(int i=0;i<4;i++)
	{
		InitTest(BoxTriangleOverlapTest(i));
		unsigned int Time = RunTest<1>();
		printf("Time (optimized): %d\n", Time/1024);
		Time1[i] = Time/1024;
		Hits1[i] = gNbHits;
	}

	for(int i=0;i<4;i++)
	{
		printf("Hits0/Hits1: %d/%d\n", Hits0[i], Hits1[i]);
	}

	for(int i=0;i<4;i++)
	{
		if(i==0)
			printf("Best case: ");
		else if(i==1)
			printf("Worst case:");
		else if(i==2)
			printf("No hit:    ");
		else if(i==3)
			printf("Mixed:     ");

		printf(" Speedup: %f\n", float(Time0[i])/float(Time1[i]));
	}

	delete [] gTris;

	return 0;
}
