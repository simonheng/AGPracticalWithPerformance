#include <CGAL/Polygon_2_algorithms.h>
#include <CGAL/Exact_predicates_exact_constructions_kernel.h>
#include <CGAL/Polygon_2.h>
#include <CGAL/Segment_2.h>
#include <CGAL/Line_2.h>
#include <CGAL/Ray_2.h>
#include <CGAL/enum.h>
#include <CGAL/intersections.h>
#include <iostream>

typedef CGAL::Exact_predicates_exact_constructions_kernel K;
typedef K::Point_2 Point_2;
typedef CGAL::Polygon_2<K> Polygon_2;
typedef CGAL::Segment_2<K> Segment_2;
typedef CGAL::Line_2<K> Line_2;
typedef CGAL::Ray_2<K> Ray_2;
typedef K::Intersect_2 Intersect_2;

double alpha = 1;
Point_2 empty_point = Point_2(-10000, -10000);

bool point_isset(Point_2 p)
{
	return p != empty_point;
}
bool compare_points_true(Point_2 a, Point_2 b) {
	return a == b;
	return(b.hx() + alpha >= a.hx() && b.hx() - alpha <= a.hx() && b.hy() + alpha >= a.hy() && b.hy() - alpha <= a.hy());
}

Point_2 point_intersection(Line_2 pq, Segment_2 sgm, int seg_mode = 0) {

	auto result = intersection(pq, sgm);

	if (result) {
		if (const Segment_2* s = boost::get<Segment_2>(&*result)) {
			if (compare_points_true(s->source(), s->target()))
				return empty_point;
			else if (seg_mode == 1)
				return s->source();
			else if (seg_mode == 2)
				return s->target();
		}
		else {
			Point_2* p = boost::get<Point_2>(&*result);
			return *p;
		}
	}

	return empty_point;
}

Point_2 point_intersection(Ray_2 pq, Segment_2 sgm, int seg_mode = 0) {
	if (compare_points_true(pq.source(), sgm.source()))
		return pq.source();
	if (compare_points_true(pq.source(), sgm.target()))
		return pq.source();
	//if (pq.has_on(sgm.source()))
		//return sgm.source();
	//if (pq.has_on(sgm.target()))
	//	return sgm.target();
	
	auto result = intersection(pq, sgm);
	if (result) {
		if (const Segment_2* s = boost::get<Segment_2>(&*result)) {
			if (seg_mode == 1)
				return s->source();
			else if (seg_mode == 2)
				return s->target();
		}
		else {
			Point_2* p = boost::get<Point_2>(&*result);
			return *p;
		}
	}
	return empty_point;
}

Point_2 point_intersection(Segment_2 pq, Segment_2 sgm, int seg_mode = 0) {
	auto result = intersection(pq, sgm);

	if (result) {
		if (const Segment_2* s = boost::get<Segment_2>(&*result)) {
			if (seg_mode == 1)
				return s->source();
			else if (seg_mode == 2)
				return s->target();
		}
		else {
			Point_2* p = boost::get<Point_2>(&*result);
			return *p;
		}
	}
	return empty_point;
}
