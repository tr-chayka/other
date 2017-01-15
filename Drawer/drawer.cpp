#include "drawer.h"
#include <cmath>

static ULONG_PTR token;

static double MAX(double a, double b)
{
	return a > b ? a : b;
}

static double MIN(double a, double b)
{
	return a < b ? a : b;
}

static int GetEncoderClsid(const WCHAR* format, CLSID* pClsid)
{
	UINT  num = 0;
	UINT  size = 0;

	ImageCodecInfo* pImageCodecInfo = NULL;

	GetImageEncodersSize(&num, &size);
	if (size == 0)
		return -1;

	pImageCodecInfo = (ImageCodecInfo*)(malloc(size));
	if (pImageCodecInfo == NULL)
		return -1;

	GetImageEncoders(num, size, pImageCodecInfo);

	for (UINT j = 0; j < num; ++j)
	{
		if (wcscmp(pImageCodecInfo[j].MimeType, format) == 0)
		{
			*pClsid = pImageCodecInfo[j].Clsid;
			free(pImageCodecInfo);
			return j;
		}
	}

	free(pImageCodecInfo);
	return -1;
}

void Init()
{
	ULONG_PTR token;
	GdiplusStartupInput si;
	GdiplusStartup(&token, &si, NULL);

	::token = token;
}

void ShutDown()
{
	GdiplusShutdown(token);
}

Drawer::Drawer(int w, int h) : w(w), h(h), cell_size(100), border(20.0) //-V730
{
	FontFamily Family(L"Consolas");
	
	Fnt = new Font(&Family, 10.0);
	Btm = new Bitmap(w + 300, h);
	Canvas = new Graphics(Btm);

	Canvas->SetSmoothingMode(SmoothingModeHighQuality);
}

Drawer::~Drawer()
{
	delete Fnt;
	delete Btm;
	delete Canvas;

	for (Graph *g : G)
	{
		delete[] g->x;
		delete[] g->y;
		delete g;
	}
}

void Drawer::SetBorder(double b)
{
	border = b;
}

void Drawer::SetCellSize(double cs)
{
	cell_size = cs;
}

void Drawer::SaveToFile(wchar_t *file_name)
{
	CLSID png;
	GetEncoderClsid(L"image/png", &png);

	Btm->Save(file_name, &png, 0);
}

void Drawer::Draw(double *x, double *y, size_t count, TColor color, wchar_t *name)
{
	Graph *g = new Graph;

	g->x = new double[count];
	g->y = new double[count];

	memcpy(g->x, x, sizeof(double) * count);
	memcpy(g->y, y, sizeof(double) * count);

	g->size = count;
	g->color = color;
	g->name = name;

	G.push_back(g);
}

void Drawer::Prepare()
{
	double max_x, min_x, max_y, min_y;
	max_x = min_x = G[0]->x[0];
	max_y = min_y = G[0]->y[0];

	for (Graph *g : G)
		for (int i = 0; i < g->size; i++)
		{
			max_x = MAX(max_x, g->x[i]);
			max_y = MAX(max_y, g->y[i]);
			min_x = MIN(min_x, g->x[i]);
			min_y = MIN(min_y, g->y[i]);
		}

	sx = (w - 2 * border) / fabs(max_x - min_x);
	sy = (h - 2 * border) / fabs(max_y - min_y);
	
	if (fabs(min_y - max_y) == 0)
	{
		sy = 0.5 * (h - 2 * border) / fabs(max_y);
		min_y = (min_y < 0.0) ? min_y * 2.0 : 0.0;
	}

	ox = border - min_x * sx;
	oy = h - border + min_y * sy;
}

void Drawer::DrawGrid()
{
	wchar_t buffer[100];

	double x = MIN(MAX(border, ox), w - border);
	double y = MIN(MAX(border, oy), h - border);

	Pen axpen(Color::Black, 1.5);
	Pen grpen(Color::Gray, 1.0);
	SolidBrush text_brush(Color::Gray);

	Canvas->DrawRectangle(&axpen, (REAL)border, border, w - 2 * border, h - 2 * border);
	Canvas->DrawLine(&axpen, (REAL)border, y, w - border, y);
	Canvas->DrawLine(&axpen, x, (REAL)border, x, h - border);

	for (double l = x - cell_size, r = x + cell_size; l > border || r < w - border; l -= cell_size, r += cell_size)
	{
		PointF pos;
		pos.Y = oy > w - border ? w - border - 14 : oy + 2;

		double gl = (l - ox) / sx;
		double gr = (r - ox) / sx;

		pos.X = l;
		swprintf(buffer, L"%10.5f", gl);
		Canvas->DrawString(buffer, -1, Fnt, pos, &text_brush);
		
		pos.X = r; 
		swprintf(buffer, L"%10.5f", gr);
		Canvas->DrawString(buffer, -1, Fnt, pos, &text_brush);

		Canvas->DrawLine(&grpen, (REAL)l, border, l, h - border);
		Canvas->DrawLine(&grpen, (REAL)r, border, r, h - border);
	}

	for (double u = y - cell_size, d = y + cell_size; u > border || d < h - border; u -= cell_size, d += cell_size)
	{
		PointF pos;
		pos.X = ox < border ? border : ox;

		double gu = (oy - u) / sy;
		double gd = (oy - d) / sy;

		pos.Y = u;
		swprintf(buffer, L"%10.5f", gu);
		Canvas->DrawString(buffer, -1, Fnt, pos, &text_brush);

		pos.Y = d;
		swprintf(buffer, L"%10.5f", gd);
		Canvas->DrawString(buffer, -1, Fnt, pos, &text_brush);

		Canvas->DrawLine(&grpen, (REAL)border, u, w - border, u);
		Canvas->DrawLine(&grpen, (REAL)border, d, w - border, d);
	}
}

void Drawer::Save(wchar_t *file_name)
{
	SolidBrush bg(Color::White);
	SolidBrush text_brush(Color::Black);

	Canvas->FillRectangle(&bg, 0, 0, w + 300, h);

	if (G.size() == 0)
		goto SAVE;

	Prepare();
	DrawGrid();

	for (int i = 0; i < G.size(); i++)
	{
		Graph *g = G[i];

		Pen pen(Color(240, g->color.R, g->color.G, g->color.B), 2.0);

		double lx = ox + g->x[0] * sx;
		double ly = oy - g->y[0] * sy;

		for (int i = 1; i < g->size; i++)
		{
			double x = ox + g->x[i] * sx;
			double y = oy - g->y[i] * sy;

			Canvas->DrawLine(&pen, (REAL)lx, (REAL)ly, (REAL)x, (REAL)y);

			lx = x;
			ly = y;
		}

		PointF position = { (REAL)(w + 10.0), (REAL)border + (REAL)(i * 30.0) };
		if (g->name != nullptr)
		{
			Canvas->DrawString(g->name, -1, Fnt, position, &text_brush);
			Canvas->DrawLine(&pen, (REAL)(w + 10.0), position.Y + 20.0f, (REAL)(w + 60.0), position.Y + 20.0f);
		}
	}

	SAVE : SaveToFile(file_name);
}