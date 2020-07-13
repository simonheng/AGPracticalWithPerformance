#include <weak_visibility.h>

//Computes the visibility polygon of a point
Polygon_2 compute_visibility_from_point(Point_2 a, Arrangement_2& env, TEV* tev)
{
	//Initialize variables
	std::vector<Segment_2> segments;
	std::vector<Point_2> points = {};
	
	//Find the face
	Arrangement_2::Face_const_handle * face;
	CGAL::Arr_naive_point_location<Arrangement_2> pl(env);
	CGAL::Arr_point_location_result<Arrangement_2>::Type obj = pl.locate(a);

	// The query point locates in the interior of a face
	face = boost::get<Arrangement_2::Face_const_handle>(&obj);
	Arrangement_2 output_arr;
	typedef CGAL::Simple_polygon_visibility_2<Arrangement_2, CGAL::Tag_false> NSPV;
	Face_handle fh;
	Halfedge_const_handle he = Halfedge_const_handle();


	//If the point is within a face, we can compute the visbility that way
	if (face != NULL)
	{		
		fh = tev->compute_visibility(a, *face, output_arr);
	}
	else
	{
		//If the point in a boundary segment, find the corresponding half edge
		he = env.halfedges_begin();		
		bool cont = !Segment_2(he->source()->point(), he->target()->point()).has_on(a) || he->source()->point() == a || he->face()->is_unbounded();
		//While we are not in the right half edge, or while q is the source, continue
		while (cont) {
			he++;
			if (he == env.halfedges_end()) {
				throw(std::exception());
			}

			cont = !Segment_2(he->source()->point(), he->target()->point()).has_on(a) || he->source()->point() == a || he->face()->is_unbounded();
		}

		//Use the half edge to compute the visibility
		fh = tev->compute_visibility(a, he, output_arr);
	}
	//Make sure the visibility polygon we find has an outer boundary
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

	return Polygon_2(points.begin(), points.end());
}


Polygon_2 compute_visibility_from_segment(std::vector<Point_2>& poly, Point_2 a, Point_2 b) {
	//check if we need to split the orignal polygon into more polygons 
	std::vector<std::vector<Point_2>> split_pgns = split_for_vis(a, b, poly);


	std::vector<Point_2> points = {};

	if (same((Point_2)poly[0], (Point_2)a) && same((Point_2)poly[1], (Point_2)b)) {
		points.push_back(a);
		points.push_back(b);

	}

	//points.push_back(b);

	//compute visibility per split polygon and then combine
	for (int k = 0; k < split_pgns.size();k++)
	{
		//initiliaze variables
		int R = 0;
		int L = 0;
		std::vector<Point_2> current_pgn_points = split_pgns[k];
		if (current_pgn_points.size() < 3)
			continue;

		//This means we are computing visibility for a point and not segment
		if (same((Point_2)current_pgn_points[0], (Point_2)current_pgn_points[1]))
		{
			//Erase the first point because it is duplicated
			current_pgn_points.erase(current_pgn_points.begin());

			if (current_pgn_points.size() < 3)
				continue;

			//compute the visibility using the method
			Arrangement_2 env = polygon_to_arrangement(current_pgn_points);
			TEV tev(env);
			std::vector<Point_2> vis_points = 
				compute_visibility_from_point(current_pgn_points[0], 
					env, &tev).container();
			
			//If the method results a valid polygon
			if (vis_points.size() > 0) {
				std::vector<Point_2> ordered_points = {};
				int viewer = -1;

				for (int i = 0; i < vis_points.size(); i++) {
					//Find index of viewer
					if (vis_points[i] == current_pgn_points[0]) {
						viewer = i;
						break;
					}					
				}
				if(viewer==vis_points.size()-1)
					points.insert(points.end(), vis_points.begin(), vis_points.end() - 1);
				else {
					points.insert(points.end(), vis_points.begin() + viewer + 1, vis_points.end());
					points.insert(points.end(), vis_points.begin(), vis_points.begin() + viewer);
				}
			}
			continue; //skip to the next iteration
		}

		//Loop through the vertices
		for (int i = 1; i < current_pgn_points.size() - 1; i++)
		{
			if (current_pgn_points[i + 1] == current_pgn_points[1]) {
				points.push_back(current_pgn_points[i + 1]);
				continue;
			}

		
			//Check for the righmost beam
			std::tuple<int, int> rmb = find_rightmost_beam(split_pgns[k], i);
			R = std::get<0>(rmb);
			L = std::get<1>(rmb);

			//No proper beam available!
			if (R == 0 && L == 0)
			{
				continue;
			}
			if (L == i + 1)
			{
				//add vi1 to result
				points.push_back(current_pgn_points[i + 1]);
				Point_2 s = empty_point;
				//point closest to v0 is a, so the source.				

				//compute point visibility
				Arrangement_2 env = polygon_to_arrangement(poly);
				TEV tev(env);
				Polygon_2 point_vis = compute_visibility_from_point(current_pgn_points[i + 1], env, &tev);

				CGAL::Bounded_side b = CGAL::bounded_side_2(point_vis.container().begin(), point_vis.container().end(), current_pgn_points[0]);
				
				if (b != CGAL::ON_UNBOUNDED_SIDE) //a is visible
					s = current_pgn_points[0];
				else {
					//iterate the polygon and find the part of the edge that is inside
					for (int i_vis = 0; i_vis < point_vis.size();i_vis++) {
						Point_2 curp = point_vis.container()[i_vis];
						Point_2 nextp = point_vis.container()[(i_vis + 1) % point_vis.size()];

						Point_2 interpoint = point_intersection(Segment_2(current_pgn_points[0], current_pgn_points[1]), 
							Segment_2(curp, nextp));
						if (isset(interpoint) && Segment_2(current_pgn_points[0], interpoint).squared_length() < Segment_2(current_pgn_points[0], s).squared_length())
							s = interpoint;
					}
				}				
			
				//is s in LHP(vi1,vi2)?
				if (!isset(s) || current_pgn_points.size() <= (i + 2) || in_half_plane({ CGAL::LEFT_TURN,  CGAL::COLLINEAR }, current_pgn_points[i + 1], current_pgn_points[i + 2], s))
					continue;
				else
				{	

					//x is the point where ray svi1 exits P, and vivi1 is the edge in which this happens (update i)
					std::tuple<Segment_2, int> sx_and_i = polygon_exits(Ray_2(s, current_pgn_points[i + 1]), current_pgn_points, current_pgn_points[i + 1], i + 1);
					Point_2 x = std::get<0>(sx_and_i).target();
					if (!same((Point_2)s, (Point_2)x) && !same((Point_2)current_pgn_points[i + 1], (Point_2)x) )
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
	
	//remove duplicates
	std::vector<Point_2> pure_points = {};
	for (int i = 0; i < points.size();i++) {
		int next = (i + 1) % points.size();
		if (!same((Point_2)points[i], (Point_2)points[next]))
			pure_points.push_back(points[i]);
	}
	

	return Polygon_2(pure_points.begin(), pure_points.end());
}