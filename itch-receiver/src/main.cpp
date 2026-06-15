// Day 1 — Hello, ITCH
// 목표: Mac과 Linux 컨테이너 양쪽에서 C++20 빌드가 되는지 확인한다.
//
// 이 작은 프로그램이 알려주는 것:
//   1) 우리가 진짜 C++20으로 컴파일하고 있는가  (__cplusplus)
//   2) 내 CPU의 바이트 순서는?  (ITCH는 big-endian이라 앞으로 중요)

#include <cstdio>
#include <bit>     // std::endian (C++20)

int main() {
    std::puts("Hello, ITCH!");

    // C++ 표준 버전 확인: C++20이면 202002 이상이 찍힌다.
    std::printf("__cplusplus = %ld\n", __cplusplus);

    // ITCH 프로토콜은 big-endian. 내 PC는 어느 쪽인지 확인해 둔다.
    if constexpr (std::endian::native == std::endian::little) {
        std::puts("Host byte order: little-endian"
                  "  -> ITCH(big-endian)을 읽을 때 byteswap이 필요하다 (Stage 1에서)");
    } else {
        std::puts("Host byte order: big-endian"
                  "  -> ITCH와 같은 순서 (드문 경우)");
    }



    return 0;
}
