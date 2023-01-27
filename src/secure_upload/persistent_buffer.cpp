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

        // Move to end, get position and return to beginning.
        std::fseek(file, 0, SEEK_END);
        auto endPos = std::ftell(file);
        std::fseek(file, 0, SEEK_SET);

        size_t itemsRead = 0;

        while (std::ftell(file) != endPos)
        {
            auto result = Measurements::DeserializeMeasurement(file);
            if (result.second != ESP_OK)
            {
                ESP_LOGW(TAG, "An error occured while deserializing an item to persistent buffer");
                continue;
            }

            auto measurementPtr = new Measurements::Measurement(std::move(result.first));

            items.push_back(measurementPtr);

            itemsRead++;
        }

        auto err = std::fclose(file);
        if (err != 0)
        {
            ESP_LOGE(TAG, "An error occured when closing file");
            return {};
        }

        ESP_LOGI(TAG, "Loaded %d items from SPIFFS", itemsRead);

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

        ESP_LOGD(TAG, "Wrote %d items to SPIFFS", size);
    }

    void PersistentBuffer::AppendToSPIFFS(const ItemsContainer &items)
    {
        ESP_LOGD(TAG, "Appending items to SPIFFS");

        auto file = std::fopen("/spiffs/buffer", "a");
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

        ESP_LOGD(TAG, "Appended %d items to SPIFFS", size);
    }

    size_t PersistentBuffer::SerializeToFile(const ItemsContainer &items, FILE *&file)
    {
        if (items.size() <= 0)
            return 0;

        int itemCount = 0;
        for (auto &item : items)
        {
            auto err = Measurements::SerializeMeasurement(*item, file);
            if (err != ESP_OK)
            {
                ESP_LOGW(TAG, "An error occured when serializing measurement");
                continue;
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
