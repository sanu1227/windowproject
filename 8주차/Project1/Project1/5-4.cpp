#include <windows.h>     // 윈도우 API 사용
#include <tchar.h>       // 유니코드 문자열 사용
#include <time.h>        // time 함수 사용
#include <atlimage.h>    // CImage 사용
#include <stdlib.h>      // rand, srand, abs 사용
#include "resource.h"    // 메뉴 리소스 ID 사용

HINSTANCE g_hInst;       // 프로그램 인스턴스 핸들
LPCTSTR lpszClass = L"My Window Class";
LPCTSTR lpszWindowName = L"Window Programming Lab";

LRESULT CALLBACK WndProc(HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam);


// ------------------------------------------------------------
// 기본 설정값
// ------------------------------------------------------------
#define cellsize 100          // 한 칸의 가로/세로 크기
#define boardsize 6           // 6 x 6 보드
#define b_num2 2              // 게임 시작 시 숫자 2의 초기 개수

#define max_num_count 12      // 화면에 존재할 수 있는 숫자 블록의 최대 개수
#define max_num_value 64      // 현재 이미지가 준비된 최대 숫자


// ------------------------------------------------------------
// 드래그 방향 상수
// ------------------------------------------------------------
#define DIR_NONE  0
#define DIR_LEFT  1
#define DIR_RIGHT 2
#define DIR_UP    3
#define DIR_DOWN  4


// ------------------------------------------------------------
// 마우스 드래그 위치 저장
// ------------------------------------------------------------
int dragStartX = 0;       // 마우스를 처음 누른 x
int dragStartY = 0;       // 마우스를 처음 누른 y
int dragEndX = 0;         // 마우스를 뗀 x
int dragEndY = 0;         // 마우스를 뗀 y

int slideDir = DIR_NONE;  // 현재 드래그 방향


// ------------------------------------------------------------
// 게임 상태 변수
// ------------------------------------------------------------
int block = 2;            // 장애물 개수
int startx = 100;         // 보드 시작 x 좌표
int starty = 100;         // 보드 시작 y 좌표
bool isStart = false;     // 게임 시작 여부
int winNumber = 64;       // 기본 승리 조건은 64

int cells[boardsize * boardsize];       // 보드 칸 번호를 섞기 위한 배열
int board[boardsize][boardsize];        // 실제 보드 상태 저장 배열

// 이번 드래그에서 합쳐져 새로 만들어진 숫자 위치 저장
// true인 칸의 숫자는 그 턴에는 이동하지 않음
bool mergedCell[boardsize][boardsize];

/*
board 값의 의미
0  : 빈칸
-1 : 장애물
2  : 숫자 2
4  : 숫자 4
8  : 숫자 8
16 : 숫자 16
32 : 숫자 32
64 : 숫자 64
*/


// ------------------------------------------------------------
// 숫자 이미지
// ------------------------------------------------------------
CImage num2;
CImage num4;
CImage num8;
CImage num16;
CImage num32;
CImage num64;


// ------------------------------------------------------------
// 함수 선언
// ------------------------------------------------------------
void initBoard();
void drawBoard(HDC hDC);
void drawBlock(HDC hDC);
void makeRandomCells();
void drawNumbers(HDC hDC);
void checkDragDirection();

void clearMergedCells();

bool mergeNumbers(int dir);
bool moveNumbersOneStep(int dir);
bool createNewNum2();

bool checkWin();
bool canDoAnyAction();

void winGame(HWND hWnd);
void loseGame(HWND hWnd);


// ------------------------------------------------------------
// 보드 초기화
// ------------------------------------------------------------
void initBoard() {
	// 모든 보드 칸을 빈칸으로 초기화
	for (int y = 0; y < boardsize; ++y) {
		for (int x = 0; x < boardsize; ++x) {
			board[y][x] = 0;
		}
	}

	// 합쳐진 칸 기록도 초기화
	clearMergedCells();

	// 전체 칸 번호를 랜덤하게 섞음
	makeRandomCells();

	// 장애물 배치
	for (int i = 0; i < block; i++) {
		int cellNumber = cells[i];

		int x = cellNumber % boardsize;
		int y = cellNumber / boardsize;

		board[y][x] = -1;
	}

	// 숫자 2 초기 배치
	for (int i = block; i < block + b_num2; i++) {
		int cellNumber = cells[i];

		int x = cellNumber % boardsize;
		int y = cellNumber / boardsize;

		board[y][x] = 2;
	}
}


// ------------------------------------------------------------
// 보드 격자 그리기
// ------------------------------------------------------------
void drawBoard(HDC hDC) {
	// 가로선 그리기
	for (int i = 0; i <= boardsize; ++i) {
		MoveToEx(hDC, startx, starty + i * cellsize, NULL);
		LineTo(hDC, startx + boardsize * cellsize, starty + i * cellsize);
	}

	// 세로선 그리기
	for (int i = 0; i <= boardsize; ++i) {
		MoveToEx(hDC, startx + i * cellsize, starty, NULL);
		LineTo(hDC, startx + i * cellsize, starty + boardsize * cellsize);
	}
}


// ------------------------------------------------------------
// 장애물 그리기
// ------------------------------------------------------------
void drawBlock(HDC hDC) {
	HBRUSH hBrush = CreateSolidBrush(RGB(255, 0, 0));    // 장애물 색상: 빨강
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


// ------------------------------------------------------------
// 전체 칸 번호를 랜덤하게 섞기
// ------------------------------------------------------------
void makeRandomCells() {
	// 0 ~ 35까지 칸 번호 저장
	for (int i = 0; i < boardsize * boardsize; i++) {
		cells[i] = i;
	}

	// 피셔-예이츠 셔플 방식으로 섞기
	for (int i = boardsize * boardsize - 1; i > 0; i--) {
		int r = rand() % (i + 1);

		int temp = cells[i];
		cells[i] = cells[r];
		cells[r] = temp;
	}
}


// ------------------------------------------------------------
// 숫자 이미지 그리기
// ------------------------------------------------------------
void drawNumbers(HDC hDC) {
	for (int y = 0; y < boardsize; y++) {
		for (int x = 0; x < boardsize; x++) {

			// 숫자 블록이 있는 칸만 처리
			if (board[y][x] > 0) {
				int left = startx + x * cellsize;
				int top = starty + y * cellsize;
				int right = left + cellsize;
				int bottom = top + cellsize;

				RECT rectNumber = { left, top, right, bottom };

				// 해당 숫자에 맞는 이미지 출력
				if (board[y][x] == 2)
					num2.Draw(hDC, rectNumber);

				else if (board[y][x] == 4)
					num4.Draw(hDC, rectNumber);

				else if (board[y][x] == 8)
					num8.Draw(hDC, rectNumber);

				else if (board[y][x] == 16)
					num16.Draw(hDC, rectNumber);

				else if (board[y][x] == 32)
					num32.Draw(hDC, rectNumber);

				else if (board[y][x] == 64)
					num64.Draw(hDC, rectNumber);
			}
		}
	}
}


// ------------------------------------------------------------
// 드래그 방향 판별
// ------------------------------------------------------------
void checkDragDirection() {
	int dx = dragEndX - dragStartX;
	int dy = dragEndY - dragStartY;

	// 가로 이동량이 더 크면 좌/우 판정
	if (abs(dx) > abs(dy)) {
		if (dx > 0)
			slideDir = DIR_RIGHT;
		else if (dx < 0)
			slideDir = DIR_LEFT;
		else
			slideDir = DIR_NONE;
	}

	// 세로 이동량이 더 크면 상/하 판정
	else if (abs(dx) < abs(dy)) {
		if (dy > 0)
			slideDir = DIR_DOWN;
		else if (dy < 0)
			slideDir = DIR_UP;
		else
			slideDir = DIR_NONE;
	}

	// 가로와 세로 이동량이 같으면 무시
	else {
		slideDir = DIR_NONE;
	}
}


// ------------------------------------------------------------
// 이번 드래그에서 합쳐진 위치 기록 초기화
// ------------------------------------------------------------
void clearMergedCells() {
	for (int y = 0; y < boardsize; y++) {
		for (int x = 0; x < boardsize; x++) {
			mergedCell[y][x] = false;
		}
	}
}


// ------------------------------------------------------------
// 같은 숫자끼리 먼저 합치기
// 합쳐진 경우 true 반환
// 합쳐져 새로 만들어진 숫자 위치는 mergedCell에 기록
// ------------------------------------------------------------
bool mergeNumbers(int dir) {
	bool isMerged = false;

	// --------------------------------------------------------
	// 오른쪽 방향 합치기
	// 오른쪽부터 왼쪽으로 검사
	// --------------------------------------------------------
	if (dir == DIR_RIGHT) {
		for (int y = 0; y < boardsize; ++y) {
			for (int x = boardsize - 2; x >= 0; --x) {

				if (
					board[y][x] > 0 &&
					board[y][x] == board[y][x + 1] &&
					board[y][x] < max_num_value
					) {
					board[y][x + 1] *= 2;
					board[y][x] = 0;

					// 합쳐져서 새로 생긴 위치 기록
					mergedCell[y][x + 1] = true;

					isMerged = true;
				}
			}
		}
	}

	// --------------------------------------------------------
	// 왼쪽 방향 합치기
	// 왼쪽부터 오른쪽으로 검사
	// --------------------------------------------------------
	else if (dir == DIR_LEFT) {
		for (int y = 0; y < boardsize; ++y) {
			for (int x = 1; x < boardsize; ++x) {

				if (
					board[y][x] > 0 &&
					board[y][x] == board[y][x - 1] &&
					board[y][x] < max_num_value
					) {
					board[y][x - 1] *= 2;
					board[y][x] = 0;

					// 합쳐져서 새로 생긴 위치 기록
					mergedCell[y][x - 1] = true;

					isMerged = true;
				}
			}
		}
	}

	// --------------------------------------------------------
	// 위쪽 방향 합치기
	// 위에서 아래로 검사
	// --------------------------------------------------------
	else if (dir == DIR_UP) {
		for (int x = 0; x < boardsize; ++x) {
			for (int y = 1; y < boardsize; ++y) {

				if (
					board[y][x] > 0 &&
					board[y][x] == board[y - 1][x] &&
					board[y][x] < max_num_value
					) {
					board[y - 1][x] *= 2;
					board[y][x] = 0;

					// 합쳐져서 새로 생긴 위치 기록
					mergedCell[y - 1][x] = true;

					isMerged = true;
				}
			}
		}
	}

	// --------------------------------------------------------
	// 아래쪽 방향 합치기
	// 아래에서 위로 검사
	// --------------------------------------------------------
	else if (dir == DIR_DOWN) {
		for (int x = 0; x < boardsize; ++x) {
			for (int y = boardsize - 2; y >= 0; --y) {

				if (
					board[y][x] > 0 &&
					board[y][x] == board[y + 1][x] &&
					board[y][x] < max_num_value
					) {
					board[y + 1][x] *= 2;
					board[y][x] = 0;

					// 합쳐져서 새로 생긴 위치 기록
					mergedCell[y + 1][x] = true;

					isMerged = true;
				}
			}
		}
	}

	return isMerged;
}


// ------------------------------------------------------------
// 숫자 블록을 한 칸 이동
// 단, 이번 드래그에서 합쳐져 새로 만들어진 숫자는 이동하지 않음
// 이동이 발생하면 true 반환
// ------------------------------------------------------------
bool moveNumbersOneStep(int dir) {
	bool isMoved = false;

	// 오른쪽 이동
	if (dir == DIR_RIGHT) {
		for (int y = 0; y < boardsize; ++y) {
			for (int x = boardsize - 2; x >= 0; --x) {

				if (
					board[y][x] > 0 &&
					mergedCell[y][x] == false &&
					board[y][x + 1] == 0
					) {
					board[y][x + 1] = board[y][x];
					board[y][x] = 0;
					isMoved = true;
				}
			}
		}
	}

	// 왼쪽 이동
	else if (dir == DIR_LEFT) {
		for (int y = 0; y < boardsize; ++y) {
			for (int x = 1; x < boardsize; ++x) {

				if (
					board[y][x] > 0 &&
					mergedCell[y][x] == false &&
					board[y][x - 1] == 0
					) {
					board[y][x - 1] = board[y][x];
					board[y][x] = 0;
					isMoved = true;
				}
			}
		}
	}

	// 위쪽 이동
	else if (dir == DIR_UP) {
		for (int x = 0; x < boardsize; ++x) {
			for (int y = 1; y < boardsize; ++y) {

				if (
					board[y][x] > 0 &&
					mergedCell[y][x] == false &&
					board[y - 1][x] == 0
					) {
					board[y - 1][x] = board[y][x];
					board[y][x] = 0;
					isMoved = true;
				}
			}
		}
	}

	// 아래쪽 이동
	else if (dir == DIR_DOWN) {
		for (int x = 0; x < boardsize; ++x) {
			for (int y = boardsize - 2; y >= 0; --y) {

				if (
					board[y][x] > 0 &&
					mergedCell[y][x] == false &&
					board[y + 1][x] == 0
					) {
					board[y + 1][x] = board[y][x];
					board[y][x] = 0;
					isMoved = true;
				}
			}
		}
	}

	return isMoved;
}


// ------------------------------------------------------------
// 빈칸에 숫자 2 하나 생성
// 생성 성공 시 true 반환
// ------------------------------------------------------------
bool createNewNum2() {
	int emptyCells[boardsize * boardsize];   // 빈칸 번호 저장 배열
	int emptyCount = 0;                      // 빈칸 개수
	int numberCount = 0;                     // 현재 숫자 블록 개수

	// 빈칸 수와 숫자 블록 개수 확인
	for (int y = 0; y < boardsize; y++) {
		for (int x = 0; x < boardsize; x++) {

			// 빈칸이면 저장
			if (board[y][x] == 0) {
				emptyCells[emptyCount] = y * boardsize + x;
				emptyCount++;
			}

			// 숫자 블록이면 개수 증가
			else if (board[y][x] > 0) {
				numberCount++;
			}
		}
	}

	// 숫자 블록이 최대 개수에 도달했으면 생성하지 않음
	if (numberCount >= max_num_count) {
		return false;
	}

	// 빈칸이 없으면 생성하지 않음
	if (emptyCount == 0) {
		return false;
	}

	// 빈칸 중 하나를 랜덤 선택
	int randomIndex = rand() % emptyCount;
	int cellNumber = emptyCells[randomIndex];

	int x = cellNumber % boardsize;
	int y = cellNumber / boardsize;

	// 선택된 빈칸에 숫자 2 생성
	board[y][x] = 2;

	return true;
}


// ------------------------------------------------------------
// 승리 조건 확인
// 선택한 목표 숫자 이상이 만들어졌는지 검사
// ------------------------------------------------------------
bool checkWin() {
	for (int y = 0; y < boardsize; y++) {
		for (int x = 0; x < boardsize; x++) {
			if (board[y][x] >= winNumber) {
				return true;
			}
		}
	}

	return false;
}


// ------------------------------------------------------------
// 더 이상 이동하거나 합칠 수 있는지 확인
// 아무 행동도 할 수 없으면 false 반환
// ------------------------------------------------------------
bool canDoAnyAction() {
	for (int y = 0; y < boardsize; y++) {
		for (int x = 0; x < boardsize; x++) {

			// 숫자 블록만 검사
			if (board[y][x] > 0) {

				// 왼쪽 검사
				if (x - 1 >= 0) {
					if (board[y][x - 1] == 0)
						return true;

					if (board[y][x - 1] == board[y][x] && board[y][x] < max_num_value)
						return true;
				}

				// 오른쪽 검사
				if (x + 1 < boardsize) {
					if (board[y][x + 1] == 0)
						return true;

					if (board[y][x + 1] == board[y][x] && board[y][x] < max_num_value)
						return true;
				}

				// 위쪽 검사
				if (y - 1 >= 0) {
					if (board[y - 1][x] == 0)
						return true;

					if (board[y - 1][x] == board[y][x] && board[y][x] < max_num_value)
						return true;
				}

				// 아래쪽 검사
				if (y + 1 < boardsize) {
					if (board[y + 1][x] == 0)
						return true;

					if (board[y + 1][x] == board[y][x] && board[y][x] < max_num_value)
						return true;
				}
			}
		}
	}

	return false;
}


// ------------------------------------------------------------
// 승리 처리
// ------------------------------------------------------------
void winGame(HWND hWnd) {
	WCHAR message[100];

	wsprintf(message, L"%d를 만들었습니다!\n게임 승리!", winNumber);

	MessageBox(hWnd, message, L"승리", MB_OK | MB_ICONINFORMATION);

	isStart = false;
	InvalidateRect(hWnd, NULL, TRUE);
}


// ------------------------------------------------------------
// 패배 처리
// ------------------------------------------------------------
void loseGame(HWND hWnd) {
	MessageBox(hWnd, L"더 이상 진행할 수 없습니다.\n게임 패배!", L"패배", MB_OK | MB_ICONWARNING);

	isStart = false;
	InvalidateRect(hWnd, NULL, TRUE);
}


// ------------------------------------------------------------
// WinMain
// ------------------------------------------------------------
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

	hWnd = CreateWindow(
		lpszClass,
		lpszWindowName,
		WS_OVERLAPPEDWINDOW,
		0,
		0,
		1600,
		900,
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

	return Message.wParam;
}


// ------------------------------------------------------------
// 윈도우 메시지 처리
// ------------------------------------------------------------
LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	PAINTSTRUCT ps;
	HDC hDC, mDC;
	RECT rt;
	HBITMAP hBitmap;
	HBITMAP oldBitmap;

	switch (uMsg) {

	case WM_CREATE:
		// 이미지 한 번만 로드
		num2.Load(L"5-4/2.png");
		num4.Load(L"5-4/4.png");
		num8.Load(L"5-4/8.png");
		num16.Load(L"5-4/16.png");
		num32.Load(L"5-4/32.png");
		num64.Load(L"5-4/64.png");

		// 랜덤 시드 초기화
		srand((unsigned int)time(NULL));

		// 초기 보드 생성
		initBoard();
		break;


	case WM_PAINT:
		GetClientRect(hWnd, &rt);

		hDC = BeginPaint(hWnd, &ps);

		// 더블 버퍼링용 메모리 DC 생성
		mDC = CreateCompatibleDC(hDC);

		// 메모리 비트맵 생성
		hBitmap = CreateCompatibleBitmap(hDC, rt.right, rt.bottom);

		// 기존 비트맵 저장 후 새 비트맵 선택
		oldBitmap = (HBITMAP)SelectObject(mDC, hBitmap);

		// 배경 흰색으로 지우기
		FillRect(mDC, &rt, (HBRUSH)GetStockObject(WHITE_BRUSH));

		// 보드판은 항상 그리기
		drawBoard(mDC);

		// 게임이 시작된 경우만 장애물과 숫자 블록 그리기
		if (isStart) {
			drawBlock(mDC);
			drawNumbers(mDC);
		}

		// 메모리 DC 내용을 실제 화면으로 복사
		BitBlt(hDC, 0, 0, rt.right, rt.bottom, mDC, 0, 0, SRCCOPY);

		// 리소스 정리
		SelectObject(mDC, oldBitmap);
		DeleteObject(hBitmap);
		DeleteDC(mDC);

		EndPaint(hWnd, &ps);
		break;


	case WM_LBUTTONDOWN:
		// 게임 시작 상태에서만 드래그 시작 위치 저장
		if (isStart) {
			dragStartX = LOWORD(lParam);
			dragStartY = HIWORD(lParam);
		}
		break;


	case WM_LBUTTONUP:
		if (isStart) {
			dragEndX = LOWORD(lParam);
			dragEndY = HIWORD(lParam);

			checkDragDirection();

			if (slideDir != DIR_NONE) {
				bool isChanged = false;

				// 이번 드래그에서 합쳐진 위치 기록 초기화
				clearMergedCells();

				// 1. 같은 숫자는 먼저 합침
				bool isMerged = mergeNumbers(slideDir);

				// 2. 합쳐진 숫자는 그대로 두고,
				//    합쳐지지 않은 나머지 숫자는 한 칸 이동
				bool isMoved = moveNumbersOneStep(slideDir);

				// 합치기 또는 이동이 있었다면 변화가 발생한 것
				if (isMerged || isMoved) {
					isChanged = true;
				}

				// 3. 보드에 변화가 있었다면 처리
				if (isChanged) {

					// 목표 숫자가 만들어졌으면 즉시 승리
					if (checkWin()) {
						InvalidateRect(hWnd, NULL, TRUE);
						winGame(hWnd);
						slideDir = DIR_NONE;
						break;
					}

					// 새 숫자 2 생성 시도
					bool isCreated = createNewNum2();

					// 새 2를 만들 수 없으면 패배
					if (!isCreated) {
						InvalidateRect(hWnd, NULL, TRUE);
						loseGame(hWnd);
						slideDir = DIR_NONE;
						break;
					}
				}

				// 4. 현재 상태에서 아무 행동도 할 수 없으면 패배
				if (!canDoAnyAction()) {
					InvalidateRect(hWnd, NULL, TRUE);
					loseGame(hWnd);
					slideDir = DIR_NONE;
					break;
				}

				InvalidateRect(hWnd, NULL, TRUE);
			}

			slideDir = DIR_NONE;
		}
		break;


	case WM_COMMAND:
		switch (LOWORD(wParam)) {

		case ID_GOAL_32:
			winNumber = 32;
			break;

		case ID_GOAL_64:
			winNumber = 64;
			break;

		case ID_GAME_START:
			isStart = true;
			initBoard();
			InvalidateRect(hWnd, NULL, TRUE);
			break;

		case ID_GAME_END:
			isStart = false;
			InvalidateRect(hWnd, NULL, TRUE);
			break;

		case ID_BLOCK_2:
			block = 2;
			initBoard();
			InvalidateRect(hWnd, NULL, TRUE);
			break;

		case ID_BLOCK_3:
			block = 3;
			initBoard();
			InvalidateRect(hWnd, NULL, TRUE);
			break;

		case ID_BLOCK_4:
			block = 4;
			initBoard();
			InvalidateRect(hWnd, NULL, TRUE);
			break;
		}
		break;


	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	}

	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}