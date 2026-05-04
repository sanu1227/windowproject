#include <windows.h>
#include <tchar.h>
#include <atlimage.h>

#define MAX_IMAGE_COUNT 6

HINSTANCE g_hInst;
LPCTSTR lpszClass = L"My Window Class";
LPCTSTR lpszWindowName = L"Window Programming Lab";

LRESULT CALLBACK WndProc(HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam);

// 이미지
CImage g_image;

// 현재 상태
int g_divideCount = 1;              // 1, 2, 4, 6
double g_scale = 1.0;               // +/- 확대 축소 비율
bool g_fillWindow = false;          // a키: 창에 꽉 채우기 여부
int g_selectedIndex = -1;           // 선택된 그림 번호
bool g_reverse[MAX_IMAGE_COUNT] = { false }; // 색상 반전 여부

void ResetReverse()
{
	for (int i = 0; i < MAX_IMAGE_COUNT; i++) {
		g_reverse[i] = false;
	}
}

int GetImageRects(HWND hWnd, RECT drawRects[MAX_IMAGE_COUNT], RECT clickRects[MAX_IMAGE_COUNT])
{
	RECT clientRect;
	GetClientRect(hWnd, &clientRect);

	int clientW = clientRect.right - clientRect.left;
	int clientH = clientRect.bottom - clientRect.top;

	if (clientW <= 0 || clientH <= 0) {
		return 0;
	}

	// a키 모드: 창에 빈 공간 없이 꽉 채우기
	if (g_fillWindow) {
		drawRects[0] = clientRect;
		clickRects[0] = clientRect;
		return 1;
	}

	// 1등분: 원래 그림 크기로 그리기
	if (g_divideCount == 1) {
		int imgW = 300;
		int imgH = 300;

		if (!g_image.IsNull()) {
			imgW = g_image.GetWidth();
			imgH = g_image.GetHeight();
		}

		int drawW = (int)(imgW * g_scale);
		int drawH = (int)(imgH * g_scale);

		if (drawW < 1) drawW = 1;
		if (drawH < 1) drawH = 1;

		drawRects[0].left = 0;
		drawRects[0].top = 0;
		drawRects[0].right = drawW;
		drawRects[0].bottom = drawH;

		clickRects[0] = drawRects[0];

		return 1;
	}

	// 2, 4, 6등분
	int cols = 1;
	int rows = 1;

	if (g_divideCount == 2) {
		cols = 2;
		rows = 1;
	}
	else if (g_divideCount == 4) {
		cols = 2;
		rows = 2;
	}
	else if (g_divideCount == 6) {
		cols = 3;
		rows = 2;
	}

	int index = 0;

	for (int r = 0; r < rows; r++) {
		for (int c = 0; c < cols; c++) {
			RECT cell;

			cell.left = clientW * c / cols;
			cell.top = clientH * r / rows;
			cell.right = clientW * (c + 1) / cols;
			cell.bottom = clientH * (r + 1) / rows;

			clickRects[index] = cell;

			int cellW = cell.right - cell.left;
			int cellH = cell.bottom - cell.top;

			int drawW = (int)(cellW * g_scale);
			int drawH = (int)(cellH * g_scale);

			if (drawW < 1) drawW = 1;
			if (drawH < 1) drawH = 1;

			// 각 칸 중앙에 그리기
			drawRects[index].left = cell.left + (cellW - drawW) / 2;
			drawRects[index].top = cell.top + (cellH - drawH) / 2;
			drawRects[index].right = drawRects[index].left + drawW;
			drawRects[index].bottom = drawRects[index].top + drawH;

			index++;
		}
	}

	return g_divideCount;
}

void DrawOneImage(HDC hDC, RECT rt, bool reverse)
{
	if (g_image.IsNull()) {
		return;
	}

	int w = rt.right - rt.left;
	int h = rt.bottom - rt.top;

	if (w <= 0 || h <= 0) {
		return;
	}

	// 일반 출력
	if (!reverse) {
		g_image.Draw(hDC, rt.left, rt.top, w, h);
		return;
	}

	// 색상 반전 출력
	HDC hTempDC = CreateCompatibleDC(hDC);
	HBITMAP hTempBitmap = CreateCompatibleBitmap(hDC, w, h);
	HBITMAP hOldBitmap = (HBITMAP)SelectObject(hTempDC, hTempBitmap);

	RECT tempRect = { 0, 0, w, h };
	FillRect(hTempDC, &tempRect, (HBRUSH)GetStockObject(WHITE_BRUSH));

	g_image.Draw(hTempDC, 0, 0, w, h);

	BitBlt(
		hDC,
		rt.left,
		rt.top,
		w,
		h,
		hTempDC,
		0,
		0,
		NOTSRCCOPY
	);

	SelectObject(hTempDC, hOldBitmap);
	DeleteObject(hTempBitmap);
	DeleteDC(hTempDC);
}

void DrawSelectedBorder(HDC hDC, RECT rt)
{
	HPEN hPen = CreatePen(PS_SOLID, 6, RGB(255, 0, 0));
	HPEN hOldPen = (HPEN)SelectObject(hDC, hPen);

	HBRUSH hOldBrush = (HBRUSH)SelectObject(hDC, GetStockObject(NULL_BRUSH));

	Rectangle(
		hDC,
		rt.left + 3,
		rt.top + 3,
		rt.right - 3,
		rt.bottom - 3
	);

	SelectObject(hDC, hOldBrush);
	SelectObject(hDC, hOldPen);
	DeleteObject(hPen);
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

	// 클라이언트 영역을 800x600으로 만들기
	RECT winRect = { 0, 0, 800, 600 };
	AdjustWindowRect(&winRect, WS_OVERLAPPEDWINDOW, FALSE);

	int windowW = winRect.right - winRect.left;
	int windowH = winRect.bottom - winRect.top;

	hWnd = CreateWindow(
		lpszClass,
		lpszWindowName,
		WS_OVERLAPPEDWINDOW,
		0,
		0,
		windowW,
		windowH,
		NULL,
		NULL,
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

LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	PAINTSTRUCT ps;
	HDC hDC;
	HDC hMemDC;
	HBITMAP hBitmap;
	HBITMAP hOldBitmap;
	RECT rt;

	switch (uMsg) {
	case WM_CREATE:
	{
		HRESULT hr = g_image.Load(L"image.png");

		if (FAILED(hr)) {
			MessageBox(hWnd, L"image.png 파일을 불러오지 못했습니다.", L"오류", MB_OK);
		}

		return 0;
	}

	case WM_ERASEBKGND:
		// 깜빡임 줄이기
		return 1;

	case WM_KEYDOWN:
	{
		if (wParam == 'A') {
			// 창에 꽉 채우기 / 원래 크기 전환
			g_fillWindow = !g_fillWindow;
			g_divideCount = 1;
			g_scale = 1.0;
			g_selectedIndex = -1;
			ResetReverse();

			InvalidateRect(hWnd, NULL, FALSE);
		}
		else if (wParam == VK_OEM_PLUS || wParam == VK_ADD) {
			// 크게
			g_fillWindow = false;
			g_scale += 0.1;

			if (g_scale > 3.0) {
				g_scale = 3.0;
			}

			InvalidateRect(hWnd, NULL, FALSE);
		}
		else if (wParam == VK_OEM_MINUS || wParam == VK_SUBTRACT) {
			// 작게
			g_fillWindow = false;
			g_scale -= 0.1;

			if (g_scale < 0.2) {
				g_scale = 0.2;
			}

			InvalidateRect(hWnd, NULL, FALSE);
		}
		else if (wParam == '1') {
			g_fillWindow = false;
			g_divideCount = 1;
			g_scale = 1.0;
			g_selectedIndex = -1;
			ResetReverse();

			InvalidateRect(hWnd, NULL, FALSE);
		}
		else if (wParam == '2') {
			g_fillWindow = false;
			g_divideCount = 2;
			g_scale = 1.0;
			g_selectedIndex = -1;
			ResetReverse();

			InvalidateRect(hWnd, NULL, FALSE);
		}
		else if (wParam == '4') {
			g_fillWindow = false;
			g_divideCount = 4;
			g_scale = 1.0;
			g_selectedIndex = -1;
			ResetReverse();

			InvalidateRect(hWnd, NULL, FALSE);
		}
		else if (wParam == '6') {
			g_fillWindow = false;
			g_divideCount = 6;
			g_scale = 1.0;
			g_selectedIndex = -1;
			ResetReverse();

			InvalidateRect(hWnd, NULL, FALSE);
		}
		else if (wParam == 'R') {
			// 선택된 그림 색상 반전 / 원래대로
			if (g_selectedIndex >= 0 && g_selectedIndex < MAX_IMAGE_COUNT) {
				g_reverse[g_selectedIndex] = !g_reverse[g_selectedIndex];
				InvalidateRect(hWnd, NULL, FALSE);
			}
		}
		else if (wParam == 'Q') {
			DestroyWindow(hWnd);
		}

		return 0;
	}

	case WM_LBUTTONDOWN:
	{
		int mouseX = LOWORD(lParam);
		int mouseY = HIWORD(lParam);

		POINT pt;
		pt.x = mouseX;
		pt.y = mouseY;

		RECT drawRects[MAX_IMAGE_COUNT];
		RECT clickRects[MAX_IMAGE_COUNT];

		int count = GetImageRects(hWnd, drawRects, clickRects);

		g_selectedIndex = -1;

		for (int i = 0; i < count; i++) {
			if (PtInRect(&clickRects[i], pt)) {
				g_selectedIndex = i;
				break;
			}
		}

		InvalidateRect(hWnd, NULL, FALSE);
		return 0;
	}

	case WM_PAINT:
	{
		hDC = BeginPaint(hWnd, &ps);

		GetClientRect(hWnd, &rt);

		int width = rt.right - rt.left;
		int height = rt.bottom - rt.top;

		hMemDC = CreateCompatibleDC(hDC);
		hBitmap = CreateCompatibleBitmap(hDC, width, height);
		hOldBitmap = (HBITMAP)SelectObject(hMemDC, hBitmap);

		// 배경 지우기
		FillRect(hMemDC, &rt, (HBRUSH)GetStockObject(WHITE_BRUSH));

		RECT drawRects[MAX_IMAGE_COUNT];
		RECT clickRects[MAX_IMAGE_COUNT];

		int count = GetImageRects(hWnd, drawRects, clickRects);

		// 그림 출력
		for (int i = 0; i < count; i++) {
			DrawOneImage(hMemDC, drawRects[i], g_reverse[i]);
		}

		// 선택된 그림 빨간 테두리
		if (g_selectedIndex >= 0 && g_selectedIndex < count) {
			DrawSelectedBorder(hMemDC, drawRects[g_selectedIndex]);
		}

		// 메모리 DC를 실제 화면으로 복사
		BitBlt(
			hDC,
			0,
			0,
			width,
			height,
			hMemDC,
			0,
			0,
			SRCCOPY
		);

		SelectObject(hMemDC, hOldBitmap);
		DeleteObject(hBitmap);
		DeleteDC(hMemDC);

		EndPaint(hWnd, &ps);
		return 0;
	}

	case WM_DESTROY:
		if (!g_image.IsNull()) {
			g_image.Destroy();
		}

		PostQuitMessage(0);
		return 0;
	}

	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}