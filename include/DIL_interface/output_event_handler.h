#ifndef OUTPUT_EVENT_HANDLER_H
#define OUTPUT_EVENT_HANDLER_H

#include <libevdev-1.0/libevdev/libevdev.h>
#include <linux/input.h>
#include <string>
#include <thread>
#include <atomic>
#include <fcntl.h>
#include <unistd.h>
#include <iostream>
#include <cstring>

class OutputEventHandler
{
public:
    OutputEventHandler(const std::string& devicePath);
    ~OutputEventHandler();

    void start();
    void stop();

private:
    std::string devicePath;
    struct libevdev *dev;    // libevdev device
    int fd;                  // File descriptor for the input device

    std::thread outputThread;
    std::atomic<bool> running;

    void feedbackLoop();
    void sendConstantForce(int level, int duration_ms);
};

#endif // OUTPUT_EVENT_HANDLER_H