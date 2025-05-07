#pragma once

constexpr uint32_t MAX_FRAMES_IN_FLIGHT = 2;

#define DISABLE_COPY(className)           \
    className(const className&) = delete; \
    className& operator=(const className&) = delete

#define DISABLE_MOVE(className)      \
    className(className&&) = delete; \
    className& operator=(className&&) = delete