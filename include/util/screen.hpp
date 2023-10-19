#pragma once

#include <string>
#include <vector>
#include <util/strings.hpp>

#define LGFX_USE_V1
#include <LovyanGFX.h>
#include <LGFX_AUTODETECT.hpp>

class Screen
{
public:
	/**
	 * Create a new screen object.
	 * This uses the LovyanGFX library.
	 */
	Screen();

	/**
	 * Clear the screen.
	 */
	void Clear();

	/**
	 * Show a QR code on the screen.
	 *
	 * @param payload QR code payload.
	 * @param padding Minimum mount of white pixels around QR code.
	 * @param text Optional text to show under the QR code.
	 */
	void DisplayQR(const std::string &payload, int padding, const std::string &text = "");

	/**
	 * Display smartphones.
	 * 
	 * @param smartphones String cointaing all smartphones
	 * @param position Current selected smartphone 
	 */
	void DisplaySmartphones(std::string smartphones, uint8_t position);

	/**
	 * Info prompt on how to participate with presence detection.
	 */
	void InfoScreen();

	/**
	 * Prompt to remove phone.
	 * 
	 * @param smartphones String cointaing all smartphones
	 * @param position Current selected button
	 * @param phoneID Current selected smartphone  
	 */
	void RemoveSmartphone(std::string smartphones, uint8_t position, uint8_t phoneID);

private:
	/**
	 * LGFX display.
	 */
	LGFX m_display;
	LGFX_Sprite canvas;
};
