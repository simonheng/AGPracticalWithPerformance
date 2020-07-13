#include <polygon_functions.h>

std::vector<Point_2> add_points_between(std::vector<Point_2> points, std::vector<Point_2> original_points, int from, int to)
{
	if(from < to)
		points.insert(points.end(), original_points.begin() + from, original_points.begin() + to);
	else
	{
		points.insert(points.end(), original_points.begin() + from, original_points.end());
		points.insert(points.end(), original_points.begin(), original_points.begin() + to);
	}
	return points;

}

std::vector<Point_2> reverse_pgn(std::vector<Point_2> points, int i)
{
	Point_2 v0 = points[0];
	Point_2 v1 = points[1];

	points.erase(points.begin(), points.begin() + 2);

	points.push_back(v0);
	points.push_back(v1);

	std::reverse(points.begin(), points.end());

	return points;
}

std::vector<Point_2> orientate_pgn(std::vector<Point_2> points, int a_index, int b_index)
{
	std::vector<Point_2> results = {};
	//add a and b
	results.push_back(points[a_index]);
	results.push_back(points[b_index]);

	//add between b and end and between begin and a
	results.insert(results.end(), points.begin() + 1 + b_index, points.end());
	results.insert(results.end(), points.begin(), points.begin() + a_index);

	return results;
}

bool in_half_plane(std::vector<CGAL::Orientation> sides, Point_2 p, Point_2 q, Point_2 x) 
{
	bool result = false;
	for (int i = 0; i < sides.size(); i++)
	{
		CGAL::Orientation side = sides[i];
		if (side == CGAL::COLLINEAR)
			result = result || (CGAL::orientation(p, q, x) == side);
		//||
			//	CGAL::orientation(p, q, Point_2(x.x() + alpha, x.y() + alpha)) == side ||
				//CGAL::orientation(p, q, Point_2(x.x() + alpha, x.y() - alpha)) == side ||
				//CGAL::orientation(p, q, Point_2(x.x() - alpha, x.y() - alpha)) == side ||
				//CGAL::orientation(p, q, Point_2(x.x() - alpha, x.y() + alpha)) == side);
		else
				result = result || (CGAL::orientation(p, q, x) == side);
				//CGAL::orientation(p, q, Point_2(x.x() + alpha, x.y() + alpha)) == side &&
				//CGAL::orientation(p, q, Point_2(x.x() + alpha, x.y() - alpha)) == side &&
				//CGAL::orientation(p, q, Point_2(x.x() - alpha, x.y() - alpha)) == side &&
				//CGAL::orientation(p, q, Point_2(x.x() - alpha, x.y() + alpha)) == side);
	}
	return result;
}

bool segment_enters_HP_quad(std::vector<Point_2> quad_points, Point_2 p, Point_2 q, Point_2 a, Point_2 b, CGAL::Orientation side) 
{	
	std::vector<CGAL::Orientation> sides = { CGAL::LEFT_TURN, CGAL::COLLINEAR };
	bool z = in_half_plane({ side }, p, q, a);
	bool x = in_half_plane({ side }, p, q, b);
	bool y = in_half_plane(sides, quad_points[0], quad_points[1], a);
	bool g = in_half_plane(sides , quad_points[1], quad_points[2], a);
	bool e = in_half_plane(sides , quad_points[2], quad_points[3], a);
	bool f = in_half_plane(sides , quad_points[3], quad_points[0], b);
	bool c = in_half_plane(sides , quad_points[1], quad_points[2], b);
	bool d = in_half_plane(sides, quad_points[2], quad_points[3], b);

	return (
		in_half_plane({ side }, p, q, a) &&
		in_half_plane(sides, quad_points[3], quad_points[0], a) && in_half_plane(sides, quad_points[0], quad_points[1], a) &&
		in_half_plane(sides, quad_points[1], quad_points[2], a) && in_half_plane(sides, quad_points[2], quad_points[3], a)
			) ||
			(
		in_half_plane({ side }, p, q, b) &&
		in_half_plane(sides, quad_points[3], quad_points[0], b) && in_half_plane(sides, quad_points[0], quad_points[1], b) &&
		in_half_plane(sides, quad_points[1], quad_points[2], b) && in_half_plane(sides, quad_points[2], quad_points[3], b)
			);
}

std::tuple<Segment_2, int> polygon_exits(Ray_2 pq, std::vector<Point_2> points, Point_2 target, int def)
{
	std::vector<std::tuple<Segment_2, int>> candidates_and_indices = {};
	
	std::tuple<Segment_2, int> winner = std::make_tuple(Segment_2(pq.source(), pq.source()), def);

	for (int i = 0; i < points.size(); i++) {
		int prev = i - 1;
		if (i == 0)
			prev = points.size() - 1;

		Segment_2 sgm(points[prev], points[i]);
		Point_2 inter_point = point_intersection(pq, sgm, 2);

		double x = CGAL::to_double(inter_point.x());
		double y = CGAL::to_double(inter_point.y());

		if (isset(inter_point))
		{
			//is s the intersection point? (s = pq.source()), or did we check this already last iteration?
			if (same(pq.source(), inter_point) || same(points[prev], inter_point))
				continue;

			//are we hitting a vertex?
			if (same(inter_point, points[prev]) || same(inter_point, points[i]))
			{
				int next = (i + 1) % points.size();
				CGAL::Orientation or1 = CGAL::orientation(pq.source(), target, points[next]);
				CGAL::Orientation or2 = CGAL::orientation(pq.source(), target, points[prev]);
				//CGAL::Orientation or3 = CGAL::orientation(pq.source(), target, points[i]);

				if (or1 == or2)
				{
					// we never left the polygon
					continue;
				}
				if (or1 == CGAL::COLLINEAR  || (or2 == CGAL::COLLINEAR && or1 == CGAL::RIGHT_TURN))
					continue; //i is reflex vertex, we are still in!
			}	
			Segment_2 winning_smg = Segment_2(pq.source(), inter_point);

			double x = CGAL::to_double(winning_smg.squared_length());
			double y = CGAL::to_double(std::get<0>(winner).squared_length());

			//We are not intersecting in a vertex and thus leaving the polygon
			if (std::get<1>(winner) == def || winning_smg.squared_length() < std::get<0>(winner).squared_length())
				winner = std::make_tuple(winning_smg, prev);
			
		}
	}
	return winner;
}

std::tuple<int, int> find_rightmost_beam(std::vector<Point_2> points, int i)
{
	int R = 1, L = i + 1, r = R, l = L, n = points.size() - 1, side = 1;
	std::vector<Point_2> quad_points = { points[0],  points[1],  points[i],  points[i + 1] };
	Polygon_2 quad(quad_points.begin(), quad_points.end());
	Point_2 p = points[1];
	Point_2 q = points[i + 1];
	std::tuple<int, int> null_val = std::make_tuple(0, 0);

	while (r < i || l < n)
	{
		if (side == 1)
		{
			if (r < i) {
				r++;

				//does vr-1vr enter LHP(pq) inter quad?
				if (segment_enters_HP_quad(quad_points, p, q, points[r - 1], points[r], CGAL::LEFT_TURN))
				{
					if (isset(point_intersection(Line_2(points[r - 1], points[r]), Segment_2(points[L], q), 1)))
						return null_val;
					R = r, l = L;
				}
			}
		}
		else {
			if (l < n) {
				l++;
				//does vl-1vl enter RHP(pq) inter quad?
				if (segment_enters_HP_quad(quad_points, p, q, points[l - 1], points[l], CGAL::RIGHT_TURN))
				{
					if (isset(point_intersection(Line_2(points[l - 1], points[l]), Segment_2(points[R], p), 1)))
						return null_val;
					L = l, r = R;
				}
			}
		}

		Point_2 temp_p = point_intersection(Ray_2(points[L], points[R]), Segment_2(points[0], points[1]), 1);
		Point_2 temp_q = point_intersection(Ray_2(points[R], points[L]), Segment_2(points[i], points[i + 1]), 1);
		if (isset(temp_q) && isset(temp_p))
			p = temp_p, q = temp_q;
		else
			return null_val;
		side = -side;
	}

	Line_2 v0v1(points[0], points[1]);
	Line_2 vivi1(points[i], points[i + 1]);

	//Is pq in LHP v0v1 and LHP vivi1 (so within the polygon)?
	bool is_q_good = in_half_plane({ CGAL::LEFT_TURN, CGAL::COLLINEAR }, points[0], points[1], q);
	bool is_p_good = in_half_plane({ CGAL::LEFT_TURN, CGAL::COLLINEAR }, points[i], points[i + 1], p);
	bool is_vi1_good = in_half_plane({ CGAL::LEFT_TURN, CGAL::COLLINEAR }, points[0], points[1], points[i+1]);
	bool is_v1_good = in_half_plane({ CGAL::LEFT_TURN, CGAL::COLLINEAR }, points[i], points[i + 1], points[1]);

	is_vi1_good = is_vi1_good || !in_half_plane({ CGAL::RIGHT_TURN}, points[0], points[1], points[i + 1]);
	is_v1_good = is_vi1_good || !in_half_plane({ CGAL::RIGHT_TURN }, points[i], points[i+1], points[1]);

	//return std::make_tuple(R, L);

	if ((is_q_good && is_p_good) || (is_vi1_good && is_v1_good))
		return std::make_tuple(R, L);
	else
		return null_val;


}

Segment_2 point_to_edge_visibility(std::vector<Point_2> points, Point_2 p)
{
	Segment_2 empty_segment = Segment_2(empty_point, empty_point);
	int L = 0, R = 1, n = points.size();
	Point_2 a = points[0], b = points[1];

	//is p in LHP(v0v1)? if not, this subroutine does not work.
	if (!in_half_plane({ CGAL::LEFT_TURN, CGAL::COLLINEAR }, a, b, p))
		return empty_segment;


	for (int i = 1; i < n - 1; i++)
	{

		Point_2 i1 = points[(i + 1) % n];
		Segment_2 vivi1(points[i], i1);

		//triangle check
		if (in_half_plane({ CGAL::LEFT_TURN }, a, b, i1) && in_half_plane({ CGAL::LEFT_TURN }, b, p, i1) && in_half_plane({ CGAL::LEFT_TURN }, p, a, i1))
		{
			//if vivi1 crosses ap left to right (if there is an intersection, and if vi is in LHP(ap) and vi1 is in RHP(ap)
			if (in_half_plane({ CGAL::LEFT_TURN, CGAL::COLLINEAR }, a, p, points[i]) && in_half_plane({ CGAL::RIGHT_TURN, CGAL::COLLINEAR }, a, p, i1) && isset(point_intersection(vivi1, Segment_2(a, p))))
			{

				//is vi1 in RHP(bp)?
				if (in_half_plane({ CGAL::RIGHT_TURN, CGAL::COLLINEAR }, b, p, i1))
					return empty_segment;
				else
				{
					Point_2 pi = point_intersection(Ray_2(p, i1), Segment_2(points[0], points[1]));
					if (isset(pi)) {
						L = (i + 1) % n;
						a = point_intersection(Ray_2(p, i1), Segment_2(points[0], points[1]));
					}
				}
			}

			//if vivi1 crosses bp right to left (if there is an intersection, and if vi is in RHP(bp) and vi1 is in LHP(bp)
			if (in_half_plane({ CGAL::RIGHT_TURN, CGAL::COLLINEAR }, b, p, points[i]) && in_half_plane({ CGAL::LEFT_TURN, CGAL::COLLINEAR }, b, p, i1) && isset(point_intersection(vivi1, Segment_2(b, p))))
			{
				//is vi1 in LHP(ap)?
				if (in_half_plane({ CGAL::LEFT_TURN, CGAL::COLLINEAR }, a, p, i1))
					return empty_segment;
				else
				{
					Point_2 pi = point_intersection(Ray_2(p, i1), Segment_2(points[0], points[1]));
					if (isset(pi)) {
						R = (i + 1) % n;
						b = pi;
					}
				}
			}
		}
	}
	
	return Segment_2(a, b);	
}

std::vector<std::vector<Point_2>> split_for_vis(Point_2 a, Point_2 b, std::vector<Point_2>& original_points) {
	//find first intersections
	Point_2 a_prime = first_intersection(b,a, original_points);
	Point_2 b_prime = first_intersection(a, b, original_points);
	int a_index = -1, b_index = -1, a_prime_index = -1, b_prime_index = -1;
	bool a_b_col_seg = false;
	std::vector<int> vertices_hit = {};

	//pre process, add a,b, a_exit_point and b_exit_point as vertices to polygon!
	for (int i = 0; i < original_points.size(); i++) {
		Point_2 cur_point = original_points[i];
		Point_2 next_point = original_points[(i+1) % original_points.size()];
		Segment_2 cur_seg(cur_point, next_point);
		//if(cur_seg.collinear_has_on(a) && cur_seg.collinear_has_on

		//time saver?
		if (a_index > -1 && b_index > -1 && a_prime_index > -1 && b_prime_index > -1)
			break;

		//Vertex checks
		if (same(cur_point, a))  //a is at this vertex
			a_index = i;	
		else if (same(cur_point, b)) //b is at this vertex
			b_index = i;
		if (same(cur_point, a_prime))  //a exits at this vertex
			a_prime_index = i;
		else if (same(cur_point, b_prime)) //b exits at vertex
			b_prime_index = i;	

		//Segment checks
		bool a_between = in_between(cur_seg, a);
		bool a_x_between = in_between(cur_seg, a_prime);
		bool b_between = in_between(cur_seg, b);
		bool b_x_between = in_between(cur_seg, b_prime);

		if (a_between || a_x_between) {
			if (a_between && a_x_between) {
				//both, add only one vertex
				auto itPos = original_points.begin() + (i + 1);
				original_points.insert(itPos, a);
				a_index = i + 1;
				a_prime_index = i+1;
			}
			else if (a_between) {
				auto itPos = original_points.begin() + (i + 1);
				original_points.insert(itPos, a);
				a_index = i + 1;
			}
			else {
				auto itPos = original_points.begin() + (i + 1);
				original_points.insert(itPos, a_prime);
				a_prime_index = i + 1;
			}		
		}
		else if(b_between || b_x_between) {
			//add point in between
			if (b_between && b_x_between) {
				//both, add only one vertex
				auto itPos = original_points.begin() + (i + 1);
				original_points.insert(itPos, b);
				b_index = i + 1;
				b_prime_index = i + 1;
			}
			else if (b_between) {
				auto itPos = original_points.begin() + (i + 1);
				original_points.insert(itPos, b);
				b_index = i + 1;
			}
			else {
				auto itPos = original_points.begin() + (i + 1);
				original_points.insert(itPos, b_prime);
				b_prime_index = i + 1;
			}
		}	
		if(in_between(Segment_2(a,b),cur_point))
			vertices_hit.push_back(i);
	}

	/*if (a_prime_index == -1)
		a_prime_index = a_index;
	if(b_prime_index == -1)
		b_prime_index = b_index;*/

	if (b_prime_index == -1) {
		for (int i = 0; i < original_points.size();i++) {
			double x = CGAL::to_double(original_points[i].x());
			double y = CGAL::to_double(original_points[i].y());

			OutputDebugString(std::string("x_" + std::to_string(i) + " " + std::to_string(x) + "\n").c_str());
			OutputDebugString(std::string("y_" + std::to_string(i) + " " + std::to_string(y) + "\n\n").c_str());
		}

		int cirsb = 5;
	}

	bool last = (b_index == (original_points.size() - 1) && a_index == 0);
	bool will_last = (a_index == (original_points.size() - 1) && b_index == 0);
	int diff_ap_b = difference_in_polygon(a_prime_index, b_index, original_points.size());
	int diff_ap_bp = difference_in_polygon(a_prime_index, b_prime_index, original_points.size());
	
	int diff_b_ap = difference_in_polygon(b_index, a_prime_index, original_points.size());
	int diff_b_a = difference_in_polygon(b_index, a_index, original_points.size());

	//if ((a_index > b_index && a_index > -1 && b_index > -1 && !will_last) || last		
	//	||(b_index == -1 && a_index > -1)) {

	if((diff_ap_bp < diff_ap_b && a_index > -1 ) || (a_index > -1 && b_index == -1) || (b_index == b_prime_index && diff_b_a < diff_b_ap) )
	{
		if (a_index == -1 && b_index > -1) {
			//don't flip!
		}
		else {
			//reverse everything!
			int temp_a = a_index, temp_p = a_prime_index;
			Point_2 temp_ap = a, temp_pp = a_prime;
			a = b, a_prime = b_prime, a_index = b_index, a_prime_index = b_prime_index;
			b = temp_ap, b_index = temp_a, b_prime = temp_pp, b_prime_index = temp_p;
			diff_ap_b = difference_in_polygon(a_prime_index, b_index, original_points.size());
			diff_ap_bp = difference_in_polygon(a_prime_index, b_prime_index, original_points.size());
		}
	}


	//OutputDebugString(std::to_string(a_index).c_str());
	//OutputDebugString("\n");
	//OutputDebugString(std::to_string(a_prime_index).c_str());
	//OutputDebugString("\n");
	//OutputDebugString(std::to_string(b_index).c_str());
	//OutputDebugString("\n");
	//OutputDebugString(std::to_string(b_prime_index).c_str());
	//OutputDebugString("\n");
	//OutputDebugString("\n");

	//OutputDebugString((std::string("a x: ") + std::to_string(CGAL::to_double(a.x()))).c_str());
	//OutputDebugString("\n");
	//OutputDebugString((std::string("a y: ") + std::to_string(CGAL::to_double(a.y()))).c_str());
	//OutputDebugString("\n");
	//OutputDebugString((std::string("ap x: ") + std::to_string(CGAL::to_double(a_prime.x()))).c_str());
	//OutputDebugString("\n");
	//OutputDebugString((std::string("ap y: ") + std::to_string(CGAL::to_double(a_prime.y()))).c_str());
	//OutputDebugString("\n");
	//OutputDebugString("\n");

	//OutputDebugString((std::string("b x: ") + std::to_string(CGAL::to_double(b.x()))).c_str());
	//OutputDebugString("\n");
	//OutputDebugString((std::string("b y: ") + std::to_string(CGAL::to_double(b.y()))).c_str());
	//OutputDebugString("\n");
	//OutputDebugString((std::string("bp x: ") + std::to_string(CGAL::to_double(b_prime.x()))).c_str());
	//OutputDebugString("\n");
	//OutputDebugString((std::string("bp y: ") + std::to_string(CGAL::to_double(b_prime.y()))).c_str());
	//OutputDebugString("\n");
	//OutputDebugString("\n");

	std::vector<std::vector<Point_2>> pgns = {}, p_right = {}; 
	std::vector<Point_2> p_a = {}, p_left = {}, p_b = {}, p_r = {};
	std::vector<int> p_r_i, p_a_i, p_l_i, p_b_i, p_right_i;


	if (a_index > -1)
		vertices_hit.insert(vertices_hit.begin(), a_index);
	if (b_index > -1)
		vertices_hit.push_back(b_index);

	if (a_index != a_prime_index) {
		if (a_index > -1) { //a is reflex
			p_a = { a,a };
			p_a_i = { a_index, a_index };
			p_a = add_points_between(p_a, original_points, a_prime_index, a_index);
		}
		else { //a is floating
			if (b_index == -1) {
				//both floating
				p_r = { b, a };
				p_r = add_points_between(p_r, original_points, a_prime_index, b_prime_index);
				p_r.push_back(b_prime);
				p_right.push_back(p_r);

				p_left = { a,b};
				p_left = add_points_between(p_left, original_points, b_prime_index, a_prime_index);
				p_left.push_back(a_prime);
			}
			else {
				//only a floating
				if (b_prime_index == b_index || diff_ap_bp > diff_ap_b) {
					p_r = { b, a };
					p_r = add_points_between(p_r, original_points, a_prime_index, b_index);
					p_r.push_back(b);
					p_right.push_back(p_r);					

					p_left = { a, b };
					p_left = add_points_between(p_left, original_points, b_prime_index, a_prime_index);
					p_left.push_back(a_prime);
				}
				else {
					p_r = { b, a };
					p_r = add_points_between(p_r, original_points, a_prime_index, b_prime_index);
					p_r.push_back(b_prime);
					p_right.push_back(p_r);

					p_left = { a,b};
					p_left = add_points_between(p_left, original_points, b_index, a_prime_index);
					p_left.push_back(a_prime);
				}
			}
		}		
	}

	if (b_index != b_prime_index && b_index > -1) { //b is reflex
		if (diff_ap_b > diff_ap_bp) {
			p_b = { b, b};
			p_b = add_points_between(p_b, original_points, b_prime_index, b_index);
		}
		else {
			p_b = { b };
			p_b = add_points_between(p_b, original_points, b_index, b_prime_index);
			p_b.push_back(b_prime);
		}		
	}

	if (b_index > -1 && a_index > -1) {//neither floating:
		//p_r sorted by for loop
		int v_size = vertices_hit.size();
		for (int i = 0; i < (v_size - 1);i++) {
			int next = i + 1;
			p_r = { original_points[vertices_hit[next]], original_points[vertices_hit[i]] };
			p_r = add_points_between(p_r, original_points, vertices_hit[i], vertices_hit[next]);
			p_r.push_back(original_points[vertices_hit[next]]);
			if(p_r.size()>4)
				p_right.push_back(p_r);
		}
		//p_left simple
		p_left = { a};
		if (b_prime_index != b_index)
			p_left.push_back(b);

		p_left = add_points_between(p_left, original_points, b_prime_index, a_prime_index);
		if (a_prime_index != a_index)
			p_left.push_back(a_prime);

		//if(p_right.size() < 1)
			//p_left.push_back(a);
	}

	pgns.push_back(p_b);
	pgns.insert(pgns.end(), p_right.begin(), p_right.end());
	pgns.push_back(p_left);
	pgns.push_back(p_a);
	return pgns;
}

Arrangement_2 polygon_to_arrangement(std::vector<Point_2> poly) {
	Arrangement_2 arr = Arrangement_2();
	std::vector<Segment_2> segs = {};
	int n = poly.size();

	int crisb = -1;
	
	//Create segments from the vertices in the polygon
	for (int i = 0; i < poly.size(); i++) 
	{
		Segment_2 to_add(poly[i], poly[(i + 1) % n]);
		//segs.push_back(to_add);
		if (segs.size() == 0)
			segs.push_back(to_add);
		else {
			Segment_2 prev = segs[segs.size() - 1];
			if (prev.has_on(poly[(i + 1) % n])) {
				//do not add this segment, and remove previous!
				segs.pop_back();
				to_add = Segment_2(poly[i - 1], poly[(i + 1) % n]);
			}
			else if(to_add.has_on(prev.source())) {						
				//do not add this segment, and remove previous!
				segs.pop_back();
				to_add = Segment_2(poly[i - 1], poly[(i + 1) % n]);
			}	
			segs.push_back(to_add);
		}
	}
	std::vector<Point_2> new_poly;
	for (int i = 0; i < segs.size(); i++) {
		int next = (i + 1) % segs.size();
		if(!same(segs[next].source(), segs[i].target()))
			throw(std::exception());

		new_poly.push_back(segs[i].source());
	}


	if (!CGAL::is_simple_2(new_poly.begin(), new_poly.end())) {
		throw(std::exception());
	}

	//Insert the segments into the arrangement
	CGAL::insert_non_intersecting_curves(arr, segs.begin(), segs.end());
	
	
	return arr;
}

Arrangement_2 rotate_arrangement(Arrangement_2 original)
 {
	
	Face_handle fh;
	for (fh = original.faces_begin(); fh != original.faces_end(); ++fh) {
		if (fh->has_outer_ccb())
			break;
	}
	Arrangement_2::Ccb_halfedge_circulator curr = fh->outer_ccb();

	//rotate 90 degrees
	std::vector<Point_2> poly = {};
	poly.push_back(Point_2(-curr->source()->point().y(), curr->source()->point().x()));
	while (++curr != fh->outer_ccb()) {
		poly.push_back(Point_2(-curr->source()->point().y(), curr->source()->point().x()));
	}
	Arrangement_2 arr = polygon_to_arrangement(poly);
	std::vector<Halfedge_handle> original_edges = {};

	for (fh = arr.faces_begin(); fh != arr.faces_end(); ++fh) {
		if (fh->has_outer_ccb())
			break;
	}
	curr = fh->outer_ccb();
	original_edges.push_back(curr);
	while (++curr != fh->outer_ccb()) {
		original_edges.push_back(curr);
	}

	return arr;
}

std::vector<Segment_2> shoot_rays(std::vector<Point_2> sources, std::vector<Point_2> targets, std::vector<Point_2> next, Arrangement_2 arr, int mode) {
	std::vector<Arrangement_2> rotated_arrs = {};

	if (mode == 0) { //all directions
		rotated_arrs.push_back(arr);
		for (int i = 1; i < 4;i++)
			rotated_arrs.push_back(rotate_arrangement(rotated_arrs[i - 1]));
	}
	else if (mode == 1) {
		rotated_arrs.push_back(Arrangement_2()); //placeholder
		//horizontal
		rotated_arrs.push_back(rotate_arrangement(arr)); //shoot right
		Arrangement_2 down = rotate_arrangement(rotated_arrs.back());
		rotated_arrs.push_back(Arrangement_2()); //placeholder
		rotated_arrs.push_back(rotate_arrangement(down)); //shoot left
	}
	else {
		//vertical
		rotated_arrs.push_back(arr); //shoot up
		rotated_arrs.push_back(Arrangement_2()); //placeholder
		Arrangement_2 right = rotate_arrangement(arr);
		rotated_arrs.push_back(rotate_arrangement(right)); //shoot down
		rotated_arrs.push_back(Arrangement_2()); //placeholder
	}
	std::vector<Segment_2> segs = {};

	for(int i =0; i<sources.size();i++){
		for (int j = 0; j < 4;j++) {
			Point_2 q = targets[i];
			Point_2 src = sources[i];
			Point_2 next_q = next[i];

			for (int k = 0; k < j; k++) {
				q = Point_2(-q.y(), q.x());

				if (isset(src)) {
					src = Point_2(-src.y(), src.x());
					next_q = Point_2(-next_q.y(), next_q.x());
				}
			}

			if (rotated_arrs[j].number_of_vertices() < 2)
				continue; //this is a placeholder

			Walk_pl walk_pl(rotated_arrs[j]);

			//Shoot ray up
			Arrangement_2::Vertex_const_handle v;
			Arrangement_2::Halfedge_const_handle he;
			Point_2 hit_point = empty_point;

			//if we will shoot a ray that is parallel to seg(src, targ) or seg(targ, next) we will encounter an error
			if (src.y() > q.y() && src.x() == q.x()) //it is straight above us, abort
				continue;
			if (next_q.y() > q.y() && next_q.x() == q.x()) //it is straight above us, abort
				continue;

			CGAL::Object obj = walk_pl.ray_shoot_up(q);
			if (CGAL::assign(v, obj))
				hit_point = v->point();
			else if (CGAL::assign(he, obj))
			{
				Segment_2 curb = rotated_arrs[j].non_const_handle(he)->curve();
					//double x = CGAL::to_double(q.x());
					//double x_1 = CGAL::to_double(curb.source().x());
					//double x_2 = CGAL::to_double(curb.target().x());
					//double y_1 = CGAL::to_double(curb.source().y());
					//double y_2 = CGAL::to_double(curb.target().y());

					//double y = (x - x_2) * (y_1 - y_2) / (x_1 - x_2) + y_2;

				auto y_val = curb.source().y();
				Line_2 ln = curb.supporting_line();
				if (ln.is_vertical()) 
					y_val = curb.source().y();
				else
					y_val = ln.y_at_x(q.x());

				hit_point = Point_2(q.x(), y_val);
			}		
			if (isset(hit_point))
			{
				if (!isset(src) || CGAL::orientation(src, q, hit_point) == CGAL::LEFT_TURN || CGAL::orientation(q, next_q, hit_point) == CGAL::LEFT_TURN) {
					//arr.insert_at_vertices(res, curr, arr.non_const_handle(v));

					Segment_2 res = Segment_2(q, hit_point);
					for (int k = 0; k < j;k++)
						res = Segment_2(Point_2(res.source().y(), -res.source().x()), Point_2(res.target().y(), -res.target().x()));
					segs.push_back(res);
				}
			}
		}

	}

	return segs;


}

std::vector<Point_2> random_poly(int n) {
	Polygon_2            polygon;
	std::vector<Point_2>   point_set;
	std::vector<Point_2>   point_set_temp;

	CGAL::Random         rand;
	int size = n+10;

	// copy size points from the generator, eliminating duplicates, so the
// polygon will have <= size vertices
	CGAL::copy_n_unique(Point_generator(50), size, std::back_inserter(point_set_temp));
	
	for (int i = 0; i < point_set_temp.size();i++)
		point_set.push_back(Point_2(point_set_temp[i].x() + 50, point_set_temp[i].y() + 50));

	CGAL::random_polygon_2(point_set.size(), std::back_inserter(polygon),
		point_set.begin());

	return polygon.container();
}