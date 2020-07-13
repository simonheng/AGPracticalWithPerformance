#include "shortest_path.h"

void mark_domains(CDT& ct,
	CDT::Face_handle start,
	int index,
	std::list<CDT::Edge>& border)
{
	if (start->info().nesting_level != -1) {
		return;
	}
	std::list<CDT::Face_handle> queue;
	queue.push_back(start);

	while (!queue.empty()) {
		CDT::Face_handle fh = queue.front();
		queue.pop_front();
		if (fh->info().nesting_level == -1) {
			fh->info().nesting_level = index;
			for (int i = 0; i < 3; i++) {
				CDT::Edge e(fh, i);
				CDT::Face_handle n = fh->neighbor(i);
				if (n->info().nesting_level == -1) {
					if (ct.is_constrained(e)) border.push_back(e);
					else queue.push_back(n);
				}
			}
		}
	}
}
void mark_domains(CDT& cdt)
{
	for (CDT::All_faces_iterator it = cdt.all_faces_begin(); it != cdt.all_faces_end(); ++it) {
		it->info().nesting_level = -1;
	}

	int index = 0;
	std::list<CDT::Edge> border;
	mark_domains(cdt, cdt.infinite_face(), index++, border);
	while (!border.empty()) {
		CDT::Edge e = border.front();
		border.pop_front();
		CDT::Face_handle n = e.first->neighbor(e.second);
		if (n->info().nesting_level == -1) {
			mark_domains(cdt, n, e.first->info().nesting_level + 1, border);
		}
	}
}


Path::Path(Point_2 &from, Point_2 &to, std::vector<Point_2> points) {
	this->from = from;
	this->to = to;
	this->points = std::vector<Point_2>(points);
	set = true;
}
Path::Path() {
	set = false;
}

//Funnel::Funnel(const Funnel &f) : path_1(f.path_1), path_2(f.path_2) {
//}
Funnel::Funnel() : path_1(Path()), path_2(Path()) {
	path_1 = Path();
	path_2 = Path();
}



Funnel::Funnel(Path &path_1, Path &path_2) {

	this->path_1 = path_1;
	this->path_2 = path_2;
	
	
	split_index = -1;
	int min_size = min( this->path_1.points.size(),  this->path_2.points.size());
	while (split_index < min_size - 1 && min_size > 0) {
		if (!same(this->path_1.points[split_index + 1], this->path_2.points[split_index + 1]))
			break;
		split_index++;
	}

	if (split_index == -1)
		fp = this->path_1.from;
	else
		fp = this->path_1.points[split_index];
}

std::vector<Point_2> Funnel::merge_middle(Point_2 that_fp, bool second) {
	std::vector<Point_2> shortest_path = {};
	Segment_2 current_path(this->fp, that_fp);

	std::vector<Point_2> temp_path_1 = path_1.points;
	std::vector<Point_2> temp_path_2 = path_2.points;
	temp_path_1.push_back(path_1.to);
	temp_path_2.push_back(path_2.to);

	
	int max_size = max( temp_path_1.size(),  temp_path_2.size());
	CGAL::Orientation or1 = CGAL::orientation(this->fp, that_fp, temp_path_1[split_index + 1]);
	CGAL::Orientation or2 = CGAL::orientation(this->fp, that_fp, temp_path_2[split_index + 1]);

	if (or1 != or2 && or1 != CGAL::COLLINEAR && or2 != CGAL::COLLINEAR)
		return {};

	if (split_index == -1) {
		if (path_1.points.size() == 0 && path_2.points.size() > 0 && same(path_2.points[0], path_1.to))
			return { path_1.to };
		else if (path_2.points.size() == 0 && path_1.points.size() > 0 && same(path_1.points[0], path_2.to))
			return { path_2.to };
	}

	or1 = CGAL::orientation(this->fp, that_fp, path_1.to);
	or2 = CGAL::orientation(this->fp, that_fp, path_2.to);
	CGAL::Orientation or3 = CGAL::orientation(path_1.to, path_2.to, fp);
	CGAL::Orientation or4 = CGAL::orientation(path_1.to, path_2.to, that_fp);

	

	int start = split_index + 1;
	bool look_at1 = true;
	bool look_at2 = true;
	if ((or1 == or2 && or1 != CGAL::COLLINEAR) || or3 == or4) {
		
		auto dist_1 = Segment_2(path_1.to, current_path.supporting_line().projection(path_1.to)).squared_length();
		auto dist_2 = Segment_2(path_2.to, current_path.supporting_line().projection(path_2.to)).squared_length();

		//auto dist_1 = Segment_2(path_1.to, that_fp).squared_length();
		//auto dist_2 = Segment_2(path_2.to, that_fp).squared_length();

		Point_2 to;
		std::vector<Point_2> path;
		if (or1 == or2 && or2 == or3 && or3 == or4 && or4 == CGAL::COLLINEAR)
			return {};

		/*
		if ((dist_1 < dist_2 || (path_2.points.size() > split_index + 1 && same(path_2.points[split_index + 1], path_1.to)))
			&& !(path_1.points.size() > split_index + 1 && same(path_1.points[split_index + 1], path_2.to))
			) {

			to = path_1.to;


			if (temp_path_1.size() > 1) {
				if (path_1.points.size() > 1) {
					int i = path_1.points.size() - 2;
					for (i; i > split_index;i--) {
						CGAL::Orientation or_cur_that = CGAL::orientation(path_1.points[i], that_fp, to);
						CGAL::Orientation or_cur_next = CGAL::orientation(path_1.points[i], that_fp, path_1.points[i + 1]);


						if (or_cur_that != or_cur_next)
							break;
					}

					path = { path_1.points.begin() + split_index, path_1.points.end() + i + 2};
				}
				else if (split_index == -1)
					path = path_1.points;
			}
		}
		else {
			to = path_2.to;

			if (temp_path_2.size() > 1) {
				if (path_2.points.size() > 1) {
					int i = split_index;
					for (i; i < path_2.points.size();i++) {						
						CGAL::Orientation or_cur = CGAL::orientation(path_2.points[i], that_fp, to);
						if (or_cur != or2) {
							break;
						}
					}
					path = { path_2.points.begin() + split_index, path_2.points.begin() + i };

				}
				else if (split_index == -1)
					path = path_2.points;
			}

		}

		if (path.size() < 1) {
			//we have to go via "to"
			return { to };
		}
		else {
			CGAL::Orientation or5 = CGAL::orientation(path.back(), that_fp, to);
			if (or5 == or1)
				path.push_back(to);
			return path;
		}
		*/


		if ((dist_1 < dist_2 || (path_2.points.size() > split_index + 1 && same(path_2.points[split_index + 1], path_1.to)))
			&& !(path_1.points.size() > split_index + 1 && same(path_1.points[split_index + 1], path_2.to))
			) {

			if (or3 == or4) {
				return { temp_path_1.begin() + split_index + 1, temp_path_1.end() };
			}


			look_at2 = false;
			if (path_1.points.size() == split_index + 1)
				return { path_1.to };

			//we base it on path-1
			for (start; start < path_1.points.size();start++) {
				CGAL::Orientation new_or = CGAL::orientation(path_1.points[start], that_fp, path_1.to);
				if (new_or != or1)
					break;
			}

			if (start == path_1.points.size())
				return { temp_path_1.begin() + split_index + 1, temp_path_1.end() };

			current_path = Segment_2(path_1.points[start], that_fp);
			start++;
		}

		else {
			if (or3 == or4) {
				return { temp_path_2.begin() + split_index + 1, temp_path_2.end() };
			}
			look_at1 = false;
			if (path_2.points.size() == split_index + 1)
				return { path_2.to };
				
			for (start; start < path_2.points.size() ;start++) {
				CGAL::Orientation new_or = CGAL::orientation(path_2.points[start], that_fp, path_2.to);
				if (new_or != or2)
					break;

			}

			if (start == path_2.points.size())
				return { temp_path_2.begin() + split_index + 1, temp_path_2.end() };

			current_path = Segment_2(path_2.points[start], that_fp);
			start++;
		}

	}
			
	//Add middle part of shortest path
	//For both funnels, find intersections 
	for (int i = start; i < max_size - 1 ;i++) {
		//check path_1 for intersection
		if (look_at1 && i < temp_path_1.size() - 1) {
			Point_2 next = temp_path_1[i + 1];

			Segment_2 curseg(temp_path_1[i], next);
			Point_2 inter_point = point_intersection(current_path, curseg);

			//valid intersection?
			try {

				if (isset(inter_point) && (!same(inter_point, next) || same(inter_point, path_1.to))) {

					int j = -1;
					//hop back to make sure we are correct
					for (j; i + j > split_index; j--) {
						Segment_2 seg_from_prev(that_fp, temp_path_1[i + j]);

						//do we still intersect curseg from here?
						inter_point = point_intersection(seg_from_prev, curseg);
						if(isset(inter_point)){
							break;
						}
					}

					return { temp_path_1.begin() + 1 + split_index, temp_path_1.begin() + i + 2 + j};




				}
			}
			catch (...) {
				return { temp_path_1[i + 1] };
			}

		}
		//check path_2 for intersection
		if (look_at2 && i < temp_path_2.size() - 1) {
			Point_2 next = temp_path_2[i + 1];

			Segment_2 curseg(temp_path_2[i], next);
			Point_2 inter_point = point_intersection(current_path, curseg); 

			//valid intersection?
			if (isset(inter_point) && (!same(inter_point, next) || same(inter_point, path_2.to))) {
				int j = -1;
				//hop back to make sure we are correct
				for (j; i + j > split_index; j--) {
					Segment_2 seg_from_prev(that_fp, temp_path_2[i + j]);

					//do we still intersect curseg from here?
					inter_point = point_intersection(seg_from_prev, curseg);
					if (isset(inter_point)) {
						break;
					}
				}

				return { temp_path_2.begin() + 1 + split_index, temp_path_2.begin() + i + 2 + j };
			}

			
			
		}
	}
	
	if (start == split_index + 1)
		return {};
	else if(look_at1)
		return { temp_path_1.begin() + 1 + split_index, temp_path_1.begin() + start};
	else
		return { temp_path_2.begin() + 1 + split_index, temp_path_2.begin() + start};

}




Path Funnel::Merge(Funnel &that, bool second_try) {
	//Make sure path_1 and path_2 are in right order

	if (!same(this->path_1.to, that.path_1.to)) {
		Path temp = this->path_1;
		this->path_1 = this->path_2;
		this->path_2 = temp;
	}
	//The diagonal is this->path_1.to, this->path_2.to 
	//and that.path_1.to and that.path_2.to

	//Add this until split vertex
	std::vector<Point_2> shortest_path = {};
	if (split_index > -1)
		shortest_path = { this->path_1.points.begin(), this->path_1.points.begin() + this->split_index + 1 };


	std::vector<Point_2> temp_p1_from = {};
	std::vector<Point_2> temp_p2_from = {};
	std::vector<Point_2> temp_p1_to = {};
	std::vector<Point_2> temp_p2_to = {};

	if (split_index == -1) {
		temp_p1_from.push_back(path_1.from);
		temp_p2_from.push_back(path_2.from);
	}
	else {
		temp_p1_from.push_back(path_1.points[split_index]);
		temp_p2_from.push_back(path_2.points[split_index]);
	}

	temp_p1_from.insert(temp_p1_from.end(), path_1.points.begin() + split_index + 1, path_1.points.end());
	temp_p2_from.insert(temp_p2_from.end(), path_2.points.begin() + split_index + 1, path_2.points.end());
	temp_p1_from.push_back(path_1.to);
	temp_p2_from.push_back(path_2.to);

	if (that.split_index == -1) {
		temp_p1_to.push_back(that.path_1.from);
		temp_p2_to.push_back(that.path_2.from);
	}
	else {
		temp_p1_to.push_back(that.path_1.points[that.split_index]);
		temp_p2_to.push_back(that.path_2.points[that.split_index]);
	}

	temp_p1_to.insert(temp_p1_to.end(), that.path_1.points.begin() + that.split_index + 1, that.path_1.points.end());
	temp_p2_to.insert(temp_p2_to.end(), that.path_2.points.begin() + that.split_index + 1, that.path_2.points.end());
	temp_p1_to.push_back(that.path_1.to);
	temp_p2_to.push_back(that.path_2.to);


	//start at index of last point
	//we need at least 3
	int marker_from_1 = temp_p1_from.size() - 2;
	int marker_to_1 = temp_p1_to.size() - 2;

	int marker_from_2 = temp_p2_from.size() - 2;
	int marker_to_2 = temp_p2_to.size() - 2;

	

	int count = 0;
	bool inter_from_1 = false;
	bool inter_from_2 = false;
	bool inter_to_1 = false;
	bool inter_to_2 = false;

	Segment_2 current_path(this->fp, that.fp);

	CGAL::Orientation or1 = CGAL::orientation(this->fp, that.fp, path_1.to);
	CGAL::Orientation or2 = CGAL::orientation(this->fp, that.fp, path_2.to);

	CGAL::Orientation or3 = CGAL::orientation(path_1.to, path_2.to, fp);
	CGAL::Orientation or4 = CGAL::orientation(path_1.to, path_2.to, that.fp);

	int closest = -1;



	//reverse both?
	if ((or1 == or2 && or1 != CGAL::COLLINEAR) || or3 == or4) {
		//find closest diag point
		auto dist_1 = Segment_2(path_1.to, current_path.supporting_line().projection(path_1.to)).squared_length();
		auto dist_2 = Segment_2(path_2.to, current_path.supporting_line().projection(path_2.to)).squared_length();

		if ((path_1.points.size() > 0 && same(path_1.points[path_1.points.size() - 1], path_2.to)) ||
			(that.path_1.points.size() > 0 && same(that.path_1.points[that.path_1.points.size() - 1], that.path_2.to))			
			)
			closest = 2;
		else if ((path_2.points.size() > 0 && same(path_2.points.back(), path_1.to)) ||
			(that.path_2.points.size() > 0 && same(that.path_2.points.back(), that.path_1.to))
			)
			closest = 1;
		else {
			if (dist_1 < dist_2) {
				closest = 1;
			}
			else {
				closest = 2;
			}
		}
		if (closest == 1) {
			or1 = CGAL::orientation(that.fp, this->fp, path_1.to); //reverse or1
		}
		else
			or2 = CGAL::orientation(that.fp, this->fp, path_2.to); //reverse or2
	}
	while (or3 != or4 && (
		((closest == -1 || closest == 1) && marker_from_1 >= 0 && !inter_from_1) ||
		((closest == -1 || closest == 1) && marker_to_1 >= 0 && !inter_to_1) ||
		((closest == -1 || closest == 2) && marker_from_2 >= 0 && !inter_from_2) ||
		((closest == -1 || closest == 2) &&  marker_to_2 >= 0 && !inter_to_2)
		)){	
		if (count % 2 == 0) {
			if ((closest == -1 || closest == 1) && !inter_from_1 && marker_from_1 >= 0) {
				int target = max(0, marker_to_1);
				CGAL::Orientation or_test = CGAL::orientation(temp_p1_from[marker_from_1], temp_p1_to[target], temp_p1_from[marker_from_1 + 1]);
				if (or_test != or1 && or_test != CGAL::COLLINEAR) {
					inter_from_1 = true;
				}
				else
					marker_from_1--;
			}
			if ((closest == -1 || closest == 2) && !inter_from_2 && marker_from_2 >= 0) {
				int target = max(0, marker_to_2);
				CGAL::Orientation or_test = CGAL::orientation(temp_p2_from[marker_from_2], temp_p2_to[target], temp_p2_from[marker_from_2 + 1]);


				if (or_test != or2 && or_test !=  CGAL::COLLINEAR) {
					inter_from_2 = true;
				}
				else
					marker_from_2--;
			}
		}
		else {
			if ((closest == -1 || closest == 1) && !inter_to_1 && marker_to_1 >= 0) {
				int target = max(0, marker_from_1);
				CGAL::Orientation or_test = CGAL::orientation(temp_p1_from[target], temp_p1_to[marker_to_1], temp_p1_to[marker_to_1 + 1]);
				if (or_test != or1 && or_test != CGAL::COLLINEAR) {
					inter_to_1 = true;
				}
				else
					marker_to_1--;
			}
			if ((closest == -1 || closest == 2) && !inter_to_2 && marker_to_2 >= 0) {
				int target = max(0, marker_from_2);
				CGAL::Orientation or_test = CGAL::orientation(temp_p2_from[target], temp_p2_to[marker_to_2], temp_p2_to[marker_to_2 + 1]);
				if (or_test != or2 && or_test != CGAL::COLLINEAR) {
					inter_to_2 = true;
				}
				else
					marker_to_2--;
			}
		}
		count++;
	}

	
	
	if (inter_from_1) {
		shortest_path.insert(shortest_path.end(), temp_p1_from.begin() + 1, temp_p1_from.begin() + marker_from_1 + 2);
	}
	if (inter_to_1) {
		//add in reverse!
		std::vector<Point_2> temp = { temp_p1_to.begin() + 1, temp_p1_to.begin() + marker_to_1 + 2 };
		for (int j = temp.size() - 1; j>=0; j--)
			shortest_path.push_back(temp[j]);
	}

	if (inter_from_2) {
		shortest_path.insert(shortest_path.end(), temp_p2_from.begin() + 1, temp_p2_from.begin() + marker_from_2 + 2);
	}
	if (inter_to_2) {
		//add in reverse!
		std::vector<Point_2> temp = { temp_p2_to.begin() + 1, temp_p2_to.begin() + marker_to_2 + 2 };
		for (int j = temp.size() - 1; j >= 0; j--)
			shortest_path.push_back(temp[j]);
	}
	
	if (!inter_to_1 && !inter_to_2 && !inter_from_1 && !inter_from_2 && closest>-1) {
		if (closest == 1) {
			//add entirety of path1
			shortest_path.insert(shortest_path.end(), path_1.points.begin() + split_index + 1, path_1.points.end());

			shortest_path.push_back(path_1.to);


			for (int j = that.path_1.points.size() - 1; j > that.split_index; j--)
				shortest_path.push_back(that.path_1.points[j]);
		}
		else {
			//add entirety of path2
			shortest_path.insert(shortest_path.end(), path_2.points.begin() + split_index + 1, path_2.points.end());

			shortest_path.push_back(path_2.to);

			for (int j = that.path_2.points.size() - 1; j > that.split_index; j--)
				shortest_path.push_back(that.path_2.points[j]);

		}
	}


	//Add split vertex until end of that
	for (int j = that.split_index; j >= 0; j--)
		shortest_path.push_back(that.path_1.points[j]);

	return Path(this->path_1.from, that.path_1.from, shortest_path);
}

//Path Funnel::Merge(Funnel &that, bool second_try) {
//	Point_2 from = this->path_1.from;
//	Point_2 to = that.path_1.from;
//	
//	if (!second_try) {
//		std::vector<Point_2> temp_path_1 = path_1.points;
//		std::vector<Point_2> temp_path_2 = path_2.points;
//		temp_path_1.push_back(path_1.to);
//		temp_path_2.push_back(path_2.to);
//
//
//		int max_size = max( temp_path_1.size(),  temp_path_2.size());
//		CGAL::Orientation or1 = CGAL::orientation(this->fp, that.fp, temp_path_1[split_index + 1]);
//		CGAL::Orientation or2 = CGAL::orientation(this->fp, that.fp, temp_path_2[split_index + 1]);
//
//		if (or1 != or2 && or1 != CGAL::COLLINEAR && or2 != CGAL::COLLINEAR) { //try the other way around!
//			Path p = that.Merge(*this, true);
//			p.reverse();
//			return p;
//		}
//	}
//
//	/*if (same(this->path_1.to, that.path_1.from))
//		return this->path_1;
//	if (same(this->path_2.to, that.path_1.from))
//		return this->path_2;
//
//	if (same(that.path_1.to, this->path_1.from))
//		return that.path_1.reverse();
//	if (same(that.path_2.to, this->path_1.from))
//		return that.path_2.reverse();
//*/
//
//	//Make sure path_1 and path_2 are in right order
//	if (!same(this->path_1.to, that.path_1.to)) {
//		Path temp = this->path_1;
//		this->path_1 = this->path_2;
//		this->path_2 = temp;
//	}
//	//The diagonal is this->path_1.to, this->path_2.to 
//	//and that.path_1.to and that.path_2.to
//
//	//Add this until split vertex
//	std::vector<Point_2> shortest_path = {};
//	if (split_index > -1)
//		shortest_path = { this->path_1.points.begin(), this->path_1.points.begin() + this->split_index + 1 };
//
//
//	//"this" funnel 
//	std::vector<Point_2> temp_pts = this->merge_middle(that.fp, false);
//	shortest_path.insert(shortest_path.end(), temp_pts.begin(), temp_pts.end());
//	   
//	//"that" funnel 
//	if (shortest_path.size() > 0) {
//		temp_pts = that.merge_middle(shortest_path.back(), true);
//		std::reverse(temp_pts.begin(), temp_pts.end());
//		shortest_path.insert(shortest_path.end(), temp_pts.begin(), temp_pts.end());
//	}
//	/*else {
//		temp_pts = that.merge_middle(this->fp, true);
//		std::reverse(temp_pts.begin(), temp_pts.end());
//		shortest_path.insert(shortest_path.end(), temp_pts.begin(), temp_pts.end());
//	}*/
//
//	//Add split vertex until end of that
//	for (int j = that.split_index; j >= 0; j--)
//		shortest_path.push_back(that.path_1.points[j]);
//
//	return Path(from, to, shortest_path);
//}
Shortest_Map::Shortest_Map() {};

void WeakVisPoly::recursive_shortest(CDT_FH f, int query) {
	//did we visit this already?
	if (f != NULL && f->info().in_domain()) {
		if (std::find(f->info().visited.begin(), f->info().visited.end(), query) == f->info().visited.end()) { //if we haven't visited this yet
			f->info().visited.push_back(query);
			level_faces[query].push_back(K::Triangle_2(f->vertex(0)->point(), f->vertex(1)->point(), f->vertex(2)->point()));
			int query_t = -1;
			for (int i = 0;i < 3;i++) {
				if (same(f->vertex(i)->point(), poly[query]))
					query_t = i;
			}

			if (query_t == -1)  {
				std::vector<int> set_indices;
				int targ_index = -1;

				//identify pre computed shortest_paths 
				for (int i = 0; i < 3; i++) {
					if (map[query][f->vertex(i)->info().id].set)
						set_indices.push_back(f->vertex(i)->info().id);
					else
						targ_index = f->vertex(i)->info().id;
				}



				if (set_indices.size() != 2) {
					//something is wrong... (already computed, or not yet computed?)
					int c = 5;
				}
				else {
					if (targ_index < 0 || set_indices[0] < 0 || set_indices[1] < 0)
						return;


					if (targ_index == 5)
						int c = 4;
					//One funnel is trivial, triangle from set_indices
					Funnel f1(Path(poly[targ_index], poly[set_indices[0]], {}), Path(poly[targ_index], poly[set_indices[1]], {}));

					//Other funnel is from query point to set_indices
					Funnel f2(map[query][set_indices[0]], map[query][set_indices[1]]);

					map[query][targ_index] = f2.Merge(f1);

				}
			}
			else {
				int cw_index = f->vertex(f->cw(query_t))->info().id;
				int ccw_index = f->vertex(f->ccw(query_t))->info().id;
				if (cw_index < 0)
					return;

				//shortest path is trivial: segments
				map[query][cw_index] = Path(poly[query], poly[cw_index], {});
				map[query][ccw_index] = Path(poly[query], poly[ccw_index], {});
			}
			//compute shortest paths

			//call recursively for adjacent faces

			recursive_shortest(f->neighbor(0), query);
			recursive_shortest(f->neighbor(1), query);
			recursive_shortest(f->neighbor(2), query);
		}
	}
}


Path Path::reverse() {
	std::reverse(points.begin(), points.end());
	Point_2 old_from = this->from;
	this->from = this->to;
	this->to = old_from;
	return *this;
}

void WeakVisPoly::recursive_diagonal_map(CDT_FH& f, int query, int diag1, int diag2) {
	if (f->info().in_domain()) {
		if (std::find(f->info().visited_diag.begin(), f->info().visited_diag.end(), query) == f->info().visited_diag.end()) {
			f->info().visited_diag.push_back(query);

			//save the diagonal
			diagonalMap[query][f->info().id] = { diag1, diag2 };
			//we are the first
			if (query == f->info().id) {
				recursive_diagonal_map(f->neighbor(0), query, f->vertex(1)->info().id, f->vertex(2)->info().id);
				recursive_diagonal_map(f->neighbor(1), query, f->vertex(0)->info().id, f->vertex(2)->info().id);
				recursive_diagonal_map(f->neighbor(2), query, f->vertex(0)->info().id, f->vertex(1)->info().id);
			}
			else {
				recursive_diagonal_map(f->neighbor(0), query, diag1, diag2);
				recursive_diagonal_map(f->neighbor(1), query, diag1, diag2);
				recursive_diagonal_map(f->neighbor(2), query, diag1, diag2);
			}
		}
	}
}


triangle_loc WeakVisPoly::find_segment(Point_2 p, Point_2 q, bool second_try) {
	lt t;

	CDT_FH triangle  = get_loc(p, t);
	int p_vid = -1;
	if (t != lt::VERTEX) {
		int crisb = 5; //shouldn't happen
	}
	else {
		for (int i = 0;i < 3;i++)
		{
			if (same(triangle->vertex(i)->point(), p)) {
				p_vid = i;
			}

			if (same(triangle->vertex(i)->point(), q)) {
				triangle_loc tl;
				tl.t_id = triangle->info().id;
				tl.pts = {triangle->vertex(0)->point(),triangle->vertex(1)->point(), triangle->vertex(2)->point() };
				return tl;
			}
		}
		CDT_FH new_t = triangle;
		new_t = triangle->neighbor(triangle->ccw(p_vid));
		int count = 0;
		//gotta check neighbours ccw
		while (new_t->info().in_domain()) {
			if (count > 10)
				break;
			for (int i = 0;i < 3;i++)
			{
				if (same(new_t->vertex(i)->point(), p)) {
					p_vid = i;
				}

				if (same(new_t->vertex(i)->point(), q)) {
					triangle_loc tl;
					tl.t_id = new_t->info().id;
					tl.pts = { new_t->vertex(0)->point(),new_t->vertex(1)->point(), new_t->vertex(2)->point() };
					return tl;
				}
			}
			new_t = new_t->neighbor(new_t->ccw(p_vid));
			count++;
		}
		count = 0;
		new_t = triangle->neighbor(triangle->cw(p_vid));
		//gotta check neighbours cw
		while (new_t->info().in_domain()) {
			if (count > 10)
				break;
			for (int i = 0;i < 3;i++)
			{
				if (same(new_t->vertex(i)->point(), p)) {
					p_vid = i;
				}

				if (same(new_t->vertex(i)->point(), q)) {
					triangle_loc tl;
					tl.t_id = new_t->info().id;
					tl.pts = { new_t->vertex(0)->point(),new_t->vertex(1)->point(), new_t->vertex(2)->point() };
					return tl;
				}
			}
			new_t = new_t->neighbor(new_t->cw(p_vid));
			count++;
		}

		if (!second_try)
			return find_segment(q, p, true);
		int crisb = 14; //plz never here
		


	}


}



CDT_FH WeakVisPoly::get_loc(Point_2& p, lt& t)
{
	int temp = 0;
	CDT_FH triangle = T.locate(p, t, temp);
	if (!triangle->info().in_domain()) {
		if (t == lt::VERTEX) {
			CDT_FH neighbor1 = triangle->neighbor(triangle->ccw(temp));
			CDT_FH neighbor2 = triangle->neighbor(triangle->cw(temp));

			if (neighbor1->info().in_domain())
				triangle = neighbor1;
			else if (neighbor2->info().in_domain())
				triangle = neighbor2;
			else {
				for (CFFI it = T.finite_faces_begin();
					it != T.finite_faces_end();
					++it) {
					if (it->info().in_domain()) {
						Point_2 temp_p = p;
						//check if p1 or p2 is inside this face
						if (same(it->vertex(0)->point(), temp_p) || same(it->vertex(1)->point(), temp_p) || same(it->vertex(2)->point(), temp_p))
							triangle = it;
					}
				}
			}
		}
		else if(t == lt::EDGE) {
			//EDGE
			CDT_FH neighbor = triangle->neighbor(temp);
			if (neighbor->info().in_domain())
				triangle = neighbor;
			else
				int crisb = 10; //should not come here...
		}
	}
	return triangle;
}


Vertex_info WeakVisPoly::get_location(Vertex_info &inf, Point_2 v) {
	CDT_FH triangle = get_loc(v, inf.type);	
	inf.triangle = triangle->info().id;
	inf.poly_index = -1;
	if (inf.type == lt::VERTEX) {
		//Find triangle index
		for (int i = 0;i < 3;i++)
		{
			if (same(triangle->vertex(i)->point(), v)) {
				inf.poly_index = triangle->vertex(i)->info().id;


				/*if (inf.poly_index == 0)
					v0 = inf.triangle;
				else if (inf.poly_index == 1)
					v1 = inf.triangle;

				for (int j = 0; j < children.size();j++) {
					if (inf.poly_index == children_i0[j])
						children_v0[j] = inf.triangle;
					else if (inf.poly_index == children_i1[j])
						children_v1[j] = inf.triangle;
				}*/
				break;
			}
		}
		if (inf.poly_index == -1)
			int crisp_b = 10;
	}
	return inf;
}



WeakVisPoly::WeakVisPoly() {};

WeakVisPoly::WeakVisPoly(std::vector<Point_2> poly) {
	this->poly = poly;
};

WeakVisPoly::WeakVisPoly(std::vector<Point_2>& temp_poly, int prev) {
	origin = Segment_2(temp_poly[0], temp_poly[1]);
	children = {};
	parentId = prev;	
	
	Polygon_2 temp = compute_visibility_from_segment(temp_poly, temp_poly[0], temp_poly[1]);

	
	poly = temp.container();
		


	if (poly.size() < 3) {//
		throw(std::exception());
	}
	
	//fix poly 2.0
	if (!CGAL::is_simple_2(poly.begin(), poly.end()))
		throw(std::exception());

	
	
	//fix poly!
	for (u_int j = 0; j < poly.size(); j++) {
		u_int next = (j + 1) % poly.size();
		Segment_2 seg(poly[j], poly[next]);

		for (int i = 0; i < temp_poly.size(); i++) {
			if (in_between(seg, temp_poly[i])) {
				poly.insert(poly.begin() + next, temp_poly[i]);

			}

		}
	}

	std::vector<Point_2> poly_copy = poly;

	index_map = std::vector<int>(poly.size(), -1);
	on_segment = std::vector<bool>(poly.size(), false);

	for (int j = 0; j < poly.size(); j++) {
		for (int i = 0; i < temp_poly.size(); i++) {
			if (same(temp_poly[i], poly[j])) {
				index_map[j] = i;
				break;
			}
			int next = (i + 1) % temp_poly.size();
			Segment_2 s(temp_poly[i], temp_poly[next]);
			if (in_between(s, poly[j])) {
				index_map[j] = next;
				on_segment[j] = true;
				break;
			}

		}
	}
	

	level_faces = std::vector<std::vector<K::Triangle_2>>(poly.size(), std::vector<K::Triangle_2>());
	//index_to_face = std::vector<CDT_FH>(poly.size(), CDT_FH());
	
	// last point
	CDT::Vertex_handle vh_p = T.insert(poly[poly.size() - 1]);

	vh_p->info().id = poly.size() - 1;
	//start at 0 but dont visit last point
	for (int i = 0; i < poly.size();i++) {
		CDT::Vertex_handle vh_c = T.insert(poly[i]);
		vh_c->info().id = i;
		v_handles.push_back(vh_c);
		T.insert_constraint(vh_p, vh_c);
		vh_p = vh_c; //update prev vertex
	}

	mark_domains(T);
	f_handles = {};
	int counter = 0;
	//O(N)
	for (CFFI it = T.finite_faces_begin();
		it != T.finite_faces_end();
		++it) {
		if (it->info().in_domain()) {
			it->info().id = counter;
			f_handles.push_back(it);
			it->vertex(0)->info().face_id = counter;
			it->vertex(1)->info().face_id = counter;
			it->vertex(2)->info().face_id = counter;				
		
			level_faces[0].push_back(K::Triangle_2(it->vertex(0)->point(), it->vertex(1)->point(), it->vertex(2)->point()));
			counter++;
		}
	}

	map = std::vector<std::vector<Path>>(poly.size(), std::vector<Path>(poly.size()));
	diagonalMap = std::vector<std::vector<std::vector<int>>>(counter, std::vector<std::vector<int>>(counter));
	

	//make diagonal map
	for (CFFI it = T.finite_faces_begin();
		it != T.finite_faces_end();
		++it) {
		if (it->info().in_domain()) {
			//we are sure we have an ID
			recursive_diagonal_map((CDT_FH)it, it->info().id, it->info().id, it->info().id);
		}
	}

	//make shortest path map within our own
	for (int i = 0; i < poly.size(); i++) {
		map[i][i] = Path(poly[i], poly[i], {});
		recursive_shortest(f_handles[v_handles[i]->info().face_id], i);
		//int crisb = 5;
	}

	//make shortest path to parent
}

Path WeakVisPoly::shortest_point_point(Point_2 p1, Point_2 p2, Vertex_info inf1, Vertex_info inf2, int status, WeakVisPoly &other) {
	bool vertex1 = inf1.type == lt::VERTEX;
	bool vertex2 = inf2.type == lt::VERTEX;
	int v1 = inf1.poly_index;
	int v2 = inf2.poly_index;
	int triangle1 = inf1.triangle; //p1 = parent
	int triangle2 = inf2.triangle; //p2 = child
	bool reverse = false;

	
	if (status == 0 && triangle1 == triangle2)
		return Path(p1, p2, {});

	if (vertex1 && vertex2) {
		//shortest path is inside map...
		if (status == 0)
			return map[v1][v2];
		else if (status == 1) {
			//v1 is in parent 
			//v2 is in child (current decomp)
			return parentMap[v1][v2]; //from parent to us
		}
		else {
			return siblingMap[other.childId][v1][v2];
		}
	}

	std::vector<int> diag1 = {};
	std::vector<int> diag2 = {};

	if (status == 0) {
		diag1 = diagonalMap[triangle1][triangle2];
		diag2 = diagonalMap[triangle2][triangle1];
	}
	else if (status == 1) {


		CGAL::Bounded_side check = CGAL::bounded_side_2(other.children_v0_t[childId].begin(), other.children_v0_t[childId].end(), p1);
		//
		if (triangle1 == other.children_v0[childId] || check == CGAL::ON_BOUNDARY || check == CGAL::ON_BOUNDED_SIDE)
			diag1 = { other.children_i0[childId], other.children_i1[childId] };
		else
			diag1 = other.diagonalMap[triangle1][other.children_v0[childId]];  //in parent decomp: is going to be diag to children seperator

		check = CGAL::bounded_side_2(v0_t.begin(), v0_t.end(), p2);
		// || check == CGAL::ON_BOUNDARY || check == CGAL::ON_BOUNDED_SIDE
		if (triangle2 == v0 || check == CGAL::ON_BOUNDARY || check == CGAL::ON_BOUNDED_SIDE)
			diag2 = { 0,1 }; //first two vertices!
		else
			diag2 = diagonalMap[triangle2][v0]; //from child to parent: parent diagonal is always 0->1?
		//WARNING: diags might be incorrect: look into more
	}
	else {
		CGAL::Bounded_side check = CGAL::bounded_side_2(v0_t.begin(), v0_t.end(), p1);

		if (triangle1 == v0)
			diag1 = { 0,1 };
		else
			diag1 = diagonalMap[triangle1][v0];

		check = CGAL::bounded_side_2(other.v0_t.begin(), other.v0_t.end(), p2);

		// || check == CGAL::ON_BOUNDARY || check == CGAL::ON_BOUNDED_SIDE
		if (triangle2 == other.v0 || check == CGAL::ON_BOUNDARY || check == CGAL::ON_BOUNDED_SIDE)
			diag2 = { 0,1 };
		else
			diag2 = other.diagonalMap[triangle2][other.v0];
	}
	Point_2 diag1p0, diag1p1, diag2p0, diag2p1;
	if (status == 0) {
		diag1p0 = poly[diag1[0]];
		diag1p1 = poly[diag1[1]];
		diag2p0 = poly[diag2[0]];
		diag2p1 = poly[diag2[1]];
	}
	else if (status == 1) {
		diag1p0 = other.poly[diag1[0]]; //diag1 in papa
		diag1p1 = other.poly[diag1[1]];
		diag2p0 = poly[diag2[0]]; //diag2 in baby
		diag2p1 = poly[diag2[1]];
	}
	else {
		diag1p0 = poly[diag1[0]]; //diag1 in us
		diag1p1 = poly[diag1[1]];
		diag2p0 = other.poly[diag2[0]]; //diag2 in other
		diag2p1 = other.poly[diag2[1]];
	}
	std::sort(diag1.begin(), diag1.end()); //same order
	std::sort(diag2.begin(), diag2.end()); //same order

	bool p1_on_diag = inf1.type == lt::EDGE && fast_has_on(diag1p0, diag1p1, p1);
	bool p2_on_diag = inf2.type == lt::EDGE && fast_has_on(diag2p0, diag2p1, p2);
	//if (p1_on_diag || p2_on_diag)
	//{
	//	int crisb = 5;
	//	CDT_FH t1;
	//	CDT_FH t2;

	//	if (p1_on_diag) {
	//		//we are either on the diag edge or on the vertex.. simply move our foundface to the correct neighbour
	//		int j = -1;

	//		lt type;

	//		if(status == 1)				
	//			t1 = other.get_loc(p1, type);
	//		else
	//			t1 = get_loc(p1, type);

	//		for (int i = 0; i < 3; i++)
	//		{
	//			if (t1->vertex(i)->info().id != diag1[0] && t1->vertex(i)->info().id != diag1[1]) {
	//				j = i;
	//				break;
	//			}
	//		}
	//		t1 = t1->neighbor(j);
	//	}

	//	if (p2_on_diag) {
	//		lt type;

	//		if (status == 2)
	//			t2 = other.get_loc(p2, type);
	//		else
	//			t2 = get_loc(p2, type);

	//		//we are either on the diag edge or on the vertex.. simply move our foundface to the correct neighbour
	//		int j = -1;
	//		for (int i = 0; i < 3; i++)
	//		{
	//			if (t2->vertex(i)->info().id != diag2[0] && t2->vertex(i)->info().id != diag2[1]) {
	//				j = i;
	//				break;
	//			}
	//		}
	//		t2 = t2->neighbor(j);
	//	}

	//	if (p1_on_diag && t1->info().id > -1)
	//		triangle1 = t1->info().id;
	//	if (p2_on_diag && t2->info().id > -1)
	//		triangle2 = t2->info().id;

	//	//Update!
	//	if (status == 0) {
	//		diag1 = diagonalMap[triangle1][triangle2];
	//		diag2 = diagonalMap[triangle2][triangle1];
	//	}
	//	else if (status == 1) {


	//		CGAL::Bounded_side check = CGAL::bounded_side_2(other.children_v0_t[childId].begin(), other.children_v0_t[childId].end(), p1);

	//		if (triangle1 == other.children_v0[childId] || check == CGAL::ON_BOUNDARY || check == CGAL::ON_BOUNDED_SIDE)
	//			diag1 = { other.children_i0[childId], other.children_i1[childId] };
	//		else
	//			diag1 = other.diagonalMap[triangle1][other.children_v0[childId]];  //in parent decomp: is going to be diag to children seperator

	//		check = CGAL::bounded_side_2(v0_t.begin(), v0_t.end(), p2);

	//		if (triangle2 == v0 || check == CGAL::ON_BOUNDARY || check == CGAL::ON_BOUNDED_SIDE)
	//			diag2 = { 0,1 }; //first two vertices!
	//		else
	//			diag2 = diagonalMap[triangle2][v0]; //from child to parent: parent diagonal is always 0->1?
	//		//WARNING: diags might be incorrect: look into more
	//	}
	//	else {
	//		CGAL::Bounded_side check = CGAL::bounded_side_2(v0_t.begin(), v0_t.end(), p1);

	//		if (triangle1 == v0)
	//			diag1 = { 0,1 };
	//		else
	//			diag1 = diagonalMap[triangle1][v0];

	//		check = CGAL::bounded_side_2(other.v0_t.begin(), other.v0_t.end(), p2);


	//		if (triangle2 == other.v0 || check == CGAL::ON_BOUNDARY || check == CGAL::ON_BOUNDED_SIDE)
	//			diag2 = { 0,1 };
	//		else
	//			diag2 = other.diagonalMap[triangle2][other.v0];
	//	}
	//	Point_2 diag1p0, diag1p1, diag2p0, diag2p1;
	//	if (status == 0) {
	//		diag1p0 = poly[diag1[0]];
	//		diag1p1 = poly[diag1[1]];
	//		diag2p0 = poly[diag2[0]];
	//		diag2p1 = poly[diag2[1]];
	//	}
	//	else if (status == 1) {
	//		diag1p0 = other.poly[diag1[0]]; //diag1 in papa
	//		diag1p1 = other.poly[diag1[1]];
	//		diag2p0 = poly[diag2[0]]; //diag2 in baby
	//		diag2p1 = poly[diag2[1]];
	//	}
	//	else {
	//		diag1p0 = poly[diag1[0]]; //diag1 in us
	//		diag1p1 = poly[diag1[1]];
	//		diag2p0 = other.poly[diag2[0]]; //diag2 in other
	//		diag2p1 = other.poly[diag2[1]];
	//	}
	//	std::sort(diag1.begin(), diag1.end()); //same order
	//	std::sort(diag2.begin(), diag2.end()); //same order
	//}



	if (status == 0 && triangle1 == triangle2)
		return Path(p1, p2, {});



	if (diag1[0] == diag1[1] || diag2[0] == diag2[1])
		OutputDebugString(std::string("Starting:").c_str());





	if (status == 0 && 
		(fast_has_on(poly[diag1[0]], poly[diag1[1]], p2) ||
		fast_has_on(poly[diag2[0]], poly[diag2[1]], p1)))
		return Path(p1, p2, {});

	if (!vertex1 && !vertex2) {
		//First funnel is (trivial) triangle p1 to diag
		Funnel f1(Path(p1, diag1p0, {}), Path(p1, diag1p1, {}));
		//Fourth funnel is (trivial) tiangle p2 to diag2
		Funnel f4(Path(p2, diag2p0, {}), Path(p2, diag2p1, {}));

		//check if diagonals are the same
		if (status == 0 && diag1[0] == diag2[0] && diag1[1] == diag2[1])
		{
			return f1.Merge(f4); //only one merge necessary
		}
		else {			
			//Second funnel is from diag2[0] to diag
			Funnel f2;
			//Third funnel is from diag2[1] to diag
			Funnel f3;
			Path path1;
			Path path2;

			if (status == 0) {
				f2 = Funnel(map[diag2[0]][diag1[0]], map[diag2[0]][diag1[1]]);
				f3 = Funnel(map[diag2[1]][diag1[0]], map[diag2[1]][diag1[1]]);
				
				//Do necessary merging to create fitfh funnel
				path1 = f2.Merge(f1);
				path2 = f3.Merge(f1);
				path1.reverse();
				path2.reverse();
				Funnel f5(path1, path2);

				//The final shortest path will be the result of the last merge
				return f5.Merge(f4);
			}
			else if(status == 1){

				//diag1 is in parent
				//diag2 is in child
				f2 = Funnel(parentMap[diag1[0]][diag2[0]], parentMap[diag1[0]][diag2[1]]);
				f3 = Funnel(parentMap[diag1[1]][diag2[0]], parentMap[diag1[1]][diag2[1]]);
				
				
				path1 = f2.Merge(f4);
				path2 = f3.Merge(f4);
				path1.reverse();
				path2.reverse();

				Funnel f5(path1, path2);
				Path res = f5.Merge(f1);
				res.reverse();
				return res;
			}		
			else {
				f2 = Funnel(other.siblingMap[childId][diag2[0]][diag1[0]], other.siblingMap[childId][diag2[0]][diag1[1]]);
				f3 = Funnel(other.siblingMap[childId][diag2[1]][diag1[0]], other.siblingMap[childId][diag2[1]][diag1[1]]);

				//Do necessary merging to create fitfh funnel
				path1 = f2.Merge(f1);
				path2 = f3.Merge(f1);
				path1.reverse();
				path2.reverse();
				Funnel f5(path1, path2);

				//The final shortest path will be the result of the last merge
				return f5.Merge(f4);
			}
			
		}
	}
	else if (vertex1 && !vertex2) {
		
		//First funnel is easy, from v1 to diag2
		Funnel f1;
		
		if(status == 0)
			f1 = Funnel(map[v1][diag2[0]], map[v1][diag2[1]]);
		else if(status == 1)
			f1 = Funnel(parentMap[v1][diag2[0]], parentMap[v1][diag2[1]]);
		else
			f1 = Funnel(siblingMap[other.childId][v1][diag2[0]], siblingMap[other.childId][v1][diag2[1]]);

		//Second funnel is (trivial) tiangle p2 to diag2
		Funnel f2(Path(p2, diag2p0, {}), Path(p2, diag2p1, {}));
		return f1.Merge(f2);
	}
	else { //vertex2, !vertex1
		
		//First funnel is (trivial) tiangle p1 to diag
		Funnel f1(Path(p1, diag1p0, {}), Path(p1, diag1p1, {}));

		//Second funnel is easy, from v2 to diag1
		Funnel f2;		
		if(status == 0)
			f2 = Funnel(map[v2][diag1[0]], map[v2][diag1[1]]);
		else if(status == 1){
			Path p1 = parentMap[diag1[0]][v2];
			Path p2 = parentMap[diag1[1]][v2];
			p1.reverse();
			p2.reverse();
			f2 = Funnel(p1, p2 );			
		}
		else {
			f2 = Funnel(other.siblingMap[childId][v2][diag1[0]], other.siblingMap[childId][v2][diag1[1]]);
		}
		Path res = f2.Merge(f1);
		res.reverse();
		return res;
	}
}

Path Shortest_Map::shortest_point_point(Vertex_info p, Vertex_info q, Point_2 p_p, Point_2 q_p, bool &found) {
	Path path;
	found = false;

	if (p.decomp_index == q.decomp_index) {
		//same decomp
		path = decomp[p.decomp_index].shortest_point_point(p_p, q_p, p, q);
		found = true;
	}
	else if (decomp[p.decomp_index].parentId == q.decomp_index) {
		//p is child of q
		if (fast_has_on(decomp[p.decomp_index].poly[0], decomp[p.decomp_index].poly[1], p_p)) {
			//p is actually in parent map...
			//locate p in parent decomp
			Vertex_info inf = decomp[q.decomp_index].get_location(inf, p_p);
			path = decomp[q.decomp_index].shortest_point_point(p_p, q_p, inf, q);

		}
		else if (fast_has_on(decomp[p.decomp_index].poly[0], decomp[p.decomp_index].poly[1], q_p)) {
			//q is actually in child map!
			Vertex_info inf = decomp[p.decomp_index].get_location(inf, q_p);
			path = decomp[p.decomp_index].shortest_point_point(p_p, q_p, p, inf);
		}
		else {
			path = decomp[p.decomp_index].shortest_point_point(q_p, p_p, q, p, 1, decomp[q.decomp_index]);
			path.reverse();
		
		}
		found = true;

	}
	else if (decomp[q.decomp_index].parentId == p.decomp_index) {
		//q is child of p
		if (fast_has_on(decomp[q.decomp_index].poly[0], decomp[q.decomp_index].poly[1], q_p)) {
			//q is actually in parent map...
			//locate p in parent decomp
			Vertex_info inf = decomp[p.decomp_index].get_location(inf, q_p);
			path = decomp[p.decomp_index].shortest_point_point(p_p, q_p, p, inf);

		}
		else if (fast_has_on(decomp[q.decomp_index].poly[0], decomp[q.decomp_index].poly[1], p_p)) {
			//p is actually in child map!
			Vertex_info inf = decomp[q.decomp_index].get_location(inf, p_p);
			path = decomp[q.decomp_index].shortest_point_point(p_p, q_p, inf, q);
		}
		else {
			path = decomp[q.decomp_index].shortest_point_point(p_p, q_p, p, q, 1, decomp[p.decomp_index]);
			
		}
		found = true;

	}
	else {
		int parentId = decomp[q.decomp_index].parentId;

		if (parentId == decomp[p.decomp_index].parentId) {
			for (int i = 0; i < decomp[decomp[q.decomp_index].parentId].children.size(); i++) {
				if (p.decomp_index == decomp[decomp[q.decomp_index].parentId].children[i]) {
					found = true;
					bool p_in_diag = fast_has_on(decomp[p.decomp_index].poly[0], decomp[p.decomp_index].poly[1], p_p);
					bool q_in_diag = fast_has_on(decomp[q.decomp_index].poly[0], decomp[q.decomp_index].poly[1], q_p);

					if (p_in_diag && q_in_diag) {
						//they are actually both in parent
						Vertex_info infP = decomp[parentId].get_location(infP, p_p);
						Vertex_info infQ = decomp[parentId].get_location(infQ, q_p);
						path = decomp[parentId].shortest_point_point(p_p, q_p, infP, infQ);

					}
					else if (p_in_diag)
					{
						//p is actually in parent
						Vertex_info inf = decomp[parentId].get_location(inf, p_p);
						path = decomp[q.decomp_index].shortest_point_point(p_p, q_p, inf, q, 1, decomp[parentId]);						
					}
					else if (q_in_diag)
					{
						//q is actually in parent
						Vertex_info inf = decomp[parentId].get_location(inf, q_p);
						path = decomp[p.decomp_index].shortest_point_point(q_p, p_p, inf, p, 1, decomp[parentId]);
						path.reverse();
						

					}
					else
						path = decomp[p.decomp_index].shortest_point_point(p_p, q_p, p, q, 2, decomp[q.decomp_index]);
				}
			}
		}
	}
	   

	/*if (path.points.size() == 0 && path.set) {
		Segment_2 seg(path.from, path.to);
		for (int i = 0; i < decomp[p.decomp_index].poly.size();i++) {
			int next = (i + 1) % decomp[p.decomp_index].poly.size();
			Segment_2 edge(decomp[p.decomp_index].poly[i], decomp[p.decomp_index].poly[next]);
			Point_2 inter = point_intersection(edge, seg);
			if (isset(inter) && !same(inter, edge.source()) &&
				!same(inter, edge.target()) &&
				!same(inter, seg.source()) &&
				!same(inter, seg.target())
				)
				int crisb = 15;
		}

	}*/

	return path; //empty if found is false
}


bool Shortest_Map::segment_sees_point(Vertex_info s1, Vertex_info s2, Vertex_info p, Point_2 s1_p, Point_2 s2_p, Point_2 p_p) {
	if (fast_has_on(s1_p, s2_p, p_p))
		return true;

	bool found1 = false;
	bool found2 = false;

	Path p1 = shortest_point_point(p, s1, p_p, s1_p, found1);
	Path p2 = shortest_point_point(p, s2, p_p, s2_p, found2);

	if (!found1 && !found2)
		return false;


	if (s1.decomp_index != s2.decomp_index) {
		int crisb = 5; //this should never happen?
	}

	//Can the segment s see point p?
	//Compute shortest path funnel from p to s_1 and s_2

	Funnel f1(p1, p2);
	


	if (f1.split_index > -1)
		return false;
	else {
		if (p1.points.empty() || p2.points.empty())
			return true;

		if (fast_has_on(p1.from, p1.points[0], p2.points[0]))
			return false;
		if (fast_has_on(p2.from, p2.points[0], p1.points[0]))
			return false;

		//check for collinearity
		for (int i = 0; i < p1.points.size(); i++) {
			int nexti = (i + 1) % p1.points.size();
			for (int j = 0; j < p2.points.size(); j++) {
				int nextj = (j + 1) % p2.points.size();
				if (fast_has_on(p1.points[i], p1.points[nexti], p2.points[j]))
					return false;
				if (fast_has_on(p2.points[j], p2.points[nextj], p1.points[i]))
					return false;
			}
		}
		return true;
	}
}

//bool Shortest_Map::point_sees_point(Vertex_handle &p, Vertex_handle &q) {
//	if (same(q->point(), p->point()))
//		return true;
//	
//	bool found;
//	Path path = shortest_point_point(p, q, found);
//
//	if (!found)
//		return false;
//
//	//any wrong turns?
//	for (int i = 0; i < path.points.size();i++) {
//		CGAL::Orientation or1 = CGAL::orientation(p->point(), q->point(), path.points[i]);
//
//		if (or1 != CGAL::COLLINEAR)
//			return false;
//	}
//	return true; 
//}

Vertex_info Shortest_Map::get_location(Vertex_info &inf, Vertex_handle v) {
	////first we must find the correct wv
	//for (int i = 0; i < decomp.size();i++) {
	//	CGAL::Bounded_side loc = CGAL::bounded_side_2(decomp[i].poly.begin(), decomp[i].poly.end(), v->point());
	//	if (loc == CGAL::ON_BOUNDED_SIDE) {
	//		inf.decomp_index = i;
	//		inf = decomp[i].get_location(inf, v);

	//		//done
	//		break;
	//	}
	//	else if (loc == CGAL::ON_BOUNDARY) {
	//		bool found = false;
	//		for (int j = 0; j < decomp[i].neighbours.size();j++) {
	//			//check all neighbours
	//			Segment_2 seg(decomp[decomp[i].neighbours[j]].poly[0], decomp[decomp[i].neighbours[j]].poly[1]);
	//			if (seg.has_on(v->point())) {
	//				inf.decomp_index = decomp[i].neighbours[j];
	//				inf = decomp[inf.decomp_index].get_location(inf, v);
	//				found = true;
	//				break;
	//			}				
	//		}
	//		if (!found) {
	//			inf.decomp_index = i;
	//			inf = decomp[i].get_location(inf, v);
	//			break;
	//		}
	//	}
	//}
	return inf;
}

void Shortest_Map::recursive_decomp(std::vector<Point_2> sub_poly, int prev_id) {
	
	//wtf
	WeakVisPoly wv = WeakVisPoly(sub_poly, prev_id);
	
	prev_id = decomp.size();
	wv.id = prev_id;

	if(wv.parentId > -1)
		wv.childId = decomp[wv.parentId].children.size() - 1;
	decomp.push_back(wv);	   

	
	for (int i = 0; i < wv.index_map.size(); i++) {
		int prev = i - 1;
		if (prev == -1)
			prev = wv.index_map.size() - 1;
		std::vector<Point_2> new_sub_poly = { wv.poly[i], wv.poly[prev] };

		if (wv.index_map[i] == -1)
			return;

		int diff = difference_in_polygon(wv.index_map[prev], wv.index_map[i], sub_poly.size());
		bool create = false;

		if (wv.on_segment[prev]) {
			if (diff > 0) {
				//this time include prev
				for (int j = wv.index_map[prev]; j != wv.index_map[i]; j = (j + 1) % sub_poly.size()) {
					new_sub_poly.push_back(sub_poly[j]);
					create = true;
				}
			}
		}
		else if (wv.on_segment[i]) {
			if (diff > 1) {
				//both are on vertices (don't include prev, its already in the first)
				for (int j = wv.index_map[prev] + 1; j != wv.index_map[i]; j = (j + 1) % sub_poly.size()) {
					new_sub_poly.push_back(sub_poly[j]);
					create = true;
				}
			}
		}
		else if (diff > 1) {
			//both are on vertices (don't include prev, its already in the first)
			for (int j = (wv.index_map[prev] + 1) % sub_poly.size(); j != wv.index_map[i]; j = (j + 1) % sub_poly.size()) {
				new_sub_poly.push_back(sub_poly[j]);
				create = true;
			}
		}


		if (create) {
			//lets go boys
			if (new_sub_poly.size() > 2) {

				int i0 = i;
				int i1 = prev;

				Polygon_2 temp(new_sub_poly.begin(), new_sub_poly.end());

				/*try {
					try {*/
					//someone tell me why??
				if (temp.orientation() == CGAL::NEGATIVE) {
					std::vector<Point_2> tempest = temp.container();
					Polygon_2 test(wv.poly.begin(), wv.poly.end());
					Polygon_2 test2(sub_poly.begin(), sub_poly.end());

					CGAL::Orientation or1 = test.orientation();
					CGAL::Orientation or2 = test2.orientation();

					//reverse fix??
					std::reverse(tempest.begin(), tempest.end());

					new_sub_poly = {};

					//1 and then 0
					new_sub_poly.push_back(tempest[tempest.size() - 2]);
					new_sub_poly.push_back(tempest.back());
					new_sub_poly.insert(new_sub_poly.end(), tempest.begin(), tempest.end() - 2); //the rest


					i0 = prev;
					i1 = i;
				}

				decomp[prev_id].children.push_back(decomp.size());
				decomp[prev_id].children_i0.push_back(i0);
				decomp[prev_id].children_i1.push_back(i1);
				decomp[prev_id].children_v0.push_back(-1);
				decomp[prev_id].children_v0_t.push_back({});
				recursive_decomp(new_sub_poly, prev_id);
				/*}
				catch (std::exception e) {
					return;
				}
			}
			catch (...) {
				return;
			}*/
			}
		}
	}
	
	////find diagonals
	//int ori = 0;
	//int finder = 0;	
	//
	//while (ori < sub_poly.size() && finder < wv.poly.size()) {
	//	if (finder > 0 && same(wv.poly[finder - 1], wv.poly[finder])) {
	//		int crisb = 0;
	//	}
	//	if (!same(sub_poly[ori], wv.poly[finder]))
	//	{	
	//		int i0 = -1;
	//		int i1 = -1;

	//		//there must be a new edge!
	//		int prev_ori = ori - 1;
	//		if (prev_ori < 0)
	//			prev_ori = sub_poly.size() - 1;
	//		int prev_finder = finder - 1;
	//		if (prev_finder < 0)
	//			prev_finder = wv.poly.size() - 1;

	//		Segment_2 seg(sub_poly[prev_ori], sub_poly[ori]);
	//		std::vector<Point_2> new_sub_poly;
	//		if (seg.has_on(wv.poly[finder])) {
	//			//Case 1: new is in middle of segment
	//			//Place holder point
	//			new_sub_poly = { wv.poly[finder+1], wv.poly[finder] };
	//			i0 = finder + 1;
	//			i1 = finder;

	//			//We have to look for the next point
	//			finder = (finder + 1) % sub_poly.size();

	//			//first 
	//			while (ori < sub_poly.size()) {
	//				if (!same(sub_poly[ori], wv.poly[finder]))
	//					new_sub_poly.push_back(sub_poly[ori]);
	//				else {
	//					
	//					break;
	//				}
	//				ori++;
	//			}

	//		}
	//		else {		
	//			new_sub_poly = { wv.poly[finder], wv.poly[prev_finder] };
	//			i0 = finder;
	//			i1 = prev_finder;

	//			prev_ori = ori;
	//			//first 
	//			while (ori < sub_poly.size() + 1) {
	//				seg = Segment_2(sub_poly[prev_ori], sub_poly[ori]);
	//				if (!seg.has_on(wv.poly[finder])) {
	//					new_sub_poly.push_back(sub_poly[ori]);
	//				}
	//				else {
	//					if(!same(sub_poly[ori], wv.poly[finder]))
	//						ori--; //make sure we are in sync again
	//					//finder--;
	//					break;
	//				}

	//				prev_ori = ori;
	//				ori++;
	//			}
	//		}
	//		//new recursive call!		
	//		decomp[prev_id].children.push_back(decomp.size());
	//		decomp[prev_id].children_i0.push_back(i0);
	//		decomp[prev_id].children_i1.push_back(i1);
	//		decomp[prev_id].children_v0.push_back(-1);
	//		decomp[prev_id].children_v0_t.push_back({});

	//		if(new_sub_poly.size() > 2)
	//			recursive_decomp(new_sub_poly, prev_id);
	//	}
	//	ori++;
	//	finder++;		
	//}

}

Shortest_Map::Shortest_Map(std::vector<Point_2> poly) {
	this->poly = poly;   	
	decomp = {};
	
	bool done = false;

	int max_time = (int)poly.size() / 2;
	std::thread timer(
		[&done, max_time]()
		{
			int time_taken = 0;
			
			while (!done)
			{
				if (time_taken > max_time)
					exit(0);
				std::this_thread::sleep_for(std::chrono::seconds(5));
				time_taken += 5;
			}
		});

	std::srand(std::time(NULL));
	bool first = true;
	//do first
	while(true){
		//rotate once to change beginning segment
			
		int randm = rand() % (poly.size()-1) + 1;
		if (first)
		{
			randm = 0;
			first = false;
		}
		
		std::rotate(this->poly.begin(), this->poly.begin() + randm, this->poly.end());
		try {
			
			recursive_decomp(this->poly, -1); 
			//return;

			//find v0,v1 and children v0/v1
			for (int i = 0; i < decomp.size(); i++) {
				triangle_loc tl = decomp[i].find_segment(decomp[i].poly[0], decomp[i].poly[1]);
				decomp[i].v0 = tl.t_id;
				decomp[i].v0_t = tl.pts;

				for (int j = 0; j < decomp[i].children.size(); j++) {
					//find babies
					tl = decomp[i].find_segment(decomp[i].poly[decomp[i].children_i0[j]], decomp[i].poly[decomp[i].children_i1[j]]);;
					decomp[i].children_v0_t[j] = tl.pts;
					decomp[i].children_v0[j] = tl.t_id;
				}
			}

			//Make parent maps (starting at 1, 0 has no parent)
			for (int i = 1; i < decomp.size(); i++) {
				if (decomp[i].parentId > -1) {
					WeakVisPoly parent = decomp[decomp[i].parentId];
					decomp[i].parentMap = std::vector<std::vector<Path>>(parent.poly.size(), std::vector<Path>(decomp[i].poly.size()));

					for (int j = 0; j < decomp[decomp[i].parentId].poly.size(); j++) {

						//parent funnel
						Funnel f1(
							parent.map[j][parent.children_i0[decomp[i].childId]],
							parent.map[j][parent.children_i1[decomp[i].childId]]
						);


						if (j == parent.children_i0[decomp[i].childId]) {
							decomp[i].parentMap[j][0] = decomp[i].map[0][0];
							decomp[i].parentMap[j][1] = decomp[i].map[0][1];
						}
						else if (j == parent.children_i1[decomp[i].childId]) {
							decomp[i].parentMap[j][0] = decomp[i].map[1][0];
							decomp[i].parentMap[j][1] = decomp[i].map[1][1];
						}
						else {
							if (j == 2 && decomp[i].parentId == 0)
								int crisb = 5;
							decomp[i].parentMap[j][0] = parent.map[j][parent.children_i0[decomp[i].childId]];
							decomp[i].parentMap[j][1] = parent.map[j][parent.children_i1[decomp[i].childId]];
						}
						for (int k = 2; k < decomp[i].poly.size(); k++) {
							if (j == parent.children_i0[decomp[i].childId]) {
								decomp[i].parentMap[j][k] = decomp[i].map[0][k];
							}
							else if (j == parent.children_i1[decomp[i].childId]) {
								decomp[i].parentMap[j][k] = decomp[i].map[1][k];
							}
							else {
								Funnel f2(
									decomp[i].map[k][0],
									decomp[i].map[k][1]
								);




								decomp[i].parentMap[j][k] = f1.Merge(f2);
							}
						}
					}
				}
			}
			for (int i = 0; i < decomp.size(); i++) {
				if (decomp[i].children.size() > 1) {
					//for every child
					for (int j = 0; j < decomp[i].children.size() - 1; j++) {
						decomp[decomp[i].children[j]].siblingMap.push_back({}); //map to yourself is empty!

						WeakVisPoly j_child = decomp[decomp[i].children[j]];

						//for every other child		
						for (int k = j + 1; k < decomp[i].children.size(); k++) {
							WeakVisPoly k_child = decomp[decomp[i].children[k]];

							decomp[decomp[i].children[j]].siblingMap.push_back(
								std::vector<std::vector<Path>>(j_child.poly.size(), std::vector<Path>(k_child.poly.size()))
							); //map from j_child to k_child
							decomp[decomp[i].children[k]].siblingMap.push_back(
								std::vector<std::vector<Path>>(k_child.poly.size(), std::vector<Path>(j_child.poly.size()))
							); //map from k_child to j_child

							//for every point of the jth child
							for (int j_1 = 0; j_1 < j_child.poly.size(); j_1++) {
								//from j_1 to window of j_1 (0,1)
								Funnel f1(j_child.map[j_1][0], j_child.map[j_1][1]);

								for (int k_1 = 0; k_1 < k_child.poly.size(); k_1++) {
									//from the window of j_child to k_child!
									Path p1;
									Path p2;

									//if (k_1 == 0) {
									//	p1 = j_child.parentMap[decomp[i].children_i0[k_child.childId]][j_1];
									//	p1.reverse();
									//	p2 = j_child.parentMap[decomp[i].children_i0[k_child.childId]][j_1];
									//	//just check parentMap!
									//	decomp[decomp[i].children[j]].siblingMap[k][j_1][k_1] = p1;
									//	decomp[decomp[i].children[k]].siblingMap[j][k_1][j_1] = p2;
									//	

									//}
									//else if (k_1 == 1) {
									//	p1 = j_child.parentMap[decomp[i].children_i1[k_child.childId]][j_1];
									//	p1.reverse();
									//	p2 = j_child.parentMap[decomp[i].children_i1[k_child.childId]][j_1];
									//	//just check parentMap!
									//	decomp[decomp[i].children[j]].siblingMap[k][j_1][k_1] = p1;
									//	decomp[decomp[i].children[k]].siblingMap[j][k_1][j_1] = p2;
									//}

									

									p1 = decomp[decomp[i].children[k]].parentMap[decomp[i].children_i0[j_child.childId]][k_1];
									p2 = decomp[decomp[i].children[k]].parentMap[decomp[i].children_i1[j_child.childId]][k_1];
									p1.reverse();
									p2.reverse();

									if (j_1 == 0) {
										//no merge needed
										decomp[decomp[i].children[k]].siblingMap[j][k_1][j_1] = p1;
										Path p3 = Path(p1);
										p3.reverse();
										decomp[decomp[i].children[j]].siblingMap[k][j_1][k_1] = p3;
									}
									else if (j_1 == 1) {
										//no merge needed
										decomp[decomp[i].children[k]].siblingMap[j][k_1][j_1] = p2;
										Path p3 = Path(p2);
										p3.reverse();
										decomp[decomp[i].children[j]].siblingMap[k][j_1][k_1] = p3;
									}
									else {

										//from k_poly to j_window
										Funnel f2(
											p1,
											p2
										);

										Path merged = f1.Merge(f2);
										decomp[decomp[i].children[j]].siblingMap[k][j_1][k_1] = merged;
										decomp[decomp[i].children[k]].siblingMap[j][k_1][j_1] = merged.reverse();
									}

								}

							}
						}
					}

				}

			}
			break;
		}
		catch (...) {
			OutputDebugString(std::string("Starting:").c_str());
			this->decomp = {}; //empty decomp!
		}
	}
	done = true;
	timer.join();

	int crisb = 5;
}


