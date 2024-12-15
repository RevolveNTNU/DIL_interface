#ifndef INPUT_EVENT_HANDLER_H
#define INPUT_EVENT_HANDLER_H

#include <libevdev-1.0/libevdev/libevdev.h>
#include <string>
#include <thread>
#include <atomic>
#include <unordered_map>
#include <functional>
#include <iostream>
#include <fcntl.h>
#include <unistd.h>
#include <stdexcept>
#include <chrono>
#include <dirent.h>
#include <cstring>

enum class InputType
{
    NONE
    // Buttons
};

class InputEventHandler 
{
public:
    InputEventHandler(const std::string& devicePath);
    ~InputEventHandler();

    void start();
    void stop();

    double getSteering() const;
    double getThrottle() const;
    double getBrake() const;

private:
    std::string devicePath;
    struct libevdev *dev;
    int fd;

    std::thread eventThread;
    std::atomic<bool> running;

    double steering;
    double throttle;
    double brake;
    std::unordered_map<InputType, bool> buttonStates;

    void eventLoop();
    void processEvent(const struct input_event& ev);
    double normalizeAxis(int value, int min, int max);
    // InputType mapInput(int type, int code);
};

#endif // INPUT_EVENT_HANDLER_H