#pragma once

#include <vector>

#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>

#include <measurements.hpp>

namespace SecureUpload
{
    class PersistentBuffer
    {
    public:
        using ItemsContainer = std::vector<Measurements::Measurement *>;

        /**
         * Get a list of all buffers that were created.
         * Used for persisting buffers in PowerOffHook().
         */
        static std::vector<PersistentBuffer *> &GetBuffers();

    public:
        /**
         * Create a new PersistentBuffer.
         */
        PersistentBuffer();

        /**
         * Add a new item to the queue.
         *
         * @param measurement Measurement to add.
         */
        void Add(const Measurements::Measurement &measurement);

        /**
         * Get all stored items.
         * This can include items in memory and items previously saved to SPIFFS.
         *
         * @returns Vector filled with all items.
         */
        const ItemsContainer &GetItems();

        /**
         * Persist all items to SPIFFS to survive power cycle.
         */
        void PersistItems();

        /**
         * Erase items in persistent buffer.
         *
         * This can be called once you know persistence and items
         * are no longer needed, to prevent reading items again.
         */
        void EraseItems();

    private:
        /**
         * Get Items stored in memory.
         *
         * @returns Vector filled with items from memory.
         */
        ItemsContainer GetItemsFromMemory();

        /**
         * @returns Vector filled with items from SPIFFS.
         */
        ItemsContainer GetItemsFromSPIFFS();

        /**
         * Write items to SPIFFS while overwriting old items.
         *
         * @param items Items to write to SPIFFS.
         */
        void WriteToSPIFFS(const ItemsContainer &items);

        /**
         * Write items to SPIFFS while leaving old items intact.
         *
         * @param items Items to write to SPIFFS.
         */
        void AppendToSPIFFS(const ItemsContainer &items);

        /**
         * Serialize items to a file.
         *
         * @param items Items to serialize to file.
         * @param file File to serialize items into.
         *
         * @returns The amount of items serialized.
         */
        size_t SerializeToFile(const ItemsContainer &items, FILE *&file);

    private:
        /**
         * Queue of measurements in memory.
         */
        QueueHandle_t m_memoryQueue;

        /**
         * Temporary storage for items (from memory and SPIFFS) when being read.
         */
        ItemsContainer m_tempItems;

        /**
         * Amount of items in m_tempItems that came from memory, not SPIFFS.
         * The first n items in m_tempItems are from memory.
         */
        size_t m_tempItemsFromMemoryCount;

    private:
        /**
         * List of all persistent buffers that were created.
         * Used for persisting buffers in PowerOffHook().
         */
        static std::vector<PersistentBuffer *> s_buffers;
    };

    /**
     * PowerOffHook that gets called before powering off.
     * This hook persists all buffers.
     */
    void PersistAllBuffers();
} // namespace SecureUpload
