#pragma once

#include <string>

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

private:
	/**
	 * LGFX display.
	 */
	LGFX m_display;
};
