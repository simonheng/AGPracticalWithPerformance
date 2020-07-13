// HelloWindowsDesktop.cpp
// compile with: /D_UNICODE /DUNICODE /DWIN32 /D_WINDOWS /c
#include <CGAL/Exact_predicates_exact_constructions_kernel.h>
#include <Art_gallery.h>
#include <weak_visibility.h>
#include <thread>         // std::this_thread::sleep_for
#pragma comment (lib,"Gdiplus.lib")
#include <string>
#include <array>
#include <vector>
#include <stdio.h>
#include <objsafe.h>
#include <objbase.h>
#include <atlbase.h>

#include <stdlib.h>
#include <windows.h>
#include <atlimage.h>
//#include <ocidl.h>
//#include <olectl.h>
#include <comdef.h>
#include <tchar.h>
#include <objidl.h>
#include <gdiplus.h>
#include <fstream>

using namespace Gdiplus;
using namespace ATL;

struct GraphData {
	std::string x;
	std::string y;
};
//typedef CGAL::Quotient<CGAL::MP_Float> Ring_number_type;
//typedef CGAL::Homogeneous<Ring_number_type> K;
typedef CGAL::Fraction_traits<CGAL::Gmpq> FT;
typedef FT::Numerator_type Numerator_type;
typedef FT::Denominator_type Denominator_type;
typedef boost::adjacency_list<boost::setS, boost::vecS, boost::undirectedS, GraphData> Graph;


using std::cout; using std::endl;

// Global variables

// The main window class name.
static TCHAR szWindowClass[] = _T("DesktopApp");

// The string that appears in the application's title bar.
static TCHAR szTitle[] = _T("Thesis");

int mode = 0; //0 art gallery, 1 funnel merge, 2 shortest map
static int ag_num = 0;
static int funnel_num = -1;
static int pages = 0;
static int iterator_i = 0;
static int iterator_f = 0;
static bool done = false;
static int firsti = 0;
static int firstj = 1;
static int firstjadd = 0;
static int firstiadd = 0;

Art_Gallery art_gal({},0, "");
static std::vector <std::vector<Point_2>> draw_pgns = {};
static std::vector<Segment_2> sgms = {};
static std::vector<Point_2> points;
static std::vector<Arrangement_2> arrs;
std::vector<std::vector<Segment_2>> faces = {};
std::vector<std::vector<Segment_2>> face_witnesses = {};

//TEST POLYGONS: p5 is irrational guard polygon
std::vector<Point_2> p1 = { Point_2(1,1), Point_2(55,1), Point_2(60,20), Point_2(40,30), Point_2(65,50), Point_2(55,80), Point_2(5,80), Point_2(15,65),
		Point_2(10,50), Point_2(36,43), Point_2(30,37), Point_2(10,30), Point_2(15, 10) };

std::vector<Point_2> p2 = { Point_2(9,11), Point_2(67,11), Point_2(61,1), Point_2(75,9), Point_2(77,15), Point_2(67,25), Point_2(51,25),
	Point_2(39,35), Point_2(75,35), Point_2(73,47), Point_2(77,55), Point_2(71,57), Point_2(69,51), Point_2(15,51), Point_2(5, 45),
	Point_2(17,47), Point_2(37,41), Point_2(29,47), Point_2(21,47), Point_2(19,50), Point_2(29,48), Point_2(47,46), Point_2(16, 25),
	Point_2(1, 35), Point_2(1, 1) };

std::vector<Point_2> p3 = { Point_2(1,27), Point_2(50,27), Point_2(75,1), Point_2(75,51), Point_2(25,51), Point_2(10,76) };

std::vector<Point_2> p4 = { Point_2(-2,55), Point_2(1,50), Point_2(1,1), Point_2(75,1), Point_2(75,50), Point_2(70,60), Point_2(65,50), Point_2(60,60),
	Point_2(55,50), Point_2(25,50),	Point_2(20,60), Point_2(15,50) };

std::vector<Point_2> p5 = {
	Point_2(0,-12,19),
	Point_2(2 * 19,-12, 19), Point_2(2,0), //first nook		
	Point_2(3,0), Point_2(3,-0.15), Point_2(3.5,0), //second nook
	Point_2(4,0), Point_2(4 * 19,-12,19), Point_2(8 * 19,-18,19), Point_2(8,0), //first bottom rectangle		
	Point_2(12,0), Point_2(12 * 21,-34,21), Point_2(16 * 21,-36,21), Point_2(16,0), //second bottom rectangle
	//Point_2(18.9,0), Point_2(19,-0.5), Point_2(19,0), //third bottom book
	Point_2(20,0), //Point_2(20,0.5), Point_2(30,0.5), Point_2(30,0.6), Point_2(20,0.6), //right tube
	Point_2(20 * 375 ,1776, 375), Point_2(19 * 375, 1776, 375), Point_2(19,4), //first top nook
	Point_2(17 * 6 + 2,4 * 6, 6), Point_2(17 * 6 + 2,4.15 * 6, 6), Point_2(16 * 6 + 5 ,4 * 6, 6), //second top nook
	Point_2(16,4), Point_2(16 * 375 ,1776, 375), Point_2(12 * 375,2486, 375), Point_2(12,4), //first top rectangle
	//Point_2(10.6,4), Point_2(10.6,8), Point_2(10.5,8), Point_2(10.5,4), //top tube
	Point_2(8,4), Point_2(8 * 47,294,47), Point_2(4 * 47,280,47), Point_2(4,4), //second top rectangle
	//Point_2(2.1,4), Point_2(2,4.5), Point_2(2,4), //third top nook
	Point_2(0,4), //Point_2(0,1.8), Point_2(-10,1.8), Point_2(-10,1.7), //left tube
	//Point_2(0,1.7)
};
std::vector<Point_2> p50 = {
	Point_2(10, 0), Point_2(24, 0), Point_2(26, 14), Point_2(21.1, 13.9), Point_2(21, 17), Point_2(26, 16), Point_2(26.5, 21.5), Point_2(30, 21),
	Point_2(28.5, 16), Point_2(32.5, 16), Point_2(33, 14), Point_2(30, 13.5), Point_2(31, 0), Point_2(53, 0), Point_2(52, 2), Point_2(57, 2), Point_2(56, 0), Point_2(70, 0),
	Point_2(70, 13), Point_2(53.5, 15), Point_2(54, 11), Point_2(50, 12), Point_2(51, 17), Point_2(70, 18), Point_2(70, 35), Point_2(53, 34), Point_2(51, 38),
	Point_2(70, 37), Point_2(70, 44), Point_2(18, 43), Point_2(18, 40), Point_2(30, 40), Point_2(31, 36), Point_2(16, 36), Point_2(16, 32), Point_2(18, 30),
	Point_2(13, 31), Point_2(13, 44), Point_2(0, 44), Point_2(0, 28.5), Point_2(13.5, 27), Point_2(14, 23), Point_2(0, 24), Point_2(0, 15), Point_2(11, 14)
};


/// <summary>Reads an input polygon from an XML file (Salzburg database)</summary>
/// <param name="name">Name of file (without extension)</param>  
/// <param name="number">Name of file (without extension)</param>  
/// <param name="rev">Whether or not result should be reversed (to get CCW)</param>  
/// <returns>Polygon in terms of a vector of its points</returns>  
std::vector<Point_2> read_poly_from_xml_file(std::string name, bool rev) {
	LPCSTR filename = std::string("c:\\dev\\" + name + ".graphml").c_str();

	std::ifstream is(filename);
	Graph g;
	boost::dynamic_properties dp(boost::ignore_other_properties);
	dp.property("vertex-coordinate-x", boost::get(&GraphData::x, g));
	dp.property("vertex-coordinate-y", boost::get(&GraphData::y, g));

	read_graphml(is, g, dp);
	
	std::vector<Point_2> poly = {};
	for (auto vp = vertices(g); vp.first != vp.second; ++vp.first)
	{
		poly.push_back(Point_2(std::stod(boost::get(&GraphData::x, g, *vp.first)),
			std::stod(boost::get(&GraphData::y, g, *vp.first))));
	}
	
	if(rev)
		std::reverse(poly.begin(), poly.end());

	return poly;
}

/// <summary>Reads an input polygon from text file </summary>
/// <param name="name">Name of file (without extension)</param> 
/// <returns>Polygon in terms of a vector of its points</returns>  
std::vector<Point_2> read_poly_from_file(std::string name) {
	FILE *fp;
	char str[MAXCHAR];
	std::string filename = std::string("c:\\dev\\" + name + ".txt");
	std::vector<Point_2> poly = {};
	errno_t err = fopen_s(&fp, filename.c_str(), "r");

	if (fp == NULL) {
		printf("Could not open file %s", filename.c_str());
	}
	while (fgets(str, MAXCHAR, fp) != NULL) {
		std::istringstream iss(str);
		std::vector<std::string> results(std::istream_iterator<std::string>{iss},
			std::istream_iterator<std::string>());

		double x = std::stod(results[0]);
		double y = std::stod(results[1]);

		poly.push_back(Point_2(x, y));
	}
	fclose(fp);
	return poly;
}

/// <summary>Reads an input polygon from .pol file </summary>
/// <param name="name">Name of file (randsimple, ortho, etc...)</param> 
/// <param name="n">Size of the polygon</param> 
/// <param name="num">ID of the polygon</param> 
/// <returns>Polygon in terms of a vector of its points</returns> 
std::vector<Point_2> read_poly_from_pol_file(std::string name, int n, int num) {


	std::string filename = std::string("c:\\dev\\poly\\" + name + "\\" + name + "-" + std::to_string(n) + "-" + std::to_string(num) + ".pol");
	std::vector<Point_2> poly = {};

	std::ifstream inFile;
	inFile.open(filename);
	std::string str;
	std::getline(inFile, str);
	std::istringstream iss(str);
	std::vector<std::string> results(std::istream_iterator<std::string>{iss},
		std::istream_iterator<std::string>());

	std::string delimiter = "/";
	for (int i = 1; i < results.size(); i = i + 2) {
		double num1, div1;
		double num2, div2;

		int findpos1 = results[i].find(delimiter);
		int findpos2 = results[i + 1].find(delimiter);

		num1 = std::stod(results[i].substr(0, findpos1));
		div1 = std::stod(results[i].substr(findpos1 + 1, results[i].length() - 1 - findpos1));

		num2 = std::stod(results[i + 1].substr(0, findpos2));
		div2 = std::stod(results[i + 1].substr(findpos2 + 1, results[i + 1].length() - 1 - findpos2));

		poly.push_back(Point_2(num1 / div1, num2 / div2));
	}

	/*poly = {Point_2(1,1), Point_2(2, 1), Point_2(2,2)};

	Polygon_2 pol(poly.begin(), poly.end());
	CGAL::Orientation or = pol.orientation();

	std::reverse(poly.begin(), poly.end());*/
	return poly;
}



/// <summary> Finds solution for a polygon </summary>
/// <param name="poly">The polygon to solve</param> 
/// <param name="sheet">The excel sheet object</param> 
/// <param name="line">Which line to write to in the excel file</param> 
/// <param name="time_limit">Time limit of the test, in case it is unsolvable</param> 
void do_tests(std::vector<Point_2> poly, libxl::Sheet* sheet, int line, double time_limit) {
	int startloc = 2;
	if (sheet)
	{	
		//Shortest_Map sm = Shortest_Map(poly);
		bool with_arr = false;
		bool with_big = false;
		bool with_crit = true;

		Art_Gallery art_gal = Art_Gallery::Art_Gallery(poly, 1, "", with_arr, time_limit, with_crit, with_big);


		sheet->writeNum(startloc + line, 1,  poly.size()); //Size
		sheet->writeNum(startloc + line, 2,  art_gal.time_taken.size()); //Total iterations
		sheet->writeNum(startloc + line, 3, art_gal.vertices_count.back()); //no. vertices at end
		sheet->writeNum(startloc + line, 4, art_gal.faces_count.back()); //no. faces at end
		sheet->writeNum(startloc + line, 5, std::accumulate(art_gal.crit_cycles.begin(), art_gal.crit_cycles.end(), 0)); // total crit cycles
		sheet->writeNum(startloc + line, 6, art_gal.crit_vert.back()); //crit vert at end
		sheet->writeNum(startloc + line, 7, art_gal.crit_fac.back()); //crit faces at end
		sheet->writeNum(startloc + line, 8, std::accumulate(art_gal.unspl_count.begin(), art_gal.unspl_count.end(), 0)); //number of unsresult_arrplittable faces
		sheet->writeNum(startloc + line, 9, std::accumulate(art_gal.normal_splits_count.begin(), art_gal.normal_splits_count.end(), 0)); //total square splits
		sheet->writeNum(startloc + line, 10, std::accumulate(art_gal.ray_splits_count.begin(), art_gal.ray_splits_count.end(), 0)); //total ray splits
		sheet->writeNum(startloc + line, 11, std::accumulate(art_gal.chord_splits_count.begin(), art_gal.chord_splits_count.end(), 0)); //total chord splits
		sheet->writeNum(startloc + line, 12, std::accumulate(art_gal.extension_splits_count.begin(), art_gal.extension_splits_count.end(), 0)); //total ext splits
		sheet->writeNum(startloc + line, 13, art_gal.rays_used_count.back()); //Angular rays at end
		sheet->writeNum(startloc + line, 14,  art_gal.result_pg.back().size()); //Solution size at end
		sheet->writeNum(startloc + line, 15, art_gal.current_time_taken); //Total time taken (!)
		sheet->writeNum(startloc + line, 16, std::accumulate(art_gal.skips_count.begin(), art_gal.skips_count.end(), 0)); //number of unsplittable faces
		sheet->writeNum(startloc + line, 17,  art_gal.sm.decomp.size()); //maximum number of vertices in a decomp
		sheet->writeNum(startloc + line, 18, art_gal.max_decomp_v); //maximum number of vertices in a decomp
		sheet->writeNum(startloc + line, 19, art_gal.max_decomp_rv); //maximum number of reflex vertices in a decomp

	}
}



std::vector<Point_2> p6 = { read_poly_from_xml_file("200_vertices", true) }; //example way to read a Salzburg file

std::vector<Point_2> p11 = { read_poly_from_pol_file("randsimple", 60, 3) };

std::vector<std::vector<Point_2>> pgns = { p11 };

LPCSTR current_p = "p14";
std::vector<std::vector<Point_2>> pgns_for_vis = {};
std::vector<Segment_2> sgms_for_vis = {};
LPCSTR p5_str = "p5";
LPCSTR p6_str = "p6";
LPCSTR p7_str = "p7";
LPCSTR p9_str = "p9";
LPCSTR p10_str = "p10"; //1000 vertices
LPCSTR p11_str = "p11"; //40 vertices 
LPCSTR p12_str = "p12"; 
LPCSTR p13_str = "p13"; //500 vertices
LPCSTR p14_str = "p14"; //60 vertices 
LPCSTR p15_str = "p15"; //200 vertices 

Shortest_Map sm;
int map_num = -1;

HINSTANCE hInst;

int scale_coordinate(int in) {
	if (mode == 0 || mode == 2) {
		if (strcmp(current_p, p5_str) == 0) {
			//return Point(int(20 + x * 4 * 10), int(500 - y * 4 * 10 + 350)); //irational
			//return 10 * in;
			return in;
		}
		else
			return in;
	}
	if (mode == 1)
		return 10 * in;
	return 0;
}

/// <summary>Converts CGAL coordinates to polar coordinates to draw on the screen using WINAPI</summary>
/// <param name="x">X-value of coordinate</param> 
/// <param name="y">Y-value of coordinate</param> 
/// <returns>Polar WINAPI point</returns> 
Point to_polar(double x, double y)
{
	//screenwidth & height = 1900, 1000	
//	return Point(int(x * 1.8 * 3 + 950 ), int(500 - y * 1.8 * 3));
	if (mode == 0 || mode == 2) { 
		if (strcmp(current_p, p5_str) == 0) {
			return Point(int(100 + x * 4 * 10), int(500 - y * 4 * 10 + 350)); //irational
			//return Point(x*600  + 2500, 5000 - y * 600); //irational
		}
		else if (strcmp(current_p, p6_str) == 0) {
			return Point(int(20 + x * 1800), int(500 - y * 880 + 450)); //200 vertices
		}
		else if (strcmp(current_p, p7_str) == 0) {
			return Point(int(x * 1600 + 25), int(500 - y  * 880 + 450)); //other polygons
		}
		else if (strcmp(current_p, p9_str) == 0) {
			return Point(int(300 + x * 70), int(500 - y * 70 + 350)); //other polygons

		}
		else if (strcmp(current_p, p10_str) == 0) {
			return Point(int(400 + x * 1.9), int(400 - y * 1.8 + 525)); //other polygons
		}
		else if (strcmp(current_p, p11_str) == 0) {
			return Point(int(250 + x * 35), int(500 - y * 35 + 350)); //other polygons
		}
		else if (strcmp(current_p, p12_str) == 0) 
			return Point(int(400 + x * 0.7), int(400 - y * 0.7 + 525)); //other polygons
		else if (strcmp(current_p, p13_str) == 0) {
			return Point(int(375 + x * 3.5), int(400 - y * 3.5 + 500)); //other polygons
		}
		else if (strcmp(current_p, p14_str) == 0) {
			return Point(int(250 + x * 15), int(500 - y * 15 + 350)); //other polygons
		}
		else if (strcmp(current_p, p15_str) == 0) {
			return Point(int(450 + x * 7), int(500 - y * 7 + 450)); //other polygons
		}
		else
			return Point(int(150 + x * 4 * 2), int(500 - y * 4 * 2 + 350)); //other polygons
	}
	if(mode == 1)
		return Point(int(20 + x * 2), int(500 - y * 2 + 350)); //funnels

	return Point();
}

/// <summary>Saves screen as a png file </summary>
/// <param name="hScreen">WINAPI screen object</param> 
/// <param name="pts">Polygon points, to find bounday of image</param> 
/// <param name="file_name">File name to save to</param> 
void GetScreenShot(HDC hScreen, std::vector<Point> pts, std::string file_name)
{
	int x1 = 5000, y1=5000, x2=0, y2=0, w, h;
	std::string str = std::string("D:\\Uni\\Master\\thesis\\output\\") + file_name + std::string(".png");

	// get screen dimensions
	for (int i = 0; i < pts.size();i++) {
		if (pts[i].X < x1)
			x1 = pts[i].X;
		if (pts[i].X > x2)
			x2 = pts[i].X;
		if (pts[i].Y < y1)
			y1 = pts[i].Y;
		if (pts[i].Y > y2)
			y2 = pts[i].Y;
	}
	x1 = x1 - 10;
	y1 = y1 - 10;	
	w = x2 - x1 + 10;
	h = y2 - y1 + 10;

	// copy screen to bitmap
	HDC     hDC = CreateCompatibleDC(hScreen);
	HBITMAP hBitmap = CreateCompatibleBitmap(hScreen, w, h);
	HGDIOBJ old_obj = SelectObject(hDC, hBitmap);
	BOOL    bRet = BitBlt(hDC, 0, 0, w, h, hScreen, x1, y1, SRCCOPY);
	/*Bitmap* bmp = Bitmap::FromHBITMAP(hBitmap, HPALETTE());
	Gdiplus::Image* image = bmp;
	CLSID   encoderClsid;
	std::wstring wide_string(str.begin(), str.end());
	Status stat = image->Save(wide_string.c_str(), &encoderClsid, NULL);
	
	if (stat == Ok)
		printf("Bird.png was saved successfully\n");
	else
		printf("Failure: stat = %d\n", stat);*/

	CImage image;
	image.Attach(hBitmap);
	image.Save(str.c_str());

	// save bitmap to clipboard
	//OpenClipboard(NULL);
	//EmptyClipboard();
	//SetClipboardData(CF_BITMAP, hBitmap);
	//CloseClipboard();


	//PICTDESC pictdesc = {};
	//pictdesc.cbSizeofstruct = sizeof(pictdesc);
	//pictdesc.picType = PICTYPE_BITMAP;
	//pictdesc.bmp.hbitmap = hBitmap;

	//CComPtr<IPicture> picture;

	//#define _S(exp) (([](HRESULT hr) { if (FAILED(hr)) _com_raise_error(hr); return hr; })(exp));
	//_S(OleCreatePictureIndirect(&pictdesc, __uuidof(IPicture), FALSE, (LPVOID*)&picture));

	//// Save to a stream
	//CComPtr<IStream> stream;
	//_S(CreateStreamOnHGlobal(NULL, TRUE, &stream));
	//LONG cbSize = 0;
	//_S(picture->SaveAsFile(stream, TRUE, &cbSize));

	//// Or save to a file

	//CComPtr<IPictureDisp> disp;
	//_S(picture->QueryInterface(&disp));

	//_S(OleSavePictureFile(disp, CComBSTR(str.c_str())));

	//// clean up
	//SelectObject(hDC, old_obj);
	//DeleteDC(hDC);
	//ReleaseDC(NULL, hScreen);
	//DeleteObject(hBitmap);
}


/// <summary>Paint the art gallery iterations </summary>
VOID Paint_AG(HDC hdc, std::vector<std::vector<Point_2>> pgs, std::vector<Segment_2> sgms, std::vector<Arrangement_2> arrs, std::vector<Point_2> vis_points, 
	std::vector<std::vector<Segment_2>> faces, std::vector<std::vector<Segment_2>> face_witnesses, 
	std::vector<std::vector<Segment_2>> critical_faces, std::vector<Point_2> critical_points, Art_Gallery ag)
{
	Graphics graphics(hdc);
	graphics.SetSmoothingMode(SmoothingModeAntiAlias);

	Pen outline_pen(Color(255, 0, 0, 0));
	outline_pen.SetWidth(2);

	Pen face_pen(Color(255, 192, 192, 192));
	face_pen.SetWidth(1);

	Gdiplus::SolidBrush brush(Color(125, 0, 255, 0));
	Gdiplus::SolidBrush fw_brush(Color(70, 0, 255, 0));
	Gdiplus::SolidBrush pg_brush(Color(255, 255, 127, 80));
	Gdiplus::SolidBrush fg_brush(Color(200, 255, 140, 0));
	Gdiplus::HatchBrush cf_brush(HatchStyle::HatchStyleWideUpwardDiagonal, Color(200, 53, 255, 219), Color(0, 53, 255, 219));
	Gdiplus::SolidBrush cp_brush(Color(255, 53, 255, 219));


	Polygon_2 p;
	std::vector<Point> pts;

	for (int i = 0; i < sgms.size(); i++)
	{
		Point src = to_polar(CGAL::to_double(sgms[i].source().x()), CGAL::to_double(sgms[i].source().y()));
		Point targ = to_polar(CGAL::to_double(sgms[i].target().x()), CGAL::to_double(sgms[i].target().y()));

		graphics.DrawLine(&outline_pen, src.X, src.Y, targ.X, targ.Y);
	}
	for (int i = 0; i < pgs.size(); i++)
	{
		if (pgs[i].size() == 0)
			continue;

		pts = {};

		int j = i + 1;
		p = Polygon_2(pgs[i].begin(), pgs[i].end());
		for (auto vi = p.vertices_begin(); vi != p.vertices_end(); ++vi)
		{
			pts.push_back(to_polar(CGAL::to_double((*vi).x()), CGAL::to_double((*vi).y())));
		}
		if (i == (pgs.size() / 2))
			graphics.FillPolygon(&brush, &pts[0],  pts.size());
		else
			graphics.DrawPolygon(&outline_pen, &pts[0],  pts.size());
	}
	sgms = {};
	Face_handle fh;
	for (int i = 0; i < arrs.size();i++) {
		for (fh = arrs[i].faces_begin(); fh != arrs[i].faces_end(); ++fh)
		{
			if (fh->has_outer_ccb()) {
				sgms = {};

				Arrangement_2::Ccb_halfedge_circulator curr = fh->outer_ccb();
				sgms.push_back(Segment_2(curr->source()->point(), curr->target()->point()));
				while (++curr != fh->outer_ccb()) {
					sgms.push_back(Segment_2(curr->source()->point(), curr->target()->point()));
				}

				for (int i = 0; i < sgms.size(); i++)
				{
					Point src = to_polar(CGAL::to_double(sgms[i].source().x()), CGAL::to_double(sgms[i].source().y()));
					Point targ = to_polar(CGAL::to_double(sgms[i].target().x()), CGAL::to_double(sgms[i].target().y()));
					graphics.DrawLine(&face_pen, src.X, src.Y, targ.X, targ.Y);
				}
			}
		}
	}

	std::vector<Point> draw_pts = pts;
	
	//for (int i = 0; i < critical_points.size();i++) {
	//	Point p = to_polar(CGAL::to_double(critical_points[i].x()), CGAL::to_double(critical_points[i].y()));
	//	int scaled = scale_coordinate(6);

	//	graphics.FillEllipse(&cp_brush, p.X - scaled, p.Y - scaled, scaled*2, scaled*2);
	//}
	//for (int j = 0; j < critical_faces.size(); j++) {
	//	pts = {};
	//	std::vector<Segment_2> face_segs = critical_faces[j];
	//	//make polygon
	//	for (int i = 0; i < face_segs.size(); i++)
	//	{
	//		Point src = to_polar(CGAL::to_double(face_segs[i].source().x()), CGAL::to_double(face_segs[i].source().y()));
	//		//Point trg = to_polar(CGAL::to_double(face_segs[i].target().x()), CGAL::to_double(face_segs[i].target().y()));
	//		pts.push_back(src);
	//	}
	//	graphics.FillPolygon(&cf_brush, &pts[0], pts.size());
	//}

	for (int j = 0; j < faces.size(); j++) {
		Color cj = Color(255, rand() % 256, rand() % 256, rand() % 256);
		Gdiplus::SolidBrush cp_check_brush(cj);
		Gdiplus::Pen cp_check_pen(cj);

		//colour all vertices visible for this face
		//for (int i = 0; i < ag.vertices_count[pages]; i++) {

		//	if (ag.faceguards[ag.result_fg_i[pages][j]].sees_witness(ag.all_vertices[i], 1, ag.sm, ag.pointguards, ag.faceguards)) {
		//		pts = {};
		//		Point p = to_polar(CGAL::to_double(ag.all_vertices[i]->point().x()), CGAL::to_double(ag.all_vertices[i]->point().y()));

		//		int scaled = scale_coordinate(5);
		//		graphics.FillEllipse(&cp_check_brush, p.X - scaled, p.Y - scaled, scaled * 2, scaled * 2);

		//		for (int k = 0; k < ag.faceguards[ag.result_fg_i[pages][j]].sgms.size(); k++) {
		//			bool found;

		//			if (ag.sm.segment_sees_point(ag.faceguards[ag.result_fg_i[pages][j]].sgms[k]->source()->data(),
		//				ag.faceguards[ag.result_fg_i[pages][j]].sgms[k]->target()->data(), ag.all_vertices[i]->data(),
		//				ag.faceguards[ag.result_fg_i[pages][j]].sgms[k]->source()->point(),
		//				ag.faceguards[ag.result_fg_i[pages][j]].sgms[k]->target()->point(), ag.all_vertices[i]->point()
		//				))
		//			{
		//				Path path = ag.sm.shortest_point_point(ag.faceguards[ag.result_fg_i[pages][j]].sgms[k]->source()->data(),
		//					ag.all_vertices[i]->data(), ag.faceguards[ag.result_fg_i[pages][j]].sgms[k]->source()->point(),
		//					ag.all_vertices[i]->point(), found);
		//				pts = {};

		//				pts.push_back(to_polar(CGAL::to_double(path.from.x()), CGAL::to_double(path.from.y())));
		//				for (int i = 0; i < path.points.size(); i++)
		//					pts.push_back(to_polar(CGAL::to_double(path.points[i].x()), CGAL::to_double(path.points[i].y())));
		//				pts.push_back(to_polar(CGAL::to_double(path.to.x()), CGAL::to_double(path.to.y())));

		//				graphics.FillEllipse(&cp_check_brush, pts.back().X - 7, pts.back().Y - 7, 14, 14);
		//				//graphics.DrawLines(&cp_check_pen, &pts[0], pts.size());
		//			}


		//		}
		//	}
		//}

		pts = {};
		std::vector<Segment_2> face_segs = faces[j];
		//make polygon
		for (int i = 0; i < face_segs.size(); i++)
		{
			Point src = to_polar(CGAL::to_double(face_segs[i].source().x()), CGAL::to_double(face_segs[i].source().y()));
			//Point trg = to_polar(CGAL::to_double(face_segs[i].target().x()), CGAL::to_double(face_segs[i].target().y()));
			pts.push_back(src);
			//pts.push_back(trg);

		}
		graphics.FillPolygon(&fg_brush, &pts[0],  pts.size());

	}

	for (int j = 0; j < vis_points.size();j++) {
		Gdiplus::SolidBrush cp_check_brush(Color(255, rand()%256, rand() % 256, rand() % 256));


		//colour all vertices visible for this face
		/*for (int i = 0; i < ag.vertices_count[pages]; i++) {		

			if (ag.pointguards[ag.result_pg_i[pages][j]].sees_witness(ag.all_vertices[i], 1, ag.sm, ag.pointguards, ag.faceguards)) {
				Point p = to_polar(CGAL::to_double(ag.all_vertices[i]->point().x()), CGAL::to_double(ag.all_vertices[i]->point().y()));
					
				int scaled = scale_coordinate(5);
				graphics.FillEllipse(&cp_check_brush, p.X - scaled, p.Y - scaled, scaled * 2, scaled * 2);
			}
		}*/



		Point p = to_polar(CGAL::to_double(vis_points[j].x()), CGAL::to_double(vis_points[j].y()));
		int scaled = scale_coordinate(5);
		graphics.FillEllipse(&pg_brush, p.X - scaled, p.Y - scaled, scaled * 2, scaled * 2);
	}

	/*for (int k = 0; k < ag.reflexes.size(); k++) {
	
		int j = ag.reflexes[k]->data().id;
		if (k != 23 && k != 28)
			continue;
		if (ag.all_vertices[j]->data().is_reflex) {
			Point p = to_polar(CGAL::to_double(ag.all_vertices[j]->point().x()), CGAL::to_double(ag.all_vertices[j]->point().y()));
			int scaled = scale_coordinate(10);
			graphics.FillEllipse(&fw_brush, p.X - scaled, p.Y - scaled, scaled * 2, scaled * 2);
		}

	}*/


	for (int j = 0; j < face_witnesses.size(); j++) {
		pts = {};
		std::vector<Segment_2> face_segs = face_witnesses[j];
		//make polygon
		for (int i = 0; i < face_segs.size(); i++)
		{
			Point src = to_polar(CGAL::to_double(face_segs[i].source().x()), CGAL::to_double(face_segs[i].source().y()));
			//Point trg = to_polar(CGAL::to_double(face_segs[i].target().x()), CGAL::to_double(face_segs[i].target().y()));
			pts.push_back(src);
			//pts.push_back(trg);

		}
		graphics.FillPolygon(&fw_brush, &pts[0],  pts.size());
	}

	GetScreenShot(hdc, draw_pts, std::string(current_p) + "_" + std::to_string(pages));
}

/// <summary>Paint the shortest path map </summary>

VOID Paint_Map(HDC hdc, Shortest_Map sm) {

		Graphics graphics(hdc);
		graphics.SetSmoothingMode(SmoothingModeAntiAlias);

		Pen outline_pen(Color(255, 0, 0, 0));
		outline_pen.SetWidth(3);
		Gdiplus::SolidBrush funnel_brush(Color(255, 0, 0, 0));

		Pen funnel_pen(Color(255, 255, 0, 0));
		funnel_pen.SetWidth(2);

		Pen path_pen(Color(255, 255, 0, 255));
		path_pen.SetWidth(2);
		Gdiplus::SolidBrush path_brush(Color(255, 0, 255, 0));

		Pen diagonal_pen(Color(255, 255, 127, 80));
		diagonal_pen.SetWidth(2);
		Gdiplus::SolidBrush origin_brush(Color(255, 255, 127, 80));

		Pen sees_pen(Color(255, 0, 0, 255));
		sees_pen.SetWidth(1);
		Gdiplus::SolidBrush sees_brush(Color(255, 0, 0, 255));

		std::vector<Point> pts = {};
		std::vector<Point> pts_o = {};
		std::vector<Point> pts_c = {};

		for (int i = 0; i < sm.poly.size(); i++)
		{
			pts.push_back(to_polar(CGAL::to_double(sm.poly[i].x()), CGAL::to_double(sm.poly[i].y())));
		}
		pts_o = pts;
		graphics.FillPolygon(&sees_brush, &pts[0],  pts.size());
		graphics.DrawPolygon(&outline_pen, &pts[0],  pts.size());
		std::vector<Point> screenshot_pts = pts;

		for (int j = 0; j < sm.decomp.size(); j++) {
			/*if (j != 116)
				continue;*/

			map_num = 0;
			//int multiplier = 255 / sm.decomp[j].level_faces[map_num].size();
				//Gdiplus::SolidBrush temp_brush(Color(255, i * multiplier, i * multiplier, i * multiplier));
			//Gdiplus::SolidBrush temp_brush(Color(255, 50, 250 - j * 10, 50));
				
			Gdiplus::SolidBrush temp_brush(Color(255, 50, 226, 50));
			Gdiplus::SolidBrush temp_brush2(Color(255, 50, 180, 50));



			Pen path2_pen(Color(255, 255, 255, 0));
			Pen offending_pen(Color(200, 255, 140, 0));

			offending_pen.SetWidth(3); 
			pts = {};

			for (int k = 0; k < sm.decomp[j].poly.size();k++) {
				pts.push_back(to_polar(CGAL::to_double(sm.decomp[j].poly[k].x()), CGAL::to_double(sm.decomp[j].poly[k].y())));
			}
			pts_c = {};
			pts_c.push_back(pts[0]);
			pts_c.push_back(pts[1]);

			if (j % 2 == 0)				
				graphics.FillPolygon(&temp_brush, &pts[0],  pts.size());
			else
				graphics.FillPolygon(&temp_brush2, &pts[0],  pts.size());

			for (int i = 0;i < sm.decomp[j].level_faces[map_num].size();i++) {			
				pts = {};
				K::Triangle_2 f = sm.decomp[j].level_faces[map_num][i];
				pts.push_back(to_polar(CGAL::to_double(f.vertex(0).x()), CGAL::to_double(f.vertex(0).y())));
				pts.push_back(to_polar(CGAL::to_double(f.vertex(1).x()), CGAL::to_double(f.vertex(1).y())));
				pts.push_back(to_polar(CGAL::to_double(f.vertex(2).x()), CGAL::to_double(f.vertex(2).y())));

				graphics.DrawPolygon(&sees_pen, &pts[0],  pts.size());
			}



			//for (int i = 0; i < sm.decomp[j].children.size(); i++) {
			//	pts = {};

			//	pts.push_back(to_polar(CGAL::to_double(sm.decomp[j].children_v0_t[i][0].x()), CGAL::to_double(sm.decomp[j].children_v0_t[i][0].y())));
			//	pts.push_back(to_polar(CGAL::to_double(sm.decomp[j].children_v0_t[i][1].x()), CGAL::to_double(sm.decomp[j].children_v0_t[i][1].y())));
			//	pts.push_back(to_polar(CGAL::to_double(sm.decomp[j].children_v0_t[i][2].x()), CGAL::to_double(sm.decomp[j].children_v0_t[i][2].y())));
			//	graphics.FillPolygon(&origin_brush, &pts[0], pts.size());
			//}

			


			graphics.DrawLine(&offending_pen, pts_c[0], pts_c[1]);

			
			for (int k = 0; k < sm.decomp[j].map.size();k++) {
				//map_num = k;
				for (int i = 0; i < sm.decomp[j].map[map_num].size(); i++)
				{
					/*if (!sm.decomp[j].map[map_num][i].set)
						continue;

					Path shortest_path = sm.decomp[j].map[map_num][i];
					pts = {};
					pts.push_back(to_polar(CGAL::to_double(shortest_path.from.x()), CGAL::to_double(shortest_path.from.y())));


					for (int i = 0; i < shortest_path.points.size(); i++)
						pts.push_back(to_polar(CGAL::to_double(shortest_path.points[i].x()), CGAL::to_double(shortest_path.points[i].y())));
					pts.push_back(to_polar(CGAL::to_double(shortest_path.to.x()), CGAL::to_double(shortest_path.to.y())));

					graphics.FillEllipse(&path_brush, pts.back().X - 7, pts.back().Y - 7, 14, 14);
					graphics.DrawLines(&path_pen, &pts[0], pts.size());*/
				}
			}
		}
		pts = { to_polar(CGAL::to_double(sm.poly[map_num].x()), CGAL::to_double(sm.poly[map_num].y())) };
		//graphics.FillEllipse(&origin_brush, pts.back().X - 7, pts.back().Y - 7, 14, 14);

		//diagonalmap test
		//Point_2 qp = midpoint(sm.decomp[0].poly[10], sm.decomp[0].poly[10]);
		//Point_2 qp = sm.decomp[0].poly[6];
		//Point_2 qp = Point_2(sm.decomp[5].poly[2].x(), sm.decomp[5].poly[1].y() - 1.1);

		//Point_2 pq = sm.decomp[0].poly[3];
		//Point_2 newP1 = sm.decomp[1].poly[9];
		/*Point_2 pq = point_intersection(
			Ray_2(newP1, Point_2(newP1.x(), newP1.y() + 1)),
			Segment_2(sm.decomp[1].poly[0], sm.decomp[1].poly[1])
		);

		
*/

//		Point_2 pq = sm.decomp[4].poly[1];
//		//Point_2 pq = sm.decomp[1].poly[9];
//		//Point_2 newP2 = Point_2(sm.decomp[1].poly[9].x() + 0.4, sm.decomp[1].poly[9].y());
//		Point_2 pq2 = Point_2(pq.x() + 0.01, pq.y());
//		//Point_2 pq2 = Point_2(pq.x() - 1.6, pq.y());
//		//Point_2 pq2 = newP1;
//
//		//Point_2 pq2 = sm.decomp[0].poly[0];
//
//		Vertex_handle v1;
//		Vertex_handle v2;
//		Vertex_handle v3;
//
//
//		Art_Gallery arr = Art_Gallery({}, 0);
//
//		Arrangement_2 arr1 = arr.orthogonal_creation(sm.decomp[0].poly);
//		Arrangement_2 arr2 = arr.orthogonal_creation(sm.decomp[4].poly);
//		Arrangement_2 arr3 = arr.orthogonal_creation(sm.decomp[5].poly);
//
//		//Segment_2(newP2, pq2)
//		/*std::vector<Segment_2> segs = { Segment_2(pq2, Point_2(pq2.x(), sm.decomp[1].poly[16].y())) };
//		insert(arr2, segs.begin(), segs.end());
//
//		segs = { Segment_2(qp, sm.decomp[5].poly[1]) };
//		insert(arr3, segs.begin(), segs.end());*/
//
//		Arrangement_2::Vertex_handle curr = arr1.vertices_begin();
//
//		//arr.insert_in_face_interior(qp, arr.faces_begin());
//		//arr2.insert_in_face_interior(pq2, arr2.faces_begin());
//
//		//Add pointguards
//		while (curr != arr1.vertices_end()) {
//			Vertex_info inf;
//			if (same(curr->point(), qp)) {
//				v1 = curr;
//				inf.id = 0;
//				inf.decomp_index = 0;
//				inf = sm.decomp[0].get_location(inf, curr);
//
//				v1->set_data(inf);
//			}		
//			/*if (same(curr->point(), pq)) {
//				v2 = curr;
//				inf.id = 0;
//				inf.decomp_index = 0;
//				inf = sm.decomp[0].get_location(inf, curr);
//				v2->set_data(inf);
//			}*/
//			/*if (same(curr->point(), pq2)) {
//				v3 = curr;
//				inf.id = 0;
//				inf.decomp_index = 0;
//				inf = sm.decomp[0].get_location(inf, curr);
//				v3->set_data(inf);
//			}*/
//			sm.decomp[0].get_location(inf, curr);
//			curr++;
//		}
//		curr = arr2.vertices_begin();
//		while (curr != arr2.vertices_end()) {
//			Vertex_info inf;		
//		
//		if (same(curr->point(), pq)) {
//				v2 = curr;
//				inf.id = 0;
//				inf.decomp_index = 4;
//				inf = sm.decomp[4].get_location(inf, curr);
//				v2 ->set_data(inf);
//			}
//			if (same(curr->point(), pq2)) {
//				v3 = curr;
//				inf.id = 0;
//				inf.decomp_index = 1;
//				inf = sm.decomp[1].get_location(inf, curr);
//				v3->set_data(inf);
//			}
//			sm.decomp[4].get_location(inf, curr);
//			curr++;
//		}
//		curr = arr3.vertices_begin();
//
//		while (curr != arr3.vertices_end()) {
//			Vertex_info inf;
//			/*if (same(curr->point(), qp)) {
//				v1 = curr;
//				inf.id = 0;
//				inf.decomp_index = 5;
//				inf = sm.decomp[5].get_location(inf, curr);
//				v1->set_data(inf);
//			}*/
//			/*if (same(curr->point(), pq)) {
//				v2 = curr;
//				inf.id = 0;
//				inf.decomp_index = 0;
//				inf = sm.decomp[0].get_location(inf, curr);
//				v2->set_data(inf);
//			}*/
//			/*if (same(curr->point(), pq2)) {
//				v3 = curr;
//				inf.id = 0;
//				inf.decomp_index = 0;
//				inf = sm.decomp[0].get_location(inf, curr);
//				v3->set_data(inf);
//			}*/
//			sm.decomp[5].get_location(inf, curr);
//			curr++;
//		}
//		bool f;
//		//Path path = sm.shortest_point_point(v3, v1, f);
//		Path path = sm.shortest_point_point(v2, v1, f);
//
//		//Funnel f1(sm.decomp[0].map[9][2], sm.decomp[0].map[9][3]);
//		//Funnel f2(sm.decomp[1].map[9][1], sm.decomp[1].map[9][0]);
//		
//		/*Funnel f1(sm.decomp[1].map[9][6], sm.decomp[1].map[9][13]);
//		Funnel f2(Path(sm.decomp[1].poly[0], sm.decomp[1].poly[6], {}), Path(sm.decomp[1].poly[0], sm.decomp[1].poly[13], {}));
//*/
//		Path p1 = sm.decomp[5].parentMap[5][8];
//		p1.reverse();
//		Path p2 = sm.decomp[5].parentMap[4][8];
//		p2.reverse();
//		
//		Funnel f1(p1, p2);
//		Funnel f2(sm.decomp[4].map[2][0], sm.decomp[4].map[2][1]);
//		/*Funnel f1(sm.decomp[0].map[13][0], sm.decomp[0].map[6][3]);
//		Funnel f2(sm.decomp[0].map[4][5], sm.decomp[0].map[4][3]);*/
//
//
//		path = f2.Merge(f1);
//
//		//from 8 and 13
//		//path = sm.decomp[1].map[9][6];
//
//		//for (int i = 0; i < sm.decomp[1].poly.size();i++) {
//			//path = sm.decomp[1].parentMap[4][12];
//			//path = sm.decomp[1].map[12][i];
//		//path = sm.decomp[5].parentMap[5][8];
//			path.reverse();
//			//bool test = sm.segment_sees_point(v3, v2, v1);
//			bool test = sm.point_sees_point(v2, v1);
//			pts = {};
//			pts.push_back(to_polar(CGAL::to_double(path.from.x()), CGAL::to_double(path.from.y())));
//			graphics.FillEllipse(&sees_brush, pts.back().X - 7, pts.back().Y - 7, 14, 14);
//
//			for (int i = 0; i < path.points.size(); i++)
//				pts.push_back(to_polar(CGAL::to_double(path.points[i].x()), CGAL::to_double(path.points[i].y())));
//			pts.push_back(to_polar(CGAL::to_double(path.to.x()), CGAL::to_double(path.to.y())));
//
//			graphics.FillEllipse(&sees_brush, pts.back().X - 7, pts.back().Y - 7, 14, 14);
//
//			if (test)
//				graphics.DrawLines(&path_pen, &pts[0], pts.size());
//			else
//				graphics.DrawLines(&funnel_pen, &pts[0], pts.size());
		//}
		
		//for (int k = 0; k < sm.decomp.size(); k++) {			
		//	for (int j = 0; j < sm.decomp[k].poly.size();j++) { //sm.decomp[k].poly.size() 
		//		if (sm.decomp[k].parentId > -1) {
		//			for (int i = 0; i < sm.decomp[sm.decomp[k].parentId].poly.size();i++) { //sm.decomp[sm.decomp[k].parentId]
		//				Path path = sm.decomp[k].parentMap[i][j];
		//				pts = {};
		//				pts.push_back(to_polar(CGAL::to_double(path.from.x()), CGAL::to_double(path.from.y())));
		//				graphics.FillEllipse(&sees_brush, pts.back().X - 7, pts.back().Y - 7, 14, 14);
		//				for (int i = 0; i < path.points.size(); i++)
		//					pts.push_back(to_polar(CGAL::to_double(path.points[i].x()), CGAL::to_double(path.points[i].y())));
		//				pts.push_back(to_polar(CGAL::to_double(path.to.x()), CGAL::to_double(path.to.y())));
		//				graphics.FillEllipse(&sees_brush, pts.back().X - 7, pts.back().Y - 7, 14, 14);
		//				graphics.DrawLines(&sees_pen, &pts[0], pts.size());
		//			}
		//		}
		//	}
		//}

		for (int k = 0; k < sm.decomp.size(); k++) {
			for (int j = 0; j < sm.decomp[k].poly.size();j++) { //sm.decomp[k].poly.size() 
				if (sm.decomp[k].parentId > -1) {
					//for all other childrens
					for (int l = sm.decomp[k].childId + 1; l < sm.decomp[sm.decomp[k].parentId].children.size(); l++) {
						for (int i = 0; i < sm.decomp[sm.decomp[sm.decomp[k].parentId].children[l]].poly.size();i++) { //sm.decomp[sm.decomp[k].parentId]
							Path path = sm.decomp[k].siblingMap[l][j][i];
							pts = {};
							pts.push_back(to_polar(CGAL::to_double(path.from.x()), CGAL::to_double(path.from.y())));
							graphics.FillEllipse(&sees_brush, pts.back().X - 7, pts.back().Y - 7, 14, 14);
							for (int i = 0; i < path.points.size(); i++)
								pts.push_back(to_polar(CGAL::to_double(path.points[i].x()), CGAL::to_double(path.points[i].y())));
							pts.push_back(to_polar(CGAL::to_double(path.to.x()), CGAL::to_double(path.to.y())));
							graphics.FillEllipse(&sees_brush, pts.back().X - 7, pts.back().Y - 7, 14, 14);

							graphics.DrawLines(&sees_pen, &pts[0], pts.size());
						}
					}
				}
			}
		}
		/*graphics.FillEllipse(&origin_brush, q.X - 7, q.Y - 7, 14, 14);
		graphics.FillEllipse(&origin_brush, p.X - 7, p.Y - 7, 14, 14);*/


		Gdiplus::SolidBrush sbrush(Color(255, 0, 120, 255));
		//graphics.FillEllipse(&sbrush, pts_o[0].X - 7, pts_o[0].Y - 7, 14, 14);
		//Gdiplus::SolidBrush cbrush(Color(255, 0, 255, 0));
		//graphics.FillEllipse(&sbrush, pts_o[1].X - 7, pts_o[1].Y - 7, 14, 14);

		pts_c.push_back(pts_o[4]);


		//graphics.FillEllipse(&sbrush, pts_c[0].X - 7, pts_c[0].Y - 7, 14, 14);
		//graphics.FillEllipse(&cbrush, pts_c[1].X - 7, pts_c[1].Y - 7, 14, 14);

		GetScreenShot(hdc, screenshot_pts, std::string(current_p) + "_" + std::to_string(map_num));
}


// Forward declarations of functions included in this code module:
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

INT WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, PSTR, INT iCmdShow)
{
	HWND                hWnd;
	MSG                 msg;
	WNDCLASS            wndClass;
	GdiplusStartupInput gdiplusStartupInput;
	ULONG_PTR           gdiplusToken;

	// Initialize GDI+.
	GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);

	wndClass.style = CS_HREDRAW | CS_VREDRAW;
	wndClass.lpfnWndProc = WndProc;
	wndClass.cbClsExtra = 0;
	wndClass.cbWndExtra = 0;
	wndClass.hInstance = hInstance;
	wndClass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wndClass.hCursor = LoadCursor(NULL, IDC_ARROW);
	wndClass.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
	wndClass.lpszMenuName = NULL;
	wndClass.lpszClassName = TEXT("GettingStarted");

	RegisterClass(&wndClass);

	hWnd = CreateWindow(
		TEXT("GettingStarted"),   // window class name
		TEXT("Getting Started"),  // window caption
		WS_OVERLAPPEDWINDOW,      // window style
		10,            // initial x position
		10,            // initial y position
		1900,            // initial x size
		1000,            // initial y size
		NULL,                     // parent window handle
		NULL,                     // window menu handle
		hInstance,                // program instance handle
		NULL);                    // creation parameters

	ShowWindow(hWnd, iCmdShow);
	UpdateWindow(hWnd);

	while (GetMessage(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	GdiplusShutdown(gdiplusToken);
	return  msg.wParam;
}  // WinMain

//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE:  Processes messages for the main window.
//
//  WM_PAINT    - Paint the main window
//  WM_DESTROY  - post a quit message and return
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	PAINTSTRUCT ps;
	HDC hdc;
	char buffer[33];

	switch (message)
	{
	case WM_CHAR:
		switch (wParam)
		{
	
		case 't':
		{
			//This mode draws the empty polygon for testing purposes.
			mode = 0;			
			
			art_gal = Art_Gallery::Art_Gallery(pgns[0], 2, current_p);

			draw_pgns = { pgns[0] ,{} };
			sgms = {};		
			InvalidateRect(hWnd, NULL, TRUE);
			return 0L;
			break;
		}
		case 'l':
		{
			//This mode solves one polygon (current_p) and shows the iterations 
			bool with_arr = true;
			bool with_big = false;
			bool with_crit = true;

			mode = 0;
			if (art_gal.result_arr.size() == 0) { //first one
				print_time();
				art_gal = Art_Gallery::Art_Gallery(pgns[ag_num], 1, current_p, with_arr, 2400, with_crit, with_big);
				pages = 0;
			}
			else if (pages < art_gal.result_arr.size() - 1)
				pages++;
			else {
				//go to next
				ag_num = (ag_num + 1) % pgns.size();
				art_gal = Art_Gallery::Art_Gallery(pgns[ag_num], 1, current_p, with_arr, 2400, with_crit, with_big);
				pages = 0;
			}
			draw_pgns = { pgns[ag_num] ,{} };
			sgms = {};
			if(with_arr)
				arrs = art_gal.result_arr[pages];
			points = art_gal.result_pg[pages];
			faces = art_gal.result_fg[pages];
			face_witnesses = art_gal.result_fw[pages];

			//write to xls
			libxl::Book* book = xlCreateBook();
			if (book)
			{
				libxl::Sheet* sheet = book->addSheet("Sheet1");
				if (sheet)
				{
					int startloc = 2;
					sheet->writeStr(startloc, 1, "Iteration");
					sheet->writeStr(startloc, 2, "Time");
					sheet->writeStr(startloc, 3, "No. vertices");
					sheet->writeStr(startloc, 4, "No. faces");
					sheet->writeStr(startloc, 5, "No. critical cycles");
					sheet->writeStr(startloc, 6, "No. critical vertices");
					sheet->writeStr(startloc, 7, "No. critical faces");
					sheet->writeStr(startloc, 8, "Sol PG");
					sheet->writeStr(startloc, 9, "Sol FG");
					sheet->writeStr(startloc, 10, "Sol FW");
					sheet->writeStr(startloc, 11, "Hausdorff");
					sheet->writeStr(startloc, 12, "Chord splits");
					sheet->writeStr(startloc, 13, "Extension splits");
					sheet->writeStr(startloc, 14, "Ray splits");
					sheet->writeStr(startloc, 15, "Orthogonal splits");
					sheet->writeStr(startloc, 16, "Unsplittable faces added");
					sheet->writeStr(startloc, 17, "Angular rays used");
					sheet->writeStr(startloc, 18, "Skips :o");

					startloc++;
					for (int i = 0; i < art_gal.result_fg.size(); i++) {
						sheet->writeNum(startloc + i, 1, i+1);
						sheet->writeNum(startloc + i, 2, art_gal.time_taken[i]); //time
						sheet->writeNum(startloc + i, 3, art_gal.vertices_count[i]); //no. vertices
						sheet->writeNum(startloc + i, 4, art_gal.faces_count[i]); //no. faces
						sheet->writeNum(startloc + i, 5, art_gal.crit_cycles[i]); //crit cycles
						sheet->writeNum(startloc + i, 6, art_gal.crit_vert[i]); //crit vert
						sheet->writeNum(startloc + i, 7, art_gal.crit_fac[i]); //crit faces
						sheet->writeNum(startloc + i, 8,  art_gal.result_pg[i].size()); //sol pg
						sheet->writeNum(startloc + i, 9,  art_gal.result_fg[i].size()); //sol fg
						sheet->writeNum(startloc + i, 10,  art_gal.result_fw[i].size()); //sol fw
						sheet->writeNum(startloc + i, 11, art_gal.hausdorff_dist[i]); //Hausdorff distance (p5 only)
						sheet->writeNum(startloc + i, 12, art_gal.chord_splits_count[i]); //
						sheet->writeNum(startloc + i, 13, art_gal.extension_splits_count[i]); //
						sheet->writeNum(startloc + i, 14, art_gal.ray_splits_count[i]); //
						sheet->writeNum(startloc + i, 15, art_gal.normal_splits_count[i]); //
						sheet->writeNum(startloc + i, 16, art_gal.unspl_count[i]); //
						sheet->writeNum(startloc + i, 17, art_gal.rays_used_count[i]); //
						sheet->writeNum(startloc + i, 18, art_gal.skips_count[i]); //

					}

				}

				std::string filename = std::string("c:\\dev\\" + std::string(current_p) + ".xls");
				if (!book->save(filename.c_str())) {
					OutputDebugString(book->errorMessage());

				}
				book->release();
			}

			InvalidateRect(hWnd, NULL, TRUE);
			return 0L;
			break;
		}
		case 'z':
		{
			//This mode runs the tests on the Brazilian input files.
			libxl::Book* book = xlCreateBook();
			libxl::Sheet* sheet;
			int startloc = 2;
			std::vector<int> sizes = {			20, 40, 60, 80, 100, 200, 300, 400, 500, 600, 700, 800, 900, 1000, 1250, 1500, 1750, 2000, 2250, 2500, 5000 };
			std::vector<double> time_limits = { 15,  30, 25, 30, 35,  100, 150, 250, 400, 450, 650, 700, 800, 1000, 1800, 2500, 1900, 2200, 2500, 3000, 10000 };

			sizes = {60, 100, 200, 500};
			time_limits = { 45, 155, 300, 450 };

			std::string filename = std::string("c:\\dev\\randsimple.xls");

			if (book)
			{				
				if (book->load("c:\\dev\\randsimple.xls"))
				{
					sheet = book->getSheet(0);
					//int i = startloc + 1 + 410;
					int i = startloc + 1;
					for (i; i < sizes.size() * 30; i++) {
						int d = sheet->readNum(i, 2);
						
						if (d == 0){
							OutputDebugString(book->errorMessage());

							break;
						}
					}

					int rows_had = i - startloc - 1;
					int sizes_done = rows_had / 30;
					int poly_num = rows_had % 30;
					OutputDebugString(std::to_string(rows_had).c_str());
					OutputDebugString("\n");
					OutputDebugString(std::to_string(sizes_done).c_str());
					OutputDebugString("\n");
					OutputDebugString(std::to_string(poly_num).c_str());
					OutputDebugString("\n");
					firstj =   poly_num + 1;
					firsti =   sizes_done;
				}
				else {
					sheet = book->addSheet("Sheet1");
					
					sheet->writeStr(startloc, 1, "Size");
					sheet->writeStr(startloc, 2, "No. iterations");
					sheet->writeStr(startloc, 3, "No. vertices");
					sheet->writeStr(startloc, 4, "No. faces");
					sheet->writeStr(startloc, 5, "No. critical cycles");
					sheet->writeStr(startloc, 6, "No. critical vertices");
					sheet->writeStr(startloc, 7, "No. critical faces");
					sheet->writeStr(startloc, 8, "No. u-faces total");
					sheet->writeStr(startloc, 9, "No. square spits");
					sheet->writeStr(startloc, 10, "No. angular splits");
					sheet->writeStr(startloc, 11, "No. chord splits");
					sheet->writeStr(startloc, 12, "No. extension splits");
					sheet->writeStr(startloc, 13, "Angular rays (end)");
					sheet->writeStr(startloc, 14, "Solution size (end)");
					sheet->writeStr(startloc, 15, "Total time taken");
					sheet->writeStr(startloc, 16, "Total skips");
					sheet->writeStr(startloc, 17, "Decomp size");
					sheet->writeStr(startloc, 18, "Max decomp vertices");
					sheet->writeStr(startloc, 19, "Max decomp reflex vertices");
				}
				bool first = true;
				bool firstfirst = true;

				for (int i = 0; i < sizes.size(); i++) {
					if (firstfirst) {
						i = firsti + firstiadd;
						firstfirst = false;
					}

					for (int j = 1; j <= 30; j++) {
						if (first) {
							j = firstj + firstjadd;
							first = false;
						}

						int d = sheet->readNum((i * 30) + j + startloc, 2);
						if (d > 0)
							continue;

						FILE* fp;
						char str[MAXCHAR];
						char* filenam1e = "c:\\dev\\output.txt";

						fp = fopen(filenam1e, "a");
						LPCSTR built = std::string(
							"Doing size: " +
							std::to_string(sizes[i]) +
							" id :" +
							std::to_string(j) +
							"\n "
						).c_str();
						fprintf(fp, built);


						/* close the file*/
						fclose(fp);

						do_tests(read_poly_from_pol_file("randsimple", sizes[i], j), sheet, (i * 30) + j, time_limits[i]);
						
						
						if (!book->save(filename.c_str()))
							OutputDebugString(book->errorMessage());
					}
				}
				book->release();
				exit(0);
				//name = "randsimple";
				//name = "random";
				//	name = "randvon";
				//	name = "vonkoch";
				

			}
			break;
		}
		
		
		}
		break;	
	case WM_PAINT:
		if (mode == 0 && (art_gal.result_arr.size() > 0 || (draw_pgns.size()))) {
			hdc = BeginPaint(hWnd, &ps);
			_itoa_s(pages, buffer, 10);
			TextOut(hdc, 0 /* X */, 0 /* Y */, buffer, 2 /* Number of chars */);


					
			Paint_AG(hdc, draw_pgns, sgms, arrs, points, faces, face_witnesses, art_gal.result_cf[pages], art_gal.result_cp[pages], art_gal);

			EndPaint(hWnd, &ps);
		}				
		else if (mode == 2 && !done) {
			done = true;
			hdc = BeginPaint(hWnd, &ps);
			Paint_Map(hdc, sm);
			EndPaint(hWnd, &ps);	
		}
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
		break;
	}

	return 0;
}