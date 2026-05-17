---
subject: 운영체제
study_type: B
concept: Deadlocks
tags: [4-1, 운영체제, deadlock, resource-allocation-graph, prevention, avoidance]
source_files: [운영체제/필기본/ch7/os-ch7_18p까지.pdf]
created: 2026-05-17
last_reviewed: 2026-05-17
status: review
---

# Chapter 7: Deadlocks 정리본

## 개념도

```mermaid
flowchart TD
    A[Concurrent processes] --> B[Resource request]
    B --> C[request]
    C --> D[use]
    D --> E[release]
    B --> F[Deadlock]
    F --> G[Mutual exclusion]
    F --> H[Hold and wait]
    F --> I[No preemption]
    F --> J[Circular wait]
    F --> K[Resource-allocation graph]
    K --> L[No cycle: no deadlock]
    K --> M[Cycle + one instance: deadlock]
    K --> N[Cycle + multiple instances: possible deadlock]
    F --> O[Handling methods]
    O --> P[Prevention]
    O --> Q[Avoidance]
    O --> R[Detection and recovery]
    O --> S[Ignore]
```

## 1. Deadlock의 목표와 배경

Deadlock은 **동시에 실행되는 여러 프로세스 집합이 서로가 가진 자원을 기다리느라 더 이상 작업을 완료하지 못하는 상태**다. Chapter 7의 목표는 deadlock이 어떤 조건에서 생기는지 설명하고, 시스템이 deadlock에 빠지지 않도록 **prevention** 또는 **avoidance**하는 방법의 기본 관점을 잡는 것이다.

시험에서는 정의만 묻기보다, 작은 상황을 주고 "이 조건이 성립하는가", "그래프에 cycle이 있으면 반드시 deadlock인가", "어떤 prevention 방식이 어떤 조건을 깨는가"처럼 비교형으로 묻기 쉽다.

## 2. System Model

시스템은 여러 **resource type**으로 이루어진다.

- 자원 타입: `R1, R2, ..., Rm`
- 예시: CPU cycles, memory space, I/O devices
- 각 자원 타입 `Ri`는 `Wi`개의 **instance**를 가진다.

프로세스가 자원을 사용하는 흐름은 세 단계다.

1. **request**: 자원을 요청한다.
2. **use**: 할당받은 자원을 사용한다.
3. **release**: 사용을 마친 자원을 반납한다.

Deadlock은 이 흐름 중 **request 단계에서 대기 상태가 꼬일 때** 발생한다. 특히 이미 자원을 가진 프로세스가 추가 자원을 요청하고, 그 추가 자원을 다른 프로세스가 잡고 있으면 대기 관계가 생긴다.

## 3. Deadlock Characterization: 네 가지 필요 조건

Deadlock은 다음 네 조건이 **동시에 성립할 때** 발생할 수 있다. 하나라도 깨지면 deadlock은 성립하지 않는다.

### 3.1 Mutual Exclusion

**한 번에 하나의 프로세스만 자원을 사용할 수 있는 조건**이다.

프린터, mutex lock처럼 동시에 공유할 수 없는 자원은 mutual exclusion이 필요하다. 반대로 read-only file처럼 공유 가능한 자원에는 이 조건이 필요하지 않다.

### 3.2 Hold and Wait

**프로세스가 최소 하나의 자원을 가진 상태에서 다른 프로세스가 가진 추가 자원을 기다리는 조건**이다.

핵심은 "잡고 있으면서 기다린다"는 점이다. 아무 자원도 들고 있지 않은 채 기다리기만 하면 hold and wait 조건이 성립하지 않는다.

### 3.3 No Preemption

**프로세스가 가진 자원은 그 프로세스가 작업을 끝내고 자발적으로 반납할 때만 release될 수 있다는 조건**이다.

운영체제가 강제로 자원을 빼앗을 수 없다면, 이미 잘못 형성된 대기 관계를 중간에 끊기 어렵다.

### 3.4 Circular Wait

**대기 중인 프로세스들이 원형으로 서로의 자원을 기다리는 조건**이다.

예를 들어 `{P0, P1, ..., Pn}`에 대해 `P0`은 `P1`이 가진 자원을 기다리고, `P1`은 `P2`가 가진 자원을 기다리며, 마지막 `Pn`은 다시 `P0`이 가진 자원을 기다리면 circular wait가 성립한다.

## 4. Mutex Lock에서의 Deadlock

Deadlock은 시스템 콜이나 lock 사용에서도 발생할 수 있다. 대표적인 실수는 두 thread가 같은 mutex 두 개를 **서로 다른 순서**로 획득하는 경우다.

```c
/* thread one */
pthread_mutex_lock(&first_mutex);
pthread_mutex_lock(&second_mutex);
/* do some work */
pthread_mutex_unlock(&second_mutex);
pthread_mutex_unlock(&first_mutex);

/* thread two */
pthread_mutex_lock(&second_mutex);
pthread_mutex_lock(&first_mutex);
/* do some work */
pthread_mutex_unlock(&first_mutex);
pthread_mutex_unlock(&second_mutex);
```

문제 상황은 다음처럼 생긴다.

| 단계 | Thread 1 | Thread 2 |
|---|---|---|
| 1 | `first_mutex` 획득 |  |
| 2 |  | `second_mutex` 획득 |
| 3 | `second_mutex` 요청 후 대기 |  |
| 4 |  | `first_mutex` 요청 후 대기 |

이때 Thread 1은 Thread 2가 가진 `second_mutex`를 기다리고, Thread 2는 Thread 1이 가진 `first_mutex`를 기다린다. 두 thread 모두 이미 하나의 lock을 잡고 있으므로 **hold and wait**가 성립하고, lock을 강제로 빼앗지 못하면 **no preemption**도 성립한다. 서로의 lock을 기다리므로 **circular wait**가 완성된다.

## 5. Resource-Allocation Graph

Deadlock을 시각적으로 분석할 때 **Resource-Allocation Graph**를 사용한다. 그래프는 vertex 집합 `V`와 edge 집합 `E`로 구성된다.

### 5.1 Vertex 종류

- `P = {P1, P2, ..., Pn}`: 시스템의 프로세스 집합
- `R = {R1, R2, ..., Rm}`: 시스템의 자원 타입 집합

자원 타입이 여러 instance를 가지면 자원 노드 안에 여러 instance가 표시된다.

### 5.2 Edge 종류

| Edge | 의미 | 방향 |
|---|---|---|
| Request edge | 프로세스 `Pi`가 자원 `Rj`를 요청 중 | `Pi -> Rj` |
| Assignment edge | 자원 `Rj`의 instance가 프로세스 `Pi`에 할당됨 | `Rj -> Pi` |

방향을 반대로 외우면 그래프 해석이 완전히 달라진다. **프로세스에서 자원으로 가면 요청**, **자원에서 프로세스로 가면 할당**이다.

## 6. Resource-Allocation Graph의 기본 판정

그래프에서 cycle의 존재 여부는 deadlock 판정의 출발점이다.

| 그래프 상태 | 판정 |
|---|---|
| Cycle 없음 | **deadlock 없음** |
| Cycle 있음 + 각 resource type이 instance 1개 | **deadlock** |
| Cycle 있음 + 어떤 resource type이 여러 instance | **deadlock 가능성 있음**, 반드시 deadlock은 아님 |

시험 함정은 세 번째 줄이다. **cycle이 있다고 항상 deadlock은 아니다.** 자원 타입에 여러 instance가 있으면 cycle이 있어도 다른 instance가 release되면서 대기가 풀릴 수 있다.

### 그림 문제 해석 요령

1. 프로세스와 자원 타입을 나눈다.
2. `Pi -> Rj`는 요청, `Rj -> Pi`는 할당으로 읽는다.
3. 방향을 따라 cycle이 있는지 확인한다.
4. cycle이 있다면 cycle에 포함된 자원 타입의 instance 수를 확인한다.
5. instance가 모두 1개면 deadlock, 여러 instance가 섞이면 가능성으로만 판단한다.

## 7. Methods for Handling Deadlocks

Deadlock 처리 방법은 크게 네 가지 관점으로 나뉜다.

### 7.1 Deadlock이 절대 생기지 않게 보장

시스템이 deadlock state에 들어가지 않도록 만드는 방식이다.

- **Deadlock prevention**: deadlock의 네 필요 조건 중 하나를 구조적으로 깨뜨린다.
- **Deadlock avoidance**: 실행 중 자원 할당 상태를 검사하여 위험한 상태로 들어가지 않게 한다.

### 7.2 Deadlock을 허용한 뒤 복구

시스템이 deadlock state에 들어가는 것을 허용하고, 이후 **detection**으로 deadlock을 찾은 뒤 **recovery**를 수행한다.

### 7.3 문제를 무시

Deadlock이 실제로는 드물다고 보고, deadlock이 절대 발생하지 않는 것처럼 취급한다. UNIX를 포함한 많은 운영체제가 이 접근을 사용한다.

이 방식은 구현 비용이 낮지만, deadlock이 발생하면 사용자나 관리자가 개입해야 할 수 있다.

## 8. Deadlock Prevention

Deadlock prevention은 **request가 만들어지는 방식을 제한**해서 네 필요 조건 중 하나가 성립하지 못하게 한다.

### 8.1 Mutual Exclusion 깨기

공유 가능한 자원에는 mutual exclusion이 필요하지 않다. 예를 들어 read-only file은 여러 프로세스가 동시에 읽어도 된다.

하지만 프린터, mutex lock 같은 non-sharable resource는 본질적으로 mutual exclusion이 필요하다. 따라서 모든 자원에서 mutual exclusion을 제거하는 것은 현실적이지 않다.

### 8.2 Hold and Wait 깨기

프로세스가 자원을 요청할 때 **다른 자원을 들고 있지 않도록 보장**한다.

방법은 두 가지로 볼 수 있다.

- 프로세스가 실행을 시작하기 전에 필요한 모든 자원을 한 번에 요청하고 할당받게 한다.
- 프로세스가 아무 자원도 할당받지 않은 상태에서만 자원을 요청하게 한다.

단점도 중요하다.

- 필요한 자원을 미리 오래 잡을 수 있어 **resource utilization이 낮아진다**.
- 특정 프로세스가 필요한 모든 자원을 동시에 얻지 못해 **starvation이 가능하다**.

### 8.3 No Preemption 깨기

프로세스가 어떤 자원을 가진 상태에서 즉시 할당될 수 없는 다른 자원을 요청하면, 현재 들고 있던 자원을 모두 release하게 한다.

처리 흐름은 다음과 같다.

1. 프로세스가 자원을 가진 상태에서 추가 자원을 요청한다.
2. 추가 자원을 즉시 받을 수 없다.
3. 현재 들고 있던 자원을 모두 반납한다.
4. 반납된 자원은 그 프로세스가 기다리는 자원 목록에 추가된다.
5. 프로세스는 예전 자원과 새로 요청한 자원을 모두 다시 얻을 수 있을 때 재시작된다.

이 방식은 lock처럼 중간에 빼앗기 어려운 자원에는 적용이 까다롭다.

### 8.4 Circular Wait 깨기

모든 resource type에 **total ordering**을 부여하고, 각 프로세스가 자원을 **번호가 증가하는 순서로만** 요청하게 한다.

예를 들어 `R1 < R2 < R3`로 순서를 정했다면, `R2`를 잡은 프로세스가 나중에 `R1`을 요청할 수 없게 한다. 이렇게 하면 대기 관계가 한 방향으로만 생겨 원형 대기가 만들어지지 않는다.

## 9. Lock Ordering 예시

계좌 이체 코드에서 두 계좌의 lock을 잡아야 한다고 하자.

```c
void transaction(Account from, Account to, double amount)
{
    mutex lock1, lock2;
    lock1 = get_lock(from);
    lock2 = get_lock(to);
    acquire(lock1);
        acquire(lock2);
            withdraw(from, amount);
            deposit(to, amount);
        release(lock2);
    release(lock1);
}
```

Transaction 1이 A에서 B로 이체하고, Transaction 2가 B에서 A로 이체하면 다음 위험이 생긴다.

- Transaction 1: A lock을 먼저 잡고 B lock을 기다림
- Transaction 2: B lock을 먼저 잡고 A lock을 기다림

이 상황은 circular wait를 만들 수 있다. 해결하려면 계좌나 lock에 전역 순서를 부여하고, 모든 transaction이 항상 같은 순서로 lock을 획득해야 한다. 즉, `from/to`의 역할이 아니라 **정해진 lock ordering**이 기준이 되어야 한다.

## 10. Deadlock Avoidance

Deadlock avoidance는 prevention처럼 요청 방식을 무조건 제한하기보다, 시스템이 각 요청을 처리할 때 현재 상태를 검사해서 deadlock이 생길 수 있는 위험 상태로 들어가지 않게 한다.

가장 단순하고 유용한 모델은 각 프로세스가 실행 전에 **각 resource type별 최대 필요량(maximum number of resources)**을 선언한다고 가정한다.

Avoidance 알고리즘은 자원 할당 상태를 동적으로 검사하여 **circular-wait condition이 절대 생기지 않도록** 보장한다.

자원 할당 상태는 다음 정보로 정의된다.

- available resources: 현재 사용 가능한 자원 수
- allocated resources: 각 프로세스에 이미 할당된 자원 수
- maximum demands: 각 프로세스가 최대로 요구할 수 있는 자원 수

핵심 전제는 **추가적인 사전 정보(a priori information)**가 필요하다는 점이다. 즉, 프로세스가 앞으로 얼마나 많은 자원을 필요로 할 수 있는지 모르면 avoidance를 적용하기 어렵다.

## 11. 시험 포인트

- Deadlock은 네 조건이 **동시에** 성립할 때 발생 가능하다.
- `Pi -> Rj`는 request edge, `Rj -> Pi`는 assignment edge다.
- 그래프에 cycle이 없으면 deadlock은 없다.
- cycle이 있고 각 resource type instance가 하나면 deadlock이다.
- cycle이 있어도 resource type에 여러 instance가 있으면 deadlock은 가능성이지 확정이 아니다.
- Prevention은 네 필요 조건 중 하나를 깨는 방식이다.
- Hold and wait를 깨면 resource utilization이 낮아지고 starvation이 가능하다.
- Circular wait를 깨려면 resource type에 total ordering을 부여하고 증가 순서로 요청하게 한다.
- Lock ordering은 `from/to` 같은 호출 맥락이 아니라 모든 실행이 공유하는 전역 순서를 기준으로 해야 한다.
- Avoidance는 최대 요구량 같은 사전 정보를 필요로 한다.
