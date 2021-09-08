# SIMD_TriangleAABB
A SIMD version of Möller's triangle-box overlap code

Initially posted on my blog: http://www.codercorner.com/blog/?p=1118

This is an online copy of the files published there. Below is a copy of the original readme file.

======================================================================================================

A long time ago (around 2001), I helped Tomas Möller optimize a triangle-vs-box, SAT-based overlap test (http://fileadmin.cs.lth.se/cs/Personal/Tomas_Akenine-Moller/code/). At that time I was working on the Opcode library, and such a function was needed to achieve my goals.

When working on Opcode 2.0, I wrote a SIMD version of that same function. Since then I have found several other SIMD implementations of that code, and while benchmarking them I found that most of them were surprisingly slow - sometimes slower than the original code! I cannot really explain why, but it seems to indicate that writing the SIMD version is not as straightforward as one could have imagined. And thus, I am now releasing my implementation, in the hope somebody will find it useful.

There are two versions of the original code, labelled "old code" and "new, faster code" online (http://fileadmin.cs.lth.se/cs/Personal/Tomas_Akenine-Moller/code/). Despite the claims, on my Windows-based machine the "faster code" is significantly slower. It seems correct and expected that doing the "class III" tests last is the best, but I suppose this depends on both the architecture and the test scenario. In any case both versions (more or less verbatim, not quite) are included in the test benchmark.

The original code contains a lot of early exits, and thus the time it takes depends a lot on what codepath is taken. The best case is when you reach the first early exit. The worst case is when you reach the end of the function, i.e. the triangle and the box overlap. I believe one of the biggest mistakes made in the slow SIMD implementations was to get rid of branches and early exits at all costs ("Hey look! Zero branches! It must be super fast!"). Well of course that is a very naive view, things are way more subtle and complex than that. The fastest code is the one that is never executed, and thus, you should embrace early exits, not avoid them. In fact, my implementation contains one more early exit than the original code, for when the box fully contains the triangle (this was an important case for me in some application).

I have 4 different test scenarios:
- best case
- worst case
- no hit
- mixed

Best case is the aforementioned new early exit, for when the box fully contains the triangle. This is basically a point-in-AABB test, it is the first early exit in the code, and thus the "best case" is when we reach that codepath.

The worst case is when the box touches the triangle, but the triangle is not fully inside. In that case we reach the end of the overlap function without taking any early exits. This should be the slowest codepath.

No hit is when the triangle does not touch the box, and we reach one of the "regular" early exits (i.e. the ones that also existed in the non-SIMD code). Time depends on what codepath is reached.

Mixed is just a mix of the above cases.

Here are typical results on my test PC:

Version1 (Möller's "old code"):

	Time (default): 5072
	Time (default): 3773
	Time (default): 953
	Time (default): 5088
	Time (optimized): 246
	Time (optimized): 1898
	Time (optimized): 329
	Time (optimized): 1544
	Hits0/Hits1: 16384/16384
	Hits0/Hits1: 16384/16384
	Hits0/Hits1: 0/0
	Hits0/Hits1: 12420/12420
	Best case:  Speedup: 20.617886
	Worst case: Speedup: 1.987882
	No hit:     Speedup: 2.896657
	Mixed:      Speedup: 3.295337

Version2 (Möller's "new, faster code"):

	Time (default): 6209
	Time (default): 4714
	Time (default): 1304
	Time (default): 5199
	Time (optimized): 237
	Time (optimized): 1822
	Time (optimized): 316
	Time (optimized): 1540
	Hits0/Hits1: 16384/16384
	Hits0/Hits1: 16384/16384
	Hits0/Hits1: 0/0
	Hits0/Hits1: 12420/12420
	Best case:  Speedup: 26.198313
	Worst case: Speedup: 2.587267
	No hit:     Speedup: 4.126582
	Mixed:      Speedup: 3.375974

So what we see here:

- the "new, faster code" is in fact slower. Thus, ignore "Version2" from now on.

- the non-SIMD and SIMD version all find the same numbers of hits (as they should). I could have better tests for this but I've been using the SIMD version for a while now and it should be quite solid.

- the new, additional early exit gives me a 20X speedup. Not bad for a SIMD version whose theoretical maximum speedup is 4X. In the SIMD code, this is taken care of by this snippet:

		if(1)
		{
			const unsigned int MaskI = 0x7fFFffFF;
			__m128 cV = v0V;
			cV = _mm_and_ps(cV, _mm_load1_ps((const float*)&MaskI));
			const unsigned int test = _mm_movemask_ps(_mm_sub_ps(cV, extentsV));
			if((test&7)==7)
				return 1;
		}

That is well worth it if you ask me. Helped enormously in some past projects of mine.

- Speedups otherwise vary between 2X and 3X, which is what you usually expect from a good SIMD implementation.


And that's about it. This is straightforward but apparently it is easy to write a bad SIMD version, that ends up slower than the original. I suppose some people just write the code, assume it's good, and do not run benchmarks? I don't know.

Bitcoing tip jar is here if you end up using that code: http://www.codercorner.com/blog/?p=1107



