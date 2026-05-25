#include <windows.h>     // 윈도우 API 사용
#include <tchar.h>       // _T(), TCHAR 사용
#include <atlimage.h>    // CImage 사용
#include <time.h>        // srand() 시간값 사용
#include <stdlib.h>      // rand() 사용

// ----------------------------------------------------
// 기본 윈도우 전역 변수
// ----------------------------------------------------
HINSTANCE g_hInst;
LPCTSTR lpszClass = L"My Window Class";
LPCTSTR lpszWindowName = L"Window Programming Lab";

LRESULT CALLBACK WndProc(HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam);

// ----------------------------------------------------
// 상수 정의
// ----------------------------------------------------
#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600

#define TIMER_GAME 1
#define TIMER_INTERVAL 30

#define OBSTACLE_COUNT 3
#define MAX_CHARACTER 20

#define CH1_FRAME_COUNT 6
#define D_FRAME_COUNT 7

#define CHARACTER_WIDTH 60
#define CHARACTER_HEIGHT 80

#define STATE_FALL 0
#define STATE_MOVE_ON_OBSTACLE 1

// ----------------------------------------------------
// 이미지 전역 변수
// ----------------------------------------------------
CImage backImage;                         // 배경 이미지 back.bmp
CImage ch1Image[CH1_FRAME_COUNT];          // 떨어지는 애니메이션 ch1~ch6.png
CImage dImage[D_FRAME_COUNT];              // 장애물 위 애니메이션 d1~d7.png
CImage tileImage;                          // 장애물 이미지 tile.bmp

// ----------------------------------------------------
// 장애물 구조체
// ----------------------------------------------------
typedef struct Obstacle {
    int x;      // 장애물 왼쪽 위치
    int y;      // 장애물 위쪽 위치
    int w;      // 장애물 가로 크기
    int h;      // 장애물 세로 크기
} Obstacle;

// ----------------------------------------------------
// 캐릭터 구조체
// ----------------------------------------------------
typedef struct Character {
    int active;             // 캐릭터가 화면에 존재하는지 여부
    int x;                  // 캐릭터 왼쪽 위치
    int y;                  // 캐릭터 위쪽 위치
    int fallSpeed;          // 아래로 떨어지는 속도
    int vx;                 // 장애물 위에서 좌우 이동 속도
    int state;              // 현재 상태: 떨어지는 중 / 장애물 위 이동 중
    int frame;              // 현재 애니메이션 프레임 번호
    int frameDelay;         // 프레임 변경 속도 조절용 카운터
    int currentObstacle;    // 현재 올라가 있는 장애물 번호
} Character;

// ----------------------------------------------------
// 게임 전역 변수
// ----------------------------------------------------
Obstacle obstacles[OBSTACLE_COUNT];        // 장애물 3개
Character characters[MAX_CHARACTER];       // 캐릭터 최대 20개

RECT clientRect;                           // 클라이언트 영역 크기 저장

int dragObstacleIndex = -1;                // 현재 드래그 중인 장애물 번호
int dragOffsetX = 0;                       // 마우스와 장애물 왼쪽 사이의 거리
int dragOffsetY = 0;                       // 마우스와 장애물 위쪽 사이의 거리

// ----------------------------------------------------
// 정수 값을 minValue ~ maxValue 사이로 제한하는 함수
// ----------------------------------------------------
int ClampInt(int value, int minValue, int maxValue)
{
    if (value < minValue) {
        return minValue;
    }

    if (value > maxValue) {
        return maxValue;
    }

    return value;
}

// ----------------------------------------------------
// 이미지 로드 함수
// ----------------------------------------------------
void LoadImages()
{
    TCHAR fileName[100];

    backImage.Load(_T("5-8/back.bmp"));

    for (int i = 0; i < CH1_FRAME_COUNT; i++) {
        wsprintf(fileName, _T("5-8/ch%d.png"), i + 1);
        ch1Image[i].Load(fileName);
    }

    for (int i = 0; i < D_FRAME_COUNT; i++) {
        wsprintf(fileName, _T("5-8/d%d.png"), i + 1);
        dImage[i].Load(fileName);
    }

    tileImage.Load(_T("5-8/tile1.bmp"));
}

// ----------------------------------------------------
// 이미지 해제 함수
// ----------------------------------------------------
void ReleaseImages()
{
    if (!backImage.IsNull()) {
        backImage.Destroy();
    }

    for (int i = 0; i < CH1_FRAME_COUNT; i++) {
        if (!ch1Image[i].IsNull()) {
            ch1Image[i].Destroy();
        }
    }

    for (int i = 0; i < D_FRAME_COUNT; i++) {
        if (!dImage[i].IsNull()) {
            dImage[i].Destroy();
        }
    }

    if (!tileImage.IsNull()) {
        tileImage.Destroy();
    }
}

// ----------------------------------------------------
// 장애물 3개의 위치와 크기를 랜덤으로 설정하는 함수
// ----------------------------------------------------
void RandomizeObstacles(HWND hWnd)
{
    GetClientRect(hWnd, &clientRect);

    int clientW = clientRect.right - clientRect.left;
    int clientH = clientRect.bottom - clientRect.top;

    for (int i = 0; i < OBSTACLE_COUNT; i++) {
        obstacles[i].w = 150 + rand() % 51;   // 가로 크기: 150~200
        obstacles[i].h = 50 + rand() % 51;    // 세로 크기: 50~100

        obstacles[i].x = rand() % (clientW - obstacles[i].w);

        // 너무 위나 너무 아래에 몰리지 않도록 y 범위 조절
        obstacles[i].y = 100 + rand() % (clientH - 220);

        obstacles[i].x = ClampInt(obstacles[i].x, 0, clientW - obstacles[i].w);
        obstacles[i].y = ClampInt(obstacles[i].y, 0, clientH - obstacles[i].h);
    }
}

// ----------------------------------------------------
// 캐릭터 전체를 비활성화하는 함수
// ----------------------------------------------------
void ClearCharacters()
{
    for (int i = 0; i < MAX_CHARACTER; i++) {
        characters[i].active = 0;
        characters[i].x = 0;
        characters[i].y = 0;
        characters[i].fallSpeed = 0;
        characters[i].vx = 0;
        characters[i].state = STATE_FALL;
        characters[i].frame = 0;
        characters[i].frameDelay = 0;
        characters[i].currentObstacle = -1;
    }
}

// ----------------------------------------------------
// s키를 눌렀을 때 캐릭터 20개 생성
// ----------------------------------------------------
void StartCharacters(HWND hWnd)
{
    GetClientRect(hWnd, &clientRect);

    int clientW = clientRect.right - clientRect.left;

    for (int i = 0; i < MAX_CHARACTER; i++) {
        characters[i].active = 1;

        characters[i].x = rand() % (clientW - CHARACTER_WIDTH);

        // 시작 y를 음수로 주면 캐릭터들이 위에서 순차적으로 내려오는 느낌이 난다.
        characters[i].y = -(rand() % 500) - i * 20;

        characters[i].fallSpeed = 3 + rand() % 6;       // 낙하 속도: 3~8
        characters[i].vx = (rand() % 2 == 0) ? -3 : 3;  // 장애물 위 좌우 속도
        characters[i].state = STATE_FALL;
        characters[i].frame = rand() % CH1_FRAME_COUNT;
        characters[i].frameDelay = 0;
        characters[i].currentObstacle = -1;
    }
}

// ----------------------------------------------------
// 게임 리셋 함수
// ----------------------------------------------------
void ResetGame(HWND hWnd)
{
    ClearCharacters();
    RandomizeObstacles(hWnd);
    InvalidateRect(hWnd, NULL, FALSE);
}

// ----------------------------------------------------
// 캐릭터와 장애물 착지 충돌 검사 함수
// ----------------------------------------------------
int CheckLandingObstacle(Character ch, int oldY, int newY)
{
    int oldBottom = oldY + CHARACTER_HEIGHT;
    int newBottom = newY + CHARACTER_HEIGHT;

    int charLeft = ch.x + 10;
    int charRight = ch.x + CHARACTER_WIDTH - 10;

    for (int i = 0; i < OBSTACLE_COUNT; i++) {
        int obstacleTop = obstacles[i].y;
        int obstacleLeft = obstacles[i].x;
        int obstacleRight = obstacles[i].x + obstacles[i].w;

        int isFallingAcrossTop = oldBottom <= obstacleTop && newBottom >= obstacleTop;
        int isHorizontalOverlap = charRight > obstacleLeft && charLeft < obstacleRight;

        if (isFallingAcrossTop && isHorizontalOverlap) {
            return i;
        }
    }

    return -1;
}

// ----------------------------------------------------
// 애니메이션 프레임 업데이트 함수
// ----------------------------------------------------
void UpdateAnimationFrame(Character* ch)
{
    ch->frameDelay++;

    if (ch->frameDelay < 5) {
        return;
    }

    ch->frameDelay = 0;

    if (ch->state == STATE_FALL) {
        ch->frame++;

        if (ch->frame >= CH1_FRAME_COUNT) {
            ch->frame = 0;
        }
    }
    else if (ch->state == STATE_MOVE_ON_OBSTACLE) {
        ch->frame++;

        if (ch->frame >= D_FRAME_COUNT) {
            ch->frame = 0;
        }
    }
}

// ----------------------------------------------------
// 캐릭터 위치 업데이트 함수
// ----------------------------------------------------
void UpdateCharacters(HWND hWnd)
{
    GetClientRect(hWnd, &clientRect);

    int clientW = clientRect.right - clientRect.left;
    int clientH = clientRect.bottom - clientRect.top;

    for (int i = 0; i < MAX_CHARACTER; i++) {
        if (characters[i].active == 0) {
            continue;
        }

        UpdateAnimationFrame(&characters[i]);

        if (characters[i].state == STATE_FALL) {
            int oldY = characters[i].y;
            int newY = characters[i].y + characters[i].fallSpeed;

            int landingIndex = CheckLandingObstacle(characters[i], oldY, newY);

            if (landingIndex != -1) {
                characters[i].state = STATE_MOVE_ON_OBSTACLE;
                characters[i].currentObstacle = landingIndex;
                characters[i].y = obstacles[landingIndex].y - CHARACTER_HEIGHT;
                characters[i].frame = 0;

                if (rand() % 2 == 0) {
                    characters[i].vx = -3;
                }
                else {
                    characters[i].vx = 3;
                }
            }
            else {
                characters[i].y = newY;
            }

            if (characters[i].y > clientH) {
                characters[i].active = 0;
            }
        }
        else if (characters[i].state == STATE_MOVE_ON_OBSTACLE) {
            int obstacleIndex = characters[i].currentObstacle;

            if (obstacleIndex < 0 || obstacleIndex >= OBSTACLE_COUNT) {
                characters[i].state = STATE_FALL;
                characters[i].currentObstacle = -1;
                continue;
            }

            characters[i].x += characters[i].vx;

            // 장애물이 드래그되어 움직이면 캐릭터도 장애물 위 높이에 맞춰 따라간다.
            characters[i].y = obstacles[obstacleIndex].y - CHARACTER_HEIGHT;

            // 화면 좌우 끝에서는 튕기게 한다.
            if (characters[i].x < 0) {
                characters[i].x = 0;
                characters[i].vx *= -1;
            }

            if (characters[i].x + CHARACTER_WIDTH > clientW) {
                characters[i].x = clientW - CHARACTER_WIDTH;
                characters[i].vx *= -1;
            }

            // 캐릭터 중심이 장애물 범위를 벗어나면 다시 떨어지는 애니메이션으로 변경
            int charCenterX = characters[i].x + CHARACTER_WIDTH / 2;
            int obstacleLeft = obstacles[obstacleIndex].x;
            int obstacleRight = obstacles[obstacleIndex].x + obstacles[obstacleIndex].w;

            if (charCenterX < obstacleLeft || charCenterX > obstacleRight) {
                characters[i].state = STATE_FALL;
                characters[i].currentObstacle = -1;
                characters[i].frame = 0;
            }
        }
    }
}

// ----------------------------------------------------
// 장애물 안에 마우스가 있는지 검사하는 함수
// ----------------------------------------------------
int HitTestObstacle(int mx, int my)
{
    for (int i = OBSTACLE_COUNT - 1; i >= 0; i--) {
        if (
            mx >= obstacles[i].x &&
            mx <= obstacles[i].x + obstacles[i].w &&
            my >= obstacles[i].y &&
            my <= obstacles[i].y + obstacles[i].h
            ) {
            return i;
        }
    }

    return -1;
}

// ----------------------------------------------------
// 화면 그리기 함수
// ----------------------------------------------------
void DrawGame(HDC hDC, HWND hWnd)
{
    GetClientRect(hWnd, &clientRect);

    int clientW = clientRect.right - clientRect.left;
    int clientH = clientRect.bottom - clientRect.top;

    HDC memDC = CreateCompatibleDC(hDC);
    HBITMAP memBitmap = CreateCompatibleBitmap(hDC, clientW, clientH);
    HBITMAP oldBitmap = (HBITMAP)SelectObject(memDC, memBitmap);

    HBRUSH whiteBrush = (HBRUSH)GetStockObject(WHITE_BRUSH);
    FillRect(memDC, &clientRect, whiteBrush);

    if (!backImage.IsNull()) {
        backImage.Draw(memDC, 0, 0, clientW, clientH);
    }

    for (int i = 0; i < OBSTACLE_COUNT; i++) {
        if (!tileImage.IsNull()) {
            tileImage.Draw(
                memDC,
                obstacles[i].x,
                obstacles[i].y,
                obstacles[i].w,
                obstacles[i].h
            );
        }
        else {
            Rectangle(
                memDC,
                obstacles[i].x,
                obstacles[i].y,
                obstacles[i].x + obstacles[i].w,
                obstacles[i].y + obstacles[i].h
            );
        }
    }

    for (int i = 0; i < MAX_CHARACTER; i++) {
        if (characters[i].active == 0) {
            continue;
        }

        if (characters[i].state == STATE_FALL) {
            int frame = characters[i].frame;

            if (!ch1Image[frame].IsNull()) {
                ch1Image[frame].Draw(
                    memDC,
                    characters[i].x,
                    characters[i].y,
                    CHARACTER_WIDTH,
                    CHARACTER_HEIGHT
                );
            }
            else {
                Ellipse(
                    memDC,
                    characters[i].x,
                    characters[i].y,
                    characters[i].x + CHARACTER_WIDTH,
                    characters[i].y + CHARACTER_HEIGHT
                );
            }
        }
        else if (characters[i].state == STATE_MOVE_ON_OBSTACLE) {
            int frame = characters[i].frame;

            if (!dImage[frame].IsNull()) {
                dImage[frame].Draw(
                    memDC,
                    characters[i].x,
                    characters[i].y,
                    CHARACTER_WIDTH,
                    CHARACTER_HEIGHT
                );
            }
            else {
                Rectangle(
                    memDC,
                    characters[i].x,
                    characters[i].y,
                    characters[i].x + CHARACTER_WIDTH,
                    characters[i].y + CHARACTER_HEIGHT
                );
            }
        }
    }

    BitBlt(hDC, 0, 0, clientW, clientH, memDC, 0, 0, SRCCOPY);

    SelectObject(memDC, oldBitmap);
    DeleteObject(memBitmap);
    DeleteDC(memDC);
}

// ----------------------------------------------------
// WinMain 함수
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
        0,
        0,
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

    return Message.wParam;
}

// ----------------------------------------------------
// WndProc 함수
// ----------------------------------------------------
LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    PAINTSTRUCT ps;
    HDC hDC;

    int mx;
    int my;

    switch (uMsg) {
    case WM_CREATE:
        srand((unsigned int)time(NULL));

        LoadImages();

        ClearCharacters();
        RandomizeObstacles(hWnd);

        SetTimer(hWnd, TIMER_GAME, TIMER_INTERVAL, NULL);
        return 0;

    case WM_SIZE:
        GetClientRect(hWnd, &clientRect);
        return 0;

    case WM_TIMER:
        if (wParam == TIMER_GAME) {
            UpdateCharacters(hWnd);
            InvalidateRect(hWnd, NULL, FALSE);
        }
        return 0;

    case WM_KEYDOWN:
        if (wParam == 'S' || wParam == 's') {
            StartCharacters(hWnd);
            InvalidateRect(hWnd, NULL, FALSE);
        }
        else if (wParam == 'M' || wParam == 'm') {
            RandomizeObstacles(hWnd);

            for (int i = 0; i < MAX_CHARACTER; i++) {
                if (characters[i].state == STATE_MOVE_ON_OBSTACLE) {
                    characters[i].state = STATE_FALL;
                    characters[i].currentObstacle = -1;
                }
            }

            InvalidateRect(hWnd, NULL, FALSE);
        }
        else if (wParam == 'R' || wParam == 'r') {
            ResetGame(hWnd);
        }
        else if (wParam == 'Q' || wParam == 'q') {
            DestroyWindow(hWnd);
        }
        return 0;

    case WM_LBUTTONDOWN:
        mx = LOWORD(lParam);
        my = HIWORD(lParam);

        dragObstacleIndex = HitTestObstacle(mx, my);

        if (dragObstacleIndex != -1) {
            dragOffsetX = mx - obstacles[dragObstacleIndex].x;
            dragOffsetY = my - obstacles[dragObstacleIndex].y;
            SetCapture(hWnd);
        }
        return 0;

    case WM_MOUSEMOVE:
        if (dragObstacleIndex != -1 && (wParam & MK_LBUTTON)) {
            mx = LOWORD(lParam);
            my = HIWORD(lParam);

            GetClientRect(hWnd, &clientRect);

            int clientW = clientRect.right - clientRect.left;
            int clientH = clientRect.bottom - clientRect.top;

            obstacles[dragObstacleIndex].x = mx - dragOffsetX;
            obstacles[dragObstacleIndex].y = my - dragOffsetY;

            obstacles[dragObstacleIndex].x = ClampInt(
                obstacles[dragObstacleIndex].x,
                0,
                clientW - obstacles[dragObstacleIndex].w
            );

            obstacles[dragObstacleIndex].y = ClampInt(
                obstacles[dragObstacleIndex].y,
                0,
                clientH - obstacles[dragObstacleIndex].h
            );

            InvalidateRect(hWnd, NULL, FALSE);
        }
        return 0;

    case WM_LBUTTONUP:
        if (dragObstacleIndex != -1) {
            dragObstacleIndex = -1;
            ReleaseCapture();
        }
        return 0;

    case WM_ERASEBKGND:
        return 1;

    case WM_PAINT:
        hDC = BeginPaint(hWnd, &ps);
        DrawGame(hDC, hWnd);
        EndPaint(hWnd, &ps);
        return 0;

    case WM_DESTROY:
        KillTimer(hWnd, TIMER_GAME);
        ReleaseImages();
        PostQuitMessage(0);
        return 0;
    }

    return DefWindowProc(hWnd, uMsg, wParam, lParam);
}