#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define SIZE 20
#define ITEM_COUNT 12   // 아이템 개수 (10개 이상으로 자유롭게 변경 가능)

// 칸 종류
#define EMPTY 0
#define WALL 1
#define ITEM 2

// 돌 정보 저장용 구조체
typedef struct {
    int x;
    int y;
    int score;
    char shape;
} Stone;

// 함수 선언
void initBoard(int board[SIZE][SIZE], Stone* s1, Stone* s2, int itemCount);
void printBoard(int board[SIZE][SIZE], Stone s1, Stone s2, int remainItems, int turn);
int canPlaceRect(int board[SIZE][SIZE], int h, int w, int x, int y);
void placeRect(int board[SIZE][SIZE], int h, int w);
void placeItems(int board[SIZE][SIZE], int count);
void placeStone(int board[SIZE][SIZE], Stone* s1, Stone* s2);
int moveStone(int board[SIZE][SIZE], Stone* me, Stone other, char cmd, int* remainItems);
void printResult(Stone s1, Stone s2);

int main() {
    int board[SIZE][SIZE];
    Stone stone1, stone2;
    int remainItems = ITEM_COUNT;
    int turn = 1;   // 1이면 stone1 차례, 2이면 stone2 차례
    char cmd;

    srand((unsigned int)time(NULL));

    initBoard(board, &stone1, &stone2, ITEM_COUNT);

    while (1) {
        printBoard(board, stone1, stone2, remainItems, turn);

        if (remainItems == 0) {
            printf("\n모든 아이템이 사라졌습니다.\n");
            printResult(stone1, stone2);
            break;
        }

        if (turn == 1) {
            printf("돌 1(#) 차례 - 입력(w/a/s/d, r, q): ");
        }
        else {
            printf("돌 2(@) 차례 - 입력(i/j/k/l, r, q): ");
        }

        scanf(" %c", &cmd);

        if (cmd == 'q') {
            printf("\n게임을 종료합니다.\n");
            printResult(stone1, stone2);
            break;
        }

        if (cmd == 'r') {
            printf("\n게임을 리셋합니다.\n");
            remainItems = ITEM_COUNT;
            turn = 1;
            initBoard(board, &stone1, &stone2, ITEM_COUNT);
            continue;
        }

        if (turn == 1) {
            if (cmd == 'w' || cmd == 'a' || cmd == 's' || cmd == 'd') {
                moveStone(board, &stone1, stone2, cmd, &remainItems);
                turn = 2;
            }
            else {
                printf("잘못된 입력입니다. 돌 1은 w/a/s/d만 사용할 수 있습니다.\n");
            }
        }
        else {
            if (cmd == 'i' || cmd == 'j' || cmd == 'k' || cmd == 'l') {
                moveStone(board, &stone2, stone1, cmd, &remainItems);
                turn = 1;
            }
            else {
                printf("잘못된 입력입니다. 돌 2는 i/j/k/l만 사용할 수 있습니다.\n");
            }
        }
    }

    return 0;
}

// 보드 초기화
void initBoard(int board[SIZE][SIZE], Stone* s1, Stone* s2, int itemCount) {
    int i, j;

    // 전체를 빈칸으로 초기화
    for (i = 0; i < SIZE; i++) {
        for (j = 0; j < SIZE; j++) {
            board[i][j] = EMPTY;
        }
    }

    // 돌 정보 초기화
    s1->score = 0;
    s2->score = 0;
    s1->shape = '#';
    s2->shape = '@';

    // 장애물 3개 배치
    placeRect(board, 3, 4);
    placeRect(board, 5, 2);
    placeRect(board, 4, 4);

    // 아이템 배치
    placeItems(board, itemCount);

    // 돌 배치
    placeStone(board, s1, s2);
}

// 보드 출력
void printBoard(int board[SIZE][SIZE], Stone s1, Stone s2, int remainItems, int turn) {
    int i, j;

    printf("\n");
    printf("============================================================\n");
    printf("돌1(#) 점수: %d   돌2(@) 점수: %d   남은 아이템: %d\n", s1.score, s2.score, remainItems);
    if (turn == 1) {
        printf("현재 차례: 돌 1 (#)\n");
    }
    else {
        printf("현재 차례: 돌 2 (@)\n");
    }
    printf("기본칸: .   장애물: X   아이템: *\n");
    printf("============================================================\n");

    for (i = 0; i < SIZE; i++) {
        for (j = 0; j < SIZE; j++) {
            if (s1.x == i && s1.y == j) {
                printf("%c ", s1.shape);
            }
            else if (s2.x == i && s2.y == j) {
                printf("%c ", s2.shape);
            }
            else if (board[i][j] == EMPTY) {
                printf(". ");
            }
            else if (board[i][j] == WALL) {
                printf("X ");
            }
            else if (board[i][j] == ITEM) {
                printf("* ");
            }
        }
        printf("\n");
    }
    printf("\n");
}

// 직사각형 장애물을 놓을 수 있는지 검사
int canPlaceRect(int board[SIZE][SIZE], int h, int w, int x, int y) {
    int i, j;

    if (x < 0 || y < 0 || x + h > SIZE || y + w > SIZE) {
        return 0;
    }

    for (i = x; i < x + h; i++) {
        for (j = y; j < y + w; j++) {
            if (board[i][j] != EMPTY) {
                return 0;
            }
        }
    }

    return 1;
}

// 직사각형 장애물 배치
void placeRect(int board[SIZE][SIZE], int h, int w) {
    int x, y;
    int i, j;

    while (1) {
        x = rand() % SIZE;
        y = rand() % SIZE;

        if (canPlaceRect(board, h, w, x, y)) {
            for (i = x; i < x + h; i++) {
                for (j = y; j < y + w; j++) {
                    board[i][j] = WALL;
                }
            }
            break;
        }
    }
}

// 아이템 배치
void placeItems(int board[SIZE][SIZE], int count) {
    int x, y;
    int placed = 0;

    while (placed < count) {
        x = rand() % SIZE;
        y = rand() % SIZE;

        if (board[x][y] == EMPTY) {
            board[x][y] = ITEM;
            placed++;
        }
    }
}

// 돌 2개 배치
void placeStone(int board[SIZE][SIZE], Stone* s1, Stone* s2) {
    while (1) {
        s1->x = rand() % SIZE;
        s1->y = rand() % SIZE;
        if (board[s1->x][s1->y] == EMPTY) {
            break;
        }
    }

    while (1) {
        s2->x = rand() % SIZE;
        s2->y = rand() % SIZE;
        if (board[s2->x][s2->y] == EMPTY && !(s2->x == s1->x && s2->y == s1->y)) {
            break;
        }
    }
}

// 돌 이동
int moveStone(int board[SIZE][SIZE], Stone* me, Stone other, char cmd, int* remainItems) {
    int nx = me->x;
    int ny = me->y;

    // 돌1: w a s d
    if (cmd == 'w') nx--;
    else if (cmd == 's') nx++;
    else if (cmd == 'a') ny--;
    else if (cmd == 'd') ny++;

    // 돌2: i j k l
    else if (cmd == 'i') nx--;
    else if (cmd == 'k') nx++;
    else if (cmd == 'j') ny--;
    else if (cmd == 'l') ny++;

    // 가장자리 넘으면 반대편으로
    if (nx < 0) nx = SIZE - 1;
    if (nx >= SIZE) nx = 0;
    if (ny < 0) ny = SIZE - 1;
    if (ny >= SIZE) ny = 0;

    // 다른 돌이 있는 칸으로는 이동 불가
    if (nx == other.x && ny == other.y) {
        printf("다른 돌이 있는 칸으로는 이동할 수 없습니다.\n");
        return 0;
    }

    // 장애물 통과 불가
    if (board[nx][ny] == WALL) {
        printf("장애물이 있어서 이동할 수 없습니다.\n");
        return 0;
    }

    // 아이템 먹기
    if (board[nx][ny] == ITEM) {
        me->score++;
        (*remainItems)--;
        board[nx][ny] = WALL;   // 아이템 먹은 자리는 장애물로 바뀜
        printf("아이템을 먹었습니다! 현재 점수: %d\n", me->score);
    }

    me->x = nx;
    me->y = ny;

    return 1;
}

// 최종 결과 출력
void printResult(Stone s1, Stone s2) {
    printf("====================================\n");
    printf("돌 1(#)이 먹은 아이템 수: %d\n", s1.score);
    printf("돌 2(@)이 먹은 아이템 수: %d\n", s2.score);

    if (s1.score > s2.score) {
        printf("승리: 돌 1(#)\n");
    }
    else if (s2.score > s1.score) {
        printf("승리: 돌 2(@)\n");
    }
    else {
        printf("무승부\n");
    }
    printf("====================================\n");
}