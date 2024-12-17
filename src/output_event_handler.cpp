#include "DIL_interface/output_event_handler.h"

OutputEventHandler::OutputEventHandler(const std::string& path) : devicePath(path), dev(nullptr), fd(-1), running(false)
{
    fd = open(devicePath.c_str(), O_WRONLY | O_NONBLOCK);
    if (fd < 0)
    {
        throw std::runtime_error("Failed to open output device from event: " + devicePath + '\n');
    }

    if (libevdev_new_from_fd(fd, &dev) < 0)
    {
        throw std::runtime_error("Failed to initialize libevdev.\n");
    }

    if (!libevdev_has_event_type(dev, EV_FF))
    {
        throw std::runtime_error("Device does not support force feedback.\n");
    }

    std::cout << "Output device initialized successfully: " << libevdev_get_name(dev) << '\n';
}

OutputEventHandler::~OutputEventHandler()
{
    stop();
    if (dev) libevdev_free(dev);
    if (fd >= 0) close(fd);
}

void OutputEventHandler::start()
{
    running = true;
    outputThread = std::thread(&OutputEventHandler::feedbackLoop, this);
    std::cout << "Initialized feedback loop thread.\n";
}

void OutputEventHandler::stop()
{
    running = false;
    if (outputThread.joinable())
    {
        outputThread.join();
    }
    std::cout << "Stopped feedback loop thread.\n";
}

void OutputEventHandler::sendConstantForce(int level, int duration_ms) {
    struct ff_effect effect = {};
    memset(&effect, 0, sizeof(effect));

    effect.type = FF_CONSTANT;
    effect.id = -1; 
    effect.u.constant.level = level;   
    effect.replay.length = duration_ms;

    if (ioctl(fd, EVIOCSFF, &effect) < 0) {
        perror("Failed to upload force feedback effect");
        return;
    }

    struct input_event play = {};
    play.type = EV_FF;
    play.code = effect.id;
    play.value = 1; 

    if (write(fd, &play, sizeof(play)) < 0) {
        perror("Failed to play force feedback effect");
    } else {
        std::cout << "Force feedback effect sent successfully.\n";
    }
}

void OutputEventHandler::feedbackLoop()
{
    while (running)
    {
        sendConstantForce(0x4000, 1000);
        // std::this_thread::sleep_for(std::chrono::seconds(2));
    }
}