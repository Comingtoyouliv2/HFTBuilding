# itch-receiver — 빌드 설정
# 4주 학습 동안 단계별로 실행 파일을 여기에 추가해 나간다.

cmake_minimum_required(VERSION 3.16)
project(itch_receiver CXX)

# --- C++ 표준: C++20 강제 (std::span, std::endian 등 사용) ---
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)        # GNU 확장 끄고 순수 표준으로

# --- 빌드 타입 기본값: Release (최적화 켜짐) ---
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

# --- 경고는 많이, 최적화는 -O2 (측정할 땐 의미 있게) ---
add_compile_options(-Wall -Wextra)

# === 실행 파일들 ===
# Day 1: Hello, ITCH
add_executable(hello src/main.cpp)

# (다음 단계에서 add_executable(parse src/parse.cpp) 식으로 추가 예정)
add_executable(hexdump src/hexdump.cpp)
