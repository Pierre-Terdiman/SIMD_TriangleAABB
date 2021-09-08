#ifndef RANDOM_H
#define RANDOM_H

	class BasicRandom
	{
		public:
										BasicRandom(unsigned int seed=0) : mRnd(seed)	{}
										~BasicRandom()									{}

		__forceinline	void			setSeed(unsigned int seed)	{ mRnd = seed;											}
		__forceinline	unsigned int	getCurrentValue()	const	{ return mRnd;											}
						unsigned int	randomize()					{ mRnd = mRnd * 2147001325 + 715136305; return mRnd;	}

		__forceinline	unsigned int	rand()						{ return randomize() & 0xffff;							}

						float			rand(float a, float b)
										{
											const float r = (float)rand()/((float)0x7fff+1);
											return r*(b-a) + a;
										}

						float			randomFloat()
										{
											return (float(randomize() & 0xffff)/65535.0f) - 0.5f;
										}

						void			unitRandomPt(Point& v);
		private:
						unsigned int	mRnd;
	};

#endif