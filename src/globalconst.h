#pragma once
#include <cstdint>

constexpr uint32_t max_frames_in_flight = 2; // TODO: Would like this to be dynamic
constexpr bool     is_imgui_enabeld = true;

#define DISABLE_COPY(className)           \
    className(const className&) = delete; \
    className& operator=(const className&) = delete

#define DISABLE_MOVE(className)      \
    className(className&&) = delete; \
    className& operator=(className&&) = delete
