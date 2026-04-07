#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

/* 스레드들이 공유하는 데이터 (shared data) */
/* Java 예제의 Sum 클래스 역할을 하며, 전역 변수로 선언되어 모든 스레드가 접근 가능하다. */
int sum; 

/* 스레드가 실행할 함수 선언 */
void *runner(void *param); 

int main(int argc, char *argv[]) {
    pthread_t tid;       /* 스레드 식별자 (thread identifier) */
    pthread_attr_t attr; /* 스레드 속성 (thread attributes) */

    /* 명령행 인자(argument) 확인 */
    if (argc != 2) {
        fprintf(stderr, "usage: a.out <integer value>\n");
        return -1;
    }

    if (atoi(argv[1]) < 0) {
        fprintf(stderr, "%d must be >= 0\n", atoi(argv[1]));
        return -1;
    }

    /* 스레드 속성을 기본값으로 초기화 */
    pthread_attr_init(&attr);

    /* 새로운 스레드 생성 (Java의 thrd.start()와 대응) */
    /* runner 함수를 새로운 실행 흐름으로 시작하며, argv[1]을 인자로 전달한다. */
    pthread_create(&tid, &attr, runner, argv[1]);

    /* 생성된 스레드가 종료될 때까지 대기 (Java의 thrd.join()과 대응) */
    /* 동기화(synchronization)를 위해 사용하며, 계산이 끝난 후 결과를 출력하도록 보장한다. */
    pthread_join(tid, NULL);

    printf("sum = %d\n", sum);

    return 0;
}

/* 스레드가 실제로 제어권을 갖고 실행하는 함수 */
void *runner(void *param) {
    int i, upper = atoi(param);
    sum = 0;

    /* 1부터 입력받은 값(upper)까지 합산 수행 */
    for (i = 1; i <= upper; i++) {
        sum += i;
    }

    /* 스레드 종료 및 제어권 반환 */
    pthread_exit(0);
}