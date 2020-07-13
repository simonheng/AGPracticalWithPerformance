
#include <point_functions.h>
#include <polygon_functions.h>
#include <shortest_path.h>

void print_time();
class FaceGuard; 

class Guard {
public:
	int id;
	Polygon_2 vis;
	std::vector<Point_2> points;
	std::vector<char> sees_points;
	int parent;
	bool is_critical = false;

	int age = 0;
	bool sees_witness(Vertex_handle &w, int version, Shortest_Map &sm, std::vector<PointGuard> &pg, std::vector<FaceGuard> &fg);
	bool sees_witness(std::vector<Vertex_handle> &pts, int version, Shortest_Map &sm, std::vector<PointGuard> &pg, std::vector<FaceGuard> &fg);
	virtual bool sees_witness_v1(Vertex_handle &w, Shortest_Map &sm, std::vector<PointGuard> &pg, std::vector<FaceGuard> &fg);
	double split_times = 0;
	bool removed = false;

	//Witness variable!
	IloExpr expr;
	IloConstraint c;

};
class PointGuard : public Guard {
public:              // Access specifier
	Vertex_handle v;  // Attribute
	PointGuard(Vertex_handle v, int version, Arrangement_2 &arr, TEV* tev);
	bool sees_witness_v1(Vertex_handle &w, Shortest_Map &sm, std::vector<PointGuard> &pg, std::vector<FaceGuard> &fg);
};

class FaceGuard : public Guard {
public:              // Access specifier
	Face_handle f;  // Attribute
	std::vector<Polygon_2> seg_vis = {};
	std::vector<Halfedge_handle> sgms = {};
	std::vector<Segment_2> sgms2 = {};
	bool too_small = false;
	bool last_split_ss = false; //true if last split was a squaresplit

	int interior_p = -1;

	FaceGuard(Face_handle f, std::vector<Point_2> pgn, int version);
	bool sees_witness_v1(Vertex_handle &w, Shortest_Map &sm, std::vector<PointGuard> &pg, std::vector<FaceGuard> &fg);

	IloConstraint unsplittable_c;
	IloConstraint unsplittable_w;


};



class Art_Gallery {
public:
	LPCSTR current_p;
	LPCSTR p5_str = "p5";

	int version;
	std::vector<PointGuard> pointguards;
	std::vector<FaceGuard> faceguards;
	std::vector<int> critical_points;
	std::vector<int> critical_faces;
	std::vector<int> point_ids;
	std::vector<int> face_ids;
	int rays_used = 16;

//	std::vector<Point_2> p5_solution = {Point_2(10.57,2.12), Point_2(2,0.59), Point_2(19,1.71)};

		
	std::vector<Point_2> p5_solution = {
		Point_2(3.5 + 5* std::sqrt(2),	1.5 * std::sqrt(2)),		
		Point_2(2, 2 - std::sqrt(2)),
		Point_2(19,1 + (std::sqrt(2))/2) };

	int splits;
	std::chrono::time_point<std::chrono::high_resolution_clock> prev_time;

	//Arrangement_2 arr;
	std::vector<Arrangement_2> arrs;
	Shortest_Map sm;
	IloEnv   env;
	IloModel model;

	IloBoolVarArray pointguard_var;
	IloBoolVarArray faceguard_var;
	IloBoolVarArray facewitness_var;
	IloBoolVarArray facewitness_var_big;

	IloIntArray point_vals;
	IloIntArray fg_vals;
	IloIntArray fw_vals;

	std::vector<int> point_vals_c;
	std::vector<int> face_vals_c;
	std::vector<int> fg_vals_c;

	std::vector<std::vector<Point_2>> result_pg;
	std::vector<std::vector<Point_2>> result_cp;

	std::vector< std::vector<int>> result_fg_i;
	std::vector< std::vector<int>> result_pg_i;
	std::vector< std::vector<int>> result_fw_i;

	std::vector< std::vector<int>> critical_p_i;
	std::vector< std::vector<int>> critical_f_i;

	bool unsplittable_faces_used = false;
	int previous_pure_sol_size = -1;

	std::vector< std::vector<std::vector<Segment_2>>> result_fg;
	std::vector< std::vector<std::vector<Segment_2>>> result_cf;
	std::vector< std::vector<std::vector<Segment_2>>> result_fw;
	double current_time_taken = 0;
	std::vector<double> time_taken;
	std::vector<int> crit_cycles;
	std::vector<int> crit_vert;
	std::vector<int> crit_fac;
	std::vector<int> faces_count;
	std::vector<int> vertices_count;
	std::vector<double> hausdorff_dist;

	std::vector<int> chord_splits_count;
	std::vector<int> ray_splits_count;;
	std::vector<int> normal_splits_count;
	std::vector<int> extension_splits_count;
	std::vector<int> unspl_count;

	std::vector<int> skips_count;
	std::vector<int> rays_used_count;
	int max_decomp_v = 0;
	int max_decomp_rv = 0;
	int chord_splits = 0;
	int ray_splits = 0;
	int normal_splits = 0;
	int extension_splits = 0;
	int unspl = 0;
	int skips = 0;
	int total_skips = 0;
	std::vector<std::vector<Arrangement_2>> result_arr;
	int split_num = 1;
	int all_vertices_size = 0;
	int all_faces_size = 0;
	std::vector<Vertex_handle> reflexes;
	std::vector<Vertex_handle> reflex_prevs;

	std::vector<Segment_2> reflex_chords; 
	std::vector<Vertex_handle> reflex_chords_0;
	std::vector<Vertex_handle> reflex_chords_1;

	std::vector<Vertex_handle> all_vertices;
	std::vector<Face_handle> all_faces;
	Arrangement_2 poly_arr;
	void Art_Gallery::critical_witnesses(TEV* tev);
	Art_Gallery(std::vector<Point_2> poly, int version, LPCSTR curr_p, bool use_result_arr = false, double time_limit = 1000, bool use_critical = true, bool use_big_ip = false);
	bool use_result_arr = false;
	bool use_critical = false;
	bool use_big_ip = false;

	Arrangement_2 orthogonal_creation(std::vector<Point_2> poly);
	void Art_Gallery::witness_visibilities(std::vector<int> p_witness_ids, std::vector<int> f_witness_ids, std::vector<int> pg_ids, std::vector<int> fg_ids);
	void Art_Gallery::ILP_minimize();
	void Art_Gallery::insert_point_into_int(FaceGuard& f, TEV*);
};