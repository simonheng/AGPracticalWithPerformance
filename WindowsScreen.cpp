// HelloWindowsDesktop.cpp
// compile with: /D_UNICODE /DUNICODE /DWIN32 /D_WINDOWS /c

#include <CGAL/Exact_predicates_exact_constructions_kernel.h>
#include <CGAL/Boolean_set_operations_2.h>
#include <CGAL/Polygon_2_algorithms.h>
#include <stdlib.h>

#include <windows.h>	
#include <tchar.h>
#include <objidl.h>
#include <gdiplus.h>
#include <weak_visibility.h>
#pragma comment (lib,"Gdiplus.lib")
#include <iostream>
#include <string>
#include <array>
#include <vector>

using namespace Gdiplus;

typedef CGAL::Exact_predicates_exact_constructions_kernel K;
typedef K::Point_2 CGPoint;
typedef CGAL::Polygon_2<K> Polygon_2;
typedef CGAL::Polygon_with_holes_2<K> Polygon_with_holes_2;

using std::cout; using std::endl;

// Global variables

// The main window class name.
static TCHAR szWindowClass[] = _T("DesktopApp");

// The string that appears in the application's title bar.
static TCHAR szTitle[] = _T("Thesis");

HINSTANCE hInst;

void add_demo_segment(Segment_2 sgm, std::vector<Segment_2> &sgms, int &segment_iterator)
{
	sgms.push_back(sgm);
	segment_iterator++;
}

void add__demo_point(Point_2 p, std::vector<Point_2> &pts, int &point_iterator)
{
	pts.push_back(p);
	point_iterator++;
}

void add__demo_triangle(Segment_2 sgm1, Segment_2 sgm2, Segment_2 sgm3, std::vector<Segment_2> &t_sgms, int &triangle_iterator)
{
	t_sgms.push_back(sgm1);
	t_sgms.push_back(sgm2);
	t_sgms.push_back(sgm3);

	triangle_iterator++;
}
Point to_polar(double x, double y)
{
	//screenwidth & height = 1900, 1000	
	return Point(int(x * 1.8 + 950), int(500 - y * 1.8));
}

VOID OnPaint(HDC hdc, std::vector<std::vector<Point_2>> pgs, std::vector <Segment_2> sgms, std::vector<Point_2> vis_points, std::vector <Segment_2> t_sgms)
{
	Graphics graphics(hdc);
	Pen pen(Color(123, 0, 0, 255));
	Polygon_2 p;
	std::vector<Point> pts;

	for(int i=0; i<pgs.size(); i++)
	{
		pts = {};
		//int j = i + 1;

		if(i == (pgs.size() /2))
			pen.SetColor(Color(255, 0, 255, 0));


		p = Polygon_2(pgs[i].begin(), pgs[i].end());
		for (auto vi = p.vertices_begin(); vi != p.vertices_end(); ++vi)
		{
			pts.push_back(to_polar(CGAL::to_double((*vi).x()), CGAL::to_double((*vi).y())));
		}


		if (pts.size() == 0)
			continue;
		graphics.DrawLine(&pen, pts.back().X, pts.back().Y, pts[0].X, pts[0].Y);

		for (int i = 1; i < pts.size();i++) {
			graphics.DrawLine(&pen, pts[i-1].X, pts[i-1].Y, pts[i].X, pts[i].Y);
		}
	}

	pts = {};
	for (int i = 0; i < sgms.size(); i++) {
		pts.push_back(to_polar(CGAL::to_double(sgms[i].source().hx()), CGAL::to_double(sgms[i].source().hy())));
		pts.push_back(to_polar(CGAL::to_double(sgms[i].target().hx()), CGAL::to_double(sgms[i].target().hy())));
	}
	
	pen.SetColor(Color(255, 0, 0, 0));
	for (int i = 1; i < pts.size();i = i + 2)
		graphics.DrawLine(&pen, pts[i - 1].X, pts[i - 1].Y, pts[i].X, pts[i].Y);

	pts = {};
	for (int i = 0; i < t_sgms.size(); i++) {
		pts.push_back(to_polar(CGAL::to_double(t_sgms[i].source().hx()), CGAL::to_double(t_sgms[i].source().hy())));
		pts.push_back(to_polar(CGAL::to_double(t_sgms[i].target().hx()), CGAL::to_double(t_sgms[i].target().hy())));
	}

	pen.SetColor(Color(255, 255, 150, 0));
	for (int i = 1; i < pts.size();i = i + 2)
		graphics.DrawLine(&pen, pts[i - 1].X, pts[i - 1].Y, pts[i].X, pts[i].Y);


	for (int i = 0; i < vis_points.size();i++) {
		pen.SetColor(Color(255, 255, 0, 0));
		Point p = to_polar(CGAL::to_double(vis_points[i].x()), CGAL::to_double(vis_points[i].y()));
		graphics.DrawLine(&pen, p.X - 2, p.Y - 2 , p.X + 2, p.Y + 2);
		graphics.DrawLine(&pen, p.X + 2, p.Y - 2, p.X - 2, p.Y + 2);
	}
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
	return msg.wParam;
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

	std::vector<std::vector<Point_2>> pgns;
	std::vector<Segment_2> sgms;
	std::vector<Segment_2> t_sgms;
	std::vector<Point_2> pts;

	std::string greeting;
	std::vector<Point_2> points;

	std::vector<Point_2> p1 = { Point_2(1,1), Point_2(55,1), Point_2(60,20), Point_2(40,30), Point_2(65,50), Point_2(55,80), Point_2(5,80), Point_2(15,65),
		Point_2(10,50), Point_2(36,43), Point_2(30,37), Point_2(10,30), Point_2(15, 10) };

	std::vector<Point_2> p2 = { Point_2(-70,-48), Point_2(-12,-48), Point_2(-18,-58), Point_2(-4,-50), Point_2(-2,-44), Point_2(-12,-34), Point_2(-28,-34),
		Point_2(-40,-24), Point_2(-4,-24), Point_2(-6,-12), Point_2(-2,-4), Point_2(-8,-2), Point_2(-10,-8), Point_2(-64,-8), Point_2(-74, -14),
		Point_2(-62,-12), Point_2(-42,-18), Point_2(-50,-12), Point_2(-58,-12), Point_2(-60,-9), Point_2(-50,-11), Point_2(-32,-13), Point_2(-63, -34),
		Point_2(-78, -24), Point_2(-78, -58) };

	

	std::vector<std::vector<Point_2>> pt_pgns;
	std::vector<std::vector<Point_2>> t_pgns;

	std::vector<std::vector<Point_2>> vis_pgns;
	Point_2 mp1;
	Segment_2 sgm_test;
	Line_2 ln;

	int x_transpose = 64;
	int y_transpose = 84;
	int segment_iterator = 0;
	int point_iterator = 0;
	int triangle_iterator = 0;

	//reversed
	//std::vector<Point_2> points = { Point_2(60,20), Point_2(40,30), Point_2(65,50), Point_2(55,80), Point_2(5,80), Point_2(15,65),
		//Point_2(10,50), Point_2(36,43), Point_2(30,37), Point_2(10,30), Point_2(15, 10), Point_2(1,1), Point_2(55,1)};

	switch (message)
	{
	case WM_PAINT:
		hdc = BeginPaint(hWnd, &ps);
		greeting = "";
		//points = { Point_2(9,-70), Point_2(45,-65), Point_2(74,-48), Point_2(70,-17), Point_2(60,-1), Point_2(5,-1), Point_2(1,-40) };
		//pgns.push_back(Polygon_2(points.begin(), points.end()));
		//sgms.push_back(Segment_2(points[0], points[1]));		
		pgns = {};
		for (int i = 0; i < p2.size(); i++)
			p2[i] = Point_2(p2[i].x() * 1.5, p2[i].y() * 1.8);

		//Test polygons, top layer
		for (int i = -8; i <= 7; i++)
		{
			points = p1;
			for (int j = 0; j < points.size(); j++)
				points[j] = Point_2(points[j].x() + x_transpose * i, points[j].y() + y_transpose * 2);
			pgns.push_back(points);
		}

		//Test polygons, second layer
		for (int i = -8; i <= 7; i++)
		{
			points = p1;
			for (int j = 0; j < points.size(); j++)
				points[j] = Point_2(points[j].x() + x_transpose * i, points[j].y() + y_transpose);
			pgns.push_back(points);
		}

		//Original segments
		add_demo_segment(Segment_2(pgns[segment_iterator][0], pgns[segment_iterator][1]), sgms, segment_iterator); //convex
		add_demo_segment(Segment_2(pgns[segment_iterator][1], pgns[segment_iterator][2]), sgms, segment_iterator); //convex
		add_demo_segment(Segment_2(pgns[segment_iterator][2], pgns[segment_iterator][3]), sgms, segment_iterator); //b concave
		add_demo_segment(Segment_2(pgns[segment_iterator][3], pgns[segment_iterator][4]), sgms, segment_iterator); //a concave
		add_demo_segment(Segment_2(pgns[segment_iterator][6], pgns[segment_iterator][7]), sgms, segment_iterator); //b concave	
		add_demo_segment(Segment_2(pgns[segment_iterator][7], pgns[segment_iterator][8]), sgms, segment_iterator); //a concave
		add_demo_segment(Segment_2(pgns[segment_iterator][8], pgns[segment_iterator][9]), sgms, segment_iterator); //b concave
		add_demo_segment(Segment_2(pgns[segment_iterator][9], pgns[segment_iterator][10]), sgms, segment_iterator); //both concave
		add_demo_segment(Segment_2(pgns[segment_iterator][10], pgns[segment_iterator][11]), sgms, segment_iterator); //a concave
		add_demo_segment(Segment_2(pgns[segment_iterator][12], pgns[segment_iterator][0]), sgms, segment_iterator); //a concave

		//Half segments (a in midpoint)
		add_demo_segment(Segment_2(midpoint(pgns[segment_iterator][1], pgns[segment_iterator][2]), pgns[segment_iterator][2]), sgms, segment_iterator); // convex
		add_demo_segment(Segment_2(midpoint(pgns[segment_iterator][2], pgns[segment_iterator][3]), pgns[segment_iterator][3]), sgms, segment_iterator); // b concave
		add_demo_segment(Segment_2(midpoint(pgns[segment_iterator][3], pgns[segment_iterator][4]), pgns[segment_iterator][4]), sgms, segment_iterator); // a concave
		add_demo_segment(Segment_2(midpoint(pgns[segment_iterator][6], pgns[segment_iterator][7]), pgns[segment_iterator][7]), sgms, segment_iterator); // b concave

		//Half segments (b in midpoint)
		add_demo_segment(Segment_2(pgns[segment_iterator][7], midpoint(pgns[segment_iterator][7], pgns[segment_iterator][8])), sgms, segment_iterator); // convex
		add_demo_segment(Segment_2(pgns[segment_iterator][8], midpoint(pgns[segment_iterator][8], pgns[segment_iterator][9])), sgms, segment_iterator); // convex
		add_demo_segment(Segment_2(pgns[segment_iterator][10], midpoint(pgns[segment_iterator][10], pgns[segment_iterator][11])), sgms, segment_iterator); // convex

		//Within one segment
		add_demo_segment(Segment_2(Point_2(pgns[segment_iterator][0].x() + 15, pgns[segment_iterator][0].y()),
			Point_2(pgns[segment_iterator][1].x() - 20, pgns[segment_iterator][0].y())), sgms, segment_iterator); //within segment 0 - 1
		mp1 = midpoint(pgns[segment_iterator][8], pgns[segment_iterator][9]);
		add_demo_segment(Segment_2(midpoint(pgns[segment_iterator][8], mp1), midpoint(mp1, pgns[segment_iterator][9])), sgms, segment_iterator); //within segment 8 - 9
		mp1 = midpoint(pgns[segment_iterator][9], pgns[segment_iterator][10]);
		add_demo_segment(Segment_2(midpoint(pgns[segment_iterator][9], mp1), midpoint(mp1, pgns[segment_iterator][10])), sgms, segment_iterator); //within segment 9 - 10
		mp1 = midpoint(pgns[segment_iterator][11], pgns[segment_iterator][12]);
		add_demo_segment(Segment_2(midpoint(pgns[segment_iterator][11], mp1), midpoint(mp1, pgns[segment_iterator][12])), sgms, segment_iterator); //within segment 11 - 12

		//Floating both sides
		add_demo_segment(Segment_2(pgns[segment_iterator][3], pgns[segment_iterator][11]), sgms, segment_iterator); //both orginal, but segment floating
		add_demo_segment(Segment_2(midpoint(pgns[segment_iterator][3], pgns[segment_iterator][4]), midpoint(pgns[segment_iterator][9], pgns[segment_iterator][10]))
			, sgms, segment_iterator); //segment floating, but a and b are both midpoints of original segments

		add_demo_segment(Segment_2(Point_2(pgns[segment_iterator][9].x(), pgns[segment_iterator][9].y() + 10), 
			Point_2(pgns[segment_iterator][9].x(), pgns[segment_iterator][9].y() + 20)), sgms, segment_iterator); //both floating (vertical)		
		add_demo_segment(Segment_2(Point_2(pgns[segment_iterator][9].x(), pgns[segment_iterator][12].y()),
			Point_2(pgns[segment_iterator][9].x() + 10, pgns[segment_iterator][12].y())), sgms, segment_iterator); //both floating (horizontal)
		add_demo_segment(Segment_2(Point_2(pgns[segment_iterator][12].x() + 20, pgns[segment_iterator][12].y() + 5),
			Point_2(pgns[segment_iterator][12].x() + 10, pgns[segment_iterator][12].y() + 5)), sgms, segment_iterator); //both floating (horizontal)
		add_demo_segment(Segment_2(Point_2(pgns[segment_iterator][9].x() + 7, pgns[segment_iterator][9].y()),
			pgns[segment_iterator][9]), sgms, segment_iterator); //a (completely) floating, b original!
		ln = Line_2(pgns[segment_iterator][8], pgns[segment_iterator][9]);
		add_demo_segment(Segment_2(pgns[segment_iterator][8], Point_2(pgns[segment_iterator][9].x() + 5, ln.y_at_x(pgns[segment_iterator][8].x() + 5))),
			sgms, segment_iterator); //a original (8), b (completely) floating

		//Floating on one side
		ln = Line_2(pgns[segment_iterator][3], pgns[segment_iterator][4]);
		add_demo_segment(Segment_2(Point_2(pgns[segment_iterator][3].x() - 15, ln.y_at_x(pgns[segment_iterator][3].x() - 15)),
			pgns[segment_iterator][4]), sgms, segment_iterator); //a floating, b original (4)		
		ln = Line_2(pgns[segment_iterator][10], pgns[segment_iterator][11]);
		add_demo_segment(Segment_2(Point_2(pgns[segment_iterator][10].x() + 10, ln.y_at_x(pgns[segment_iterator][10].x() + 10)),
			pgns[segment_iterator][11]), sgms, segment_iterator); //a floating, b original (11)					
		ln = Line_2(pgns[segment_iterator][12], pgns[segment_iterator][0]);
		add_demo_segment(Segment_2(Point_2(pgns[segment_iterator][12].x() + 15, ln.y_at_x(pgns[segment_iterator][12].x() + 15)),
			pgns[segment_iterator][0]), sgms, segment_iterator); //a floating, b original (0)
		
		
		//third layer
		for (int i = -8; i <= -1; i++)
		{
			points = p1;
			for (int j = 0; j < points.size(); j++)
				points[j] = Point_2(points[j].x() + x_transpose * i, points[j].y());
			pgns.push_back(points);
		}

		add_demo_segment(Segment_2(pgns[segment_iterator][3], pgns[segment_iterator][11]), sgms, segment_iterator);
		add_demo_segment(Segment_2(pgns[segment_iterator][5], pgns[segment_iterator][9]), sgms, segment_iterator);
		add_demo_segment(Segment_2(CGAL::midpoint(pgns[segment_iterator][5], pgns[segment_iterator][6]), pgns[segment_iterator][9]), sgms, segment_iterator);
		add_demo_segment(Segment_2(pgns[segment_iterator][3],CGAL::midpoint(pgns[segment_iterator][5], pgns[segment_iterator][6])), sgms, segment_iterator);
		add_demo_segment(Segment_2(CGAL::midpoint(pgns[segment_iterator][5], pgns[segment_iterator][6]), CGAL::midpoint(pgns[segment_iterator][8], 
			pgns[segment_iterator][9])), sgms, segment_iterator);

		
		mp1 = CGAL::midpoint(CGAL::midpoint(pgns[segment_iterator][2], pgns[segment_iterator][3]), pgns[segment_iterator][11]);
		add_demo_segment(Segment_2(mp1, CGAL::midpoint(pgns[segment_iterator][2],	pgns[segment_iterator][3])), sgms, segment_iterator); //a completely floating, b between 2 and 3
		mp1 = CGAL::midpoint(CGAL::midpoint(pgns[segment_iterator][2], pgns[segment_iterator][3]), pgns[segment_iterator][11]);
		add_demo_segment(Segment_2(mp1,pgns[segment_iterator][2]), sgms, segment_iterator); //a completely floating, b 2


		for (int i = 1; i <= 1; i++)
		{
			points = p1;
			for (int j = 0; j < points.size(); j++)
				points[j] = Point_2(points[j].x() + x_transpose * i, points[j].y());
			t_pgns.push_back(points);
		}	
		
		add__demo_triangle(Segment_2(t_pgns[triangle_iterator][10], t_pgns[triangle_iterator][11]),
			Segment_2(t_pgns[triangle_iterator][11], t_pgns[triangle_iterator][3]), Segment_2(t_pgns[triangle_iterator][3], t_pgns[triangle_iterator][10]),
			t_sgms, triangle_iterator); 
		
		//Test polygons, third layer
		for (int i = -8; i <= -1; i++)
		{
			points = p1;
			for (int j = 0; j < points.size(); j++)
				points[j] = Point_2(points[j].x() + x_transpose * i, points[j].y() - y_transpose);
			pgns.push_back(points);
		}

		//Between two concave
		add_demo_segment(Segment_2(pgns[segment_iterator][3], pgns[segment_iterator][10]), sgms, segment_iterator);

		ln = Line_2(pgns[segment_iterator][3], pgns[segment_iterator][9]);
		add_demo_segment(Segment_2(Point_2(pgns[segment_iterator][3].x() + 2, ln.y_at_x(pgns[segment_iterator][3].x() + 2)),
			Point_2(pgns[segment_iterator][9].x() - 2, ln.y_at_x(pgns[segment_iterator][9].x() - 2))), sgms, segment_iterator);
		ln = Line_2(pgns[segment_iterator][3], pgns[segment_iterator][9]);
		add_demo_segment(Segment_2(Point_2(pgns[segment_iterator][3].x() + 2, ln.y_at_x(pgns[segment_iterator][3].x() + 2)),
			pgns[segment_iterator][9]), sgms, segment_iterator); //floating on a side, cutting two concave
		ln = Line_2(pgns[segment_iterator][3], pgns[segment_iterator][9]);
		add_demo_segment(Segment_2(pgns[segment_iterator][3], Point_2(pgns[segment_iterator][9].x() - 2, ln.y_at_x(pgns[segment_iterator][9].x() - 2))),
			sgms, segment_iterator); //floating on b side, cutting two concave
		ln = Line_2(pgns[segment_iterator][8], pgns[segment_iterator][9]);
		add_demo_segment(Segment_2(CGAL::midpoint(pgns[segment_iterator][8], pgns[segment_iterator][9]), Point_2(pgns[segment_iterator][9].x() + 5, ln.y_at_x(pgns[segment_iterator][8].x() + 5))),
			sgms, segment_iterator); //a between 8 and 9, b (completely) floating
		
		//From line segment to line segment
		add_demo_segment(Segment_2(Point_2(pgns[segment_iterator][9].x(), pgns[segment_iterator][0].y()),
			Point_2(pgns[segment_iterator][9].x(), pgns[segment_iterator][5].y())), sgms, segment_iterator); //cutting one concave
		ln = Line_2(pgns[segment_iterator][3], pgns[segment_iterator][9]);
		add_demo_segment(Segment_2(Point_2(ln.x_at_y(pgns[segment_iterator][0].y()), pgns[segment_iterator][0].y()), Point_2(ln.x_at_y(pgns[segment_iterator][5].y()),
			pgns[segment_iterator][5].y())), sgms, segment_iterator); //cutting two concave

		//Floating, crossing sgm with two concaves
		ln = Line_2(pgns[segment_iterator][9], pgns[segment_iterator][10]);
		add_demo_segment(Segment_2(Point_2(pgns[segment_iterator][9].x() + 2, ln.y_at_x(pgns[segment_iterator][9].x() + 2)),
			pgns[segment_iterator][10]), sgms, segment_iterator); //floating on a side
		ln = Line_2(pgns[segment_iterator][9], pgns[segment_iterator][10]);
		add_demo_segment(Segment_2(pgns[segment_iterator][9], Point_2(pgns[segment_iterator][10].x() - 2, ln.y_at_x(pgns[segment_iterator][10].x() - 2))),
			sgms, segment_iterator); //floating on b side
		ln = Line_2(pgns[segment_iterator][9], pgns[segment_iterator][10]);
		add_demo_segment(Segment_2(Point_2(pgns[segment_iterator][9].x() + 2, ln.y_at_x(pgns[segment_iterator][9].x() + 2)),
			Point_2(pgns[segment_iterator][10].x() - 2, ln.y_at_x(pgns[segment_iterator][10].x() - 2))), sgms, segment_iterator); //floating on both sides

		for (int i = 0; i <= 7; i++)
		{
			points = p1;
			for (int j = 0; j < points.size(); j++)
				points[j] = Point_2(points[j].x() + x_transpose * i, points[j].y() - y_transpose);
			pt_pgns.push_back(points);
		}

		//Points on vertices
		add__demo_point(pt_pgns[point_iterator][0], pts, point_iterator); //convex
		add__demo_point(pt_pgns[point_iterator][9], pts, point_iterator); //concave
		//add__demo_point(pt_pgns[point_iterator][10], pts, point_iterator); //concave
		//add__demo_point(pt_pgns[point_iterator][12], pts, point_iterator); //concave

		//Points in middle of segments
		add__demo_point(midpoint(pt_pgns[point_iterator][0], pt_pgns[point_iterator][1]), pts, point_iterator); //middle of 0,1
		add__demo_point(midpoint(pt_pgns[point_iterator][12], pt_pgns[point_iterator][0]), pts, point_iterator); //middle of 12,0
		add__demo_point(midpoint(pt_pgns[point_iterator][8], midpoint(pt_pgns[point_iterator][8], pt_pgns[point_iterator][9])), pts, point_iterator); //quarter of 8, 9

		//Floating points
		add__demo_point(Point_2(pt_pgns[point_iterator][2].x() - 15, pt_pgns[point_iterator][2].y()), pts, point_iterator); //floating near 2
		add__demo_point(Point_2(pt_pgns[point_iterator][10].x() + 5, pt_pgns[point_iterator][10].y()), pts, point_iterator); //floating near 10
		mp1 = midpoint(midpoint(pt_pgns[point_iterator][5], pt_pgns[point_iterator][6]), pt_pgns[point_iterator][5]);
		add__demo_point(Point_2(mp1.x(), mp1.y() - 15), pts, point_iterator); //floating near 3/4 of seg 5 to 6



		x_transpose = 125;
		y_transpose = 100;

		//Test polygons, fourth layer
		for (int i = -3; i <= 4; i++)
		{
			//points = p2;
			//for (int j = 0; j < points.size(); j++)
			//	points[j] = Point_2(points[j].x() + x_transpose * i, points[j].y());
			//pgns.push_back(points);
		}
		//Test polygons, fifth layer

		for (int i = -3; i <= 0; i++)
		{
			points = p2;
			for (int j = 0; j < points.size(); j++)
				points[j] = Point_2(points[j].x() + x_transpose * i, points[j].y() + y_transpose * -1);
			pgns.push_back(points);
		}

		//Original segments
		//add_demo_segment(Segment_2(pgns[segment_iterator][0], pgns[segment_iterator][1]), sgms, segment_iterator); //both concave
		//add_demo_segment(Segment_2(pgns[segment_iterator][1], pgns[segment_iterator][2]), sgms, segment_iterator); //a concave
		//add_demo_segment(Segment_2(pgns[segment_iterator][2], pgns[segment_iterator][3]), sgms, segment_iterator); //both convex
		//add_demo_segment(Segment_2(pgns[segment_iterator][17], pgns[segment_iterator][18]), sgms, segment_iterator); //b concave

		//half segments (a in mp)
		//add_demo_segment(Segment_2(midpoint(pgns[segment_iterator][6], pgns[segment_iterator][7]), pgns[segment_iterator][7]), sgms, segment_iterator); //b concave

		//half segments (b in mp)
		//add_demo_segment(Segment_2(midpoint(pgns[segment_iterator][14], pgns[segment_iterator][15]), pgns[segment_iterator][15]), sgms, segment_iterator); //b concave

		//Within one segment
		//add_demo_segment(Segment_2(Point_2(pgns[segment_iterator][0].x() + 15, pgns[segment_iterator][0].y()),
		//	Point_2(pgns[segment_iterator][1].x() - 20, pgns[segment_iterator][0].y())), sgms, segment_iterator); //within segment 0 - 1

		//Floating both sides
		//add_demo_segment(Segment_2(pgns[segment_iterator][7], pgns[segment_iterator][21]), sgms, segment_iterator); //both orginal, but segment floating
		ln = Line_2(Segment_2(pgns[segment_iterator][21], pgns[segment_iterator][22]));
		add_demo_segment(Segment_2(Point_2(pgns[segment_iterator][7].x(), pgns[segment_iterator][0].y()), Point_2(pgns[segment_iterator][7].x(), ln.y_at_x(pgns[segment_iterator][7].x())))
			, sgms, segment_iterator); //segment floating, a and b are not vertices, but are on original segments		
		add_demo_segment(Segment_2(Point_2(pgns[segment_iterator][7].x(), pgns[segment_iterator][7].y() - 10), Point_2(pgns[segment_iterator][7].x() - 10,
			pgns[segment_iterator][7].y())), sgms, segment_iterator); //both completely floating
		ln = Line_2(pgns[segment_iterator][8], pgns[segment_iterator][9]);
		add_demo_segment(Segment_2(	pgns[segment_iterator][8], Point_2(pgns[segment_iterator][9].x() - 1, ln.y_at_x(pgns[segment_iterator][9].x() - 1))),
			sgms, segment_iterator); //a original (8), b floating 		
		ln = Line_2(pgns[segment_iterator][21], pgns[segment_iterator][22]);
		add_demo_segment(Segment_2(Point_2(pgns[segment_iterator][21].x() + 10, ln.y_at_x(pgns[segment_iterator][21].x() + 10)),
			pgns[segment_iterator][22]), sgms, segment_iterator); //a floating, b original (22)	

		for (int i = 1; i <= 4; i++)
		{
			points = p2;
			for (int j = 0; j < points.size(); j++)
				points[j] = Point_2(points[j].x() + x_transpose * i, points[j].y() + y_transpose * -1);
			pt_pgns.push_back(points);
		}

		//Points on vertices
		add__demo_point(pt_pgns[point_iterator][7], pts, point_iterator); //convex
		add__demo_point(pt_pgns[point_iterator][22], pts, point_iterator); //concave

		//Points on segment
		add__demo_point(midpoint(pt_pgns[point_iterator][0], pt_pgns[point_iterator][1]), pts, point_iterator); 

		//Points floating
		add__demo_point(midpoint(pt_pgns[point_iterator][7], pt_pgns[point_iterator][12]), pts, point_iterator); 

		vis_pgns = {};

		pgns.erase(pgns.begin() + 14, pgns.end());
		sgms.erase(sgms.begin() + 14, sgms.end());


		for (int i = pgns.size() - 1; i >= 0; i--)
		{
			vis_pgns.push_back(compute_visibility_from_segment(Polygon_2(pgns[i].begin(), pgns[i].end()), sgms[i].source(), sgms[i].target()));
		}
		for (int i = pt_pgns.size() - 1; i >= 0; i--)
		{
			vis_pgns.push_back(compute_visibility_from_point(Polygon_2(pt_pgns[i].begin(), pt_pgns[i].end()), pts[i]));
		}

		//for (int i = 0; i < t_pgns.size(); i++)
		//{
			//std::vector<Point_2> pgn_1 = compute_visibility_from_segment(Polygon_2(t_pgns[i].begin(), t_pgns[i].end()), t_sgms[i * 3].source(), t_sgms[i * 3].target());
			//std::vector<Point_2> pgn_2 = compute_visibility_from_segment(Polygon_2(t_pgns[i].begin(), t_pgns[i].end()), t_sgms[i * 3 + 1].source(), t_sgms[i * 3 + 1].target());
			//std::vector<Point_2> pgn_3 = compute_visibility_from_segment(Polygon_2(t_pgns[i].begin(), t_pgns[i].end()), t_sgms[i * 3 + 2].source(), t_sgms[i * 3 + 2].target());

			//Polygon_with_holes_2 triangle_p;
			//CGAL::join(Polygon_2(pgn_2.begin(), pgn_2.end()), Polygon_2(pgn_1.begin(), pgn_1.end()), triangle_p);
		    //CGAL::join(Polygon_2(pgn_3.begin(), pgn_3.end()), triangle_p, triangle_p);

			//vis_pgns.push_back(triangle_p.outer_boundary);
		//}
		pgns.insert(pgns.end(), pt_pgns.begin(), pt_pgns.end());

		//pgns.insert(pgns.end(), t_pgns.begin(), t_pgns.end());
		pgns.insert(pgns.end(), vis_pgns.begin(), vis_pgns.end());


		// Here your application is laid out.
		// For this introduction, we just print out "Hello, Windows desktop!"
		// in the top left corner.
		TextOut(hdc,
			5, 5,
			greeting.c_str(), greeting.length());
		// End application-specific layout section.
		OnPaint(hdc, pgns, sgms, pts, t_sgms);
		EndPaint(hWnd, &ps);
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