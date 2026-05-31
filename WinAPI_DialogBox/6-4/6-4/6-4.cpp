#include <windows.h>
#include <tchar.h>
#include "resource.h"

// ----------------------------------------------------
// 기본 윈도우 전역 변수
// ----------------------------------------------------
HINSTANCE g_hInst;
HWND g_hWnd;

LPCTSTR lpszClass = L"My Window Class";
LPCTSTR lpszWindowName = L"DialogBox Paint Board";

// ----------------------------------------------------
// 함수 원형 선언
// ----------------------------------------------------
LRESULT CALLBACK WndProc(HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK ControlDlgProc(HWND hDlg, UINT iMessage, WPARAM wParam, LPARAM lParam);

// ----------------------------------------------------
// 보드판 설정
// ----------------------------------------------------
#define BOARD_ROW       20
#define BOARD_COL       20
#define CELL_SIZE       25
#define BOARD_START_X   30
#define BOARD_START_Y   30

#define COLOR_EMPTY     -1

// ----------------------------------------------------
// 색상 번호
// ----------------------------------------------------
#define COLOR_RED       0      // 빨강 색상 번호
#define COLOR_GREEN     1      // 초록 색상 번호
#define COLOR_BLUE      2      // 파랑 색상 번호
#define COLOR_YELLOW    3      // 노랑 색상 번호
#define COLOR_BLACK     4      // 검정 색상 번호
#define COLOR_WHITE     5      // 흰색 색상 번호

// ----------------------------------------------------
// 전역 변수
// ----------------------------------------------------
int board[BOARD_ROW][BOARD_COL];

int selectedColor = COLOR_RED;
int isDrawMode = 0;
int isMouseDown = 0;
int programQuit = 0;

// ----------------------------------------------------
// 실제 색상 배열
// ----------------------------------------------------
COLORREF colorList[6] = {
	RGB(255, 0, 0),        // 빨강
	RGB(0, 180, 0),        // 초록
	RGB(0, 0, 255),        // 파랑
	RGB(255, 255, 0),      // 노랑
	RGB(0, 0, 0),          // 검정
	RGB(255, 255, 255)     // 흰색
};

// ----------------------------------------------------
// 보드판 초기화 함수
// ----------------------------------------------------
void ClearBoard()
{
	for (int y = 0; y < BOARD_ROW; y++) {
		for (int x = 0; x < BOARD_COL; x++) {
			board[y][x] = COLOR_EMPTY;
		}
	}
}

// ----------------------------------------------------
// 마우스 좌표가 보드 안인지 확인하는 함수
// ----------------------------------------------------
int IsInsideBoard(int mx, int my)
{
	int boardEndX = BOARD_START_X + BOARD_COL * CELL_SIZE;
	int boardEndY = BOARD_START_Y + BOARD_ROW * CELL_SIZE;

	if (mx < BOARD_START_X) {
		return 0;
	}

	if (mx >= boardEndX) {
		return 0;
	}

	if (my < BOARD_START_Y) {
		return 0;
	}

	if (my >= boardEndY) {
		return 0;
	}

	return 1;
}

// ----------------------------------------------------
// 마우스가 있는 칸을 색칠하는 함수
// ----------------------------------------------------
void PaintCellByMouse(HWND hWnd, int mx, int my)
{
	if (isDrawMode == 0) {
		return;
	}

	if (IsInsideBoard(mx, my) == 0) {
		return;
	}

	int cellX = (mx - BOARD_START_X) / CELL_SIZE;
	int cellY = (my - BOARD_START_Y) / CELL_SIZE;

	board[cellY][cellX] = selectedColor;

	RECT cellRect;
	cellRect.left = BOARD_START_X + cellX * CELL_SIZE;
	cellRect.top = BOARD_START_Y + cellY * CELL_SIZE;
	cellRect.right = cellRect.left + CELL_SIZE + 1;
	cellRect.bottom = cellRect.top + CELL_SIZE + 1;

	InvalidateRect(hWnd, &cellRect, FALSE);
}

// ----------------------------------------------------
// 보드판 그리기 함수
// ----------------------------------------------------
void DrawBoard(HDC hDC)
{
	RECT boardRect;
	boardRect.left = BOARD_START_X;
	boardRect.top = BOARD_START_Y;
	boardRect.right = BOARD_START_X + BOARD_COL * CELL_SIZE;
	boardRect.bottom = BOARD_START_Y + BOARD_ROW * CELL_SIZE;

	HBRUSH whiteBrush = CreateSolidBrush(RGB(255, 255, 255));
	FillRect(hDC, &boardRect, whiteBrush);
	DeleteObject(whiteBrush);

	for (int y = 0; y < BOARD_ROW; y++) {
		for (int x = 0; x < BOARD_COL; x++) {

			if (board[y][x] != COLOR_EMPTY) {
				RECT cellRect;

				cellRect.left = BOARD_START_X + x * CELL_SIZE + 1;
				cellRect.top = BOARD_START_Y + y * CELL_SIZE + 1;
				cellRect.right = cellRect.left + CELL_SIZE - 1;
				cellRect.bottom = cellRect.top + CELL_SIZE - 1;

				HBRUSH colorBrush = CreateSolidBrush(colorList[board[y][x]]);
				FillRect(hDC, &cellRect, colorBrush);
				DeleteObject(colorBrush);
			}
		}
	}

	HPEN gridPen = CreatePen(PS_SOLID, 1, RGB(180, 180, 180));
	HPEN oldPen = (HPEN)SelectObject(hDC, gridPen);

	for (int x = 0; x <= BOARD_COL; x++) {
		int lineX = BOARD_START_X + x * CELL_SIZE;

		MoveToEx(hDC, lineX, BOARD_START_Y, NULL);
		LineTo(hDC, lineX, BOARD_START_Y + BOARD_ROW * CELL_SIZE);
	}

	for (int y = 0; y <= BOARD_ROW; y++) {
		int lineY = BOARD_START_Y + y * CELL_SIZE;

		MoveToEx(hDC, BOARD_START_X, lineY, NULL);
		LineTo(hDC, BOARD_START_X + BOARD_COL * CELL_SIZE, lineY);
	}

	SelectObject(hDC, oldPen);
	DeleteObject(gridPen);
}

// ----------------------------------------------------
// 현재 상태 글자 출력
// ----------------------------------------------------
void DrawInfoText(HDC hDC)
{
	SetBkMode(hDC, TRANSPARENT);

	if (isDrawMode == 1) {
		TextOut(hDC, 30, 540, L"Draw Mode : ON", 14);
	}
	else {
		TextOut(hDC, 30, 540, L"Draw Mode : OFF", 15);
	}
}

// ----------------------------------------------------
// WinMain
// ----------------------------------------------------
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

	hWnd = CreateWindow(
		lpszClass,
		lpszWindowName,
		WS_OVERLAPPEDWINDOW,
		100,
		100,
		600,
		620,
		NULL,
		(HMENU)NULL,
		hInstance,
		NULL
	);

	g_hWnd = hWnd;

	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);

	DialogBox(
		g_hInst,
		MAKEINTRESOURCE(IDD_CONTROL_PANEL),
		NULL,
		ControlDlgProc
	);

	if (programQuit == 1) {
		return 0;
	}

	while (GetMessage(&Message, 0, 0, 0)) {
		TranslateMessage(&Message);
		DispatchMessage(&Message);
	}

	return Message.wParam;
}

// ----------------------------------------------------
// 메인 윈도우 메시지 처리 함수
// ----------------------------------------------------
LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	PAINTSTRUCT ps;
	HDC hDC;

	switch (uMsg) {

	case WM_CREATE:
		ClearBoard();
		break;

	case WM_LBUTTONDOWN:
	{
		int mx = LOWORD(lParam);
		int my = HIWORD(lParam);

		isMouseDown = 1;

		PaintCellByMouse(hWnd, mx, my);

		break;
	}

	case WM_MOUSEMOVE:
	{
		int mx = LOWORD(lParam);
		int my = HIWORD(lParam);

		if (isMouseDown == 1) {
			PaintCellByMouse(hWnd, mx, my);
		}

		break;
	}

	case WM_LBUTTONUP:
		isMouseDown = 0;
		break;

	case WM_PAINT:
		hDC = BeginPaint(hWnd, &ps);

		DrawBoard(hDC);
		DrawInfoText(hDC);

		EndPaint(hWnd, &ps);
		break;

	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	}

	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

// ----------------------------------------------------
// DialogBox 메시지 처리 함수
// ----------------------------------------------------
INT_PTR CALLBACK ControlDlgProc(HWND hDlg, UINT iMessage, WPARAM wParam, LPARAM lParam)
{
	switch (iMessage) {

	case WM_INITDIALOG:
		CheckRadioButton(
			hDlg,
			IDC_RADIO_RED,
			IDC_RADIO_WHITE,
			IDC_RADIO_RED
		);
		return TRUE;

	case WM_COMMAND:
	{
		int controlId = LOWORD(wParam);

		if (controlId == IDC_BTN_DRAW) {
			isDrawMode = 1;

			InvalidateRect(g_hWnd, NULL, TRUE);
		}
		else if (controlId == IDC_BTN_CLEAR) {
			ClearBoard();

			InvalidateRect(g_hWnd, NULL, TRUE);
		}
		else if (controlId == IDC_BTN_QUIT) {
			programQuit = 1;

			DestroyWindow(g_hWnd);

			EndDialog(hDlg, 0);
		}
		else if (controlId == IDC_RADIO_RED) {
			selectedColor = COLOR_RED;
		}
		else if (controlId == IDC_RADIO_GREEN) {
			selectedColor = COLOR_GREEN;
		}
		else if (controlId == IDC_RADIO_BLUE) {
			selectedColor = COLOR_BLUE;
		}
		else if (controlId == IDC_RADIO_YELLOW) {
			selectedColor = COLOR_YELLOW;
		}
		else if (controlId == IDC_RADIO_BLACK) {   // 검정 라디오 버튼을 눌렀다면
			selectedColor = COLOR_BLACK;           // 현재 선택 색상을 검정으로 바꾼다
		}
		else if (controlId == IDC_RADIO_WHITE) {   // 흰색 라디오 버튼을 눌렀다면
			selectedColor = COLOR_WHITE;           // 현재 선택 색상을 흰색으로 바꾼다
		}

		return TRUE;
	}

	case WM_CLOSE:
		EndDialog(hDlg, 0);
		return TRUE;
	}

	return FALSE;
}