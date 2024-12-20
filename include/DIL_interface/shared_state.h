#ifndef SHARED_STATE_H
#define SHARED_STATE_H

#include <atomic>

/**
 * @brief Shared boolean flag for thread synchronization.
 * 
 * Using atomic boolean flag to ensure thread safety and avoid undefined behavior.
 * ``RUNNING`` will always be visible to both threads. Atomicity ensures read and write
 * operations are performed atomically. i.e. a thread either sees the complete update
 * or the previous value, but never a partial update.
 * 
 */
extern std::atomic<bool> RUNNING;

#endif // SHARED_STATE_H