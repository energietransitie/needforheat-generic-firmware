#include <util/screen.hpp>
#include <presence_detection.hpp>

#include <esp_log.h>

constexpr const char *TAG = "Screen";

constexpr float TEXT_SIZE_MAX = 3.5; // Maximum text size. Any bigger and it will clip the QR code.
constexpr float TEXT_SIZE_MIN = 1; // Minimum text size. Any smaller and the text becomes unreadable.

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

void Screen::DisplaySmartphones(std::string smartphones, uint8_t position)
{		
	int32_t size = m_display.width() - 20;
	uint8_t count = 0, yAxis = 30;
	static uint8_t prevPos = 0;
	std::vector<std::string> smartphoneStrings;

	m_display.setTextSize(2);
	m_display.drawString("Smartphones:", 10, 10);  

	// iterate over all onboarded smartphones
	smartphoneStrings = Strings::Split("+ toevoegen;" + smartphones, ';');
	//yAxis += 20*(position-1);
	
	for (const auto &smartphone : smartphoneStrings)
	{
		if(count == position)
		{
			m_display.setTextSize(2);  
		}
		else
		{
			m_display.setTextSize(1.5);  
		}
		if(position == 0 || count == prevPos || count == position)
		{
			//draw all the smartphones	
			m_display.fillRect(10, yAxis, size+10, 20);
			if (count == 0)
			{
				m_display.drawString((smartphone).c_str(), 10, yAxis);
			}
			else
			{
				m_display.drawString(("- " + smartphone).c_str(), 10, yAxis);
			}	
		}
		yAxis+=20;
		count++;
	}  	
	prevPos = position;
}

void Screen::InfoScreen()
{
	m_display.setTextSize(2);
	m_display.drawString("Toevoegen?", 10, 10); 

	m_display.setTextSize(1.5);
	m_display.drawString("Ga op je mobiel", 10, 30); 
	m_display.drawString("naar Bluetooth", 10, 45); 
	m_display.drawString("en koppel met", 10, 60); 
	m_display.drawString(PresenceDetection::getDevName().c_str(), 10, 75); 
	m_display.setTextSize(2);
	m_display.drawString("Terug", 10, m_display.height()-20); 

}


void Screen::RemoveSmartphone(std::string smartphones, uint8_t position, uint8_t phoneID)
{		
	int32_t size = m_display.width() - 20;
	uint8_t count = 0;
	std::vector<std::string> smartphoneStrings;

	m_display.setTextSize(2); 

	// loop true all current smartphones
	smartphoneStrings = Strings::Split(smartphones, ';');
	
	for (const auto &smartphone : smartphoneStrings)
	{
		if(count == phoneID)
		{
			m_display.drawString(smartphone.c_str(), 10, 10); 
			break;
		}
		count++;
	}  	
	
	m_display.drawString("Verwijderen?", 10, 30); 

	
	if(position == 1)
	{	
		m_display.setTextSize(1.5); 
		m_display.fillRect(10, 60, size, 30);
		m_display.drawString("Ja", 10, 60); 
		m_display.setTextSize(2); 
		m_display.fillRect(10, 90, size, 30);
		m_display.drawString("Nee", 10, 90); 
	}
	else
	{	
		m_display.fillRect(10, 60, size, 30);
		m_display.drawString("Ja", 10, 60); 
		m_display.setTextSize(1.5); 
		m_display.fillRect(10, 90, size, 30);
		m_display.drawString("Nee", 10, 90); 
	}
}
