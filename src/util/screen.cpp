#include <util/screen.hpp>
#include <presence_detection.hpp>
#include <vector>
#include <string>

#include <esp_log.h>

constexpr const char *TAG = "Screen";

constexpr float TEXT_SIZE_MAX = 3.5; // Maximum text size. Any bigger and it will clip the QR code.
constexpr float TEXT_SIZE_MIN = 1; // Minimum text size. Any smaller and the text becomes unreadable.
#define M5_COREINK_TEXT_SIZE     2
// #define MAX_MENU_SIZE 10  	// Maximum number of menu items (including title and final item)
// #define M5_COREINK_MARGIN_TOP 10  		// Top margin in pixels
// #define M5_COREINK_MARGIN_LEFT 10  	// Left margin in pixels
// #define M5_COREINK_MARGIN_LINE 2	  	// Margin between lines in pixels

std::string Screen::s_infoURL;
std::string Screen::s_infoText;
int Screen::s_infoPadding = 0;

Screen::Screen()
{
	e_ink_display.init_without_reset();
	e_ink_display.setEpdMode(epd_mode_t::epd_text);

	// ESP_LOGI(TAG, "Initialed display with width: %d and height: %d", e_ink_display.width(), e_ink_display.height());
}

void Screen::Clear()
{
	e_ink_display.setEpdMode(epd_mode_t::epd_quality);
	e_ink_display.clear();
	e_ink_display.setEpdMode(epd_mode_t::epd_text);
}

void Screen::DisplayQR(const std::string &payload, int padding, const std::string &text)
{
	Clear();

	int32_t size = std::min(e_ink_display.width(), e_ink_display.height()) - padding * 2;

	if (!text.empty())
	{
		// Remove another padding from the QR code size.
		size -= padding;

		e_ink_display.setTextDatum(textdatum_t::middle_center);

		// Calculate text size.
		e_ink_display.setTextSize(1.0F);
		float textSize = ((float)e_ink_display.width() - (float)padding * 2.0f) / (float)e_ink_display.textWidth(text.c_str());
		textSize = std::max(std::min(textSize, TEXT_SIZE_MAX), TEXT_SIZE_MIN);
		e_ink_display.setTextSize(textSize);
		e_ink_display.setTextColor(TFT_BLACK, TFT_WHITE);

		int32_t displayCenter = e_ink_display.width() / 2;
		int32_t belowQRCenter = e_ink_display.height() - (e_ink_display.height() - (padding + size)) / 2;
		e_ink_display.drawString(text.c_str(), displayCenter, belowQRCenter);
	}

	e_ink_display.qrcode(payload.c_str(), (e_ink_display.width() - size) / 2, padding, size);

	// ESP_LOGI(TAG,
	// 		 "Displaying QR: screen_size = %dx%d, qr_size = %d, with text = %s",
	// 		 e_ink_display.width(),
	// 		 e_ink_display.height(),
	// 		 size,
	// 		 text.empty() ? "n" : "y");
}

void Screen::DrawMenu(std::vector<std::string> menuLines, int highlightedLine) 
{
	Clear();
	e_ink_display.setTextSize(M5_COREINK_TEXT_SIZE);
	int menuSize = menuLines.size();;

	// Draw the title (not highlighted)
	e_ink_display.setTextColor(TFT_BLACK, TFT_WHITE);
	e_ink_display.setCursor(0, 0); 
	e_ink_display.println(menuLines[0].c_str());

	// Draw menu items
	for (int i = 1; i < menuSize - 1; i++)
	{
			if (i == highlightedLine)
			{
				// Highlighted item: colors reversed
				e_ink_display.setTextColor(TFT_WHITE, TFT_BLACK);
			} else 
			{
				e_ink_display.setTextColor(TFT_BLACK, TFT_WHITE);
			}
			e_ink_display.println(menuLines[i].c_str());
	}

	// Draw the last line (always at the bottom)
	if (highlightedLine == menuSize - 1)
	{
		// Highlighted item: colors reversed
		e_ink_display.setTextColor(TFT_WHITE, TFT_BLACK);
	} else 
	{
		e_ink_display.setTextColor(TFT_BLACK, TFT_WHITE);
	}
	e_ink_display.setCursor(0, e_ink_display.height() - e_ink_display.fontHeight());
	e_ink_display.print(menuLines[menuSize - 1].c_str());
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