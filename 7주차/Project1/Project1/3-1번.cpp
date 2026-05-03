#include <windows.h> //--- 윈도우 헤더 파일
#include <tchar.h>

HINSTANCE g_hInst;
LPCTSTR lpszClass = L"My Window Class";
LPCTSTR lpszWindowName = L"Window Programming Lab";
LRESULT CALLBACK WndProc(HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam);

#define MOVEshape_circle 0
#define MOVEshape_triangle 1
#define MOVEshape_rect 2

// direction 값 이름 정의
#define DIR_RIGHT        0
#define DIR_LEFT         1
#define DIR_DOWN         2
#define DIR_UP           3
#define DIR_UP_RIGHT     4
#define DIR_DOWN_RIGHT   5
#define DIR_DOWN_LEFT    6
#define DIR_UP_LEFT      7

void killtimer_All(HWND hWnd) {
	for (int i = 1; i < 9; i++) {
		KillTimer(hWnd, i);
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

	hWnd = CreateWindow(
		lpszClass,
		lpszWindowName,
		WS_OVERLAPPEDWINDOW,
		0,
		0,
		800,
		600,
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

LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	PAINTSTRUCT ps;
	HDC hDC;
	static RECT rectView;
	static int mx, my;
	static int shape_type = MOVEshape_rect;
	POINT pt[3];
	static int isClicked = 0;
	static int direction = DIR_RIGHT;
	static int movespeed = 1000;

	switch (uMsg) {
	case WM_CREATE:
		GetClientRect(hWnd, &rectView);
		break;

	case WM_SIZE:
		GetClientRect(hWnd, &rectView);
		break;

	case WM_LBUTTONDOWN:
		mx = LOWORD(lParam);
		my = HIWORD(lParam);
		isClicked = 1;
		InvalidateRect(hWnd, NULL, TRUE);
		break;

	case WM_KEYDOWN:
		if ((wParam == 'h' || wParam == 'H') && isClicked == 1) {
			killtimer_All(hWnd);
			SetTimer(hWnd, 1, movespeed, NULL);
		}

		if ((wParam == 'v' || wParam == 'V') && isClicked == 1) {
			killtimer_All(hWnd);
			SetTimer(hWnd, 3, movespeed, NULL);
		}

		if ((wParam == 's' || wParam == 'S') && isClicked == 1) {
			killtimer_All(hWnd);
			SetTimer(hWnd, 5, movespeed, NULL);
		}

		if (wParam == 'r' || wParam == 'R') {
			shape_type = MOVEshape_rect;
			InvalidateRect(hWnd, NULL, TRUE);
		}

		if (wParam == 'e' || wParam == 'E') {
			shape_type = MOVEshape_circle;
			InvalidateRect(hWnd, NULL, TRUE);
		}

		if (wParam == 't' || wParam == 'T') {
			shape_type = MOVEshape_triangle;
			InvalidateRect(hWnd, NULL, TRUE);
		}

		if (wParam == 'p' || wParam == 'P') {
			killtimer_All(hWnd);
		}

		if (wParam == 'q' || wParam == 'Q') {
			DestroyWindow(hWnd);
			return 0;
		}

		if (wParam == VK_NEXT) {
			movespeed += 100;
			if (movespeed > 2000) {
				movespeed = 2000;
			}

			if (isClicked == 1) {
				killtimer_All(hWnd);

				if (direction == DIR_RIGHT) {
					SetTimer(hWnd, 1, movespeed, NULL);
				}
				else if (direction == DIR_LEFT) {
					SetTimer(hWnd, 2, movespeed, NULL);
				}
				else if (direction == DIR_DOWN) {
					SetTimer(hWnd, 3, movespeed, NULL);
				}
				else if (direction == DIR_UP) {
					SetTimer(hWnd, 4, movespeed, NULL);
				}
				else if (direction == DIR_UP_RIGHT) {
					SetTimer(hWnd, 5, movespeed, NULL);
				}
				else if (direction == DIR_DOWN_RIGHT) {
					SetTimer(hWnd, 6, movespeed, NULL);
				}
				else if (direction == DIR_DOWN_LEFT) {
					SetTimer(hWnd, 7, movespeed, NULL);
				}
				else if (direction == DIR_UP_LEFT) {
					SetTimer(hWnd, 8, movespeed, NULL);
				}
			}
		}

		if (wParam == VK_PRIOR) {
			movespeed -= 100;
			if (movespeed < 100) {
				movespeed = 100;
			}

			if (isClicked == 1) {
				killtimer_All(hWnd);

				if (direction == DIR_RIGHT) {
					SetTimer(hWnd, 1, movespeed, NULL);
				}
				else if (direction == DIR_LEFT) {
					SetTimer(hWnd, 2, movespeed, NULL);
				}
				else if (direction == DIR_DOWN) {
					SetTimer(hWnd, 3, movespeed, NULL);
				}
				else if (direction == DIR_UP) {
					SetTimer(hWnd, 4, movespeed, NULL);
				}
				else if (direction == DIR_UP_RIGHT) {
					SetTimer(hWnd, 5, movespeed, NULL);
				}
				else if (direction == DIR_DOWN_RIGHT) {
					SetTimer(hWnd, 6, movespeed, NULL);
				}
				else if (direction == DIR_DOWN_LEFT) {
					SetTimer(hWnd, 7, movespeed, NULL);
				}
				else if (direction == DIR_UP_LEFT) {
					SetTimer(hWnd, 8, movespeed, NULL);
				}
			}
		}

		break;

	case WM_TIMER:
		switch (wParam) {
		case 1:
			direction = DIR_RIGHT;
			mx += 40;
			if (mx + 20 > rectView.right) {
				mx = rectView.right - 20;
				my += 50;
				killtimer_All(hWnd);
				SetTimer(hWnd, 2, movespeed, NULL);
			}
			break;

		case 2:
			direction = DIR_LEFT;
			mx -= 40;
			if (mx - 20 < rectView.left) {
				mx = rectView.left + 20;
				my += 50;
				killtimer_All(hWnd);
				SetTimer(hWnd, 1, movespeed, NULL);
			}
			break;

		case 3:
			direction = DIR_DOWN;
			my += 40;
			if (my + 40 > rectView.bottom) {
				my = rectView.bottom - 40;
				mx += 50;
				killtimer_All(hWnd);
				SetTimer(hWnd, 4, movespeed, NULL);
			}
			break;

		case 4:
			direction = DIR_UP;
			my -= 40;
			if (my - 40 < rectView.top) {
				my = rectView.top + 40;
				mx += 50;
				killtimer_All(hWnd);
				SetTimer(hWnd, 3, movespeed, NULL);
			}
			break;

			// 오른쪽 위 대각선
		case 5:
			direction = DIR_UP_RIGHT;
			mx += 40;
			my -= 40;
			break;

			// 오른쪽 아래 대각선
		case 6:
			direction = DIR_DOWN_RIGHT;
			mx += 40;
			my += 40;
			break;

			// 왼쪽 아래 대각선
		case 7:
			direction = DIR_DOWN_LEFT;
			mx -= 40;
			my += 40;
			break;

			// 왼쪽 위 대각선
		case 8:
			direction = DIR_UP_LEFT;
			mx -= 40;
			my -= 40;
			break;
		}

		if ((my + 20 > rectView.bottom) && (direction == DIR_RIGHT || direction == DIR_LEFT)) {
			my = 20;
			mx = 20;

			killtimer_All(hWnd);

			if (direction == DIR_RIGHT) {
				SetTimer(hWnd, 2, movespeed, NULL);
			}
			else {
				SetTimer(hWnd, 1, movespeed, NULL);
			}
		}

		if ((mx + 50 > rectView.right) && (direction == DIR_DOWN || direction == DIR_UP)) {
			my = 20;
			mx = 20;

			killtimer_All(hWnd);

			if (direction == DIR_DOWN) {
				SetTimer(hWnd, 3, movespeed, NULL);
			}
			else {
				SetTimer(hWnd, 4, movespeed, NULL);
			}
		}

		if (direction == DIR_UP_RIGHT || direction == DIR_DOWN_RIGHT ||
			direction == DIR_DOWN_LEFT || direction == DIR_UP_LEFT) {

			if (my - 40 < rectView.top) {
				my = rectView.top + 10;
				killtimer_All(hWnd);
				SetTimer(hWnd, 6, movespeed, NULL);
			}
			else if (mx - 40 < rectView.left) {
				mx = rectView.left + 10;
				killtimer_All(hWnd);
				SetTimer(hWnd, 5, movespeed, NULL);
			}
			else if (my + 40 > rectView.bottom) {
				my = rectView.bottom - 10;
				killtimer_All(hWnd);
				SetTimer(hWnd, 8, movespeed, NULL);
			}
			else if (mx + 40 > rectView.right) {
				mx = rectView.right - 10;
				killtimer_All(hWnd);
				SetTimer(hWnd, 7, movespeed, NULL);
			}
		}

		InvalidateRect(hWnd, NULL, TRUE);
		break;

	case WM_PAINT:
		hDC = BeginPaint(hWnd, &ps);

		if (isClicked == 1) {
			if (shape_type == MOVEshape_circle) {
				Ellipse(hDC, mx - 20, my - 20, mx + 20, my + 20);
			}
			else if (shape_type == MOVEshape_rect) {
				Rectangle(hDC, mx - 20, my - 20, mx + 20, my + 20);
			}
			else if (shape_type == MOVEshape_triangle) {
				pt[0].x = mx;
				pt[0].y = my - 20;
				pt[1].x = mx - 20;
				pt[1].y = my + 20;
				pt[2].x = mx + 20;
				pt[2].y = my + 20;
				Polygon(hDC, pt, 3);
			}
		}

		EndPaint(hWnd, &ps);
		break;

	case WM_DESTROY:
		killtimer_All(hWnd);
		PostQuitMessage(0);
		break;
	}

	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}