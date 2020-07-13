#pragma once
#include <point_functions.h>

std::vector<Point_2> add_points_between(std::vector<Point_2> points, std::vector<Point_2> original_points, int from, int to);
std::vector<Point_2> reverse_pgn(std::vector<Point_2> points, int i);
std::vector<Point_2> orientate_pgn(std::vector<Point_2> points, int a_index, int b_index);
bool in_half_plane(std::vector<CGAL::Orientation> sides, Point_2 p, Point_2 q, Point_2 x);
bool segment_enters_HP_quad(std::vector<Point_2> quad_points, Point_2 p, Point_2 q, Point_2 a, Point_2 b, CGAL::Orientation side);
std::tuple<Segment_2, int> polygon_exits(Ray_2 pq, std::vector<Point_2> points, Point_2 target, int def);
std::tuple<int, int> find_rightmost_beam(std::vector<Point_2> pgn, int i);
Segment_2 point_to_edge_visibility(std::vector<Point_2> pgn, Point_2 p);
Arrangement_2 polygon_to_arrangement(std::vector<Point_2> poly);
Arrangement_2 rotate_arrangement(Arrangement_2 original);
std::vector<Segment_2> shoot_rays(std::vector<Point_2> sources, std::vector<Point_2> targets, std::vector<Point_2> next, Arrangement_2 arr, int mode);
std::vector<std::vector<Point_2>> split_for_vis(Point_2 a, Point_2 b, std::vector<Point_2>& original_points);
std::vector<Point_2> random_poly(int n);