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
	 * Set details used for the Info QR screen.
	 * 
	 * Show this screen by calling DisplayInfoQR.
	 *
	 * @param payload QR code payload.
	 * @param padding Minimum mount of white pixels around QR code.
	 * @param text Optional text to show under the QR code.
	 */
	void SetInfoQRDetails(const std::string &payload, int padding, const std::string &text = "");

	/**
	 * Show info QR screen.
	 * 
	 * The details used for this are set by calling SetInfoQRDetails(..).
	 * If no details were set, the function will do nothing.
	 */
	void DisplayInfoQR();

	/**
	 * Display menu.
	 * 
	 * @param menItems array of strins containing all menu items (first item: title, last item: back)
	 * @param selectedLine Current hightlighted item
	 * @param menuSize total number of items in menu.
	 */
	void DrawMenu(std::vector<std::string> lines, int selectedLine);

private:
	// Storage for Info QR.
	static std::string s_infoURL;
	static std::string s_infoText;
	static int s_infoPadding;

private:
	/**
	 * LGFX display.
	 */
	LGFX m_display;
};
