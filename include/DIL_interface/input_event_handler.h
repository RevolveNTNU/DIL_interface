#ifndef INPUT_EVENT_HANDLER_H
#define INPUT_EVENT_HANDLER_H

#include "DIL_interface/shared_state.h"

#include <libevdev-1.0/libevdev/libevdev.h>
#include <string>
#include <thread>
#include <atomic>
#include <unordered_map>
#include <iostream>
#include <fcntl.h>
#include <unistd.h>
#include <stdexcept>
#include <chrono>
#include <dirent.h>
#include <cstring>

/**
 * @class InputEventHandler
 * @brief Handles input events from an input device using the libevdev library.
 * 
 * The InputEventHandler class opens the input device from ``/dev/input/``, initializes
 * it using libevdev, and manages its lifecycle. This includes processing axis events by 
 * event code and normalizing values to appropriate ranges.
 * 
 */
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
    struct libevdev *dev;    // libevdev device
    int fd;                  // File descriptor for the input device

    std::thread eventThread;

    double steering;
    double throttle;
    double brake;
    // std::unordered_map<InputType, bool> buttonStates;

    void eventLoop();
    void processAxisEvent(const struct input_event& ev);
    // TODO: void processButtonEvent(const struct input_event& ev);

    double normalizeYokeAxis(int value, int min, int max);
    double normalizePedalAxis(int value, int min, int max);

    // InputType mapInput(int type, int code);
};

#endif // INPUT_EVENT_HANDLER_H