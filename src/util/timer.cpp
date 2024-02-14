#include "timer.h"

void TimeManager::update()
{
    frame_ctr++;

    delta_time = timer.get_time_ns();
    time_since_start += delta_time;
    timer.reset_timer();

    if (time_since_last_second >= std::chrono::seconds(1))
    {
        fps = frame_ctr;
        frame_ctr = 0;
        time_since_last_second = std::chrono::nanoseconds(0);
    }
    else
    {
        time_since_last_second += delta_time;
    }
}
