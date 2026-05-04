#include <windows.h>        // 윈도우 API 함수, 자료형, 메시지 사용
#include <tchar.h>          // TEXT(), TCHAR, LPCTSTR 사용
#include <stdlib.h>         // rand(), srand() 사용
#include <time.h>           // time() 사용

HINSTANCE g_hInst;                                      // 현재 프로그램 인스턴스 핸들
LPCTSTR lpszClass = L"My Window Class";                 // 윈도우 클래스 이름
LPCTSTR lpszWindowName = L"부분 이미지 돋보기 보기";    // 창 제목

LRESULT CALLBACK WndProc(HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam); // 메시지 처리 함수 선언

#define MAX_PASTE 5                 // p 키로 붙여 넣을 수 있는 최대 개수
#define MAX_DBL_COPY 10             // 더블클릭으로 만들 수 있는 최대 복사 개수

#define TIMER_BOUNCE 1              // m 키 이동 애니메이션 타이머 번호
#define TIMER_SIZE 2                // n 키 크기 애니메이션 타이머 번호

#define MODE_NONE 0                 // 마우스가 아무 작업도 하지 않는 상태
#define MODE_CREATE 1               // 마우스로 새 돋보기를 만드는 상태
#define MODE_MOVE 2                 // 돋보기를 이동하는 상태
#define MODE_RESIZE 3               // 돋보기 크기를 조절하는 상태

#define EDGE_LEFT 1                 // 왼쪽 가장자리 선택
#define EDGE_RIGHT 2                // 오른쪽 가장자리 선택
#define EDGE_TOP 4                  // 위쪽 가장자리 선택
#define EDGE_BOTTOM 8               // 아래쪽 가장자리 선택

#define EDGE_GAP 8                  // 가장자리 판정 범위
#define MIN_LENS_SIZE 30            // 돋보기 최소 크기

typedef struct COPY_INFO            // 복사된 그림의 위치 정보를 저장하는 구조체
{
    RECT rc;                        // 복사 그림이 그려질 위치
    int visible;                    // 보이는지 여부
} COPY_INFO;

HBITMAP g_hBmp[2];                  // test.bmp, image.bmp를 저장하는 비트맵 핸들 배열
int g_bmpW[2];                      // 각 비트맵의 가로 크기
int g_bmpH[2];                      // 각 비트맵의 세로 크기
int g_curImg = 0;                   // 현재 출력 중인 이미지 번호, 0이면 test.bmp, 1이면 image.bmp

RECT g_lens;                        // 현재 돋보기 사각형 위치
int g_hasLens = FALSE;              // 돋보기가 만들어졌는지 여부

double g_lensScale = 1.0;           // 돋보기 내부 그림 확대/축소 배율
int g_invertLens = FALSE;           // 돋보기 내부 그림 색상 반전 여부

int g_hasCopied = FALSE;            // c 키로 복사를 했는지 여부
COPY_INFO g_paste[MAX_PASTE];       // p 키로 붙여 넣은 그림 위치 배열
int g_pasteCount = 0;               // 현재 p 키 붙여 넣기 개수
int g_pasteReplaceIndex = 0;        // 5개가 찼을 때 교체할 위치 번호

COPY_INFO g_dblCopy[MAX_DBL_COPY];  // 더블클릭으로 만들어진 복사 그림 위치 배열
int g_dblCount = 0;                 // 현재 더블클릭 복사 그림 개수

int g_fullPaste = FALSE;            // f 키 전체 붙여넣기 상태
int g_flipH = FALSE;                // h 키 좌우 반전 상태
int g_flipV = FALSE;                // v 키 상하 반전 상태

int g_dragMode = MODE_NONE;         // 현재 마우스 드래그 상태
int g_resizeFlag = 0;               // 어떤 가장자리를 크기 조절 중인지 저장
POINT g_dragStart;                  // 드래그 시작 좌표
RECT g_dragOldRect;                 // 드래그 시작 당시 돋보기 위치

int g_bounceMove = FALSE;           // m 키 튕기기 이동 상태
int g_dx = 6;                       // 돋보기 자동 이동 x 방향 속도
int g_dy = 5;                       // 돋보기 자동 이동 y 방향 속도

int g_sizeAni = FALSE;              // n 키 크기 애니메이션 상태
int g_sizeDir = 1;                  // 커지는 중인지 작아지는 중인지 저장
int g_normalW = 160;                // 애니메이션 정지 시 돌아갈 돋보기 기본 가로 크기
int g_normalH = 120;                // 애니메이션 정지 시 돌아갈 돋보기 기본 세로 크기

int RectW(RECT rc)                  // RECT의 가로 길이를 구하는 함수
{
    return rc.right - rc.left;      // 오른쪽 좌표에서 왼쪽 좌표를 뺀 값 반환
}

int RectH(RECT rc)                  // RECT의 세로 길이를 구하는 함수
{
    return rc.bottom - rc.top;      // 아래쪽 좌표에서 위쪽 좌표를 뺀 값 반환
}

int MaxInt(int a, int b)            // 두 정수 중 큰 값을 반환하는 함수
{
    if (a > b) return a;            // a가 크면 a 반환
    return b;                       // 아니면 b 반환
}

int MinInt(int a, int b)            // 두 정수 중 작은 값을 반환하는 함수
{
    if (a < b) return a;            // a가 작으면 a 반환
    return b;                       // 아니면 b 반환
}

RECT NormalizeRectSimple(RECT rc)   // 왼쪽/오른쪽, 위/아래가 뒤집힌 RECT를 정상화하는 함수
{
    int temp;                       // 좌표 교환에 사용할 임시 변수

    if (rc.left > rc.right) {       // 왼쪽 좌표가 오른쪽 좌표보다 크면
        temp = rc.left;             // left 값을 임시 저장
        rc.left = rc.right;         // right 값을 left로 이동
        rc.right = temp;            // 임시 저장한 값을 right로 이동
    }

    if (rc.top > rc.bottom) {       // 위쪽 좌표가 아래쪽 좌표보다 크면
        temp = rc.top;              // top 값을 임시 저장
        rc.top = rc.bottom;         // bottom 값을 top으로 이동
        rc.bottom = temp;           // 임시 저장한 값을 bottom으로 이동
    }

    return rc;                      // 정리된 RECT 반환
}

void ClampRectToClient(RECT* rc, RECT client)              // 사각형이 화면 밖으로 나가지 않게 보정
{
    int cw = RectW(client);                               // 클라이언트 영역 가로 크기
    int ch = RectH(client);                               // 클라이언트 영역 세로 크기
    int rw = RectW(*rc);                                  // 사각형 가로 크기
    int rh = RectH(*rc);                                  // 사각형 세로 크기

    if (rw > cw) {                                        // 사각형이 화면보다 가로로 크면
        rc->left = 0;                                     // 왼쪽을 화면 시작으로 맞춤
        rc->right = cw;                                   // 오른쪽을 화면 끝으로 맞춤
    }
    else {                                                // 사각형이 화면 안에 들어갈 수 있으면
        if (rc->left < 0) OffsetRect(rc, -rc->left, 0);    // 왼쪽이 화면 밖이면 오른쪽으로 이동
        if (rc->right > cw) OffsetRect(rc, cw - rc->right, 0); // 오른쪽이 화면 밖이면 왼쪽으로 이동
    }

    if (rh > ch) {                                        // 사각형이 화면보다 세로로 크면
        rc->top = 0;                                      // 위쪽을 화면 시작으로 맞춤
        rc->bottom = ch;                                  // 아래쪽을 화면 끝으로 맞춤
    }
    else {                                                // 사각형이 화면 안에 들어갈 수 있으면
        if (rc->top < 0) OffsetRect(rc, 0, -rc->top);      // 위쪽이 화면 밖이면 아래로 이동
        if (rc->bottom > ch) OffsetRect(rc, 0, ch - rc->bottom); // 아래쪽이 화면 밖이면 위로 이동
    }
}

void UpdateNormalLensSize()               // 현재 돋보기 크기를 기본 크기로 저장
{
    if (g_hasLens == FALSE) return;        // 돋보기가 없으면 함수 종료

    g_normalW = RectW(g_lens);             // 현재 돋보기 가로 크기 저장
    g_normalH = RectH(g_lens);             // 현재 돋보기 세로 크기 저장

    if (g_normalW < MIN_LENS_SIZE) g_normalW = MIN_LENS_SIZE; // 너무 작으면 최소 크기로 보정
    if (g_normalH < MIN_LENS_SIZE) g_normalH = MIN_LENS_SIZE; // 너무 작으면 최소 크기로 보정
}

void SetLensByCenter(int cx, int cy, int w, int h, RECT client) // 중심 좌표와 크기로 돋보기 위치 설정
{
    g_lens.left = cx - w / 2;                              // 중심 기준 왼쪽 좌표 계산
    g_lens.right = g_lens.left + w;                        // 오른쪽 좌표 계산
    g_lens.top = cy - h / 2;                               // 중심 기준 위쪽 좌표 계산
    g_lens.bottom = g_lens.top + h;                        // 아래쪽 좌표 계산

    ClampRectToClient(&g_lens, client);                    // 화면 밖으로 나가지 않게 보정
}

void MakeDefaultLensAt(int x, int y, RECT client)          // 지정 위치에 기본 돋보기 생성
{
    int w = 160;                                           // 기본 돋보기 가로 크기
    int h = 120;                                           // 기본 돋보기 세로 크기

    if (w > RectW(client)) w = RectW(client);              // 화면보다 크면 화면 크기에 맞춤
    if (h > RectH(client)) h = RectH(client);              // 화면보다 크면 화면 크기에 맞춤

    g_hasLens = TRUE;                                      // 돋보기가 있다고 표시
    SetLensByCenter(x, y, w, h, client);                   // 지정 위치를 중심으로 돋보기 생성
    UpdateNormalLensSize();                               // 기본 크기 저장
}

RECT MakeRandomRect(RECT client, int w, int h)             // 랜덤 위치 RECT를 만드는 함수
{
    RECT rc;                                               // 랜덤 사각형 변수
    int cw = RectW(client);                                // 클라이언트 가로 크기
    int ch = RectH(client);                                // 클라이언트 세로 크기
    int maxX;                                              // 랜덤 x 범위
    int maxY;                                              // 랜덤 y 범위

    if (w < MIN_LENS_SIZE) w = MIN_LENS_SIZE;              // 너무 작은 가로 크기 보정
    if (h < MIN_LENS_SIZE) h = MIN_LENS_SIZE;              // 너무 작은 세로 크기 보정
    if (w > cw) w = cw;                                    // 화면보다 크면 화면 가로에 맞춤
    if (h > ch) h = ch;                                    // 화면보다 크면 화면 세로에 맞춤

    maxX = cw - w;                                         // 랜덤으로 고를 수 있는 최대 x
    maxY = ch - h;                                         // 랜덤으로 고를 수 있는 최대 y

    rc.left = rand() % (maxX + 1);                         // 랜덤 왼쪽 좌표
    rc.top = rand() % (maxY + 1);                          // 랜덤 위쪽 좌표
    rc.right = rc.left + w;                                // 오른쪽 좌표
    rc.bottom = rc.top + h;                                // 아래쪽 좌표

    return rc;                                             // 만들어진 랜덤 사각형 반환
}

void LoadBitmaps()                                         // bmp 파일 2개를 불러오는 함수
{
    BITMAP bm;                                             // 비트맵 정보를 받을 구조체

    g_hBmp[0] = (HBITMAP)LoadImage(NULL, TEXT("test.bmp"), IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE | LR_CREATEDIBSECTION); // test.bmp 로드
    g_hBmp[1] = (HBITMAP)LoadImage(NULL, TEXT("image.bmp"), IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE | LR_CREATEDIBSECTION); // image.bmp 로드

    g_bmpW[0] = 0;                                         // 첫 번째 이미지 가로 초기화
    g_bmpH[0] = 0;                                         // 첫 번째 이미지 세로 초기화
    g_bmpW[1] = 0;                                         // 두 번째 이미지 가로 초기화
    g_bmpH[1] = 0;                                         // 두 번째 이미지 세로 초기화

    if (g_hBmp[0] != NULL) {                               // 첫 번째 이미지가 정상 로드되었으면
        GetObject(g_hBmp[0], sizeof(BITMAP), &bm);         // 비트맵 정보 얻기
        g_bmpW[0] = bm.bmWidth;                            // 가로 크기 저장
        g_bmpH[0] = bm.bmHeight;                           // 세로 크기 저장
    }

    if (g_hBmp[1] != NULL) {                               // 두 번째 이미지가 정상 로드되었으면
        GetObject(g_hBmp[1], sizeof(BITMAP), &bm);         // 비트맵 정보 얻기
        g_bmpW[1] = bm.bmWidth;                            // 가로 크기 저장
        g_bmpH[1] = bm.bmHeight;                           // 세로 크기 저장
    }
}

void DeleteBitmaps()                                       // 사용한 비트맵을 삭제하는 함수
{
    if (g_hBmp[0] != NULL) DeleteObject(g_hBmp[0]);         // 첫 번째 비트맵 삭제
    if (g_hBmp[1] != NULL) DeleteObject(g_hBmp[1]);         // 두 번째 비트맵 삭제
}

void DrawMissingMessage(HDC hDC)                           // 이미지가 없을 때 안내 문구 출력
{
    TCHAR msg1[] = TEXT("BMP 파일을 찾지 못했습니다.");      // 첫 번째 안내 문장
    TCHAR msg2[] = TEXT("test.bmp, image.bmp를 exe와 같은 폴더에 넣으세요."); // 두 번째 안내 문장

    SetBkMode(hDC, TRANSPARENT);                           // 글자 배경을 투명하게 설정
    TextOut(hDC, 30, 30, msg1, lstrlen(msg1));              // 첫 번째 문장 출력
    TextOut(hDC, 30, 55, msg2, lstrlen(msg2));              // 두 번째 문장 출력
}

void DrawBaseImage(HDC hDC, RECT client)                   // 현재 선택된 원본 이미지를 화면 전체에 그림
{
    HDC imgDC;                                             // 이미지 전용 메모리 DC
    HGDIOBJ oldBmp;                                        // 이전에 선택되어 있던 비트맵 저장
    int cw = RectW(client);                                // 화면 가로 크기
    int ch = RectH(client);                                // 화면 세로 크기

    if (g_hBmp[g_curImg] == NULL) {                        // 현재 이미지가 없으면
        FillRect(hDC, &client, (HBRUSH)GetStockObject(WHITE_BRUSH)); // 흰 배경으로 칠함
        DrawMissingMessage(hDC);                           // 안내 문구 출력
        return;                                            // 함수 종료
    }

    imgDC = CreateCompatibleDC(hDC);                       // 현재 DC와 호환되는 메모리 DC 생성
    oldBmp = SelectObject(imgDC, g_hBmp[g_curImg]);         // 이미지 DC에 현재 비트맵 선택

    SetStretchBltMode(hDC, HALFTONE);                      // 이미지 확대/축소 품질을 조금 좋게 설정
    StretchBlt(hDC, 0, 0, cw, ch, imgDC, 0, 0, g_bmpW[g_curImg], g_bmpH[g_curImg], SRCCOPY); // 이미지를 화면 전체에 그림

    SelectObject(imgDC, oldBmp);                           // 원래 비트맵으로 복구
    DeleteDC(imgDC);                                       // 이미지 DC 삭제
}

RECT GetLensSourceRect(RECT client, double scale)          // 돋보기 위치에 맞는 원본 이미지 영역 계산
{
    RECT src;                                              // 원본 이미지에서 잘라낼 영역
    int cw = RectW(client);                                // 화면 가로 크기
    int ch = RectH(client);                                // 화면 세로 크기
    int iw = g_bmpW[g_curImg];                             // 현재 이미지 가로 크기
    int ih = g_bmpH[g_curImg];                             // 현재 이미지 세로 크기
    double centerClientX;                                  // 돋보기 중심 x, 화면 좌표 기준
    double centerClientY;                                  // 돋보기 중심 y, 화면 좌표 기준
    double centerImgX;                                     // 돋보기 중심 x, 이미지 좌표 기준
    double centerImgY;                                     // 돋보기 중심 y, 이미지 좌표 기준
    double baseSrcW;                                       // 배율 1일 때 원본에서 가져올 가로 영역
    double baseSrcH;                                       // 배율 1일 때 원본에서 가져올 세로 영역
    double srcW;                                           // 실제 원본에서 가져올 가로 영역
    double srcH;                                           // 실제 원본에서 가져올 세로 영역

    src.left = 0;                                          // 기본 왼쪽 좌표
    src.top = 0;                                           // 기본 위쪽 좌표
    src.right = 1;                                         // 기본 오른쪽 좌표
    src.bottom = 1;                                        // 기본 아래쪽 좌표

    if (cw <= 0 || ch <= 0 || iw <= 0 || ih <= 0) return src; // 잘못된 크기면 기본값 반환

    if (scale <= 0.1) scale = 0.1;                         // 배율이 너무 작아지는 것을 방지

    centerClientX = (g_lens.left + g_lens.right) / 2.0;    // 돋보기 중심 x 계산
    centerClientY = (g_lens.top + g_lens.bottom) / 2.0;    // 돋보기 중심 y 계산

    centerImgX = centerClientX * iw / cw;                  // 화면 x 좌표를 이미지 x 좌표로 변환
    centerImgY = centerClientY * ih / ch;                  // 화면 y 좌표를 이미지 y 좌표로 변환

    baseSrcW = (double)RectW(g_lens) * iw / cw;            // 현재 돋보기 가로만큼 원본 이미지 영역 계산
    baseSrcH = (double)RectH(g_lens) * ih / ch;            // 현재 돋보기 세로만큼 원본 이미지 영역 계산

    srcW = baseSrcW / scale;                               // 확대면 원본 영역을 작게, 축소면 원본 영역을 크게 잡음
    srcH = baseSrcH / scale;                               // 확대면 원본 영역을 작게, 축소면 원본 영역을 크게 잡음

    if (srcW < 1) srcW = 1;                                // 원본 영역이 1보다 작아지지 않게 보정
    if (srcH < 1) srcH = 1;                                // 원본 영역이 1보다 작아지지 않게 보정
    if (srcW > iw) srcW = iw;                              // 원본 영역이 이미지보다 커지지 않게 보정
    if (srcH > ih) srcH = ih;                              // 원본 영역이 이미지보다 커지지 않게 보정

    src.left = (int)(centerImgX - srcW / 2.0);             // 원본 영역 왼쪽 계산
    src.top = (int)(centerImgY - srcH / 2.0);              // 원본 영역 위쪽 계산
    src.right = src.left + (int)srcW;                      // 원본 영역 오른쪽 계산
    src.bottom = src.top + (int)srcH;                      // 원본 영역 아래쪽 계산

    if (src.left < 0) {                                    // 왼쪽이 이미지 밖이면
        src.right -= src.left;                             // 오른쪽도 같이 이동
        src.left = 0;                                      // 왼쪽을 0으로 맞춤
    }

    if (src.top < 0) {                                     // 위쪽이 이미지 밖이면
        src.bottom -= src.top;                             // 아래쪽도 같이 이동
        src.top = 0;                                       // 위쪽을 0으로 맞춤
    }

    if (src.right > iw) {                                  // 오른쪽이 이미지 밖이면
        src.left -= src.right - iw;                        // 왼쪽도 같이 이동
        src.right = iw;                                    // 오른쪽을 이미지 끝으로 맞춤
    }

    if (src.bottom > ih) {                                 // 아래쪽이 이미지 밖이면
        src.top -= src.bottom - ih;                        // 위쪽도 같이 이동
        src.bottom = ih;                                   // 아래쪽을 이미지 끝으로 맞춤
    }

    if (src.left < 0) src.left = 0;                        // 보정 후에도 왼쪽이 음수면 0
    if (src.top < 0) src.top = 0;                          // 보정 후에도 위쪽이 음수면 0
    if (src.right <= src.left) src.right = src.left + 1;   // 오른쪽 좌표가 잘못되면 최소 1픽셀 확보
    if (src.bottom <= src.top) src.bottom = src.top + 1;   // 아래쪽 좌표가 잘못되면 최소 1픽셀 확보

    return src;                                            // 계산된 원본 영역 반환
}

void DrawLensContent(HDC hDC, RECT client, RECT dest, double scale, int invert, int flipH, int flipV) // 돋보기 안의 그림을 특정 위치에 그림
{
    HDC imgDC;                                             // 이미지용 메모리 DC
    HGDIOBJ oldBmp;                                        // 이전 선택 객체 저장
    RECT src;                                              // 원본 이미지에서 가져올 영역
    int sx;                                                // StretchBlt에 사용할 원본 x
    int sy;                                                // StretchBlt에 사용할 원본 y
    int sw;                                                // StretchBlt에 사용할 원본 가로
    int sh;                                                // StretchBlt에 사용할 원본 세로
    int dw = RectW(dest);                                  // 목적지 가로 크기
    int dh = RectH(dest);                                  // 목적지 세로 크기
    DWORD rop;                                             // 래스터 연산 코드

    if (g_hasLens == FALSE) return;                        // 돋보기가 없으면 그릴 수 없으므로 종료
    if (g_hBmp[g_curImg] == NULL) return;                  // 이미지가 없으면 종료
    if (dw <= 0 || dh <= 0) return;                        // 목적지 크기가 잘못되면 종료

    src = GetLensSourceRect(client, scale);                // 돋보기 위치와 배율에 맞는 원본 영역 계산

    sx = src.left;                                         // 기본 원본 x는 src.left
    sy = src.top;                                          // 기본 원본 y는 src.top
    sw = RectW(src);                                       // 기본 원본 가로 크기
    sh = RectH(src);                                       // 기본 원본 세로 크기

    if (flipH == TRUE) {                                   // 좌우 반전이면
        sx = src.right;                                    // 오른쪽에서 시작
        sw = -sw;                                          // 음수 폭을 사용해서 좌우 반전
    }

    if (flipV == TRUE) {                                   // 상하 반전이면
        sy = src.bottom;                                   // 아래쪽에서 시작
        sh = -sh;                                          // 음수 높이를 사용해서 상하 반전
    }

    if (invert == TRUE) rop = NOTSRCCOPY;                  // 반전 상태면 원본 색상을 반전해서 복사
    else rop = SRCCOPY;                                    // 아니면 원본 그대로 복사

    imgDC = CreateCompatibleDC(hDC);                       // 이미지 출력용 메모리 DC 생성
    oldBmp = SelectObject(imgDC, g_hBmp[g_curImg]);         // 현재 이미지를 메모리 DC에 선택

    SetStretchBltMode(hDC, HALFTONE);                      // 확대/축소 품질 설정
    StretchBlt(hDC, dest.left, dest.top, dw, dh, imgDC, sx, sy, sw, sh, rop); // 원본 일부를 목적지에 확대/축소하여 그림

    SelectObject(imgDC, oldBmp);                           // 이전 객체 복구
    DeleteDC(imgDC);                                       // 이미지 DC 삭제
}

void DrawFrame(HDC hDC, RECT rc, COLORREF color, int thick) // 사각형 테두리를 그리는 함수
{
    HPEN hPen;                                             // 새 펜 핸들
    HGDIOBJ oldPen;                                        // 이전 펜 저장
    HGDIOBJ oldBrush;                                      // 이전 브러시 저장

    hPen = CreatePen(PS_SOLID, thick, color);              // 지정 색상과 굵기의 실선 펜 생성
    oldPen = SelectObject(hDC, hPen);                      // 새 펜 선택
    oldBrush = SelectObject(hDC, GetStockObject(NULL_BRUSH)); // 내부를 칠하지 않는 브러시 선택

    Rectangle(hDC, rc.left, rc.top, rc.right, rc.bottom);  // 사각형 테두리 출력

    SelectObject(hDC, oldBrush);                           // 이전 브러시 복구
    SelectObject(hDC, oldPen);                             // 이전 펜 복구
    DeleteObject(hPen);                                    // 생성한 펜 삭제
}

void DrawScene(HDC hDC, RECT client)                       // 전체 화면을 그리는 함수
{
    int i;                                                 // 반복문 변수

    if (g_fullPaste == TRUE && g_hasLens == TRUE) {        // 전체 붙여넣기 상태이고 돋보기가 있으면
        DrawLensContent(hDC, client, client, g_lensScale, FALSE, g_flipH, g_flipV); // 돋보기 안의 그림을 화면 전체에 그림
    }
    else {                                                 // 전체 붙여넣기 상태가 아니면
        DrawBaseImage(hDC, client);                        // 원본 이미지를 화면 전체에 그림
    }

    for (i = 0; i < g_pasteCount; i++) {                   // p 키로 붙인 그림 개수만큼 반복
        if (g_paste[i].visible == TRUE) {                  // 해당 복사 그림이 보이면
            DrawLensContent(hDC, client, g_paste[i].rc, g_lensScale, FALSE, g_flipH, g_flipV); // 현재 돋보기 위치의 그림을 복사 위치에 그림
            DrawFrame(hDC, g_paste[i].rc, RGB(0, 120, 255), 1); // 복사 그림 테두리 출력
        }
    }

    for (i = 0; i < g_dblCount; i++) {                     // 더블클릭 복사 그림 개수만큼 반복
        if (g_dblCopy[i].visible == TRUE) {                // 해당 복사 그림이 보이면
            DrawLensContent(hDC, client, g_dblCopy[i].rc, g_lensScale, FALSE, g_flipH, g_flipV); // 현재 돋보기 위치의 그림을 랜덤 위치에 그림
            DrawFrame(hDC, g_dblCopy[i].rc, RGB(0, 180, 80), 1); // 더블클릭 복사 그림 테두리 출력
        }
    }

    if (g_hasLens == TRUE) {                               // 돋보기가 있으면
        DrawLensContent(hDC, client, g_lens, g_lensScale, g_invertLens, FALSE, FALSE); // 돋보기 내부 그림을 확대/축소/반전 상태로 그림
        DrawFrame(hDC, g_lens, RGB(255, 0, 0), 2);         // 돋보기 테두리를 빨간색으로 그림
    }
}

void AddPasteCopy(RECT client)                             // p 키 붙여넣기 추가 함수
{
    int w;                                                 // 붙여넣기 가로 크기
    int h;                                                 // 붙여넣기 세로 크기
    int index;                                             // 저장할 배열 번호

    if (g_hasLens == FALSE) return;                        // 돋보기가 없으면 종료
    if (g_hasCopied == FALSE) return;                      // c 키로 복사하지 않았으면 종료

    w = RectW(g_lens) / 2;                                 // p 붙여넣기는 돋보기의 1/2 가로 크기
    h = RectH(g_lens) / 2;                                 // p 붙여넣기는 돋보기의 1/2 세로 크기

    if (g_pasteCount < MAX_PASTE) {                        // 아직 5개보다 적으면
        index = g_pasteCount;                              // 새 칸에 저장
        g_pasteCount++;                                    // 개수 증가
    }
    else {                                                 // 이미 5개가 있으면
        index = g_pasteReplaceIndex;                       // 기존 칸 하나를 교체
        g_pasteReplaceIndex++;                             // 다음 교체 위치 증가
        if (g_pasteReplaceIndex >= MAX_PASTE) g_pasteReplaceIndex = 0; // 5를 넘으면 다시 0
    }

    g_paste[index].rc = MakeRandomRect(client, w, h);       // 랜덤 위치 생성
    g_paste[index].visible = TRUE;                         // 보이도록 설정
}

void AddDoubleClickCopy(int x, int y, RECT client)          // 더블클릭 복사 그림 추가 함수
{
    int w;                                                 // 복사 그림 가로 크기
    int h;                                                 // 복사 그림 세로 크기

    if (g_hasLens == FALSE) {                              // 돋보기가 없으면
        MakeDefaultLensAt(x, y, client);                   // 더블클릭 위치에 기본 돋보기 생성
    }

    if (g_dblCount >= MAX_DBL_COPY) {                      // 이미 10개가 있으면
        g_dblCount = 0;                                    // 11번째 더블클릭에서 모두 삭제
        return;                                            // 새 그림은 만들지 않고 종료
    }

    w = RectW(g_lens);                                     // 더블클릭 복사 그림은 돋보기와 같은 가로 크기
    h = RectH(g_lens);                                     // 더블클릭 복사 그림은 돋보기와 같은 세로 크기

    g_dblCopy[g_dblCount].rc = MakeRandomRect(client, w, h); // 랜덤 위치 생성
    g_dblCopy[g_dblCount].visible = TRUE;                  // 보이도록 설정
    g_dblCount++;                                          // 개수 증가
}

int GetMouseWorkMode(int x, int y)                         // 마우스 클릭 위치에 따라 작업 모드 결정
{
    int nearLeft;                                          // 왼쪽 가장자리 근처인지 여부
    int nearRight;                                         // 오른쪽 가장자리 근처인지 여부
    int nearTop;                                           // 위쪽 가장자리 근처인지 여부
    int nearBottom;                                        // 아래쪽 가장자리 근처인지 여부
    POINT pt;                                              // PtInRect에 사용할 점 좌표

    if (g_hasLens == FALSE) return MODE_CREATE;            // 돋보기가 없으면 새로 만들기

    pt.x = x;                                              // 점의 x 좌표 설정
    pt.y = y;                                              // 점의 y 좌표 설정

    nearLeft = (x >= g_lens.left - EDGE_GAP && x <= g_lens.left + EDGE_GAP && y >= g_lens.top - EDGE_GAP && y <= g_lens.bottom + EDGE_GAP); // 왼쪽 가장자리 판정
    nearRight = (x >= g_lens.right - EDGE_GAP && x <= g_lens.right + EDGE_GAP && y >= g_lens.top - EDGE_GAP && y <= g_lens.bottom + EDGE_GAP); // 오른쪽 가장자리 판정
    nearTop = (y >= g_lens.top - EDGE_GAP && y <= g_lens.top + EDGE_GAP && x >= g_lens.left - EDGE_GAP && x <= g_lens.right + EDGE_GAP); // 위쪽 가장자리 판정
    nearBottom = (y >= g_lens.bottom - EDGE_GAP && y <= g_lens.bottom + EDGE_GAP && x >= g_lens.left - EDGE_GAP && x <= g_lens.right + EDGE_GAP); // 아래쪽 가장자리 판정

    g_resizeFlag = 0;                                      // 크기 조절 방향 초기화

    if (nearLeft == TRUE) g_resizeFlag |= EDGE_LEFT;       // 왼쪽 가장자리면 플래그 추가
    if (nearRight == TRUE) g_resizeFlag |= EDGE_RIGHT;     // 오른쪽 가장자리면 플래그 추가
    if (nearTop == TRUE) g_resizeFlag |= EDGE_TOP;         // 위쪽 가장자리면 플래그 추가
    if (nearBottom == TRUE) g_resizeFlag |= EDGE_BOTTOM;   // 아래쪽 가장자리면 플래그 추가

    if (g_resizeFlag != 0) return MODE_RESIZE;             // 가장자리면 크기 조절 모드 반환

    if (PtInRect(&g_lens, pt)) return MODE_MOVE;           // 돋보기 내부면 이동 모드 반환

    return MODE_CREATE;                                    // 내부도 가장자리도 아니면 새 돋보기 만들기
}

void ResetAll(HWND hWnd)                                   // r 키 리셋 함수
{
    g_hasLens = FALSE;                                     // 돋보기 제거
    g_lensScale = 1.0;                                     // 확대/축소 배율 초기화
    g_invertLens = FALSE;                                  // 반전 해제

    g_hasCopied = FALSE;                                   // 복사 상태 해제
    g_pasteCount = 0;                                      // p 붙여넣기 제거
    g_pasteReplaceIndex = 0;                               // 교체 인덱스 초기화
    g_dblCount = 0;                                        // 더블클릭 복사 제거

    g_fullPaste = FALSE;                                   // 전체 붙여넣기 해제
    g_flipH = FALSE;                                       // 좌우 반전 해제
    g_flipV = FALSE;                                       // 상하 반전 해제

    g_bounceMove = FALSE;                                  // 튕기기 이동 해제
    g_sizeAni = FALSE;                                     // 크기 애니메이션 해제

    KillTimer(hWnd, TIMER_BOUNCE);                         // 튕기기 타이머 종료
    KillTimer(hWnd, TIMER_SIZE);                           // 크기 애니메이션 타이머 종료

    InvalidateRect(hWnd, NULL, FALSE);                     // 화면 다시 그리기
}

void MoveLensByKey(HWND hWnd, int mx, int my)              // 방향키로 돋보기를 이동하는 함수
{
    RECT client;                                           // 클라이언트 영역 저장

    if (g_hasLens == FALSE) return;                        // 돋보기가 없으면 종료

    GetClientRect(hWnd, &client);                          // 현재 화면 크기 얻기
    OffsetRect(&g_lens, mx, my);                           // 돋보기 위치 이동
    ClampRectToClient(&g_lens, client);                    // 화면 밖으로 나가지 않게 보정

    InvalidateRect(hWnd, NULL, FALSE);                     // 다시 그리기
}

void StopSizeAnimation(HWND hWnd)                          // n 애니메이션을 멈추고 원래 크기로 돌리는 함수
{
    RECT client;                                           // 클라이언트 영역
    int cx;                                                // 돋보기 중심 x
    int cy;                                                // 돋보기 중심 y

    if (g_hasLens == FALSE) return;                        // 돋보기가 없으면 종료

    GetClientRect(hWnd, &client);                          // 화면 크기 얻기

    cx = (g_lens.left + g_lens.right) / 2;                 // 현재 돋보기 중심 x
    cy = (g_lens.top + g_lens.bottom) / 2;                 // 현재 돋보기 중심 y

    g_sizeAni = FALSE;                                     // 크기 애니메이션 상태 해제
    KillTimer(hWnd, TIMER_SIZE);                           // 크기 애니메이션 타이머 종료

    SetLensByCenter(cx, cy, g_normalW, g_normalH, client); // 원래 크기로 복구
    InvalidateRect(hWnd, NULL, FALSE);                     // 다시 그리기
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpszCmdParam, int nCmdShow)
{
    HWND hWnd;                                             // 생성할 윈도우 핸들
    MSG Message;                                           // 메시지 저장 구조체
    WNDCLASSEX WndClass;                                   // 윈도우 클래스 구조체

    g_hInst = hInstance;                                   // 전역 인스턴스 핸들 저장

    WndClass.cbSize = sizeof(WndClass);                    // 구조체 크기 설정
    WndClass.style = CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS; // 가로/세로 다시 그리기 + 더블클릭 메시지 사용
    WndClass.lpfnWndProc = (WNDPROC)WndProc;               // 메시지 처리 함수 등록
    WndClass.cbClsExtra = 0;                               // 추가 클래스 메모리 사용 안 함
    WndClass.cbWndExtra = 0;                               // 추가 윈도우 메모리 사용 안 함
    WndClass.hInstance = hInstance;                        // 현재 인스턴스 등록
    WndClass.hIcon = LoadIcon(NULL, IDI_APPLICATION);      // 기본 아이콘 사용
    WndClass.hCursor = LoadCursor(NULL, IDC_ARROW);        // 기본 마우스 커서 사용
    WndClass.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH); // 기본 배경 흰색
    WndClass.lpszMenuName = NULL;                          // 메뉴 사용 안 함
    WndClass.lpszClassName = lpszClass;                    // 클래스 이름 설정
    WndClass.hIconSm = LoadIcon(NULL, IDI_APPLICATION);    // 작은 아이콘 설정

    RegisterClassEx(&WndClass);                            // 윈도우 클래스 등록

    hWnd = CreateWindow(lpszClass, lpszWindowName, WS_OVERLAPPEDWINDOW, 0, 0, 900, 650, NULL, (HMENU)NULL, hInstance, NULL); // 윈도우 생성

    ShowWindow(hWnd, nCmdShow);                            // 윈도우 보이기
    UpdateWindow(hWnd);                                    // 윈도우 즉시 갱신

    while (GetMessage(&Message, 0, 0, 0)) {                // 메시지 큐에서 메시지를 계속 가져옴
        TranslateMessage(&Message);                        // 키보드 메시지를 문자 메시지로 변환
        DispatchMessage(&Message);                         // 메시지를 WndProc으로 전달
    }

    return Message.wParam;                                 // 프로그램 종료 코드 반환
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    PAINTSTRUCT ps;                                        // BeginPaint/EndPaint에 필요한 구조체
    HDC hDC;                                               // 화면 DC
    HDC memDC;                                             // 더블 버퍼링용 메모리 DC
    HBITMAP hBackBmp;                                      // 더블 버퍼링용 백버퍼 비트맵
    HGDIOBJ oldBackBmp;                                    // 이전 백버퍼 객체
    RECT client;                                           // 클라이언트 영역
    int x;                                                 // 마우스 x 좌표
    int y;                                                 // 마우스 y 좌표
    int dx;                                                // 드래그 x 이동량
    int dy;                                                // 드래그 y 이동량
    RECT temp;                                             // 임시 RECT
    int cx;                                                // 중심 x
    int cy;                                                // 중심 y
    int w;                                                 // 가로 크기
    int h;                                                 // 세로 크기

    switch (uMsg) {                                        // 메시지 종류에 따라 처리
    case WM_CREATE:                                        // 윈도우가 처음 생성될 때
        srand((unsigned int)time(NULL));                   // 랜덤 위치 생성을 위해 난수 초기화
        LoadBitmaps();                                     // test.bmp, image.bmp 불러오기
        return 0;                                          // 메시지 처리 완료

    case WM_SIZE:                                          // 창 크기가 변경될 때
        GetClientRect(hWnd, &client);                      // 새 클라이언트 영역 얻기
        if (g_hasLens == TRUE) ClampRectToClient(&g_lens, client); // 돋보기가 화면 밖으로 나가지 않게 보정
        InvalidateRect(hWnd, NULL, FALSE);                 // 화면 다시 그리기
        return 0;                                          // 메시지 처리 완료

    case WM_PAINT:                                         // 화면을 그려야 할 때
        hDC = BeginPaint(hWnd, &ps);                       // 그리기 시작
        GetClientRect(hWnd, &client);                      // 현재 클라이언트 영역 얻기

        memDC = CreateCompatibleDC(hDC);                   // 화면 DC와 호환되는 메모리 DC 생성
        hBackBmp = CreateCompatibleBitmap(hDC, RectW(client), RectH(client)); // 화면 크기만큼 백버퍼 생성
        oldBackBmp = SelectObject(memDC, hBackBmp);        // 메모리 DC에 백버퍼 선택

        FillRect(memDC, &client, (HBRUSH)GetStockObject(WHITE_BRUSH)); // 백버퍼를 흰색으로 초기화
        DrawScene(memDC, client);                          // 모든 그림을 백버퍼에 그림

        BitBlt(hDC, 0, 0, RectW(client), RectH(client), memDC, 0, 0, SRCCOPY); // 백버퍼를 실제 화면으로 복사

        SelectObject(memDC, oldBackBmp);                   // 이전 객체 복구
        DeleteObject(hBackBmp);                            // 백버퍼 비트맵 삭제
        DeleteDC(memDC);                                   // 메모리 DC 삭제

        EndPaint(hWnd, &ps);                               // 그리기 종료
        return 0;                                          // 메시지 처리 완료

    case WM_LBUTTONDOWN:                                   // 마우스 왼쪽 버튼을 눌렀을 때
        GetClientRect(hWnd, &client);                      // 클라이언트 영역 얻기
        x = LOWORD(lParam);                                // 마우스 x 좌표 얻기
        y = HIWORD(lParam);                                // 마우스 y 좌표 얻기

        SetCapture(hWnd);                                  // 드래그 중 마우스 메시지를 계속 받도록 설정

        g_dragStart.x = x;                                 // 드래그 시작 x 저장
        g_dragStart.y = y;                                 // 드래그 시작 y 저장
        g_dragOldRect = g_lens;                            // 현재 돋보기 위치 저장

        g_dragMode = GetMouseWorkMode(x, y);               // 클릭 위치에 따라 작업 모드 결정

        if (g_dragMode == MODE_CREATE) {                   // 새 돋보기 만들기 모드면
            g_hasLens = TRUE;                              // 돋보기 존재 상태로 변경
            g_lens.left = x;                               // 시작 x를 왼쪽으로 설정
            g_lens.top = y;                                // 시작 y를 위쪽으로 설정
            g_lens.right = x;                              // 시작 x를 오른쪽으로 설정
            g_lens.bottom = y;                             // 시작 y를 아래쪽으로 설정
        }

        InvalidateRect(hWnd, NULL, FALSE);                 // 화면 다시 그리기
        return 0;                                          // 메시지 처리 완료

    case WM_MOUSEMOVE:                                     // 마우스가 움직일 때
        if (g_dragMode == MODE_NONE) return 0;             // 드래그 중이 아니면 종료

        GetClientRect(hWnd, &client);                      // 클라이언트 영역 얻기
        x = LOWORD(lParam);                                // 현재 마우스 x
        y = HIWORD(lParam);                                // 현재 마우스 y

        dx = x - g_dragStart.x;                            // 시작점 기준 x 이동량
        dy = y - g_dragStart.y;                            // 시작점 기준 y 이동량

        if (g_dragMode == MODE_CREATE) {                   // 새 돋보기 생성 중이면
            g_lens.left = g_dragStart.x;                   // 시작 x 저장
            g_lens.top = g_dragStart.y;                    // 시작 y 저장
            g_lens.right = x;                              // 현재 x를 끝점으로 저장
            g_lens.bottom = y;                             // 현재 y를 끝점으로 저장
            g_lens = NormalizeRectSimple(g_lens);          // 사각형 좌표 정상화
        }
        else if (g_dragMode == MODE_MOVE) {                // 돋보기 이동 중이면
            g_lens = g_dragOldRect;                        // 시작 당시 위치로 복구
            OffsetRect(&g_lens, dx, dy);                   // 이동량만큼 이동
            ClampRectToClient(&g_lens, client);            // 화면 밖으로 나가지 않게 보정
        }
        else if (g_dragMode == MODE_RESIZE) {              // 돋보기 크기 조절 중이면
            temp = g_dragOldRect;                          // 시작 당시 위치 복사

            if (g_resizeFlag & EDGE_LEFT) temp.left += dx; // 왼쪽 가장자리를 드래그한 경우
            if (g_resizeFlag & EDGE_RIGHT) temp.right += dx; // 오른쪽 가장자리를 드래그한 경우
            if (g_resizeFlag & EDGE_TOP) temp.top += dy;   // 위쪽 가장자리를 드래그한 경우
            if (g_resizeFlag & EDGE_BOTTOM) temp.bottom += dy; // 아래쪽 가장자리를 드래그한 경우

            temp = NormalizeRectSimple(temp);              // 좌표 정상화

            if (RectW(temp) < MIN_LENS_SIZE) temp.right = temp.left + MIN_LENS_SIZE; // 최소 가로 크기 보정
            if (RectH(temp) < MIN_LENS_SIZE) temp.bottom = temp.top + MIN_LENS_SIZE; // 최소 세로 크기 보정

            g_lens = temp;                                 // 조절된 크기 적용
            ClampRectToClient(&g_lens, client);            // 화면 밖으로 나가지 않게 보정
        }

        InvalidateRect(hWnd, NULL, FALSE);                 // 화면 다시 그리기
        return 0;                                          // 메시지 처리 완료

    case WM_LBUTTONUP:                                     // 마우스 왼쪽 버튼을 뗐을 때
        GetClientRect(hWnd, &client);                      // 클라이언트 영역 얻기
        ReleaseCapture();                                  // 마우스 캡처 해제

        if (g_hasLens == TRUE) {                           // 돋보기가 있으면
            g_lens = NormalizeRectSimple(g_lens);          // 사각형 좌표 정상화

            if (RectW(g_lens) < 10 || RectH(g_lens) < 10) { // 너무 작게 만들었으면
                x = LOWORD(lParam);                        // 마우스 x 얻기
                y = HIWORD(lParam);                        // 마우스 y 얻기
                MakeDefaultLensAt(x, y, client);           // 기본 크기 돋보기 생성
            }

            ClampRectToClient(&g_lens, client);            // 화면 밖 보정

            if (g_sizeAni == FALSE) {                      // 크기 애니메이션 중이 아니면
                UpdateNormalLensSize();                    // 현재 크기를 기본 크기로 저장
            }
        }

        g_dragMode = MODE_NONE;                            // 드래그 상태 해제
        InvalidateRect(hWnd, NULL, FALSE);                 // 다시 그리기
        return 0;                                          // 메시지 처리 완료

    case WM_LBUTTONDBLCLK:                                 // 마우스 왼쪽 더블클릭
        GetClientRect(hWnd, &client);                      // 클라이언트 영역 얻기
        x = LOWORD(lParam);                                // 더블클릭 x
        y = HIWORD(lParam);                                // 더블클릭 y

        ReleaseCapture();                                  // 혹시 남아있는 캡처 해제
        g_dragMode = MODE_NONE;                            // 드래그 상태 해제

        AddDoubleClickCopy(x, y, client);                  // 더블클릭 복사 그림 추가 또는 전체 삭제
        InvalidateRect(hWnd, NULL, FALSE);                 // 다시 그리기
        return 0;                                          // 메시지 처리 완료

    case WM_KEYDOWN:                                       // 키보드 키를 눌렀을 때
        GetClientRect(hWnd, &client);                      // 현재 클라이언트 영역 얻기

        switch (wParam) {                                  // 어떤 키인지 확인
        case '1':                                          // 1 키
            g_curImg = 0;                                  // 첫 번째 이미지 선택
            InvalidateRect(hWnd, NULL, FALSE);             // 다시 그리기
            break;                                         // 처리 종료

        case '2':                                          // 2 키
            g_curImg = 1;                                  // 두 번째 이미지 선택
            InvalidateRect(hWnd, NULL, FALSE);             // 다시 그리기
            break;                                         // 처리 종료

        case 'E':                                          // e 키
            if (g_hasLens == TRUE) {                       // 돋보기가 있으면
                g_lensScale += 0.2;                        // 조금씩 확대
                if (g_lensScale > 2.6) g_lensScale = 1.0;  // 특정 크기 이상이면 원래 크기
                InvalidateRect(hWnd, NULL, FALSE);         // 다시 그리기
            }
            break;                                         // 처리 종료

        case 'S':                                          // s 키
            if (g_hasLens == TRUE) {                       // 돋보기가 있으면
                g_lensScale -= 0.15;                       // 조금씩 축소
                if (g_lensScale < 0.45) g_lensScale = 1.0; // 특정 크기 이하이면 원래 크기
                InvalidateRect(hWnd, NULL, FALSE);         // 다시 그리기
            }
            break;                                         // 처리 종료

        case 'B':                                          // b 키
            g_lensScale = 1.0;                             // 돋보기 내부 그림 원래 크기
            InvalidateRect(hWnd, NULL, FALSE);             // 다시 그리기
            break;                                         // 처리 종료

        case 'C':                                          // c 키
            if (g_hasLens == TRUE) g_hasCopied = TRUE;     // 현재 돋보기 영역을 복사했다고 표시
            break;                                         // 처리 종료

        case 'P':                                          // p 키
            AddPasteCopy(client);                          // 랜덤 위치에 1/2 크기로 붙여넣기
            InvalidateRect(hWnd, NULL, FALSE);             // 다시 그리기
            break;                                         // 처리 종료

        case 'F':                                          // f 키
            if (g_hasLens == TRUE) {                       // 돋보기가 있으면
                g_fullPaste = !g_fullPaste;                // 전체 붙여넣기 켜기/끄기
                InvalidateRect(hWnd, NULL, FALSE);         // 다시 그리기
            }
            break;                                         // 처리 종료

        case 'H':                                          // h 키
            g_flipH = !g_flipH;                            // 붙여넣기 그림 좌우 반전 토글
            InvalidateRect(hWnd, NULL, FALSE);             // 다시 그리기
            break;                                         // 처리 종료

        case 'V':                                          // v 키
            g_flipV = !g_flipV;                            // 붙여넣기 그림 상하 반전 토글
            InvalidateRect(hWnd, NULL, FALSE);             // 다시 그리기
            break;                                         // 처리 종료

        case 'M':                                          // m 키
            if (g_hasLens == TRUE) {                       // 돋보기가 있으면
                g_bounceMove = !g_bounceMove;              // 튕기기 이동 켜기/끄기

                if (g_bounceMove == TRUE) SetTimer(hWnd, TIMER_BOUNCE, 30, NULL); // 이동 타이머 시작
                else KillTimer(hWnd, TIMER_BOUNCE);        // 이동 타이머 종료
            }
            break;                                         // 처리 종료

        case 'N':                                          // n 키
            if (g_hasLens == TRUE) {                       // 돋보기가 있으면
                if (g_sizeAni == FALSE) {                  // 애니메이션이 꺼져 있으면
                    UpdateNormalLensSize();                // 현재 크기를 원래 크기로 저장
                    g_sizeAni = TRUE;                      // 애니메이션 켜기
                    g_sizeDir = 1;                         // 처음에는 커지는 방향
                    SetTimer(hWnd, TIMER_SIZE, 40, NULL);  // 크기 애니메이션 타이머 시작
                }
                else {                                     // 애니메이션이 켜져 있으면
                    StopSizeAnimation(hWnd);               // 멈추고 원래 크기로 복구
                }
            }
            break;                                         // 처리 종료

        case 'I':                                          // i 키
            if (g_hasLens == TRUE) {                       // 돋보기가 있으면
                g_invertLens = !g_invertLens;              // 돋보기 내부 색상 반전 토글
                InvalidateRect(hWnd, NULL, FALSE);         // 다시 그리기
            }
            break;                                         // 처리 종료

        case 'R':                                          // r 키
            ResetAll(hWnd);                                // 전체 상태 리셋
            break;                                         // 처리 종료

        case 'Q':                                          // q 키
            DestroyWindow(hWnd);                           // 프로그램 종료
            break;                                         // 처리 종료

        case VK_LEFT:                                      // 왼쪽 방향키
            MoveLensByKey(hWnd, -10, 0);                   // 돋보기 왼쪽 이동
            break;                                         // 처리 종료

        case VK_RIGHT:                                     // 오른쪽 방향키
            MoveLensByKey(hWnd, 10, 0);                    // 돋보기 오른쪽 이동
            break;                                         // 처리 종료

        case VK_UP:                                        // 위쪽 방향키
            MoveLensByKey(hWnd, 0, -10);                   // 돋보기 위로 이동
            break;                                         // 처리 종료

        case VK_DOWN:                                      // 아래쪽 방향키
            MoveLensByKey(hWnd, 0, 10);                    // 돋보기 아래로 이동
            break;                                         // 처리 종료
        }

        return 0;                                          // 메시지 처리 완료

    case WM_TIMER:                                         // 타이머 메시지
        GetClientRect(hWnd, &client);                      // 클라이언트 영역 얻기

        if (wParam == TIMER_BOUNCE) {                      // 튕기기 이동 타이머이면
            if (g_hasLens == TRUE) {                       // 돋보기가 있으면
                OffsetRect(&g_lens, g_dx, g_dy);           // 현재 속도만큼 이동

                if (g_lens.left < 0) {                     // 왼쪽 벽에 닿으면
                    OffsetRect(&g_lens, -g_lens.left, 0);  // 화면 안으로 보정
                    g_dx = -g_dx;                          // x 방향 반전
                }

                if (g_lens.right > RectW(client)) {        // 오른쪽 벽에 닿으면
                    OffsetRect(&g_lens, RectW(client) - g_lens.right, 0); // 화면 안으로 보정
                    g_dx = -g_dx;                          // x 방향 반전
                }

                if (g_lens.top < 0) {                      // 위쪽 벽에 닿으면
                    OffsetRect(&g_lens, 0, -g_lens.top);   // 화면 안으로 보정
                    g_dy = -g_dy;                          // y 방향 반전
                }

                if (g_lens.bottom > RectH(client)) {       // 아래쪽 벽에 닿으면
                    OffsetRect(&g_lens, 0, RectH(client) - g_lens.bottom); // 화면 안으로 보정
                    g_dy = -g_dy;                          // y 방향 반전
                }

                InvalidateRect(hWnd, NULL, FALSE);         // 다시 그리기
            }
        }
        else if (wParam == TIMER_SIZE) {                   // 크기 애니메이션 타이머이면
            if (g_hasLens == TRUE) {                       // 돋보기가 있으면
                cx = (g_lens.left + g_lens.right) / 2;     // 현재 중심 x
                cy = (g_lens.top + g_lens.bottom) / 2;     // 현재 중심 y

                w = RectW(g_lens) + g_sizeDir * 8;         // 현재 가로 크기 변경
                h = RectH(g_lens) + g_sizeDir * 6;         // 현재 세로 크기 변경

                if (w > g_normalW * 2 || h > g_normalH * 2) { // 원래 크기의 2배보다 커지면
                    g_sizeDir = -1;                        // 작아지는 방향으로 변경
                }

                if (w < g_normalW / 2 || h < g_normalH / 2) { // 원래 크기의 절반보다 작아지면
                    g_sizeDir = 1;                         // 커지는 방향으로 변경
                }

                if (w < MIN_LENS_SIZE) w = MIN_LENS_SIZE;  // 최소 가로 크기 보정
                if (h < MIN_LENS_SIZE) h = MIN_LENS_SIZE;  // 최소 세로 크기 보정

                SetLensByCenter(cx, cy, w, h, client);     // 중심을 유지하면서 크기 변경
                InvalidateRect(hWnd, NULL, FALSE);         // 다시 그리기
            }
        }

        return 0;                                          // 메시지 처리 완료

    case WM_DESTROY:                                       // 윈도우가 파괴될 때
        KillTimer(hWnd, TIMER_BOUNCE);                     // 이동 타이머 종료
        KillTimer(hWnd, TIMER_SIZE);                       // 크기 애니메이션 타이머 종료
        DeleteBitmaps();                                   // 비트맵 리소스 삭제
        PostQuitMessage(0);                                // 프로그램 종료 메시지 발생
        return 0;                                          // 메시지 처리 완료
    }

    return DefWindowProc(hWnd, uMsg, wParam, lParam);      // 처리하지 않은 메시지는 기본 처리 함수로 넘김
}