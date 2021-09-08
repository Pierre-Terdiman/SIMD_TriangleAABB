#include "stdafx.h"
#include "Random.h"

void BasicRandom::unitRandomPt(Point& v)
{
	v.x = randomFloat();
	v.y = randomFloat();
	v.z = randomFloat();
	v.Normalize();
}
