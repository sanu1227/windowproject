#include <windows.h> //--- 윈도우 헤더 파일
#include <tchar.h>
#include <time.h>
#include <atlimage.h>

HINSTANCE g_hInst;
LPCTSTR lpszClass = L"My Window Class";
LPCTSTR lpszWindowName = L"Window Programming Lab";
LRESULT CALLBACK WndProc(HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam);

#define cellsize 100
#define boardsize 6
#define block 2
#define b_num2 2

int startx = 100;
int starty = 100;

int cells[boardsize * boardsize];


CImage num2;
CImage num4;
CImage num8;
CImage num16;
CImage num32;
CImage num64;

void drawboard(HDC hDC) {
	for (int i = 0; i <= boardsize; ++i) {
		MoveToEx(hDC, startx, starty + i * cellsize, NULL);
		LineTo(hDC, startx + boardsize * cellsize, starty + i * cellsize);
	}
	for (int i = 0; i <= boardsize; ++i) {
		MoveToEx(hDC, startx + i * cellsize, starty, NULL);
		LineTo(hDC, startx + i * cellsize, starty + boardsize * cellsize);
	}
}

void drawblock(HDC hDC) {

	HBRUSH hBrush = CreateSolidBrush(RGB(255, 0, 0)); // 빨간색 블록
	HBRUSH oldBrush = (HBRUSH)SelectObject(hDC, hBrush);
	
	for (int i = 0; i < block; i++) {
		int cellNumber = cells[i];

		int randx = cellNumber % boardsize;
		int randy = cellNumber / boardsize;

		Rectangle(
			hDC,
			startx + randx * cellsize,
			starty + randy * cellsize,
			startx + (randx + 1) * cellsize,
			starty + (randy + 1) * cellsize
		);
	}

	SelectObject(hDC, oldBrush);
	DeleteObject(hBrush);
}

void makeRandomBlocks() {
	// 1. 보드판 전체 칸 번호 저장
	for (int i = 0; i < boardsize * boardsize; i++) {
		cells[i] = i;
	}

	// 2. 배열 섞기
	for (int i = boardsize * boardsize - 1; i > 0; i--) {
		int r = rand() % (i + 1);

		int temp = cells[i];
		cells[i] = cells[r];
		cells[r] = temp;
	}
}

void drawNum2(HDC hDC) {
	num2.Load(L"5-4/2.png");

	for (int i = block; i < block + b_num2; ++i) {
		int cellNumber = cells[i];          // 선택된 칸 번호

		int cellx = cellNumber % boardsize; // 몇 번째 열인지
		int celly = cellNumber / boardsize; // 몇 번째 행인지

		int left = startx + cellx * cellsize;       // 셀의 좌상단 x
		int top = starty + celly * cellsize;       // 셀의 좌상단 y
		int right = left + cellsize;                 // 셀의 우하단 x
		int bottom = top + cellsize;                  // 셀의 우하단 y

		RECT Rectnum2 = { left, top, right, bottom };
		num2.Draw(hDC, Rectnum2);
	}
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpszCmdParam, int nCmdShow)
{
	HWND hWnd;
	MSG Message;
	WNDCLASSEX WndClass;
	g_hInst = hInstance;
	WndClass.cbSize = sizeof(WndClass);
	WndClass.style = CS_HREDRAW | CS_VREDRAW;
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
	hWnd = CreateWindow(lpszClass, lpszWindowName, WS_OVERLAPPEDWINDOW, 0, 0, 1600, 900, NULL, (HMENU)NULL, hInstance, NULL);
	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);
	while (GetMessage(&Message, 0, 0, 0)) {
		TranslateMessage(&Message);
		DispatchMessage(&Message);
	}
	return Message.wParam;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	PAINTSTRUCT ps;
	HDC hDC, mDC;
	RECT rt;
	HBITMAP hBitmap;


	//--- 메시지 처리하기
	switch (uMsg) {
	case WM_CREATE:
		srand((unsigned int)time(NULL));
		makeRandomBlocks();
		break;
	case WM_PAINT:
		GetClientRect(hWnd, &rt);
		hDC = BeginPaint(hWnd, &ps);
		mDC = CreateCompatibleDC(hDC); //--- 메모리 DC 만들기
		hBitmap = CreateCompatibleBitmap(hDC, rt.right, rt.bottom); //--- 메모리 DC와 연결할 비트맵 만들기
		SelectObject(mDC, (HBITMAP)hBitmap);
		FillRect(mDC, &rt, (HBRUSH)GetStockObject(WHITE_BRUSH));

		drawboard(mDC);
		drawblock(mDC);
		drawNum2(mDC);

		BitBlt(hDC, 0, 0, rt.right, rt.bottom, mDC, 0, 0, SRCCOPY);

		DeleteDC(mDC); //--- 생성한 메모리 DC 삭제
		DeleteObject(hBitmap);
		EndPaint(hWnd, &ps);
		break;
	case WM_TIMER:
		InvalidateRect(hWnd, NULL, false);
		break;

	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	}
	return DefWindowProc(hWnd, uMsg, wParam, lParam); //--- 위의 세 메시지 외의 나머지 메시지는 OS로
}
