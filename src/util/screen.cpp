#include <util/screen.hpp>

#include <esp_log.h>

constexpr const char *TAG = "Screen";

constexpr float TEXT_SIZE_MAX = 3.5; // Maximum text size. Any bigger and it will clip the QR code.
constexpr float TEXT_SIZE_MIN = 1; // Minimum text size. Any smaller and the text becomes unreadable.

Screen::Screen()
{
	m_display.init_without_reset();
	m_display.setEpdMode(epd_mode_t::epd_text);

	ESP_LOGI(TAG, "Initialed display with width: %d and height: %d", m_display.width(), m_display.height());
}

void Screen::Clear()
{
	m_display.clear();
}

void Screen::DisplayQR(const std::string &payload, int padding, const std::string &text)
{
	Clear();

	int32_t size = std::min(m_display.width(), m_display.height()) - padding * 2;

	if (!text.empty())
	{
		// Remove another padding from the QR code size.
		size -= padding;

		m_display.setTextDatum(textdatum_t::middle_center);

		// Calculate text size.
		float textSize = ((float)m_display.width() - (float)padding * 2.0f) / (float)m_display.textWidth(text.c_str());
		textSize = std::max(std::min(textSize, TEXT_SIZE_MAX), TEXT_SIZE_MIN);
		m_display.setTextSize(textSize);

		int32_t displayCenter = m_display.width() / 2;
		int32_t belowQRCenter = m_display.height() - (m_display.height() - (padding + size)) / 2;
		m_display.drawString(text.c_str(), displayCenter, belowQRCenter);
	}

	m_display.qrcode(payload.c_str(), (m_display.width() - size) / 2, padding, size);

	ESP_LOGI(TAG,
			 "Displaying QR: screen_size = %dx%d, qr_size = %d, with text = %s",
			 m_display.width(),
			 m_display.height(),
			 size,
			 text.empty() ? "n" : "y");
}
