#ifndef SHARED_STATE_H
#define SHARED_STATE_H

#include <atomic>

// Using atomic boolean flag to ensure thread safety and avoid undefined behavior.
// ``RUNNING`` will always be visible to the event loop thread.
extern std::atomic<bool> RUNNING;

#endif // SHARED_STATE_H