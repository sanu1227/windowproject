#include <windows.h> //--- 윈도우 헤더 파일
#include <tchar.h>
#include <time.h>
#include <atlimage.h>
#include <stdlib.h>
#include "resource.h"

HINSTANCE g_hInst;
LPCTSTR lpszClass = L"My Window Class";
LPCTSTR lpszWindowName = L"Window Programming Lab";
LRESULT CALLBACK WndProc(HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam);

#define cellsize 100
#define boardsize 6
#define b_num2 2

#define DIR_NONE  0
#define DIR_LEFT  1
#define DIR_RIGHT 2
#define DIR_UP    3
#define DIR_DOWN  4

int dragStartX = 0;
int dragStartY = 0;
int dragEndX = 0;
int dragEndY = 0;

int slideDir = DIR_NONE;

int block = 2;
int startx = 100;
int starty = 100;
bool isStart = false;
int cells[boardsize * boardsize];
int board[boardsize][boardsize];

CImage num2;
CImage num4;
CImage num8;
CImage num16;
CImage num32;
CImage num64;

void intboard();
void drawboard(HDC hDC);
void drawblock(HDC hDC);
void makeRandomBlocks();
void drawNum2(HDC hDC);
void DragDirection();

void intboard() {
	for (int y = 0; y < boardsize; ++y) {
		for (int x = 0; x < boardsize; ++x) {
			board[y][x] = 0;
		}
	}
	makeRandomBlocks();

	// 장애물 배치
	for (int i = 0; i < block; i++) {
		int cellNumber = cells[i];

		int x = cellNumber % boardsize;
		int y = cellNumber / boardsize;

		board[y][x] = -1;
	}
	// 숫자 2 배치
	for (int i = block; i < block + b_num2; i++) {
		int cellNumber = cells[i];

		int x = cellNumber % boardsize;
		int y = cellNumber / boardsize;

		board[y][x] = 2;
	}

}

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
	
	for (int y = 0; y < boardsize; y++) {
		for (int x = 0; x < boardsize; x++) {

			// board 값이 -1이면 장애물
			if (board[y][x] == -1) {
				Rectangle(
					hDC,
					startx + x * cellsize,
					starty + y * cellsize,
					startx + (x + 1) * cellsize,
					starty + (y + 1) * cellsize
				);
			}
		}
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

	for (int y = 0; y < boardsize; y++) {
		for (int x = 0; x < boardsize; x++) {

			// board 값이 2이면 숫자 2 이미지 그림
			if (board[y][x] == 2) {
				int left = startx + x * cellsize;
				int top = starty + y * cellsize;
				int right = left + cellsize;
				int bottom = top + cellsize;

				RECT Rectnum2 = { left, top, right, bottom };
				num2.Draw(hDC, Rectnum2);
			}
		}
	}
}

void DragDirection() {
	int dx = dragEndX - dragStartX;
	int dy = dragEndY - dragStartY;

	if (abs(dx) > abs(dy)) {
		if (dx > 0) {
			slideDir = DIR_RIGHT;
		}
		else if (dx < 0) {
			slideDir = DIR_LEFT;
		}
	}
	else if (abs(dx) < abs(dy)) {
		if (dy > 0)
			slideDir = DIR_DOWN;
		else if (dy < 0)
			slideDir = DIR_UP;
	}
	else {
		slideDir = DIR_NONE;
	}

}

bool moveNumbersOneStep(int dir) {
	bool isMoved = false;

	if (dir == DIR_RIGHT) {
		for (int y = 0; y < boardsize; ++y) {
			for (int x = boardsize - 2; x >= 0; --x) {

				if (board[y][x] > 0 && board[y][x + 1] == 0) {
					board[y][x + 1] = board[y][x];
					board[y][x] = 0;
					isMoved = true;
				}
			}
		}
	}

	if (dir == DIR_LEFT) {
		for (int y = 0; y < boardsize; ++y) {
			for (int x = 1; x < boardsize; ++x) {
	
				if (board[y][x] > 0 && board[y][x - 1] == 0) {
					board[y][x - 1] = board[y][x];
					board[y][x] = 0;
					isMoved = true;
				}
			}
		}
	}

	if (dir == DIR_UP) {
		for (int y = 1; y < boardsize; ++y) {
			for (int x = 0; x < boardsize; ++x) {

				if (board[y][x] > 0 && board[y - 1][x] == 0) {
					board[y - 1][x] = board[y][x];
					board[y][x] = 0;
					isMoved = true;
				}
			}
		}
	}

	if (dir == DIR_DOWN) {
		for (int y = boardsize - 2; y >= 0; --y) {
			for (int x = 0; x < boardsize; ++x) {

				if (board[y][x] > 0 && board[y + 1][x] == 0) {
					board[y + 1][x] = board[y][x];
					board[y][x] = 0;
					isMoved = true;
				}
			}
		}
	}

	return isMoved;
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
	WndClass.lpszMenuName = MAKEINTRESOURCE(IDR_MENU1);
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
		num2.Load(L"5-4/2.png");
		num4.Load(L"5-4/4.png");
		num8.Load(L"5-4/8.png");
		num16.Load(L"5-4/16.png");
		num32.Load(L"5-4/32.png");
		num64.Load(L"5-4/64.png");
		srand((unsigned int)time(NULL));
		intboard();
		break;
	case WM_PAINT:
		GetClientRect(hWnd, &rt);
		hDC = BeginPaint(hWnd, &ps);
		mDC = CreateCompatibleDC(hDC); //--- 메모리 DC 만들기
		hBitmap = CreateCompatibleBitmap(hDC, rt.right, rt.bottom); //--- 메모리 DC와 연결할 비트맵 만들기
		SelectObject(mDC, (HBITMAP)hBitmap);
		FillRect(mDC, &rt, (HBRUSH)GetStockObject(WHITE_BRUSH));

		drawboard(mDC);
		if (isStart) {
			drawblock(mDC);
			drawNum2(mDC);
		}
		BitBlt(hDC, 0, 0, rt.right, rt.bottom, mDC, 0, 0, SRCCOPY);

		DeleteDC(mDC); //--- 생성한 메모리 DC 삭제
		DeleteObject(hBitmap);
		EndPaint(hWnd, &ps);
		break;
	case WM_TIMER:
		InvalidateRect(hWnd, NULL, FALSE);
		break;
	case WM_LBUTTONDOWN:
		if (isStart) {
			dragStartX = LOWORD(lParam);
			dragStartY = HIWORD(lParam);
		}
		break;
	case WM_LBUTTONUP:
		if (isStart) {
			dragEndX = LOWORD(lParam);
			dragEndY = HIWORD(lParam);

			DragDirection();

			if (slideDir != DIR_NONE) {
				moveNumbersOneStep(slideDir);   // 드래그 방향으로 딱 한 칸 이동
				InvalidateRect(hWnd, NULL, TRUE);
			}

			slideDir = DIR_NONE;
		}
		break;
	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case ID_GAME_START:
			isStart = true;
			InvalidateRect(hWnd, NULL, TRUE);
			break;
		case ID_GAME_END:
			isStart = false;
			InvalidateRect(hWnd, NULL, TRUE);
			break;
		case ID_BLOCK_2:
			block = 2;
			intboard();
			InvalidateRect(hWnd, NULL, TRUE);
			break;
		case ID_BLOCK_3:
			block = 3;
			intboard();
			InvalidateRect(hWnd, NULL, TRUE);
			break;
		case ID_BLOCK_4:
			block = 4;
			intboard();
			InvalidateRect(hWnd, NULL, TRUE);
			break;
		}
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	}
	return DefWindowProc(hWnd, uMsg, wParam, lParam); //--- 위의 세 메시지 외의 나머지 메시지는 OS로
}
