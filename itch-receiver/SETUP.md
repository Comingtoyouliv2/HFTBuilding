# Day 1 — 환경 셋업 체크리스트

목표: **Mac과 Linux 컨테이너 양쪽에서 `Hello, ITCH`가 빌드·실행**되면 끝.
프로젝트 위치: 이 폴더(`itch-receiver/`). 코드는 이미 준비돼 있고, 빌드는 내가(Claude) 검증해뒀어요.

---

## A. Mac에서 빌드 (먼저)

### 1) 도구 확인
터미널을 열고 하나씩:
```bash
clang++ --version     # Xcode Command Line Tools에 포함. 없으면: xcode-select --install
cmake --version       # 없으면: brew install cmake   (Homebrew 없으면 https://brew.sh)
```

### 2) 빌드 & 실행
```bash
cd "/Users/wchoi/Documents/Claude/Projects/HFTBuilding/itch-receiver"
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
./build/hello
```

기대 출력:
```
Hello, ITCH!
__cplusplus = 202002        (또는 그 이상)
Host byte order: little-endian  -> ...
```

> `202002`가 보이면 C++20으로 잘 컴파일된 거예요. Apple Silicon이면 `little-endian`이 정상.

---

## B. Linux 컨테이너에서 빌드 (저수준 실습용)

Stage 6부터는 캐시·affinity 같은 걸 컨테이너에서 실습해요. 미리 깔아둡니다.

### 1) Docker Desktop 설치
https://www.docker.com/products/docker-desktop/ → 설치 후 실행 (메뉴바에 고래 아이콘)

### 2) 이미지 빌드 (이 폴더에서)
```bash
cd "/Users/wchoi/Documents/Claude/Projects/HFTBuilding/itch-receiver"
docker build -t itch-dev .
```

### 3) 컨테이너 안에서 빌드·실행
```bash
docker run --rm -it -v "$PWD":/work itch-dev
# ↓ 여기부터는 컨테이너 안쪽 셸
cd /work
cmake -B build-linux -DCMAKE_BUILD_TYPE=Release
cmake --build build-linux
./build-linux/hello
exit    # 컨테이너 나가기
```

> Mac 빌드는 `build/`, 컨테이너 빌드는 `build-linux/`로 폴더를 나눠 섞이지 않게 했어요.

---

## C. (선택) x86 동작을 직접 보고 싶을 때

캐시 라인 64B, `rdtsc` 같은 **x86 고유 동작**을 보려면 amd64로:
```bash
docker build --platform linux/amd64 -t itch-dev-x86 .
docker run --rm -it --platform linux/amd64 -v "$PWD":/work itch-dev-x86
```
⚠️ **주의**: M칩에서 amd64는 에뮬레이션(qemu)이라 **느리고, 지연 측정값은 신뢰하면 안 돼요.** "x86은 이렇게 동작하는구나"를 *보는* 용도로만. 정확한 측정은 나중에 실제 x86 리눅스(클라우드 VM)에서.

---

## ✅ Day 1 체크포인트
- [ ] Mac에서 `./build/hello` 실행 → `Hello, ITCH!` + `__cplusplus = 202002`
- [ ] 컨테이너에서 `./build-linux/hello` 실행 → 동일 출력
- [ ] (이해) ITCH는 big-endian, 내 PC는 little-endian → 앞으로 byteswap이 필요하다는 점

세 개 다 되면 Day 1 완료예요. 다음은 **Day 2 — 바이트의 기초**(hex 덤프, `std::byte`, 고정폭 정수).

---

## 막히면
- `clang++` 없다 → `xcode-select --install`
- `cmake` 없다 → `brew install cmake`
- `docker: command not found` → Docker Desktop 설치/실행 확인
- 그 외 에러 메시지는 그대로 복사해서 물어보세요. 같이 해결해요.

## 참고: 남아있는 `build/` 폴더
이 폴더에 `build/`가 이미 보이면, 제가 빌드를 검증하며 생긴 흔적이에요. 무해하고 git에서 제외돼 있어요. 깔끔하게 지우려면 Mac 터미널에서 `rm -rf build` 하면 됩니다.
