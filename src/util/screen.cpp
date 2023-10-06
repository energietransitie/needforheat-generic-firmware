#include <util/screen.hpp>
#include <presence_detection.hpp>
#include <vector>
#include <string>

#include <esp_log.h>

constexpr const char *TAG = "Screen";

constexpr float TEXT_SIZE_MAX = 3.5; // Maximum text size. Any bigger and it will clip the QR code.
constexpr float TEXT_SIZE_MIN = 1; // Minimum text size. Any smaller and the text becomes unreadable.
#define TEXT_SIZE     2
#define MAX_MENU_SIZE 10  	// Maximum number of menu items (including title and final item)
#define MARGIN_TOP 10  		// Top margin in pixels
#define MARGIN_LEFT 10  	// Left margin in pixels
#define MARGIN_LINE 2	  	// Margin between lines in pixels

std::string Screen::s_infoURL;
std::string Screen::s_infoText;
int Screen::s_infoPadding = 0;

Screen::Screen()
{
	m_display.init_without_reset();
	m_display.setEpdMode(epd_mode_t::epd_text);

	ESP_LOGI(TAG, "Initialed display with width: %d and height: %d", m_display.width(), m_display.height());
}

void Screen::Clear()
{
	m_display.setEpdMode(epd_mode_t::epd_quality);
	m_display.clear();
	m_display.setEpdMode(epd_mode_t::epd_text);
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
		m_display.setTextSize(1.0F);
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

void Screen::DrawMenu(std::vector<std::string> menuLines, int highlightedLine) 
{
	Clear();
	m_display.setTextSize(TEXT_SIZE);
	int menuSize = menuLines.size();;
	int lineHeight = m_display.fontHeight() + MARGIN_LINE; // Get line height dynamically

	// Calculate the position of the last line (bottom of the screen)
	int lastLineY = m_display.height() - lineHeight;

	// Draw the title (not highlighted)
	m_display.setTextColor(TFT_BLACK);
	m_display.setCursor(MARGIN_LEFT, MARGIN_TOP); 
	m_display.print(menuLines[0].c_str());

	// Draw menu items
	for (int i = 1; i < menuSize - 1; i++)
	{
			if (i == highlightedLine)
			{
				// Highlighted item
				m_display.fillRect(0, i * lineHeight + MARGIN_TOP , m_display.width(), lineHeight, TFT_BLACK);
				m_display.setTextColor(TFT_WHITE);
			} else 
			{
				m_display.setTextColor(TFT_BLACK);
			}
			m_display.setCursor(MARGIN_LEFT, i * lineHeight + MARGIN_TOP );
			m_display.print(menuLines[i].c_str());
	}

	// Draw the last line (always at the bottom)
	if (highlightedLine == menuSize - 1)
	{
		m_display.fillRect(0, lastLineY, m_display.width(), lineHeight, TFT_BLACK);
		m_display.setTextColor(TFT_WHITE);
	} else 
	{
		m_display.setTextColor(TFT_BLACK);
	}
	m_display.setCursor(MARGIN_LEFT, lastLineY);
	m_display.print(menuLines[menuSize - 1].c_str());
	m_display.setTextColor(TFT_BLACK);
}


void Screen::SetInfoQRDetails(const std::string &payload, int padding, const std::string &text)
{
	s_infoURL = payload;
	s_infoPadding = padding;
	s_infoText = text;
}

void Screen::DisplayInfoQR()
{
	if (s_infoURL.empty())
		return;
	
	DisplayQR(s_infoURL, s_infoPadding, s_infoText);
}