#include <point_functions.h>

double alpha = 1;
Point_2 empty_point = Point_2(-10000, -10000);

double Hausdorff(std::vector<Point_2> A, std::vector<Point_2> B) {
	auto h = Segment_2(A[0], B[0]).squared_length();
	h = 0;
	for (int i = 0; i < A.size(); i++) {
		auto shortest = Segment_2(A[i], B[0]).squared_length();
		for (int j = 0; j < B.size(); j++) {
			auto dist = Segment_2(A[i], B[j]).squared_length();
			if (dist < shortest)
				shortest = dist;
		}
		if (shortest > h)
			h = shortest;
	}
	return std::sqrt(CGAL::to_double(h));
}

std::vector<std::vector<int>> SplitVector(const std::vector<int>& vec, int n)
{
	std::vector<std::vector<int>> outVec;

	int length = vec.size() / n;
	int remain = vec.size() % n;

	int begin = 0;
	int end = 0;

	for (int i = 0; i < min(n, vec.size()); ++i)
	{
		end += (remain > 0) ? (length + !!(remain--)) : length;

		outVec.push_back(std::vector<int>(vec.begin() + begin, vec.begin() + end));

		begin = end;
	}

	return outVec;
}


Ray_2 get_ray_for_id(int rayz, int mid, Point_2 reflex) {
	int per_quadrant = rayz / 8;
	int i;
	if (mid < per_quadrant) { //smaller than 2
		i = mid;
		return Ray_2(reflex, Point_2(reflex.x() + i * (1.0 / per_quadrant), reflex.y() + 1)); //top - top right
	}
	else if (mid < per_quadrant * 2) { //smaller than 4
		i = mid - per_quadrant;
		return Ray_2(reflex, Point_2(reflex.x() + 1, reflex.y() + (per_quadrant - i) * (1.0 / per_quadrant))); //top right to right
	}
	else if (mid < per_quadrant * 3) { //smaller than 6
		i = mid - per_quadrant * 2;
		return Ray_2(reflex, Point_2(reflex.x() + 1, reflex.y() - i * (1.0 / per_quadrant))); //right to bottom right
	}
	else if (mid < per_quadrant * 4) {//smaller than 8
		i = mid - per_quadrant * 3;
		return Ray_2(reflex, Point_2(reflex.x() + (per_quadrant - i) * (1.0 / per_quadrant), reflex.y() - 1)); //bottom right to bottom
	}
	else if (mid < per_quadrant * 5) {//smaller than 10
		i = mid - per_quadrant * 4;
		return Ray_2(reflex, Point_2(reflex.x() - i * (1.0 / per_quadrant), reflex.y() - 1)); //bottom to bottom left
	}
	else if (mid < per_quadrant * 6) {//smaller than 12
		i = mid - per_quadrant * 5;
		return Ray_2(reflex, Point_2(reflex.x() - 1, reflex.y() - (per_quadrant - i) * (1.0 / per_quadrant))); //bottom left to left
	}
	else if (mid < per_quadrant * 7) {//smaller than 14
		i = mid - per_quadrant * 6;
		return Ray_2(reflex, Point_2(reflex.x() - 1, reflex.y() + i * (1.0 / per_quadrant))); //left to topleft
	}
	else if (mid < per_quadrant * 8){//smaller than 16
		i = mid - per_quadrant * 7;
		return Ray_2(reflex, Point_2(reflex.x() - (per_quadrant - i) * (1.0 / per_quadrant), reflex.y() + 1)); //top left to top
	}
	else {
		//to top?
		return Ray_2(reflex, Point_2(reflex.x(), reflex.y() + 1)); //to top
	}
}

std::vector<Segment_2> recursive_ray_intersection(int rayz, int l, int r, std::vector<Segment_2> face, Point_2 reflex) {
	int mid = l + (r - l) / 2;
	Segment_2 res;
	if (r >= l) {
		Ray_2 mid_ray = get_ray_for_id(rayz, mid,  reflex);	

		if (proper_intersect_face(face, mid_ray, res))
		{
			Segment_2 middle_inter = res;
			std::vector<Segment_2> inter_segs = {};
			//Lets try it boys
			for (int k = mid - 1; k >= 0; k--) {
				//keep trying until we go out
				Segment_2 res2;
				if (proper_intersect_face(face, get_ray_for_id(rayz, k, reflex), res2)) {
					inter_segs.push_back(res2);
				}
				else
					break;
			}

			std::reverse(inter_segs.begin(), inter_segs.end());
			inter_segs.push_back(middle_inter);
			for (int k = mid + 1; k <rayz; k++) {
				//keep trying until we go out
				Segment_2 res2;
				if (proper_intersect_face(face, get_ray_for_id(rayz, k, reflex), res2)) {
					inter_segs.push_back(res2);
				}
				else
					break;
			}

				
			return inter_segs;
		}
		//check any point
		CGAL::Orientation or1 = mid_ray.supporting_line().oriented_side(face[0].source());
		int counter = 1;
		while (or1 == CGAL::COLLINEAR && counter < face.size()) {
			or1 = mid_ray.supporting_line().oriented_side(face[counter].source());
			counter++;
		}

		if (or1 == CGAL::LEFT_TURN) {
			//first half (include mid)
			return(recursive_ray_intersection(rayz, l, mid - 1, face, reflex));
		}
		else {
			//second half (include mid)
			return(recursive_ray_intersection(rayz, mid + 1, r, face, reflex));
		}
	}
	else
		return {};
}




CGAL::Lazy_exact_nt<CGAL::Gmpq> power_angle(Point_2 reflex, std::vector<Point_2> face) {
	CGAL::Lazy_exact_nt<CGAL::Gmpq> largest = -10000000;

	for (int i = 0; i < face.size(); i++) {
		for (int j = i + 1; j < face.size(); j++) {

			bool col = CGAL::collinear(reflex, face[i], face[j]);
			bool same1 = same(reflex, face[i]);
			bool same2 = same(reflex, face[j]);


			if(CGAL::collinear(reflex, face[i], face[j]) || same(reflex, face[i]) || same(reflex, face[j]))
				continue; //skip

			//all pairs
			Segment_2 seg1(reflex, face[i]);
			Segment_2 seg2(reflex, face[j]);

			auto mag1 = seg1.squared_length();
			auto mag2 = seg2.squared_length();
			auto x1 = seg1.target().x() - seg1.source().x();
			auto x2 = seg2.target().x() - seg2.source().x();
			auto y1 = seg1.target().y() - seg1.source().y();
			auto y2 = seg2.target().y() - seg2.source().y();



			auto dot_prod = x1 * x2 + y1 * y2;
			auto dot_product_sq = dot_prod * dot_prod;
			//double dot_product2 = CGAL::to_double(seg1.x() * seg2.x() + seg1.y() * seg2.y());

			
			auto cos_a_sq =dot_product_sq / (mag1 * mag2);
			
			if (largest < cos_a_sq)
				largest = cos_a_sq;
		}

	}

	return largest;
}


bool proper_intersect_face(std::vector<Segment_2> face, Ray_2 ray, Segment_2& res) {
	std::vector<Point_2> inters = { };

	for (int l = 0; l < face.size(); l++) {
		Point_2 inter = point_intersection(ray, face[l]);
		if (isset(inter) && !same(inter, face[l].source())) { //can't be the same as source, only target
			inters.push_back(inter);
			if (inters.size() > 1)
				break;
		}

	}

	if (inters.size() > 1) {
		res = Segment_2(inters[0], inters[1]);
		for (int l = 0; l < face.size(); l++) {
			if ((same(res.source(), face[l].source()) && same(res.target(), face[l].target())) ||
				(same(res.source(), face[l].target()) && same(res.target(), face[l].source())))
				return false; //nuh uh
		}
		return true;
	}
	return false;
}
bool proper_intersect_face(std::vector<Segment_2> face, Segment_2 chord, Segment_2 &res) {
	std::vector<Point_2> inters = { };

	for (int l = 0; l < face.size(); l++) {
		Point_2 inter = point_intersection(face[l], chord);
		if (isset(inter) && !same(inter, face[l].source())) { //can't be the same as source, only target
			inters.push_back(inter);
			if (inters.size() > 1)
				break;
		}
	}

	if (inters.size() > 1) {
		res = Segment_2(inters[0], inters[1]);
		for (int l = 0; l < face.size(); l++) {
			if (same(res.source(), face[l].source()) && same(res.target(), face[l].target()))
				return false; //nuh uh
		}
		return true;
	}
	return false;
}

void remove_intersection(std::vector<int>& a, std::vector<int>& b) {
	std::unordered_multiset<int> st;
	st.insert(a.begin(), a.end());
	st.insert(b.begin(), b.end());
	auto predicate = [&st](const int& k) { return st.count(k) > 1; };
	a.erase(std::remove_if(a.begin(), a.end(), predicate), a.end()); //only remove b from a
	//b.erase(std::remove_if(b.begin(), b.end(), predicate), b.end());
}

bool same(Point_2& a, Point_2& b) {
	return a == b;
	return a.x() == b.x() && a.y() == b.y();
	return(b.hx() + alpha >= a.hx() && b.hx() - alpha <= a.hx() && b.hy() + alpha >= a.hy() && b.hy() - alpha <= a.hy());
}
bool isset(Point_2 p)
{
	return !same(p, empty_point);
}
bool in_between(Segment_2 s, Point_2 a) {
	return s.has_on(a) && !same(a, s.source()) && !same(a, s.target());
}

Point_2 point_intersection(Line_2 pq, Segment_2 sgm, int seg_mode) {

	auto result = intersection(pq, sgm);

	if (result) {
		if (const Segment_2* s = boost::get<Segment_2>(&*result)) {
			if (same(s->source(), s->target()))
				return empty_point;
			else if (seg_mode == 1)
				return s->source();
			else if (seg_mode == 2)
				return s->target();
		}
		else {
			Point_2* p = boost::get<Point_2>(&*result);
			return *p;
		}
	}

	return empty_point;
}

Point_2 point_intersection(Ray_2 pq, Segment_2 sgm, int seg_mode) {
	if (same(pq.source(), sgm.source()))
		return pq.source();
	if (same(pq.source(), sgm.target()))
		return pq.source();
	//if (pq.has_on(sgm.source()))
		//return sgm.source();
	//if (pq.has_on(sgm.target()))
	//	return sgm.target();
	
	auto result = intersection(pq, sgm);
	if (result) {
		if (const Segment_2* s = boost::get<Segment_2>(&*result)) {
			if (seg_mode == 1)
				return s->source();
			else if (seg_mode == 2)
				return s->target();
		}
		else {
			Point_2* p = boost::get<Point_2>(&*result);
			return *p;
		}
	}
	return empty_point;
}

Point_2 point_intersection(Segment_2 pq, Segment_2 sgm, int seg_mode) {
	auto result = intersection(pq, sgm);

	if (result) {
		if (const Segment_2* s = boost::get<Segment_2>(&*result)) {
			if (seg_mode == 1)
				return s->source();
			else if (seg_mode == 2)
				return s->target();
		}
		else {
			Point_2* p = boost::get<Point_2>(&*result);
			return *p;
		}
	}
	return empty_point;
}

Point_2 first_intersection(Point_2 p, Point_2 q, std::vector<Point_2> points) {
	Ray_2 pq(p, q);
	Point_2 res = q;
	std::vector<Point_2> inters = {};
	for (int i = 0; i < points.size();i++) {		
		int prev = (i - 1 + points.size()) % points.size();
		int next = (i + 1) % points.size();
		int next_next = (next + 1) % points.size();
		
		Segment_2 curseg(points[i], points[next]);
		Point_2 inter_point = point_intersection(pq, curseg);
		

		//If q is in the middle of a segment and the next vertex not reflex
		if (in_between(curseg, q))
			return q;
		
		//if q is a vertex and it is not reflex, there will be no intersection point
		if (points[i] == q && CGAL::orientation(points[prev], points[i], points[next]) == CGAL::LEFT_TURN)
			return q;

		//if q is a vertex and prev is on a different side than next of the ray, there is no intersection
		CGAL::Orientation or1 = CGAL::orientation(p, q, points[next]);
		CGAL::Orientation or2 = CGAL::orientation(p, q, points[prev]);

		if (points[i] == q &&  or1 != or2 && or1 != CGAL::COLLINEAR && or2 != CGAL::COLLINEAR)
			return q;

		//If q is on the current segment and p is not, and the current segment is colinear with the ray
		bool seg_inside_rayseg = Segment_2(p, q).has_on(points[i]) && Segment_2(p, q).has_on(points[next]);
		if (!curseg.has_on(p) && pq.has_on(points[i]) && pq.has_on(points[next]) && curseg.has_on(q) && !seg_inside_rayseg)
			return q;

		if (isset(inter_point) && !same(inter_point, p) && !same(inter_point, q))
				inters.push_back(inter_point);		
	}
	if (inters.size() > 0) {
		res = inters[0];
		for (int i = 1; i < inters.size();i++) {
			if (Segment_2(q, inters[i]).squared_length() < Segment_2(q, res).squared_length())
				res = inters[i];
		}
	}
	return res;
}

bool fast_has_on(Point_2 &a, Point_2 &b, Point_2 &c) {
	return CGAL::collinear(a, b, c) &&
		Segment_2(a, b).has_on(c);
}


int difference_in_polygon(int a, int b, int n) {
	if (a <= b)
	{
		return b - a;
	}
	else
	{
		return n - a + b;
	}
}