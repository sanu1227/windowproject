#include <windows.h>                         // Windows API 함수를 사용하기 위한 헤더 파일입니다.
#include <tchar.h>                           // TCHAR, LPCTSTR 같은 문자형을 사용하기 위한 헤더 파일입니다.
#include <stdlib.h>                          // rand(), srand() 함수를 사용하기 위한 헤더 파일입니다.
#include <time.h>                            // time(), clock() 함수를 사용하기 위한 헤더 파일입니다.

HINSTANCE g_hInst;                           // 현재 실행 중인 프로그램의 인스턴스 핸들을 저장하는 전역 변수입니다.
LPCTSTR lpszClass = L"My Window Class";      // 윈도우 클래스 이름을 저장하는 문자열입니다.
LPCTSTR lpszWindowName = L"Brick Breaker";   // 윈도우 제목 표시줄에 출력될 문자열입니다.

LRESULT CALLBACK WndProc(HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam); // 윈도우 메시지 처리 함수 선언입니다.

#define WINDOW_WIDTH 800                     // 프로그램 창의 가로 크기입니다.
#define WINDOW_HEIGHT 600                    // 프로그램 창의 세로 크기입니다.

#define BRICK_ROWS 3                         // 벽돌의 줄 개수입니다. 이 값을 바꾸면 줄 수가 변경됩니다.
#define BRICK_COLS 10                        // 벽돌의 칸 개수입니다. 이 값을 바꾸면 칸 수가 변경됩니다.

#define MAX_FALLING 100                      // 동시에 아래로 떨어질 수 있는 벽돌의 최대 개수입니다.

#define TIMER_GAME 1                         // 공과 게임 전체 이동을 담당하는 타이머 번호입니다.
#define TIMER_BRICK 2                        // 벽돌 좌우 이동을 담당하는 타이머 번호입니다.

#define MENU_START 1001                      // 메뉴 Start 명령 번호입니다.
#define MENU_PAUSE 1002                      // 메뉴 Pause/Restart 명령 번호입니다.
#define MENU_TIME 1003                       // 메뉴 Time 명령 번호입니다.
#define MENU_RESET 1004                      // 메뉴 Reset 명령 번호입니다.
#define MENU_QUIT 1005                       // 메뉴 Quit 명령 번호입니다.

#define BALL_SIZE 14                         // 공의 지름 크기입니다.
#define BAR_WIDTH 120                        // 바의 가로 크기입니다.
#define BAR_HEIGHT 18                        // 바의 세로 크기입니다.

#define BRICK_WIDTH 55                       // 벽돌 하나의 가로 크기입니다.
#define BRICK_HEIGHT 25                      // 벽돌 하나의 세로 크기입니다.
#define BRICK_GAP 6                          // 벽돌 사이의 간격입니다.

#define BRICK_START_Y 60                     // 벽돌들이 처음 그려질 y 좌표입니다.
#define BRICK_MOVE_SPEED 3                   // 벽돌 묶음이 좌우로 움직이는 속도입니다.

#define FALL_SPEED 8                         // 맞은 벽돌이 아래로 떨어지는 속도입니다.

struct Brick {                               // 일반 벽돌 하나의 정보를 저장하는 구조체입니다.
	int exist;                               // 벽돌이 아직 화면 위쪽 배열에 존재하는지 저장합니다. 1이면 존재, 0이면 사라짐입니다.
	COLORREF color;                          // 벽돌의 현재 색상을 저장합니다.
};

struct FallingBrick {                        // 아래로 떨어지는 벽돌 하나의 정보를 저장하는 구조체입니다.
	int active;                              // 떨어지는 벽돌이 현재 사용 중인지 저장합니다. 1이면 떨어지는 중입니다.
	int x;                                   // 떨어지는 벽돌의 현재 x 좌표입니다.
	int y;                                   // 떨어지는 벽돌의 현재 y 좌표입니다.
	COLORREF color;                          // 떨어지는 벽돌의 현재 색상을 저장합니다.
};

Brick bricks[BRICK_ROWS][BRICK_COLS];        // 위쪽에 배치되는 벽돌들을 2차원 배열로 저장합니다.
FallingBrick falling[MAX_FALLING];           // 아래로 떨어지는 벽돌들을 배열로 저장합니다.

RECT rectView;                               // 클라이언트 영역의 크기를 저장하는 RECT 구조체입니다.

int brickBaseX = 90;                         // 벽돌 전체 묶음의 시작 x 좌표입니다.
int brickDir = 1;                            // 벽돌 묶음의 이동 방향입니다. 1은 오른쪽, -1은 왼쪽입니다.

int barX = 340;                              // 바의 왼쪽 x 좌표입니다.
int barY = 520;                              // 바의 위쪽 y 좌표입니다.
int isBarDrag = 0;                           // 마우스로 바를 드래그 중인지 저장합니다.
int dragOffsetX = 0;                         // 바를 클릭했을 때 클릭 위치와 바 왼쪽 사이의 차이입니다.

int ballX = 0;                               // 공의 왼쪽 x 좌표입니다.
int ballY = 0;                               // 공의 위쪽 y 좌표입니다.
int ballDX = 5;                              // 공의 x 방향 이동량입니다.
int ballDY = -5;                             // 공의 y 방향 이동량입니다.

int gameStarted = 0;                         // 게임이 한 번이라도 시작되었는지 저장합니다.
int gameRunning = 0;                         // 현재 게임이 움직이는 중인지 저장합니다.
int gamePaused = 0;                          // 현재 게임이 일시정지 상태인지 저장합니다.

int ballTimerInterval = 20;                  // 공 이동 타이머의 간격입니다. 값이 작을수록 공이 빠릅니다.

int changedBrickCount = 0;                   // 색이 바뀐 벽돌의 총 개수를 저장합니다.
int removedBrickCount = 0;                   // 화면 아래로 사라진 벽돌의 총 개수를 저장합니다.

int showPauseInfo = 0;                       // p를 눌렀을 때 통계 정보를 화면에 보여줄지 저장합니다.
int showTimeInfo = 0;                        // t를 눌렀을 때 시간 정보를 화면에 보여줄지 저장합니다.

time_t startTime;                            // 게임 시작 시간을 저장합니다.
time_t currentTime;                          // 현재 시간을 저장합니다.

COLORREF RandomColor() {                     // 랜덤 색상을 만들어 반환하는 함수입니다.
	int r = rand() % 256;                    // 빨간색 값을 0부터 255 사이에서 랜덤으로 만듭니다.
	int g = rand() % 256;                    // 초록색 값을 0부터 255 사이에서 랜덤으로 만듭니다.
	int b = rand() % 256;                    // 파란색 값을 0부터 255 사이에서 랜덤으로 만듭니다.
	return RGB(r, g, b);                     // r, g, b 값을 COLORREF 색상으로 만들어 반환합니다.
}

void SetBallOnBar() {                        // 공을 바 위에 올려놓는 함수입니다.
	ballX = barX + BAR_WIDTH / 2 - BALL_SIZE / 2; // 공의 x 좌표를 바의 중앙에 맞춥니다.
	ballY = barY - BALL_SIZE - 2;            // 공의 y 좌표를 바 바로 위로 맞춥니다.
	ballDX = 5;                              // 공이 처음에는 오른쪽 위 대각선으로 움직이도록 x 이동량을 설정합니다.
	ballDY = -5;                             // 공이 처음에는 위쪽으로 움직이도록 y 이동량을 설정합니다.
}

void InitGame() {                            // 게임 전체를 처음 상태로 초기화하는 함수입니다.
	int row;                                  // 벽돌 줄 반복에 사용할 변수입니다.
	int col;                                  // 벽돌 칸 반복에 사용할 변수입니다.
	int i;                                    // 떨어지는 벽돌 배열 반복에 사용할 변수입니다.

	brickBaseX = 90;                         // 벽돌 묶음의 시작 x 좌표를 초기값으로 되돌립니다.
	brickDir = 1;                            // 벽돌 묶음의 이동 방향을 오른쪽으로 초기화합니다.

	barX = 340;                              // 바의 x 좌표를 초기값으로 되돌립니다.
	barY = 520;                              // 바의 y 좌표를 초기값으로 되돌립니다.
	isBarDrag = 0;                           // 바 드래그 상태를 해제합니다.
	dragOffsetX = 0;                         // 드래그 보정값을 초기화합니다.

	for (row = 0; row < BRICK_ROWS; row++) {  // 모든 벽돌 줄을 반복합니다.
		for (col = 0; col < BRICK_COLS; col++) { // 각 줄의 모든 벽돌 칸을 반복합니다.
			bricks[row][col].exist = 1;       // 모든 벽돌을 존재하는 상태로 만듭니다.
			bricks[row][col].color = RandomColor(); // 각 벽돌에 랜덤 색상을 넣습니다.
		}
	}

	for (i = 0; i < MAX_FALLING; i++) {       // 떨어지는 벽돌 배열 전체를 반복합니다.
		falling[i].active = 0;               // 모든 떨어지는 벽돌을 비활성 상태로 만듭니다.
		falling[i].x = 0;                    // 떨어지는 벽돌의 x 좌표를 초기화합니다.
		falling[i].y = 0;                    // 떨어지는 벽돌의 y 좌표를 초기화합니다.
		falling[i].color = RGB(0, 0, 0);      // 떨어지는 벽돌의 색상을 검정으로 초기화합니다.
	}

	gameStarted = 0;                         // 게임 시작 상태를 초기화합니다.
	gameRunning = 0;                         // 게임 실행 상태를 멈춤으로 초기화합니다.
	gamePaused = 0;                          // 일시정지 상태를 해제합니다.

	ballTimerInterval = 20;                  // 공 이동 속도를 기본값으로 되돌립니다.

	changedBrickCount = 0;                   // 색이 바뀐 벽돌 개수를 초기화합니다.
	removedBrickCount = 0;                   // 사라진 벽돌 개수를 초기화합니다.

	showPauseInfo = 0;                       // 일시정지 정보 출력을 끕니다.
	showTimeInfo = 0;                        // 시간 정보 출력을 끕니다.

	startTime = 0;                           // 시작 시간을 초기화합니다.
	currentTime = 0;                         // 현재 시간을 초기화합니다.

	SetBallOnBar();                          // 공을 바 위에 올려놓습니다.
}

void CreateGameMenu(HWND hWnd) {             // 프로그램 메뉴를 만드는 함수입니다.
	HMENU hMainMenu;                          // 가장 바깥쪽 메뉴를 저장할 핸들입니다.
	HMENU hGameMenu;                          // Game 메뉴를 저장할 핸들입니다.

	hMainMenu = CreateMenu();                 // 메인 메뉴를 생성합니다.
	hGameMenu = CreatePopupMenu();            // Game 하위 메뉴를 생성합니다.

	AppendMenu(hGameMenu, MF_STRING, MENU_START, L"Start\tS");          // Start 메뉴를 추가합니다.
	AppendMenu(hGameMenu, MF_STRING, MENU_PAUSE, L"Pause/Restart\tP");  // Pause/Restart 메뉴를 추가합니다.
	AppendMenu(hGameMenu, MF_STRING, MENU_TIME, L"Time\tT");            // Time 메뉴를 추가합니다.
	AppendMenu(hGameMenu, MF_SEPARATOR, 0, NULL);                        // 메뉴 사이에 구분선을 추가합니다.
	AppendMenu(hGameMenu, MF_STRING, MENU_RESET, L"Reset\tR");          // Reset 메뉴를 추가합니다.
	AppendMenu(hGameMenu, MF_STRING, MENU_QUIT, L"Quit\tQ");            // Quit 메뉴를 추가합니다.

	AppendMenu(hMainMenu, MF_POPUP, (UINT_PTR)hGameMenu, L"Game");      // Game 메뉴를 메인 메뉴에 붙입니다.
	SetMenu(hWnd, hMainMenu);                  // 완성된 메뉴를 현재 윈도우에 적용합니다.
}

void StartGame(HWND hWnd) {                  // 게임 시작 함수입니다.
	if (gameStarted == 0) {                   // 게임이 아직 시작된 적이 없다면 실행합니다.
		startTime = time(NULL);              // 현재 시간을 게임 시작 시간으로 저장합니다.
		gameStarted = 1;                     // 게임이 시작되었다고 표시합니다.
	}

	gameRunning = 1;                          // 게임을 움직이는 상태로 바꿉니다.
	gamePaused = 0;                           // 일시정지 상태를 해제합니다.
	showPauseInfo = 0;                        // 일시정지 정보 출력을 끕니다.

	SetTimer(hWnd, TIMER_GAME, ballTimerInterval, NULL); // 공 이동 타이머를 시작합니다.
	SetTimer(hWnd, TIMER_BRICK, 30, NULL);     // 벽돌 좌우 이동 타이머를 시작합니다.
}

void PauseOrRestart(HWND hWnd) {             // p 명령으로 멈춤과 재시작을 처리하는 함수입니다.
	if (gameRunning == 1) {                   // 현재 게임이 움직이는 중이면 멈춥니다.
		gameRunning = 0;                     // 게임 실행 상태를 멈춤으로 바꿉니다.
		gamePaused = 1;                      // 일시정지 상태로 표시합니다.
		showPauseInfo = 1;                   // 일시정지 정보를 화면에 출력하도록 설정합니다.
		KillTimer(hWnd, TIMER_GAME);         // 공 이동 타이머를 멈춥니다.
		KillTimer(hWnd, TIMER_BRICK);        // 벽돌 이동 타이머를 멈춥니다.
	}
	else {                                    // 현재 게임이 멈춰 있다면 다시 시작합니다.
		gameRunning = 1;                     // 게임 실행 상태로 바꿉니다.
		gamePaused = 0;                      // 일시정지 상태를 해제합니다.
		showPauseInfo = 0;                   // 일시정지 정보 출력을 끕니다.
		SetTimer(hWnd, TIMER_GAME, ballTimerInterval, NULL); // 공 이동 타이머를 다시 시작합니다.
		SetTimer(hWnd, TIMER_BRICK, 30, NULL); // 벽돌 이동 타이머를 다시 시작합니다.
	}
}

void AddFallingBrick(int x, int y, COLORREF color) { // 맞은 벽돌을 아래로 떨어지는 배열에 추가하는 함수입니다.
	int i;                                    // 떨어지는 벽돌 배열 반복에 사용할 변수입니다.

	for (i = 0; i < MAX_FALLING; i++) {       // 떨어지는 벽돌 배열을 처음부터 검사합니다.
		if (falling[i].active == 0) {         // 비어 있는 칸을 찾으면 실행합니다.
			falling[i].active = 1;            // 해당 칸을 사용 중으로 바꿉니다.
			falling[i].x = x;                 // 떨어질 벽돌의 x 좌표를 저장합니다.
			falling[i].y = y;                 // 떨어질 벽돌의 y 좌표를 저장합니다.
			falling[i].color = color;         // 떨어질 벽돌의 색상을 저장합니다.
			break;                            // 하나만 추가하면 되므로 반복을 종료합니다.
		}
	}
}

void ChangeRowColor(int row) {               // 특정 줄 전체의 벽돌 색상을 랜덤하게 바꾸는 함수입니다.
	int col;                                  // 벽돌 칸 반복에 사용할 변수입니다.
	COLORREF newColor;                        // 새로 적용할 랜덤 색상입니다.

	newColor = RandomColor();                 // 줄 전체에 적용할 랜덤 색상을 만듭니다.

	for (col = 0; col < BRICK_COLS; col++) {  // 해당 줄의 모든 벽돌 칸을 반복합니다.
		if (bricks[row][col].exist == 1) {    // 벽돌이 아직 존재하는 경우만 색을 바꿉니다.
			bricks[row][col].color = newColor; // 해당 벽돌 색상을 새 색으로 바꿉니다.
			changedBrickCount++;              // 색이 바뀐 벽돌 개수를 1 증가시킵니다.
		}
	}
}

void MoveFallingBricks() {                   // 아래로 떨어지는 벽돌들을 이동시키는 함수입니다.
	int i;                                    // 떨어지는 벽돌 배열 반복에 사용할 변수입니다.

	for (i = 0; i < MAX_FALLING; i++) {       // 떨어지는 벽돌 배열 전체를 검사합니다.
		if (falling[i].active == 1) {         // 현재 떨어지는 중인 벽돌만 처리합니다.
			falling[i].y += FALL_SPEED;       // 벽돌을 아래쪽으로 이동시킵니다.
			falling[i].color = RandomColor(); // 내려오는 동안 색이 계속 바뀌도록 랜덤 색을 다시 넣습니다.

			if (falling[i].y > rectView.bottom) { // 벽돌이 화면 아래로 완전히 내려갔는지 확인합니다.
				falling[i].active = 0;        // 화면 밖으로 나간 벽돌을 비활성화합니다.
				removedBrickCount++;          // 없어진 벽돌 개수를 1 증가시킵니다.
			}
		}
	}
}

void MoveBrickGroup() {                      // 위쪽 벽돌 묶음을 좌우로 이동시키는 함수입니다.
	int totalWidth;                           // 벽돌 전체 묶음의 가로 길이를 저장합니다.

	totalWidth = BRICK_COLS * BRICK_WIDTH + (BRICK_COLS - 1) * BRICK_GAP; // 벽돌 묶음 전체 너비를 계산합니다.
	brickBaseX += brickDir * BRICK_MOVE_SPEED; // 현재 방향에 따라 벽돌 묶음을 좌우로 이동합니다.

	if (brickBaseX <= 50) {                   // 벽돌 묶음이 왼쪽 여백에 닿았는지 확인합니다.
		brickBaseX = 50;                      // 너무 왼쪽으로 가지 않도록 위치를 보정합니다.
		brickDir = 1;                         // 이동 방향을 오른쪽으로 바꿉니다.
	}

	if (brickBaseX + totalWidth >= rectView.right - 50) { // 벽돌 묶음이 오른쪽 여백에 닿았는지 확인합니다.
		brickBaseX = rectView.right - 50 - totalWidth; // 너무 오른쪽으로 가지 않도록 위치를 보정합니다.
		brickDir = -1;                        // 이동 방향을 왼쪽으로 바꿉니다.
	}
}

void CheckBrickCollision() {                 // 공과 벽돌의 충돌을 검사하는 함수입니다.
	int row;                                  // 벽돌 줄 반복에 사용할 변수입니다.
	int col;                                  // 벽돌 칸 반복에 사용할 변수입니다.
	int bx;                                   // 현재 검사 중인 벽돌의 x 좌표입니다.
	int by;                                   // 현재 검사 중인 벽돌의 y 좌표입니다.
	RECT ballRect;                            // 공의 충돌 영역입니다.
	RECT brickRect;                           // 벽돌의 충돌 영역입니다.
	RECT hitRect;                             // IntersectRect 결과를 받을 영역입니다.

	ballRect.left = ballX;                    // 공 영역의 왼쪽 좌표입니다.
	ballRect.top = ballY;                     // 공 영역의 위쪽 좌표입니다.
	ballRect.right = ballX + BALL_SIZE;       // 공 영역의 오른쪽 좌표입니다.
	ballRect.bottom = ballY + BALL_SIZE;      // 공 영역의 아래쪽 좌표입니다.

	for (row = 0; row < BRICK_ROWS; row++) {  // 모든 벽돌 줄을 검사합니다.
		for (col = 0; col < BRICK_COLS; col++) { // 각 줄의 모든 벽돌 칸을 검사합니다.
			if (bricks[row][col].exist == 1) { // 현재 벽돌이 존재할 때만 충돌 검사를 합니다.
				bx = brickBaseX + col * (BRICK_WIDTH + BRICK_GAP); // 현재 벽돌의 x 좌표를 계산합니다.
				by = BRICK_START_Y + row * (BRICK_HEIGHT + BRICK_GAP); // 현재 벽돌의 y 좌표를 계산합니다.

				brickRect.left = bx;          // 벽돌 영역의 왼쪽 좌표입니다.
				brickRect.top = by;           // 벽돌 영역의 위쪽 좌표입니다.
				brickRect.right = bx + BRICK_WIDTH; // 벽돌 영역의 오른쪽 좌표입니다.
				brickRect.bottom = by + BRICK_HEIGHT; // 벽돌 영역의 아래쪽 좌표입니다.

				if (IntersectRect(&hitRect, &ballRect, &brickRect)) { // 공과 벽돌 영역이 겹쳤는지 검사합니다.
					ChangeRowColor(row);      // 공이 맞은 줄 전체의 색을 바꿉니다.
					AddFallingBrick(bx, by, bricks[row][col].color); // 맞은 벽돌을 아래로 떨어지는 벽돌 배열에 넣습니다.
					bricks[row][col].exist = 0; // 맞은 벽돌은 위쪽 배열에서 제거합니다.
					ballDY = -ballDY;         // 공의 y 방향을 반대로 바꿉니다.
					return;                    // 한 번에 하나의 벽돌만 처리하기 위해 함수를 종료합니다.
				}
			}
		}
	}
}

void CheckBarCollision() {                   // 공과 바의 충돌을 검사하는 함수입니다.
	RECT ballRect;                            // 공의 충돌 영역입니다.
	RECT barRect;                             // 바의 충돌 영역입니다.
	RECT hitRect;                             // IntersectRect 결과를 받을 영역입니다.
	int ballCenter;                           // 공의 중심 x 좌표입니다.
	int barCenter;                            // 바의 중심 x 좌표입니다.

	ballRect.left = ballX;                    // 공 영역의 왼쪽 좌표입니다.
	ballRect.top = ballY;                     // 공 영역의 위쪽 좌표입니다.
	ballRect.right = ballX + BALL_SIZE;       // 공 영역의 오른쪽 좌표입니다.
	ballRect.bottom = ballY + BALL_SIZE;      // 공 영역의 아래쪽 좌표입니다.

	barRect.left = barX;                      // 바 영역의 왼쪽 좌표입니다.
	barRect.top = barY;                       // 바 영역의 위쪽 좌표입니다.
	barRect.right = barX + BAR_WIDTH;         // 바 영역의 오른쪽 좌표입니다.
	barRect.bottom = barY + BAR_HEIGHT;       // 바 영역의 아래쪽 좌표입니다.

	if (IntersectRect(&hitRect, &ballRect, &barRect)) { // 공과 바가 겹쳤는지 검사합니다.
		ballCenter = ballX + BALL_SIZE / 2;   // 공의 중심 x 좌표를 계산합니다.
		barCenter = barX + BAR_WIDTH / 2;     // 바의 중심 x 좌표를 계산합니다.

		ballDY = -abs(ballDY);                // 공이 항상 위쪽으로 튕기도록 y 이동량을 음수로 만듭니다.

		if (ballCenter < barCenter) {         // 공이 바의 왼쪽 부분에 맞았다면 실행합니다.
			ballDX = -abs(ballDX);            // 공을 왼쪽 위 대각선으로 튕깁니다.
		}
		else {                                // 공이 바의 오른쪽 부분에 맞았다면 실행합니다.
			ballDX = abs(ballDX);             // 공을 오른쪽 위 대각선으로 튕깁니다.
		}

		ballY = barY - BALL_SIZE - 1;         // 공이 바 안으로 파고들지 않도록 공 위치를 바 위로 보정합니다.
	}
}

void MoveBall(HWND hWnd) {                   // 공을 이동시키고 충돌을 처리하는 함수입니다.
	ballX += ballDX;                          // 공의 x 좌표를 이동량만큼 변경합니다.
	ballY += ballDY;                          // 공의 y 좌표를 이동량만큼 변경합니다.

	if (ballX <= 0) {                         // 공이 왼쪽 벽에 닿았는지 확인합니다.
		ballX = 0;                            // 공이 화면 밖으로 나가지 않도록 보정합니다.
		ballDX = abs(ballDX);                 // 공을 오른쪽 방향으로 튕깁니다.
	}

	if (ballX + BALL_SIZE >= rectView.right) { // 공이 오른쪽 벽에 닿았는지 확인합니다.
		ballX = rectView.right - BALL_SIZE;   // 공이 화면 밖으로 나가지 않도록 보정합니다.
		ballDX = -abs(ballDX);                // 공을 왼쪽 방향으로 튕깁니다.
	}

	if (ballY <= 0) {                         // 공이 위쪽 벽에 닿았는지 확인합니다.
		ballY = 0;                            // 공이 화면 밖으로 나가지 않도록 보정합니다.
		ballDY = abs(ballDY);                 // 공을 아래쪽 방향으로 튕깁니다.
	}

	CheckBrickCollision();                    // 공과 벽돌의 충돌을 검사합니다.
	CheckBarCollision();                      // 공과 바의 충돌을 검사합니다.

	if (ballY > rectView.bottom) {            // 공이 바닥 아래로 사라졌는지 확인합니다.
		SetBallOnBar();                       // 공을 다시 바 위에 올려놓습니다.

		if (gameStarted == 1) {               // 게임이 시작된 상태라면 실행합니다.
			ballDX = 5;                       // 다시 오른쪽 위 대각선 방향으로 시작하게 합니다.
			ballDY = -5;                      // 다시 위쪽으로 튀도록 설정합니다.
		}
	}
}

void DrawBricks(HDC hDC) {                   // 위쪽 벽돌들을 화면에 그리는 함수입니다.
	int row;                                  // 벽돌 줄 반복에 사용할 변수입니다.
	int col;                                  // 벽돌 칸 반복에 사용할 변수입니다.
	int x;                                    // 현재 벽돌의 x 좌표입니다.
	int y;                                    // 현재 벽돌의 y 좌표입니다.
	HBRUSH hBrush;                            // 벽돌 내부를 칠할 브러시입니다.
	HBRUSH oldBrush;                          // 이전 브러시를 저장할 변수입니다.

	for (row = 0; row < BRICK_ROWS; row++) {  // 모든 벽돌 줄을 반복합니다.
		for (col = 0; col < BRICK_COLS; col++) { // 각 줄의 모든 벽돌 칸을 반복합니다.
			if (bricks[row][col].exist == 1) { // 존재하는 벽돌만 그립니다.
				x = brickBaseX + col * (BRICK_WIDTH + BRICK_GAP); // 벽돌의 x 좌표를 계산합니다.
				y = BRICK_START_Y + row * (BRICK_HEIGHT + BRICK_GAP); // 벽돌의 y 좌표를 계산합니다.

				hBrush = CreateSolidBrush(bricks[row][col].color); // 벽돌 색상의 브러시를 만듭니다.
				oldBrush = (HBRUSH)SelectObject(hDC, hBrush); // 새 브러시를 선택하고 이전 브러시를 저장합니다.
				Rectangle(hDC, x, y, x + BRICK_WIDTH, y + BRICK_HEIGHT); // 벽돌 사각형을 그립니다.
				SelectObject(hDC, oldBrush);  // 이전 브러시로 되돌립니다.
				DeleteObject(hBrush);         // 만든 브러시를 삭제하여 메모리 누수를 막습니다.
			}
		}
	}
}

void DrawFallingBricks(HDC hDC) {            // 아래로 떨어지는 벽돌들을 그리는 함수입니다.
	int i;                                    // 떨어지는 벽돌 배열 반복에 사용할 변수입니다.
	HBRUSH hBrush;                            // 벽돌 내부를 칠할 브러시입니다.
	HBRUSH oldBrush;                          // 이전 브러시를 저장할 변수입니다.

	for (i = 0; i < MAX_FALLING; i++) {       // 떨어지는 벽돌 배열 전체를 검사합니다.
		if (falling[i].active == 1) {         // 현재 떨어지는 중인 벽돌만 그립니다.
			hBrush = CreateSolidBrush(falling[i].color); // 떨어지는 벽돌 색상의 브러시를 만듭니다.
			oldBrush = (HBRUSH)SelectObject(hDC, hBrush); // 새 브러시를 선택하고 이전 브러시를 저장합니다.
			Rectangle(hDC, falling[i].x, falling[i].y, falling[i].x + BRICK_WIDTH, falling[i].y + BRICK_HEIGHT); // 떨어지는 벽돌을 그립니다.
			SelectObject(hDC, oldBrush);      // 이전 브러시로 되돌립니다.
			DeleteObject(hBrush);             // 만든 브러시를 삭제합니다.
		}
	}
}

void DrawBar(HDC hDC) {                      // 바를 화면에 그리는 함수입니다.
	HBRUSH hBrush;                            // 바 내부를 칠할 브러시입니다.
	HBRUSH oldBrush;                          // 이전 브러시를 저장할 변수입니다.

	hBrush = CreateSolidBrush(RGB(40, 40, 40)); // 진한 회색 브러시를 만듭니다.
	oldBrush = (HBRUSH)SelectObject(hDC, hBrush); // 새 브러시를 선택하고 이전 브러시를 저장합니다.
	RoundRect(hDC, barX, barY, barX + BAR_WIDTH, barY + BAR_HEIGHT, 12, 12); // 둥근 모서리의 바를 그립니다.
	SelectObject(hDC, oldBrush);             // 이전 브러시로 되돌립니다.
	DeleteObject(hBrush);                    // 만든 브러시를 삭제합니다.
}

void DrawBall(HDC hDC) {                     // 공을 화면에 그리는 함수입니다.
	HBRUSH hBrush;                            // 공 내부를 칠할 브러시입니다.
	HBRUSH oldBrush;                          // 이전 브러시를 저장할 변수입니다.

	hBrush = CreateSolidBrush(RGB(255, 80, 80)); // 빨간색 계열의 공 브러시를 만듭니다.
	oldBrush = (HBRUSH)SelectObject(hDC, hBrush); // 새 브러시를 선택하고 이전 브러시를 저장합니다.
	Ellipse(hDC, ballX, ballY, ballX + BALL_SIZE, ballY + BALL_SIZE); // 원 모양의 공을 그립니다.
	SelectObject(hDC, oldBrush);             // 이전 브러시로 되돌립니다.
	DeleteObject(hBrush);                    // 만든 브러시를 삭제합니다.
}

void DrawInfo(HDC hDC) {                     // 화면에 안내 문구와 상태 정보를 출력하는 함수입니다.
	TCHAR str[200];                           // 화면에 출력할 문자열을 저장하는 배열입니다.
	int playTime;                             // 게임 플레이 시간을 초 단위로 저장합니다.

	SetBkMode(hDC, TRANSPARENT);              // 글자 배경을 투명하게 설정합니다.
	SetTextColor(hDC, RGB(0, 0, 0));           // 글자 색상을 검정으로 설정합니다.

	TextOut(hDC, 20, 20, L"S: Start   P: Pause/Restart   T: Time   +: Fast   -: Slow   R: Reset   Q: Quit", 84); // 조작법을 출력합니다.

	if (showPauseInfo == 1) {                 // 일시정지 정보 출력 상태라면 실행합니다.
		wsprintf(str, L"Pause - Color Changed Bricks: %d, Removed Bricks: %d", changedBrickCount, removedBrickCount); // 통계 문자열을 만듭니다.
		TextOut(hDC, 20, 460, str, lstrlen(str)); // 통계 문자열을 화면에 출력합니다.
	}

	if (showTimeInfo == 1) {                  // 시간 정보 출력 상태라면 실행합니다.
		currentTime = time(NULL);             // 현재 시간을 가져옵니다.

		if (gameStarted == 1) {               // 게임이 시작된 상태라면 실행합니다.
			playTime = (int)difftime(currentTime, startTime); // 현재 시간과 시작 시간의 차이를 계산합니다.
			wsprintf(str, L"Start Time: %lld   Current Time: %lld   Playing Time: %d sec", (long long)startTime, (long long)currentTime, playTime); // 시간 정보를 문자열로 만듭니다.
		}
		else {                                // 게임이 아직 시작되지 않았다면 실행합니다.
			wsprintf(str, L"Game has not started yet."); // 시작되지 않았다는 문자열을 만듭니다.
		}

		TextOut(hDC, 20, 485, str, lstrlen(str)); // 시간 정보를 화면에 출력합니다.
	}
}

void ResetGame(HWND hWnd) {                  // 게임을 리셋하는 함수입니다.
	KillTimer(hWnd, TIMER_GAME);              // 공 이동 타이머를 멈춥니다.
	KillTimer(hWnd, TIMER_BRICK);             // 벽돌 이동 타이머를 멈춥니다.
	InitGame();                               // 게임 데이터를 처음 상태로 초기화합니다.
	InvalidateRect(hWnd, NULL, TRUE);         // 화면을 다시 그리도록 요청합니다.
}

void ChangeBallSpeed(HWND hWnd, int faster) { // 공의 속도를 바꾸는 함수입니다.
	if (faster == 1) {                        // 더 빠르게 만드는 경우입니다.
		if (ballTimerInterval > 5) {          // 타이머 간격이 너무 작아지지 않도록 제한합니다.
			ballTimerInterval -= 5;           // 타이머 간격을 줄여서 공을 빠르게 만듭니다.
		}
	}
	else {                                    // 더 느리게 만드는 경우입니다.
		if (ballTimerInterval < 80) {         // 타이머 간격이 너무 커지지 않도록 제한합니다.
			ballTimerInterval += 5;           // 타이머 간격을 늘려서 공을 느리게 만듭니다.
		}
	}

	if (gameRunning == 1) {                   // 게임이 움직이는 중이면 타이머를 새 속도로 다시 설정합니다.
		KillTimer(hWnd, TIMER_GAME);          // 기존 공 이동 타이머를 제거합니다.
		SetTimer(hWnd, TIMER_GAME, ballTimerInterval, NULL); // 새 간격으로 공 이동 타이머를 다시 시작합니다.
	}
}

void ProcessCommand(HWND hWnd, int command) { // 메뉴와 키보드 명령을 같이 처리하는 함수입니다.
	switch (command) {                         // 전달받은 명령 번호에 따라 실행할 코드를 선택합니다.
	case MENU_START:                           // Start 명령인 경우입니다.
		StartGame(hWnd);                      // 게임을 시작합니다.
		break;                                // switch문을 빠져나갑니다.

	case MENU_PAUSE:                           // Pause/Restart 명령인 경우입니다.
		PauseOrRestart(hWnd);                 // 멈춤 또는 재시작을 처리합니다.
		InvalidateRect(hWnd, NULL, TRUE);      // 화면을 다시 그리도록 요청합니다.
		break;                                // switch문을 빠져나갑니다.

	case MENU_TIME:                            // Time 명령인 경우입니다.
		showTimeInfo = 1;                     // 시간 정보를 화면에 출력하도록 설정합니다.
		InvalidateRect(hWnd, NULL, TRUE);      // 화면을 다시 그리도록 요청합니다.
		break;                                // switch문을 빠져나갑니다.

	case MENU_RESET:                           // Reset 명령인 경우입니다.
		ResetGame(hWnd);                      // 게임을 리셋합니다.
		break;                                // switch문을 빠져나갑니다.

	case MENU_QUIT:                            // Quit 명령인 경우입니다.
		PostQuitMessage(0);                   // 프로그램 종료 메시지를 발생시킵니다.
		break;                                // switch문을 빠져나갑니다.
	}
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpszCmdParam, int nCmdShow) // Windows 프로그램의 시작 함수입니다.
{
	HWND hWnd;                                 // 생성된 윈도우의 핸들을 저장할 변수입니다.
	MSG Message;                              // 메시지 정보를 저장할 구조체 변수입니다.
	WNDCLASSEX WndClass;                      // 윈도우 클래스 정보를 저장할 구조체 변수입니다.

	g_hInst = hInstance;                      // 전역 인스턴스 핸들에 현재 프로그램 인스턴스를 저장합니다.

	WndClass.cbSize = sizeof(WndClass);       // WNDCLASSEX 구조체의 크기를 저장합니다.
	WndClass.style = CS_HREDRAW | CS_VREDRAW; // 창의 가로 또는 세로 크기가 바뀌면 다시 그리도록 설정합니다.
	WndClass.lpfnWndProc = (WNDPROC)WndProc;  // 메시지를 처리할 함수를 등록합니다.
	WndClass.cbClsExtra = 0;                  // 추가 클래스 메모리를 사용하지 않습니다.
	WndClass.cbWndExtra = 0;                  // 추가 윈도우 메모리를 사용하지 않습니다.
	WndClass.hInstance = hInstance;           // 현재 프로그램 인스턴스를 저장합니다.
	WndClass.hIcon = LoadIcon(NULL, IDI_APPLICATION); // 기본 프로그램 아이콘을 사용합니다.
	WndClass.hCursor = LoadCursor(NULL, IDC_ARROW);   // 기본 화살표 커서를 사용합니다.
	WndClass.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH); // 배경색을 흰색으로 설정합니다.
	WndClass.lpszMenuName = NULL;             // 메뉴는 코드에서 직접 만들 것이므로 NULL로 둡니다.
	WndClass.lpszClassName = lpszClass;       // 윈도우 클래스 이름을 등록합니다.
	WndClass.hIconSm = LoadIcon(NULL, IDI_APPLICATION); // 작은 아이콘도 기본 아이콘을 사용합니다.

	RegisterClassEx(&WndClass);               // 위에서 설정한 윈도우 클래스를 운영체제에 등록합니다.

	hWnd = CreateWindow(lpszClass, lpszWindowName, WS_OVERLAPPEDWINDOW, 0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, NULL, (HMENU)NULL, hInstance, NULL); // 실제 윈도우 창을 생성합니다.

	ShowWindow(hWnd, nCmdShow);               // 생성한 윈도우를 화면에 보여줍니다.
	UpdateWindow(hWnd);                       // 윈도우를 즉시 한 번 갱신합니다.

	while (GetMessage(&Message, 0, 0, 0)) {   // 프로그램 메시지 큐에서 메시지를 계속 가져옵니다.
		TranslateMessage(&Message);           // 키보드 입력 메시지를 문자 메시지로 변환합니다.
		DispatchMessage(&Message);            // 메시지를 WndProc 함수로 전달합니다.
	}

	return (int)Message.wParam;               // 프로그램 종료 코드를 반환합니다.
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) // 윈도우 메시지를 처리하는 함수입니다.
{
	PAINTSTRUCT ps;                           // WM_PAINT에서 그림 그리기 정보를 저장하는 구조체입니다.
	HDC hDC;                                  // 그림을 그릴 때 사용할 디바이스 컨텍스트 핸들입니다.
	int mouseX;                               // 마우스의 x 좌표를 저장할 변수입니다.
	int mouseY;                               // 마우스의 y 좌표를 저장할 변수입니다.

	switch (uMsg) {                            // 전달된 메시지 종류에 따라 처리할 코드를 선택합니다.

	case WM_CREATE:                            // 윈도우가 처음 생성될 때 발생하는 메시지입니다.
		srand((unsigned int)time(NULL));       // 랜덤값이 매번 다르게 나오도록 현재 시간으로 시드를 설정합니다.
		GetClientRect(hWnd, &rectView);        // 현재 클라이언트 영역 크기를 가져옵니다.
		CreateGameMenu(hWnd);                 // 게임 메뉴를 생성합니다.
		InitGame();                           // 게임 데이터를 초기화합니다.
		break;                                // WM_CREATE 처리를 끝냅니다.

	case WM_SIZE:                              // 윈도우 크기가 변경될 때 발생하는 메시지입니다.
		GetClientRect(hWnd, &rectView);        // 변경된 클라이언트 영역 크기를 다시 가져옵니다.
		barY = rectView.bottom - 60;           // 창 크기에 맞게 바의 y 좌표를 아래쪽으로 맞춥니다.
		if (gameStarted == 0) {                // 게임이 아직 시작되지 않았다면 실행합니다.
			SetBallOnBar();                    // 공을 바 위에 다시 맞춥니다.
		}
		break;                                // WM_SIZE 처리를 끝냅니다.

	case WM_COMMAND:                           // 메뉴 명령이 들어왔을 때 발생하는 메시지입니다.
		ProcessCommand(hWnd, LOWORD(wParam));  // 메뉴에서 선택한 명령 번호를 처리합니다.
		break;                                // WM_COMMAND 처리를 끝냅니다.

	case WM_LBUTTONDOWN:                       // 마우스 왼쪽 버튼을 눌렀을 때 발생하는 메시지입니다.
		mouseX = LOWORD(lParam);               // lParam에서 마우스 x 좌표를 꺼냅니다.
		mouseY = HIWORD(lParam);               // lParam에서 마우스 y 좌표를 꺼냅니다.

		if (mouseX >= barX && mouseX <= barX + BAR_WIDTH && mouseY >= barY && mouseY <= barY + BAR_HEIGHT) { // 마우스가 바 위를 클릭했는지 검사합니다.
			isBarDrag = 1;                     // 바 드래그 상태를 켭니다.
			dragOffsetX = mouseX - barX;       // 클릭 위치와 바 왼쪽 사이의 차이를 저장합니다.
		}
		break;                                // WM_LBUTTONDOWN 처리를 끝냅니다.

	case WM_MOUSEMOVE:                         // 마우스가 움직일 때 발생하는 메시지입니다.
		mouseX = LOWORD(lParam);               // lParam에서 마우스 x 좌표를 꺼냅니다.

		if (isBarDrag == 1) {                  // 바를 드래그 중일 때만 실행합니다.
			barX = mouseX - dragOffsetX;       // 마우스 위치에 맞게 바의 x 좌표를 변경합니다.

			if (barX < 0) {                    // 바가 왼쪽 화면 밖으로 나가려는지 검사합니다.
				barX = 0;                      // 바를 왼쪽 끝에 고정합니다.
			}

			if (barX + BAR_WIDTH > rectView.right) { // 바가 오른쪽 화면 밖으로 나가려는지 검사합니다.
				barX = rectView.right - BAR_WIDTH;   // 바를 오른쪽 끝에 고정합니다.
			}

			if (gameStarted == 0) {            // 게임이 아직 시작되지 않았다면 실행합니다.
				SetBallOnBar();                // 공도 바 위에서 같이 이동하도록 합니다.
			}

			InvalidateRect(hWnd, NULL, TRUE);  // 화면을 다시 그리도록 요청합니다.
		}
		break;                                // WM_MOUSEMOVE 처리를 끝냅니다.

	case WM_LBUTTONUP:                         // 마우스 왼쪽 버튼을 뗐을 때 발생하는 메시지입니다.
		isBarDrag = 0;                         // 바 드래그 상태를 해제합니다.
		break;                                // WM_LBUTTONUP 처리를 끝냅니다.

	case WM_CHAR:                              // 일반 문자 키가 입력되었을 때 발생하는 메시지입니다.
		if (wParam == 's' || wParam == 'S') {  // s 또는 S 키가 입력되었는지 검사합니다.
			ProcessCommand(hWnd, MENU_START);  // Start 명령을 실행합니다.
		}
		else if (wParam == 'p' || wParam == 'P') { // p 또는 P 키가 입력되었는지 검사합니다.
			ProcessCommand(hWnd, MENU_PAUSE);  // Pause/Restart 명령을 실행합니다.
		}
		else if (wParam == 't' || wParam == 'T') { // t 또는 T 키가 입력되었는지 검사합니다.
			ProcessCommand(hWnd, MENU_TIME);   // Time 명령을 실행합니다.
		}
		else if (wParam == '+') {              // + 키가 입력되었는지 검사합니다.
			ChangeBallSpeed(hWnd, 1);          // 공 속도를 빠르게 만듭니다.
		}
		else if (wParam == '-') {              // - 키가 입력되었는지 검사합니다.
			ChangeBallSpeed(hWnd, 0);          // 공 속도를 느리게 만듭니다.
		}
		else if (wParam == 'r' || wParam == 'R') { // r 또는 R 키가 입력되었는지 검사합니다.
			ProcessCommand(hWnd, MENU_RESET);  // Reset 명령을 실행합니다.
		}
		else if (wParam == 'q' || wParam == 'Q') { // q 또는 Q 키가 입력되었는지 검사합니다.
			ProcessCommand(hWnd, MENU_QUIT);   // Quit 명령을 실행합니다.
		}

		InvalidateRect(hWnd, NULL, TRUE);      // 키 입력 후 화면을 다시 그립니다.
		break;                                // WM_CHAR 처리를 끝냅니다.

	case WM_TIMER:                             // 타이머가 작동할 때 발생하는 메시지입니다.
		if (wParam == TIMER_GAME) {            // 공 이동 타이머인지 확인합니다.
			if (gameRunning == 1) {            // 게임이 움직이는 상태일 때만 실행합니다.
				MoveBall(hWnd);                // 공을 이동시키고 충돌을 처리합니다.
				MoveFallingBricks();           // 떨어지는 벽돌들을 아래로 이동시킵니다.
				InvalidateRect(hWnd, NULL, TRUE); // 화면을 다시 그리도록 요청합니다.
			}
		}

		if (wParam == TIMER_BRICK) {           // 벽돌 이동 타이머인지 확인합니다.
			if (gameRunning == 1) {            // 게임이 움직이는 상태일 때만 실행합니다.
				MoveBrickGroup();              // 위쪽 벽돌 묶음을 좌우로 이동시킵니다.
				InvalidateRect(hWnd, NULL, TRUE); // 화면을 다시 그리도록 요청합니다.
			}
		}
		break;                                // WM_TIMER 처리를 끝냅니다.

	case WM_PAINT:                             // 화면을 다시 그려야 할 때 발생하는 메시지입니다.
		hDC = BeginPaint(hWnd, &ps);           // 그리기를 시작하고 HDC를 얻습니다.

		DrawBricks(hDC);                       // 위쪽 벽돌들을 그립니다.
		DrawFallingBricks(hDC);                // 아래로 떨어지는 벽돌들을 그립니다.
		DrawBar(hDC);                          // 아래쪽 바를 그립니다.
		DrawBall(hDC);                         // 공을 그립니다.
		DrawInfo(hDC);                         // 조작법과 정보를 출력합니다.

		EndPaint(hWnd, &ps);                   // 그리기를 종료합니다.
		break;                                // WM_PAINT 처리를 끝냅니다.

	case WM_DESTROY:                           // 윈도우가 종료될 때 발생하는 메시지입니다.
		KillTimer(hWnd, TIMER_GAME);           // 공 이동 타이머를 제거합니다.
		KillTimer(hWnd, TIMER_BRICK);          // 벽돌 이동 타이머를 제거합니다.
		PostQuitMessage(0);                    // 프로그램 종료 메시지를 발생시킵니다.
		break;                                // WM_DESTROY 처리를 끝냅니다.
	}

	return DefWindowProc(hWnd, uMsg, wParam, lParam); // 직접 처리하지 않은 메시지는 기본 메시지 처리 함수로 넘깁니다.
}