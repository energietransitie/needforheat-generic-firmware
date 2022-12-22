#include <persistent_buffer.hpp>

#include <esp_spiffs.h>

#include <power_manager.hpp>
#include <util/error.hpp>

constexpr const char *TAG = "PersistentBuffer";

constexpr int UPLOAD_QUEUE_MAX = 50;

namespace SecureUpload
{
    std::vector<PersistentBuffer *> PersistentBuffer::s_buffers;
    ;

    std::vector<PersistentBuffer *> &PersistentBuffer::GetBuffers()
    {
        return s_buffers;
    }

    PersistentBuffer::PersistentBuffer()
        : m_memoryQueue(xQueueCreate(UPLOAD_QUEUE_MAX, sizeof(Measurements::Measurement *)))
    {
        static bool powerOffHookAdded = false;

        if (!powerOffHookAdded)
            PowerManager::GetInstance().AddPowerOffHook("SavePersistentBuffer", PersistAllBuffers);

        s_buffers.push_back(this);

        ESP_LOGD(TAG, "Queue created");
    }

    void PersistentBuffer::Add(const Measurements::Measurement &measurement)
    {
        auto measurementPtr = new Measurements::Measurement(std::move(measurement));

        auto err = xQueueSend(m_memoryQueue, &measurementPtr, 0);
        if (err != pdTRUE)
            ESP_LOGE(TAG, "An error occured when adding an item to the queue: no space.");
    }

    const PersistentBuffer::ItemsContainer &PersistentBuffer::GetItems()
    {
        m_tempItems = GetItemsFromMemory();
        m_tempItemsFromMemoryCount = m_tempItems.size();

        auto spiffsItems = GetItemsFromSPIFFS();

        // Append spiffsItems to the end of itemsMemory.
        m_tempItems.reserve(sizeof(m_tempItems) + sizeof(spiffsItems));
        m_tempItems.insert(m_tempItems.end(),
                           std::make_move_iterator(spiffsItems.begin()),
                           std::make_move_iterator(spiffsItems.end()));

        return m_tempItems;
    }

    void PersistentBuffer::PersistItems()
    {
        int totalSaved = 0;

        // We need to save items from memory that are no longer in the queue.
        if (m_tempItemsFromMemoryCount > 0)
        {
            auto tempItemsFromMemory = ItemsContainer(m_tempItems.begin(), m_tempItems.begin() + m_tempItemsFromMemoryCount);
            AppendToSPIFFS(tempItemsFromMemory);
            totalSaved += tempItemsFromMemory.size();
        }

        // Check if there are still items in the memory queue.
        auto itemsFromMemory = GetItemsFromMemory();
        if (itemsFromMemory.size() > 0)
        {
            AppendToSPIFFS(itemsFromMemory);
            totalSaved += itemsFromMemory.size();
        }

        ESP_LOGD(TAG, "Persisted %d items to from memory to persistent storage", totalSaved);
    }

    void PersistentBuffer::EraseItems()
    {
        ESP_LOGD(TAG, "Erasing items from SPIFFS");

        auto err = std::remove("/spiffs/buffer");
        if (err)
        {
            ESP_LOGE(TAG, "An error occured when removing file \"/spiffs/buffer\"");
            return;
        }

        ESP_LOGD(TAG, "Erased items from SPIFFS");
    }

    PersistentBuffer::ItemsContainer PersistentBuffer::GetItemsFromMemory()
    {
        ItemsContainer items;

        while (true)
        {
            auto item = new Measurements::Measurement();
            if (xQueueReceive(m_memoryQueue, &item, 0) != pdTRUE)
                break;

            items.push_back(item);
        }

        return items;
    }

    PersistentBuffer::ItemsContainer PersistentBuffer::GetItemsFromSPIFFS()
    {
        ESP_LOGD(TAG, "Loading items from SPIFFS");

        ItemsContainer items;

        auto file = std::fopen("/spiffs/buffer", "r");
        if (!file)
        {
            ESP_LOGW(TAG, "An error occured when opening file \"/spiffs/buffer\". File might not exist.");
            return {};
        }

        size_t size = 0;
        auto read = std::fread(&size, sizeof(size_t), 1, file);
        if (read != 1 || size == 0)
        {
            std::fclose(file);
            return {};
        }

        if (size == 0)
        {
            std::fclose(file);
            return {};
        }

        for (size_t i = 0; i < size; i++)
        {
            auto result = Measurements::DeserializeMeasurement(file);
            if (result.second != ESP_OK)
            {
                ESP_LOGW(TAG, "An error occured while deserializing persistent buffer");
                std::fclose(file);
                return {};
            }

            auto measurementPtr = new Measurements::Measurement(std::move(result.first));

            items.push_back(measurementPtr);
        }

        auto err = std::fclose(file);
        if (err != 0)
        {
            ESP_LOGE(TAG, "An error occured when closing file");
            return {};
        }

        ESP_LOGI(TAG, "Loaded %d items from SPIFFS", size);

        return items;
    }

    void PersistentBuffer::WriteToSPIFFS(const ItemsContainer &items)
    {
        ESP_LOGD(TAG, "Writing items to SPIFFS");

        auto file = std::fopen("/spiffs/buffer", "w");
        if (!file)
        {
            ESP_LOGE(TAG, "An error occured when opening file \"/spiffs/buffer\"");
            return;
        }

        auto size = SerializeToFile(items, file);

        auto err = std::fclose(file);
        if (err != 0)
        {
            ESP_LOGE(TAG, "An error occured when closing file \"/spiffs/buffer\"");
            return;
        }

        ESP_LOGD(TAG, "Saved %d items to SPIFFS", size);
    }

    void PersistentBuffer::AppendToSPIFFS(const ItemsContainer &items)
    {
        ESP_LOGD(TAG, "Appending items to SPIFFS");

        // Create new container with old items and add new items to it.
        auto allItems = GetItemsFromSPIFFS();
        
        allItems.insert(allItems.end(), items.begin(), items.end());

        WriteToSPIFFS(allItems);

        ESP_LOGI(TAG, "Appended %d measurements to SPIFFS", items.size());
    }

    size_t PersistentBuffer::SerializeToFile(const ItemsContainer &items, FILE *&file)
    {
        size_t size = items.size();
        if (size == 0)
        {
            return size;
        }

        auto written = std::fwrite(&size, sizeof(size_t), 1, file);
        if (written != 1)
            return written;

        int itemCount = 0;
        for (auto &item : items)
        {
            auto err = Measurements::SerializeMeasurement(*item, file);
            if (err != ESP_OK)
            {
                ESP_LOGE(TAG, "An error occured while serializing persistent buffer");
                return itemCount;
            }
            itemCount++;
        }

        return itemCount;
    }

    void PersistAllBuffers()
    {
        for (auto buffer : PersistentBuffer::GetBuffers())
        {
            if (buffer == nullptr)
            {
                ESP_LOGE(TAG, "Encountered a null reference to a PersistentBuffer");
                return;
            }

            buffer->PersistItems();
        }
    }
} // namespace SecureUpload
