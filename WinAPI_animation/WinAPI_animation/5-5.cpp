#include <windows.h> //--- 윈도우 헤더 파일
#include <tchar.h>
#include <atlimage.h>
#include <time.h>
#include <stdlib.h>

CImage back;
CImage tigger[3];
CImage doyoon[3];
CImage attack[3];
CImage food;

int dx = 500;
int dy = 500;
int tiggerFrame = 0;
int dox = 0;
int doy = 0;
int doFrame = 0;

int moveX = 0;              // x축 이동 방향과 속도
int moveY = 0;              // y축 이동 방향과 속도
int moveSpeed = 15;          // 한 번에 이동할 거리
bool domove = false;

int attackFrame = 0;          // 공격 애니메이션 현재 프레임
bool isAttack = false;        // 호랑이가 공격 애니메이션 중인지 확인

int foodx = 0;              // 먹이 x좌표
int foody = 0;              // 먹이 y좌표
bool isFood = false;        // 먹이가 화면에 존재하는지 확인

void SetRandomDirection()
{
	int direction = rand() % 8;     // 0~7 중 하나를 랜덤하게 선택

	switch (direction) {
	case 0:                         // 오른쪽
		moveX = moveSpeed;
		moveY = 0;
		break;

	case 1:                         // 왼쪽
		moveX = -moveSpeed;
		moveY = 0;
		break;

	case 2:                         // 아래쪽
		moveX = 0;
		moveY = moveSpeed;
		break;

	case 3:                         // 위쪽
		moveX = 0;
		moveY = -moveSpeed;
		break;

	case 4:                         // 오른쪽 아래 대각선
		moveX = moveSpeed;
		moveY = moveSpeed;
		break;

	case 5:                         // 오른쪽 위 대각선
		moveX = moveSpeed;
		moveY = -moveSpeed;
		break;

	case 6:                         // 왼쪽 아래 대각선
		moveX = -moveSpeed;
		moveY = moveSpeed;
		break;

	case 7:                         // 왼쪽 위 대각선
		moveX = -moveSpeed;
		moveY = -moveSpeed;
		break;
	}
}

void animation(HDC hDC, int dx, int dy)
{
	if (isAttack == true) {
		attack[attackFrame].Draw(hDC, dx, dy, 64, 64);
	}
	else {
		tigger[tiggerFrame].Draw(hDC, dx, dy, 64, 64);
	}
}

void doanimation(HDC hDC, int dx, int dy) {
	if (domove) {
		doyoon[doFrame].Draw(hDC, dox, doy, 64, 64);
	}
}

void MoveTiggerToDoyoon()
{
	// 도윤이가 호랑이보다 오른쪽에 있으면 오른쪽으로 이동
	if (dx < dox) {
		dx += moveSpeed;
	}

	// 도윤이가 호랑이보다 왼쪽에 있으면 왼쪽으로 이동
	if (dx > dox) {
		dx -= moveSpeed;
	}

	// 도윤이가 호랑이보다 아래쪽에 있으면 아래쪽으로 이동
	if (dy < doy) {
		dy += moveSpeed;
	}

	// 도윤이가 호랑이보다 위쪽에 있으면 위쪽으로 이동
	if (dy > doy) {
		dy -= moveSpeed;
	}
}

bool IsClickTigger(int mx, int my)
{
	// 호랑이가 그려진 64 x 64 범위 안을 클릭했는지 확인
	if (mx >= dx && mx <= dx + 64 &&
		my >= dy && my <= dy + 64) {
		return true;
	}

	return false;
}

void MoveTiggerRandomPosition()
{
	dx = rand() % (1920 - 64);
	dy = rand() % (1080 - 64);

	SetRandomDirection();     // 새 위치에서 새 랜덤 방향으로 이동
}

void foodanimation(HDC hDC)
{
	if (isFood == true) {
		food.Draw(hDC, foodx, foody, 64, 64);
	}
}

void MoveTiggerToFood()
{
	// 먹이가 호랑이보다 오른쪽에 있으면 오른쪽으로 이동
	if (dx < foodx) {
		dx += moveSpeed;
	}

	// 먹이가 호랑이보다 왼쪽에 있으면 왼쪽으로 이동
	if (dx > foodx) {
		dx -= moveSpeed;
	}

	// 먹이가 호랑이보다 아래쪽에 있으면 아래쪽으로 이동
	if (dy < foody) {
		dy += moveSpeed;
	}

	// 먹이가 호랑이보다 위쪽에 있으면 위쪽으로 이동
	if (dy > foody) {
		dy -= moveSpeed;
	}
}

bool IsTiggerEatFood()
{
	if (dx < foodx + 64 &&
		dx + 64 > foodx &&
		dy < foody + 64 &&
		dy + 64 > foody) {
		return true;
	}

	return false;
}

void ResetGame()
{
	// 호랑이 위치를 처음 위치로 되돌린다
	dx = 500;
	dy = 500;

	// 호랑이 이동 속도를 처음 값으로 되돌린다
	moveSpeed = 15;

	// 호랑이 일반 애니메이션 프레임 초기화
	tiggerFrame = 0;

	// 공격 애니메이션 초기화
	attackFrame = 0;
	isAttack = false;

	// 도윤이 제거 및 애니메이션 초기화
	dox = 0;
	doy = 0;
	doFrame = 0;
	domove = false;

	// 먹이 기능을 추가했다면 먹이도 제거
	isFood = false;
	foodx = 0;
	foody = 0;

	// 초기 랜덤 이동 방향 다시 설정
	SetRandomDirection();
}

HINSTANCE g_hInst;
LPCTSTR lpszClass = L"My Window Class";
LPCTSTR lpszWindowName = L"Window Programming Lab";
LRESULT CALLBACK WndProc(HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam);
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
	hWnd = CreateWindow(lpszClass, lpszWindowName, WS_OVERLAPPEDWINDOW, 0, 0, 1920, 1080, NULL, (HMENU)NULL, hInstance, NULL);
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
		back.Load(L"gg.png");
		tigger[0].Load(L"t1.png");
		tigger[1].Load(L"t2.png");
		tigger[2].Load(L"t3.png");
		doyoon[0].Load(L"do1.png");
		doyoon[1].Load(L"do2.png");
		doyoon[2].Load(L"do3.png");
		attack[0].Load(L"a1.png");
		attack[1].Load(L"a2.png");
		attack[2].Load(L"a3.png");
		food.Load(L"ff.png");

		
		srand((unsigned int)time(NULL));          // 실행할 때마다 다른 난수 결과가 나오도록 설정한다
		SetRandomDirection();                     // 처음 이동 방향을 랜덤하게 정한다

		SetTimer(hWnd, 1, 100, NULL);
		SetTimer(hWnd, 2, 100, NULL);
		break;
	case WM_PAINT:
		GetClientRect(hWnd, &rt);
		hDC = BeginPaint(hWnd, &ps);
		mDC = CreateCompatibleDC(hDC); //--- 메모리 DC 만들기
		hBitmap = CreateCompatibleBitmap(hDC, rt.right, rt.bottom); //--- 메모리 DC와 연결할 비트맵 만들기
		SelectObject(mDC, (HBITMAP)hBitmap);
		FillRect(mDC, &rt, (HBRUSH)GetStockObject(WHITE_BRUSH));

		back.Draw(mDC, 0, 0, 1920, 1080);
		foodanimation(mDC);          // 먹이 출력
		animation(mDC, dx, dy);
		doanimation(mDC, dox, doy);
		BitBlt(hDC, 0, 0, rt.right, rt.bottom, mDC, 0, 0, SRCCOPY);

		DeleteDC(mDC); //--- 생성한 메모리 DC 삭제
		DeleteObject(hBitmap);
		EndPaint(hWnd, &ps);
		break;
	case WM_TIMER:
		if (wParam == 1) {

			// 공격 애니메이션 중일 때
			if (isAttack == true) {
				attackFrame++;

				// 공격 애니메이션 3장을 모두 보여줬다면
				if (attackFrame >= 3) {
					attackFrame = 0;              // 공격 프레임 초기화
					isAttack = false;             // 공격 상태 종료

				}

				InvalidateRect(hWnd, NULL, FALSE);
				break;
			}

			// 평상시 걷기 애니메이션
			tiggerFrame++;

			if (tiggerFrame >= 3) {
				tiggerFrame = 0;
			}

			// 먹이가 있으면 먹이 방향으로 가장 먼저 이동
			if (isFood == true) {
				MoveTiggerToFood();
			}
			// 먹이가 없고 도윤이가 있으면 도윤이 방향으로 이동
			else if (domove == true) {
				MoveTiggerToDoyoon();
			}
			// 먹이도 없고 도윤이도 없으면 랜덤 이동
			else {
				dx += moveX;
				dy += moveY;
			}
			// 먹이가 존재하고, 호랑이가 먹이와 겹쳤다면
			if (isFood == true && IsTiggerEatFood() == true) {
				isFood = false;            // 먹이 제거

				moveSpeed += 5;            // 호랑이 속도 증가

				if (moveSpeed > 200) {     // 속도가 너무 커지지 않도록 제한
					moveSpeed = 200;
				}

				SetRandomDirection();      // 증가한 속도를 랜덤 이동에도 반영
			}
			// 왼쪽 또는 오른쪽 화면 끝에 닿았을 때
			if (dx <= 0 || dx + 64 >= 1920) {

				if (dx <= 0) {
					dx = 0;
				}

				if (dx + 64 >= 1920) {
					dx = 1920 - 64;
				}

				if (domove == false && isFood == false) {
					SetRandomDirection();
				}
			}

			// 위쪽 또는 아래쪽 화면 끝에 닿았을 때
			if (dy <= 0 || dy + 64 >= 1080) {

				if (dy <= 0) {
					dy = 0;
				}

				if (dy + 64 >= 1080) {
					dy = 1080 - 64;
				}

				if (domove == false) {
					SetRandomDirection();
				}
			}

			InvalidateRect(hWnd, NULL, FALSE);
		}

		if (wParam == 2) {
			doFrame++;
			if (doFrame >= 3) {
				doFrame = 0;
			}

			InvalidateRect(hWnd, NULL, FALSE);
		}
		break;
	case WM_LBUTTONDOWN:
	{
		int mx = LOWORD(lParam);     // 마우스 클릭 x 좌표
		int my = HIWORD(lParam);     // 마우스 클릭 y 좌표

		// 호랑이를 클릭했을 때
		if (IsClickTigger(mx, my) == true) {
			isAttack = true;          // 공격 애니메이션 시작
			attackFrame = 0;          // 공격 첫 프레임부터 시작

			domove = false;           // 도윤이는 생성하지 않음
			ReleaseCapture();

			InvalidateRect(hWnd, NULL, FALSE);
			break;
		}

		// 호랑이가 아닌 곳을 클릭했을 때는 기존처럼 도윤이 생성
		dox = mx;
		doy = my;
		doFrame = 0;
		domove = true;

		SetCapture(hWnd);

		InvalidateRect(hWnd, NULL, FALSE);
		break;
	}
	case WM_LBUTTONUP:
		domove = false;           // 도윤이 출력 중지

		ReleaseCapture();         // 마우스 캡처 해제

		InvalidateRect(hWnd, NULL, FALSE);
		break;
	case WM_MOUSEMOVE:
		if (domove == true) {
			dox = LOWORD(lParam);     // 이동한 마우스 x 좌표 저장
			doy = HIWORD(lParam);     // 이동한 마우스 y 좌표 저장

			InvalidateRect(hWnd, NULL, FALSE);
		}
		break;

	case WM_RBUTTONDOWN:
		foodx = LOWORD(lParam);     // 오른쪽 클릭한 x좌표에 먹이 생성
		foody = HIWORD(lParam);     // 오른쪽 클릭한 y좌표에 먹이 생성

		isFood = true;              // 먹이가 존재하는 상태로 변경

		InvalidateRect(hWnd, NULL, FALSE);
		break;

	case WM_KEYDOWN:
		if (wParam == 'R') {
			ResetGame();                     // 프로그램 상태 초기화
			ReleaseCapture();                // 마우스 누름 상태가 남아 있으면 해제
			InvalidateRect(hWnd, NULL, FALSE); // 화면 다시 그리기
		}

		if (wParam == '+') {
			moveSpeed += 5;
			if (moveSpeed > 200) {
				moveSpeed = 5;
			}
			SetRandomDirection();   // 변경된 속도를 이동 방향에 다시 반영
			InvalidateRect(hWnd, NULL, FALSE);
		}
		if (wParam == '-') {
			moveSpeed -= 5;
			if (moveSpeed <= 0) {
				moveSpeed = 1;
			}
			SetRandomDirection();   // 변경된 속도를 이동 방향에 다시 반영
			InvalidateRect(hWnd, NULL, FALSE);
		}

		break;
	case WM_DESTROY:
		KillTimer(hWnd, 1);
		KillTimer(hWnd, 2);

		back.Destroy();
		food.Destroy();

		for (int i = 0; i < 3; ++i) {
			tigger[i].Destroy();
			doyoon[i].Destroy();
			attack[i].Destroy();
		}

		PostQuitMessage(0);
		break;
	}
	return DefWindowProc(hWnd, uMsg, wParam, lParam); //--- 위의 세 메시지 외의 나머지 메시지는 OS로
}
