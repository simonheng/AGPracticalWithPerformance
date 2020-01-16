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

std::vector<Point_2> add_points_between(std::vector<Point_2> points, std::vector<Point_2> original_points, int from, int to);
std::vector<Point_2> reverse_pgn(std::vector<Point_2> points, int i);
std::vector<Point_2> orientate_pgn(std::vector<Point_2> points, int a_index, int b_index);
bool in_half_plane(std::vector<CGAL::Orientation> sides, Point_2 p, Point_2 q, Point_2 x);
bool segment_enters_HP_quad(std::vector<Point_2> quad_points, Point_2 p, Point_2 q, Point_2 a, Point_2 b, CGAL::Orientation side);
std::tuple<Segment_2, int> polygon_exits(Ray_2 pq, std::vector<Point_2> points, Point_2 target, int def);
std::tuple<int, int> find_rightmost_beam(Polygon_2 pgn, int i);
Segment_2 point_to_edge_visibility(Polygon_2 pgn, Point_2 p);
std::tuple < std::vector<Polygon_2>, Point_2, Point_2, std::vector<Point_2>> extend_and_split(Point_2 a, Point_2 b, std::vector<Point_2> original_points);