#include "DIL_interface/input_event_handler.h"

/**
 * @brief Construct a new Input Event Handler:: Input Event Handler object
 * 
 * @param path Path to the input device.
 */
InputEventHandler::InputEventHandler(const std::string& path) : devicePath(path) ,dev(nullptr), fd(-1), steering(0.0), throttle(0.0), brake(0.0)
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

/**
 * @brief Destroy the Input Event Handler:: Input Event Handler object
 * 
 * Stops the event loop thread and frees the libevdev device and file descriptor.
 */
InputEventHandler::~InputEventHandler() 
{
    stop();
    if (dev) libevdev_free(dev);
    if (fd >= 0) close(fd);
}

/**
 * @brief Start the event loop thread
 * 
 * Launches the thread by passing in the non-static member function ``eventLoop()``
 * as reference. The thread will run until the shared ``RUNNING`` flag is set to false.
 */
void InputEventHandler::start()
{
    if (!dev)
    {
        std::cerr << "Error: libevdev device is not initialized.\n";
        return;
    }
    RUNNING.store(true);

    eventThread = std::thread(&InputEventHandler::eventLoop, this);
    std::cout << "Initialized event loop thread.\n";
}

/**
 * @brief Stop the event loop thread.
 * 
 * Sets the shared ``RUNNING`` flag to false and waits for the event loop thread to join.
 */
void InputEventHandler::stop()
{
    RUNNING.store(false);

    if (eventThread.joinable()) 
    {
        eventThread.join();
    }
    std::cout << "Stopped event loop thread.\n";
}

/**
 * @brief Event loop for processing input event types.
 * 
 * The event loop reads the next event from the input device and processes it.
 */
void InputEventHandler::eventLoop()
{
    struct input_event ev;
    while (RUNNING.load())
    {
        int rc = libevdev_next_event(dev, LIBEVDEV_READ_FLAG_NORMAL, &ev);
        if (rc == 0)
        {
            switch (ev.type)
            {
                case EV_ABS:
                    processAxisEvent(ev);
                    break;
                case EV_KEY:
                    // TODO: Process button events.
                    break;
                default:
                    break;
            }
        }
        else if (rc == -EAGAIN)
        {
            // Stalling the thread until the next event is available.
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

/**
 * @brief Process axis events from the input device.
 * 
 * This method processes axis events by event code and normalizes the values to
 * appropriate ranges.
 * 
 * @param ev Input event structure.
 */
void InputEventHandler::processAxisEvent(const struct input_event& ev)
{
    switch (ev.code)
    {
        case ABS_X:
            // Normalize steering value to range [-1, 1]
            steering = normalizeYokeAxis(ev.value, 0, 65535);
            break;
        case ABS_Y:
            // Normalize throttle value to range [0, 1]
            throttle = normalizePedalAxis(ev.value, 0, 255);
            break;
        case ABS_Z:
            // Normalize brake value to range [0, 1]
            brake = normalizePedalAxis(ev.value, 0, 255);
            break;
        default:
            break;
    }
}

/**
 * @brief Normalize the yoke axis value to the range [-1, 1].
 * @param value Raw axis event value.
 * @param min Minimum raw axis event value.
 * @param max Maximum raw axis event value.
 * @return double
 */
double InputEventHandler::normalizeYokeAxis(int value, int min, int max)
{
    return 2.0 * (value - min) / (max - min) - 1.0;
}

/**
 * @brief Normalize the pedal axis value to the range [0, 1].
 * @param value Raw axis event value.
 * @param min Minimum raw axis event value.
 * @param max Maximum raw axis event value.
 * @return double
 */
double InputEventHandler::normalizePedalAxis(int value, int min, int max)
{
    return (value - min) / (max - min);
}

/**
 * @brief Get the current normalized steering value.
 * @return double 
 */
double InputEventHandler::getSteering() const
{
    return steering;
}

/**
 * @brief Get the current normalized throttle value.
 * @return double 
 */
double InputEventHandler::getThrottle() const
{
    return throttle;
}

/**
 * @brief Get the current normalized brake value.
 * @return double 
 */
double InputEventHandler::getBrake() const
{
    return brake;
}

// Global helper functions
// TODO: Implement automatic connection to correct device.
// Simple terminal interface to continue with the option (Fanatec)

void listSupportedInputs(const std::string& devicePath) 
{
    int fd = open(devicePath.c_str(), O_RDONLY | O_NONBLOCK);
    if (fd < 0) 
    {
        std::cerr << "Failed to open device: " << devicePath << std::endl;
        return;
    }

    struct libevdev* dev = nullptr;
    if (libevdev_new_from_fd(fd, &dev) < 0) 
    {
        std::cerr << "Failed to initialize libevdev." << std::endl;
        close(fd);
        return;
    }

    std::cout << "Device: " << libevdev_get_name(dev) << std::endl;

    for (int code = 0; code < ABS_MAX; code++) 
    {
        if (libevdev_has_event_code(dev, EV_ABS, code)) 
        {
            const struct input_absinfo* abs = libevdev_get_abs_info(dev, code);
            std::cout << "Axis: " << libevdev_event_code_get_name(EV_ABS, code)
                      << " (Code: " << code << ")"
                      << ", Min: " << abs->minimum << ", Max: " << abs->maximum
                      << '\n';
        }
    }

    for (int code = 0; code < KEY_MAX; code++) 
    {
        if (libevdev_has_event_code(dev, EV_KEY, code)) 
        {
            std::cout << "Button: " << libevdev_event_code_get_name(EV_KEY, code)
                      << " (Code: " << code << ")" << '\n';
        }
    }

    libevdev_free(dev);
    close(fd);
}

bool isDeviceConnected(const std::string& devicePath) 
{
    int fd = open(devicePath.c_str(), O_RDONLY | O_NONBLOCK);
    if (fd < 0) return false; 

    struct libevdev* dev = nullptr;
    bool connected = false;
    if (libevdev_new_from_fd(fd, &dev) == 0) 
    {
        connected = true; 
        std::cout << "Device: " << libevdev_get_name(dev) << " at " << devicePath << '\n';
    }
    libevdev_free(dev);
    close(fd);
    return connected;
}

void checkInputDevices() 
{
    const std::string inputDir = "/dev/input";
    DIR* dir = opendir(inputDir.c_str());
    if (!dir) 
    {
        throw std::runtime_error("Failed to open input directory: " + inputDir + '\n');
    }

    struct dirent* entry;
    while ((entry = readdir(dir)) != nullptr) 
    {
        if (std::string(entry->d_name) == "." || std::string(entry->d_name) == "..")
            continue;

        std::string devicePath = inputDir + "/" + entry->d_name;

        if (isDeviceConnected(devicePath)) 
        {
            std::cout << "Device is active: " << devicePath << '\n';
        } else 
        {
            std::cout << "Device is not active: " << devicePath << '\n';
        }
    }
    closedir(dir);
}