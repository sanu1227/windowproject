#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <Windows.h>
#include <conio.h>

int onekey() {
    int key = _getch();
    if (key == 0 || key == 224) { // 확장키
        key = _getch() + 1000;   // 일반키와 구분
    }
    return key;
}

int main() {
    int arr[10][10], init_arr[10][10];
    int color_state[10][10] = { 0 }; // 파란색 토글
    int sum_state[10][10] = { 0 };   // 엔터 합산 상태
    int i, j, temp, r, chain;
    int rand_row, rand_columns;
    int original_value = 0;

    srand((unsigned int)time(NULL));
    rand_row = rand() % 10;
    rand_columns = rand() % 10;

    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);

    // 초기 배열 생성 및 섞기
    for (i = 0; i < 10; i++) {
        for (j = 0; j < 10; j++) arr[i][j] = j;
        for (j = 0; j < 10; j++) {
            r = rand() % 10;
            temp = arr[i][j];
            arr[i][j] = arr[i][r];
            arr[i][r] = temp;
        }
    }

    // 초기값 저장
    for (i = 0; i < 10; i++)
        for (j = 0; j < 10; j++)
            init_arr[i][j] = arr[i][j];

    // 첫 화면 출력
    system("cls");
    for (i = 0; i < 10; i++) {
        for (j = 0; j < 10; j++) {
            if (i == rand_row && j == rand_columns) {
                SetConsoleTextAttribute(hConsole, 12);
                printf("%d ", arr[i][j]);
                SetConsoleTextAttribute(hConsole, 7);
            }
            else {
                printf("%d ", arr[i][j]);
            }
        }
        printf("\n");
    }

    while (1) {
        int key = onekey();

        // r: 배열 재설정
        if (key == 'r') {
            for (i = 0; i < 10; i++) {
                for (j = 0; j < 10; j++) arr[i][j] = j;
                for (j = 0; j < 10; j++) {
                    r = rand() % 10;
                    temp = arr[i][j];
                    arr[i][j] = arr[i][r];
                    arr[i][r] = temp;
                }
            }
            for (i = 0; i < 10; i++)
                for (j = 0; j < 10; j++) {
                    color_state[i][j] = 0;
                    sum_state[i][j] = 0;
                    init_arr[i][j] = arr[i][j]; // 초기값도 갱신
                }
            rand_row = rand() % 10;
            rand_columns = rand() % 10;
        }

        // q: 종료
        if (key == 'q') break;

        // 숫자 토글 (파란색)
        if (key >= '0' && key <= '9') {
            int num = key - '0';
            for (i = 0; i < 10; i++)
                for (j = 0; j < 10; j++)
                    if (arr[i][j] == num && !sum_state[i][j])
                        color_state[i][j] = !color_state[i][j]; // 직접 토글
        }

        // 엔터: 현재 위치 합산 / 초기값 복원
        if (key == 13) {
            if (!sum_state[rand_row][rand_columns]) {
                int sum = 0;
                for (i = 0; i < 10; i++)
                    for (j = 0; j < 10; j++)
                        if (color_state[i][j]) sum += arr[i][j];

                original_value = arr[rand_row][rand_columns];
                arr[rand_row][rand_columns] = original_value + sum;
                sum_state[rand_row][rand_columns] = 1;
            }
            else {
                arr[rand_row][rand_columns] = init_arr[rand_row][rand_columns];
                sum_state[rand_row][rand_columns] = 0;
            }
        }

        // w/a/s/d 원형 이동
        if (key == 'w') rand_row = (rand_row + 9) % 10;
        if (key == 's') rand_row = (rand_row + 1) % 10;
        if (key == 'a') rand_columns = (rand_columns + 9) % 10;
        if (key == 'd') rand_columns = (rand_columns + 1) % 10;

        // 화살표 이동 원형 처리
        if (key == 1000 + 72) { // ↑
            for (j = 0; j < 10; j++) {
                chain = arr[rand_row][j];
                arr[rand_row][j] = arr[(rand_row + 9) % 10][j];
                arr[(rand_row + 9) % 10][j] = chain;

                chain = color_state[rand_row][j];
                color_state[rand_row][j] = color_state[(rand_row + 9) % 10][j];
                color_state[(rand_row + 9) % 10][j] = chain;

                chain = sum_state[rand_row][j];
                sum_state[rand_row][j] = sum_state[(rand_row + 9) % 10][j];
                sum_state[(rand_row + 9) % 10][j] = chain;
            }
            rand_row = (rand_row + 9) % 10;
        }
        if (key == 1000 + 80) { // ↓
            for (j = 0; j < 10; j++) {
                chain = arr[rand_row][j];
                arr[rand_row][j] = arr[(rand_row + 1) % 10][j];
                arr[(rand_row + 1) % 10][j] = chain;

                chain = color_state[rand_row][j];
                color_state[rand_row][j] = color_state[(rand_row + 1) % 10][j];
                color_state[(rand_row + 1) % 10][j] = chain;

                chain = sum_state[rand_row][j];
                sum_state[rand_row][j] = sum_state[(rand_row + 1) % 10][j];
                sum_state[(rand_row + 1) % 10][j] = chain;
            }
            rand_row = (rand_row + 1) % 10;
        }
        if (key == 1000 + 75) { // ←
            for (i = 0; i < 10; i++) {
                chain = arr[i][rand_columns];
                arr[i][rand_columns] = arr[i][(rand_columns + 9) % 10];
                arr[i][(rand_columns + 9) % 10] = chain;

                chain = color_state[i][rand_columns];
                color_state[i][rand_columns] = color_state[i][(rand_columns + 9) % 10];
                color_state[i][(rand_columns + 9) % 10] = chain;

                chain = sum_state[i][rand_columns];
                sum_state[i][rand_columns] = sum_state[i][(rand_columns + 9) % 10];
                sum_state[i][(rand_columns + 9) % 10] = chain;
            }
            rand_columns = (rand_columns + 9) % 10;
        }
        if (key == 1000 + 77) { // →
            for (i = 0; i < 10; i++) {
                chain = arr[i][rand_columns];
                arr[i][rand_columns] = arr[i][(rand_columns + 1) % 10];
                arr[i][(rand_columns + 1) % 10] = chain;

                chain = color_state[i][rand_columns];
                color_state[i][rand_columns] = color_state[i][(rand_columns + 1) % 10];
                color_state[i][(rand_columns + 1) % 10] = chain;

                chain = sum_state[i][rand_columns];
                sum_state[i][rand_columns] = sum_state[i][(rand_columns + 1) % 10];
                sum_state[i][(rand_columns + 1) % 10] = chain;
            }
            rand_columns = (rand_columns + 1) % 10;
        }

        // 화면 출력
        system("cls");
        for (i = 0; i < 10; i++) {
            for (j = 0; j < 10; j++) {
                if (i == rand_row && j == rand_columns) {
                    SetConsoleTextAttribute(hConsole, 12);
                    printf("%d ", arr[i][j]);
                    SetConsoleTextAttribute(hConsole, 7);
                }
                else if (color_state[i][j]) {
                    SetConsoleTextAttribute(hConsole, 9);
                    printf("%d ", arr[i][j]);
                    SetConsoleTextAttribute(hConsole, 7);
                }
                else {
                    printf("%d ", arr[i][j]);
                }
            }
            printf("\n");
        }
    }

    return 0;
}