# ITCH 수신기로 배우는 저지연 C++ — 학습 로드맵

> 목표: ITCH 시장 데이터를 받아 파싱하는 C++ 수신기를 **작은 단계로** 만들면서, 매 단계마다 중급 STL + 저지연 시스템 개념(mutex, cache line, prefetch, CPU affinity, kernel bypass)을 하나씩 익힌다.
> 전제: 기초 문법은 안다 / 개발은 Mac, 저수준 실습은 Linux 컨테이너 / 천천히, 손으로.

---

## 0. 환경 셋업 (먼저 1회)

**왜 두 환경인가?**
- **Mac (개발)**: 코드 작성·컴파일·일반 디버깅. 빠르고 편함.
- **Linux x86 컨테이너 (저수준 실습)**: `sched_setaffinity`(CPU 고정), `AF_XDP`, `perf`, 정확한 캐시 동작 등은 **Linux 전용**이거나 x86에서만 제대로 배움.

> ⚠️ **Apple Silicon 주의**: M칩은 ARM이고 **캐시 라인이 128바이트**예요(x86은 64바이트). HFT 실무는 x86 기준이라, 캐시·prefetch·affinity 실습은 꼭 Linux x86 컨테이너에서 하세요. 개념 학습은 Mac에서도 다 됩니다.

**준비물**
- Mac: `clang++`(Xcode CLT), `cmake`, 에디터
- Docker Desktop → x86 리눅스 컨테이너 (`--platform linux/amd64`로 ubuntu, `g++`/`clang++`/`perf` 설치)
- C++ 표준: **C++20** (`std::span`, `std::endian`, `std::bit_cast`, `<bit>` 등 핵심 기능 사용)

**체크포인트 0**: Mac과 컨테이너 양쪽에서 `Hello, ITCH` 출력되는 `-std=c++20 -O2` 빌드 성공.

---

## 진행 방식

각 단계는 이렇게 구성돼요:
- 🎯 **목표** — 이 단계에서 끝내는 것
- 🔨 **만드는 것** — 작은 코드 한 조각
- 📘 **새 개념** — C++ STL/문법 + 시스템 개념
- 🏛 **HFT 연결** — 실무에서 왜 중요한가
- ✅ **체크포인트** — 넘어가기 전 확인할 작은 과제

> 한 단계 = 하루~며칠. 절대 서두르지 말고, 체크포인트를 통과해야 다음으로.

---

## Part A. 바이트를 다루기 (네트워크 없이)

### Stage 1 — ITCH 메시지 한 개 파싱하기
- 🎯 하드코딩된 바이트 배열에서 Add Order 메시지 하나를 읽어 필드 출력
- 🔨 `parse_add_order(바이트 버퍼) → 구조체` 함수. 입력은 MIT 문서의 예시 hex 사용
- 📘 새 개념
  - **고정 너비 정수** `<cstdint>` (`uint8_t`, `uint32_t`, `uint64_t`) — 왜 `int` 쓰면 안 되나
  - **빅엔디언 디코딩** — ITCH는 big-endian. `<bit>`의 `std::endian`, `std::byteswap`(C++23) 또는 직접 시프트
  - `std::byte` vs `char` vs `uint8_t`
  - **`std::span<const std::byte>`** (C++20) — 복사 없이 버퍼를 가리키는 뷰 ← 저지연 핵심 개념의 시작
- 🏛 HFT 연결: 파싱은 Hot Path. **복사 없이(zero-copy)** 바이트를 해석하는 게 출발점
- ✅ 체크포인트: Add Order의 order_id·price·shares·stock을 올바르게 출력

### Stage 2 — 메시지 스트림 파싱 + 타입 분기
- 🎯 여러 메시지가 이어진 버퍼를 돌며 타입별로 분기 (Add / Cancel / Execute)
- 🔨 길이→타입 읽고 `switch`로 디스패치하는 루프
- 📘 새 개념
  - **`enum class`** 로 메시지 타입 (`'A'`, `'X'`, `'D'`…)
  - `switch` 디스패치, `[[likely]]`/`[[unlikely]]` 힌트
  - **`std::string_view` / `std::span`** — 비소유 뷰로 잘라보기 (메모리 할당 0)
  - `constexpr` 상수, 함수 분리
- 🏛 HFT 연결: 메시지 디스패치는 분기 예측·할당 회피가 중요. 여기서 "할당하지 마라"의 감을 잡음
- ✅ 체크포인트: 3종 메시지가 섞인 버퍼를 끝까지 정확히 파싱, 미지원 타입은 길이만큼 스킵

### Stage 3 — 파일에서 ITCH 리플레이
- 🎯 ITCH 메시지가 담긴 바이너리 파일을 읽어 전부 파싱 (= 미니 거래소 리플레이)
- 🔨 파일 → 버퍼 적재 → Stage 2 파서 실행
- 📘 새 개념
  - 파일 I/O: `std::ifstream` vs POSIX `read()` vs **`mmap`** (각 트레이드오프)
  - **`std::vector<std::byte>`** 버퍼 관리, `reserve`로 재할당 회피
  - **RAII** — 파일·자원 자동 정리 (소멸자, `std::fstream`/커스텀 핸들)
  - 예외 vs 에러코드
- 🏛 HFT 연결: 마켓 데이터 **리플레이**는 Cold Path 도구(테스트·백테스트의 기반). 이게 우리 "거래소 시뮬레이터"의 씨앗
- ✅ 체크포인트: 수천 개 메시지 파일을 무할당 루프로 파싱, 메시지 수·타입 집계 출력

---

## Part B. 네트워크로 받기

### Stage 4 — UDP로 ITCH 수신
- 🎯 송신기(Stage 3을 네트워크 버전으로)가 보내는 ITCH 패킷을 UDP 소켓으로 수신
- 🔨 `socket`/`bind`/`recvfrom` 수신기 + 같은 코드로 `sendto` 송신기
- 📘 새 개념
  - **BSD 소켓 API**, UDP vs TCP (HFT 피드는 보통 UDP **멀티캐스트**)
  - MoldUDP64 같은 프레이밍 개념 (시퀀스 번호, 패킷당 여러 메시지)
  - 패킷 손실·순서 → 시퀀스 갭 처리 맛보기
  - **커널 네트워크 스택 경로** 이해 (나중에 "왜 bypass" 의 빌드업)
- 🏛 HFT 연결: 표준 인그레스. 멀티캐스트 피드 수신이 Hot Path의 입구
- ✅ 체크포인트: 로컬에서 송신기→수신기로 ITCH가 흘러 파싱됨 (Mac·컨테이너 둘 다)

### Stage 5 — 지연 측정 (제대로)
- 🎯 메시지당 파싱 지연을 측정하고 분포를 본다
- 🔨 각 메시지에 타임스탬프, 지연 히스토그램 출력
- 📘 새 개념
  - **`std::chrono`** high-resolution clock, `steady_clock`
  - `rdtsc`(x86 사이클 카운터) — 컨테이너에서
  - **올바른 측정법**: 워밍업, 평균이 아니라 **p50/p99/p99.9 퍼센타일**, 지터(jitter)
  - Linux `perf stat` 첫 사용
- 🏛 HFT 연결: "측정 못 하면 최적화 못 한다." HFT는 평균이 아니라 **꼬리 지연(tail latency)·결정성**이 생명
- ✅ 체크포인트: p50/p99 지연을 숫자로 보고하고, -O0 vs -O3 차이를 측정

---

## Part C. 저수준 성능 (Linux 컨테이너 필수)

### Stage 6 — 캐시 라인 & 데이터 레이아웃
- 🎯 오더북 엔트리를 캐시 친화적으로 배치하고 false sharing을 체감
- 🔨 같은 데이터를 AoS vs SoA로 두고 처리 속도 비교 / false sharing 데모
- 📘 새 개념
  - **캐시 라인 크기** (x86 64B, Apple Silicon 128B), `std::hardware_destructive_interference_size`(C++17)
  - **AoS(Array of Structs) vs SoA(Struct of Arrays)**
  - `alignas`, 패딩, 구조체 멤버 정렬
  - **false sharing** — 두 스레드가 같은 캐시라인 다투기
- 🏛 HFT 연결: 알고리즘보다 **데이터 레이아웃이 실측 성능을 지배**하는 경우가 많음
- ✅ 체크포인트: false sharing 있는 버전 vs `alignas(64)`로 고친 버전의 속도 차이 측정

### Stage 7 — Prefetching
- 🎯 다음에 쓸 데이터를 미리 캐시로 당겨와 메모리 지연 숨기기
- 🔨 오더북 순회 시 다음 엔트리 prefetch, 효과 측정
- 📘 새 개념
  - `__builtin_prefetch` / `_mm_prefetch`
  - 언제 도움 되고 언제 해가 되는가 (잘못 쓰면 오히려 느려짐)
  - 메모리 계층(L1/L2/L3/RAM) 지연 감각
- 🏛 HFT 연결: 메모리 접근 지연을 계산과 겹쳐 숨김 — 핫패스 상수 지연 확보
- ✅ 체크포인트: prefetch 유/무로 처리량 차이 측정, 역효과 케이스도 한 번 만들어보기

### Stage 8 — 스레드, mutex, 그리고 lock-free
- 🎯 수신 스레드(producer) → 파서 스레드(consumer)를 큐로 연결
- 🔨 ① 먼저 `std::mutex` 보호 큐 → ② SPSC lock-free 링버퍼로 교체
- 📘 새 개념
  - **`std::thread`**, **`std::mutex`**, `std::lock_guard`/`scoped_lock`
  - **왜 mutex가 핫패스에서 나쁜가** — 경합, 커널 진입, 지터
  - **`std::atomic`**, 메모리 순서(`memory_order_acquire/release`) 입문
  - **SPSC(단일 생산자-단일 소비자) lock-free 링버퍼** — HFT의 단골 자료구조
- 🏛 HFT 연결: 핫패스는 락을 피한다. 스레드 간 통신은 거의 SPSC 큐
- ✅ 체크포인트: mutex 버전 vs lock-free 버전의 p99 지연·지터 비교

### Stage 9 — CPU Affinity & 코어 격리
- 🎯 수신·파서 스레드를 특정 코어에 고정
- 🔨 `pthread_setaffinity_np`로 코어 핀, busy-poll 수신 루프
- 📘 새 개념
  - **`sched_setaffinity` / `pthread_setaffinity_np`** (Linux 전용)
  - **코어 격리** (`isolcpus`), 컨텍스트 스위치·인터럽트 회피, NUMA 기초
  - busy-polling vs blocking 수신
- 🏛 HFT 연결: 전용 코어에 핀하고 OS 방해를 차단해 **결정적 지연** 확보
- ⚠️ macOS는 코어 핀이 제한적(QoS 힌트만) → **이 단계는 컨테이너에서**
- ✅ 체크포인트: 핀 전/후 지터 차이 측정

### Stage 10 — Kernel Bypass (개념 + 현실적 실습)
- 🎯 커널 우회가 왜 필요한지 이해하고, 접근 가능한 대안을 실습
- 🔨 `io_uring` 또는 `AF_XDP`로 저오버헤드 수신 시도 (DPDK·Solarflare Onload는 개념으로)
- 📘 새 개념
  - **왜 bypass** — 커널 스택의 복사·컨텍스트 스위치·인터럽트 제거
  - user-space 드라이버, busy-poll, zero-copy RX
  - 실무 옵션: **DPDK, AF_XDP, Solarflare Onload** (대부분 전용 NIC 필요)
- 🏛 HFT 연결: 프로덕션 인그레스의 실체. 마이크로초를 더 깎는 마지막 단계
- ⚠️ **현실 메모**: 진짜 kernel bypass(DPDK/Onload)는 특정 NIC·하드웨어가 필요해 VM에서 완전 재현은 어려워요. 그래서 여기선 **개념 이해 + `AF_XDP`/`io_uring`** 으로 가장 근접한 체험을 합니다
- ✅ 체크포인트: 일반 소켓 vs io_uring/AF_XDP 수신 오버헤드 비교, bypass 동기를 자기 말로 설명

---

## Part D. 마무리 — 전체 프로젝트와 연결

이 10단계를 끝내면 손에 남는 것:
1. **C++ ITCH 수신·파서** → 우리 HFT 가속기의 "거래소 시뮬레이터 + 골든 레퍼런스"의 토대
2. **저지연 사고방식** → Hot/Warm/Cold Path를 코드로 체감
3. **측정 습관** → 나중에 FPGA RTL과 결과·지연을 비교할 기준

> 다음 큰 그림: 이 C++ 수신기가 (a) FPGA에 보낼 ITCH를 만들어주는 송신기가 되고, (b) FPGA 오더북이 내놓는 결과를 대조할 정답지가 됩니다.

---

## 빠른 참조 — 단계별 한 줄 요약

| 단계 | 만드는 것 | 핵심 개념 |
|------|-----------|-----------|
| 1 | 메시지 1개 파싱 | 고정폭 정수·빅엔디언·`std::span` |
| 2 | 스트림 파싱 | `enum class`·`string_view`·무할당 |
| 3 | 파일 리플레이 | I/O·`vector`·RAII·`mmap` |
| 4 | UDP 수신 | 소켓·멀티캐스트·프레이밍 |
| 5 | 지연 측정 | `chrono`·rdtsc·퍼센타일·perf |
| 6 | 데이터 레이아웃 | 캐시 라인·AoS/SoA·false sharing |
| 7 | Prefetch | `__builtin_prefetch`·메모리 계층 |
| 8 | 스레드 큐 | mutex·atomic·SPSC lock-free |
| 9 | CPU 핀 | affinity·코어 격리·NUMA |
| 10 | Kernel bypass | io_uring·AF_XDP·DPDK 개념 |

---

### 다음 단계
이 로드맵이 괜찮으면 **Stage 1부터 바로 코딩**을 시작해요. 첫 코드는 작게 — MIT 문서의 Add Order 예시 hex 바이트를 그대로 써서, 복사 없이 필드를 읽어내는 함수 하나부터 만들 거예요. 준비되면 "Stage 1 시작"이라고 말해주세요.
