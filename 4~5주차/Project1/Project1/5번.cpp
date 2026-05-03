#include <windows.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

/*
    프로그램 인스턴스 핸들
*/
HINSTANCE g_hInst;

/*
    문자열은 포인터 대신 배열로 선언
*/
char lpszClass[] = "My Window Class";
char lpszWindowName[] = "Window Programming Lab";

/*
    메시지 처리 함수
*/
LRESULT CALLBACK WndProc(HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam);

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpszCmdParam, int nCmdShow)
{
    HWND hWnd;
    MSG Message;
    WNDCLASSEXA WndClass;

    srand((unsigned)time(NULL));
    g_hInst = hInstance;

    WndClass.cbSize = sizeof(WNDCLASSEXA);
    WndClass.style = CS_HREDRAW | CS_VREDRAW;
    WndClass.lpfnWndProc = WndProc;
    WndClass.cbClsExtra = 0;
    WndClass.cbWndExtra = 0;
    WndClass.hInstance = hInstance;
    WndClass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    WndClass.hCursor = LoadCursor(NULL, IDC_ARROW);
    WndClass.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
    WndClass.lpszMenuName = NULL;
    WndClass.lpszClassName = lpszClass;
    WndClass.hIconSm = LoadIcon(NULL, IDI_APPLICATION);

    RegisterClassExA(&WndClass);

    hWnd = CreateWindowA(
        lpszClass,
        lpszWindowName,
        WS_OVERLAPPEDWINDOW,
        0, 0, 800, 600,
        NULL, NULL, hInstance, NULL
    );

    ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd);

    while (GetMessage(&Message, NULL, 0, 0)) {
        TranslateMessage(&Message);
        DispatchMessage(&Message);
    }

    return (int)Message.wParam;
}

/*
    핵심 로직
*/
LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    PAINTSTRUCT ps;
    HDC hDC;
    SIZE size;
    int i;

#define MAX_LINES 10
#define MAX_COLS 30

    /*
        문자열 저장 (포인터 없이 배열만 사용)
    */
    static char lines[MAX_LINES][MAX_COLS + 1];

    static int lineCount = 1;
    static int curLine = 0;
    static int curCol = 0;
    static int allFilled = 0;
    static int lineHeight = 20;

    switch (uMsg) {

    case WM_CREATE:
        CreateCaret(hWnd, NULL, 5, 15);
        ShowCaret(hWnd);

        for (i = 0; i < MAX_LINES; i++) {
            lines[i][0] = '\0';
        }

        break;

    case WM_CHAR:

        if (wParam == VK_ESCAPE) {
            DestroyWindow(hWnd);
            break;
        }

        /*
            백스페이스
        */
        if (wParam == VK_BACK) {

            if (curCol > 0) {
                curCol--;
                lines[curLine][curCol] = '\0';
            }
            else {
                if (allFilled || curLine > 0) {

                    if (curLine == 0)
                        curLine = MAX_LINES - 1;
                    else
                        curLine--;

                    curCol = (int)strlen(lines[curLine]);
                }
            }

            InvalidateRect(hWnd, NULL, TRUE);
            break;
        }

        /*
            엔터
        */
        if (wParam == VK_RETURN) {

            if (curLine < MAX_LINES - 1) {
                curLine++;

                if (curLine >= lineCount)
                    lineCount = curLine + 1;

                curCol = (int)strlen(lines[curLine]);
            }
            else {
                /*
                    마지막 줄이면 0번으로 돌아가서 덮어쓰기
                */
                curLine = 0;
                curCol = 0;
                lines[curLine][0] = '\0';
                allFilled = 1;
                lineCount = MAX_LINES;
            }

            InvalidateRect(hWnd, NULL, TRUE);
            break;
        }

        /*
            일반 문자
        */
        if (wParam >= 32 && wParam <= 126) {

            if (curCol < MAX_COLS) {
                lines[curLine][curCol] = (char)wParam;
                curCol++;
                lines[curLine][curCol] = '\0';
            }

            /*
                자동 줄바꿈
            */
            if (curCol >= MAX_COLS) {

                if (curLine < MAX_LINES - 1) {
                    curLine++;

                    if (curLine >= lineCount)
                        lineCount = curLine + 1;

                    curCol = (int)strlen(lines[curLine]);
                }
                else {
                    curLine = 0;
                    curCol = 0;
                    lines[curLine][0] = '\0';
                    allFilled = 1;
                    lineCount = MAX_LINES;
                }
            }

            InvalidateRect(hWnd, NULL, TRUE);
        }

        break;

    case WM_PAINT:
    {
        TEXTMETRICA tm;

        hDC = BeginPaint(hWnd, &ps);

        GetTextMetricsA(hDC, &tm);
        lineHeight = tm.tmHeight + 4;

        for (i = 0; i < lineCount; i++) {
            TextOutA(hDC, 0, i * lineHeight, lines[i], (int)strlen(lines[i]));
        }

        GetTextExtentPoint32A(
            hDC,
            lines[curLine],
            (int)strlen(lines[curLine]),
            &size
        );

        SetCaretPos(size.cx, curLine * lineHeight);

        EndPaint(hWnd, &ps);
        break;
    }

    case WM_DESTROY:
        HideCaret(hWnd);
        DestroyCaret();
        PostQuitMessage(0);
        return 0;
    }

    return DefWindowProc(hWnd, uMsg, wParam, lParam);
}