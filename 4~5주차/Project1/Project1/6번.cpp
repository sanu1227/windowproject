#include <windows.h>
#include <tchar.h>
#include <time.h>
#include <stdlib.h>

HINSTANCE g_hInst;
LPCTSTR lpszClass = L"My Window Class";
LPCTSTR lpszWindowName = L"Window Programming Lab";

LRESULT CALLBACK WndProc(HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam);

typedef struct {
    int x;              // 시작 x좌표
    int y;              // 시작 y좌표
    int n;              // 출력할 숫자
    int count;          // 가로/세로 출력 개수
    COLORREF textColor; // 문자색
    COLORREF backColor; // 배경색
} OUTPUTDATA;

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpszCmdParam, int nCmdShow)
{
    srand((unsigned)time(NULL));

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
        lpszClass, lpszWindowName, WS_OVERLAPPEDWINDOW,
        0, 0, 800, 600,
        NULL, NULL, hInstance, NULL
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

    static int inputCount;              // 입력 문자열 길이
    static TCHAR inputStr[100];         // 키보드 입력 저장
    static SIZE size;                   // 문자열 크기 저장용

    static OUTPUTDATA data[10];         // 최대 10개의 출력물 저장
    static int savedCount;              // 현재 저장된 출력물 개수

    static int nums[4];                 // 입력한 정수 4개 저장용
    static int numCount;                // 실제 읽은 정수 개수

    static int showAll;                 // 0: 최근 것만 출력, 1: 전체 출력

    int x = 0;
    int y = 0;

    switch (uMsg) {

    case WM_CREATE:
        CreateCaret(hWnd, NULL, 5, 15);
        ShowCaret(hWnd);

        inputCount = 0;
        inputStr[0] = '\0';

        savedCount = 0;
        numCount = 0;
        showAll = 0;

        srand((unsigned int)time(NULL));
        return 0;

    case WM_CHAR:

        // q : 프로그램 종료
        if (wParam == 'q' || wParam == 'Q') {
            DestroyWindow(hWnd);
            return 0;
        }

        // r : 전체 리셋
        else if (wParam == 'r' || wParam == 'R') {
            savedCount = 0;
            inputCount = 0;
            inputStr[0] = '\0';
            showAll = 0;

            InvalidateRect(hWnd, NULL, TRUE);
            return 0;
        }

        // a : 지금까지 저장된 모든 출력물 보기
        else if (wParam == 'a' || wParam == 'A') {
            showAll = 1;
            InvalidateRect(hWnd, NULL, TRUE);
            return 0;
        }

        // 엔터 : 새 정수 4개 입력받아 새 출력물 생성
        else if (wParam == '\r') {
            int i = 0;
            int current = 0;
            int hasNumber = 0;

            nums[0] = nums[1] = nums[2] = nums[3] = 0;
            numCount = 0;

            // 입력 문자열에서 정수 4개 분리
            while (inputStr[i] != '\0') {

                if (inputStr[i] >= '0' && inputStr[i] <= '9') {
                    current = current * 10 + (inputStr[i] - '0');
                    hasNumber = 1;
                }
                else if (inputStr[i] == ' ') {
                    if (hasNumber == 1) {
                        if (numCount < 4) {
                            nums[numCount] = current;
                            numCount++;
                        }
                        current = 0;
                        hasNumber = 0;
                    }
                }

                i++;
            }

            // 마지막 숫자 저장
            if (hasNumber == 1 && numCount < 4) {
                nums[numCount] = current;
                numCount++;
            }

            // 숫자 4개를 정확히 입력했고, 저장 공간이 남아 있으면 저장
            if (numCount == 4 && savedCount < 10) {
                int inputX = nums[0];
                int inputY = nums[1];
                int inputN = nums[2];
                int inputRepeat = nums[3];

                // 범위 검사
                if (inputX >= 0 && inputX <= 600 &&
                    inputY >= 0 && inputY <= 400 &&
                    inputN >= 0 && inputN <= 200 &&
                    inputRepeat >= 5 && inputRepeat <= 20) {

                    data[savedCount].x = inputX;
                    data[savedCount].y = inputY;
                    data[savedCount].n = inputN;
                    data[savedCount].count = inputRepeat;
                    data[savedCount].textColor = RGB(rand() % 256, rand() % 256, rand() % 256);
                    data[savedCount].backColor = RGB(rand() % 256, rand() % 256, rand() % 256);

                    savedCount++;

                    // 엔터를 누르면 이전 출력은 숨기고 가장 최근 것만 보이게
                    showAll = 0;
                }
            }

            // 입력창 비우기
            inputCount = 0;
            inputStr[0] = '\0';

            InvalidateRect(hWnd, NULL, TRUE);
            return 0;
        }

        // 백스페이스
        else if (wParam == '\b') {
            if (inputCount > 0) {
                inputCount--;
                inputStr[inputCount] = '\0';
            }

            InvalidateRect(hWnd, NULL, TRUE);
            return 0;
        }

        // 숫자와 공백만 입력 문자열에 저장
        else if ((wParam >= '0' && wParam <= '9') || wParam == ' ') {
            if (inputCount < 99) {
                inputStr[inputCount++] = (TCHAR)wParam;
                inputStr[inputCount] = '\0';
            }

            InvalidateRect(hWnd, NULL, TRUE);
            return 0;
        }

        return 0;

    case WM_PAINT:
    {
        int i, j, k;
        TCHAR numStr[20];

        hDC = BeginPaint(hWnd, &ps);

        // 현재 입력 중인 문자열 출력
        GetTextExtentPoint32(hDC, inputStr, lstrlen(inputStr), &size);
        TextOut(hDC, x, y, inputStr, lstrlen(inputStr));
        SetCaretPos(x + size.cx, y);

        // a를 누른 경우: 저장된 모든 출력물 출력
        if (showAll == 1) {
            for (k = 0; k < savedCount; k++) {
                SetTextColor(hDC, data[k].textColor);
                SetBkColor(hDC, data[k].backColor);

                wsprintf(numStr, L"%d", data[k].n);
                GetTextExtentPoint32(hDC, numStr, lstrlen(numStr), &size);

                for (i = 0; i < data[k].count; i++) {
                    for (j = 0; j < data[k].count; j++) {
                        TextOut(
                            hDC,
                            data[k].x + j * size.cx,
                            data[k].y + i * size.cy,
                            numStr,
                            lstrlen(numStr)
                        );
                    }
                }
            }
        }

        // 기본: 가장 최근 출력물 1개만 출력
        else {
            if (savedCount > 0) {
                k = savedCount - 1;

                SetTextColor(hDC, data[k].textColor);
                SetBkColor(hDC, data[k].backColor);

                wsprintf(numStr, L"%d", data[k].n);
                GetTextExtentPoint32(hDC, numStr, lstrlen(numStr), &size);

                for (i = 0; i < data[k].count; i++) {
                    for (j = 0; j < data[k].count; j++) {
                        TextOut(
                            hDC,
                            data[k].x + j * size.cx,
                            data[k].y + i * size.cy,
                            numStr,
                            lstrlen(numStr)
                        );
                    }
                }
            }
        }

        EndPaint(hWnd, &ps);
        return 0;
    }

    case WM_DESTROY:
        HideCaret(hWnd);
        DestroyCaret();
        PostQuitMessage(0);
        return 0;
    }

    return DefWindowProc(hWnd, uMsg, wParam, lParam);
}
