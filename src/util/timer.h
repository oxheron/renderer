#pragma once

// std
#include <chrono>
#include <concepts>
#include <thread>

template <typename T>
concept Clock = std::same_as<T, std::chrono::high_resolution_clock> || std::same_as<T, std::chrono::steady_clock> || std::same_as<T, std::chrono::system_clock>;

template <Clock T>
class Timer
{
private:
    std::chrono::time_point<T> start;
public:
    Timer() { reset_timer(); }

    void reset_timer() { start = T::now(); }
    float get_time() const { return std::chrono::duration_cast<std::chrono::seconds>(T::now() - start).count(); }
    std::chrono::nanoseconds get_time_ns() const { return std::chrono::duration_cast<std::chrono::nanoseconds>(T::now() - start); }
};

// Manages all of the time needs, such as delta time, time since start, fps, tps, etc.
// Also does the timing for the game loop
class TimeManager
{
private:
    Timer<std::chrono::high_resolution_clock> timer;
    std::chrono::nanoseconds time_since_start;
    std::chrono::nanoseconds time_since_last_second;
    std::chrono::nanoseconds delta_time;
    uint32_t fps = 0;
    uint32_t frame_ctr = 0;

    uint32_t target_fps = 60;
public:
    TimeManager() {}
    TimeManager(uint32_t target_fps) : target_fps(target_fps) {}

    void update();

    inline std::chrono::nanoseconds get_time_since_start() const { return time_since_start; }
    inline std::chrono::nanoseconds get_delta_time() const { return delta_time; }
    inline uint32_t get_fps() const { return fps; }
    inline bool is_second() const { return time_since_last_second >= std::chrono::seconds(1); }
    inline void hold_at_fps() const { std::this_thread::sleep_for(std::chrono::milliseconds((1000 / target_fps)) - std::chrono::duration_cast<std::chrono::milliseconds>(timer.get_time_ns())); }
    inline void set_fps(uint32_t fps) { target_fps = fps; }
};
