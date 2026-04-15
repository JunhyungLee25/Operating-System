#include <pthread.h>
#include <stdio.h>
#define NUM_THREADS 5

int main(int argc, char *argv[])
{
    int i, scope;
    pthread_t tid[NUM_THREADS]; // 생성할 스레드들의 ID를 저장할 배열
    pthread_attr_t attr;        // 스레드의 속성(Attribute)을 설정할 변수

    /* 1. 스레드 속성 초기화 */
    /* 기본 속성값으로 attr 구조체를 초기화한다. */
    pthread_attr_init(&attr);

    /* 2. 현재 스케줄링 범위(Scope) 확인 */
    /* 현재 시스템에 설정된 기본 스케줄링 범위가 무엇인지 조회하여 scope 변수에 저장한다. */
    if (pthread_attr_getscope(&attr, &scope) != 0)
        fprintf(stderr, "Unable to get scheduling scope\n");
    else
    {
        // 조회된 범위가 PCS(프로세스 내부 경쟁)인지 SCS(시스템 전체 경쟁)인지 출력한다.
        if (scope == PTHREAD_SCOPE_PROCESS)
            printf("PTHREAD_SCOPE_PROCESS");
        else if (scope == PTHREAD_SCOPE_SYSTEM)
            printf("PTHREAD_SCOPE_SYSTEM");
        else
            fprintf(stderr, "Illegal scope value.\n");
    }

    /* 3. 스케줄링 범위를 SCS로 강제 설정 */
    /* 생성될 스레드들이 커널 스레드에 직접 1:1로 매핑되어 시스템 전체에서 경쟁하도록 설정한다. */
    pthread_attr_setscope(&attr, PTHREAD_SCOPE_SYSTEM); 

    /* 4. 스레드 생성 */
    /* 설정된 attr 속성을 바탕으로 5개의 스레드를 생성하고, 각각 'runner' 함수를 실행시킨다. */
    for (i = 0; i < NUM_THREADS; i++)
        pthread_create(&tid[i], &attr, runner, NULL);

    /* 5. 스레드 종료 대기 (동기화) */
    /* 메인 스레드(프로세스)가 먼저 종료되는 것을 막기 위해, 생성된 모든 자식 스레드의 작업이 끝날 때까지 대기(join)한다. */
    for (i = 0; i < NUM_THREADS; i++)
        pthread_join(tid[i], NULL);
        
    return 0;
}

/* 각각의 생성된 스레드가 실질적으로 수행할 작업(함수)이다. */ 
void *runner(void *param)
{
    /* do some work ... (실제 작업 수행 영역) */ 
    
    /* 스레드 자신의 실행을 안전하게 종료한다. */
    pthread_exit(0);
}