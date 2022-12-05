#pragma once

#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>

namespace Mutex
{
    /**
     * Mtx is a FreeRTOS with Take() and Give() functions.
     */
    class Mtx
    {
    public:
        /**
         * Create a new mutex.
         */
        Mtx();

        /**
         * Destroy the mutex. This tries to take the mutex and WILL wait for it.
         */
        ~Mtx();

        /**
         * Take the mutex.
         *
         * Wait until it is available.
         */
        void Take();

        /**
         * Take the mutex.
         *
         * Wait until xBlockTime expires.
         *
         * @param xBlockTime Maximum time to wait for the mutex before continuing.
         *
         * @returns true if the mutex was taken. false if it was not taken.
         */
        bool Take(TickType_t xBlockTime);

        /**
         * Give the mutex.
         */
        void Give();

    private:
        SemaphoreHandle_t m_semaphoreHandle;
    };

    /**
     * ScopedLock can lock a mutex and will unlock it when this object goes out of scope.
     */
    class ScopedLock
    {
    public:
        ScopedLock() = delete;

        /**
         * Create a new ScopedLock. This will automatically try to take the mutex.
         * It will wait indefinately for the mutex.
         *
         * @param mtx The mutex to lock.
         */
        ScopedLock(Mtx &mtx);

        /**
         * Create a new ScopedLock. This will automatically try to take the mutex.
         *
         * @param mtx The mutex to lock.
         * @param xBlockTime Maximum time to wait for the mutex before continuing.
         */
        ScopedLock(Mtx &mtx, TickType_t xBlockTime);

        /**
         * Delete the ScopedLock. This will release the mutex.
         *
         * This will be called automatically when the ScopedLock goes out of scope.
         */
        ~ScopedLock();

        bool MtxWasTaken();

    private:
        /**
         * Reference to the mutex to take and give.
         */
        Mtx &m_mtx;

        /**
         * This value keeps track of the fact if the mutex was taken or not.
         * Useful for when ScopedLock was created with a blockTime.
         */
        bool m_mtxWasTaken;
    };
} // namespace Mutex
