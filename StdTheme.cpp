
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
 * @date	Wednesday 27-May-2026 3:09 pm, on board Jetstar flight GK335 from Tokyo to Naha
 * @param	a_colourText	Pointer to the string containing the colour specification in the form "r,g,b"
 * @param	a_colour		Reference to a variable in which to return the colour as a COLORREF value
 * @return	KErrNone if the colour was successfully extracted
 * @return	KErrCorrupt if the input string was invalid
 */

int RStdTheme::extractColours(const char *a_colourText, COLORREF &a_colour)
{
	const char *red, *green, *blue;
	int redLength, greenLength, blueLength;
	int redValue, greenValue, blueValue;

	int retVal = KErrCorrupt;

	/* Colour keywords are separated by the ',' character so create and configure a TLex instance for scanning */
	TLex Lex(a_colourText, (int) strlen(a_colourText));

	Lex.SetWhitespace(" \t,");

	/* Extract the red, green, and blue components */
	red = Lex.NextToken(&redLength);
	green = Lex.NextToken(&greenLength);
	blue = Lex.NextToken(&blueLength);

	if (red != nullptr && green != nullptr && blue != nullptr)
	{
		/* Now put the components into std::string instances so that we can use Utils::StringToInt to validate */
		/* and convert them */
		std::string redString(red, redLength);
		std::string greenString(green, greenLength);
		std::string blueString(blue, blueLength);

		if (Utils::StringToInt(redString.c_str(), &redValue) == KErrNone &&
			Utils::StringToInt(greenString.c_str(), &greenValue) == KErrNone &&
			Utils::StringToInt(blueString.c_str(), &blueValue) == KErrNone)
		{
			/* All values are correct, so merge them into a single COLORREF */
			retVal = KErrNone;

			a_colour = RGB(redValue, greenValue, blueValue);
		}
	}

	return retVal;
}

/**
 * Determine the current OS colour scheme.
 * Queries the operating system for its current colour scheme, in order to be able to decide whether to use
 * Brunel's light or dark colour scheme by default.
 *
 * @date	Saturday 30-May-2026 07:11 am Best Western Okinawa Kouki Beach balcony
 * @return	true if a light colour scheme is in use, otherwise false
 */

bool RStdTheme::lightTheme()
{

#ifdef __amigaos__

	/* Lock the current screen in order to query its palette values */
	struct Screen *screen = LockPubScreen(NULL);

	/* Default to light theme if we can't obtain a screen lock */
	if (!screen)
	{
		return true;
	}

	/* Get background pen and convert it to an RGB value */
	ULONG rgb = GetRGB4(screen->ViewPort.ColorMap, screen->BlockPen);

	UnlockPubScreen(NULL, screen);

	/* Extract the RGB components from the packed value returned by GetRGB4() */
	int red = (rgb >> 16) & 0xff;
	int green = (rgb >> 8) & 0xff;
	int blue = rgb & 0xff;

	/* Calculate the luminance of the RGB values */
	int luminance = (red * 299 + green * 587 + blue * 114) / 1000;

	return (luminance >= 128);

#elif defined(QT_GUI_LIB)

	QStyleHints *styleHints = QGuiApplication::styleHints();

	return (styleHints->colorScheme() != Qt::ColorScheme::Dark);

#elif defined(WIN32)

	HKEY key;
	LONG result = RegOpenKeyExA(HKEY_CURRENT_USER, "Software\\Microsoft\\Windows\\CurrentVersion\\Themes\\Personalize",
		0, KEY_READ, &key);

	/* Default to light theme if we can't open the registry key */
	if (result != ERROR_SUCCESS)
	{
		return true;
	}

	DWORD value = 1;
	DWORD size = sizeof(value);
	RegQueryValueExA(key, "AppsUseLightTheme", nullptr, nullptr, (LPBYTE) &value, &size);

	RegCloseKey(key);

	return (value == 1);

#else /* ! WIN32 */

	return true;

#endif /* ! WIN32 */

}
