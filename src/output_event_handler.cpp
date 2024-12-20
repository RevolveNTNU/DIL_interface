#include "DIL_interface/output_event_handler.h"

/**
 * @brief Construct a new Output Event Handler:: Output Event Handler object
 * 
 * @param path Path to the output device.
 */
OutputEventHandler::OutputEventHandler(const std::string& path) : devicePath(path), dev(nullptr), fd(-1), steering(0.0)
{
    fd = open(devicePath.c_str(), O_RDWR);
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

/**
 * @brief Destroy the Output Event Handler:: Output Event Handler object
 * 
 * Stops the feedback loop thread and frees the libevdev device and file descriptor.
 */
OutputEventHandler::~OutputEventHandler()
{
    stop();
    if (dev) libevdev_free(dev);
    if (fd >= 0) close(fd);
}

/**
 * @brief Start the feedback loop thread.
 * 
 * Launches the thread by passing in the non-static member function ``feedbackLoop()``
 * as reference. The thread will run until the shared ``RUNNING`` flag is set to false.
 */
void OutputEventHandler::start()
{
    RUNNING.store(true);
    outputThread = std::thread(&OutputEventHandler::feedbackLoop, this);
    std::cout << "Initialized feedback loop thread.\n";
}

/**
 * @brief Stop the feedback loop thread.
 * 
 * Sets the shared ``RUNNING`` flag to false and waits for the feedback loop thread to join.
 */
void OutputEventHandler::stop()
{
    RUNNING.store(false);

    if (outputThread.joinable())
    {
        outputThread.join();
    }
    std::cout << "Stopped feedback loop thread.\n";
}


/**
 * @brief Convert a normalized steering angle [-1, 1] to force feedback direction.
 *
 * @param steering Steering angle in range [-1, 1].
 * @return uint16_t Direction value (0x0000 - 0xFFFF).
 */
uint16_t OutputEventHandler::steeringToDirection(double steering)
{
    if (steering < -1.0) steering = -1.0;
    if (steering > 1.0) steering = 1.0;

    uint16_t direction = static_cast<uint16_t>(0x4000 + steering * 0x8000);

    return direction;
}

void OutputEventHandler::setSteering(double st)
{
    steering = st;
}

/**
 * @brief Send a constant force feedback effect to the output device.
 * 
 * This method is mostly just for testing and debugging purposes. I am currently using it
 * to get a feel for how the force feedback works and how it can be controlled.
 * This includes the related functions ``steeringToDirection()`` and ``setSteering()``.
 *
 * @param level Force level in range [-32767, 32767].
 * @param duration_ms Duration of the effect in milliseconds.
 */
void OutputEventHandler::sendConstantForce(int level, int duration_ms) 
{
    static int effect_id = -1;

    if (level < -MAX_FORCE || level > MAX_FORCE)
    {
        throw std::invalid_argument("Force level must be in range [-32767, 32767].\n");
    }

    struct ff_effect effect;
    memset(&effect, 0, sizeof(effect));

    effect.type = FF_CONSTANT;
    effect.id = effect_id;
    effect.u.constant.level = level;
    effect.direction = steeringToDirection(steering);
    effect.replay.length = duration_ms;

    if (ioctl(fd, EVIOCSFF, &effect) < 0) 
    {
        perror("Failed to upload FF_CONSTANT effect");
        return;
    }

    if (effect_id == -1) 
    {
    effect_id = effect.id;
    std::cout << "Uploaded FF_CONSTANT effect: ID = " << effect_id << '\n';
    }

    struct input_event play = {};
    play.type = EV_FF;        
    play.code = effect.id;    
    play.value = 1;           

    if (write(fd, &play, sizeof(play)) < 0) 
    {
        perror("Failed to play FF_CONSTANT effect");
    } else 
    {
        std::cout << "Force feedback sent: Level = " << level
                  << ", Duration = " << duration_ms << " ms\n";
    }
}

void OutputEventHandler::feedbackLoop()
{
    while (RUNNING.load())
    {
    }
}