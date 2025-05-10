#pragma once
#include <cstdint>

constexpr uint32_t MAX_FRAMES_IN_FLIGHT = 1;

#define DISABLE_COPY(className)           \
    className(const className&) = delete; \
    className& operator=(const className&) = delete

#define DISABLE_MOVE(className)      \
    className(className&&) = delete; \
    className& operator=(className&&) = delete