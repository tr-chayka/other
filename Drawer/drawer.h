/*
	����� ������� ������ ������� Init();
	����� ��������� ShutDown();

	��������� ��������:

	�������

		Drawer drawer(w, h); // w - ������ ����������� (� ��������), h - ������

	���������� ������

		drawer.Draw(x, y, count, {R, G, B}); // x, y - �������, �������� �������� �������������� ��������� (��� double *)
												count - ���-�� �����
												R, G, B - ���������� �����

	��������� � ����

		drawer.Save(L"file_name"); // file_name - ��� ����� (*) �������������� ������ ������ PNG
*/

#pragma once
#pragma comment(lib, "gdiplus.lib")

#include <windows.h>
#include <gdiplus.h>
#include <vector>

using namespace Gdiplus;
using std::vector;

void Init();
void ShutDown();

struct TColor
{
	BYTE R;
	BYTE G;
	BYTE B;
};

class Drawer
{
	double w, h;
	double cell_size;
	double border;

	Font *Fnt;
	Bitmap *Btm;
	Graphics *Canvas;

	struct Graph
	{
		double *x;
		double *y;
		size_t size;
		TColor color;
		wchar_t *name;
	};

	vector<Graph *> G;

	double ox, oy;
	double sx, sy;

	void Prepare();
	void DrawGrid();
	void SaveToFile(wchar_t *);

public:
	Drawer(int, int);
	~Drawer();

	void SetBorder(double);
	void SetCellSize(double);
	void Draw(double *x, double *y, size_t count, TColor color = {100, 100, 100}, wchar_t *name = nullptr);
	void Save(wchar_t *file_name);
};