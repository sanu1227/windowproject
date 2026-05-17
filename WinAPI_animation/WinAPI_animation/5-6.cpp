#include <windows.h>     // 윈도우 프로그래밍 기본 헤더
#include <tchar.h>       // 유니코드 문자열 사용을 위한 헤더
#include <atlstr.h>      // CImage 사용 시 필요한 ATL 문자열 헤더
#include <atlimage.h>    // CImage 클래스 사용 헤더
#include <stdlib.h>      // rand(), srand()
#include <time.h>        // time()
#include <math.h>        // cos(), sin()

// ============================================================
// 기본 윈도우 설정
// ============================================================

HINSTANCE g_hInst;                                           // 현재 프로그램 인스턴스
LPCTSTR lpszClass = L"My Window Class";                      // 윈도우 클래스 이름
LPCTSTR lpszWindowName = L"Rampaging Pacman";                // 윈도우 제목
LRESULT CALLBACK WndProc(HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam);

// ============================================================
// 상수 정의
// ============================================================

#define TIMER_GAME 1                 // 게임 타이머 번호
#define TIMER_INTERVAL 30            // 30ms마다 게임 갱신

#define DIR_RIGHT 0                  // 오른쪽
#define DIR_LEFT 1                   // 왼쪽
#define DIR_UP 2                     // 위쪽
#define DIR_DOWN 3                   // 아래쪽

#define MOVE_HORIZONTAL 0            // 먹이 좌우 이동
#define MOVE_VERTICAL 1              // 먹이 상하 이동

#define OBSTACLE_COUNT 10            // 장애물 개수
#define MAX_FOOD 30                   // 최대 생성 먹이 수
#define MAX_BULLET 40                 // 최대 총알 수

#define BULLET_SPEED 14              // 총알 속도
#define PACMAN_MAX_SPEED 15          // 팩맨 최대 속도

#define JUMP_TOTAL_FRAME 20           // 점프 전체 애니메이션 프레임 수
#define SPECIAL_ANIM_TIME 35          // 클릭 시 다른 애니메이션 유지 시간

#define MAX_TWIN 3                    // 트윈 팩맨 최대 개수
#define HISTORY_MAX 1200              // 팩맨 이동 기록 최대 개수
#define TWIN_DELAY 18                 // 트윈 팩맨 간격용 지연 프레임

const double PI_VALUE = 3.14159265358979323846;

// ============================================================
// 구조체 정의
// ============================================================

// 먹이 구조체
struct FOOD
{
	int x;                           // 먹이 중심 x좌표
	int y;                           // 먹이 중심 y좌표
	int radius;                      // 먹이 반지름
	int moveType;                    // 좌우 / 상하 이동
	int direction;                   // 이동 방향: 1 또는 -1
	int speed;                       // 먹이 속도
	int active;                      // 존재 여부
	COLORREF color;                  // 먹이 색상
};

// 장애물 구조체
struct OBSTACLE
{
	int x;          // 장애물 왼쪽 위 x좌표
	int y;          // 장애물 왼쪽 위 y좌표
	int width;      // 장애물 가로 크기
	int height;     // 장애물 세로 크기
	int hp;         // 현재 체력
	int maxHp;      // 최대 체력
	int active;     // 존재 여부
};

// 총알 구조체
struct BULLET
{
	int x;                           // 총알 중심 x좌표
	int y;                           // 총알 중심 y좌표
	int dx;                          // x축 이동량
	int dy;                          // y축 이동량
	int radius;                      // 총알 반지름
	int active;                      // 존재 여부
};

// ============================================================
// 전역 변수
// ============================================================

RECT g_rectView;                     // 화면 클라이언트 영역
CImage back;                         // 배경 이미지

FOOD g_food[MAX_FOOD];               // 먹이 배열
OBSTACLE g_obstacle[OBSTACLE_COUNT]; // 장애물 배열
BULLET g_bullet[MAX_BULLET];         // 총알 배열

int g_createdFoodCount = 0;          // 지금까지 생성된 먹이 수
int g_activeFoodCount = 0;           // 현재 화면에 존재하는 먹이 수
int g_eatenFoodCount = 0;            // 팩맨이 먹은 먹이 수

int g_pacmanX = 100;                 // 팩맨 중심 x좌표
int g_pacmanY = 300;                 // 팩맨 중심 y좌표
int g_pacmanRadius = 32;             // 팩맨 크기
int g_pacmanSpeed = 4;               // 팩맨 속도
int g_pacmanDirection = DIR_RIGHT;   // 팩맨 방향
COLORREF g_pacmanColor = RGB(255, 230, 0); // 팩맨 색상

int g_mouthFrame = 0;                // 입 애니메이션 프레임
int g_animationCounter = 0;          // 애니메이션 속도 조절

int g_jumpFrame = 0;                 // 점프 진행 프레임, 0이면 점프 안 함
int g_specialAnimationTimer = 0;     // 왼쪽 클릭 시 다른 애니메이션 유지 시간

int g_twinCount = 0;                 // 현재 트윈 팩맨 개수

int g_historyX[HISTORY_MAX];         // 팩맨의 과거 x좌표 기록
int g_historyY[HISTORY_MAX];         // 팩맨의 과거 y좌표 기록
int g_historyDir[HISTORY_MAX];       // 팩맨의 과거 방향 기록
int g_historyIndex = 0;              // 현재 기록 위치
int g_historyCount = 0;              // 실제 저장된 기록 개수

// ============================================================
// 함수 원형 선언
// ============================================================

int RandomRange(int minValue, int maxValue);
int ClampValue(int value, int minValue, int maxValue);
COLORREF RandomBrightColor(void);
int CanPacmanMoveTo(int nextX, int nextY);

int GetJumpOffset(void);
int GetPacmanDrawY(void);

int IsCircleCircleHit(int x1, int y1, int r1, int x2, int y2, int r2);
int IsCircleRectHit(int cx, int cy, int radius, int rx, int ry, int rw, int rh);
int IsPointInPacman(int mx, int my);
int IsPointInObstacle(int mx, int my, int index);

void InitGame(void);
void InitObstacle(int index);

void ClearAllFoods(void);
int CreateFood(int originX, int originY, int splitMode);
void CreateSequentialFoodIfNeeded(void);

void FireBullet(void);
void SavePacmanHistory(void);

void UpdateGame(HWND hWnd);
void UpdatePacman(void);
void UpdateFoods(void);
void UpdateBullets(void);
void CheckPacmanEatFoods(void);

void PacmanClickedAction(void);
void MoveClickedObstacle(int mx, int my);

void DrawGame(HDC hDC);
void DrawBackground(HDC hDC);
void DrawObstacles(HDC hDC);
void DrawFoods(HDC hDC);
void DrawBullets(HDC hDC);
void DrawTwins(HDC hDC);
void DrawMainPacman(HDC hDC);
void DrawOnePacman(HDC hDC, int centerX, int centerY, int radius, int direction, COLORREF color, int specialMode);
void DrawHud(HDC hDC);

// ============================================================
// WinMain
// ============================================================

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
		1100,
		750,
		NULL,
		(HMENU)NULL,
		hInstance,
		NULL
	);

	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);

	while (GetMessage(&Message, 0, 0, 0))
	{
		TranslateMessage(&Message);
		DispatchMessage(&Message);
	}

	return Message.wParam;
}

// ============================================================
// 윈도우 메시지 처리
// ============================================================

LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	PAINTSTRUCT ps;
	HDC hDC;

	HDC hMemDC;
	HBITMAP hMemBitmap;
	HBITMAP hOldBitmap;

	int clientWidth;
	int clientHeight;

	int mx;
	int my;

	switch (uMsg)
	{
	case WM_CREATE:
		srand((unsigned int)time(NULL));              // 랜덤 초기화
		GetClientRect(hWnd, &g_rectView);             // 화면 크기 저장
		back.Load(_T("back.png"));                    // 배경 이미지 불러오기
		InitGame();                                   // 게임 초기화
		SetTimer(hWnd, TIMER_GAME, TIMER_INTERVAL, NULL);
		return 0;

	case WM_SIZE:
		GetClientRect(hWnd, &g_rectView);
		return 0;

	case WM_TIMER:
		if (wParam == TIMER_GAME)
		{
			UpdateGame(hWnd);
		}
		return 0;

	case WM_KEYDOWN:
		switch (wParam)
		{
		case VK_RIGHT:
			g_pacmanDirection = DIR_RIGHT;
			break;

		case VK_LEFT:
			g_pacmanDirection = DIR_LEFT;
			break;

		case VK_UP:
			g_pacmanDirection = DIR_UP;
			break;

		case VK_DOWN:
			g_pacmanDirection = DIR_DOWN;
			break;

		case VK_RETURN:
			FireBullet();
			break;

		case 'J':
			if (g_jumpFrame == 0)
			{
				g_jumpFrame = 1;
			}
			break;

		case 'T':
			if (g_twinCount < MAX_TWIN)
			{
				g_twinCount++;
			}
			break;

		case 'A':
			ClearAllFoods();
			CreateSequentialFoodIfNeeded();
			break;

		case 'R':
			InitGame();
			break;
		}

		InvalidateRect(hWnd, NULL, FALSE);
		return 0;

	case WM_LBUTTONDOWN:
		mx = LOWORD(lParam);
		my = HIWORD(lParam);

		if (IsPointInPacman(mx, my) == 1)
		{
			PacmanClickedAction();
		}

		InvalidateRect(hWnd, NULL, FALSE);
		return 0;

	case WM_RBUTTONDOWN:
		mx = LOWORD(lParam);
		my = HIWORD(lParam);

		MoveClickedObstacle(mx, my);

		InvalidateRect(hWnd, NULL, FALSE);
		return 0;

	case WM_PAINT:
		hDC = BeginPaint(hWnd, &ps);

		clientWidth = g_rectView.right - g_rectView.left;
		clientHeight = g_rectView.bottom - g_rectView.top;

		hMemDC = CreateCompatibleDC(hDC);
		hMemBitmap = CreateCompatibleBitmap(hDC, clientWidth, clientHeight);
		hOldBitmap = (HBITMAP)SelectObject(hMemDC, hMemBitmap);

		DrawGame(hMemDC);

		BitBlt(
			hDC,
			0,
			0,
			clientWidth,
			clientHeight,
			hMemDC,
			0,
			0,
			SRCCOPY
		);

		SelectObject(hMemDC, hOldBitmap);
		DeleteObject(hMemBitmap);
		DeleteDC(hMemDC);

		EndPaint(hWnd, &ps);
		return 0;

	case WM_DESTROY:
		KillTimer(hWnd, TIMER_GAME);

		if (!back.IsNull())
		{
			back.Destroy();
		}

		PostQuitMessage(0);
		return 0;
	}

	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

// ============================================================
// 랜덤 정수
// ============================================================

int RandomRange(int minValue, int maxValue)
{
	if (maxValue <= minValue)
	{
		return minValue;
	}

	return minValue + rand() % (maxValue - minValue + 1);
}

// ============================================================
// 값 제한
// ============================================================

int ClampValue(int value, int minValue, int maxValue)
{
	if (value < minValue)
	{
		return minValue;
	}

	if (value > maxValue)
	{
		return maxValue;
	}

	return value;
}

// ============================================================
// 밝은 랜덤 색상
// ============================================================

COLORREF RandomBrightColor(void)
{
	int r = RandomRange(80, 255);
	int g = RandomRange(80, 255);
	int b = RandomRange(80, 255);

	return RGB(r, g, b);
}

// ============================================================
// 점프 높이 계산
// ============================================================

int GetJumpOffset(void)
{
	int halfFrame;

	if (g_jumpFrame == 0)
	{
		return 0;
	}

	halfFrame = JUMP_TOTAL_FRAME / 2;

	if (g_jumpFrame <= halfFrame)
	{
		return g_jumpFrame * 4;
	}
	else
	{
		return (JUMP_TOTAL_FRAME - g_jumpFrame) * 4;
	}
}

// ============================================================
// 실제로 그려지는 팩맨 y좌표
// ============================================================

int GetPacmanDrawY(void)
{
	return g_pacmanY - GetJumpOffset();
}

// ============================================================
// 원-원 충돌
// ============================================================

int IsCircleCircleHit(int x1, int y1, int r1, int x2, int y2, int r2)
{
	int dx = x1 - x2;
	int dy = y1 - y2;
	int radiusSum = r1 + r2;

	if (dx * dx + dy * dy <= radiusSum * radiusSum)
	{
		return 1;
	}

	return 0;
}

// ============================================================
// 원-사각형 충돌
// ============================================================

int IsCircleRectHit(int cx, int cy, int radius, int rx, int ry, int rw, int rh)
{
	int nearestX;
	int nearestY;
	int dx;
	int dy;

	nearestX = ClampValue(cx, rx, rx + rw);
	nearestY = ClampValue(cy, ry, ry + rh);

	dx = cx - nearestX;
	dy = cy - nearestY;

	if (dx * dx + dy * dy <= radius * radius)
	{
		return 1;
	}

	return 0;
}

// ============================================================
// 마우스가 팩맨 안에 있는지 검사
// ============================================================

int IsPointInPacman(int mx, int my)
{
	int dx = mx - g_pacmanX;
	int dy = my - GetPacmanDrawY();

	if (dx * dx + dy * dy <= g_pacmanRadius * g_pacmanRadius)
	{
		return 1;
	}

	return 0;
}

// ============================================================
// 마우스가 장애물 안에 있는지 검사
// ============================================================

int IsPointInObstacle(int mx, int my, int index)
{
	if (g_obstacle[index].active == 0)
	{
		return 0;
	}

	if (
		mx >= g_obstacle[index].x &&
		mx <= g_obstacle[index].x + g_obstacle[index].width &&
		my >= g_obstacle[index].y &&
		my <= g_obstacle[index].y + g_obstacle[index].height
		)
	{
		return 1;
	}

	return 0;
}

// ============================================================
// 게임 초기화
// ============================================================

void InitGame(void)
{
	int i;

	g_pacmanX = 100;
	g_pacmanY = g_rectView.bottom / 2;
	g_pacmanRadius = 32;
	g_pacmanSpeed = 4;
	g_pacmanDirection = DIR_RIGHT;
	g_pacmanColor = RGB(255, 230, 0);

	g_mouthFrame = 0;
	g_animationCounter = 0;

	g_jumpFrame = 0;
	g_specialAnimationTimer = 0;

	g_twinCount = 0;

	g_createdFoodCount = 0;
	g_activeFoodCount = 0;
	g_eatenFoodCount = 0;

	g_historyIndex = 0;
	g_historyCount = 0;

	for (i = 0; i < MAX_FOOD; i++)
	{
		g_food[i].active = 0;
	}

	for (i = 0; i < MAX_BULLET; i++)
	{
		g_bullet[i].active = 0;
		g_bullet[i].radius = 6;
	}

	for (i = 0; i < OBSTACLE_COUNT; i++)
	{
		InitObstacle(i);
	}

	CreateSequentialFoodIfNeeded();
	SavePacmanHistory();
}

void InitObstacle(int index)
{
	int sizeType;
	int size;
	int retryCount = 0;

	sizeType = RandomRange(0, 2);

	if (sizeType == 0)
	{
		size = 50;                      // 작은 장애물
		g_obstacle[index].hp = 2;       // 총알 2발
	}
	else if (sizeType == 1)
	{
		size = 75;                      // 중간 장애물
		g_obstacle[index].hp = 4;       // 총알 4발
	}
	else
	{
		size = 100;                     // 큰 장애물
		g_obstacle[index].hp = 6;       // 총알 6발
	}

	g_obstacle[index].width = size;
	g_obstacle[index].height = size;
	g_obstacle[index].maxHp = g_obstacle[index].hp;
	g_obstacle[index].active = 1;

	do
	{
		g_obstacle[index].x = RandomRange(220, g_rectView.right - size - 30);
		g_obstacle[index].y = RandomRange(80, g_rectView.bottom - size - 60);

		retryCount++;
	} while (
		IsCircleRectHit(
			g_pacmanX,
			g_pacmanY,
			g_pacmanRadius + 40,
			g_obstacle[index].x,
			g_obstacle[index].y,
			g_obstacle[index].width,
			g_obstacle[index].height
		)
		&& retryCount < 50
		);
}

// ============================================================
// 모든 먹이 제거
// ============================================================

void ClearAllFoods(void)
{
	int i;

	for (i = 0; i < MAX_FOOD; i++)
	{
		g_food[i].active = 0;
	}

	g_activeFoodCount = 0;
}

// ============================================================
// 먹이 생성
// splitMode == 0 : 일반 랜덤 생성
// splitMode == 1 : 총알에 맞은 위치 근처에서 분열 생성
// ============================================================

int CreateFood(int originX, int originY, int splitMode)
{
	int i;
	int slot = -1;
	int retryCount = 0;
	int overlap;
	int j;

	if (g_createdFoodCount >= MAX_FOOD)
	{
		return 0;
	}

	for (i = 0; i < MAX_FOOD; i++)
	{
		if (g_food[i].active == 0)
		{
			slot = i;
			break;
		}
	}

	if (slot == -1)
	{
		return 0;
	}

	g_food[slot].radius = 13;
	g_food[slot].moveType = RandomRange(0, 1);
	g_food[slot].direction = RandomRange(0, 1) == 0 ? -1 : 1;
	g_food[slot].speed = RandomRange(2, 5);
	g_food[slot].color = RandomBrightColor();
	g_food[slot].active = 1;

	if (splitMode == 1)
	{
		g_food[slot].x = ClampValue(
			originX + RandomRange(-45, 45),
			30,
			g_rectView.right - 30
		);

		g_food[slot].y = ClampValue(
			originY + RandomRange(-45, 45),
			50,
			g_rectView.bottom - 30
		);
	}
	else
	{
		do
		{
			overlap = 0;

			g_food[slot].x = RandomRange(40, g_rectView.right - 40);
			g_food[slot].y = RandomRange(70, g_rectView.bottom - 40);

			if (
				IsCircleCircleHit(
					g_food[slot].x,
					g_food[slot].y,
					g_food[slot].radius,
					g_pacmanX,
					GetPacmanDrawY(),
					g_pacmanRadius + 25
				)
				)
			{
				overlap = 1;
			}

			for (j = 0; j < OBSTACLE_COUNT; j++)
			{
				if (
					g_obstacle[j].active == 1 &&
					IsCircleRectHit(
						g_food[slot].x,
						g_food[slot].y,
						g_food[slot].radius,
						g_obstacle[j].x,
						g_obstacle[j].y,
						g_obstacle[j].width,
						g_obstacle[j].height
					)
					)
				{
					overlap = 1;
				}
			}

			retryCount++;
		} while (overlap == 1 && retryCount < 100);
	}

	g_createdFoodCount++;
	g_activeFoodCount++;

	return 1;
}

// ============================================================
// 먹이가 하나도 없으면 다음 먹이 1개 생성
// ============================================================

void CreateSequentialFoodIfNeeded(void)
{
	if (g_activeFoodCount == 0 && g_createdFoodCount < MAX_FOOD)
	{
		CreateFood(0, 0, 0);
	}
}

// ============================================================
// 총알 발사
// ============================================================

void FireBullet(void)
{
	int i;
	int drawY;

	drawY = GetPacmanDrawY();

	for (i = 0; i < MAX_BULLET; i++)
	{
		if (g_bullet[i].active == 0)
		{
			g_bullet[i].active = 1;
			g_bullet[i].radius = 6;

			if (g_pacmanDirection == DIR_RIGHT)
			{
				g_bullet[i].x = g_pacmanX + g_pacmanRadius + 8;
				g_bullet[i].y = drawY;
				g_bullet[i].dx = BULLET_SPEED;
				g_bullet[i].dy = 0;
			}
			else if (g_pacmanDirection == DIR_LEFT)
			{
				g_bullet[i].x = g_pacmanX - g_pacmanRadius - 8;
				g_bullet[i].y = drawY;
				g_bullet[i].dx = -BULLET_SPEED;
				g_bullet[i].dy = 0;
			}
			else if (g_pacmanDirection == DIR_UP)
			{
				g_bullet[i].x = g_pacmanX;
				g_bullet[i].y = drawY - g_pacmanRadius - 8;
				g_bullet[i].dx = 0;
				g_bullet[i].dy = -BULLET_SPEED;
			}
			else
			{
				g_bullet[i].x = g_pacmanX;
				g_bullet[i].y = drawY + g_pacmanRadius + 8;
				g_bullet[i].dx = 0;
				g_bullet[i].dy = BULLET_SPEED;
			}

			break;
		}
	}
}

// ============================================================
// 팩맨 이동 기록 저장
// ============================================================

void SavePacmanHistory(void)
{
	g_historyX[g_historyIndex] = g_pacmanX;
	g_historyY[g_historyIndex] = GetPacmanDrawY();
	g_historyDir[g_historyIndex] = g_pacmanDirection;

	g_historyIndex++;

	if (g_historyIndex >= HISTORY_MAX)
	{
		g_historyIndex = 0;
	}

	if (g_historyCount < HISTORY_MAX)
	{
		g_historyCount++;
	}
}

// ============================================================
// 게임 전체 업데이트
// ============================================================

void UpdateGame(HWND hWnd)
{
	UpdatePacman();
	UpdateFoods();
	UpdateBullets();
	CheckPacmanEatFoods();

	InvalidateRect(hWnd, NULL, FALSE);
}

// ============================================================
// 팩맨 이동 / 애니메이션 처리
// 장애물과 충돌하면 이동하지 않음
// ============================================================

void UpdatePacman(void)
{
	int nextX;
	int nextY;

	nextX = g_pacmanX;
	nextY = g_pacmanY;

	// --------------------------------------------------------
	// 현재 방향을 기준으로 "이동 예정 위치"를 먼저 계산
	// --------------------------------------------------------

	if (g_pacmanDirection == DIR_RIGHT)
	{
		nextX += g_pacmanSpeed;
	}
	else if (g_pacmanDirection == DIR_LEFT)
	{
		nextX -= g_pacmanSpeed;
	}
	else if (g_pacmanDirection == DIR_UP)
	{
		nextY -= g_pacmanSpeed;
	}
	else if (g_pacmanDirection == DIR_DOWN)
	{
		nextY += g_pacmanSpeed;
	}

	// --------------------------------------------------------
	// 화면 밖으로 나가지 않게 보정
	// --------------------------------------------------------

	if (nextX - g_pacmanRadius < 0)
	{
		nextX = g_pacmanRadius;
		g_pacmanDirection = DIR_RIGHT;
	}

	if (nextX + g_pacmanRadius > g_rectView.right)
	{
		nextX = g_rectView.right - g_pacmanRadius;
		g_pacmanDirection = DIR_LEFT;
	}

	if (nextY - g_pacmanRadius < 0)
	{
		nextY = g_pacmanRadius;
		g_pacmanDirection = DIR_DOWN;
	}

	if (nextY + g_pacmanRadius > g_rectView.bottom)
	{
		nextY = g_rectView.bottom - g_pacmanRadius;
		g_pacmanDirection = DIR_UP;
	}

	// --------------------------------------------------------
	// 다음 위치에 장애물이 없을 때만 실제 이동
	// --------------------------------------------------------

	if (CanPacmanMoveTo(nextX, nextY) == 1)
	{
		g_pacmanX = nextX;
		g_pacmanY = nextY;
	}

	// --------------------------------------------------------
	// 팩맨 입 애니메이션
	// --------------------------------------------------------

	g_animationCounter++;

	if (g_animationCounter >= 4)
	{
		g_mouthFrame++;

		if (g_mouthFrame >= 3)
		{
			g_mouthFrame = 0;
		}

		g_animationCounter = 0;
	}

	// --------------------------------------------------------
	// 점프 애니메이션 진행
	// --------------------------------------------------------

	if (g_jumpFrame > 0)
	{
		g_jumpFrame++;

		if (g_jumpFrame > JUMP_TOTAL_FRAME)
		{
			g_jumpFrame = 0;
		}
	}

	// --------------------------------------------------------
	// 왼쪽 클릭 특별 애니메이션 시간 감소
	// --------------------------------------------------------

	if (g_specialAnimationTimer > 0)
	{
		g_specialAnimationTimer--;
	}

	// --------------------------------------------------------
	// 트윈 팩맨이 따라오기 위한 이동 기록 저장
	// --------------------------------------------------------

	SavePacmanHistory();
}

// ============================================================
// 먹이 이동
// ============================================================

void UpdateFoods(void)
{
	int i;

	for (i = 0; i < MAX_FOOD; i++)
	{
		if (g_food[i].active == 0)
		{
			continue;
		}

		if (g_food[i].moveType == MOVE_HORIZONTAL)
		{
			g_food[i].x += g_food[i].direction * g_food[i].speed;

			if (g_food[i].x - g_food[i].radius < 0)
			{
				g_food[i].x = g_food[i].radius;
				g_food[i].direction = 1;
			}

			if (g_food[i].x + g_food[i].radius > g_rectView.right)
			{
				g_food[i].x = g_rectView.right - g_food[i].radius;
				g_food[i].direction = -1;
			}
		}
		else
		{
			g_food[i].y += g_food[i].direction * g_food[i].speed;

			if (g_food[i].y - g_food[i].radius < 0)
			{
				g_food[i].y = g_food[i].radius;
				g_food[i].direction = 1;
			}

			if (g_food[i].y + g_food[i].radius > g_rectView.bottom)
			{
				g_food[i].y = g_rectView.bottom - g_food[i].radius;
				g_food[i].direction = -1;
			}
		}
	}
}
// ============================================================
// 팩맨이 다음 위치로 이동 가능한지 검사
// 장애물과 부딪히면 0, 이동 가능하면 1 반환
// ============================================================

int CanPacmanMoveTo(int nextX, int nextY)
{
	int i;

	for (i = 0; i < OBSTACLE_COUNT; i++)
	{
		if (g_obstacle[i].active == 0)
		{
			continue;
		}

		if (
			IsCircleRectHit(
				nextX,
				nextY,
				g_pacmanRadius,
				g_obstacle[i].x,
				g_obstacle[i].y,
				g_obstacle[i].width,
				g_obstacle[i].height
			)
			)
		{
			return 0;
		}
	}

	return 1;
}
// ============================================================
// 총알 이동 및 충돌 처리
// ============================================================

void UpdateBullets(void)
{
	int i;
	int j;
	int hit;
	int oldFoodX;
	int oldFoodY;

	for (i = 0; i < MAX_BULLET; i++)
	{
		if (g_bullet[i].active == 0)
		{
			continue;
		}

		g_bullet[i].x += g_bullet[i].dx;
		g_bullet[i].y += g_bullet[i].dy;

		if (
			g_bullet[i].x < -30 ||
			g_bullet[i].x > g_rectView.right + 30 ||
			g_bullet[i].y < -30 ||
			g_bullet[i].y > g_rectView.bottom + 30
			)
		{
			g_bullet[i].active = 0;
			continue;
		}

		hit = 0;

		// --------------------------------------------------------
		// 총알과 장애물 충돌
		// 충돌하면 장애물은 즉시 사라진다.
		// --------------------------------------------------------

		for (j = 0; j < OBSTACLE_COUNT; j++)
		{
			if (g_obstacle[j].active == 0)
			{
				continue;
			}

			if (
				IsCircleRectHit(
					g_bullet[i].x,
					g_bullet[i].y,
					g_bullet[i].radius,
					g_obstacle[j].x,
					g_obstacle[j].y,
					g_obstacle[j].width,
					g_obstacle[j].height
				)
				)
			{
				g_bullet[i].active = 0;      // 총알은 충돌하면 사라짐
				g_obstacle[j].hp--;          // 장애물 체력 1 감소
				hit = 1;

				// 체력이 모두 깎였을 때만 장애물 파괴
				if (g_obstacle[j].hp <= 0)
				{
					g_obstacle[j].active = 0;

					// 장애물이 완전히 파괴될 때만 팩맨 속도 증가
					if (g_pacmanSpeed < PACMAN_MAX_SPEED)
					{
						g_pacmanSpeed++;
					}
				}

				break;
			}
		}

		if (hit == 1)
		{
			continue;
		}

		// --------------------------------------------------------
		// 총알과 먹이 충돌
		// 먹이 하나가 사라지고, 그 위치 근처에 2개 생성
		// --------------------------------------------------------

		for (j = 0; j < MAX_FOOD; j++)
		{
			if (g_food[j].active == 0)
			{
				continue;
			}

			if (
				IsCircleCircleHit(
					g_bullet[i].x,
					g_bullet[i].y,
					g_bullet[i].radius,
					g_food[j].x,
					g_food[j].y,
					g_food[j].radius
				)
				)
			{
				oldFoodX = g_food[j].x;
				oldFoodY = g_food[j].y;

				g_bullet[i].active = 0;
				g_food[j].active = 0;
				g_activeFoodCount--;

				CreateFood(oldFoodX, oldFoodY, 1);
				CreateFood(oldFoodX, oldFoodY, 1);

				break;
			}
		}
	}
}

// ============================================================
// 팩맨이 먹이를 먹는 처리
// ============================================================

void CheckPacmanEatFoods(void)
{
	int i;
	int pacmanDrawY;

	pacmanDrawY = GetPacmanDrawY();

	for (i = 0; i < MAX_FOOD; i++)
	{
		if (g_food[i].active == 0)
		{
			continue;
		}

		if (
			IsCircleCircleHit(
				g_pacmanX,
				pacmanDrawY,
				g_pacmanRadius,
				g_food[i].x,
				g_food[i].y,
				g_food[i].radius
			)
			)
		{
			g_pacmanColor = g_food[i].color;
			g_food[i].active = 0;
			g_activeFoodCount--;
			g_eatenFoodCount++;

			if (g_pacmanRadius < 80)
			{
				g_pacmanRadius += 3;
			}
		}
	}

	CreateSequentialFoodIfNeeded();
}

// ============================================================
// 왼쪽 클릭으로 팩맨 클릭 시 동작
// ============================================================

void PacmanClickedAction(void)
{
	int newDirection;

	g_specialAnimationTimer = SPECIAL_ANIM_TIME;

	do
	{
		newDirection = RandomRange(0, 3);
	} while (newDirection == g_pacmanDirection);

	g_pacmanDirection = newDirection;
}

// ============================================================
// 오른쪽 클릭으로 장애물 위치 조금 변경
// ============================================================

void MoveClickedObstacle(int mx, int my)
{
	int i;

	for (i = 0; i < OBSTACLE_COUNT; i++)
	{
		if (IsPointInObstacle(mx, my, i) == 1)
		{
			g_obstacle[i].x += RandomRange(-45, 45);
			g_obstacle[i].y += RandomRange(-45, 45);

			g_obstacle[i].x = ClampValue(
				g_obstacle[i].x,
				10,
				g_rectView.right - g_obstacle[i].width - 10
			);

			g_obstacle[i].y = ClampValue(
				g_obstacle[i].y,
				70,
				g_rectView.bottom - g_obstacle[i].height - 10
			);

			break;
		}
	}
}

// ============================================================
// 전체 그리기
// ============================================================

void DrawGame(HDC hDC)
{
	DrawBackground(hDC);
	DrawObstacles(hDC);
	DrawFoods(hDC);
	DrawBullets(hDC);
	DrawTwins(hDC);
	DrawMainPacman(hDC);
	DrawHud(hDC);
}

// ============================================================
// 배경 그리기
// ============================================================

void DrawBackground(HDC hDC)
{
	HBRUSH hBrush;

	if (!back.IsNull())
	{
		back.Draw(
			hDC,
			0,
			0,
			g_rectView.right,
			g_rectView.bottom
		);
	}
	else
	{
		hBrush = CreateSolidBrush(RGB(25, 25, 35));
		FillRect(hDC, &g_rectView, hBrush);
		DeleteObject(hBrush);
	}
}

// ============================================================
// 장애물 그리기
// ============================================================

void DrawObstacles(HDC hDC)
{
	int i;

	HBRUSH hBrush;
	HPEN hPen;
	HBRUSH hOldBrush;
	HPEN hOldPen;

	for (i = 0; i < OBSTACLE_COUNT; i++)
	{
		if (g_obstacle[i].active == 0)
		{
			continue;
		}

		hBrush = CreateSolidBrush(RGB(120, 120, 120));
		hPen = CreatePen(PS_SOLID, 3, RGB(40, 40, 40));

		hOldBrush = (HBRUSH)SelectObject(hDC, hBrush);
		hOldPen = (HPEN)SelectObject(hDC, hPen);

		RoundRect(
			hDC,
			g_obstacle[i].x,
			g_obstacle[i].y,
			g_obstacle[i].x + g_obstacle[i].width,
			g_obstacle[i].y + g_obstacle[i].height,
			14,
			14
		);

		SelectObject(hDC, hOldBrush);
		SelectObject(hDC, hOldPen);

		DeleteObject(hBrush);
		DeleteObject(hPen);
	}
}

// ============================================================
// 먹이 그리기
// ============================================================

void DrawFoods(HDC hDC)
{
	int i;

	HBRUSH hBrush;
	HPEN hPen;
	HBRUSH hOldBrush;
	HPEN hOldPen;

	for (i = 0; i < MAX_FOOD; i++)
	{
		if (g_food[i].active == 0)
		{
			continue;
		}

		hBrush = CreateSolidBrush(g_food[i].color);
		hPen = CreatePen(PS_SOLID, 2, RGB(20, 20, 20));

		hOldBrush = (HBRUSH)SelectObject(hDC, hBrush);
		hOldPen = (HPEN)SelectObject(hDC, hPen);

		Ellipse(
			hDC,
			g_food[i].x - g_food[i].radius,
			g_food[i].y - g_food[i].radius,
			g_food[i].x + g_food[i].radius,
			g_food[i].y + g_food[i].radius
		);

		SelectObject(hDC, hOldBrush);
		SelectObject(hDC, hOldPen);

		DeleteObject(hBrush);
		DeleteObject(hPen);
	}
}

// ============================================================
// 총알 그리기
// ============================================================

void DrawBullets(HDC hDC)
{
	int i;

	HBRUSH hBrush;
	HPEN hPen;
	HBRUSH hOldBrush;
	HPEN hOldPen;

	hBrush = CreateSolidBrush(RGB(255, 210, 60));
	hPen = CreatePen(PS_SOLID, 2, RGB(80, 50, 0));

	hOldBrush = (HBRUSH)SelectObject(hDC, hBrush);
	hOldPen = (HPEN)SelectObject(hDC, hPen);

	for (i = 0; i < MAX_BULLET; i++)
	{
		if (g_bullet[i].active == 0)
		{
			continue;
		}

		Ellipse(
			hDC,
			g_bullet[i].x - g_bullet[i].radius,
			g_bullet[i].y - g_bullet[i].radius,
			g_bullet[i].x + g_bullet[i].radius,
			g_bullet[i].y + g_bullet[i].radius
		);
	}

	SelectObject(hDC, hOldBrush);
	SelectObject(hDC, hOldPen);

	DeleteObject(hBrush);
	DeleteObject(hPen);
}

// ============================================================
// 트윈 팩맨 그리기
// ============================================================

void DrawTwins(HDC hDC)
{
	int twinIndex;
	int framesBack;
	int historyPosition;
	int drawX;
	int drawY;
	int drawDirection;
	int fallbackGap;

	for (twinIndex = 1; twinIndex <= g_twinCount; twinIndex++)
	{
		framesBack = twinIndex * TWIN_DELAY;

		if (g_historyCount > framesBack)
		{
			historyPosition = g_historyIndex - framesBack;

			while (historyPosition < 0)
			{
				historyPosition += HISTORY_MAX;
			}

			drawX = g_historyX[historyPosition];
			drawY = g_historyY[historyPosition];
			drawDirection = g_historyDir[historyPosition];
		}
		else
		{
			fallbackGap = twinIndex * (g_pacmanRadius * 2 + 20);

			drawX = g_pacmanX;
			drawY = GetPacmanDrawY();
			drawDirection = g_pacmanDirection;

			if (g_pacmanDirection == DIR_RIGHT)
			{
				drawX -= fallbackGap;
			}
			else if (g_pacmanDirection == DIR_LEFT)
			{
				drawX += fallbackGap;
			}
			else if (g_pacmanDirection == DIR_UP)
			{
				drawY += fallbackGap;
			}
			else
			{
				drawY -= fallbackGap;
			}
		}

		DrawOnePacman(
			hDC,
			drawX,
			drawY,
			g_pacmanRadius,
			drawDirection,
			g_pacmanColor,
			0
		);
	}
}

// ============================================================
// 메인 팩맨 그리기
// ============================================================

void DrawMainPacman(HDC hDC)
{
	DrawOnePacman(
		hDC,
		g_pacmanX,
		GetPacmanDrawY(),
		g_pacmanRadius,
		g_pacmanDirection,
		g_pacmanColor,
		g_specialAnimationTimer > 0
	);
}

// ============================================================
// 팩맨 1마리 그리기
// Pie()를 사용하여 입이 열리고 닫히는 애니메이션 구현
// ============================================================

void DrawOnePacman(HDC hDC, int centerX, int centerY, int radius, int direction, COLORREF color, int specialMode)
{
	double normalAngle[3] = { 6.0, 22.0, 42.0 };
	double specialAngle[3] = { 60.0, 15.0, 60.0 };

	double angleDegree;
	double angleRadian;
	double cosValue;
	double sinValue;

	int left;
	int top;
	int right;
	int bottom;

	int startX;
	int startY;
	int endX;
	int endY;

	int eyeX;
	int eyeY;

	int specialFrame;

	HBRUSH hBodyBrush;
	HBRUSH hEyeBrush;
	HPEN hPen;

	HBRUSH hOldBrush;
	HPEN hOldPen;

	int oldArcDirection;

	left = centerX - radius;
	top = centerY - radius;
	right = centerX + radius;
	bottom = centerY + radius;

	if (specialMode == 1)
	{
		specialFrame = (g_specialAnimationTimer / 3) % 3;
		angleDegree = specialAngle[specialFrame];
	}
	else
	{
		angleDegree = normalAngle[g_mouthFrame];
	}

	angleRadian = angleDegree * PI_VALUE / 180.0;
	cosValue = cos(angleRadian);
	sinValue = sin(angleRadian);

	if (direction == DIR_RIGHT)
	{
		startX = centerX + (int)(radius * cosValue);
		startY = centerY + (int)(radius * sinValue);

		endX = centerX + (int)(radius * cosValue);
		endY = centerY - (int)(radius * sinValue);

		eyeX = centerX + radius / 5;
		eyeY = centerY - radius / 3;
	}
	else if (direction == DIR_LEFT)
	{
		startX = centerX - (int)(radius * cosValue);
		startY = centerY - (int)(radius * sinValue);

		endX = centerX - (int)(radius * cosValue);
		endY = centerY + (int)(radius * sinValue);

		eyeX = centerX - radius / 5;
		eyeY = centerY - radius / 3;
	}
	else if (direction == DIR_UP)
	{
		startX = centerX + (int)(radius * sinValue);
		startY = centerY - (int)(radius * cosValue);

		endX = centerX - (int)(radius * sinValue);
		endY = centerY - (int)(radius * cosValue);

		eyeX = centerX - radius / 3;
		eyeY = centerY - radius / 5;
	}
	else
	{
		startX = centerX - (int)(radius * sinValue);
		startY = centerY + (int)(radius * cosValue);

		endX = centerX + (int)(radius * sinValue);
		endY = centerY + (int)(radius * cosValue);

		eyeX = centerX + radius / 3;
		eyeY = centerY - radius / 5;
	}

	hBodyBrush = CreateSolidBrush(color);

	if (specialMode == 1)
	{
		hPen = CreatePen(PS_SOLID, 4, RGB(255, 70, 70));
	}
	else
	{
		hPen = CreatePen(PS_SOLID, 3, RGB(20, 20, 20));
	}

	hOldBrush = (HBRUSH)SelectObject(hDC, hBodyBrush);
	hOldPen = (HPEN)SelectObject(hDC, hPen);

	oldArcDirection = SetArcDirection(hDC, AD_CLOCKWISE);

	Pie(
		hDC,
		left,
		top,
		right,
		bottom,
		startX,
		startY,
		endX,
		endY
	);

	SetArcDirection(hDC, oldArcDirection);

	SelectObject(hDC, hOldBrush);
	SelectObject(hDC, hOldPen);

	DeleteObject(hBodyBrush);
	DeleteObject(hPen);

	hEyeBrush = CreateSolidBrush(RGB(20, 20, 20));
	hOldBrush = (HBRUSH)SelectObject(hDC, hEyeBrush);

	Ellipse(
		hDC,
		eyeX - 4,
		eyeY - 4,
		eyeX + 4,
		eyeY + 4
	);

	SelectObject(hDC, hOldBrush);
	DeleteObject(hEyeBrush);
}

// ============================================================
// 게임 정보 출력
// ============================================================

void DrawHud(HDC hDC)
{
	int i;
	int remainObstacle = 0;

	TCHAR text1[256];
	TCHAR text2[256];
	TCHAR text3[256];

	for (i = 0; i < OBSTACLE_COUNT; i++)
	{
		if (g_obstacle[i].active == 1)
		{
			remainObstacle++;
		}
	}

	SetBkMode(hDC, TRANSPARENT);
	SetTextColor(hDC, RGB(255, 255, 255));

	wsprintf(
		text1,
		_T("먹은 먹이: %d   생성된 먹이: %d / %d   현재 먹이: %d   남은 장애물: %d"),
		g_eatenFoodCount,
		g_createdFoodCount,
		MAX_FOOD,
		g_activeFoodCount,
		remainObstacle
	);

	TextOut(hDC, 20, 20, text1, lstrlen(text1));

	wsprintf(
		text2,
		_T("팩맨 속도: %d   팩맨 크기: %d   트윈 팩맨: %d / 3"),
		g_pacmanSpeed,
		g_pacmanRadius,
		g_twinCount
	);

	TextOut(hDC, 20, 48, text2, lstrlen(text2));

	wsprintf(
		text3,
		_T("방향키: 이동방향  Enter: 발사  J: 점프  T: 트윈 생성  A: 먹이 폭파  R: 초기화")
	);

	TextOut(hDC, 20, 76, text3, lstrlen(text3));
}