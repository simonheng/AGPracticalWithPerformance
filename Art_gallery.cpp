#include "Art_gallery.h"



void print_time() {
	/*std::chrono::system_clock::time_point now = std::chrono::system_clock::now();
	std::time_t now_c = std::chrono::system_clock::to_time_t(now);
	std::tm now_tm = *std::localtime(&now_c);
	std::stringstream ss;
	ss << std::put_time(&now_tm, std::string("UTC: %Y-%m-%d %H:%M:%S").c_str());*/
	//OutputDebugString(ss.str().c_str());
	//OutputDebugString("\n");


	/*FILE *fp;
	char str[MAXCHAR];
	char* filename = "c:\\dev\\output.txt";

	fp = fopen(filename, "a");

	LPCSTR built = std::string(
		ss.str() +
		"\n" 
	).c_str();
	fprintf(fp, built);
	fclose(fp);*/

}

bool Guard::sees_witness(Vertex_handle &w, int version, Shortest_Map &sm, std::vector<PointGuard> &pg, std::vector<FaceGuard> &fg)
{
	int index = w->data().id;
	if (sees_points[index] == 's')
		return true;
	if (sees_points[index] == 'u')
		return false;
	if (parent > -1 && fg[parent].sees_points[index] == 'u') {
		sees_points[index] = 'u';
		return false; //parent didn't see it, we cannot either
	}
	bool seen;
	if (version == 0) {
		CGAL::Bounded_side b = CGAL::bounded_side_2(vis.container().begin(), vis.container().end(), w->point());
		seen = b != CGAL::ON_UNBOUNDED_SIDE;		
	}
	else if (version == 1) {
		seen = sees_witness_v1(w, sm, pg, fg);
	}

	if (seen)
		sees_points[index] = 's'; //make sure to update quick look-up
	else
		sees_points[index] = 'u';
	return seen;
}

bool Guard::sees_witness(std::vector<Vertex_handle> &pts, int version, Shortest_Map &sm, std::vector<PointGuard> &pg, std::vector<FaceGuard> &fg)
{	
	//either index == -1 or sees_faces[index] = -1
	bool seen = true;
	for (int i = 0; i < pts.size();i++) {	
		if (!sees_witness(pts[i], version, sm, pg, fg)) {//check all vertices
			seen = false;
			break;
		}
	}
	return seen;
}
bool Guard::sees_witness_v1(Vertex_handle &w, Shortest_Map &sm, std::vector<PointGuard> &pg, std::vector<FaceGuard> &fg) {
	return true;
}
bool PointGuard::sees_witness_v1(Vertex_handle &w, Shortest_Map &sm, std::vector<PointGuard> &pg, std::vector<FaceGuard> &fg) {
	if (pg[w->data().id].sees_points[v->data().id] == 's')
		return true;
	else if (pg[w->data().id].sees_points[v->data().id] == 'u')
		return false;
	bool sees = (CGAL::bounded_side_2(vis.container().begin(), vis.container().end(), w->point()) != CGAL::ON_UNBOUNDED_SIDE);
	
	//save inverse too (for faceguards!)
	if (sees) 
		pg[w->data().id].sees_points[v->data().id] = 's';
	else
		pg[w->data().id].sees_points[v->data().id] = 'u';
	return sees;
	//}
	//_finally{
	//	int cirsb = 15;
	//}
	//bool result = b != CGAL::ON_UNBOUNDED_SIDE;
	//return result;
}


bool FaceGuard::sees_witness_v1(Vertex_handle &w, Shortest_Map &sm, std::vector<PointGuard> &pg, std::vector<FaceGuard> &fg) {
	for (int i = 0; i < sgms.size();i++) {
		//do any of our vertices see the point?
		if (pg[sgms[i]->source()->data().id].sees_points[w->data().id] == 's')
			return true;
	}

	bool wait = true;

	/*while (wait) {
		wait = false;
		for (int i = 0; i < sgms.size();i++) {
			if (sgms[i]->source()->data().lock) {
				wait = true;
				break;
			}
		}
		if (w->data().lock)
			wait = true;
	}*/


	//lock em boys
	/*for (int i = 0; i < sgms.size();i++)
		sgms[i]->source()->data().lock = true;
	w->data().lock = true;*/


	bool result = false;

	//do any of our segments see the point?
	for (int i = 0; i < sgms.size();i++) {
		//auto s1_x = sgms[i]->source()->point().x();
		//auto s1_y = sgms[i]->source()->point().y();
		//auto s2_x = sgms[i]->target()->point().x();
		//auto s2_y = sgms[i]->target()->point().y();
		//auto p_x = w->point().x();
		//auto p_y = w->point().y();


		//Point_2 s1(s1_x,s1_y);
		//Point_2 s2(s2_x, s2_y);
		//Point_2 p(p_x,p_y);

		if (sm.segment_sees_point(
			sgms[i]->source()->data(), sgms[i]->target()->data(), w->data(), 
			sgms[i]->source()->point(), sgms[i]->target()->point(), w->point())) {
			result = true;
			break;
		}
	}

	////release!
	//for (int i = 0; i < sgms.size();i++)
	//	sgms[i]->source()->data().lock = false;
	//w->data().lock = false;

	return result;

}


void Art_Gallery::insert_point_into_int(FaceGuard& f, TEV* tev) {

	int size = f.points.size();

	auto x = f.points[0].x()/size;
	auto y = f.points[0].y()/size;

	for (int i = 1; i < f.points.size(); i++) {
		x += f.points[i].x() / size;
		y += f.points[i].y() / size;
	}

	Point_2 point_to_insert(x, y);

	Vertex_handle v = arrs[f.f->data().decomp_index].insert_in_face_interior(point_to_insert, f.f);

	OutputDebugString(".");

	Vertex_info inf;
	inf.id = all_vertices_size;

	point_ids.push_back(inf.id);
	if (version == 1)
		inf = sm.decomp[f.f->data().decomp_index].get_location(inf, point_to_insert);
	inf.decomp_index = f.f->data().decomp_index;
	v->set_data(inf);

	PointGuard pg = PointGuard(v, version, poly_arr, tev);
	pointguard_var.add(IloBoolVar(env, 0, 1));
	pg.parent = f.f->data().id;

	pointguards.push_back(pg);
	pointguards.back().expr = IloExpr(env);

	pg.is_critical = true; //always critical
	critical_points.push_back(inf.id);
	all_vertices.push_back(v);
	all_vertices_size++;
	point_ids.push_back(inf.id);
	f.interior_p = inf.id;
}

PointGuard::PointGuard(Vertex_handle v, int version, Arrangement_2& arr, TEV* tev) { // Constructor with parameters
	sees_points = std::vector<char>(30000, 'n');
	parent = -1;
	id = v->data().id;
	try {
		double x = CGAL::to_double(v->point().x());
		this->vis = compute_visibility_from_point(v->point(), arr, tev);
	}
	catch (std::exception e) {
		double x = CGAL::to_double(v->point().x());
		double y = CGAL::to_double(v->point().y());
	}
	
	this->v = v;
}

FaceGuard::FaceGuard(Face_handle f, std::vector<Point_2> pgn, int version)
{ // Constructor with parameters
	if (version == 10)
		return;

	id = f->data().id;
	parent = -1;
	this->f = f;
	sees_points = std::vector<char>(30000, 'n');
	this->removed = false;
	Arrangement_2::Ccb_halfedge_circulator curr_h = f->outer_ccb();
	Polygon_with_holes_2 unionR;
	sgms.push_back(curr_h);
	sgms2.push_back(Segment_2(curr_h->source()->point(), curr_h->target()->point()));

	while (++curr_h != f->outer_ccb()) {
		sgms.push_back(curr_h);
		sgms2.push_back(Segment_2(curr_h->source()->point(), curr_h->target()->point()));
		points.push_back(curr_h->source()->point());
	}

	if (version == 0) {
		for (int i = 0; i < sgms.size(); i++)
			seg_vis.push_back(compute_visibility_from_segment(pgn, sgms[i]->source()->point(), sgms[i]->target()->point()));

		try {
			std::list<Polygon_with_holes_2> res;
			CGAL::join(seg_vis.begin(), seg_vis.end(), std::back_inserter(res));
			unionR = res.front();
		}
		catch (std::exception e) {
			OutputDebugString(std::to_string(CGAL::to_double(curr_h->curve().source().x())).c_str());
			OutputDebugString(" plus ");
			OutputDebugString(std::to_string(CGAL::to_double(curr_h->curve().source().y())).c_str());
			OutputDebugString("\n");

			OutputDebugString(std::to_string(CGAL::to_double(curr_h->curve().target().x())).c_str());
			OutputDebugString(" plus ");
			OutputDebugString(std::to_string(CGAL::to_double(curr_h->curve().target().y())).c_str());
			OutputDebugString("\n");
		}
		vis = unionR.outer_boundary();
	}
	
}

Arrangement_2 Art_Gallery::orthogonal_creation(std::vector<Point_2> poly)
	{
		//poly = transform(rational_rotate,Polygon_2(poly.begin(), poly.end())).container();
		Arrangement_2 arr = polygon_to_arrangement(poly);

		Face_handle fh;
		for (fh = arr.faces_begin(); fh != arr.faces_end(); ++fh) {
			if (fh->has_outer_ccb())
				break;
		}

		std::vector<Halfedge_handle> original_edges = {};
		Arrangement_2::Ccb_halfedge_circulator curr = fh->outer_ccb();
		original_edges.push_back(curr);
		while (++curr != fh->outer_ccb()) {
			original_edges.push_back(curr);
		}

		std::vector<Point_2> sources = {};
		std::vector<Point_2> targets = {};
		std::vector<Point_2> next_p = {};

		for (int i = 0; i < original_edges.size();i++) {
			Halfedge_handle curr = original_edges[i];
			Arrangement_2::Halfedge_handle next = original_edges[(i + 1) % original_edges.size()];
			CGAL::Orientation orient_check = CGAL::orientation(curr->source()->point(), curr->target()->point(), next->target()->point());
			if (orient_check == CGAL::RIGHT_TURN || orient_check == CGAL::COLLINEAR)
			{	
				sources.push_back(curr->source()->point());
				targets.push_back(curr->target()->point());
				next_p.push_back(next->target()->point());
			}
		}

		//shoot horizontal rays (mode = 1)
		std::vector<Segment_2> added_segs = shoot_rays(sources, targets, next_p, arr, 0);

		std::vector<Segment_2> intersected_segs = {};

		for (int i = 0; i < added_segs.size();i++) {
			Point_2 reflex = added_segs[i].source();
			Segment_2 new_seg = added_segs[i];
			std::vector<Point_2> intersections = {};

			//check if segment intersects with any of the previously checked segments
			for (int j = 0; j < intersected_segs.size();j++)
			{
				//check intersection
				Point_2 inter = point_intersection(added_segs[i], intersected_segs[j]);
				if (inter != empty_point && !same(inter, reflex))
				{
					intersections.push_back(inter);
				}
			}

			//Find closest
			if (intersections.size() > 0) {
				Point_2 closest = intersections[0];
				for (int j = 1; j < intersections.size();j++) {
					if (Segment_2(reflex, intersections[j]).squared_length() < Segment_2(reflex, closest).squared_length())
						closest = intersections[j];
				}
				new_seg = Segment_2(reflex, closest);
			}
			intersected_segs.push_back(new_seg);
		}

		insert(arr, intersected_segs.begin(), intersected_segs.end());
		return arr;
	}

std::vector<Segment_2> split_face(Face_handle f, int pieces, int mode)
{
		std::vector<Point_2> boundary = {};
		Arrangement_2::Ccb_halfedge_circulator curr = f->outer_ccb();
		boundary.push_back(curr->source()->point());
		while (++curr != f->outer_ccb()) {
			boundary.push_back(curr->source()->point());
		}

		Arrangement_2 arr = polygon_to_arrangement(boundary);
		pieces = max(1, pieces);
		
		Arrangement_2::Ccb_halfedge_circulator curr_h = f->outer_ccb();
		Point_2 curr_p = curr_h->source()->point();
		auto min_x = curr_p.x();
		auto max_x = curr_p.x();
		auto min_y = curr_p.y();
		auto max_y = curr_p.y();
		while (++curr_h != f->outer_ccb()) {
			curr_p = curr_h->source()->point();
			if (curr_p.x() > max_x)
				max_x = curr_p.x();
			if ((curr_p.x() < min_x))
				min_x = curr_p.x();
			if (curr_p.y() > max_y)
				max_y = curr_p.y();
			if ((curr_p.y() < min_y))
				min_y = curr_p.y();
		}
		int count = 0;

		auto x_length = max_x - min_x;
		auto y_length = max_y - min_y;


		double temp = CGAL::to_double(x_length);
		double temp2 = CGAL::to_double(y_length);

		std::vector<Point_2> empty_p;
		std::vector<Point_2> targets;
		for (int i = 1; i < pieces + 1;i++) {
			Point_2 mid_p = Point_2(min_x + static_cast<int>(i) * (x_length / (pieces + 1)), min_y + static_cast<int>(i) * (y_length / (pieces + 1)));
			empty_p.push_back(empty_point);
			targets.push_back(mid_p);
		}
			   
		//Decide mode:
		auto ratio = y_length / x_length;
		int ray_mode = 0;
		if (mode > -1)
			ray_mode = mode;
		else if (ratio > 0.5 && ratio < 2)
			ray_mode = 0; //shoot all directoins
		else if (ratio >= 2) {
			ray_mode = 1; //shoot left/right
		}
		else
		{
			ray_mode = 2; //shoot up/down
		}

		//compute visibility polygon of point
		std::vector<Segment_2> split_segs = shoot_rays(empty_p, targets, empty_p, arr, ray_mode);
		return split_segs;
	}

void Art_Gallery::witness_visibilities(std::vector<int> p_witness_ids, std::vector<int> f_witness_ids, std::vector<int> pg_ids, std::vector<int> fg_ids) {
	std::vector<std::vector<int>> f_witness_seen_by_pg(f_witness_ids.size(), std::vector<int>(pg_ids.size()));
	std::vector<std::vector<int>> p_witness_seen_by_pg(p_witness_ids.size(), std::vector<int>(pg_ids.size()));

	//do this non parallel to avoid 
	for (int j = 0; j < f_witness_ids.size();j++) {
		if (faceguards[f_witness_ids[j]].removed)
			continue; //skip this face, we don't need to see it

		//for each half_edge check if the target is inside the polygon
		std::vector<Vertex_handle> pts = {};
		Arrangement_2::Ccb_halfedge_circulator curr_h = all_faces[f_witness_ids[j]]->outer_ccb();
		pts.push_back(curr_h->source());
		while (++curr_h != all_faces[f_witness_ids[j]]->outer_ccb()) {
			pts.push_back(curr_h->source());
		}

		//check all guards:
		for (int i = 0; i < pg_ids.size(); i++) {
			if (pointguards[pg_ids[i]].removed)
				continue; //this point can't see anything

			if (pointguards[pg_ids[i]].sees_witness(pts, version, sm, pointguards, faceguards))
				f_witness_seen_by_pg[j][i] = 1;
		}
	}

	//do this non parallel to avoid 
	for (int j = 0; j < p_witness_ids.size();j++) {
		if (pointguards[p_witness_ids[j]].removed)
			continue; //skip this point, we don't need to see it

		//check all guards:
		for (int i = 0; i < pg_ids.size(); i++) {
			if (pointguards[pg_ids[i]].removed)
				continue; //this point can't see anything

			if (pointguards[pg_ids[i]].sees_witness(all_vertices[p_witness_ids[j]], version, sm, pointguards, faceguards))
				p_witness_seen_by_pg[j][i] = 1;
		}
	}



	int threads = std::thread::hardware_concurrency();
	ctpl::thread_pool p(threads /* threads in the pool */);
	//boost::asio::thread_pool pool(threads /* threads in the pool */);


	//witness_ids.size(); 
	//int stepsize = max(1, witness_ids.size() / threads);
	//int stepsize = witness_ids.size();
	//int stepsize = 1;
	//std::vector<std::thread> workers;

	
	std::vector<std::future<void>> results;
	std::vector<std::vector<int>> f_witness_seen_by_fg(f_witness_ids.size(), std::vector<int>(fg_ids.size()));
	std::vector<std::vector<int>> p_witness_seen_by_fg(p_witness_ids.size(), std::vector<int>(fg_ids.size()));
	for (int j = 0; j < p_witness_ids.size(); ++j) {
		if (pointguards[p_witness_ids[j]].removed)
			continue; //skip this point, we don't need to see it
		results.push_back(p.push([j, this, &f_witness_ids, &p_witness_ids, &pg_ids, &fg_ids, &f_witness_seen_by_fg, &p_witness_seen_by_fg](int)
		{
			

			for (int i = 0; i < fg_ids.size();i++) {
				//this guard can't see
				if (faceguards[fg_ids[i]].removed)
					continue;
				//if a guard sees the witness, we must add to the expression, or else we must split the 
				if (faceguards[fg_ids[i]].sees_witness(all_vertices[p_witness_ids[j]], version, sm, pointguards, faceguards))
					p_witness_seen_by_fg[j][i] = 1;
			}
		}));		
	}

	for (int i = 0; i < results.size();i++)
		results[i].get();

	std::vector<std::future<void>> results1;
	for (int j = 0; j < f_witness_ids.size(); ++j) {
		//FACE WITNESSES
		//we only care about critical guards!
		if (!faceguards[f_witness_ids[j]].removed ) {
			results1.push_back(p.push([j, this, &f_witness_ids, &p_witness_ids, &pg_ids, &fg_ids, &f_witness_seen_by_fg, &p_witness_seen_by_fg](int)
			{

				//for each half_edge check if the target is inside the polygon
				std::vector<Vertex_handle> pts = {};
				Arrangement_2::Ccb_halfedge_circulator curr_h = all_faces[f_witness_ids[j]]->outer_ccb();
				pts.push_back(curr_h->source());
				while (++curr_h != all_faces[f_witness_ids[j]]->outer_ccb()) {
					pts.push_back(curr_h->source());
				}

				for (int i = 0; i < fg_ids.size();i++) {
					if (faceguards[fg_ids[i]].removed)
						continue; //skip this face, it cant see
					//if a guard sees the witness, we must add to the expression
					if (faceguards[fg_ids[i]].sees_witness(pts, version, sm, pointguards, faceguards))
						f_witness_seen_by_fg[j][i] = 1;
				}
			}));
		}		
	}

	//make expr while waiting for threads to finish :)
	for (int j = 0; j < p_witness_ids.size(); j++) {
		if (pointguards[p_witness_ids[j]].removed)
			continue; //skip this point, we don't need to see it
		for (int i = 0; i < pg_ids.size();i++) {
			if (pointguards[pg_ids[i]].removed)
				continue; //this point can't see anything
			if (p_witness_seen_by_pg[j][i] == 1)
				pointguards[p_witness_ids[j]].expr += pointguard_var[pg_ids[i]];
		}
		for (int i = 0; i < fg_ids.size();i++) {
			if (faceguards[fg_ids[i]].removed)
				continue; //skip this face, it cant see
			if (p_witness_seen_by_fg[j][i] == 1)
				pointguards[p_witness_ids[j]].expr += faceguard_var[fg_ids[i]];
		}

		//at least one must see it
		try {
			model.remove(pointguards[p_witness_ids[j]].c);
		}
		catch (...)
		{
		}
		pointguards[p_witness_ids[j]].c = IloConstraint(pointguards[p_witness_ids[j]].expr >= 1);
		model.add(pointguards[p_witness_ids[j]].c);
	}

	//wait for threads to finish
	for (int i = 0; i < results1.size();i++)
		results1[i].get();	   	 		

	for (int j = 0; j < f_witness_ids.size(); j++) {
		if (faceguards[f_witness_ids[j]].removed)
			continue; //skip this face, we don't need to see it
		for (int i = 0; i < pg_ids.size();i++) {
			if (pointguards[pg_ids[i]].removed)
				continue; //this point can't see anything
			if (f_witness_seen_by_pg[j][i] == 1)
				faceguards[f_witness_ids[j]].expr += pointguard_var[pg_ids[i]];
		}
		for (int i = 0; i < fg_ids.size();i++) {
			if (faceguards[fg_ids[i]].removed)
				continue; //skip this face it cant see
			if (f_witness_seen_by_fg[j][i] == 1)
				faceguards[f_witness_ids[j]].expr += faceguard_var[fg_ids[i]];
		}
		try {
			model.remove(faceguards[f_witness_ids[j]].c);
		}
		catch (...) {
			//int crisb = 15;
		}
		//we cannot relax if its too small!
	/*	if (!faceguards[f_witness_ids[j]].too_small)*/
		faceguards[f_witness_ids[j]].expr += facewitness_var[f_witness_ids[j]];

		faceguards[f_witness_ids[j]].c = IloConstraint(faceguards[f_witness_ids[j]].expr >= 1);
		model.add(faceguards[f_witness_ids[j]].c);
	}	
}


void Art_Gallery::ILP_minimize() {
	IloExpr v = (env);
	double weight_d = 1.0 / (all_vertices_size + all_faces_size + 1);
	IloNum weight_fw = weight_d;

	double addition = 1;
	if (splits % 2 == 0)
		addition = 1 + 0.9 * weight_d;

	IloNum weight_fg = weight_d + addition;


	for (int i = 0; i < pointguard_var.getSize(); i++)
		v += pointguard_var[i];

	for (int i = 0; i < faceguard_var.getSize(); i++) {
		v += faceguard_var[i] * weight_fg;
		v += facewitness_var[i] * weight_fw;
	}



	IloExtractable ex = model.add(IloMinimize(env, v));

	point_vals = IloIntArray(env);
	fg_vals = IloIntArray(env);
	fw_vals = IloIntArray(env);

	IloCplex cplex(model);
	cplex.solve();

	// Your code here
	IloNum sum = cplex.getObjValue();

	cplex.getIntValues(pointguard_var, point_vals);
	cplex.getIntValues(faceguard_var, fg_vals);
	cplex.getIntValues(facewitness_var, fw_vals);
	
	//copy vals before ending them
	face_vals_c = {};
	fg_vals_c = {};
	point_vals_c = {};

	for (int i = 0; i < point_vals.getSize(); i++) {
		if (point_vals[i] == 1) 
			point_vals_c.push_back(i);		
	}
	for (int i = 0; i < fg_vals.getSize(); i++) {
		if (fg_vals[i] == 1) {
			face_vals_c.push_back(i);
			fg_vals_c.push_back(i);
		}
	}

	for (int i = 0; i < fw_vals.getSize(); i++) {
		if (fw_vals[i] == 1) 
			face_vals_c.push_back(i);		
	}
	
	print_time();
	cplex.end();
	model.remove(ex);
	ex.end();
	v.end();
	fg_vals.end();
	fw_vals.end();
	point_vals.end();

	if (face_vals_c.size() > 0) {
		bool any_splittable = false;
		for (int i = 0; i < face_vals_c.size(); i++) {
			if (!faceguards[face_vals_c[i]].too_small)
				any_splittable = true;
		}

		if (!any_splittable) {
			int solsize = fg_vals_c.size() + point_vals_c.size();

			//big IP time
			for (int i = 0; i < faceguards.size(); i++) 
				if (faceguards[i].is_critical)
					model.remove(faceguards[i].c); //we must remove face witness constraint

			IloExpr v_new(env);
			IloExpr v1(env);

			for (int i = 0; i < pointguard_var.getSize(); i++)
				v1 += pointguard_var[i];

			for (int i = 0; i < faceguard_var.getSize(); i++) {
				v1 += faceguard_var[i];
				v_new += faceguard_var[i];
				if(faceguards[i].is_critical)
					v_new += facewitness_var_big[i];				
			}
			std::vector<IloConstraint> new_cs = {};
			
			for (int i = 0; i < faceguards.size(); i++) {
				if (faceguards[i].is_critical) {
					std::vector<Vertex_handle> pts = {};
					Arrangement_2::Ccb_halfedge_circulator curr_h = all_faces[i]->outer_ccb();
					pts.push_back(curr_h->source());
					while (++curr_h != all_faces[i]->outer_ccb()) {
						pts.push_back(curr_h->source());
					}
					IloExpr v2(env);
					IloExpr v3(env);

					for (int j = 0; j < pointguards.size(); j++) {
						if (pointguards[j].sees_witness(pts, version, sm, pointguards, faceguards)) {
							v2 += pointguard_var[j];
						}
					}
					for (int j = 0; j < faceguards.size(); j++) {
						if (faceguards[j].sees_witness(pts, version, sm, pointguards, faceguards)) {
							v2 += faceguard_var[j];
						}
					}
					v3 += facewitness_var_big[i];

					IloConstraint temp((1/weight_fw) - v2 >=  v3 * (1 / weight_fw));
					new_cs.push_back(temp);
				}
			}


			IloConstraint c(v1 == solsize); //the solution must be the same size
			model.add(c);

			for(int i = 0; i<new_cs.size();i++)
				model.add(new_cs[i]);
			IloExtractable ex = model.add(IloMaximize(env, v_new)); //we want to maximize splittable faces
			
			point_vals = IloIntArray(env);
			fg_vals = IloIntArray(env);
			fw_vals = IloIntArray(env);

			IloCplex cplex2(model);
			cplex2.solve();

			IloNum sum = cplex2.getObjValue();
			//copy vals before ending them
			face_vals_c = {};
			fg_vals_c = {};
			point_vals_c = {};

			cplex2.getIntValues(pointguard_var, point_vals);
			for (int i = 0; i < point_vals.getSize(); i++) {
				if (point_vals[i] == 1)
					point_vals_c.push_back(i);
			}

			cplex2.getIntValues(faceguard_var, fg_vals);
			for (int i = 0; i < fg_vals.getSize(); i++) {
				if (fg_vals[i] == 1) {
					face_vals_c.push_back(i);
					fg_vals_c.push_back(i);
				}
			}

			cplex2.getIntValues(facewitness_var_big, fw_vals);
			for (int i = 0; i < fw_vals.getSize(); i++) {
				if (fw_vals[i] == 1)
					face_vals_c.push_back(i);
			}

			OutputDebugString(std::to_string(face_vals_c.size()).c_str());


			cplex2.end();
			model.remove(c);
			for (int i = 0; i < new_cs.size(); i++)
				model.remove(new_cs[i]);
			model.remove(ex);
			new_cs.end();
			ex.end();
			v.end();
			for (int i = 0; i < faceguards.size(); i++)
				if (faceguards[i].is_critical && !faceguards[i].removed)
					model.add(faceguards[i].c); //we must re-add face witness constraint

			bool any_splittable = false;
			for (int i = 0; i < face_vals_c.size(); i++) {
				if (!faceguards[face_vals_c[i]].too_small)
					any_splittable = true;
			}

			if (!any_splittable) {
				//frick we are still unsplittable: double rays!
				//make all faces splittable again
				for (int i = 0; i < faceguards.size(); i++) 
					faceguards[i].too_small = false;				
				
				rays_used = rays_used * 2;		

				//re-run ourself
				ILP_minimize(); //zabumba
			}
		}
	}	

}

//
//void Art_Gallery::ILP_minimize() {
//	for (int i = 0; i < faceguards.size(); i++) 
//		if(faceguards[i].is_critical)
//			model.remove(faceguards[i].c); //we must remove face witness constraint
//
//	IloExpr v = (env);
//	for (int i = 0; i < pointguard_var.getSize(); i++)
//		v += pointguard_var[i];
//
//	for (int i = 0; i < faceguard_var.getSize(); i++) {
//
//		v += faceguard_var[i];
//		v += facewitness_var[i];
//
//	}
//
//	IloExtractable ex = model.add(IloMinimize(env, v));
//
//
//	IloCplex cplex(model);
//	cplex.solve();
//
//	IloNum sum = cplex.getObjValue();
//
//	cplex.end();
//	ex.end();
//	v.end();
//
//	for (int i = 0; i < faceguards.size(); i++)
//		if (faceguards[i].is_critical)
//			model.add(faceguards[i].c); //we must re-add face witness constraint
//	
//
//	v = IloExpr(env);
//	IloExpr v1(env);
//	IloExpr v2(env);
//
//
//	for (int i = 0; i < pointguard_var.getSize(); i++)
//		v += pointguard_var[i];
//
//	for (int i = 0; i < faceguard_var.getSize(); i++) {
//		v += faceguard_var[i];
//		v1 += faceguard_var[i];
//		v1 += facewitness_var[i];
//	}
//
//	
//	ex = model.add(IloConstraint(v == sum)); //must be same size as previous
//	IloExtractable ex1 = model.add(IloMinimize(env, v1)); //minimize faceguards and facewitnesses
//
//	cplex = IloCplex(model);
//	cplex.solve();
//	try {
//		cplex.getIntValues(pointguard_var, point_vals);
//		cplex.getIntValues(faceguard_var, fg_vals);
//		cplex.getIntValues(facewitness_var, fw_vals);
//	}
//	catch (...) {
//		//CPLEX Exceptions are useless..
//	}
//
//	std::vector<Point_2> sol_pg = {};
//	std::vector<std::vector<Segment_2>> sol_fg = {};
//	std::vector<std::vector<Segment_2>> sol_fw = {};
//
//	for (int i = 0; i < point_vals.getSize(); i++) {
//		if (point_vals[i] == 1) {
//			sol_pg.push_back(pointguards[i].v->point());
//			
//		}
//	}
//	for (int i = 0; i < fg_vals.getSize(); i++) {
//		if (fg_vals[i] == 1) {
//			sol_fg.push_back(faceguards[i].sgms2);		
//		}
//	}
//
//	for (int i = 0; i < fw_vals.getSize(); i++) {
//		if (fw_vals[i] == 1) {
//			sol_fw.push_back(faceguards[i].sgms2);
//			
//		}
//	}
//
//	result_pg.push_back(sol_pg);
//	result_fg.push_back(sol_fg);
//	result_fw.push_back(sol_fw);
//	if (use_result_arr)
//		result_arr.push_back(arrs);
//
//	cplex.end();
//
//	ex.end();
//	ex1.end();
//	v.end();
//	v1.end();
//}



void Art_Gallery::critical_witnesses(TEV* tev) {
	//print_time(); 
	bool can_see_all = false;
	srand((unsigned int)time(0));
	std::random_shuffle(point_ids.begin(), point_ids.end());
	srand((unsigned int)time(0));
	std::random_shuffle(face_ids.begin(), face_ids.end());

	
	int counter = 0;
	while (!can_see_all) {
		counter++;

		//Run with old critical witnesses
		ILP_minimize();

		//split ASAP
		/*if ((fg_vals_c.size() * 3 + (counter - 1) >= point_vals_c.size()) && face_vals_c.size() > 0)
			break;*/
	/*	if (fg_vals_c.size() > 0)
			break;*/

		//verify solution...
		std::vector<int> unseen_faces = {};
		std::vector<int> unseen_vertices = {};

		//face witnesses
		for (int k = 0; k < face_ids.size();k++) {		
			int j = face_ids[k];

			if (faceguards[j].is_critical)
				continue; //skip critical witnesses

			if (faceguards[j].removed)
				continue;

			std::vector<Vertex_handle> pts = {};
			Arrangement_2::Ccb_halfedge_circulator curr_h = all_faces[j]->outer_ccb();
			pts.push_back(curr_h->source());
			while (++curr_h != all_faces[j]->outer_ccb()) {
				pts.push_back(curr_h->source());
			}

			bool seen = false;
			//can we see it?
			for (int i = 0; i < point_vals_c.size(); i++) {
				if (pointguards[point_vals_c[i]].sees_witness(pts, version, sm, pointguards, faceguards)) {
					seen = true;
					break;
				}
			}
			if (!seen) {
				for (int i = 0; i < fg_vals_c.size(); i++) {
					if (faceguards[fg_vals_c[i]].sees_witness(pts, version, sm, pointguards, faceguards)) {
						seen = true;
						break;
					}
				}
			}

			if (!seen)
				unseen_faces.push_back((int)k);
		}

		//point witnesses
		for (int k = 0; k < point_ids.size();k++) {
			

			int j = point_ids[k];
			if (pointguards[j].is_critical || pointguards[j].removed)
				continue; //skip critical witnesses

			bool seen = false;
			//can we see it?
			for (int i = 0; i < point_vals_c.size(); i++) {
				if (pointguards[point_vals_c[i]].sees_witness(all_vertices[j], version, sm, pointguards, faceguards)) {
					seen = true;
					break;
				}
			}
			if (!seen) {
				for (int i = 0; i < fg_vals_c.size(); i++) {
					if (faceguards[fg_vals_c[i]].sees_witness(all_vertices[j], version, sm, pointguards, faceguards)) {
						seen = true;
						break;
					}
				}
			}

			if (!seen)
				unseen_vertices.push_back((int)k);
		}

		can_see_all = (unseen_faces.size() + unseen_vertices.size()) == 0;

		if (!can_see_all) {
			std::vector<int> new_pw_ids;
			std::vector<int> new_fw_ids;
			std::vector<int> new_pg_ids;
			
			if (!use_critical) {
				can_see_all = true;
				continue;
			}


			//add 10% of TOTAL back to critical points
			int i = 0;
			int cores = max(8, std::thread::hardware_concurrency());
			int min_age = splits + 1;
			std::vector<int> vertex_ids_from_face;
			if (unseen_faces.size() >  0) {
				//add a face
				srand((unsigned int)time(0));
				std::random_shuffle(unseen_faces.begin(), unseen_faces.end());
				/*for (int i = 0; i < unseen_faces.size();i++) {
					if (faceguards[face_ids[unseen_faces[i]]].age == 0) {
						min_age = 0;
						min_w = unseen_faces[i];
						break;
					}
					if (faceguards[unseen_faces[i]].age <= min_age) {
						min_age = faceguards[face_ids[unseen_faces[i]]].age;
						min_w = unseen_faces[i];
					}
				}*/
				std::vector<int> to_delete = {};
				for (int j = 0; j < min(unseen_faces.size(), cores); j++) {
					//add min_w
					int min_w = unseen_faces[j];
					to_delete.push_back(min_w);
					critical_faces.push_back(face_ids[min_w]);
					
					for (int l = 0; l < faceguards[face_ids[min_w]].sgms.size(); l++) {
						if(!pointguards[faceguards[face_ids[min_w]].sgms[l]->source()->data().id].is_critical)
							vertex_ids_from_face.push_back(faceguards[face_ids[min_w]].sgms[l]->source()->data().id);

					}
					new_fw_ids.push_back(face_ids[min_w]);	

				}
				std::sort(to_delete.begin(), to_delete.end(), std::greater<int>());
				for(int j = 0; j<to_delete.size();j++)
					face_ids.erase(face_ids.begin() + to_delete[j]);


			}
			if(unseen_vertices.size() > 0){
				//add a vertex
				srand((unsigned int)time(0));
				std::random_shuffle(unseen_vertices.begin(), unseen_vertices.end());


				/*for (int i = 0; i < unseen_vertices.size();i++) {
					if (pointguards[point_ids[unseen_vertices[i]]].age == 0) {
						min_age = 0;
						min_w = unseen_vertices[i];
						break;
					}
					if (pointguards[unseen_vertices[i]].age <= min_age) {
						min_age = pointguards[point_ids[unseen_vertices[i]]].age;
						min_w = unseen_vertices[i];
					}
				}*/
				std::vector<int> to_delete = {};
				

				//make sure we add all face vertices to critical points too
				for (int l = 0; l < vertex_ids_from_face.size(); l++) {
					//to_delete.push_back(vertex_ids_from_face[l]);
					critical_points.push_back(vertex_ids_from_face[l]);
					new_pw_ids.push_back(vertex_ids_from_face[l]);

					for (int j = unseen_vertices.size() - 1; j >= 0; j--) {
						if(point_ids[unseen_vertices[j]] == vertex_ids_from_face[l])
							unseen_vertices.erase(unseen_vertices.begin() + j);
					}
				}

				//remove_intersection(point_ids, vertex_ids_from_face);


				//add min_w
				for (int j = 0; j < min(unseen_vertices.size(), cores); j++) {
					int min_w = unseen_vertices[j];
					//to_delete.push_back(min_w);
					critical_points.push_back(point_ids[min_w]);
					new_pw_ids.push_back(point_ids[min_w]);
				}
				//std::sort(to_delete.begin(), to_delete.end(), std::greater<int>());

				//for (int j = 0; j < to_delete.size();j++)
					//point_ids.erase(point_ids.begin() + to_delete[j]);
			}
			
			

			std::vector<int> pgs_ids(all_vertices.size());
			std::vector<int> fgs_ids(all_faces.size());
			std::iota(std::begin(pgs_ids), std::end(pgs_ids), 0); //0 is the starting number
			std::iota(std::begin(fgs_ids), std::end(fgs_ids), 0); //0 is the starting number

			for (int i = 0; i < new_pw_ids.size(); i++)
				pointguards[new_pw_ids[i]].is_critical = true;
			for (int i = 0; i < new_fw_ids.size(); i++) {
				faceguards[new_fw_ids[i]].is_critical = true;

				//a new critical_witness: we must add interior point too
				insert_point_into_int(faceguards[new_fw_ids[i]], tev);

				new_pg_ids.push_back(all_vertices_size - 1);
				new_pw_ids.push_back(all_vertices_size - 1);
			}

			//make sure that thew new witnesses are seen by all (possible) guards
			witness_visibilities(new_pw_ids, new_fw_ids, pgs_ids, fgs_ids);

			//make sure that all critical witnesses are seen by new interior pgs
			witness_visibilities(critical_points, critical_faces, new_pg_ids, {});

		}
	}


	//if (!unsplittable_faces_used)
	//	previous_pure_sol_size = (int)fg_vals_c.size() + (int)point_vals_c.size();
	//else {
	//	int currentsize = fg_vals_c.size() + point_vals_c.size();

	//	if ((int) currentsize > previous_pure_sol_size)
	//	{
	//		//houston, we have a problem!
	//		unsplittable_faces_used = false;
	//		std::vector<int> witness_ids;
	//		std::vector<int> fg_ids;

	//		//make all faces splittable again
	//		for (int i = 0; i < faceguards.size();i++) {
	//			faceguards[i].split_times += 0.1;


	//			if (faceguards[i].too_small) {
	//				faceguards[i].too_small = false;
	//				model.remove(faceguards[i].unsplittable_c);
	//				model.remove(faceguards[i].unsplittable_w);

	//				fg_ids.push_back(i);

	//				if (faceguards[i].is_critical)
	//					witness_ids.push_back(i);
	//			}
	//		}
	//		rays_used = rays_used * 2;

	//		//make sure that all guards see new face witnesses
	//		witness_visibilities({}, witness_ids, point_ids, face_ids);

	//		//make sure that re-added guards see all witnesses
	//		witness_visibilities(critical_points, critical_faces, {}, fg_ids);
	//		
	

	//		//re-run critical witnesses
	//		critical_witnesses(tev);

	//		return; //don't continue this solution
	//	}

	//}


	//save & print results


	std::vector<Point_2> sol_pg = {};
	std::vector<std::vector<Segment_2>> sol_fg = {};
	std::vector<std::vector<Segment_2>> sol_fw = {};

	for (int i = 0; i < fg_vals_c.size(); i++) 
		sol_fg.push_back(faceguards[fg_vals_c[i]].sgms2);
	
	for (int i = 0; i < point_vals_c.size(); i++)
		sol_pg.push_back(pointguards[point_vals_c[i]].v->point());

	for (int i = fg_vals_c.size(); i < face_vals_c.size(); i++)
		sol_fw.push_back(faceguards[face_vals_c[i]].sgms2);

	result_pg.push_back(sol_pg);
	result_fg.push_back(sol_fg);
	result_fw.push_back(sol_fw);
	if (use_result_arr)
		result_arr.push_back(arrs);


	auto now = std::chrono::high_resolution_clock::now();
	std::chrono::duration<double, std::milli> elapsedTime(now - prev_time);
	time_taken.push_back(elapsedTime.count() / 1000);
	current_time_taken += time_taken.back();
	crit_cycles.push_back(counter);
	crit_vert.push_back((int)critical_points.size());
	crit_fac.push_back((int)critical_faces.size());

	result_fg_i.push_back(fg_vals_c);
	result_fw_i.push_back(face_vals_c);
	result_pg_i.push_back(point_vals_c);

	critical_p_i.push_back(critical_points);
	critical_f_i.push_back(critical_faces);
	faces_count.push_back(all_faces_size);
	vertices_count.push_back(all_vertices_size);
	rays_used_count.push_back(rays_used);


	std::vector<Point_2> criticalp = {};
	for (int id : critical_points) {
		if (!pointguards[id].removed)
			criticalp.push_back(all_vertices[id]->point());
	}
	std::vector<std::vector<Segment_2>> criticalf = {};
	for (int id : critical_faces)
	{
		if(!faceguards[id].removed)
			criticalf.push_back(faceguards[id].sgms2);
	}
	result_cp.push_back(criticalp);
	result_cf.push_back(criticalf);


	//compute hausdorff distance if necessary
	if (strcmp(current_p, p5_str) == 0) {
		std::vector<Point_2> curr_solution = {};
		for (int i = 0; i < point_vals_c.size(); i++)
			curr_solution.push_back(all_vertices[point_vals_c[i]]->point());

		for (int i = 0; i < fg_vals_c.size(); i++)
			for (int j = 0; j < faceguards[fg_vals_c[i]].sgms2.size();j++)
				curr_solution.push_back(faceguards[fg_vals_c[i]].sgms2[j].source());

		hausdorff_dist.push_back(Hausdorff(p5_solution, curr_solution));
	}
	else
		hausdorff_dist.push_back(0);
	
	print_time();
	prev_time = std::chrono::high_resolution_clock::now();	
}


Art_Gallery::Art_Gallery(std::vector<Point_2> poly, int version, LPCSTR curr_p, bool use_result_arr, double time_limit, bool use_critical, bool use_big_ip)
{
	this->use_result_arr = use_result_arr;
	this->use_critical = use_critical;
	this->use_big_ip = use_big_ip;

	current_p = curr_p;
	this->version = version;
	if (poly.size() == 0)
		return;

	poly_arr = polygon_to_arrangement(poly);

	if (version == 2) {
		//result_arr.push_back({ poly_arr });
		result_pg.push_back({});
		result_fg.push_back({});
		result_fw.push_back({});
		result_cf.push_back({});
		result_cp.push_back({});

		return;
	}

	TEV tev(poly_arr);
	

	if (version == 1) {
		sm = Shortest_Map(poly);
		/*result_arr.push_back({ poly_arr });
		result_pg.push_back({});
		result_fg.push_back({});
		result_fw.push_back({});
		result_cf.push_back({});
		result_cp.push_back({});*/
		//return;
	}


	//create initial arrangements
	for (int i = 0; i < sm.decomp.size(); i++) {
		arrs.push_back(orthogonal_creation(sm.decomp[i].poly));
	}

	//Create candidates
	pointguards = {};
	faceguards = {};

	model = IloModel(env);
	faceguard_var = IloBoolVarArray(env);
	facewitness_var = IloBoolVarArray(env);
	facewitness_var_big = IloBoolVarArray(env);

	pointguard_var = IloBoolVarArray(env);
	point_vals = IloIntArray(env);
	fg_vals = IloIntArray(env);
	fw_vals = IloIntArray(env);

	int n = poly.size();
	all_vertices = {};
	all_faces = {};

	for (int i = 0; i < arrs.size(); i++) {
		if (arrs[i].number_of_vertices() > max_decomp_v)
			max_decomp_v = (int)arrs[i].number_of_vertices();

		int cur_reflexes = 0;
		Arrangement_2::Vertex_handle curr = arrs[i].vertices_begin();
		//Add pointguards
		while (curr != arrs[i].vertices_end()) {
			//compute visibility polygon of point

			OutputDebugString(".");
			Vertex_info inf = curr->data();
			inf.id = all_vertices_size;
			if (version == 1) {
				inf = sm.decomp[i].get_location(inf, curr->point());
				inf.decomp_index = (int)i;
			}


			for (int j = 0; j < poly.size(); j++) {

				int next = (j + 1) % poly.size();
				int prev = j - 1;
				if (prev == -1)
					prev = poly.size() - 1;

				if (same(poly[j], curr->point())) {
					//we are in the same thing

					CGAL::Orientation or = CGAL::orientation(poly[prev], poly[j], poly[next]);
					if (or == CGAL::RIGHT_TURN) {
						inf.is_reflex = true;
						inf.poly_id = (int)j;
						inf.prev_id = (int)prev;
						inf.next_id = (int)next;
						reflexes.push_back(curr);
						cur_reflexes++;

					}
					break;
				}

			}



			curr->set_data(inf);
			pointguards.push_back(PointGuard(curr, version, poly_arr, &tev));
			pointguards.back().expr = IloExpr(env);
			pointguard_var.add(IloBoolVar(env, 0, 1));
			point_ids.push_back(inf.id);
			all_vertices.push_back(curr);
			all_vertices_size++;
			curr++;
		}
		OutputDebugString("\n");
		if (cur_reflexes > max_decomp_rv)
			max_decomp_rv = cur_reflexes;

		//add face guards and face witnesses
		Face_handle curr_f = arrs[i].faces_begin();
		while (curr_f != arrs[i].faces_end()) {
			if (!curr_f->has_outer_ccb()) {
				curr_f++;
				continue;
			}

			Face_info inf = curr_f->data();
			inf.id = all_faces_size;
			inf.decomp_index = (int)i;

			curr_f->set_data(inf);
			//compute visibility polygon of point
			faceguards.push_back(FaceGuard(curr_f, poly, version));
			faceguards.back().expr = IloExpr(env);

			faceguard_var.add(IloBoolVar(env, 0, 1));
			facewitness_var.add(IloBoolVar(env, 0, 1));
			facewitness_var_big.add(IloBoolVar(env, 0, 1));

			OutputDebugString("+");
			face_ids.push_back(all_faces_size);
			all_faces.push_back(curr_f);
			all_faces_size++;

			curr_f++;
		}


	}
	OutputDebugString("\n");

	OutputDebugString("Iteration 0\n");

	for (int i = 0; i < arrs.size(); i++) {
		//take 10% of each arr
		Arrangement_2::Vertex_handle curr = arrs[i].vertices_begin();
		std::vector<int> cur_point_ids = {};
		std::vector<int> cur_face_ids = {};

		//Add pointguards
		while (curr != arrs[i].vertices_end()) {
			cur_point_ids.push_back(curr->data().id);
			curr++;
		}

		Face_handle curr_f = arrs[i].faces_begin();
		while (curr_f != arrs[i].faces_end()) {
			if (!curr_f->has_outer_ccb()) {
				curr_f++;
				continue;
			}

			cur_face_ids.push_back(curr_f->data().id);
			curr_f++;
		}

		if (use_critical) {
			//random shuffle ids
			srand((unsigned int)time(0));
			std::random_shuffle(cur_point_ids.begin(), cur_point_ids.end());
			srand((unsigned int)time(0));
			std::random_shuffle(cur_face_ids.begin(), cur_face_ids.end());

			critical_points.insert(critical_points.end(), cur_point_ids.begin(), cur_point_ids.begin() + DIV_ROUND_CLOSEST(cur_point_ids.size(), 10));
			critical_faces.insert(critical_faces.end(), cur_face_ids.begin(), cur_face_ids.begin() + DIV_ROUND_CLOSEST(cur_face_ids.size(), 10));
		}
		else{
			critical_points.insert(critical_points.end(), cur_point_ids.begin(), cur_point_ids.end());
			critical_faces.insert(critical_faces.end(), cur_face_ids.begin(), cur_face_ids.end()); //everyone is critical
		}
		
	}


	for (int i = 0; i < reflexes.size(); i++) {
		//reflexes[i]->data().rays = shoot_reflex_rays(reflexes[i]->point(), 5);
		for (int j = i + 1; j < reflexes.size(); j++) {
			if (pointguards[reflexes[i]->data().id].sees_witness(all_vertices[reflexes[j]->data().id], version, sm, pointguards, faceguards))
			{
				reflex_chords_0.push_back(reflexes[i]);
				reflex_chords_1.push_back(reflexes[j]);
				reflex_chords.push_back(Segment_2(reflexes[i]->point(), reflexes[j]->point()));
			}
		}

	}
	std::vector<int> pgs_ids(all_vertices.size());
	std::vector<int> fgs_ids(all_faces.size());
	std::iota(std::begin(pgs_ids), std::end(pgs_ids), 0); //0 is the starting number
	std::iota(std::begin(fgs_ids), std::end(fgs_ids), 0); //0 is the starting number


	for (int j = 0; j < critical_points.size(); j++)
		pointguards[critical_points[j]].is_critical = true;
	for (int j = 0; j < critical_faces.size(); j++) {
		faceguards[critical_faces[j]].is_critical = true;
		insert_point_into_int(faceguards[critical_faces[j]], &tev);
	}



	//remove_intersection(point_ids, critical_points);
	//remove_intersection(face_ids, critical_faces);

	//add constraints
	witness_visibilities(critical_points, critical_faces, pgs_ids, fgs_ids);


	splits = 0;
	prev_time = std::chrono::high_resolution_clock::now();
	critical_witnesses(&tev); //run ILP and critical witnesses
	splits++;


	while (current_time_taken < time_limit && splits < 22	 && face_vals_c.size() > 0) {
		for (int i = 0; i < all_vertices_size; i++)
			pointguards[i].age++;
		for (int i = 0; i < all_faces_size; i++)
			faceguards[i].age++;

		std::vector<int> new_pg_ids = {};
		std::vector<int> new_fg_ids = {};

		std::vector<int> new_pw_ids = {};
		std::vector<int> new_fw_ids = {};

		unspl = 0;

		skips = 0;
		chord_splits = 0;
		ray_splits = 0;
		normal_splits = 0;
		extension_splits = 0;
	
		//split faces
		for (int j = 0; j < face_vals_c.size();j++) {
			int i = face_vals_c[j];
			int vertices_start = all_vertices_size;
			bool issaguard = false;
			if (std::find(fg_vals_c.begin(), fg_vals_c.end(), i) != fg_vals_c.end()) {
				issaguard = true;
			}
			if (use_big_ip && faceguards[i].too_small)
				continue; //unsplittable, try other faces

			std::vector<Vertex_handle> pts = {};
			Arrangement_2::Ccb_halfedge_circulator curr_h = all_faces[i]->outer_ccb();
			pts.push_back(curr_h->source());

			while (++curr_h != all_faces[i]->outer_ccb()) {
				pts.push_back(curr_h->source());
			}
						
			int reflex_count = 0;
			for (int k = 0; k < pts.size(); k++) {
				if (pts[k]->data().is_reflex)
					reflex_count++;
			}


			int num_new_faces;
			bool f = false;
			bool can_do_chord = true;
			bool can_do_extend = true;
			bool can_do_ray = true;
			bool can_do_normal = reflex_count > 1 || strcmp(current_p, p5_str) == 0;
			//bool can_do_normal = reflex_count > 1 || (reflex_count > 0 && issaguard);

			int prev_num_faces = arrs[faceguards[i].f->data().decomp_index].number_of_faces();

			srand((unsigned int)time(0));
			int dice = rand() % 10 + 1;
			
			while (!f && (can_do_chord || can_do_extend || can_do_ray || can_do_normal)) {
				num_new_faces = 2; //we're going to split the face in half
				//j is the i-th face guard

				
				if (can_do_normal) {

					faceguards[i].last_split_ss = true;

					//do orthogonal split
					normal_splits++;

					
					std::vector<Segment_2> split_segs = split_face(all_faces[i], split_num, 0);
					insert(arrs[all_faces[i]->data().decomp_index], split_segs.begin(), split_segs.end());
					if (split_segs.size() > 2)
						num_new_faces = 4;
					else
						num_new_faces = 2;			
					
					f = true;
					break;
				}
				else
					can_do_normal = false;
				if ((dice == 2 || !can_do_ray) && can_do_extend) {
					for (int k = 0; k < reflexes.size(); k++) {
						for (int l = 0; l < pts.size(); l++) {
							if (pointguards[reflexes[k]->data().id].sees_witness(pts[l], version, sm, pointguards, faceguards)) {
								int dec = reflexes[k]->data().decomp_index;
								int prev = reflexes[k]->data().prev_id;
								int next = reflexes[k]->data().next_id;

								//try to extend
								Ray_2 extension1(poly[prev], reflexes[k]->point());
								Ray_2 extension2(poly[next], reflexes[k]->point());

								//	faceguards[i].sgms[k]->source()->point());
								//Segment_2 extension = faceguards[i].sgms2[k];
								Segment_2 res;
								if (proper_intersect_face(faceguards[i].sgms2, extension1, res)) {
									insert(arrs[all_faces[i]->data().decomp_index], res);
									if (prev_num_faces < arrs[faceguards[i].f->data().decomp_index].number_of_faces())
									{
										f = true;
										extension_splits++;
										break;
									}
								}
								else if (proper_intersect_face(faceguards[i].sgms2, extension2, res)) {
									insert(arrs[all_faces[i]->data().decomp_index], res);
									if (prev_num_faces < arrs[faceguards[i].f->data().decomp_index].number_of_faces())
									{
										f = true;
										extension_splits++;
										break;
									}
								}
							}
						}
						if (f)
							break;

					}
					if (!f)
						can_do_extend = false;
				}
				else if ((dice == 1 || !can_do_ray) && can_do_chord) { //	chord split!
				
					for (int k = 0; k < reflex_chords.size(); k++) {
						if (faceguards[i].sees_witness(reflex_chords_0[k], version, sm, pointguards, faceguards) &&
							faceguards[i].sees_witness(reflex_chords_1[k], version, sm, pointguards, faceguards)
							
							) {

							Ray_2 ray_1(reflex_chords[k].source(), reflex_chords[k].target());
							Ray_2 ray_2(reflex_chords[k].target(), reflex_chords[k].source());

							//do we see both reflexes?
							Segment_2 res;
							if (proper_intersect_face(faceguards[i].sgms2, ray_1, res)) {

								insert(arrs[all_faces[i]->data().decomp_index], res);

								if (prev_num_faces < arrs[faceguards[i].f->data().decomp_index].number_of_faces())
								{
									f = true;
									chord_splits++;
									break;
								}

								
							}
							if (proper_intersect_face(faceguards[i].sgms2, ray_2, res)) {

								insert(arrs[all_faces[i]->data().decomp_index], res);
								if (prev_num_faces < arrs[faceguards[i].f->data().decomp_index].number_of_faces())
								{
									f = true;
									chord_splits++;
									break;
								}
							}
						}
					}
					if (!f) {
						can_do_chord = false;
					}
				}				
				else if(can_do_ray) {
					std::vector<std::tuple<std::vector<Segment_2>, int, int>> angles = {};

					for (int k = 0; k < reflexes.size(); k++) {
						if (faceguards[i].sees_witness(reflexes[k], version, sm, pointguards, faceguards)) {

							if (dice > 7) {
								bool incident = false;
								if (issaguard) {
									for (int l = 0; l < pts.size(); l++) {
										if (pts[l]->data().id == reflexes[k]->data().id)
											incident = true;
									}
								}
								//If we are a guard and we have an incident reflex vertex, don't use that vertex to split (this is not a useful split).
								if (incident) {
									//OutputDebugString(std::string("Ay no:").c_str());
									continue;
								}
							}
							int rays = 16;
							std::vector<Segment_2> inter_segs;
							int max_rays = pow(2, 18);

							if (use_big_ip)
								max_rays = rays_used; //otherwise: unsplittable

							//keep trying until we find intersection
							while (rays <= max_rays) {
								inter_segs = recursive_ray_intersection(rays, 0, rays, faceguards[i].sgms2, reflexes[k]->point());
								if (inter_segs.size() > 2) {
									if (rays < max_rays)
										max_rays = rays;
									angles.push_back(std::make_tuple(inter_segs, k, rays));
									break;
								}
								rays = rays * 2;
							}							
						}
					}

					//lowest to highest
					std::sort(angles.begin(), angles.end(), [](const std::tuple<std::vector<Segment_2>, int, int>& a, const std::tuple<std::vector<Segment_2>, int, int> b)
						{
							if (std::get<2>(a) == std::get<2>(b))
								return std::get<0>(a).size() > std::get<0>(b).size();
							return std::get<2>(a) < std::get<2>(b); //< is ascending > is descending
						});
					
					if (!use_big_ip && angles.size() > 0)
					{
						int max_rays = std::get<2>(angles[0]);;

						rays_used = max(rays_used, max_rays); //save the smallest split!
					}

					for(int k = 0; k < angles.size(); k++){

						std::vector<Segment_2> inter_segs = { };
						inter_segs = std::get<0>(angles[k]);
						//inter_segs = recursive_ray_intersection(rays_used, 0, rays_used, faceguards[i].sgms2, reflexes[angles[k].second]->point());

						int cur_rays = std::get<2>(angles[k]);

						if (inter_segs.empty())
							break;
						else if (inter_segs.size() > 1 && inter_segs.size() % 2 != 0) {
							int mid = inter_segs.size() / 2;
						
							//OutputDebugString(std::string("Reflex iD: " + std::to_string(std::get<1>(angles[k])) + "\n").c_str());


							//shoot mid ray
							insert(arrs[all_faces[i]->data().decomp_index], inter_segs[mid]);
							if (prev_num_faces < arrs[faceguards[i].f->data().decomp_index].number_of_faces())
							{
								f = true;
								break;
							}
						}
						else if (inter_segs.size() % 2 == 0) {
							//go down one level
							inter_segs = recursive_ray_intersection(cur_rays * 2, 0, cur_rays * 2, faceguards[i].sgms2, reflexes[std::get<1>(angles[k])]->point());
							
							//shoot mid ray
							int mid = inter_segs.size() / 2;
							

							insert(arrs[all_faces[i]->data().decomp_index], inter_segs[mid]);
							if (prev_num_faces < arrs[faceguards[i].f->data().decomp_index].number_of_faces())
							{
								f = true;
								break;
							}
						}						
						
					}
					if (!f) {
						can_do_ray = false;
					}
					else
						ray_splits++;
				}
			}

			

			if (f) {
				int count = 0;
				Face_handle curr_f = arrs[all_faces[i]->data().decomp_index].faces_begin();
				while (curr_f != arrs[all_faces[i]->data().decomp_index].faces_end()) {
					int curval = curr_f->data().id;
					if (!curr_f->has_outer_ccb()) {
						curr_f++;
						continue;
					}
					if (curval == -1 || curval == i) {
						//add these new faces
						OutputDebugString("+");

						curr_f->data().id = all_faces_size;
						//copy papa decomp
						curr_f->data().decomp_index = all_faces[i]->data().decomp_index;

						FaceGuard fg = FaceGuard(curr_f, poly, version);
						faceguard_var.add(IloBoolVar(env, 0, 1));
						facewitness_var.add(IloBoolVar(env, 0, 1));
						facewitness_var_big.add(IloBoolVar(env, 0, 1));

						fg.parent = i;

						//fg.first_papa->split_times++;
						if (!use_critical) {
							fg.is_critical = true;
							new_fw_ids.push_back(all_faces_size);
							critical_faces.push_back(all_faces_size);

						}
						fg.split_times = faceguards[i].split_times + 1;
						face_ids.push_back(all_faces_size);
						faceguards.push_back(fg);
						faceguards.back().expr = IloExpr(env);

						all_faces.push_back(curr_f);
						new_fg_ids.push_back(all_faces_size);
						all_faces_size++;
						if (faceguards[i].is_critical) {
							if (curval == i) {
								//mark interior point as removed (!)
								pointguards[faceguards[i].interior_p].removed = true;
								try {
									model.remove(pointguards[faceguards[i].interior_p].c);
								}
								catch (...) {

								}
								IloExpr expr(env);
								//OutputDebugString(std::to_string(faceguards[i].interior_p).c_str());
								expr += pointguard_var[faceguards[i].interior_p];
								IloConstraint c(expr == 0);
								//force it not to see anything
								model.add(c);
							}
							//add new interior point
							insert_point_into_int(faceguards.back(), &tev);
							new_pg_ids.push_back(all_vertices_size - 1);
							new_pw_ids.push_back(all_vertices_size - 1);

						}
					

						count++;
					}
					curr_f++;
				}

				//find new vertices
				Arrangement_2::Vertex_handle curr = arrs[all_faces[i]->data().decomp_index].vertices_begin();
				count = 0;
				//Add pointguards
				while (curr != arrs[all_faces[i]->data().decomp_index].vertices_end()) {
					int curval = curr->data().id;

					if (curval == -1) {
						OutputDebugString(".");

						Vertex_info inf;
						inf.id = all_vertices_size;

						point_ids.push_back(inf.id);
						if (version == 1)
							inf = sm.decomp[all_faces[i]->data().decomp_index].get_location(inf, curr->point());
						inf.decomp_index = all_faces[i]->data().decomp_index;
						curr->set_data(inf);

						PointGuard pg = PointGuard(curr, version, poly_arr, &tev);
						pointguard_var.add(IloBoolVar(env, 0, 1));

						pg.parent = i;
						if (!use_critical) {
							pg.is_critical = true;
							new_pw_ids.push_back(all_vertices_size);
							critical_points.push_back(all_vertices_size);
						}
						pointguards.push_back(pg);
						pointguards.back().expr = IloExpr(env);

						new_pg_ids.push_back(all_vertices_size);

						all_vertices.push_back(curr);
						all_vertices_size++;
						point_ids.push_back(inf.id);
					}
					count++;
					curr++;
				}

				//deactivate papa face guard	
				faceguards[i].removed = true;

				//does not have to be seen anymore
				try {
					model.remove(faceguards[i].c);
				}
				catch (...) {

				}
				IloExpr expr(env);
				expr += faceguard_var[i];
				IloConstraint c(expr == 0);
				
				IloExpr expr2(env);
				expr2 += facewitness_var_big[i];
				IloConstraint c2(expr2 == 0);


				//force it not to see anything
				model.add(c);
				model.add(c2);

			}
			else {
				if (use_big_ip) {
					//OutputDebugString("This should never happen \n");

					//we cannot split (!)
					int crispy = 15;
					faceguards[i].too_small = true;
					faceguards[i].split_times += 0.25;

					////still has to be seen but can't see anything
					//IloExpr expr(env);
					//expr += faceguard_var[i];
					//faceguards[i].unsplittable_c = IloConstraint(expr == 0);
					//model.add(faceguards[i].unsplittable_c);

					//expr = IloExpr(env);
					//expr += facewitness_var[i];
					//faceguards[i].unsplittable_w = IloConstraint(expr == 0);
					//model.add(faceguards[i].unsplittable_w);
					//if (!unsplittable_faces_used)
					//	unsplittable_faces_used = true;

					unspl++;
				
				}
			}
		}

		//std::vector<int> pgs_ids(all_vertices.size());
		//std::vector<int> fgs_ids(all_faces.size());
		//std::iota(std::begin(pgs_ids), std::end(pgs_ids), 0); //0 is the starting number
		//std::iota(std::begin(fgs_ids), std::end(fgs_ids), 0); //0 is the starting number

		//new points and faces have to be seen by all guards! NOT NECESSARY:: ONLY CRITICAL WITNESSES!
		//face_witness_visibilities(new_fg_ids, pgs_ids, fgs_ids);
		//point_witness_visibilities(new_pg_ids, pgs_ids, fgs_ids);


		//new witnesses have to be seen by all candidates
		witness_visibilities(new_pw_ids, new_fw_ids, point_ids, face_ids);

		//new point and faces have to see all critical witnesses!
		witness_visibilities(critical_points, critical_faces, new_pg_ids, new_fg_ids);


		//add constraints for the new witnesses
		OutputDebugString(std::string("Split " + std::to_string(splits+1) + "\n").c_str());
		

		chord_splits_count.push_back(chord_splits); 
		ray_splits_count.push_back(ray_splits);
		normal_splits_count.push_back(normal_splits);
		extension_splits_count.push_back(extension_splits);
		skips_count.push_back(skips);
		unspl_count.push_back(unspl);
		total_skips += skips;
		critical_witnesses(&tev); //run ILP
		splits++;
	}

	OutputDebugString(std::string("Solution size:  " + std::to_string(result_pg_i.back().size()) + "\n").c_str());
	
	faceguard_var.end();
	facewitness_var.end();
	facewitness_var_big.end();

	pointguard_var.end();
	point_vals.end();
	fg_vals.end();
	fw_vals.end();
	model.end();
	env.end();
	chord_splits_count.push_back(0);
	ray_splits_count.push_back(0);
	normal_splits_count.push_back(0);
	extension_splits_count.push_back(0);
	skips_count.push_back(0);
	
	unspl_count.push_back(0);
}


