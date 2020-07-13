#pragma once
#include <point_functions.h>
#include <weak_visibility.h>

//Triangle ID in triangulation paired with the vertices of the triangle
struct triangle_loc
{
	int t_id;
	std::vector<Point_2> pts;
};


class Path {
public:
	Point_2 from;
	Point_2 to;
	bool set;
	Path Path::reverse();
	std::vector<Point_2> points; //without to and from	
	Path();
	Path(Point_2 &from, Point_2 &to, std::vector<Point_2> points);	
};

class Funnel {	
public:
	Path path_1; //from should be the same!
	Path path_2; //from should be the same!
	int split_index;
	Point_2 fp;
	Funnel(Path &path_1, Path &path_2);
	Funnel();
	Path Merge(Funnel &that, bool second_try = false);

	std::vector<Point_2> merge_middle(Point_2 that_fp, bool second);
};

class WeakVisPoly {
public:
	std::vector<Point_2> poly;

	int id; //our id
	int parentId; //id of our parent
	int childId; //which child are we in the parents child list

	int v0; //triangle id of the first point and second point (0,1)
	std::vector<Point_2> v0_t; //triangle vertices of triangle v0

	std::vector<int> children; //our child list
	std::vector<int> children_i0; //same size as children: vertex ID of the first point of the window to the child
	std::vector<int> children_i1; //same size as children: vertex ID of the second point of the window to the child

	std::vector<int> children_v0; //same size as children: triangleID in triangulation in which the window is located
	std::vector< std::vector<Point_2>> children_v0_t; //same size as children: triangle vertices of the triangle in which the window is located

	std::vector<int> index_map;	//maps index of weak poly to original poly
	std::vector<bool> on_segment;


	std::vector<std::vector<std::vector<int>>> diagonalMap; //given two triangle IDs in triangulation holds the vertex IDs of the diagonal

	std::vector<std::vector<Path>> map; //shortest path map within weak vis poly
	std::vector<std::vector<Path>> parentMap; //shortest path map from parent to us
	std::vector<std::vector<std::vector<Path>>> siblingMap; //paths from other sibling to us

	Segment_2 origin;

	WeakVisPoly(std::vector<Point_2>& oripoly, int prev);
	WeakVisPoly();
	WeakVisPoly(std::vector<Point_2> poly);

	CDT T;
	std::vector<CDT_FH> f_handles;
	std::vector<CDT::Vertex_handle> v_handles;
	std::vector<std::vector<K::Triangle_2>> level_faces;



	Path shortest_point_point(Point_2 p1, Point_2 p2, Vertex_info inf1, Vertex_info inf2, int status = 0, WeakVisPoly &other = WeakVisPoly());
	CDT_FH get_loc(Point_2& p, lt& t);

	triangle_loc find_segment(Point_2 p, Point_2 q, bool second_try = false);

	Vertex_info get_location(Vertex_info &inf, Point_2 v);
	void recursive_shortest(CDT_FH f, int query);
	void recursive_diagonal_map(CDT_FH& f, int query, int diag1, int diag2);

};

class Shortest_Map {
public:
	std::vector<Point_2> poly;
	std::vector<WeakVisPoly> decomp;
	
	void recursive_decomp(std::vector<Point_2> sub_poly, int prev_id);
	Vertex_info get_location(Vertex_info &inf, Vertex_handle v);

	Shortest_Map();
	Shortest_Map(std::vector<Point_2> poly);

	Path shortest_point_point(Vertex_info p, Vertex_info q, Point_2 p_p, Point_2 q_p, bool &found);
	bool segment_sees_point(Vertex_info s1, Vertex_info s2, Vertex_info p, Point_2 s1_p, Point_2 s2_p, Point_2 p_p);
	//bool point_sees_point(Vertex_handle &p, Vertex_handle &q);
};

