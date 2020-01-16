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

typedef CGAL::Exact_predicates_exact_constructions_kernel K;
typedef K::Point_2 Point_2;
typedef CGAL::Polygon_2<K> Polygon_2;
typedef CGAL::Segment_2<K> Segment_2;
typedef CGAL::Line_2<K> Line_2;
typedef CGAL::Ray_2<K> Ray_2;
typedef K::Intersect_2 Intersect_2;

extern double alpha;

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

	
		if (point_isset(inter_point))
		{
			//is s the intersection point? (s = pq.source()) 
			if (compare_points_true(pq.source(), inter_point))
				continue;


			//are we hitting a vertex?
			if (compare_points_true(inter_point, points[prev]) || compare_points_true(inter_point, points[i]))
			{
				double source_x = CGAL::to_double(pq.source().x());
				double source_y = CGAL::to_double(pq.source().y());
				double target_x = CGAL::to_double(target.x());
				double target_y = CGAL::to_double(target.y());
				double point_x = CGAL::to_double(points[0].x());
				double point_y = CGAL::to_double(points[0].y());



				int next = (i + 1) % points.size();
				CGAL::Orientation or1 = CGAL::orientation(pq.source(), target, points[next]);
				CGAL::Orientation or2 = CGAL::orientation(pq.source(), target, points[prev]);
				if (or1 == or2)
				{
					// we never left the polygon
					i++;
					continue;
				}
			}			


			


			Segment_2 winning_smg = Segment_2(pq.source(), inter_point);

			//We are not intersecting in a vertex and thus leaving the polygon
			if (std::get<1>(winner) == def || winning_smg.squared_length() < std::get<0>(winner).squared_length())
				winner = std::make_tuple(winning_smg, prev);

			//if(compare_points_true(std::get<0>(candidates_and_indices.back()).target(), inter_point))
				//continue;	

			//if (i == points.size() - 1 && inter_point == points[i])
				//continue;
		}
	}


	//sort from smallest to highest length, and then check if every segment leaves the polygon (n log n)
	//std::sort(candidates_and_indices.begin(), candidates_and_indices.end(), [](const std::tuple<Segment_2, int> lhs, const std::tuple<Segment_2, int> rhs) {
		//return std::get<0>(lhs).squared_length() < std::get<0>(rhs).squared_length();
	//});
	return winner;
}

std::tuple<int, int> find_rightmost_beam(Polygon_2 pgn, int i)
{
	int R = 1, L = i + 1, r = R, l = L, n = pgn.size() - 1, side = 1;
	std::vector<Point_2> quad_points = { pgn.container()[0],  pgn.container()[1],  pgn.container()[i],  pgn.container()[i + 1] };
	Polygon_2 quad(quad_points.begin(), quad_points.end());
	Point_2 p = pgn.container()[1];
	Point_2 q = pgn.container()[i + 1];
	std::tuple<int, int> null_val = std::make_tuple(0, 0);

	while (r < i || l < n)
	{
		if (side == 1)
		{
			if (r < i) {
				r++;

				//does vr-1vr enter LHP(pq) inter quad?
				if (segment_enters_HP_quad(quad_points, p, q, pgn.container()[r - 1], pgn.container()[r], CGAL::LEFT_TURN))
				{
					if (point_isset(point_intersection(Line_2(pgn.container()[r - 1], pgn.container()[r]), Segment_2(pgn.container()[L], q), 1)))
						return null_val;
					R = r, l = L;
				}
			}
		}
		else {
			if (l < n) {
				l++;
				//does vl-1vl enter RHP(pq) inter quad?
				if (segment_enters_HP_quad(quad_points, p, q, pgn.container()[l - 1], pgn.container()[l], CGAL::RIGHT_TURN))
				{
					if (point_isset(point_intersection(Line_2(pgn.container()[l - 1], pgn.container()[l]), Segment_2(pgn.container()[R], p), 1)))
						return null_val;
					L = l, r = R;
				}
			}
		}

		Point_2 temp_p = point_intersection(Ray_2(pgn.container()[L], pgn.container()[R]), Segment_2(pgn.container()[0], pgn.container()[1]), 1);
		Point_2 temp_q = point_intersection(Ray_2(pgn.container()[R], pgn.container()[L]), Segment_2(pgn.container()[i], pgn.container()[i + 1]), 1);
		if (point_isset(temp_q) && point_isset(temp_p))
			p = temp_p, q = temp_q;
		else
			return null_val;
		side = -side;
	}

	Line_2 v0v1(pgn.container()[0], pgn.container()[1]);
	Line_2 vivi1(pgn.container()[i], pgn.container()[i + 1]);

	//Is pq in LHP v0v1 and LHP vivi1 (so within the polygon)?
	bool is_q_good = in_half_plane({ CGAL::LEFT_TURN, CGAL::COLLINEAR }, pgn.container()[0], pgn.container()[1], q);
	bool is_p_good = in_half_plane({ CGAL::LEFT_TURN, CGAL::COLLINEAR }, pgn.container()[i], pgn.container()[i + 1], p);
	bool is_vi1_good = in_half_plane({ CGAL::LEFT_TURN, CGAL::COLLINEAR }, pgn.container()[0], pgn.container()[1], pgn.container()[i+1]);
	bool is_v1_good = in_half_plane({ CGAL::LEFT_TURN, CGAL::COLLINEAR }, pgn.container()[i], pgn.container()[i + 1], pgn.container()[1]);

	is_vi1_good = is_vi1_good || !in_half_plane({ CGAL::RIGHT_TURN}, pgn.container()[0], pgn.container()[1], pgn.container()[i + 1]);
	is_v1_good = is_vi1_good || !in_half_plane({ CGAL::RIGHT_TURN }, pgn.container()[i], pgn.container()[i+1], pgn.container()[1]);

	//return std::make_tuple(R, L);

	if ((is_q_good && is_p_good) || (is_vi1_good && is_v1_good))
		return std::make_tuple(R, L);
	else
		return null_val;


}

Segment_2 point_to_edge_visibility(Polygon_2 pgn, Point_2 p)
{
	std::vector<Point_2> points = pgn.container();
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
			if (in_half_plane({ CGAL::LEFT_TURN, CGAL::COLLINEAR }, a, p, points[i]) && in_half_plane({ CGAL::RIGHT_TURN, CGAL::COLLINEAR }, a, p, i1) && point_isset(point_intersection(vivi1, Segment_2(a, p))))
			{

				//is vi1 in RHP(bp)?
				if (in_half_plane({ CGAL::RIGHT_TURN, CGAL::COLLINEAR }, b, p, i1))
					return empty_segment;
				else
				{
					Point_2 pi = point_intersection(Ray_2(p, i1), Segment_2(points[0], points[1]));
					if (point_isset(pi)) {
						L = (i + 1) % n;
						a = point_intersection(Ray_2(p, i1), Segment_2(points[0], points[1]));
					}
				}
			}

			//if vivi1 crosses bp right to left (if there is an intersection, and if vi is in RHP(bp) and vi1 is in LHP(bp)
			if (in_half_plane({ CGAL::RIGHT_TURN, CGAL::COLLINEAR }, b, p, points[i]) && in_half_plane({ CGAL::LEFT_TURN, CGAL::COLLINEAR }, b, p, i1) && point_isset(point_intersection(vivi1, Segment_2(b, p))))
			{
				//is vi1 in LHP(ap)?
				if (in_half_plane({ CGAL::LEFT_TURN, CGAL::COLLINEAR }, a, p, i1))
					return empty_segment;
				else
				{
					Point_2 pi = point_intersection(Ray_2(p, i1), Segment_2(points[0], points[1]));
					if (point_isset(pi)) {
						R = (i + 1) % n;
						b = pi;
					}
				}
			}
		}
	}
	
	return Segment_2(a, b);	
}

std::tuple < std::vector<Polygon_2>, Point_2, Point_2, std::vector<Point_2>> extend_and_split(Point_2 a, Point_2 b, std::vector<Point_2> original_points)
{
	std::vector<Polygon_2> results = {};
	Point_2 to_add_1 = a;
	Point_2 to_add_2 = b;
	std::vector<Point_2> extra_points = { empty_point , empty_point , empty_point , empty_point};

	bool a_is_original = false;
	bool b_is_original = false;
	int a_original_index = -1;
	int b_original_index = -1;
	bool a_original_concave = false;
	bool b_original_concave = false;
	std::vector<int> indices_crossed = {};

	Point_2 a_prime = empty_point;
	Point_2 b_prime = empty_point;
	int a_prime_target = -1;
	int b_prime_target = -1;
	bool a_on_original_seg = false;
	bool b_on_original_seg = false;
	int n = original_points.size();

	int a_on_original_target = -1;
	int a_on_original_prev = -1;
	int b_on_original_target = -1;
	int b_on_original_prev = -1;

	//check if a and b are vertices from the original polygon and save index
	for (int i = 0; i < original_points.size();i++) {
		if (!a_is_original && compare_points_true(a, original_points[i]))
		{
			a_is_original = true;
			a_original_index = i;

			std::vector<Point_2> temp_pts = { original_points[(((i - 1) % n) + n) % n],original_points[i], original_points[(i+1) % n]};
			CGAL::Orientation o = Polygon_2(temp_pts.begin(), temp_pts.end()).orientation();
			a_original_concave = o != CGAL::POSITIVE;

		}
		else if (!b_is_original && compare_points_true(b, original_points[i]))
		{
			b_is_original = true;
			b_original_index = i;

			std::vector<Point_2> temp_pts = { original_points[(((i - 1) % n) + n) % n],original_points[i], original_points[(i + 1) % n] };
			CGAL::Orientation o = Polygon_2(temp_pts.begin(), temp_pts.end()).orientation();
			b_original_concave = o != CGAL::POSITIVE;
		}
		if (a_is_original && b_is_original)
			break;
	}

	//try to see if its on an original segment
	for (int i = 0; i < original_points.size();i++)
	{
		if (original_points[i] != a & original_points[i] != b)
			if (Segment_2(a, b).has_on(original_points[i]))
				indices_crossed.push_back(i);

		int prev = (((i - 1) % n) + n) % n;
		bool sna = Segment_2(original_points[prev], original_points[i]).has_on(a);

		if (!a_on_original_seg)
		{			
			a_on_original_seg = original_points[i] != a && Segment_2(original_points[prev], original_points[i]).has_on(a);
			if (a_on_original_seg)
			{
				a_on_original_prev = prev;
				a_on_original_target = i;
			}

		}
		if (!b_on_original_seg)
		{
			b_on_original_seg = original_points[i] != a && Segment_2(original_points[prev], original_points[i]).has_on(b);
			if (b_on_original_seg)
			{
				b_on_original_prev = prev;
				b_on_original_target = i;
			}
		}
	}

	//try to extend and see if it intersects with any
	for (int i = 0; i < original_points.size();i++)
	{

		int prev = (((i - 1) % n) + n) % n;
		Segment_2 sgm(original_points[prev], original_points[i]);
		Line_2 vivi1 = Line_2(sgm);

		//This might cause trouble! If theres a glitch, check here first!

		if (prev == a_original_index || prev == b_original_index || i == a_original_index || i == b_original_index)
			continue;

		Point_2 inter_point = point_intersection(Ray_2(a, b), sgm, 1);

		//We need to extend it?
		if (point_isset(inter_point) && (!b_on_original_seg || b_is_original) && (!b_is_original || b_original_concave) && (!Segment_2(a,b).has_on(inter_point) || inter_point == b))
		{
			if (point_isset(b_prime))
			{
				Segment_2 old_segment(b, b_prime);
				Segment_2 new_segment(b, inter_point);

				if (old_segment.squared_length() <= new_segment.squared_length())
					continue;
			}

			//check if midpoint still inside polygon!
			Point_2 mp = CGAL::midpoint(b, inter_point);
			if (CGAL::bounded_side_2(original_points.begin(), original_points.end(), mp) == CGAL::ON_BOUNDED_SIDE) {
				b_prime = inter_point;
				b_prime_target = i;
			}
		}
		inter_point = point_intersection(Ray_2(b, a), sgm, 1);
		if (point_isset(inter_point) && (!a_on_original_seg || a_is_original) && (!a_is_original || a_original_concave && inter_point != a) && (!Segment_2(a, b).has_on(inter_point) || inter_point == a))
		{ 
			if (point_isset(a_prime))
			{
				Segment_2 old_segment(a, a_prime);
				Segment_2 new_segment(a, inter_point);
				if (old_segment.squared_length() <= new_segment.squared_length())
					continue;
			}

			//check if midpoint still inside polygon!
			Point_2 mp = CGAL::midpoint(a, inter_point);
			if (CGAL::bounded_side_2(original_points.begin(), original_points.end(), mp) == CGAL::ON_BOUNDED_SIDE) {
				double x1 = to_double(a.x());
				double x2 = to_double(inter_point.x());
				double y1 = to_double(a.y());
				double y2 = to_double(inter_point.y());

				a_prime = inter_point;
				a_prime_target = i;				
			}
		}
	}
		
	//Both are original and convex but we have to make sure that a and b are v0 and v1 (1 polygon, or two if the indices are not next to eachother!)
	if (a_is_original && b_is_original && !point_isset(a_prime) && !point_isset(b_prime))
	{
		if ((a_original_index + 1) % n == b_original_index)
		{
			std::vector<Point_2> p1 = orientate_pgn(original_points, a_original_index, b_original_index);
			results.push_back(Polygon_2(p1.begin(), p1.end()));
		}
		else
		{
			std::vector<Point_2> p1 = {};
			std::vector<Point_2> p2 = {};

			to_add_1 = empty_point;
			to_add_2 = empty_point;

			p1.push_back(a);
			p1.push_back(b);

			p1 = add_points_between(p1, original_points, b_original_index, a_original_index);

			p2.push_back(b);
			p2.push_back(a);

			p2 = add_points_between(p2, original_points, a_original_index, b_original_index);

			results.push_back(Polygon_2(p1.begin(), p1.end()));
			results.push_back(Polygon_2(p2.begin(), p2.end()));
		}
	}
	
	//We are original on the b side and concave on the a side, and only a_prime is set! (3 polygons)
	else if (a_original_concave && (b_is_original || b_on_original_seg) && point_isset(a_prime) && !point_isset(b_prime)) {

		std::vector<Point_2> p1 = {};
		std::vector<Point_2> p2 = {};
		std::vector<Point_2> p3 = {};

		//p1 goes b, a, a to b
		p1.push_back(b);
		if(b_is_original)
			p1 = add_points_between(p1, original_points, a_original_index, b_original_index);
		else
			p1 = add_points_between(p1, original_points, a_original_index, b_on_original_target);

		//p2 goes a, b, a_prime
		p2.push_back(a);
		if (b_is_original)
			p2 = add_points_between(p2, original_points, b_original_index, a_prime_target);
		else
		{
			p2.push_back(b);
			p2 = add_points_between(p2, original_points, b_on_original_target, a_prime_target);
		}
		p2.push_back(a_prime);

		//p3 goes a,a, a_prime to a
		p3.push_back(a);
		p3.push_back(a);
		p3.push_back(a_prime);
		p3 = add_points_between(p3, original_points, a_prime_target, a_original_index);

		extra_points[0] = a;
		extra_points[1] = b;
		to_add_1 = empty_point;
		to_add_2 = empty_point;

		results.push_back(Polygon_2(p1.begin(), p1.end()));
		results.push_back(Polygon_2(p2.begin(), p2.end()));
		results.push_back(Polygon_2(p3.begin(), p3.end()));
	}

	//We are original on the a side and concave on the b side, but only b_prime is set! (3 polygons)
	else if (b_original_concave && (a_is_original || a_on_original_seg) && !point_isset(a_prime) && point_isset(b_prime)) {

		std::vector<Point_2> p1 = {};
		std::vector<Point_2> p2 = {};
		std::vector<Point_2> p3 = {};

		//p1 goes a, b, b_prime to a
		p1.push_back(a);
		p1.push_back(b);
		p1.push_back(b_prime);

		if(a_is_original)
			p1 = add_points_between(p1, original_points, b_prime_target, a_original_index);
		else
			p1 = add_points_between(p1, original_points, b_prime_target, a_on_original_target);

		//p2 goes b, a to b
		p2.push_back(b);

		if (a_is_original)
			p2 = add_points_between(p2, original_points, a_original_index, b_original_index);
		else
		{
			p2.push_back(a);
			p2 = add_points_between(p2, original_points, a_on_original_target, b_original_index);
		}

		//p3 goes b,b, to b_prime
		p3.push_back(b);
		p3 = add_points_between(p3, original_points, b_original_index, b_prime_target);
		p3.push_back(b_prime);

		extra_points[0] = b;
		extra_points[2] = a;
		to_add_1 = empty_point;
		to_add_2 = empty_point;

		results.push_back(Polygon_2(p3.begin(), p3.end()));
		results.push_back(Polygon_2(p1.begin(), p1.end()));
		results.push_back(Polygon_2(p2.begin(), p2.end()));
	}

		
	//We are concave/floating on the a side! (2 polygons)
	else if ((b_is_original || b_on_original_seg) && ((!a_is_original && !a_on_original_seg) || (a_is_original && a_on_original_seg)) && point_isset(a_prime) && !point_isset(b_prime))
	{
		std::vector<Point_2> p1 = {}; //the concave end
		std::vector<Point_2> p2 = {}; //the non concave end

		//p2 goes a, b, all points from b to a'
		p2.push_back(a);

		//If a is not in the original polygon, add the vertex of the segment that a is on.
		if (!a_is_original)
		{
			p2.push_back(b);
			p2 = add_points_between(p2, original_points, b_on_original_target, a_prime_target);
		}
		else if (b_is_original)
			p2 = add_points_between(p2, original_points, b_original_index, a_prime_target);
		else //a is original
		{
			p2.push_back(b);
			p2 = add_points_between(p2, original_points, b_on_original_target, a_prime_target);
		}

		p2.push_back(a_prime);

		if (a_is_original)
		{
			//we are not floating
			//p1 goes a a a'
			p1.push_back(a);
			p1.push_back(a);

			if (!compare_points_true(a_prime, original_points[a_prime_target]))
				p1.push_back(a_prime);
			else
				p1.push_back(original_points[(((a_prime_target - 1) % n) + n) % n]);

			//all points between a prime and a
			p1 = add_points_between(p1, original_points, a_prime_target, a_original_index);
		}
		else
		{
			//we are floating
			//check which vertex is concave (whichever segment is longer)
			Segment_2 b_prev(original_points[b_on_original_prev], a);
			Segment_2 b_target(original_points[b_on_original_target], a);

			if (b_prev.squared_length() > b_target.squared_length())
			{
				p1.push_back(original_points[b_on_original_target]);
				p1.push_back(a);
				if (!compare_points_true(a_prime, original_points[a_prime_target]))
					p1.push_back(a_prime);

				//p1.push_back(original_points[b_on_original_target]);
				p1 = add_points_between(p1, original_points, a_prime_target, b_on_original_target);
				to_add_1 = original_points[b_on_original_target];
			}
			else
			{
				p1.push_back(original_points[b_on_original_prev]);
				p1.push_back(a);
				if (!compare_points_true(a_prime, original_points[a_prime_target]))
					p1.push_back(a_prime);
				//p1.push_back(original_points[b_on_original_prev]);
				p1 = add_points_between(p1, original_points, a_prime_target, b_on_original_prev);
				to_add_1 = original_points[b_on_original_prev];
			}
		}
		results.push_back(Polygon_2(p2.begin(), p2.end()));
		results.push_back(Polygon_2(p1.begin(), p1.end()));

	}

	//We are concave/floating on the b side! (2 polygons)
	else if ((a_is_original || a_on_original_seg) && ((!b_is_original && !b_on_original_seg) || (b_is_original && b_on_original_seg)) && point_isset(b_prime) && !point_isset(a_prime))
	{
		std::vector<Point_2> p1 = {}; //the concave end
		std::vector<Point_2> p2 = {}; //the non concave end

		//p2 goes a, b,  b', all points from b' to a
		p2.push_back(a);
		p2.push_back(b);

		if (!compare_points_true(b_prime, original_points[b_prime_target]))
			p2.push_back(b_prime);

		//If a is not in the original polygon, add the vertex of the segment that a is on.
		if (!a_is_original)
			p2 = add_points_between(p2, original_points, b_prime_target, a_on_original_target);
		else
			p2 = add_points_between(p2, original_points, b_prime_target, a_original_index);


		if (b_is_original)
		{
			//we are not floating
			//p1 goes b b '
			p1.push_back(b);

			//all points between b and b_prime
			p1 = add_points_between(p1, original_points, b_original_index, b_prime_target);
		}
		else
		{
			//we are floating
			//check which vertex is concave (whichever segment is longer)
			Segment_2 a_prev(original_points[a_on_original_prev], b);
			Segment_2 a_target(original_points[a_on_original_target], b);
			p1.push_back(b);
			if (a_prev.squared_length() > a_target.squared_length() && Segment_2(a,b).has_on(original_points[a_on_original_target]))
			{
				p1 = add_points_between(p1, original_points, a_on_original_target, b_prime_target);
				to_add_2 = original_points[a_on_original_target];
			}
			else if(Segment_2(a, b).has_on(original_points[a_on_original_prev]))
			{
				p1 = add_points_between(p1, original_points, a_on_original_prev, b_prime_target);
				to_add_2 = original_points[a_on_original_prev];
			}
			else
			{
				p1.push_back(a);
				p1 = add_points_between(p1, original_points, a_on_original_target, b_prime_target);
				to_add_2 = a;
			}
		}

		p1.push_back(b_prime);
		results.push_back(Polygon_2(p1.begin(), p1.end()));
		results.push_back(Polygon_2(p2.begin(), p2.end()));

	}

	//We are inside an original segment (1 polygon)
	else if ((!a_is_original || !b_is_original) && a_on_original_seg && b_on_original_seg && ((a_on_original_target == b_on_original_target) || a_is_original) && (!point_isset(a_prime) || !point_isset(b_prime)))
	{
		std::vector<Point_2> p1 = {};
		p1.push_back(a);
		p1.push_back(b);

		if (a_is_original)
			p1 = add_points_between(p1, original_points, b_on_original_target, b_on_original_prev);
		else if (b_is_original)
		{
			p1 = add_points_between(p1, original_points, b_on_original_target + 1, a_on_original_prev);
			p1.push_back(original_points[a_on_original_prev]);
		}
		else
		{
			p1 = add_points_between(p1, original_points, b_on_original_target, a_on_original_prev);
			p1.push_back(original_points[a_on_original_prev]);
		}

		results.push_back(Polygon_2(p1.begin(), p1.end()));
	}

	//We are floating on both sides (2 polygons)
	else if ((!a_is_original && !b_is_original) && ((point_isset(a_prime) || a_on_original_seg) && point_isset(b_prime) || b_on_original_seg) && indices_crossed.size() == 0)
	{
		if (a_prime_target == 0)
			a_prime_target = original_points.size() - 1;
		
		int ap_target = -1;
		int bp_target = -1;
		if (point_isset(b_prime))
			bp_target = b_prime_target;
		else
			bp_target = b_on_original_target;

		if (point_isset(a_prime))
			ap_target = a_prime_target;
		else
			ap_target = a_on_original_target;


		//First polygon starts with a,b,b' ... a'
		std::vector<Point_2> p1 = {};
		p1.push_back(a);
		p1.push_back(b);

		if (point_isset(b_prime) && !compare_points_true(b_prime, original_points[b_prime_target]))
			p1.push_back(b_prime);

		p1 = add_points_between(p1, original_points, bp_target, ap_target);

		if (point_isset(a_prime))
			p1.push_back(a_prime);


		//Second polygon starts with b, a, a_prime... b_prime
		std::vector<Point_2> p2 = {};
		p2.push_back(b);
		p2.push_back(a);

		if (point_isset(a_prime) && !compare_points_true(a_prime, original_points[ap_target]))
			p2.push_back(a_prime);

		//add all points from a_target to end and from beginning to b_target
		p2 = add_points_between(p2, original_points, ap_target, bp_target);

		if(point_isset(b_prime))
			p2.push_back(b_prime);

		results.push_back(Polygon_2(p1.begin(), p1.end()));
		results.push_back(Polygon_2(p2.begin(), p2.end()));

		to_add_1 = empty_point;
		to_add_2 = empty_point;
	}

	//We are concave on both sides and (must be) original! (3 polygons)
	else if (a_is_original && b_is_original && point_isset(a_prime) && point_isset(b_prime) && a_original_concave && b_original_concave && (a_original_index + 1 % 13) == b_original_index)
	{
		std::vector<Point_2> p1 = {};
		std::vector<Point_2> p2 = {};
		std::vector<Point_2> p3 = {};

		//p1 goes b, points from b to b_prime, b_prime
		p1.push_back(b);
		p1 = add_points_between(p1, original_points, b_original_index, b_prime_target);
		p1.push_back(b_prime);

		//p2 goes a, b, b_prime, all points from b_prime_target to a_prime_target
		p2.push_back(a);
		p2.push_back(b);
		p2.push_back(b_prime);
		p2 = add_points_between(p2, original_points, b_prime_target, a_prime_target);

		//p3 goes goes a, a, a_prime, all poitns from a_prime to a
		p3.push_back(a);
		p3.push_back(a);
		p3.push_back(a_prime);
		p3 = add_points_between(p3, original_points, a_prime_target, a_original_index);

		results.push_back(Polygon_2(p1.begin(), p1.end()));
		results.push_back(Polygon_2(p2.begin(), p2.end()));
		results.push_back(Polygon_2(p3.begin(), p3.end()));

	}
	
	//We are floating on the a side, but b is concave! (3 polygons)
	else if (b_is_original && !a_is_original && point_isset(a_prime) && b_original_concave && indices_crossed.size() == 1 && indices_crossed[0] == (((b_original_index - 1) % n) + n) % n)
	{
		std::vector<Point_2> p1 = {}; //the concave end
		std::vector<Point_2> p2 = {}; //the non concave end
		std::vector<Point_2> p3 = {}; //the non concave end

		//p1 goes b, b to b_prime
		p1.push_back(b);
		p1 = add_points_between(p1, original_points, b_original_index, b_prime_target);
		p1.push_back(b_prime);

		//p2 goes a, b, b', all points from b_prime to a_prime
		p2.push_back(a);
		p2.push_back(b);
		p2.push_back(b_prime);
		p2 = add_points_between(p2, original_points, b_prime_target, a_prime_target);
		p2.push_back(a_prime);

		//p3 goes from b's original index - 1 to a, a_prime to b's original index-1
		p3.push_back(original_points[(((b_original_index - 1) % n) + n) % n]);
		p3.push_back(a);
		p3.push_back(a_prime);
		p3 = add_points_between(p3, original_points, a_prime_target, (((b_original_index - 1) % n) + n) % n);
		
		results.push_back(Polygon_2(p1.begin(), p1.end()));
		results.push_back(Polygon_2(p2.begin(), p2.end()));
		results.push_back(Polygon_2(p3.begin(), p3.end()));

		to_add_1 = original_points[(((b_original_index - 1) % n) + n) % n];

	}

	//We are floating on the b side, but a is concave! (3 polygons)
	else if (a_is_original && !b_is_original && point_isset(a_prime) && a_original_concave && indices_crossed.size() == 1 && indices_crossed[0] == (a_original_index + 1) % n)
	{
		std::vector<Point_2> p1 = {}; //the concave end
		std::vector<Point_2> p2 = {}; //the non concave end
		std::vector<Point_2> p3 = {}; //the non concave end

		//p1 goes a,a a_prime to a
		p1.push_back(a);
		p1.push_back(a);
		p1.push_back(a_prime);
		p1 = add_points_between(p1, original_points, a_prime_target, a_original_index);
		
		//p2 goes b, a next index to b_prime
		p2.push_back(b);
		p2 = add_points_between(p2, original_points, (a_original_index + 1) % n, b_prime_target);
		p2.push_back(b_prime);
		
		//p3 goes a, b, b_prime to a_prime
		p3.push_back(a);
		p3.push_back(b);
		p3.push_back(b_prime);
		p3 = add_points_between(p3, original_points, b_prime_target, a_prime_target);
		p3.push_back(a_prime);


		results.push_back(Polygon_2(p2.begin(), p2.end()));
		results.push_back(Polygon_2(p3.begin(), p3.end()));
		results.push_back(Polygon_2(p1.begin(), p1.end()));
		to_add_2 = original_points[(a_original_index + 1) % n];
	}
	
	//We are floating on both sides, but we crossed a double concave segment (3 polygons)
	else if (!a_is_original && !b_is_original && point_isset(a_prime) && point_isset(b_prime) && indices_crossed.size() > 1 && (indices_crossed[0] + 1) % n == indices_crossed[1])
	{
		std::vector<Point_2> p1 = {}; //the concave end
		std::vector<Point_2> p2 = {}; //the non concave end
		std::vector<Point_2> p3 = {}; //the non concave end

		//p1 goes b, index_crossed_1 to b_prime
		p1.push_back(b);
		p1 = add_points_between(p1, original_points, indices_crossed[1], b_prime_target);
		p1.push_back(b_prime);

		//p2 goes a, b, b_prime to a_prime
		p2.push_back(a);
		p2.push_back(b);
		p2.push_back(b_prime);
		p2 = add_points_between(p2, original_points, b_prime_target, a_prime_target);
		p2.push_back(a_prime);

		//p3 index_crossed_0, a, a_prime to index_crossed_0
		p3.push_back(original_points[indices_crossed[0]]);
		p3.push_back(a);
		p3.push_back(a_prime);
		p3 = add_points_between(p3, original_points, a_prime_target, indices_crossed[0]);
		
		results.push_back(Polygon_2(p1.begin(), p1.end()));
		results.push_back(Polygon_2(p2.begin(), p2.end()));
		results.push_back(Polygon_2(p3.begin(), p3.end()));
		to_add_1 = original_points[indices_crossed[0]];
		to_add_2 = original_points[indices_crossed[1]];
	}
	
	//We are on two different segments, but we are crossing one (concave) point (3 polygons)
	else if ((b_is_original || b_on_original_seg) && (a_is_original || a_on_original_seg) && indices_crossed.size()  == 1)
	{
		std::vector<Point_2> p1 = {}; //first concave end
		std::vector<Point_2> p2 = {}; //first non concave end
		std::vector<Point_2> p3 = {}; //second non concave end

		if (a_on_original_prev <= indices_crossed[0] && indices_crossed[0] <= b_on_original_target)
		{
			//p1 goes index_crossed, a to index crossed
			p1.push_back(original_points[indices_crossed[0]]);
			p1.push_back(a);
			p1 = add_points_between(p1, original_points, a_on_original_target, indices_crossed[0]);

			//p2 goes b, index_crossed to b
			p2.push_back(b);
			p2 = add_points_between(p2, original_points, indices_crossed[0], b_on_original_target);
			extra_points[1] = original_points[indices_crossed[0]];

			//p3 goes a, b to a
			p3.push_back(a);
			p3.push_back(b);

			p3 = add_points_between(p3, original_points, b_on_original_target, a_on_original_target);
		}
		else
		{
			//p1 goes b, a, a to b
			p1.push_back(b);
			p1.push_back(a);
			p1 = add_points_between(p1, original_points, a_on_original_target, b_on_original_target);

			//p2 goes index_crossed, b to index crossed
			p2.push_back(original_points[indices_crossed[0]]);
			p2.push_back(b);
			p2 = add_points_between(p2, original_points, b_on_original_target, indices_crossed[0]);

			//p3 goes a, index_crossed to a
			p3.push_back(a);
			p3 = add_points_between(p3, original_points, indices_crossed[0], a_on_original_target);
			extra_points[2] = original_points[indices_crossed[0]];
		}

		results.push_back(Polygon_2(p1.begin(), p1.end()));
		results.push_back(Polygon_2(p2.begin(), p2.end()));
		results.push_back(Polygon_2(p3.begin(), p3.end()));
		to_add_1 = empty_point;
		to_add_2 = empty_point;
	}

	//We are on two different segments, but we are crossing two (concave) points (4 polygons)
	else if ((b_is_original || b_on_original_seg) && (a_is_original || a_on_original_seg) && indices_crossed.size() > 1)
	{
		std::vector<Point_2> p1 = {}; //first concave end
		std::vector<Point_2> p2 = {}; //first non concave end
		std::vector<Point_2> p3 = {}; //second non concave end
		std::vector<Point_2> p4 = {}; //second non concave end

		//p1 goes b to index_crossed_0, to b
		p1.push_back(b);
		p1 = add_points_between(p1, original_points, indices_crossed[0], b_on_original_target);
		extra_points[0] = original_points[indices_crossed[0]];

		//p2 goes index_crossed_1, b to index crossed_1
		p2.push_back(original_points[indices_crossed[1]]);
		p2.push_back(b);
		p2 = add_points_between(p2, original_points, b_on_original_target, indices_crossed[1]);

		//p3 goes a, index_crossed_1 to a
		p3.push_back(a);
		p3 = add_points_between(p3, original_points, indices_crossed[1], a_on_original_target);
		extra_points[2] = original_points[indices_crossed[1]];

		//p4 goes index_crossed_0, a to index_crossed_0
		p4.push_back(original_points[indices_crossed[0]]);
		p4.push_back(a);
		p4 = add_points_between(p4, original_points, a_on_original_target, indices_crossed[0]);


		results.push_back(Polygon_2(p1.begin(), p1.end()));
		results.push_back(Polygon_2(p2.begin(), p2.end()));
		results.push_back(Polygon_2(p3.begin(), p3.end()));
		results.push_back(Polygon_2(p4.begin(), p4.end()));

		to_add_1 = empty_point;
		to_add_2 = empty_point;
	}

	//Floating on a side, cutting two vertices (4 polygons)
	else if (b_is_original && !a_is_original && !a_on_original_seg && point_isset(a_prime) && b_original_concave && indices_crossed.size() > 0)
	{
		std::vector<Point_2> p1 = {}; //first concave end
		std::vector<Point_2> p2 = {}; //first non concave end
		std::vector<Point_2> p3 = {}; //second non concave end
		std::vector<Point_2> p4 = {}; //second concave end

		//p1 goes b, indice crossed to b_prime
		p1.push_back(b);
		p1 = add_points_between(p1, original_points, indices_crossed[0], b_prime_target);
		p1.push_back(b_prime);
		extra_points[0] = original_points[indices_crossed[0]];

		//p2 goes b, b, b_prime to b
		p2.push_back(b);
		p2.push_back(b);
		p2.push_back(b_prime);
		p2 = add_points_between(p2, original_points, b_prime_target, b_original_index);

		//p3 goes a, index crossed, b to a prime
		p3.push_back(a);
		p3.push_back(original_points[indices_crossed[0]]);
		p3 = add_points_between(p3, original_points, b_original_index, a_prime_target);
		p3.push_back(a_prime);

		//p4 goes index crossed, a, a_prime to index crossed
		p4.push_back(original_points[indices_crossed[0]]);
		p4.push_back(a);
		p4.push_back(a_prime);
		p4 = add_points_between(p4, original_points, a_prime_target, indices_crossed[0]);
		
		results.push_back(Polygon_2(p1.begin(), p1.end()));
		results.push_back(Polygon_2(p2.begin(), p2.end()));
		results.push_back(Polygon_2(p3.begin(), p3.end()));
		results.push_back(Polygon_2(p4.begin(), p4.end()));

		to_add_1 = empty_point;
		to_add_2 = empty_point;
	}

	//Floating on b side, cutting two vertices (4 polygons)
	else if (a_is_original && !b_is_original && !b_on_original_seg && point_isset(b_prime) && a_original_concave && indices_crossed.size() > 0)
	{
		std::vector<Point_2> p1 = {}; //first concave end
		std::vector<Point_2> p2 = {}; //first non concave end
		std::vector<Point_2> p3 = {}; //second non concave end
		std::vector<Point_2> p4 = {}; //second concave end

		//p1 goes b, a to b_prime
		p1.push_back(b);
		p1 = add_points_between(p1, original_points, a_original_index, b_prime_target);
		p1.push_back(b_prime);
		extra_points[0] = a;

		//p2 goes index crossed, b, b_prime to index crossed
		p2.push_back(original_points[indices_crossed[0]]);
		p2.push_back(b);
		p2.push_back(b_prime);
		p2 = add_points_between(p2, original_points, b_prime_target, indices_crossed[0]);

		//p3 a, index_crossed to a_prime
		p3.push_back(a);
		p3 = add_points_between(p3, original_points, indices_crossed[0], a_prime_target);
		p3.push_back(a_prime);
		extra_points[2] = original_points[indices_crossed[0]];

		//p4 goes a, a, a_prime to a
		p4.push_back(a);
		p4.push_back(a);
		p4.push_back(a_prime);
		p4 = add_points_between(p4, original_points, a_prime_target, a_original_index);
		
		results.push_back(Polygon_2(p1.begin(), p1.end()));
		results.push_back(Polygon_2(p2.begin(), p2.end()));
		results.push_back(Polygon_2(p3.begin(), p3.end()));
		results.push_back(Polygon_2(p4.begin(), p4.end()));

		to_add_1 = empty_point;
		to_add_2 = empty_point;
	}

	//Floating on both sides, cutting two vertices (4 polygons)
	else if (!a_is_original && !b_is_original && !b_on_original_seg && !a_on_original_seg && point_isset(b_prime) && point_isset(a_prime) && indices_crossed.size() > 1)
	{		
		std::vector<Point_2> p1 = {}; //first concave end
		std::vector<Point_2> p2 = {}; //first non concave end
		std::vector<Point_2> p3 = {}; //second non concave end
		std::vector<Point_2> p4 = {}; //second concave end

		//p1 goes b, index_crossed_0 to b_prime
		p1.push_back(b);
		p1 = add_points_between(p1, original_points, indices_crossed[0], b_prime_target);
		p1.push_back(b_prime);
		extra_points[0] = original_points[indices_crossed[0]];

		//p2 goes index_crossed_1, b, b_prime to index_crossed_1
		p2.push_back(original_points[indices_crossed[1]]);
		p2.push_back(b);
		p2.push_back(b_prime);
		p2 = add_points_between(p2, original_points, b_prime_target, indices_crossed[1]);

		//p3 goes a, index_crossed_1 to a_prime
		p3.push_back(a);
		p3 = add_points_between(p3, original_points, indices_crossed[1], a_prime_target);
		p3.push_back(a_prime);
		extra_points[2] = original_points[indices_crossed[1]];


		//p4 goes index_crossed_0, a, a_prime to indices_crossed_0
		p4.push_back(original_points[indices_crossed[0]]);
		p4.push_back(a);
		p4.push_back(a_prime);
		p4 = add_points_between(p4, original_points, a_prime_target, indices_crossed[0]);

		results.push_back(Polygon_2(p1.begin(), p1.end()));
		results.push_back(Polygon_2(p2.begin(), p2.end()));
		results.push_back(Polygon_2(p3.begin(), p3.end()));
		results.push_back(Polygon_2(p4.begin(), p4.end()));

		to_add_1 = empty_point;
		to_add_2 = empty_point;
	}
	
	//we are floating between two concave vertices (4 polygons)
	else if (a_is_original && b_is_original && point_isset(a_prime) && point_isset(b_prime))
	{
		std::vector<Point_2> p1 = {}; //first concave end
		std::vector<Point_2> p2 = {}; //first non concave end
		std::vector<Point_2> p3 = {}; //second non concave end
		std::vector<Point_2> p4 = {}; //second concave end

		//p1 goes b, a, a to b_prime
		p1.push_back(b);
		p1 = add_points_between(p1, original_points, a_original_index, b_prime_target);
		p1.push_back(b_prime);
		extra_points[0] = a;

		//p2 goes b,b, b', b' to b
		p2.push_back(b);
		p2.push_back(b);
		p2.push_back(b_prime);
		p2 = add_points_between(p2, original_points, b_prime_target, b_original_index);

		//p3 goes a, b, b to a'
		p3.push_back(a);
		p3 = add_points_between(p3, original_points, b_original_index, a_prime_target);
		p3.push_back(a_prime);
		extra_points[2] = b;

		//p4 goes a, a, a' to a_origal index
		p4.push_back(a);
		p4.push_back(a);
		p4.push_back(a_prime);
		p4 = add_points_between(p4, original_points, a_prime_target, a_original_index);

		results.push_back(Polygon_2(p1.begin(), p1.end()));
		results.push_back(Polygon_2(p2.begin(), p2.end()));
		results.push_back(Polygon_2(p3.begin(), p3.end()));
		results.push_back(Polygon_2(p4.begin(), p4.end()));

		to_add_1 = empty_point;
		to_add_2 = empty_point;
	}
	
	else
	{
		results.push_back(Polygon_2(original_points.begin(), original_points.end()));
	}
	return std::make_tuple(results, to_add_1, to_add_2, extra_points);

}



