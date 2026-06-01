
#ifndef STDTHEME_H
#define STDTHEME_H

/**
 * A class for determining information about OS colour themes.
 * Enables the client to determine information such as whether the OS is using a light or dark theme.
 */

class RStdTheme
{
public:

	int extractColours(const char *a_colourText, COLORREF &a_colour);

	bool lightTheme();
};

#endif /* ! STDTHEME_H */
