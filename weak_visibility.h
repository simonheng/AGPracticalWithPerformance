#pragma once
#include <point_functions.h>
#include <polygon_functions.h>



/// <summary>Computes the visibility polygon of a point </summary>
/// <param name="poly">The polygon boundary</param> 
/// <param name="a">The first point of the query segment</param> 
/// <param name="b">The second point of the query segment</param> 
/// <returns>Visibility polygon of segments AB</returns> 
Polygon_2 compute_visibility_from_segment(std::vector<Point_2>& poly, Point_2 a = Point_2(), Point_2 b = Point_2());


/// <summary>Computes the visibility polygon of a point </summary>
/// <param name="a">The query point</param> 
/// <param name="env">The arrangement of the outside polygon</param> 
/// <param name="tev">The triangular expansion CGAL object pointer. This is easily initialized using the environment. Uses pointer to save time and memory.</param> 
/// <returns>Visibility polygon of point a</returns> 
Polygon_2 compute_visibility_from_point(Point_2 a, Arrangement_2& env, TEV* tev);