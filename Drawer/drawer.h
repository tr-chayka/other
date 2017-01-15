/*
	Перед началом работы вызвать Init();
	После окончания ShutDown();

	Рисование графиков:

	Создать

		Drawer drawer(w, h); // w - ширина изображения (в пикселах), h - высота

	Нарисовать график

		drawer.Draw(x, y, count, {R, G, B}); // x, y - массивы, хранящие значения соответсвующих координат (тип double *)
												count - кол-во точек
												R, G, B - компоненты цвета

	Сохранить в файл

		drawer.Save(L"file_name"); // file_name - имя файла (*) поддерживается только формат PNG
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