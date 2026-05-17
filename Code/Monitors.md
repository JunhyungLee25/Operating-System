# Monitors Code Notes

## 개요

이 파일은 ch6 Synchronization의 monitor 관련 코드 묶음이다. `Theory/06_Synchronization.md`에서는 monitor의 개념과 시험 포인트만 짧게 다루고, 여기서는 condition variable, Dining-Philosophers monitor 해법, semaphore 기반 monitor 구현을 줄 단위로 정리한다.

## 1. Monitor 기본 구조

```c
monitor monitor-name
{
    // monitor 내부에서만 접근 가능한 공유 변수 선언부
    // 외부 프로세스는 이 변수들을 직접 만지지 못하고 procedure를 통해서만 접근한다.
    // 이것이 monitor를 abstract data type처럼 보이게 하는 핵심이다.
    // shared variable declarations

    procedure P1 (...) {
        // monitor procedure의 본문이다.
        // 한 프로세스가 이 procedure를 실행 중이면 다른 프로세스는 monitor 안에서 active할 수 없다.
        ...
    }

    procedure Pn (...) {
        // 여러 procedure가 있어도 monitor 전체의 mutual exclusion 규칙은 동일하다.
        ...
    }

    Initialization code (...) {
        // monitor 내부 공유 변수의 초기값을 설정한다.
        ...
    }
}
```

## 동작 방식 / 원리

- monitor 내부 변수는 monitor procedure만 접근하므로 공유 데이터 접근 위치가 한곳으로 모인다.
- monitor는 내부적으로 mutual exclusion을 제공하므로 procedure 실행 중인 프로세스가 하나로 제한된다.
- 그러나 "조건이 만족될 때까지 기다리기" 같은 동기화는 condition variable이 필요하다.

## 시험 출제 포인트

- monitor는 semaphore보다 고수준 추상화이다.
- monitor 안에서는 한 번에 하나의 프로세스만 active할 수 있다.
- 내부 변수는 외부 코드가 직접 접근하지 못하고 monitor procedure로만 접근한다.

## 2. Dining Philosophers Monitor

```c
monitor DiningPhilosophers
{
    // 각 철학자의 상태를 표현한다.
    // THINKING: 생각 중, HUNGRY: 먹고 싶어서 대기 중, EATING: 식사 중
    enum { THINKING, HUNGRY, EATING } state[5];

    // 각 철학자마다 하나씩 condition variable을 둔다.
    // self[i]는 철학자 i가 자기 차례를 기다리는 큐 역할을 한다.
    condition self[5];

    void pickup(int i) {
        // 철학자 i가 젓가락을 들고 싶다는 의사를 표시한다.
        state[i] = HUNGRY;

        // 양옆 철학자가 먹고 있지 않으면 바로 EATING으로 바꿀 수 있는지 검사한다.
        test(i);

        // test(i) 후에도 EATING이 아니라면 조건이 만족되지 않은 것이다.
        // 자기 condition variable에서 잠들고, signal을 받을 때까지 기다린다.
        if (state[i] != EATING)
            self[i].wait();
    }

    void putdown(int i) {
        // 식사를 끝낸 철학자 i는 다시 생각 상태가 된다.
        state[i] = THINKING;

        // 왼쪽 이웃이 이제 먹을 수 있는지 검사한다.
        // (i + 4) % 5는 원형 배열에서 i의 왼쪽 이웃이다.
        test((i + 4) % 5);

        // 오른쪽 이웃이 이제 먹을 수 있는지 검사한다.
        // (i + 1) % 5는 원형 배열에서 i의 오른쪽 이웃이다.
        test((i + 1) % 5);
    }

    void test(int i) {
        // 왼쪽 이웃이 먹고 있지 않고,
        // 철학자 i 자신은 배고픈 상태이며,
        // 오른쪽 이웃도 먹고 있지 않으면 i는 식사할 수 있다.
        if ((state[(i + 4) % 5] != EATING) &&
            (state[i] == HUNGRY) &&
            (state[(i + 1) % 5] != EATING)) {

            // 조건을 만족했으므로 i를 식사 상태로 바꾼다.
            state[i] = EATING;

            // self[i]에서 기다리던 철학자 i를 깨운다.
            // 이미 기다리지 않았다면 signal은 효과 없이 사라진다.
            self[i].signal();
        }
    }

    initialization_code() {
        // 모든 철학자는 처음에 생각 중이다.
        for (int i = 0; i < 5; i++)
            state[i] = THINKING;
    }
}
```

각 철학자는 다음 순서로 monitor를 사용한다.

```c
DiningPhilosophers.pickup(i);
EAT
DiningPhilosophers.putdown(i);
```

## 동작 방식 / 원리

1. `pickup(i)`에서 철학자 `i`는 먼저 `HUNGRY`가 된다.
2. `test(i)`는 양옆이 먹고 있지 않을 때만 `i`를 `EATING`으로 바꾼다.
3. 조건이 안 맞으면 `self[i].wait()`로 잠든다.
4. `putdown(i)`는 식사를 끝낸 뒤 양옆 이웃에게 기회가 생겼는지 각각 `test()`한다.
5. monitor 내부에서 상태 검사와 갱신이 보호되므로 두 이웃 철학자가 동시에 같은 젓가락을 쓰는 일이 없다.

## 장점 / 단점

- 장점: 단순 semaphore 해법처럼 모두가 한쪽 젓가락을 들고 서로 기다리는 deadlock을 피한다.
- 장점: 상태 배열을 monitor 내부에서만 바꾸므로 race condition을 막기 쉽다.
- 단점: 어떤 철학자가 이웃에게 계속 밀리면 starvation이 가능하다.
- 단점: condition variable의 signal semantics는 언어 구현에 따라 세부 동작이 달라질 수 있다.

## 시험 출제 포인트

- `test(i)`는 양옆이 `EATING`이 아니고 본인이 `HUNGRY`일 때만 `EATING`으로 바꾼다.
- `putdown(i)`는 자기 자신이 아니라 양옆 이웃을 검사한다.
- 이 monitor 해법은 deadlock-free이지만 starvation-free라고 단정하면 안 된다.

## 3. Semaphore로 Monitor 구현

```c
// monitor 내부 mutual exclusion을 위한 semaphore이다.
// monitor 안에 한 프로세스만 들어오게 한다.
semaphore mutex;      // initially = 1

// signal을 실행한 프로세스가 잠시 기다릴 때 쓰는 semaphore이다.
// 깨어난 프로세스에게 monitor 안 실행 기회를 넘기는 데 사용된다.
semaphore next;       // initially = 0

// next에서 기다리는 프로세스 수이다.
int next_count = 0;
```

각 monitor procedure `F`는 다음 형태로 바뀐다.

```c
wait(mutex);
    // monitor procedure F의 실제 본문이 실행된다.
    // 이 구간은 monitor의 critical section이다.
    ...
    body of F;
    ...

// signal 이후 monitor 내부 우선권을 받을 프로세스가 있으면 next를 깨운다.
if (next_count > 0)
    signal(next);

// next 대기자가 없으면 monitor 밖에서 기다리던 새 진입자를 위해 mutex를 푼다.
else
    signal(mutex);
```

## 동작 방식 / 원리

- `mutex`는 monitor에 동시에 여러 프로세스가 들어오는 것을 막는다.
- `next`는 condition variable에서 signal을 받은 프로세스와 signal을 보낸 프로세스 사이의 순서를 조정한다.
- procedure 종료 시 `next_count > 0`이면, 이미 monitor 안 우선권을 기다리는 프로세스가 있으므로 `mutex` 대신 `next`를 signal한다.

## 4. Condition Variable 구현

condition variable `x`마다 다음 변수를 둔다.

```c
// x에서 기다리는 프로세스들이 block될 semaphore이다.
semaphore x_sem;      // initially = 0

// x에서 기다리는 프로세스 수이다.
int x_count = 0;
```

### `x.wait()`

```c
// 현재 프로세스가 condition x에서 기다리겠다고 등록한다.
x_count++;

// signal을 보낸 뒤 next에서 기다리는 프로세스가 있으면 그 프로세스를 먼저 깨운다.
if (next_count > 0)
    signal(next);

// 그런 프로세스가 없으면 monitor 밖의 새 진입자를 위해 mutex를 푼다.
else
    signal(mutex);

// 현재 프로세스는 x_sem에서 block된다.
// 이 순간 monitor를 양보했기 때문에 다른 프로세스가 monitor 안으로 들어올 수 있다.
wait(x_sem);

// 나중에 x.signal()로 깨어나면 대기자 수를 줄인다.
x_count--;
```

### `x.signal()`

```c
// x에서 실제로 기다리는 프로세스가 있을 때만 의미가 있다.
if (x_count > 0) {
    // signal을 보낸 현재 프로세스가 next에서 기다릴 예정이므로 count를 늘린다.
    next_count++;

    // x에서 기다리던 프로세스 하나를 깨운다.
    signal(x_sem);

    // 현재 프로세스는 next에서 기다린다.
    // 깨어난 프로세스가 monitor 안에서 실행할 기회를 얻는다.
    wait(next);

    // 나중에 다시 실행되면 next 대기자 수를 줄인다.
    next_count--;
}
```

## 동작 방식 / 원리

1. `x.wait()`는 현재 프로세스를 `x` 대기자로 등록하고 monitor를 양보한 뒤 block한다.
2. `x.signal()`은 `x`에서 기다리던 프로세스가 있을 때만 하나를 깨운다.
3. `signal(x_sem)` 직후 현재 프로세스는 `wait(next)`로 물러난다.
4. 이 구조는 `signal and wait` 방식에 가깝다. signal을 받은 프로세스가 monitor 안에서 먼저 진행할 수 있다.

## 장점 / 단점

- 장점: semaphore만으로 monitor의 mutual exclusion과 condition wait/signal을 구현할 수 있다.
- 장점: `next` 큐를 두어 signal 이후의 monitor 내부 우선권을 명확히 다룬다.
- 단점: 구현이 복잡하므로 응용 프로그래머가 직접 다루기에는 semaphore보다 추상화된 monitor가 낫다.

## 시험 출제 포인트

- `x.wait()`는 `wait(x_sem)` 전에 반드시 monitor를 양보해야 한다.
- `x.signal()`은 `x_count > 0`일 때만 실제 효과가 있다.
- `next_count > 0`이면 `signal(mutex)`가 아니라 `signal(next)`가 실행된다.
- `condition variable signal`은 semaphore처럼 count를 누적하지 않는다.

## 5. Single Resource Allocator

```c
monitor ResourceAllocator
{
    // 자원이 사용 중인지 나타낸다.
    // TRUE이면 누군가 자원을 가지고 있고, FALSE이면 비어 있다.
    boolean busy;

    // 자원을 기다리는 프로세스들이 대기할 condition variable이다.
    condition x;

    void acquire(int time) {
        // 자원이 이미 사용 중이면 기다린다.
        // time은 conditional wait에서 priority number로 쓰일 수 있다.
        if (busy)
            x.wait(time);

        // 깨어났거나 처음부터 자원이 비어 있었다면 자원을 사용 중으로 표시한다.
        busy = TRUE;
    }

    void release() {
        // 자원을 비웠다고 표시한다.
        busy = FALSE;

        // 기다리는 프로세스가 있으면 하나를 깨운다.
        // 없으면 아무 효과가 없다.
        x.signal();
    }

    initialization code() {
        // 초기 상태에서는 자원이 비어 있다.
        busy = FALSE;
    }
}
```

## 동작 방식 / 원리

- `acquire(time)`은 자원이 바쁘면 condition variable에서 기다린다.
- `release()`는 자원을 비운 뒤 `x.signal()`로 대기자 하나에게 기회를 준다.
- `x.wait(time)`에서 `time` 같은 priority number를 사용하면 FCFS가 아닌 우선순위 기반 재개 정책을 구현할 수 있다.

## 시험 출제 포인트

- `busy`는 monitor 내부 변수이므로 외부에서 직접 수정하지 않는다.
- `x.signal()`은 기다리는 프로세스가 없으면 아무 효과가 없다.
- conditional wait에서 priority number는 낮을수록 높은 우선순위이다.
