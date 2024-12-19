#ifndef OUTPUT_EVENT_HANDLER_H
#define OUTPUT_EVENT_HANDLER_H

#include "DIL_interface/shared_state.h"

#include <libevdev-1.0/libevdev/libevdev.h>
#include <linux/input.h>
#include <string>
#include <thread>
#include <fcntl.h>
#include <unistd.h>
#include <iostream>
#include <cstring>

/**
 * @class OutputEventHandler
 * 
 * @brief Handles output events using ``ioctl()`` and the libevdev library.
 * 
 * The OutputEventHandler class opens the output device from ``/dev/input/``,
 * with write permissions, initializes it using libevdev, and manages its lifecycle.
 * This allows for sending force feedback effects to the output device.
 * 
 */
class OutputEventHandler
{
public:
    OutputEventHandler(const std::string& devicePath);
    ~OutputEventHandler();

    void start();
    void stop();

    uint16_t steeringToDirection(double steering);

    void setSteering(double st);

private:
    std::string devicePath;
    struct libevdev *dev;    // libevdev device
    int fd;                  // File descriptor for the input device

    std::thread outputThread;

    static constexpr int MAX_FORCE = 0x7FFF;

    void feedbackLoop();
    void sendConstantForce(int level, int duration_ms);

    double steering;         // Normalized steering value [-1, 1]
};

#endif // OUTPUT_EVENT_HANDLER_H