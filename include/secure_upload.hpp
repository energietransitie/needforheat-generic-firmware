#pragma once

#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>

#include <measurements.hpp>
#include <persistent_buffer.hpp>

namespace SecureUpload
{
    /**
     * A queue that holds measurement data.
     *
     * Use Queue::AddMeasurement(measurement) to add a measurement to the queue.
     * Use Queue::Upload() to upload the queue's contents to the backend server.
     */
    class Queue
    {
    public:
        /**
         * Get an instance of the queue. This is a singleton.
         *
         * @returns an instance of the queue.
         */
        static Queue &GetInstance();

        /**
         * Add a measurement to the queue. The measurements will be sent when Queue::Upload() is called.
         *
         * @param measurement The measurement to add to the queue.
         *
         */
        void AddMeasurement(const Measurements::Measurement &measurement);

        /**
         * Upload all measurements on the queue to the backend API.
         */
        void Upload();

    private:
        /**
         * Create a new queue.
         */
        Queue() = default;

        /**
         * Storage for measurements (memory and SPIFFS if necessary).
         */
        PersistentBuffer m_measurements;
    };
} // namespace Upload
