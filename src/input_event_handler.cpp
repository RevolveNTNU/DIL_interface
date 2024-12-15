#include "input_event_handler.h"

InputEventHandler::InputEventHandler(const std::string& path) : devicePath(path) ,dev(nullptr), fd(-1), steering(0.0), throttle(0.0), brake(0.0), running(false) 
{
    fd = open(devicePath.c_str(), O_RDONLY | O_NONBLOCK);
    if (fd < 0) 
    {
        throw std::runtime_error("Failed to open input device from event: " + devicePath + '\n');
    }

    if (libevdev_new_from_fd(fd, &dev) < 0) 
    {
        throw std::runtime_error("Failed to initialize libevdev.\n");
    }

    std::cout << "Input device initialized successfully: " << libevdev_get_name(dev) << '\n';
}

InputEventHandler::~InputEventHandler() 
{
    stop();
    if (dev) libevdev_free(dev);
    if (fd >= 0) close(fd);
}

void InputEventHandler::start()
{
    if (!dev)
    {
        std::cerr << "Error: libevdev device is not initialized.\n";
        return;
    }
    running = true;
    
    eventThread = std::thread(&InputEventHandler::eventLoop, this);
    std::cout << "Initialized event loop thread.\n";
}

void InputEventHandler::stop()
{
    running = false;
    if (eventThread.joinable()) 
    {
        eventThread.join();
    }
    std::cout << "Stopped event loop thread.\n";
}

void InputEventHandler::eventLoop()
{
    struct input_event ev;
    while (running)
    {
        int rc = libevdev_next_event(dev, LIBEVDEV_READ_FLAG_NORMAL, &ev);
        if (rc == 0)
        {
            switch (ev.type)
            {
                case EV_ABS:
                    processEvent(ev);
                    break;
                case EV_KEY:
                    // TODO: Handle button events
                    break;
                default:
                    break;
            }
        }
        else if (rc == -EAGAIN)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
        else if (rc == -ENODEV)
        {
            std::cerr << "Device disconnected.\n";
            break;
        }
        else
        {
            std::cerr << "Error reading event (code " << rc << "): " << strerror(-rc) << '\n';
        }
    }
}

void InputEventHandler::processEvent(const struct input_event& ev)
{
    switch (ev.code)
    {
        case ABS_X:
            steering = normalizeAxis(ev.value, 0, 65535);
            break;
        case ABS_Y:
            throttle = normalizeAxis(ev.value, 0, 255);
            break;
        case ABS_Z:
            brake = normalizeAxis(ev.value, 0, 255);
            break;
        default:
            break;
    }
}

double InputEventHandler::normalizeAxis(int value, int min, int max)
{
    return 2.0 * (value - min) / (max - min) - 1.0;
}

double InputEventHandler::getSteering() const
{
    return steering;
}

double InputEventHandler::getThrottle() const
{
    return throttle;
}

double InputEventHandler::getBrake() const
{
    return brake;
}

void listSupportedInputs(const std::string& devicePath) {
    int fd = open(devicePath.c_str(), O_RDONLY | O_NONBLOCK);
    if (fd < 0) {
        std::cerr << "Failed to open device: " << devicePath << std::endl;
        return;
    }

    struct libevdev* dev = nullptr;
    if (libevdev_new_from_fd(fd, &dev) < 0) {
        std::cerr << "Failed to initialize libevdev." << std::endl;
        close(fd);
        return;
    }

    std::cout << "Device: " << libevdev_get_name(dev) << std::endl;

    for (int code = 0; code < ABS_MAX; code++) {
        if (libevdev_has_event_code(dev, EV_ABS, code)) {
            const struct input_absinfo* abs = libevdev_get_abs_info(dev, code);
            std::cout << "Axis: " << libevdev_event_code_get_name(EV_ABS, code)
                      << " (Code: " << code << ")"
                      << ", Min: " << abs->minimum << ", Max: " << abs->maximum
                      << '\n';
        }
    }

    for (int code = 0; code < KEY_MAX; code++) {
        if (libevdev_has_event_code(dev, EV_KEY, code)) {
            std::cout << "Button: " << libevdev_event_code_get_name(EV_KEY, code)
                      << " (Code: " << code << ")" << '\n';
        }
    }

    libevdev_free(dev);
    close(fd);
}

// Global helper functions
// TODO: Implement automatic connection to correct device.
// Simple terminal interface to continue with the option (Fanatec)

bool isDeviceConnected(const std::string& devicePath) {
    int fd = open(devicePath.c_str(), O_RDONLY | O_NONBLOCK);
    if (fd < 0) return false; 

    struct libevdev* dev = nullptr;
    bool connected = false;
    if (libevdev_new_from_fd(fd, &dev) == 0) {
        connected = true; 
        std::cout << "Device: " << libevdev_get_name(dev) << " at " << devicePath << '\n';
    }
    libevdev_free(dev);
    close(fd);
    return connected;
}

void checkInputDevices() {
    const std::string inputDir = "/dev/input";
    DIR* dir = opendir(inputDir.c_str());
    if (!dir) {
        throw std::runtime_error("Failed to open input directory: " + inputDir + '\n');
    }

    struct dirent* entry;
    while ((entry = readdir(dir)) != nullptr) {
        if (std::string(entry->d_name) == "." || std::string(entry->d_name) == "..")
            continue;

        std::string devicePath = inputDir + "/" + entry->d_name;

        if (isDeviceConnected(devicePath)) {
            std::cout << "Device is active: " << devicePath << '\n';
        } else {
            std::cout << "Device is not active: " << devicePath << '\n';
        }
    }
    closedir(dir);
}