# Linux 학습용 컨테이너 (저수준 실습용)
#
# 기본은 '네이티브' 아키텍처로 빌드한다.
#   - Apple Silicon(M칩) Mac이면 ARM 리눅스로 떠서 '빠르다'. 일상 개발/대부분 학습에 권장.
#   - x86 특유의 동작(캐시 라인 64B, rdtsc 등)을 '직접 보고' 싶을 때만
#     아래 build/run에서 --platform linux/amd64 를 붙인다.
#     단, M칩에서 amd64는 에뮬레이션(qemu)이라 '느리고 지연 측정값을 신뢰하면 안 된다'.
#
# 빌드:  docker build -t itch-dev .
# 실행:  docker run --rm -it -v "$PWD":/work itch-dev

FROM ubuntu:24.04

RUN apt-get update && apt-get install -y --no-install-recommends \
        build-essential \
        cmake \
        gdb \
        ca-certificates \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /work
CMD ["/bin/bash"]
