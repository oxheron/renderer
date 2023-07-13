#pragma once

// external 
#include <GLFW/glfw3.h>
#include <unordered_map>

struct Key
{
    bool held = false;
    bool released = false;
    bool pressed = false;

    void update_pressed()
    {
        pressed = true;
        held = true;
    }

    void update_released()
    {
        released = true;
        held = false;
        pressed = false;
    }

    bool get_pressed()
    {
        if (pressed) 
        {
            pressed = false;
            return true;
        }

        return false;
    }

    bool get_released()
    {
        if (released)
        {
            released = false;
            return true;
        }

        return false;
    }
};

class Keyboard
{
private:
    std::unordered_map<size_t, Key> registered_keys;
public:
    void register_key(size_t key)
    {
        if (registered_keys.contains(key)) return;  
        registered_keys.insert({key, Key()});
    }

    Key& operator[](size_t key) 
    { 
        if (registered_keys.contains(key)) return registered_keys[key];
        else 
        {
            register_key(key);
            return registered_keys[key];
        }
    }
};
