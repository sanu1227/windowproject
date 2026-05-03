#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>

int main() {
	int r, t;
	printf("단을 입력 하세요:\n");
	scanf("%d", &r);
	

	for (int i = 2; i < r + 2; i++) {
		for (int k = 1; k <= 9; k++) {
			printf("%d x %d = %d\n", i, k, i * k);
		}
	}

	printf("back\n");

	for (int n = r+2; n > 2; n--) {
		for (int m = 1; m <= 9; m++) {
			printf("%d x %d = %d\n", n, m, n * m);
		}
	}

}