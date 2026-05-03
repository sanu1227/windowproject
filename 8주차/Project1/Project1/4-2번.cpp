#include <windows.h>
#include <tchar.h>
#include <stdlib.h>
#include <time.h>

HINSTANCE g_hInst;

LPCTSTR lpszClass = L"My Window Class";
LPCTSTR lpszWindowName = L"Intersection Car Program";

LRESULT CALLBACK WndProc(HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam);

#define WINDOW_WIDTH 900
#define WINDOW_HEIGHT 700

#define TIMER_MOVE 1
#define TIMER_YELLOW 2
#define TIMER_AUTO 3

#define MENU_H_BLUE_V_RED 1001
#define MENU_H_RED_V_BLUE 1002
#define MENU_AUTO 1003
#define MENU_STOP 1004
#define MENU_QUIT 1005

#define SIGNAL_RED 0
#define SIGNAL_BLUE 1
#define SIGNAL_YELLOW 2

#define DIR_RIGHT 0
#define DIR_LEFT 1
#define DIR_DOWN 2
#define DIR_UP 3

#define PERSON_WAIT 0
#define PERSON_CROSS_RIGHT_SIDE 1
#define PERSON_CROSS_BOTTOM_SIDE 2
#define PERSON_CROSS_LEFT_SIDE 3
#define PERSON_CROSS_TOP_SIDE 4
#define PERSON_CROSS_DIAGONAL 5

struct CAR {
	int x;
	int y;
	int w;
	int h;
	int speed;
	int dir;
	COLORREF color;
};

CAR cars[8];

int hSignal = SIGNAL_BLUE;
int vSignal = SIGNAL_RED;

int beforeHSignal = SIGNAL_BLUE;
int beforeVSignal = SIGNAL_RED;
int yellowTarget = 0;

int autoMode = 0;
int globalStop = 0;

int speedPlus = 0;

RECT hSignalRect = { 130, 90, 190, 150 };
RECT vSignalRect = { 220, 90, 280, 150 };

const int roadLeft = 330;
const int roadRight = 570;
const int roadTop = 230;
const int roadBottom = 470;

const int centerX = 450;
const int centerY = 350;

const int stopGap = 15;
const int stopLineGap = 45;

int personX = roadRight + 45;
int personY = roadTop - 90;

int personState = PERSON_WAIT;
int personCorner = 0;
int lastPersonSignal = -1;

int personTargetX = 0;
int personTargetY = 0;
int personDiagonalCorner = 0;

void MakeMenu(HWND hWnd);
void InitCars();

void DrawRoad(HDC hDC);
void DrawSignals(HDC hDC);
void DrawCars(HDC hDC);
void DrawOneCar(HDC hDC, CAR car);
void DrawPerson(HDC hDC);

void MoveCars();
void MoveOneCar(int index);
void FixRearCarGap();

void MovePerson();
void StartPersonMove();
void StartPersonDiagonalMove();

void SetHBlueVRed();
void SetHRedVBlue();
void StartYellow(HWND hWnd, int target);
void FinishYellow();

int IsInsideRect(int mx, int my, RECT rc);
int IsGreenSignalForCar(CAR car);
int GetStopLine(CAR car);
int CanMoveBySignalAndStopLine(CAR car);
int IsCarAlreadyPassedStopLine(CAR car);

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpszCmdParam, int nCmdShow)
{
	HWND hWnd;
	MSG Message;
	WNDCLASSEX WndClass;

	g_hInst = hInstance;

	WndClass.cbSize = sizeof(WndClass);
	WndClass.style = CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS;
	WndClass.lpfnWndProc = (WNDPROC)WndProc;
	WndClass.cbClsExtra = 0;
	WndClass.cbWndExtra = 0;
	WndClass.hInstance = hInstance;
	WndClass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	WndClass.hCursor = LoadCursor(NULL, IDC_ARROW);
	WndClass.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
	WndClass.lpszMenuName = NULL;
	WndClass.lpszClassName = lpszClass;
	WndClass.hIconSm = LoadIcon(NULL, IDI_APPLICATION);

	RegisterClassEx(&WndClass);

	hWnd = CreateWindow(
		lpszClass,
		lpszWindowName,
		WS_OVERLAPPEDWINDOW,
		100,
		50,
		WINDOW_WIDTH,
		WINDOW_HEIGHT,
		NULL,
		(HMENU)NULL,
		hInstance,
		NULL
	);

	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);

	while (GetMessage(&Message, 0, 0, 0)) {
		TranslateMessage(&Message);
		DispatchMessage(&Message);
	}

	return (int)Message.wParam;
}

void MakeMenu(HWND hWnd)
{
	HMENU hMenu = CreateMenu();
	HMENU hSignalMenu = CreatePopupMenu();
	HMENU hGameMenu = CreatePopupMenu();

	AppendMenu(hSignalMenu, MF_STRING, MENU_H_BLUE_V_RED, L"H-blue, V-red");
	AppendMenu(hSignalMenu, MF_STRING, MENU_H_RED_V_BLUE, L"H-red, V-blue");

	AppendMenu(hGameMenu, MF_STRING, MENU_AUTO, L"Auto");
	AppendMenu(hGameMenu, MF_STRING, MENU_STOP, L"Stop");
	AppendMenu(hGameMenu, MF_STRING, MENU_QUIT, L"Quit");

	AppendMenu(hMenu, MF_POPUP, (UINT_PTR)hSignalMenu, L"Signal");
	AppendMenu(hMenu, MF_POPUP, (UINT_PTR)hGameMenu, L"Game");

	SetMenu(hWnd, hMenu);
}

void InitCars()
{
	cars[0].x = 50;
	cars[0].y = 280;
	cars[0].w = 55;
	cars[0].h = 28;
	cars[0].speed = 4;
	cars[0].dir = DIR_RIGHT;
	cars[0].color = RGB(255, 80, 80);

	cars[1].x = -170;
	cars[1].y = 320;
	cars[1].w = 55;
	cars[1].h = 28;
	cars[1].speed = 3;
	cars[1].dir = DIR_RIGHT;
	cars[1].color = RGB(255, 140, 80);

	cars[2].x = 780;
	cars[2].y = 390;
	cars[2].w = 55;
	cars[2].h = 28;
	cars[2].speed = 5;
	cars[2].dir = DIR_LEFT;
	cars[2].color = RGB(80, 160, 255);

	cars[3].x = 1020;
	cars[3].y = 430;
	cars[3].w = 55;
	cars[3].h = 28;
	cars[3].speed = 3;
	cars[3].dir = DIR_LEFT;
	cars[3].color = RGB(80, 220, 255);

	cars[4].x = 385;
	cars[4].y = 30;
	cars[4].w = 28;
	cars[4].h = 55;
	cars[4].speed = 4;
	cars[4].dir = DIR_DOWN;
	cars[4].color = RGB(120, 255, 120);

	cars[5].x = 425;
	cars[5].y = -210;
	cars[5].w = 28;
	cars[5].h = 55;
	cars[5].speed = 2;
	cars[5].dir = DIR_DOWN;
	cars[5].color = RGB(180, 255, 120);

	cars[6].x = 500;
	cars[6].y = 620;
	cars[6].w = 28;
	cars[6].h = 55;
	cars[6].speed = 5;
	cars[6].dir = DIR_UP;
	cars[6].color = RGB(180, 120, 255);

	cars[7].x = 535;
	cars[7].y = 860;
	cars[7].w = 28;
	cars[7].h = 55;
	cars[7].speed = 3;
	cars[7].dir = DIR_UP;
	cars[7].color = RGB(220, 120, 255);
}

void DrawRoad(HDC hDC)
{
	HBRUSH roadBrush = CreateSolidBrush(RGB(70, 70, 70));
	HBRUSH oldBrush = (HBRUSH)SelectObject(hDC, roadBrush);

	Rectangle(hDC, 0, roadTop, WINDOW_WIDTH, roadBottom);
	Rectangle(hDC, roadLeft, 0, roadRight, WINDOW_HEIGHT);

	SelectObject(hDC, oldBrush);
	DeleteObject(roadBrush);

	HPEN centerPen = CreatePen(PS_SOLID, 2, RGB(255, 255, 255));
	HPEN oldPen = (HPEN)SelectObject(hDC, centerPen);

	MoveToEx(hDC, 0, centerY, NULL);
	LineTo(hDC, roadLeft - stopLineGap, centerY);

	MoveToEx(hDC, roadRight + stopLineGap, centerY, NULL);
	LineTo(hDC, WINDOW_WIDTH, centerY);

	MoveToEx(hDC, centerX, 0, NULL);
	LineTo(hDC, centerX, roadTop - stopLineGap);

	MoveToEx(hDC, centerX, roadBottom + stopLineGap, NULL);
	LineTo(hDC, centerX, WINDOW_HEIGHT);

	SelectObject(hDC, oldPen);
	DeleteObject(centerPen);

	HPEN stopPen = CreatePen(PS_SOLID, 4, RGB(255, 255, 255));
	oldPen = (HPEN)SelectObject(hDC, stopPen);

	MoveToEx(hDC, roadLeft - stopLineGap, roadTop, NULL);
	LineTo(hDC, roadLeft - stopLineGap, roadBottom);

	MoveToEx(hDC, roadRight + stopLineGap, roadTop, NULL);
	LineTo(hDC, roadRight + stopLineGap, roadBottom);

	MoveToEx(hDC, roadLeft, roadTop - stopLineGap, NULL);
	LineTo(hDC, roadRight, roadTop - stopLineGap);

	MoveToEx(hDC, roadLeft, roadBottom + stopLineGap, NULL);
	LineTo(hDC, roadRight, roadBottom + stopLineGap);

	SelectObject(hDC, oldPen);
	DeleteObject(stopPen);
}

void DrawSignals(HDC hDC)
{
	COLORREF hColor;
	COLORREF vColor;

	if (hSignal == SIGNAL_RED) {
		hColor = RGB(255, 0, 0);
	}
	else if (hSignal == SIGNAL_BLUE) {
		hColor = RGB(0, 120, 255);
	}
	else {
		hColor = RGB(255, 220, 0);
	}

	if (vSignal == SIGNAL_RED) {
		vColor = RGB(255, 0, 0);
	}
	else if (vSignal == SIGNAL_BLUE) {
		vColor = RGB(0, 120, 255);
	}
	else {
		vColor = RGB(255, 220, 0);
	}

	HBRUSH hBrush = CreateSolidBrush(hColor);
	HBRUSH oldBrush = (HBRUSH)SelectObject(hDC, hBrush);

	Ellipse(hDC, hSignalRect.left, hSignalRect.top, hSignalRect.right, hSignalRect.bottom);

	SelectObject(hDC, oldBrush);
	DeleteObject(hBrush);

	HBRUSH vBrush = CreateSolidBrush(vColor);
	oldBrush = (HBRUSH)SelectObject(hDC, vBrush);

	Ellipse(hDC, vSignalRect.left, vSignalRect.top, vSignalRect.right, vSignalRect.bottom);

	SelectObject(hDC, oldBrush);
	DeleteObject(vBrush);

	TextOut(hDC, 125, 160, L"H Signal", lstrlen(L"H Signal"));
	TextOut(hDC, 215, 160, L"V Signal", lstrlen(L"V Signal"));

	TextOut(hDC, 600, 40, L"+ / - : speed up / down", lstrlen(L"+ / - : speed up / down"));
	TextOut(hDC, 600, 65, L"A : auto signal on/off", lstrlen(L"A : auto signal on/off"));
	TextOut(hDC, 600, 90, L"Q : quit", lstrlen(L"Q : quit"));
	TextOut(hDC, 600, 115, L"LButton : signal click or all stop", lstrlen(L"LButton : signal click or all stop"));
	TextOut(hDC, 600, 140, L"RButton : resume cars", lstrlen(L"RButton : resume cars"));
}

void DrawOneCar(HDC hDC, CAR car)
{
	HBRUSH carBrush = CreateSolidBrush(car.color);
	HBRUSH oldBrush = (HBRUSH)SelectObject(hDC, carBrush);

	Rectangle(hDC, car.x, car.y, car.x + car.w, car.y + car.h);

	if (car.dir == DIR_RIGHT && car.x + car.w > WINDOW_WIDTH) {
		Rectangle(hDC, car.x - WINDOW_WIDTH, car.y, car.x - WINDOW_WIDTH + car.w, car.y + car.h);
	}

	if (car.dir == DIR_LEFT && car.x < 0) {
		Rectangle(hDC, car.x + WINDOW_WIDTH, car.y, car.x + WINDOW_WIDTH + car.w, car.y + car.h);
	}

	if (car.dir == DIR_DOWN && car.y + car.h > WINDOW_HEIGHT) {
		Rectangle(hDC, car.x, car.y - WINDOW_HEIGHT, car.x + car.w, car.y - WINDOW_HEIGHT + car.h);
	}

	if (car.dir == DIR_UP && car.y < 0) {
		Rectangle(hDC, car.x, car.y + WINDOW_HEIGHT, car.x + car.w, car.y + WINDOW_HEIGHT + car.h);
	}

	SelectObject(hDC, oldBrush);
	DeleteObject(carBrush);
}

void DrawCars(HDC hDC)
{
	for (int i = 0; i < 8; i++) {
		DrawOneCar(hDC, cars[i]);
	}
}

void DrawPerson(HDC hDC)
{
	HBRUSH personBrush = CreateSolidBrush(RGB(0, 0, 0));
	HBRUSH oldBrush = (HBRUSH)SelectObject(hDC, personBrush);

	Ellipse(hDC, personX - 7, personY - 22, personX + 7, personY - 8);
	Rectangle(hDC, personX - 5, personY - 7, personX + 5, personY + 15);

	MoveToEx(hDC, personX - 5, personY + 15, NULL);
	LineTo(hDC, personX - 12, personY + 28);

	MoveToEx(hDC, personX + 5, personY + 15, NULL);
	LineTo(hDC, personX + 12, personY + 28);

	SelectObject(hDC, oldBrush);
	DeleteObject(personBrush);
}

int IsGreenSignalForCar(CAR car)
{
	if (car.dir == DIR_RIGHT || car.dir == DIR_LEFT) {
		if (hSignal == SIGNAL_BLUE) {
			return 1;
		}
	}

	if (car.dir == DIR_DOWN || car.dir == DIR_UP) {
		if (vSignal == SIGNAL_BLUE) {
			return 1;
		}
	}

	return 0;
}

int GetStopLine(CAR car)
{
	if (car.dir == DIR_RIGHT) {
		return roadLeft - stopLineGap - car.w;
	}

	if (car.dir == DIR_LEFT) {
		return roadRight + stopLineGap;
	}

	if (car.dir == DIR_DOWN) {
		return roadTop - stopLineGap - car.h;
	}

	if (car.dir == DIR_UP) {
		return roadBottom + stopLineGap;
	}

	return 0;
}

int IsCarAlreadyPassedStopLine(CAR car)
{
	int stopLine = GetStopLine(car);

	if (car.dir == DIR_RIGHT) {
		if (car.x > stopLine + 5) {
			return 1;
		}
	}

	if (car.dir == DIR_LEFT) {
		if (car.x < stopLine - 5) {
			return 1;
		}
	}

	if (car.dir == DIR_DOWN) {
		if (car.y > stopLine + 5) {
			return 1;
		}
	}

	if (car.dir == DIR_UP) {
		if (car.y < stopLine - 5) {
			return 1;
		}
	}

	return 0;
}

int CanMoveBySignalAndStopLine(CAR car)
{
	int stopLine = GetStopLine(car);

	if (globalStop == 1) {
		return 0;
	}

	if (IsGreenSignalForCar(car) == 1) {
		return 1;
	}

	if (IsCarAlreadyPassedStopLine(car) == 1) {
		return 1;
	}

	if (car.dir == DIR_RIGHT) {
		if (car.x < stopLine) {
			return 1;
		}
	}

	if (car.dir == DIR_LEFT) {
		if (car.x > stopLine) {
			return 1;
		}
	}

	if (car.dir == DIR_DOWN) {
		if (car.y < stopLine) {
			return 1;
		}
	}

	if (car.dir == DIR_UP) {
		if (car.y > stopLine) {
			return 1;
		}
	}

	return 0;
}

void MoveOneCar(int index)
{
	CAR car = cars[index];

	int moveSpeed = car.speed + speedPlus;

	if (moveSpeed < 1) {
		moveSpeed = 1;
	}

	int stopLine = GetStopLine(car);

	if (CanMoveBySignalAndStopLine(car) == 1) {
		if (car.dir == DIR_RIGHT) {
			cars[index].x += moveSpeed;

			if (IsGreenSignalForCar(car) == 0 && IsCarAlreadyPassedStopLine(car) == 0) {
				if (cars[index].x > stopLine) {
					cars[index].x = stopLine;
				}
			}
		}

		if (car.dir == DIR_LEFT) {
			cars[index].x -= moveSpeed;

			if (IsGreenSignalForCar(car) == 0 && IsCarAlreadyPassedStopLine(car) == 0) {
				if (cars[index].x < stopLine) {
					cars[index].x = stopLine;
				}
			}
		}

		if (car.dir == DIR_DOWN) {
			cars[index].y += moveSpeed;

			if (IsGreenSignalForCar(car) == 0 && IsCarAlreadyPassedStopLine(car) == 0) {
				if (cars[index].y > stopLine) {
					cars[index].y = stopLine;
				}
			}
		}

		if (car.dir == DIR_UP) {
			cars[index].y -= moveSpeed;

			if (IsGreenSignalForCar(car) == 0 && IsCarAlreadyPassedStopLine(car) == 0) {
				if (cars[index].y < stopLine) {
					cars[index].y = stopLine;
				}
			}
		}
	}

	if (cars[index].dir == DIR_RIGHT && cars[index].x > WINDOW_WIDTH) {
		cars[index].x = -cars[index].w;
	}

	if (cars[index].dir == DIR_LEFT && cars[index].x + cars[index].w < 0) {
		cars[index].x = WINDOW_WIDTH;
	}

	if (cars[index].dir == DIR_DOWN && cars[index].y > WINDOW_HEIGHT) {
		cars[index].y = -cars[index].h;
	}

	if (cars[index].dir == DIR_UP && cars[index].y + cars[index].h < 0) {
		cars[index].y = WINDOW_HEIGHT;
	}
}

void FixRearCarGap()
{
	if (cars[1].x + cars[1].w + stopGap > cars[0].x && cars[0].x > cars[1].x) {
		cars[1].x = cars[0].x - cars[1].w - stopGap;
	}

	if (cars[3].x < cars[2].x + cars[2].w + stopGap && cars[3].x > cars[2].x) {
		cars[3].x = cars[2].x + cars[2].w + stopGap;
	}

	if (cars[5].y + cars[5].h + stopGap > cars[4].y && cars[4].y > cars[5].y) {
		cars[5].y = cars[4].y - cars[5].h - stopGap;
	}

	if (cars[7].y < cars[6].y + cars[6].h + stopGap && cars[7].y > cars[6].y) {
		cars[7].y = cars[6].y + cars[6].h + stopGap;
	}
}

void MoveCars()
{
	for (int i = 0; i < 8; i++) {
		MoveOneCar(i);
	}

	FixRearCarGap();
}

void StartPersonMove()
{
	if (personCorner == 0) {
		personState = PERSON_CROSS_RIGHT_SIDE;
		personX = roadRight + 45;
		personY = roadTop - 90;
	}
	else if (personCorner == 1) {
		personState = PERSON_CROSS_BOTTOM_SIDE;
		personX = roadRight + 90;
		personY = roadBottom + 45;
	}
	else if (personCorner == 2) {
		personState = PERSON_CROSS_LEFT_SIDE;
		personX = roadLeft - 45;
		personY = roadBottom + 90;
	}
	else if (personCorner == 3) {
		personState = PERSON_CROSS_TOP_SIDE;
		personX = roadLeft - 90;
		personY = roadTop - 45;
	}
}

void StartPersonDiagonalMove()
{
	personState = PERSON_CROSS_DIAGONAL;

	if (personCorner == 0) {
		personTargetX = roadLeft - 45;
		personTargetY = roadBottom + 90;
		personDiagonalCorner = 2;
	}
	else if (personCorner == 1) {
		personTargetX = roadLeft - 90;
		personTargetY = roadTop - 45;
		personDiagonalCorner = 3;
	}
	else if (personCorner == 2) {
		personTargetX = roadRight + 45;
		personTargetY = roadTop - 90;
		personDiagonalCorner = 0;
	}
	else if (personCorner == 3) {
		personTargetX = roadRight + 90;
		personTargetY = roadBottom + 45;
		personDiagonalCorner = 1;
	}
}

void MovePerson()
{
	if (personState == PERSON_WAIT && globalStop == 0) {
		if ((personCorner == 0 || personCorner == 2) && hSignal == SIGNAL_RED && lastPersonSignal != 0) {
			StartPersonMove();
			lastPersonSignal = 0;
		}
		else if ((personCorner == 1 || personCorner == 3) && vSignal == SIGNAL_RED && lastPersonSignal != 1) {
			StartPersonMove();
			lastPersonSignal = 1;
		}
	}

	if (personState == PERSON_CROSS_RIGHT_SIDE) {
		personY += 3;

		if (personY >= roadBottom + 90) {
			personY = roadBottom + 90;
			personX = roadRight + 45;

			personCorner = 1;
			personState = PERSON_WAIT;
		}
	}
	else if (personState == PERSON_CROSS_BOTTOM_SIDE) {
		personX -= 3;

		if (personX <= roadLeft - 90) {
			personX = roadLeft - 90;
			personY = roadBottom + 45;

			personCorner = 2;
			personState = PERSON_WAIT;
		}
	}
	else if (personState == PERSON_CROSS_LEFT_SIDE) {
		personY -= 3;

		if (personY <= roadTop - 90) {
			personY = roadTop - 90;
			personX = roadLeft - 45;

			personCorner = 3;
			personState = PERSON_WAIT;
		}
	}
	else if (personState == PERSON_CROSS_TOP_SIDE) {
		personX += 3;

		if (personX >= roadRight + 90) {
			personX = roadRight + 90;
			personY = roadTop - 45;

			personCorner = 0;
			personState = PERSON_WAIT;
		}
	}
	else if (personState == PERSON_CROSS_DIAGONAL) {
		if (personX < personTargetX) {
			personX += 3;

			if (personX > personTargetX) {
				personX = personTargetX;
			}
		}
		else if (personX > personTargetX) {
			personX -= 3;

			if (personX < personTargetX) {
				personX = personTargetX;
			}
		}

		if (personY < personTargetY) {
			personY += 3;

			if (personY > personTargetY) {
				personY = personTargetY;
			}
		}
		else if (personY > personTargetY) {
			personY -= 3;

			if (personY < personTargetY) {
				personY = personTargetY;
			}
		}

		if (personX == personTargetX && personY == personTargetY) {
			personCorner = personDiagonalCorner;
			personState = PERSON_WAIT;
		}
	}

	if (hSignal == SIGNAL_YELLOW || vSignal == SIGNAL_YELLOW) {
		lastPersonSignal = -1;
	}
}

void SetHBlueVRed()
{
	hSignal = SIGNAL_BLUE;
	vSignal = SIGNAL_RED;
	yellowTarget = 0;
}

void SetHRedVBlue()
{
	hSignal = SIGNAL_RED;
	vSignal = SIGNAL_BLUE;
	yellowTarget = 0;
}

void StartYellow(HWND hWnd, int target)
{
	if (hSignal == SIGNAL_YELLOW || vSignal == SIGNAL_YELLOW) {
		return;
	}

	beforeHSignal = hSignal;
	beforeVSignal = vSignal;

	yellowTarget = target;

	hSignal = SIGNAL_YELLOW;
	vSignal = SIGNAL_YELLOW;

	SetTimer(hWnd, TIMER_YELLOW, 1000, NULL);
}

void FinishYellow()
{
	if (yellowTarget == 1) {
		if (beforeHSignal == SIGNAL_RED) {
			SetHBlueVRed();
		}
		else {
			SetHRedVBlue();
		}
	}

	if (yellowTarget == 2) {
		if (beforeVSignal == SIGNAL_RED) {
			SetHRedVBlue();
		}
		else {
			SetHBlueVRed();
		}
	}

	if (yellowTarget == 3) {
		if (beforeHSignal == SIGNAL_BLUE) {
			SetHRedVBlue();
		}
		else {
			SetHBlueVRed();
		}
	}

	yellowTarget = 0;
}

int IsInsideRect(int mx, int my, RECT rc)
{
	if (mx >= rc.left && mx <= rc.right && my >= rc.top && my <= rc.bottom) {
		return 1;
	}

	return 0;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	PAINTSTRUCT ps;
	HDC hDC;
	int mx;
	int my;

	switch (uMsg) {
	case WM_CREATE:
		srand((unsigned int)time(NULL));

		MakeMenu(hWnd);

		InitCars();

		SetTimer(hWnd, TIMER_MOVE, 30, NULL);

		break;

	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case MENU_H_BLUE_V_RED:
			globalStop = 0;
			SetHBlueVRed();
			InvalidateRect(hWnd, NULL, TRUE);
			break;

		case MENU_H_RED_V_BLUE:
			globalStop = 0;
			SetHRedVBlue();
			InvalidateRect(hWnd, NULL, TRUE);
			break;

		case MENU_AUTO:
			autoMode = 1;
			SetTimer(hWnd, TIMER_AUTO, 3000, NULL);
			break;

		case MENU_STOP:
			autoMode = 0;
			KillTimer(hWnd, TIMER_AUTO);
			break;

		case MENU_QUIT:
			DestroyWindow(hWnd);
			break;
		}

		break;

	case WM_TIMER:
		if (wParam == TIMER_MOVE) {
			MoveCars();
			MovePerson();
			InvalidateRect(hWnd, NULL, TRUE);
		}

		if (wParam == TIMER_YELLOW) {
			KillTimer(hWnd, TIMER_YELLOW);
			FinishYellow();
			InvalidateRect(hWnd, NULL, TRUE);
		}

		if (wParam == TIMER_AUTO) {
			if (autoMode == 1 && hSignal != SIGNAL_YELLOW && vSignal != SIGNAL_YELLOW) {
				StartYellow(hWnd, 3);
			}
		}

		break;

	case WM_LBUTTONDOWN:
		mx = LOWORD(lParam);
		my = HIWORD(lParam);

		if (IsInsideRect(mx, my, hSignalRect) == 1) {
			globalStop = 0;
			StartYellow(hWnd, 1);
		}
		else if (IsInsideRect(mx, my, vSignalRect) == 1) {
			globalStop = 0;
			StartYellow(hWnd, 2);
		}
		else {
			globalStop = 1;
			StartPersonDiagonalMove();
		}

		InvalidateRect(hWnd, NULL, TRUE);

		break;

	case WM_RBUTTONDOWN:
		globalStop = 0;
		SetHBlueVRed();

		InvalidateRect(hWnd, NULL, TRUE);

		break;

	case WM_KEYDOWN:
		if (wParam == VK_OEM_PLUS || wParam == VK_ADD) {
			speedPlus++;
		}

		if (wParam == VK_OEM_MINUS || wParam == VK_SUBTRACT) {
			speedPlus--;

			if (speedPlus < -2) {
				speedPlus = -2;
			}
		}

		if (wParam == 'A') {
			if (autoMode == 0) {
				autoMode = 1;
				SetTimer(hWnd, TIMER_AUTO, 3000, NULL);
			}
			else {
				autoMode = 0;
				KillTimer(hWnd, TIMER_AUTO);
			}
		}

		if (wParam == 'Q') {
			DestroyWindow(hWnd);
		}

		InvalidateRect(hWnd, NULL, TRUE);

		break;

	case WM_PAINT:
		hDC = BeginPaint(hWnd, &ps);

		DrawRoad(hDC);
		DrawSignals(hDC);
		DrawCars(hDC);
		DrawPerson(hDC);

		EndPaint(hWnd, &ps);

		break;

	case WM_DESTROY:
		KillTimer(hWnd, TIMER_MOVE);
		KillTimer(hWnd, TIMER_YELLOW);
		KillTimer(hWnd, TIMER_AUTO);

		PostQuitMessage(0);

		break;
	}

	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}