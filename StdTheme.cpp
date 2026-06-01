
#include "StdFuncs.h"
#include "Lex.h"
#include "StdTheme.h"
#include <string.h>

#ifdef __amigaos__

#include <proto/graphics.h>

#elif defined(QT_GUI_LIB)

#include <QStyleHints>
#include <QApplication>

#endif /* QT_GUI_LIB */

/**
 * Extract red, green and blue values from a string.
 * This helper function can be used for pasing user strings that specify an RGB colour in the form "r,g,b" in an
 * input string. It will validate that the string contains three values and that those values are within the range
 * 0-255, and will return those values or an error indicating that the input string is invalid.
 *
 * @date	Wednesday 27-May-2026 3:09 pm, on board Jetstar flight JQ TODO from Tokyo to Naha
 * @param	Parameter		Description
 * @return	Return value
 */

int RStdTheme::extractColours(const char *a_colourText, COLORREF &a_colour)
{
	const char *red, *green, *blue;
	int redLength, greenLength, blueLength;
	int redValue, greenValue, blueValue; // TODO: CAW - Dumb names

	int retVal = KErrCorrupt;

	/* Comment keywords are separated by the ';' character so create and configure a TLex instance for scanning */
	TLex Lex(a_colourText, (int) strlen(a_colourText));

	// TODO: CAW - Using the destructive version causes calls to NextToken() and SetWhitespace() to crash
	//TLex Lex(m_pcText);

	Lex.SetWhitespace(" \t,");

	red = Lex.NextToken(&redLength);
	green = Lex.NextToken(&greenLength);
	blue = Lex.NextToken(&blueLength);

	if (red != nullptr && green != nullptr && blue != nullptr)
	{
		std::string redString(red, redLength);
		std::string greenString(green, greenLength);
		std::string blueString(blue, blueLength);

		if (Utils::StringToInt(redString.c_str(), &redValue) == KErrNone &&
			Utils::StringToInt(greenString.c_str(), &greenValue) == KErrNone &&
			Utils::StringToInt(blueString.c_str(), &blueValue) == KErrNone)
		{
			retVal = KErrNone;

			a_colour = RGB(redValue, greenValue, blueValue);
		}
	}

	return retVal;
}

/**
 * Determine the current colour scheme.
 * Queries the operating system for its current colour scheme, in order to be able to decide whether to use
 * Brunel's light or dark colour scheme by default.
 *
 * @date	Saturday 30-May-2026 07:11 am Best Western Kouri Beach balcony TODO
 * @return	true if a light colour scheme is in use, otherwise false
 */

bool RStdTheme::lightTheme()
{

#ifdef __amigaos__

	struct Screen *screen = LockPubScreen(NULL);

	if (!screen)
	{
		return false;  // Default to light
	}

	// Get background pen
	ULONG bgPen = screen->DetailPen;  // Or use screen->BlockPen

	// Get the RGB for this pen from the palette
	ULONG rgb = GetRGB4(screen->ViewPort.ColorMap, bgPen);

	UnlockPubScreen(NULL, screen);

	// Extract RGB components from the packed value
	// GetRGB4 returns in 0x00RRGGBB format
	int r = (rgb >> 16) & 0xFF;
	int g = (rgb >> 8) & 0xFF;
	int b = rgb & 0xFF;

	// Calculate luminance
	int luminance = (r * 299 + g * 587 + b * 114) / 1000;

	return (luminance < 128);

#elif defined(QT_GUI_LIB)

	QStyleHints *styleHints = QGuiApplication::styleHints();

	//qDebug() << "ColorScheme:" << styleHints->colorScheme();

	return (styleHints->colorScheme() != Qt::ColorScheme::Dark);

	/*QColor bgColor = QApplication::palette().color(QPalette::WindowText);
	qDebug() << "Base color:" << bgColor.red() << bgColor.green() << bgColor.blue();

	// Calculate luminance using standard formula
	// Weights: R=0.299, G=0.587, B=0.114
	int luminance = (bgColor.red() * 299 +
					 bgColor.green() * 587 +
					 bgColor.blue() * 114) / 1000;

	// If background is dark (luminance < 128), it's dark theme
	return luminance < 128;*/

#else /* ! QT_GUI_LIB */

	HKEY hKey;
	LONG result = RegOpenKeyExA(HKEY_CURRENT_USER,
								"Software\\Microsoft\\Windows\\CurrentVersion\\Themes\\Personalize",
								0, KEY_READ, &hKey);

	if (result != ERROR_SUCCESS)
	{
		return false;  // Default to light
	}

	DWORD value = 1;
	DWORD size = sizeof(value);
	RegQueryValueExA(hKey, "AppsUseLightTheme", nullptr, nullptr,
					 (LPBYTE)&value, &size);

	RegCloseKey(hKey);

	return value == 1;  // 0 = dark, 1 = light

#endif /* ! QT_GUI_LIB */

}
