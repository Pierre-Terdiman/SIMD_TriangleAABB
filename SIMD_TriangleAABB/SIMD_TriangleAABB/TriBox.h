#ifndef TRIBOX_H
#define TRIBOX_H

	int OverlapTriangleBox_Default		(const Point& boxcenter, const Point& extents, const Point& p0, const Point& p1, const Point& p2);
	int OverlapTriangleBox_Optimized	(const Point& boxcenter, const Point& extents, const Point& p0, const Point& p1, const Point& p2);

#endif
