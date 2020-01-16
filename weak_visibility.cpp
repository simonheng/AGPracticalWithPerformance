#include <CGAL/Polygon_2_algorithms.h>
#include <CGAL/Exact_predicates_exact_constructions_kernel.h>
#include <CGAL/Polygon_2.h>
#include <CGAL/Segment_2.h>
#include <CGAL/Line_2.h>
#include <CGAL/Ray_2.h>
#include <CGAL/enum.h>
#include <CGAL/intersections.h>
#include <iostream>
#include <point_functions.h>
#include <polygon_functions.h>
#include <CGAL/Simple_polygon_visibility_2.h>
#include <CGAL/Triangular_expansion_visibility_2.h>
#include <CGAL/Arrangement_2.h>
#include <CGAL/Arr_segment_traits_2.h>
#include <CGAL/Arr_naive_point_location.h>

typedef CGAL::Exact_predicates_exact_constructions_kernel K;
typedef K::Point_2 Point_2;
typedef CGAL::Polygon_2<K> Polygon_2;
typedef CGAL::Segment_2<K> Segment_2;
typedef CGAL::Line_2<K> Line_2;
typedef CGAL::Ray_2<K> Ray_2;
typedef K::Intersect_2 Intersect_2;
typedef CGAL::Arr_segment_traits_2<K>									Traits_2;
typedef CGAL::Arrangement_2<Traits_2>                                   Arrangement_2;
typedef Arrangement_2::Face_handle                                      Face_handle;
typedef Arrangement_2::Edge_const_iterator                              Edge_const_iterator;
typedef Arrangement_2::Ccb_halfedge_circulator                          Ccb_halfedge_circulator;
typedef CGAL::Triangular_expansion_visibility_2<Arrangement_2>  TEV;
typedef Arrangement_2::Halfedge_const_handle                    Halfedge_const_handle;

//Computes the visibility polygon of a point
std::vector<Point_2> compute_visibility_from_point(Polygon_2 pgn, Point_2 a) 
{
	//Initialize variables
	Arrangement_2 env;
	std::vector<Segment_2> segments;
	std::vector<Point_2> points = {};
	std::vector<Point_2> temp_pgn_points = pgn.container();
	
	//Create segments from the vertices in the polygon
	for (int i = 0; i<pgn.container().size(); i++)
	{
		int next = (i + 1) % pgn.size();
		segments.push_back(Segment_2(temp_pgn_points[i], temp_pgn_points[next]));
	}

	//Insert the segments into the environment
	CGAL::insert_non_intersecting_curves(env, segments.begin(), segments.end());

	//Find the face
	Arrangement_2::Face_const_handle * face;
	CGAL::Arr_naive_point_location<Arrangement_2> pl(env);
	CGAL::Arr_point_location_result<Arrangement_2>::Type obj = pl.locate(a);

	// The query point locates in the interior of a face
	face = boost::get<Arrangement_2::Face_const_handle>(&obj);
	Arrangement_2 output_arr;
	typedef CGAL::Simple_polygon_visibility_2<Arrangement_2, CGAL::Tag_false> NSPV;
	TEV tev(env);
	Face_handle fh;
	Halfedge_const_handle he = Halfedge_const_handle();

	//If the point is within a face, we can compute the visbility that way
	if (face != NULL)
	{
		fh = tev.compute_visibility(a, *face, output_arr);
	}
	else
	{
		//If the point in a boundary segment, find the corresponding half edge
		he = env.halfedges_begin();
		
		//While we are not in the right half edge, or while q is the source, continue
		while (!Segment_2(he->source()->point(), he->target()->point()).has_on(a) || he->source()->point() == a || he->face()->is_unbounded())
			he++;

		//Use the half edge to compute the visibility
		fh = tev.compute_visibility(a, he, output_arr);
	}

	//Make sure the visibility polygon we find has an outer circle
	if (fh->has_outer_ccb()) {
		Arrangement_2::Ccb_halfedge_circulator curr = fh->outer_ccb();

		//find the right halfedge first
		if (he != Halfedge_const_handle())
			while (++curr != fh->outer_ccb())
				if (curr->source()->point() == he->source()->point())
					break;

		Arrangement_2::Ccb_halfedge_circulator first = curr;
		points.push_back(curr->source()->point());

		//Save the points from the visibility polygon
		while (++curr != first)
			points.push_back(curr->source()->point());
	}
	return points;
}


std::vector<Point_2> compute_visibility_from_segment(Polygon_2 pgn, Point_2 a = Point_2(), Point_2 b = Point_2()) {
	std::vector<Point_2> points = {};

	//check if we need to split the orignal polygon into more polygons 
	std::tuple<std::vector<Polygon_2>, Point_2, Point_2, std::vector<Point_2>> split_tuple = extend_and_split(a, b, pgn.container());
	std::vector<Polygon_2> split_polygons = std::get<0>(split_tuple);

	//extra points to make sure that we handle every split correctly
	Point_2 to_add_1 = std::get<1>(split_tuple);
	Point_2 to_add_2 = std::get<2>(split_tuple);
	std::vector<Point_2> extra_points = std::get<3>(split_tuple);

	if (point_isset(to_add_1))
		points.push_back(to_add_1);
	if (point_isset(to_add_2))
		points.push_back(to_add_2);


	//compute visibility per split polygon and then combine
	for (int k = 0; k < split_polygons.size();k++)
	{
		//initiliaze variables
		int R = 0;
		int L = 0;
		std::vector<Point_2> current_pgn_points = split_polygons[k].container();

		//Add the optional extra point
		if (point_isset(extra_points[k]))
			points.push_back(extra_points[k]);

		//This means we are computing visibility for a point and not segment
		if (compare_points_true(current_pgn_points[0], current_pgn_points[1]))
		{
			//Erase the first point because it is duplicated
			current_pgn_points.erase(current_pgn_points.begin());

			//compute the visibility using the method
			std::vector<Point_2> vis_points = compute_visibility_from_point(Polygon_2(current_pgn_points.begin(), current_pgn_points.end()), current_pgn_points[0]);

			//If the method results a valid polygon
			if (vis_points.size() > 0) {
				vis_points.erase(vis_points.begin());
				points.insert(points.end(), vis_points.begin(), vis_points.end());
			}
			continue; //skip to the next iteration
		}

		//Loop through the vertices
		for (int i = 1; i < current_pgn_points.size() - 1; i++)
		{
			//Check for the righmost beam
			std::tuple<int, int> rmb = find_rightmost_beam(split_polygons[k], i);
			R = std::get<0>(rmb);
			L = std::get<1>(rmb);

			break;

			//No proper beam available!
			if (R == 0 && L == 0)
			{
				continue;
			}
			if (L == i + 1)
			{
				//add vi1 to result
				points.push_back(current_pgn_points[i + 1]);

				//point closest to v0 is a, so the source.
				Segment_2 svi1 = point_to_edge_visibility(split_polygons[k], current_pgn_points[i + 1]);
				Point_2 s = svi1.source();

				//is s in LHP(vi1,vi2)?
				if (!point_isset(s) || current_pgn_points.size() <= (i + 2) || in_half_plane({ CGAL::LEFT_TURN,  CGAL::COLLINEAR }, current_pgn_points[i + 1], current_pgn_points[i + 2], s))
					continue;
				else
				{
					//x is the point where ray svi1 exits P, and vivi1 is the edge in which this happens (update i)
					std::tuple<Segment_2, int> sx_and_i = polygon_exits(Ray_2(s, current_pgn_points[i + 1]), current_pgn_points, current_pgn_points[i + 1], i + 2);
					Point_2 x = std::get<0>(sx_and_i).target();
					if (!compare_points_true(s, x) && !compare_points_true(current_pgn_points[i + 1], x))
					{
						points.push_back(x);
						i = std::get<1>(sx_and_i) - 1;
					}
				}
			}
			else
			{
				//Use R and L to find the left most point on vivi1 that we can see
				Point_2 q = point_intersection(Ray_2(current_pgn_points[R], current_pgn_points[L]), Segment_2(current_pgn_points[i], current_pgn_points[i + 1]));
				points.push_back(q);
				points.push_back(current_pgn_points[L]);

				//any i between i and L we cannot see, so i = L
				i = L - 1;
			}
		}
	}
	return points;
}