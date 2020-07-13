#pragma once

//general
#include <libxl.h>
#include <iostream>
#include <windows.h>	
#include <queue>
#include <string>
#include <random>
#include <unordered_set>
#include <thread>
#include <numeric>
//#include <math.h>
#include <boost/range.hpp>
#include <boost/range/join.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/graphml.hpp> 
#include <boost/graph/graph_utility.hpp>
//This includes all necessary cgal objects and formulates typedefs
#include <ilcplex/ilocplex.h>
//#include <CGAL/Exact_predicates_exact_constructions_kernel.h>
#include <CGAL/Exact_predicates_exact_constructions_kernel.h>
#include <CGAL/Quotient.h>
#include <CGAL/Polygon_2.h>
#include <CGAL/Segment_2.h>
#include <CGAL/Vector_2.h>

#include <CGAL/Arr_segment_traits_2.h>
#include <CGAL/Arrangement_2.h>
#include <CGAL/Arr_walk_along_line_point_location.h>
#include <CGAL/Line_2.h>
#include <CGAL/Ray_2.h>
#include <CGAL/intersections.h>
#include <CGAL/Aff_transformation_2.h>
#include <CGAL/Gps_traits_2.h>
#include <CGAL/Boolean_set_operations_2.h>
#include <CGAL/General_polygon_with_holes_2.h>
#include <CGAL/Bbox_2.h>
#include "CGAL/Homogeneous.h"
#include <chrono> 
#include <CGAL/Surface_sweep_2_algorithms.h>
#include <CGAL/Polygon_2_algorithms.h>
#include <CGAL/enum.h>
#include "CGAL/Homogeneous.h"
#include <CGAL/point_generators_2.h>
#include <CGAL/random_polygon_2.h>
#include <CGAL/Random.h>
#include <CGAL/algorithm.h>
#include <CGAL/Triangulation_vertex_base_with_info_2.h>
#include <CGAL/Constrained_Delaunay_triangulation_2.h>
#include <CGAL/Triangulation_face_base_with_info_2.h>
#include <CGAL/Triangle_2.h>
#include <CGAL/Simple_polygon_visibility_2.h>
#include <CGAL/Triangular_expansion_visibility_2.h>
#include <CGAL/Arr_naive_point_location.h>
#include <CGAL/partition_2.h>
#include <CGAL/Partition_traits_2.h>
#include <CGAL/property_map.h>
#include <exception>
#include <ctpl.h>


//Standard
typedef CGAL::Exact_predicates_exact_constructions_kernel K;
typedef CGAL::Point_2<K>						Point_2;
typedef CGAL::Polygon_2<K> Polygon_2;
typedef CGAL::Polygon_with_holes_2<K>                Polygon_with_holes_2;
typedef CGAL::Segment_2<K> Segment_2;
typedef CGAL::Vector_2<K> Vector_2;

typedef CGAL::Line_2<K> Line_2;
typedef CGAL::Ray_2<K> Ray_2;
typedef K::Intersect_2 Intersect_2;
typedef CGAL::Arr_segment_traits_2<K>				  Traits_2;
typedef CGAL::Polygon_with_holes_2<K>                Polygon_with_holes_2;

typedef CGAL::Direction_2<K> Direction;

#define DIV_ROUND_CLOSEST(n, d) ((((n) < 0) ^ ((d) < 0)) ? (((n) - (d)/2)/(d)) : (((n) + (d)/2)/(d)))

//Random polygons
typedef CGAL::Aff_transformation_2<K> Transformation;
typedef std::list<Point_2>                                Container;
typedef CGAL::Creator_uniform_2<int, Point_2>             Creator;
typedef CGAL::Random_points_in_square_2<Point_2, Creator> Point_generator;

//Partition
typedef CGAL::Partition_traits_2<K>                         Traits;
typedef Traits::Point_2                                     Point_p;
typedef Traits::Polygon_2                                   Polygon_p;
typedef std::vector<Polygon_p>                                Polygon_list;

//Triangulation
struct Triangle_info
{
	Triangle_info() {}
	int nesting_level;
	int id = -1;
	std::vector<int> visited = {};
	std::vector<int> visited_diag = {};

	bool in_domain() {
		return nesting_level % 2 == 1;
	}
};
struct Vertex_T_info
{
	int id = -3;
	int face_id = -4;
};

typedef CGAL::Triangulation_vertex_base_with_info_2<Vertex_T_info, K> Vb;
typedef CGAL::Triangulation_face_base_with_info_2<Triangle_info, K>    Fbb;
typedef CGAL::Constrained_triangulation_face_base_2<K, Fbb>        Fb;
typedef CGAL::Triangulation_data_structure_2<Vb, Fb>               TDS;
typedef CGAL::Exact_intersections_tag                                Itag;
typedef CGAL::Constrained_Delaunay_triangulation_2<K, TDS, Itag>  CDT;
typedef CDT::Face_handle                                          CDT_FH;
typedef CDT::Point                                                CDT_Point;
typedef CDT::Locate_type lt;
typedef CGAL::Arr_segment_traits_2<K>::X_monotone_curve_2 X_monotone;
typedef CDT::Finite_faces_iterator CFFI;

class PointGuard;
class FaceGuard;

//Arrrangements
struct Vertex_info
{
	Vertex_info() {};
	int id = -1;
	int triangle;
	lt type;
	int decomp_index;
	int poly_index;
	std::vector<Ray_2> rayz;
	int rays_used;
	bool is_in_triangle1 = false;
	bool is_reflex = false;
	int poly_id;
	int next_id;
	int prev_id;
	//Point_2 prev_point;
	bool lock = false;
};

struct Face_info
{
	Face_info() {};
	int id = -1;
	int decomp_index;
	
};

typedef CGAL::Arr_extended_dcel<Traits_2, Vertex_info, int, Face_info>      Dcel;
typedef CGAL::Arrangement_2<Traits_2, Dcel>                     Arrangement_2;
typedef Arrangement_2::Vertex_handle                  Vertex_handle;
typedef Arrangement_2::Halfedge_handle                Halfedge_handle;
typedef Arrangement_2::Face_handle                    Face_handle;
typedef Arrangement_2::Halfedge_const_handle                    Halfedge_const_handle;
typedef Arrangement_2::Edge_const_iterator                              Edge_const_iterator;
typedef Arrangement_2::Ccb_halfedge_circulator                          Ccb_halfedge_circulator;

//walk/vis
typedef CGAL::Triangular_expansion_visibility_2<Arrangement_2>  TEV;
typedef CGAL::Arr_walk_along_line_point_location<Arrangement_2> Walk_pl;

extern double alpha;
extern Point_2 empty_point;

bool isset(Point_2 p);
bool same(Point_2& a, Point_2& b);
bool in_between(Segment_2 s, Point_2 a);

void remove_intersection(std::vector<int>& a, std::vector<int>& b);
std::vector<std::vector<int>> SplitVector(const std::vector<int>& vec, size_t n);
CGAL::Lazy_exact_nt<CGAL::Gmpq> power_angle(Point_2 reflex, std::vector<Point_2> face);
double Hausdorff(std::vector<Point_2> A, std::vector<Point_2> B);
bool proper_intersect_face(std::vector<Segment_2> face, Segment_2 chord, Segment_2& res);
bool proper_intersect_face(std::vector<Segment_2> face, Ray_2 ray, Segment_2& res);
std::vector<Segment_2> recursive_ray_intersection(int rayz, int l, int r, std::vector<Segment_2> face, Point_2 reflex);

Point_2 point_intersection(Line_2 pq, Segment_2 sgm, int seg_mode = 0);
Point_2 point_intersection(Ray_2 pq, Segment_2 sgm, int seg_mode = 0);
Point_2 point_intersection(Segment_2 pq, Segment_2 sgm, int seg_mode = 0);
Point_2 first_intersection(Point_2 p, Point_2 q, std::vector<Point_2> points);
bool fast_has_on(Point_2 &a, Point_2 &b, Point_2 &c);
int difference_in_polygon(int a, int b, int n);