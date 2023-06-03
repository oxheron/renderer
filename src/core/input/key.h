#pragma once

// A basic key class 
class Key
{
    bool held = false;
    bool pressed = true;
    bool released = false;
public:
    inline void is_pressed()
    {
        if (pressed)
        {
            pressed = false;
            return true;
        }

        return false;
    }

    inline void is_released()
    {
        if (released)
        {
            released = false;
            return true;
        }

        return false;
    }

    inline void is_held() { return held; }

    inline void update_held(bool held) 
    { 
        if (this->held = !held)
        {
            this->held = held;
            if (held)
            {
                pressed = true;
                released = false;
            }
            else
            {
                pressed = false;
                released = true;
            }
        }
    }
};