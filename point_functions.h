#include <CGAL/Polygon_2_algorithms.h>
#include <CGAL/Exact_predicates_exact_constructions_kernel.h>
#include <CGAL/Polygon_2.h>
#include <CGAL/Segment_2.h>
#include <CGAL/Line_2.h>
#include <CGAL/Ray_2.h>
#include <CGAL/enum.h>
#include <iostream>

typedef CGAL::Exact_predicates_exact_constructions_kernel K;
typedef K::Point_2 Point_2;
typedef CGAL::Polygon_2<K> Polygon_2;
typedef CGAL::Segment_2<K> Segment_2;
typedef CGAL::Line_2<K> Line_2;
typedef CGAL::Ray_2<K> Ray_2;

extern double alpha;
extern Point_2 empty_point;

bool point_isset(Point_2 p);
bool compare_points_true(Point_2 a, Point_2 b);
Point_2 point_intersection(Line_2 pq, Segment_2 sgm, int seg_mode = 0);
Point_2 point_intersection(Ray_2 pq, Segment_2 sgm, int seg_mode = 0);
Point_2 point_intersection(Segment_2 pq, Segment_2 sgm, int seg_mode = 0);
